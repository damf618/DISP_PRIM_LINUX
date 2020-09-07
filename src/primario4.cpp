/*=============================================================================
 * Copyright (c) 2020, DANIEL MARQUEZ <DAMF618@GMAIL.COM>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2020/04/12
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "primario4.h"
#include "antirebote.h"
#include "Primario_LEDS.h"
#include <cstdbool>
#include <cstdio>
#include <RF24DP.h>

/*=====[Definition macros of private constants]==============================*/

unsigned int counter=0;
bool hoopingchannel=true;

RF24 radio (22,0);
RF24Network network(radio);
RF24DP FireComm(radio,network);

dprimario_t prim;
volatile sig_atomic_t sig_flag;

pthread_t Threads_Pointer[PROGRAM_THREADS];

pthread_mutex_t mutexprim = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexconsola = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexspi = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexfile = PTHREAD_MUTEX_INITIALIZER;


sem_t Update_sem; 

FILE * statesfd;

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
		for(i=0;i<PROGRAM_THREADS;i++){
			thread_cancel=pthread_cancel (Threads_Pointer[i]);
			if(thread_cancel<0){
				perror("\n Impossible to close the Thread correctly \n");
			}
			else{
				perror("Succesfully Cancel signal");
			}
			thread_cancel=pthread_join (Threads_Pointer[i], NULL);
			if(FALSE!=thread_cancel){		//Esperar a que sean eliminados
				perror("\n Not possible to join the Thread \n");
			}
			else{
				perror("Succesfully Joined Thread");
			}
		}
		if(FALSE==fclose(statesfd)){
			perror("Log Succesfully closed");
		}
		else
		{
			perror("Error Closing the File");
		}
		exit(1);
	}
}

// Timestamp Generator
/** The system is based on Linux SO, this function generates the date/time stamp to print
 * on the log file andweb server.   
	
	@param actualtime char array to savethe date/time values.
	@see CurrentState, Check_thread
**/
void timestamp(char * actualtime)
{
    time_t ltime; 										// calendar time 
    ltime=time(NULL); 									// get current cal time 
    sprintf(actualtime,asctime(localtime(&ltime)));
}

// Notification of the users and update of the Log files 
/** This function is in charge of the update of the system, it prints on the console the
 * current state of the system and also if there was a change of state between calls, it 
 * updates the logfile to be uploaded to the Firebase Realtime Database.   
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@note By default the nameof the log file is "STATES_LOG.txt"
	@see timestamp, Check_thread
**/
void CurrentState(dprimario_t *prim)
{
	char CSTATE[50];

	if((prim->previous_state!=prim->state)||(prim->previous_comm_state!=prim->comm_status)){	
		prim->previous_state=prim->state;	
		prim->previous_comm_state=prim->comm_status;
		statesfd = fopen("STATES_LOG.txt","a");
		while(statesfd == NULL)
		{
			printf("Error opening the file: STATES_LOG.txt.\n");
			usleep(ERROR_INTERVALM);
		};
	
		timestamp(CSTATE);
		fprintf(statesfd,CSTATE, sizeof(CSTATE));

		switch( prim->state )
		{
			case PRENORMAL:
				printf("\r\n CURRENT STATE: PRE-NORMAL \r\n");
				sprintf(CSTATE,"PRENORMAL\n");
				fprintf(statesfd,CSTATE, sizeof(CSTATE));
				break;
			case PREALARM:
				printf("\r\n CURRENT STATE: PRE-ALARM \r\n");
				sprintf(CSTATE,"PREALARM\n");
				fprintf(statesfd,CSTATE, sizeof(CSTATE));
				break;
			case PREFAIL:
				printf("\r\n CURRENT STATE: PRE-FAIL \r\n");
				sprintf(CSTATE,"PREFAIL\n");
				fprintf(statesfd,CSTATE, sizeof(CSTATE));
				break;
			case PRE_ALARM_FAIL:
				printf("\r\n CURRENT STATE: PRE ALARM/FAIL\r\n");
				sprintf(CSTATE,"PRE_ALARM_FAIL\n");
				fprintf(statesfd,CSTATE, sizeof(CSTATE));
				break;
			case NORMAL:
				printf("\r\n CURRENT STATE: NORMAL \r\n");
				sprintf(CSTATE,"NORMAL\n");
				printf("%s\n",CSTATE);
				fprintf(statesfd,CSTATE, sizeof(CSTATE));
				break;
			case FAIL:
				printf("\r\n CURRENT STATE: FAIL\r\n");
				sprintf(CSTATE,"FAIL\n");
				fprintf(statesfd,CSTATE, sizeof(CSTATE));
				break;
			case ALARM:
				printf("\r\n CURRENT STATE: ALARM\r\n");
				sprintf(CSTATE,"ALARM\n");
				fprintf(statesfd,CSTATE, sizeof(CSTATE));
				break;
			case ALARM_FAIL:
				printf("\r\n CURRENT STATE: ALARM/FAIL \r\n");
				sprintf(CSTATE,"ALARM_FAIL\n");
				fprintf(statesfd,CSTATE, sizeof(CSTATE));
				break;
			default:
				printf("\r\n CURRENT STATE NOT DEFINED: %d\r\n",prim->state);
				sprintf(CSTATE,"ERROR\n");
				fprintf(statesfd,CSTATE, sizeof(CSTATE));
		}
	
		if(OK==prim->comm_status)
		{
			sprintf(CSTATE,"COMM OK!\n");
			fprintf(statesfd,CSTATE, sizeof(CSTATE));	
		}
		else if(ERROR==prim->comm_status)
		{
			sprintf(CSTATE,"COMM ERROR\n");
			fprintf(statesfd,CSTATE, sizeof(CSTATE));	
		}
		else if(HOPPING==prim->comm_status)
		{
			sprintf(CSTATE,"CHANNEL HOPPING\n");
			fprintf(statesfd,CSTATE, sizeof(CSTATE));
		}
		else if(FIXING==prim->comm_status)
		{
			sprintf(CSTATE,"COMM FIXING\n");
			fprintf(statesfd,CSTATE, sizeof(CSTATE));
		}
		fclose(statesfd);
	}	  
}

// Thread 3/3 responsable to notify the user of the status of the system
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
		CurrentState(&prim);
		pthread_mutex_unlock (&mutexfile);
		pthread_mutex_unlock (&mutexconsola);
		pthread_mutex_unlock (&mutexprim);
		sleep(CHECK_INTERVAL);
	}
}

// Thread 2/3 It calls the FSM of the system to process the updated information. 
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
		primControl(&prim);
		pthread_mutex_unlock (&mutexconsola);
		pthread_mutex_unlock (&mutexprim);
		usleep(CONTROL_INTERVALU);
		//sleep(CONTROL_INTERVAL);
	}
}

// Thread 1/3 to init the whole system/
/** This thread starts the entire system, is in charge of set up the software properly to
 * work withthe resources of the Linux SO it creates the other 2 threads. After the full
 * setup it continues to be called periodically to update the state of all the inputs of the
 * system. It requires the console, access to the dprimario_t struct and also the SPI comm
 * module.   
	
	@note Is called once from the main it continues until the the 2 possible Termination
	Signals.
	
	@see Control_thread, Check_thread
**/
int Init_All(void)
{
	int error_check;
	struct sigaction SIGINT_sa;
	struct sigaction SIGTERM_sa;
	
	printf("RPI DISPOSITIVO PRIMARIO");
	primInit(&prim);
	
	//SIGTERM Signal Configuration 
	SIGTERM_sa.sa_handler = SIG_Handler;
	SIGTERM_sa.sa_flags = 0; 
	sigemptyset(&SIGTERM_sa.sa_mask);

	if ( ERRORES == sigaction(SIGTERM, &SIGTERM_sa, NULL)) {
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
		perror("Error setting the configuration for the SIGINT Handler  ");
		exit(ERROR_EXIT);
	}
	else{
		perror("* ------Configuracion de SIGINT:");
	}
	
	bloquearSign();
	
	//Creacion de Archivos .txt
	statesfd = fopen("STATES_LOG.txt", "w+");

	if (statesfd == NULL) {
		printf("Error creating opening the file: STATES_LOG.txt.\n");
		while(true){
		}
	}
	fprintf(statesfd,"Inicializando Equipo\n");
	fclose(statesfd);

	//Creacion del Thread de Update
	error_check = pthread_create (&Threads_Pointer[0], NULL,Control_thread, NULL);
	if (error_check) 
	{
		perror("Error during the creation of the Update FSM Thread");
		return -1;
	}
	else{
		perror("* --Creation of Update FSM Thread:");
	}
	
	//Creacion del "Thread State"
	error_check = pthread_create (&Threads_Pointer[1], NULL,Check_thread, NULL);
	if (error_check) 
	{
		perror("Error during the creation of the Current State Thread");
		return -1;
	}
	else{
		perror("* Creation of Current State Thread:");
	}	
	
	desbloquearSign();	
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
//	
}

// To turn on/off the LEDs according to the current state
/** This function allows the user to work with the Alarm, Fail and Normal leds in a quicker
 *  and easier way. It receives 3 arguments each one referred to the desired state for the
 *  led state. 0-> OFF and 1-> ON
	
	@param x Alarm Led
	@param y Fail Led
	@param z Normal Led
	
	@see primControl
**/
void LEDsON(char x,char y,char z)
{
	if(x==1)					// if x,y,z==1, then the Yellow LED is turned on,
		turnOn(YELLOW_LIGHT);	// is turned off
	else if(x==0)
		turnOff(YELLOW_LIGHT);

	if(y==1)					
		turnOn(RED_LIGHT);		
	else if(y==0)
		turnOff(RED_LIGHT);

	if(z==1)					
		turnOn(GREEN_LIGHT);	
	else if(z==0)
		turnOff(GREEN_LIGHT);

}

// Transition Timeout Verification
/** There is a time  limit to receive a Comm signal, this function verifies if there was
 * a timeout event.   
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@see FullCheck, ButtonCheck, CommCheck
**/
bool Timeout_Polling(dprimario_t * prim)
{
	bool timeout=FALSE;
	timeout = !(delayRead(&prim->delay));
	return timeout;
}

// Whenever a change of state originated , it resets the transition conditions
/** There is a time limit to receive a Comm code, if a RF code was received calling
 * this function resets the timer count.     
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@see FullCheck, ButtonCheck, CommCheck
**/
static void ResetChange(dprimario_t * prim)
{
	delayInit( &prim->delay,prim->timeout);  //Reset the timeout transition
	FireComm.RF24DPReset();					 //Reset the UART Listening Process
	prim->count=0;							 //Reset the count of number of cycles
	prim->COMMFLAG=0;						 //Reset the UART flag
	
}

// To verify if we are stuck in the middle of a transition (PRE-STATE) or in a
// waiting in a principal state (ALARM, NORMAL ,FAIL).
/** There is a time limit to receive a Comm code, if the timeoput event is present 
 * this function check which is the current state and how we got there, for example:
 *    If we are on PREALARM because of a RF Code menas tha we only received 1 of the
 *    2 messages needed to make a full transition from NORMAL to ALARM.
 * 
 * In case we are stuck on a PRE_STATE this function will take us directly to FAIL     
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@see primUpdates
**/
static void PRESTUCK(dprimario_t * prim)
{		
	if(!Timeout_Polling(prim))
	{
		if((prim->state==PREALARM)||(prim->state==PREFAIL)||
			(prim->state==PRENORMAL)||(prim->state==PRE_ALARM_FAIL))
		{
			//Timeout:present and State: PRE-XXXX directly to FAIL
			if(prim->COMMFLAG){
				prim->state = FAIL;
				//ResetChange(prim);
				prim->COMMFLAG=1;
			}
			else
			{
				//Timeout transition limit reached
				if(PRENORMAL==prim->state){
					printf("\r\n No Event detected, Normal Mode On \r\n");
					prim->state=NORMAL;
				}
				else{
					printf("\r\n No Additional Events detected, Out of Alarm Mode \r\n");
					prim->state=PRENORMAL;
				}
			}
		}
		else{
			//Timeout transition limit reached
				if(prim->state==NORMAL){
					printf("\r\n Normal Mode \r\n");
				}	
				else if((!prim->Fire_event)&&(!prim->Fail_event)){
					printf("\r\n No Additional Events detected, Out of Alarm Mode \r\n");
					prim->state=PRENORMAL;
				}
		}
		ResetChange(prim);
	}
}

// Updates the actual state of the alarm and fail contacts
/** Extracts the current state of both Conatcts Alarm and Fail, an important portion of 
 * inputs of the system, these inputs alongwith RF Code and current state, will deteremine
 *  which actions should be taken, this decision is made in the the Contol_Thread.     
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@note this function works as a link with the 'antirebote.cpp' module                                                                               
	@see primUpdates
**/
void ButtonUpdates(dprimario_t * prim){
	bool state=FALSE;
	if(Timeout_Polling(prim)){
		state = get_State(&prim->boton1);		
		prim->Fire_event=state;
		state = get_State(&prim->boton2);
		prim->Fail_event=state;
	}
}

// Verify the transition conditions related to pushbuttons
/** Every state has an specific set of possible transitions available, if there is an event 
 * related to the Alarm or Fail Contacts present in the system, this function allows to 
 * traduce this situation to a specific state, for Example:
 *     If we are in NORMAL state and we receive a fire event we should move to "PRE_ALARM".
 *     If we are in FAIL state and we receive a fire event we should move to "ALARM".      
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@param eventcase enum dprim_state_t corresponding state in case of an event.
	@param casen enum dprim_state_t corresponding state in case of no event present.
	@param Mode dprim_state_t what event are we validating ALARM or FAIL  
	@note this function is only a traducer, from event to the corresponding state, it does
	not make the decission. The deccision is made on FullCheck                                                                                
	@see FullCheck, CommCheck
**/
dprim_state_t ButtonCheck(dprimario_t * prim, dprim_state_t eventcase,dprim_state_t casen,dprim_state_t Mode )
{
	dprim_state_t button_state;

	if(Timeout_Polling(prim)){		//Verify if the Timeout transition limit
		
		if(Mode==ALARM)
		{
			if(prim->Fire_event){
				button_state=eventcase;			//The New state is the Alarm related state
			}
			else
			{
				button_state=casen;
			}
		}
		else if(Mode==FAIL)
		{	
			if(prim->Fail_event){	
				button_state=eventcase;			//The New state is the Alarm related state	
			}
			else
			{
				button_state=casen;
			}
		}
	}
	else
	{
		button_state=NO_STATE;
	}
	return button_state;
}

// Verify the transition conditions related to uart codes
/** Every state has an specific set of possible transitions available, if there is an event 
 * associated to RF Codes present in the system, this function allows to traduce this 
 * code to a specific state, for Example:
 *     If we are in NORMAL state and we receive a fire code we should move to "PRE_ALARM".
 *     If we are in FAIL state and we receive a code event we should move to "ALARM".      
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@param casea enum dprim_state_t in case of an alarm code, which state should we move to.
	@param casef enum dprim_state_t in case of a fail code, which state should we move to.
	@param casen enum dprim_state_t in case of a normal code, which state should we move to.
	@param caseaf enum dprim_state_t in case of an alarm_fail code, which state should we move to.
	@note this function is only a traducer, from the code received to the corresponding 
	state, it does not make the decission. The deccision is made on FullCheck                                                                                
	@see FullCheck, CommCheck
**/
dprim_state_t CommCheck(dprimario_t * prim, dprim_state_t casea, dprim_state_t casef,dprim_state_t casen, dprim_state_t caseaf )
{
	int AUX;
	dprim_state_t comm_state=NO_STATE;	
	
	if(Timeout_Polling(prim)){		//Verify if the Timeout transition limit
		if((FireComm.RF24DPRead()==Comm_received)){  //Was an Alarm code received?
			AUX=FireComm.Get_Code();
			if(AUX==Alarm_Fail_Mode){
				comm_state=caseaf;
			}
			else if(AUX==Alarm_Mode){
				comm_state=casea;
			}	
			else if(AUX==Fail_Mode){
				comm_state=casef;
			}		
			else if(AUX==Normal_Mode){
				comm_state=casen;
			}else
			{
				comm_state=casen;
			}
			
			
			prim->COMMFLAG=1;							//Set the UART Flag interaction
		}
		else
		{
			comm_state=NO_STATE;
		}
	}
	return comm_state;	
}		

// In case there is a Fire ALarm event, it verifies if is a new one, in case it is, it will update to the corresponding state 
/** We do not have to update the system periodically, instead we could identify events and 
 * act only when a difference between inputs is present, this function is in charge of
 * detecting these changes. If there was a change made due to an RF Code it updates the
 * changes to make a valid comparison, for example:
 *     If we are in NORMAL state and a Fire event is NOT present we will generate a request
 * to move to "NORMAL" state. So "NORMAL" will be our current state, if a fire event occurs,
 *  we will have a request to move to "PRE_ALARM" and an change will be present.    
 *     If a RF codemade the transition, we must update the previous input states to make a 
 * proper comparison.    
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@param casea enum dprim_state_t corresponding state in case of an alarm.
	@param casen enum dprim_state_t corresponding state in case of no alarm present.
	@note this function is only a traducer, from the code received to the corresponding 
	state, it does not make the decission. The deccision is made on FullCheck                                                                                
	@see FullCheck, CommCheck
**/
static Contact_state_t Local_Fire_Event(dprimario_t * prim,dprim_state_t casea,dprim_state_t casen){
	Contact_state_t rtn=NO_EVENT;
	dprim_state_t state=NO_STATE;
	state = ButtonCheck(prim,casea,casen,ALARM);
	
	if(prim->Comm_Transition)
	{
		if(prim->Alarm_Transition)
		{
				prim->AlarmContact_state=casea;
		}else
		{
			prim->AlarmContact_state=casen;
		}
	}
	
	if(state!=NO_STATE)
	{
		if((state!=casea)&&(state!=casen))
		{
			printf("\r\n Unexpected Alarm Signal \r\n");
			rtn=EVENT_ERROR;
		}
		else
		{		
			if(state!=prim->AlarmContact_state)
			{
				prim->AlarmContact_state=state;
				rtn=EVENT;
			}
		}
	}
	return rtn;
	
}

// In case there is a Fail event, it verifies if is a new one, in case it is, it will update to the corresponding state 
/** We do not have to update the system periodically, instead we could identify events and 
 * act only when a difference between inputs is present, this function is in charge of
 * detecting these changes. If there was a change made due to an RF Code it updates the
 * changes to make a valid comparison, for example:
 *     If we are in NORMAL state and a Fire event is NOT present we will generate a request
 * to move to "NORMAL" state. So "NORMAL" will be our current state, if a fail event occurs,
 *  we will have a request to move to "PRE_FAIL" and an change will be present.    
 *     If a RF codemade the transition, we must update the previous input states to make a 
 * proper comparison.    
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@param casef enum dprim_state_t corresponding state in case of a fail state.
	@param casen enum dprim_state_t corresponding state in case of no failures are present.
	@note this function is only a traducer, from the code received to the corresponding 
	state, it does not make the decission. The deccision is made on FullCheck                                                                                
	@see FullCheck, CommCheck
**/
static Contact_state_t Local_Fail_Event(dprimario_t * prim,dprim_state_t casef,dprim_state_t casen){
	Contact_state_t rtn=NO_EVENT;
	dprim_state_t state=NO_STATE;
	
	if(prim->Comm_Transition)
	{
		if(prim->Fail_Transition)
		{
				prim->FailContact_state=casef;
		}else
		{
			prim->FailContact_state=casen;
		}
	}

	
	state = ButtonCheck(prim,casef,casen,FAIL);
	if(state!=NO_STATE)
	{
		if((state!=casef)&&(state!=casen)){
			printf("\r\n Unexpected Fail Signal \r\n");
			rtn=EVENT_ERROR;
		}
		else
		{
			if(state!=prim->FailContact_state)
			{
				prim->FailContact_state=state;
				rtn=EVENT;
			}	
		}
	}
	return rtn;
}


//IF THERE'S A CHANGE ON THE CONTACTS IT IGNORES THE COMM and change the state, then when the code is received the system updates. 


// The full transition logic between states ishere. It Verifies the RF Codes, the Alarm and Fail Contact states to determine
//the correct flow of actions
/** The system has different inputs, the current state, the Alarm and Fail Contacts and
 *  the RF Codes, once allthe inputs were converted to a request for a specific state,
 *  this function evaluates all the request and based on the priority of the event, it 
 *  will make a decision.    
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@param casea enum dprim_state_t corresponding state in case of an alarm.
	@param casef enum dprim_state_t corresponding state in case of no alarm present.
	@param casen enum dprim_state_t corresponding state in case of an alarm.
	@param caseaf enum dprim_state_t corresponding state in case of no alarm present.
	
	@note this function determines from the options of states given and all the inputs
	which isthe next state, but is not in charge of generate the correct sequence of 
	states. 
	@see primControl
**/
static void FullCheck(dprimario_t * prim,dprim_state_t casea, dprim_state_t casef,dprim_state_t casen,dprim_state_t caseaf)
{
	dprim_state_t LocalA=NO_STATE;
	dprim_state_t LocalF=NO_STATE;
	dprim_state_t Comm=NO_STATE;
	bool Event=0; 
	bool Comm_Event=0;
	bool error_detected=NO_ERROR;
	Contact_state_t Failure_event= Local_Fail_Event(prim,casef, casen);
	Contact_state_t Alarm_event=Local_Fire_Event(prim,casea,casen);
	
	switch(Alarm_event){
		case NO_EVENT:
			LocalA=prim->AlarmContact_state;
			break;
		case EVENT:
			printf("\r\n Local Alarm Signal \r\n");
			LocalA=prim->AlarmContact_state;
			Event=1;
			break;
		case EVENT_ERROR:
			printf("\r\n Unexpected Alarm Signal \r\n");
			error_detected=ERROR_DETECTED;
			break;
		default:
			printf("\r\n Local Alarm Error \r\n");
			break;
	}
	
	switch(Failure_event){
		case NO_EVENT:
			LocalF=prim->FailContact_state;
			break;
		case EVENT:
			printf("\r\n Local Fail Signal \r\n");
			LocalF=prim->FailContact_state;
			Event=1;
			break;
		case EVENT_ERROR:
			printf("\r\n Unexpected Fail Signal \r\n");
			error_detected=ERROR_DETECTED;
			break;
		default:
			printf("\r\n Local Fail Error \r\n");
			break;
	}
	if(prim->comm_status==OK)
	{
		Comm=CommCheck(prim,casea,casef,casen,caseaf);
		if(Comm!=NO_STATE)
		{
			if(Comm!=prim->Comm_Alarm_state){
				prim->Comm_Alarm_state=Comm;
				Event=1;
				Comm_Event=1;
				
				switch(Comm)
				{
					case ALARM:
						printf("\r\n Alarm Comm Signal \r\n");
						break;
						case PREALARM:
						printf("\r\n Alarm Comm Signal \r\n");
						break;
					case FAIL:
						printf("\r\n Fail Comm Signal \r\n");
						break;
					case PREFAIL:
						printf("\r\n Fail Comm Signal \r\n");
						break;
					case NORMAL:
						printf("\r\n Normal Comm Signal \r\n");
						break;
					case PRENORMAL:
						printf("\r\n Normal Comm Signal \r\n");
						break;
					case ALARM_FAIL:
						printf("\r\n Alarm_Fail Comm Signal \r\n");
						break;
					case PRE_ALARM_FAIL:
						printf("\r\n Alarm_Fail Comm Signal \r\n");
						break;		
					default:
						printf("\r\n Unexpected Comm Signal \r\n");
						error_detected=ERROR_DETECTED;
						break;
				}
			}
		}
	}else
	{
		Comm=casen;
		if(Comm!=prim->Comm_Alarm_state){
			prim->Comm_Alarm_state=Comm;
			Event=1;
		}
	}
	
	//Manejo del Evento
	if(Event)
	{
		if(error_detected==ERROR_DETECTED)
		{
			printf("\r\n ====== Unexpected Status ====== \r\n");
			printf(" Error Description \r\n");
			printf(" State:%d\r\n  LocalA:%d\r\n LocalF:%d\r\n  Comm:%d\r\n",prim->state,LocalA,LocalF,Comm);
			prim->state=FAIL;
		}
		else if(((LocalA==casea)&&(LocalF==casef)) || (Comm==caseaf) || ((LocalA==casea)&&(Comm==casef)) || ((LocalF==casef)&&(Comm==casea)) )
		{
			prim->state=caseaf;
			printf("\r\n ====== Alarm-Fail Status ====== \r\n");
		}
		else if((LocalA==casea) || (Comm==casea))
		{
			prim->state=casea;
			printf("\r\n ====== Alarm Status ====== \r\n");
		}
		else if((LocalF==casef) || (Comm==casef))
		{
			prim->state=casef;
			printf("\r\n ====== Fail Status ====== \r\n");
		}
		else if((((LocalA==casen)||(LocalA==NO_STATE))&&
			((LocalF==casen)||(LocalF==NO_STATE)))||(Comm==casen))
		{
			prim->state=casen;
			printf("\r\n ====== Normal Status ====== \r\n");
		}
		else
		{
			printf("\r\n ====== Unexpected Status ====== \r\n");
			printf(" Error Description \r\n");
			printf(" State:%d\r\n  LocalA:%d\r\n LocalF:%d\r\n  Comm:%d\r\n",prim->state,LocalA,LocalF,Comm);
			prim->state=FAIL;
		}
		
		if (Comm_Event)
		{
			ResetChange(prim);
			prim->Comm_Transition=1;
			if(prim->Fire_event){
				prim->Alarm_Transition=1;
			}else
			{
				prim->Alarm_Transition=0;
			}
			if(prim->Fail_event){
				prim->Fail_Transition=1;
			}else
			{
				prim->Fail_Transition=0;
			}
		}
		else
		{
			prim->Comm_Transition=0;
		}
	}

}	

//update the MEFSs,
/** This functions updates all the inputs of the system, is called periodically by the
 * Init_All thread (Main Thread). It must be called frequently to make sure to
 * capture the events.    
	
	@param prim struct dprimario_t containing the entire variables needed fot the system.
	@note By defaut the system the RF behavior acts as a buffer, which means that if there
	are RF Codesunprocessed, the will stack until there is none. If the update time is not
	selected properly there will be a dead time between transitions in the seondary device
	and the received code. 
	
	@see Init_All
**/
void primUpdates(dprimario_t * pPrimario)
{
	PRESTUCK(pPrimario);	
    fsmUpdate(&pPrimario->boton1);					//Update of all the MEFSs involved
	fsmUpdate(&pPrimario->boton2);
	pPrimario->comm_status=FireComm.Comm_Status();	
	FireComm.RF24DPUpdate(network);
	ButtonUpdates(pPrimario);
	
}

// It sets initial conditions for the entire program
/** This functions sets the initial conditions of the system, configures the GPIO,
 *  Inits all the different FSM involved.   
	
	@param prim struct dprimario_t containing the entire variables needed fot the system. 
	@see Init_All
**/
bool primInit(dprimario_t * pPrimario)
{

	if(NULL==pPrimario)
		return 0;

	pPrimario->state=INITIAL_DEFAULT_STATE;
	pPrimario->previous_state=INITIAL_DEFAULT_STATE;
	pPrimario->AlarmContact_state=INITIAL_DEFAULT_STATE;
	pPrimario->FailContact_state=INITIAL_DEFAULT_STATE;
	pPrimario->Fire_event = 0;
	pPrimario->Fail_event = 0;
	pPrimario->Alarm_Transition = 0;
	pPrimario->Fail_Transition = 0;
	pPrimario->Comm_Transition=0;
	pPrimario->previous_comm_state=INITIAL_COMM_DEFAULT_STATE;
	pPrimario->timeout= DEF_TIMEOUT;
	delayInit( &pPrimario->delay,pPrimario->timeout);
	gpioSet();
	LEDsON(0,0,0);
	fsmInit(&pPrimario->boton1,ALARM_BUTTON);	//Initialize buttons with "antirebote" protocol
	fsmInit(&pPrimario->boton2,FAIL_BUTTON);
	FireComm.Init(); //Initialize uart1 through UART_USB
	FireComm.Set_Up(); //Initialize uart1 through UART_USB
	radio.printDetails();
	return 1;
}

// The MEFS logic, execute the actions related to the state
// and verifies if there is any possible transition.
/** This function sets the correct sequence of states, is the one that determines which 
 * led turn on and which state should go in the presence of any event.   
	
	@param prim struct dprimario_t containing the entire variables needed fot the system. 
	@note This FSM only stablihes which are the possible state to move to, but the decision
	to move to one option is made by FullCheck 
	@see Init_All, FullCheck
**/
bool primControl(dprimario_t * pPrimario)
{

	if(NULL==pPrimario)
		return 0;

	switch( pPrimario->state ) {

		case NORMAL:
			LEDsON(0,0,1);
			FullCheck (pPrimario,PREALARM,PREFAIL,NORMAL,PRE_ALARM_FAIL);
			break;
		case ALARM:
			LEDsON(0,1,0);
			FullCheck (pPrimario,ALARM,PREFAIL,PRENORMAL,PRE_ALARM_FAIL);
			break;
		case FAIL:
			LEDsON(1,0,0);
			FullCheck (pPrimario,PREALARM,FAIL,PRENORMAL,PRE_ALARM_FAIL);
			break;
		case ALARM_FAIL:
			LEDsON(1,1,0);
			FullCheck (pPrimario,ALARM,FAIL,PRENORMAL,ALARM_FAIL);
			break;
		case PRENORMAL:
			FullCheck (pPrimario,PREALARM,PREFAIL,NORMAL,PRE_ALARM_FAIL);
			break;
		case PREALARM:
			FullCheck (pPrimario,ALARM,PREFAIL,PRENORMAL,ALARM);	//RESPONDO PRIMERO ANTE LA ALARMA 
			break;
		case PREFAIL:
			FullCheck (pPrimario,PREALARM,FAIL,PRENORMAL,PRE_ALARM_FAIL);
			break;
		case PRE_ALARM_FAIL:
			FullCheck (pPrimario,ALARM,FAIL,PRENORMAL,ALARM_FAIL);
			break;
		default:
			pPrimario->state=FAIL;
		}
	return 1;
}
