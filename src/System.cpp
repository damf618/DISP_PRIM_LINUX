#include "System.h"
#include "RFinterface.h"
#include "File_Interface.h"
#include "primario4.h"
#include "Debug_Logging.h"

#include <cstdbool>
#include <stdio.h>  /* for puts() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for sleep() */
#include <stdlib.h> /* for EXIT_SUCCESS */
#include <stdbool.h>
#include <signal.h> 

pthread_t Threads_Pointer[PROGRAM_THREADS];
pthread_mutex_t mutexprim = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexconsola = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexspi = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexfile = PTHREAD_MUTEX_INITIALIZER;

sem_t Update_sem; 
sem_t Comm_error_sem; 

timer_t timerid;
struct sigevent sev;
struct itimerspec trigger;

volatile sig_atomic_t sig_flag;

dprimario_t prim;

// RF Incomplete Tmer event corresponding task  
/** The system keeps track of how many RF Devices are actively communicating with the
 * master device, in case there are fewer active nodes than the mininum number of nodes 
 * stablished, the timer is set. If this process has happenned more than 
 * MAX_INCOMPLETE_ERRORS times, then we generate a signal to activate the 
 * RF_Maintenance_thread and restart the full system.  
	
	@param sigval timer argument casted to a pointer to struct dprimario_t containing 
	the entire variables needed fot the system.
	@see Current_State,RF_Maintenance_thread
**/
void thread_handler(union sigval sv)
{
   dprimario_t *pPrimario = (dprimario_t *)sv.sival_ptr;
   pPrimario->Incomplete_counter++;
   pPrimario->Incomplete_flag=0;
   if(pPrimario->Incomplete_counter>=MAX_INCOMPLETE_ERRORS)
   {
	   
	   Debug_Message_Warning("Fire Monitor System: RF Devices Incomplete ");
	   printf("Incomplete Reset\n");
	   pPrimario->Incomplete_counter=0;
	   sem_post(&Comm_error_sem);			//Semaphore to activate the Maintenance thread
   }
}

// Reset of the previous states used to compare an event  
/** In order for the system to notify a new event it needs to acknowledge which is the
 *  current state, to make this comparison we save the previous state of the entire 
 *  system  and this also applies to the Comm (RF) result .   
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@see RF_Maintenance_thread
**/
void RF_Reset_State(dprimario_t * prim)
{
	prim->previous_state=INITIAL_DEFAULT_STATE;
	prim->previous_comm_state=INITIAL_COMM_DEFAULT_STATE;
}

// Block of any of the unexpected signals
/** The system is based on Linux SO, to avoid the interference of unexpected signals,
 * this function allows to block all the signals, this way is possible to unblock
 * only the desire ones.     
	
	@note It must be called on the principal thread before the creation of any thread.
	@see desbloquearSign, SIG_Handler,Init_All. Kill_Them_All
**/
static void bloquearSign(void)
{
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

// Unblock thee SIGINT and SIGTERM signals
/** The system is based on Linux SO, to avoid the interference of unexpected signals,
 * this function allows to unblock aa specific set of signals, this way is possible to 
 * generate a handler for the specific allowed signals.     
	
	@note It must be called on the principal thread, after the creation of the threads.
	@see bloquearSign, SIG_Handler, Init_All, Kill_Them_All
**/
static void desbloquearSign(void)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

// Handler for the Closing Signal
/** The system is based on Linux SO, this handler modifies the status of the sig_flag 
 * variable to notify the Main thread to execute the closing procedures.     
	
	@note It will be executed automatically and handled on the main thread (if bloquearSign and 
	desbloquearSign were used properly).
	@see bloquearSign, desbloquearSign, Init_All, Kill_Them_All
**/
static void SIG_Handler(int sig)
{
// Special variable to notify the main thread of the Termination Signal	
	sig_flag=TRUE;				      
}

// Organized Exit of the system
/** The system is based on Linux SO, this function closes system properly:
 * Closes the log file.
 * Stops Threads  in execution     
	
	@note It will be executed after the SIG_Handler on the main thread (if bloquearSign and 
	desbloquearSign were used properly).
	@see bloquearSign, desbloquearSign, Init_All, SIG_Handler
**/
void Kill_Them_All(void)
{				
	int thread_cancel;
	int i;

	if(TRUE==sig_flag){
		printf("\n Procesando SIG. Eliminando Threats y Cerrando Sockets\n");
		for(i=0;i<PROGRAM_THREADS;i++)
		{
			thread_cancel=pthread_cancel (Threads_Pointer[i]);
			if(thread_cancel<0)
			{
				Debug_Message_Error("\n Error while closing Threads \n");
				perror("\n Impossible to close the Thread correctly \n");
			}
			else
			{
				perror("Succesfully Cancel signal");
			}
			thread_cancel=pthread_join (Threads_Pointer[i], NULL);
			if(FALSE!=thread_cancel)
			{	
				Debug_Message_Error("\n Error while joining Threads \n");
				perror("\n Not possible to join the Thread \n");
			}
			else
			{
				perror("Succesfully Joined Thread");
			}
		}
		
		Debug_Message_Notice("RF Reset ");
		printf("Freeing Dynamic Memory\n");
		RF_Comm_Clean();

		printf("Cleaning List  of RF Devices\n");
		Clean_File_Interface("dhcplist.txt");
		
		exit(1);
	}
}

// Thread 4/4FIXING in charge of Executing Maintenance Tasks when needed 
/** The system keeps  track of different error cases related to RF Communication:
 *      # Error(NO COMM)
 *      # Incomplete(Parcial COMM)
 * 
 * Error: means that there is not even one RF device connected, the system follows a 
 * specific protocol for this cases that consists in "channel surfinng", based on 
 * the experience and the RF spectrum three channels were selected from the 125 channels
 * available to be used as an alternative in case we do not receive any message in a
 * channel on a specific time.
 * 
 * Incomplete: Makes reference to the case where we do have communication with one or more
 * RF Secondary device but no enough to reach the min nodes value configured, for example:
 * if we expect 2 nodes and we  only have messages from one device.
 * 
 * This task is in charge of restart the full RF Communication scheme in order to try 
 * to solve a reduced set of possible errors.   
	
	@note It is blocked until the "give" action of the semaphore.
	@see CurrentState, thread_handler 
**/
void* RF_Maintenance_thread (void*parmthread)
{
	while(1){
		sem_wait(&Comm_error_sem);
		pthread_mutex_lock (&mutexspi);
		pthread_mutex_lock (&mutexconsola);
		pthread_mutex_lock (&mutexfile);
		Debug_Message_Warning("Fire Monitor System: Executing Maintenace Tasks");
		Clean_File_Interface("dhcplist.txt");
		RF_Comm_Clean();
		RF_Reset_State(&prim);
		RF_Comm_Maintenance();
		pthread_mutex_unlock (&mutexfile);
		pthread_mutex_unlock (&mutexconsola);
		pthread_mutex_unlock (&mutexspi);
	}
}

// Thread 3/4 responsable to notify the user of the status of the system
/** This thread is in charge of the update of the system, it requieres the use of the log
 * file, the console and the reads from the 'dprimario_t *' structure.
	
	@note By default it will be called every second.
	@see timestamp, CurrentState
**/
void* Check_thread (void*parmthread)
{
	while(1){
		
		pthread_mutex_lock (&mutexprim);
		pthread_mutex_lock (&mutexconsola);
		pthread_mutex_lock (&mutexfile);
		Debug_Message_Debug("Fire Monitor System: File Update");
		CurrentState(&prim);
		pthread_mutex_unlock (&mutexfile);
		pthread_mutex_unlock (&mutexconsola);
		pthread_mutex_unlock (&mutexprim);
		sleep(CHECK_INTERVAL);
	}
}

// Thread 2/4 It calls the FSM of the system to process the updated information. 
/** This thread executes the entire system logic, if there is any update on the inputs,
 * the new stage and actions will be executed from this Thread. it requires access to 
 * the 'dprimario_t *' structure and console (For DEBUG).  
	
	@note Is not the only FSM involved, the RF beahvior is responsability of another one, 
	inside.
	
	@see timestamp, CurrentState
**/
void* Control_thread (void*parmthread)
{
	while(1){
		sem_wait(&Update_sem);
		pthread_mutex_lock (&mutexprim);
		pthread_mutex_lock (&mutexconsola);	
		Debug_Message_Debug("Fire Monitor System: Control Tasks");
		primControl(&prim);
		pthread_mutex_unlock (&mutexconsola);
		pthread_mutex_unlock (&mutexprim);
		usleep(CONTROL_INTERVALU);
	}
}

// Thread 1/4 to init the whole system/
int Init_All(void)
{
	int error_check;
	struct sigaction SIGINT_sa;
	struct sigaction SIGTERM_sa;
	
	Init_Debug();
	Debug_Message_Notice("Fire Monitor System: ON");
	
	printf("RPI DISPOSITIVO PRIMARIO");
	primInit(&prim);
	
	//SIGTERM Signal Configuration 
	SIGTERM_sa.sa_handler = SIG_Handler;
	SIGTERM_sa.sa_flags = 0; 
	sigemptyset(&SIGTERM_sa.sa_mask);

	if ( ERRORES == sigaction(SIGTERM, &SIGTERM_sa, NULL)) {
		Debug_Message_Error("Fire Monitor System: SIGTERM Error");
		perror("Error setting the configuration for the SIGTERM Handler  ");
		exit(ERROR_EXIT);
	}
	else{
		perror("* ------Configuracion de SIGTERM:");
	}
	
	//SIGINT Signal Configuration 
	SIGINT_sa.sa_handler = SIG_Handler;
	SIGINT_sa.sa_flags = 0; 
	sigemptyset(&SIGINT_sa.sa_mask);

	if ( ERRORES == sigaction(SIGINT, &SIGINT_sa, NULL)) {
		Debug_Message_Error("Fire Monitor System: SIGINT Error");
		perror("Error setting the configuration for the SIGINT Handler  ");
		exit(ERROR_EXIT);
	}
	else{
		perror("* ------Configuracion de SIGINT:");
	}
	
	bloquearSign();
	
	memset(&sev, 0, sizeof(struct sigevent));
	memset(&trigger, 0, sizeof(struct itimerspec));

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = &thread_handler;
	sev.sigev_value.sival_ptr = &prim;

	timer_create(CLOCK_REALTIME, &sev, &timerid);
	trigger.it_value.tv_sec = 5;
	
	
	//Init File
	Init_File_Interface();

	Clean_File_Interface("dhcplist.txt");

	//Thread creation and configuration
	error_check = pthread_create (&Threads_Pointer[0], NULL,Control_thread, NULL);
	if (error_check) 
	{
		Debug_Message_Error("Fire Monitor System: Update FSM Error");
		perror("Error during the creation of the Update FSM Thread");
		return -1;
	}
	else{
		perror("* --Creation of Update FSM Thread:");
	}
	
	//Creation of the "Thread State"
	error_check = pthread_create (&Threads_Pointer[1], NULL,Check_thread, NULL);
	if (error_check) 
	{
		Debug_Message_Error("Fire Monitor System: Current State Error");
		perror("Error during the creation of the Current State Thread");
		return -1;
	}
	else{
		perror("* Creation of Current State Thread:");
	}	
	
	//Creation of RF_Maintenance Thread
	error_check = pthread_create (&Threads_Pointer[2], NULL,RF_Maintenance_thread, NULL);
	if (error_check) 
	{
		Debug_Message_Error("Fire Monitor System: RF Maintenance Error");
		perror("Error during the creation of the RF Maintenace Thread Thread");
		return -1;
	}
	else{
		perror("* --Creation of Update FSM Thread:");
	}
	
	desbloquearSign();	
	Nodes_Config(&prim);
	
	Debug_Message_Debug("Fire Monitor System: Config. OK!");
	
	while( true )
	{			
		pthread_mutex_lock (&mutexprim);
		pthread_mutex_lock (&mutexspi);
		primUpdates(&prim);
		pthread_mutex_unlock (&mutexspi);
		pthread_mutex_unlock (&mutexprim);
		sem_post(&Update_sem);
		Kill_Them_All();
		usleep(UPDATE_INTERVALU);
	}
	return-1;
	
}
