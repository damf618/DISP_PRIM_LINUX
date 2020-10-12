#include <File_Interface.h>
#include <files_handler.h>
#include "time.h"  
#include <cstdio>
#include <unistd.h>

void Init_File(void){
	
	FILE * statesfd;
	
	do{	
		statesfd = fopen(FILE_NAME,"w+");
		if(statesfd == NULL)
		{
			usleep(ERROR_INTERVALM);
		}
	}while(statesfd == NULL);
	
	fprintf(statesfd,"Inicializando Equipo\n");
	
	fclose(statesfd);
	
}

// This task allow us to delete the content of the given file.
void File_Clean(const char * file_name)
{
	FILE * aux;
	aux = fopen(file_name, "w+");

	if (aux == NULL)
	{
		printf("Error cleaning the file: %s\n",file_name);
	}
	if(0==fclose(aux))
	{
		printf("Succesfull Clean of file: %s\n",file_name);
	}
		else
		{
			printf("Error cleaning the file: %s\n",file_name);
		}
}

//
void File_Update(u_int16_t* Line_Counter,char* STATE, char* RF, char* NODES)
{
	FILE * statesfd;
	bool fileflag = 0;
	int tries=0;
	bool rtn=0;
	char TIME[50];
	do{	
		if(Line_Counter[0]>=N_RECORD_EVENTS)
		{
			statesfd = fopen(FILE_NAME,"w+");
			Line_Counter[0]=0;
			rtn = 1;
		}else{
			statesfd = fopen(FILE_NAME,"a");
			rtn = 1;
		}
		if((statesfd == NULL)&&(tries<3))
		{
			usleep(ERROR_INTERVALM);
			fileflag = 1;
			tries++;
		}
		else if((statesfd == NULL)&&(tries>=3))
		{
			fileflag = 0;
		}
	}while(fileflag);
	
	if(rtn)
	{
		timestamp(TIME);
		fprintf(statesfd,TIME, sizeof(TIME));
		fprintf(statesfd,STATE, sizeof(STATE));
		fprintf(statesfd,RF, sizeof(RF));
		fprintf(statesfd,NODES, sizeof(NODES));
		fclose(statesfd);
	}
}
