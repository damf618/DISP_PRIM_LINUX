#include <RFinterface.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include "Cuaima_Testing.h"

RF24 radio (22,0);
RF24Network network(radio);
RF24DP FireComm(radio,network);


void RF_Comm_Init(void){
	FireComm.Init(); 
	FireComm.Set_Up(); 
	radio.printDetails();
}	
 
void RF_Comm_Reset(void){
	FireComm.RF24DPReset();
}	

void RF_Comm_Update(void){
	FireComm.RF24DPUpdate(network);
}	

char RF_Comm_Read(void){
	char rtn;
	rtn = FireComm.RF24DPRead();
	return rtn;
}
 
int RF_Comm_Code(void){
	int rtn;
	rtn = FireComm.Get_Code();
	return rtn;
}

int RF_Comm_Nodes(void){
	int rtn;
	rtn = FireComm.Get_Nodes();
	return rtn;
}

int RF_Comm_Status(void){
	int rtn;
	rtn = FireComm.Comm_Status();
	return rtn;
}

void RF_Comm_Nodes_Update(int nnodes){
	FireComm.NnodesUpdate(nnodes);
}
 
void RF_Comm_Clean(void){
	FireComm.Clean_RFDevices();
}

void RF_Comm_Maintenance(void){
	FireComm.Maintenance_clean();
}

Nodes_Database_t RF_Comm_Nodes_Database(void){
	Nodes_Database_t rtn;
	rtn = FireComm.Nodes_Database();
	return rtn;
}
