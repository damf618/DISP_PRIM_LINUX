/*=============================================================================
 * Copyright (c) 2020, DANIEL MARQUEZ <DAMF618@GMAIL.COM>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2020/04/12
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __FILE_INTERFACE_H__
#define __FILE_INTERFACE_H__

#include "Cuaima_Testing.h"
#include <stdlib.h> 

#define N_RECORD_EVENTS 4000

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

// Timestamp Generator
/** The system is based on Linux SO, this function generates the date/time stamp to print
 * on the log file andweb server.   
	
	@param actualtime char array to savethe date/time values.
	@see CurrentState, Check_thread
**/
void timestamp(char * actualtime);

//Init File Interface
void Init_File_Interface(void); 

//Clean File Interface
void Clean_File_Interface(const char * file_name);

//Update File Interface
void Update_File_Interface(u_int16_t* Line_Counter,char* STATE, char* RF, char* NODES);

void RF_Update_File_Interface(Nodes_Database_t Nodes_Data);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __FILE_INTERFACE_H__ */
