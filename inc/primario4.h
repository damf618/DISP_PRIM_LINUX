/*=============================================================================
 * Copyright (c) 2020, DANIEL MARQUEZ <DAMF618@GMAIL.COM>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2020/04/12
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __PRIMARIO_H__
#define __PRIMARIO_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>
#include "cstdio"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <cstdbool>

#include "delay.h"
#include "antirebote.h"
#include "Primario_LEDS.h"


/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/
#define INITIAL_DEFAULT_STATE NORMAL
#define INITIAL_COMM_DEFAULT_STATE OK

#define DEF_TIMEOUT 3500			   //Timeout limit between transitionsions

#define NO_ERROR 0			   //Timeout limit between transitionsions
#define ERROR_DETECTED 1			   //Timeout limit between transitionsions

//#define DEBUG

#define TRUE 1
#define FALSE 0
#define PROGRAM_THREADS 3
#define MAX_THREADS 3
#define ERRORES 1
#define ERROR_EXIT 1
#define CHECK_INTERVAL 1
#define UPDATE_INTERVALU 10000
#define CONTROL_INTERVALU 200000 //200000   MODIFICAR EL ALGORITMO PARA QUE NO SEA ALGOPUNTUALSINO ALGO ACUMULATIVODE ESTADOS! DURANTE MAS TIEMPO EXISTENDEMASIADOS COMM8 Y ALOMENTO DE RECIBIR UNO DISTINTO ESTAMOS CAMBIANDO TODO!
#define ERROR_INTERVALM 150000
//#define N_NODES 2
#define MAX_INCOMPLETE_ERRORS 5
#define N_RECORD_EVENTS 3000

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

// Possible states for the MEFs
typedef enum{ NORMAL , ALARM, FAIL,ALARM_FAIL, PRENORMAL, PREALARM, PREFAIL,PRE_ALARM_FAIL, NO_STATE} dprim_state_t;

typedef enum{ NO_EVENT, EVENT, EVENT_ERROR } Contact_state_t;

typedef enum{ OK, HOPPING, FIXING, ERROR} comm_state_t;

// Structure with the different data types to generate an independent Monitor
typedef struct{
	tick_t timeout;
	delay_t delay;
	dprim_state_t state;
	antirebote_t boton1;	//Buttons with  debounce pattern
	antirebote_t boton2;
	uint8_t count;			//Count of Cycles.
	bool COMMFLAG;		//Flag for UART interaction 1 ON, 0 OFF
	bool TEST_MODE;		//Flag for defining TEST mode
	dprim_state_t AlarmContact_state;
	dprim_state_t FailContact_state;
	dprim_state_t Comm_Alarm_state;
	int comm_status;
	dprim_state_t previous_state;
	int previous_comm_state;
	bool Fire_event;
	bool Fail_event;
	bool Alarm_Transition;
	bool Fail_Transition;
	bool Comm_Transition;
	bool Maintenance_Timer;
	int previous_active_nodes;
	int active_nodes;
	bool Incomplete_flag;
	int Incomplete_counter;
	uint8_t Line_Counter;
	int min_node;

}dprimario_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
/** Init and configuartion of the whole system.

**/
int Init_All(void);


/** It sets initial conditions for the entire program, it establish the default values
 *  of timeout and #cycles for Alarm and Fail UART code verification.

	@param pPrimario element of type *dprimario_t* with the Monitor data types needed
	@note It must be called before primControl.
	@see primControl.

**/
bool primInit(dprimario_t * pPrimario);

/** It identifies the state we are currently and generates the corresponding
 *  outputs verifying if for the current state exists any possible transition.

	@param pPrimario element of type *dprimario_t* with the Monitor data types needed
	@note If the current state is not define it returns FALSE.
	@see primInit.

**/
bool primControl(dprimario_t * pPrimario);

/** It updates the state of all the existent MEFs and also checks the TESTMODE.
	@param pPrimario element of type *dprimario_t* with the Monitor data types needed
	@note Must be called after primInit.
	@see primInit.

**/
void primUpdates(dprimario_t * pPrimario);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __PRIMARIO_H__ */
