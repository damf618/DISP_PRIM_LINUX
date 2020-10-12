#include "File_Interface.h"
#include "files_handler.h"
#include "database.h"
#include <stdlib.h> 
#include <cstdio>
#include <unistd.h>

#include "time.h"  
#include "database.h" 

//static void timestamp(char * actualtime)
void timestamp(char * actualtime)
{
    time_t ltime; 										// calendar time 
    ltime=time(NULL); 									// get current cal time 
    sprintf(actualtime,asctime(localtime(&ltime)));
}


//Init File Interface
void Init_File_Interface(void){
	Init_File();
	Init_database();
}

//Clean File Interface
void Clean_File_Interface(const char * file_name){
	File_Clean(file_name);
}
 
void Update_File_Interface(u_int16_t* Line_Counter,char* STATE, char* RF, char* NODES){
	File_Update(Line_Counter,STATE,RF,NODES);
	database_update(Line_Counter,STATE,RF,NODES);
	
}
 
