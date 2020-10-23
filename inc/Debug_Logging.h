/*=============================================================================
 * Copyright (c) 2020, DANIEL MARQUEZ <DAMF618@GMAIL.COM>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2020/04/12
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __DEBUG_LOGGING__
#define __DEBUG_LOGGING__

#include <syslog.h> 

#define DEBUG_LEVEL LOG_NOTICE
#define DEBUG_FILENAME "Fire_Alarm_Monitor"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void Init_Debug(void);

void Debug_Message_Notice(const char * text);

void Debug_Message_Info(const char * text);

void Debug_Message_Warning(const char * text);

void Debug_Message_Error(const char * text);

void Debug_Message_Debug(const char * text);

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __DEBUG_LOGGING__ */
