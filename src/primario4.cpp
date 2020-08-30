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

/*pthread_mutex_lock (&mutexprim);
  pthread_mutex_unlock (&mutexconsola);
*/
sem_t Update_sem; 
sem_t RF_Maintenance_sem; 
/*sem_wait(&sync_sem);
  sem_post(&sync_sem);  
*/


FILE * statesfd;

// Block of any of the unexpected signals
void bloquearSign(void)
{
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

// Unblock thee SIGINT and SIGTERM signals
void desbloquearSign(void)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

// Handler for the Closing Signal
void SIG_Handler(int sig)
{
	sig_flag=TRUE;									       //Correcion 5
	printf("\n Finalizando\n");	
}

// Organized Exit of the system
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

void timestamp(char * actualtime)
{
    time_t ltime; 										/* calendar time */
    ltime=time(NULL); 									/* get current cal time */
    sprintf(actualtime,asctime(localtime(&ltime)));
}

void CurrentState(dprimario_t *prim)
{
	char CSTATE[50];

	if((prim->previous_state!=prim->state)||(prim->previous_comm_state!=prim->comm_status)){	
		prim->previous_state=prim->state;	
		prim->previous_comm_state=prim->comm_status;
		statesfd = fopen("STATES_LOG.txt","a");
		do
		{
			printf("Error opening the file: STATES_LOG.txt.\n");
			usleep(ERROR_INTERVALM);
		}while(statesfd == NULL);
	
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

// Thread to notify the user of the status of the system
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

// Thread to update the whole system
void* Update_thread (void*parmthread)
{
	while(1){
		
		sem_wait(&Update_sem);
		pthread_mutex_lock (&mutexprim);
		pthread_mutex_lock (&mutexconsola);
		primControl(&prim);
		pthread_mutex_unlock (&mutexconsola);
		pthread_mutex_unlock (&mutexprim);
		usleep(CONTROL_INTERVALU);
	}
}

// Task to init the whole system
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
	error_check = pthread_create (&Threads_Pointer[0], NULL,Update_thread, NULL);
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

// To turn on or off the LEDs according to the current state
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
bool Timeout_Polling(dprimario_t * prim)
{
	bool timeout=FALSE;
	timeout = !(delayRead(&prim->delay));
	return timeout;
}

// Whenever a change of state is made, it resets the transition conditions
static void ResetChange(dprimario_t * prim)
{
	delayInit( &prim->delay,prim->timeout);  //Reset the timeout transition
	FireComm.RF24DPReset();					 //Reset the UART Listening Process
	prim->count=0;							 //Reset the count of number of cycles
	prim->COMMFLAG=0;						 //Reset the UART flag
	
}

// To verify if we are stuck in the middle of a transition (PRE-STATE) or in a
// waiting in a principal state (ALARM, NORMAL ,FAIL).
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
					//ResetChange(prim);
				}
				else{
					printf("\r\n No Additional Events detected, Out of Alarm Mode \r\n");
					prim->state=PRENORMAL;
					//ResetChange(prim);
				}
			}
		}
		else{
			//Timeout transition limit reached
				if(NORMAL!=prim->state){
					printf("\r\n No Additional Events detected, Out of Alarm Mode \r\n");
					prim->state=PRENORMAL;
					//ResetChange(prim);
				}
		}
		ResetChange(prim);
	}
}

// Verify the transition conditions related to pushbuttons
dprim_state_t ButtonCheck(dprimario_t * prim, dprim_state_t eventcase,dprim_state_t casen,dprim_state_t Mode )
{
	bool state=FALSE;
	dprim_state_t button_state;

	if(Timeout_Polling(prim)){		//Verify if the Timeout transition limit
		if(Mode==ALARM)
		{
			state = get_State(&prim->boton1);	
			if(state)
			{	//Button pressed?
				button_state=eventcase;			//The New state is the Alarm related state
			}
			else
			{
				button_state=NO_STATE;
			}
		}
		else if(Mode==FAIL)
		{	
			state = get_State(&prim->boton2);	
			if(state)
			{	//Button pressed?
				button_state=eventcase;			//The New state is the Alarm related state
			}
			else
			{
				button_state=NO_STATE;
			}
		}
	}
	else
	{
		button_state=casen;
	}
	return button_state;
}

// Verify the transition conditions related to uart codes
dprim_state_t CommCheck(dprimario_t * prim, dprim_state_t casea, dprim_state_t casef,dprim_state_t casen, dprim_state_t caseaf )
{
	int AUX;
	dprim_state_t comm_state;	
	
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

// Verify the 3 Uart Codes and the button state transitions
static void FullCheck(dprimario_t * prim,dprim_state_t casea, dprim_state_t casef,dprim_state_t casen,dprim_state_t caseaf)
{
	dprim_state_t LocalA=NO_STATE;
	dprim_state_t LocalF=NO_STATE;
	dprim_state_t Comm=NO_STATE;
	dprim_state_t actual=prim->state;
	bool Event=0; 
	bool error_detected=NO_ERROR;
	
	LocalA = ButtonCheck(prim,casea,casen,ALARM);
	if(LocalA!=NO_STATE)
	{
		if((LocalA!=casea)&&(LocalA!=casen)){
			printf("\r\n Unexpected Alarm Signal \r\n");
			error_detected=ERROR_DETECTED;
		}
		else{
			printf("\r\n Local Alarm Signal \r\n");
			Event=1;
		}
	}
	LocalF = ButtonCheck(prim,casef,casen,FAIL);
	if(LocalF!=NO_STATE)
	{
		if((LocalF!=casef)&&(LocalF!=casen)){
			printf("\r\n Unexpected Fail Signal \r\n");
			error_detected=ERROR_DETECTED;
		}
		else{
			printf("\r\n Local Fail Signal \r\n");
			Event=1;
		}
	}
	Comm=CommCheck(prim,casea,casef,casen,caseaf);
	if(Comm!=NO_STATE)
	{
		Event=1;
		switch(Comm){
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
		default:
			printf("\r\n Unexpected Comm Signal \r\n");
			break;
		}
	}
	if(Event){
		if(error_detected==ERROR_DETECTED){
			printf("\r\n ====== Unexpected Status ====== \r\n");
			printf(" Error Description \r\n");
			printf(" State:%d\r\n  LocalA:%d\r\n LocalF:%d\r\n  Comm:%d\r\n",prim->state,LocalA,LocalF,Comm);
			prim->state=FAIL;
			//ResetChange(prim);
		}
		else if(((LocalA==casea)&&(LocalF==casef)) || (Comm==caseaf) || ((LocalA==casea)&&(Comm==casef)) || ((LocalF==casef)&&(Comm==casea)) )
		{
			prim->state=caseaf;
			printf("\r\n ====== Alarm-Fail Status ====== \r\n");
			//ResetChange(prim);
		}
		else if((LocalA==casea) || (Comm==casea))
		{
			prim->state=casea;
			printf("\r\n ====== Alarm Status ====== \r\n");
			//ResetChange(prim);
		}
		else if((LocalF==casef) || (Comm==casef))
		{
			prim->state=casef;
			printf("\r\n ====== Fail Status ====== \r\n");
			//ResetChange(prim);
		}
		else if((((LocalA==casen)||(LocalA==NO_STATE))&&
			((LocalF==casen)||(LocalF==NO_STATE)))||(Comm==casen))
		{
			prim->state=casen;
			printf("\r\n ====== Normal Status ====== \r\n");
			//ResetChange(prim);
		}
		else
		{
			printf("\r\n ====== Unexpected Status ====== \r\n");
			printf(" Error Description \r\n");
			printf(" State:%d\r\n  LocalA:%d\r\n LocalF:%d\r\n  Comm:%d\r\n",prim->state,LocalA,LocalF,Comm);
			prim->state=FAIL;
			//ResetChange(prim);
		}
		if(actual==prim->state){
			ResetChange(prim);
		}
		else if((PREALARM==prim->state)||(PREFAIL==prim->state)||(PRENORMAL==prim->state)||
			(PRE_ALARM_FAIL==prim->state)){
			ResetChange(prim);
		}
	}
	
}

//update the MEFSs,
void primUpdates(dprimario_t * pPrimario)
{
	PRESTUCK(pPrimario);	
    fsmUpdate(&pPrimario->boton1);					//Update of all the MEFSs involved
	fsmUpdate(&pPrimario->boton2);
	pPrimario->comm_status=FireComm.Comm_Status();	
	FireComm.RF24DPUpdate(network);

}

// It sets initial conditions for the entire program
bool primInit(dprimario_t * pPrimario)
{

	if(NULL==pPrimario)
		return 0;

	pPrimario->state=INITIAL_DEFAULT_STATE;
	pPrimario->previous_state=INITIAL_DEFAULT_STATE;
	pPrimario->AlarmContact_state=INITIAL_DEFAULT_STATE;
	pPrimario->FailContact_state=INITIAL_DEFAULT_STATE;
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
bool primControl(dprimario_t * pPrimario)
{

	if(NULL==pPrimario)
		return 0;

	switch( pPrimario->state ) {

		case NORMAL:
			LEDsON(0,0,1);
			FullCheck (pPrimario,PREALARM,PREFAIL,NORMAL,ALARM_PREFAIL);
			break;
		case ALARM:
			LEDsON(0,1,0);
			FullCheck (pPrimario,ALARM,PREFAIL,PRENORMAL,ALARM_PREFAIL);
			break;
		case FAIL:
			LEDsON(1,0,0);
			FullCheck (pPrimario,PREALARM,FAIL,PRENORMAL,ALARM_PREFAIL);
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
			FullCheck (pPrimario,PREALARM,FAIL,PRENORMAL,ALARM_PREFAIL);
			break;
		case ALARM_PREFAIL:
			FullCheck (pPrimario,ALARM,FAIL,PRENORMAL,ALARM_FAIL);
			break;
		default:
			pPrimario->state=FAIL;
		}
	return 1;
}
