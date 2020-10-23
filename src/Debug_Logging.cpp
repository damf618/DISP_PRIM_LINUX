#include "Debug_Logging.h"
#include <syslog.h> 

// Initialization of the the Debug Message Interface
void Init_Debug(void)
{
	setlogmask (LOG_UPTO (DEBUG_LEVEL));
}

// Function to generate a Notice Level message
void Debug_Message_Notice(const char * text)
{
	openlog (DEBUG_FILENAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog (LOG_NOTICE, text);
	closelog ();
}

// Function to generate a Info Level message
void Debug_Message_Info(const char * text)
{
	openlog (DEBUG_FILENAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog (LOG_INFO, text);
	closelog ();
}

// Function to generate a Warning Level message
void Debug_Message_Warning(const char * text)
{
	openlog (DEBUG_FILENAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog (LOG_WARNING, text);
	closelog ();
}

// Function to generate a Error Level message
void Debug_Message_Error(const char * text)
{
	openlog (DEBUG_FILENAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog (LOG_ERR, text);
	closelog ();
}

// Function to generate a Debug Level message
void Debug_Message_Debug(const char * text)
{
	openlog (DEBUG_FILENAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog (LOG_DEBUG, text);
	closelog ();
}
