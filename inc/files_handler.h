#ifndef FILES_HANDLER_H
#define FILES_HANDLER_H

#include <stdlib.h> /* for EXIT_SUCCESS */

#define N_RECORD_EVENTS 4000
#define ERROR_INTERVALM 150000
#define FILE_NAME "STATES_LOG.txt"

/** The system is based on Linux SO, this function generates the date/time stamp to print
 * on the log file andweb server.   
	
	@param actualtime char array to savethe date/time values.
	@see CurrentState, Check_thread
**/
void timestamp(char * actualtime);

/** There are several tasks related to the Maintenance of the system, some of then involves
 * delete certain files such as "dhcplist.txt" and "STATES_LOG.txt". 
	
	@param file_name const char * name of the file to be cleared.
	@see RF_Maintenance_thread
**/
void File_Clean(const char * file_name);

void Init_File(void);

bool Close_File(void);

void File_Update(u_int16_t* Line_Counter,char* STATE, char* RF, char* NODES);

#endif 

