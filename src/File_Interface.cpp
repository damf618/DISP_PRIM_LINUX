#include "File_Interface.h"
#include "files_handler.h"
#include "database.h"
#include <stdlib.h> 
#include <cstdio>
#include <unistd.h>

#include "time.h"  
#include "database.h"
#include "RF24DP.h" 
#include "Cuaima_Testing.h" 

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

void RF_Update_File_Interface(Nodes_Database_t Nodes_Info){
    int i=0;
    printf("\n----- RF DATABASE INFO ----- \n");
    Init_RF_database();
    for(i=0;i<=Nodes_Info.Counter;i++){
	if(Nodes_Info.Nodes_Data[i].Node_ID!=0){
	    printf("RF Device ID: %d\n",Nodes_Info.Nodes_Data[i].Node_ID);
	    printf("RF Device Code: %d\n",Nodes_Info.Nodes_Data[i].RF_Code);
	    printf("RF Device Status: %d\n",Nodes_Info.Nodes_Data[i].updated);
	    RF_database_update(Nodes_Info.Nodes_Data[i].Node_ID,
	    Nodes_Info.Nodes_Data[i].RF_Code,Nodes_Info.Nodes_Data[i].updated);
	}
    }
    printf("\n---------------------------- \n");
    
}
 
