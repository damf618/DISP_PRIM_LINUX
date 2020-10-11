#include "File_Interface.h"
#include "files_handler.h"
#include <stdlib.h> 

//Init File Interface
void Init_File_Interface(void){
	Init_File();
}

//Clean File Interface
void Clean_File_Interface(const char * file_name){
	File_Clean(file_name);
}
 
void Update_File_Interface(u_int16_t* Line_Counter,char* STATE, char* RF, char* NODES){
	File_Update(Line_Counter,STATE,RF,NODES);
}
 
