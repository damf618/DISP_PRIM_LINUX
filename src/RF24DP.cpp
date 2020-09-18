#include <cstdlib>						 // BIBLIOTECA DE FORMATOS STANDARD 
#include <string>		
#include <unistd.h>
#include <stdbool.h>
#include "RF24Mesh.h"					// BIBLIOTECA DE RF24 POR TMRH20
#include <RF24DP.h>
#include <delay.h>
#include <Cuaima_Testing.h>

uint8_t channels1[3] = {117,120,122};

static bool Insert(int id, int Code,RF_List_t* RF_List){
	
	RF_Device_t * Mem_Block;
	Mem_Block = (RF_Device_t *)malloc(sizeof(RF_Device_t));
	bool rtn=0;
	
	if(RF_List->counter<MAX_RF_DEVICES)
	{
		if(NULL!=Mem_Block)
		{
			Mem_Block[0].Node_ID=id;
			Mem_Block[0].RF_Code=Code;
			Mem_Block[0].updated=1;
			RF_List->RF_Devices[RF_List->counter]=Mem_Block;
			RF_List->counter++;
			rtn=1;
		}
	}
	return rtn;
}

int Header_Check(int id, RF_List_t* RF_List){
	int rtn=333;
	int i;
	RF_Device_t * Mem_Block;
	
	for(i=0;i<RF_List->counter;i++)
	{
		Mem_Block=RF_List->RF_Devices[i];
		if(id==Mem_Block[0].Node_ID)
		{
			rtn=i;
		}	
	}
	return rtn;
}

bool Header_Validation(int id, int Code, RF_List_t* RF_List){
	
	bool rtn=1;
	int aux;
	RF_Device_t * Mem_Block;
	
	
	aux = Header_Check(id,RF_List);
	if(333==aux)
	{
		rtn = Insert(id,Code,RF_List);
		if(!rtn)
		{
			rtn=0;
		}
	}else
	{
		Mem_Block=RF_List->RF_Devices[aux];
		Mem_Block[0].RF_Code=Code;
		Mem_Block[0].updated=1;
	}
	return rtn;
}

int Comm_Code(RF_List_t* RF_List){
	int Final_Code;
	bool Alarm=0;
	bool Fail=0;
	int i;
	int counter=0;
	RF_Device_t * Mem_Block;
	//printf("Number of Devices: %d\n",RF_List->counter);
	
	for(i=0;i<RF_List->counter;i++)
	{
		Mem_Block=RF_List->RF_Devices[i];
		//printf("#%d Device: %d\n",i,Mem_Block[0].Node_ID);
		if(Mem_Block[0].updated)
		{
			//printf("#%d Device: %d,State: %d\n",i,Mem_Block[0].Node_ID,Mem_Block[0].RF_Code);
			counter++;
			Mem_Block[0].updated=0;
			if(ALARM_FAIL_CODE==Mem_Block[0].RF_Code)
			{
				Alarm=1;
				Fail=1;
				
			}
		
			if(ALARM_CODE==Mem_Block[0].RF_Code)
			{
				Alarm=1;
			}
			
			if(FAIL_CODE==Mem_Block[0].RF_Code)
			{
				Fail=1;
			}
		}
	}
	RF_List->active_nodes = counter;
	//printf("active_nodes: %d\n",RF_List->active_nodes);
	
	//if(RF_List->counter!=counter)
	//if((counter<RF_List->n_nodes)&&(RF_List->counter>=RF_List->n_nodes)){
	//	RF_List->comm_incomplete=1;
	//}
	if((Alarm)&&(Fail))
	{
		Final_Code = ALARM_FAIL_CODE;
	}else if(Alarm)
	{
		Final_Code = ALARM_CODE;
	}else if(Fail)
	{
		Final_Code = FAIL_CODE;
	}else
	{
		Final_Code = NORMAL_CODE;
	}
	return Final_Code;
}

void RF24DP:: Clean_RFDevices(){
	int k;
	
	for(k=0;k<RF_List.counter;k++)
	{
		free(RF_List.RF_Devices[k]);
		RF_List.counter=0;
	}
}

/*
 * 
 * */

int Lectura_Mesh(RF24Network& netw, RF_List_t * RF_Devices ){
	int dat=EMPTY_CODE;
	bool max=0;
	if(netw.available()){
		RF24NetworkHeader header;
		netw.peek(header);
		switch(header.type){
		// Display the code received from the secundary nodes
		case 'M': netw.read(header,&dat,sizeof(dat)); 
#if defined (DEBUG)
			printf("Rcv %u from 0%o\n",dat,header.from_node);
#endif
			max = Header_Validation(header.from_node, dat, RF_Devices);
			if(!max)
			{
				//BORRAR TODO EL ARCHIVO Y DIRECCIONES
				//UTILIZAR comm_status para indicar esto!!
				printf("Error Nro Max Alcanzado\n");
				dat = RESET_REQUEST;
			}
			//dat = Comm_Code(RF_Devices);
			break;
		default:  netw.read(header,0,0); 
#if defined (DEBUG)
	        printf("Rcv bad type %d from 0%o\n",header.type,header.from_node); 
#endif	        
	        break;
		}
	}else
	{
		dat = EMPTY_CODE;
	}
	
	return dat;
}

void RF24DP:: RF24ChannelHop(void){
	
	//CUIDADO CON BLOQUEO POR TIEMPOS PROLONGADOS
	printf("New Channel Assignation: %d \n",channels1[FireComm_Channel.ChannelCounter]);
	Set_Channel(channels1[FireComm_Channel.ChannelCounter],MESH_TIME);
	FireComm_Channel.ChannelCounter++;
	if(FireComm_Channel.ChannelCounter>=3){
		fixing=1;
		FireComm_Channel.ChannelCounter=0;
	}
	if((FireComm_Channel.ChannelCounter>=3)&&(fixing)){
		error=1;
	}
	FireComm_Channel.ErrorCounter=0;
	hopping=1;
}	

void RF24DP::Maintenance_clean(void){
	error=0;
	hopping=0;
	fixing=0;
	reset_request=0;
	RF_List.counter=0;
}

int RF24DP::Comm_Status(void){
	int rtn=0;
	
	// OK      -> 0
	// HOPPING -> 1
	// FIXING  -> 2
	// ERROR_RF-> 3
	if(reset_request)
	{
		rtn=3;
	}
	else
	{
		if(hopping)
		{
			rtn=1;
		}
		if(fixing)
		{
			rtn=rtn+1;
		}
		if(error)
		{
			rtn=rtn+1;
		}
	}	
	
	return rtn;

}

void RF24DP:: RF24ErrorManage(void){
	FireComm_Channel.ErrorCounter++;
	printf("\r\n Numero de Errores Timeout: %d \r\n",FireComm_Channel.ErrorCounter);
	if(FireComm_Channel.ErrorCounter>=CHANNEL_ERROR_TOLERANCE){
		RF24ChannelHop();
	}
}

void RF24DP:: Set_Up(void) {
	setNodeID(MASTER_ID);
	begin(RF_DEFAULT_CHANNEL);
}	

void RF24DP:: Set_Channel(uint8_t Channel, uint32_t max_time) {
	begin(Channel,RF24_1MBPS,max_time);	
}	

void RF24DP:: Init(void){
	Receiver.timeout= read_timeout_default;
	Receiver.state= RECEIVE_CONFIG;
	delayInit( &Receiver.delay,Receiver.timeout);
	FireComm_Channel.ChannelCounter=0;
	FireComm_Channel.ErrorCounter=0;
	RF_List.counter=0;
	RF_List.n_nodes=N_NODES;
	RF_List.active_nodes=0;
	Active_nodes=0;
}

void RF24DP:: Init(char CE, char CSN, char Num, char CA, char CF, char SC, char Reset, unsigned int timeout){
		CE_pin = CE; 		//22;
		CSN_pin = CSN; 		//0;
		Number = Num;		//1;
		Contacto_A = CA;	//19; 
		Contacto_F = CF;	//26;
		Status_Comm = SC;	//0;
		Reset_Pin = Reset; 	//20;
		Receiver.timeout= timeout;
		counter=0;
}
	
void RF24DP::Read_Data(RF24Network& netw){
	int Status=0;
	if (delayRead(&Receiver.delay)) 
	{
		Receiver.state = RECEIVE_TIMEOUT;
	}else
	{
		Status = Lectura_Mesh(netw,&RF_List);
		if((EMPTY_CODE==Status)&&(EMPTY_CODE!=Code)){
			Receiver.state = RECEIVE_RECEIVING;
			switch(Code)
			{
				case Alarm_Fail_Mode:
					Status=ALARM_FAIL_CODE;
					break;
					
				case Alarm_Mode:
					Status=ALARM_CODE;
					break;	
				
				case Fail_Mode:
					Status= FAIL_CODE;
					break;
				
				case Normal_Mode:
					Status= NORMAL_CODE;
					break;
				
				default:
					Status=NORMAL_CODE;
					break;
					 
			}		
		}
		else
		{
			switch(Status)
			{
				case ALARM_FAIL_CODE:
					Code=Alarm_Fail_Mode;
					Receiver.state = RECEIVE_RECEIVED_OK;
					FireComm_Channel.ErrorCounter=0;
					break;
					
				case ALARM_CODE:
					Code=Alarm_Mode;
					Receiver.state = RECEIVE_RECEIVED_OK;
					FireComm_Channel.ErrorCounter=0;
					break;	
				
				case FAIL_CODE:
					Code=Fail_Mode;
					Receiver.state = RECEIVE_RECEIVED_OK;
					FireComm_Channel.ErrorCounter=0;
					break;
				
				case NORMAL_CODE:
					Code=Normal_Mode;
					Receiver.state = RECEIVE_RECEIVED_OK;
					FireComm_Channel.ErrorCounter=0;
					break;
				
				case EMPTY_CODE:
					Receiver.state = RECEIVE_RECEIVING;
					break;
					
				case RESET_REQUEST:
					reset_request = 1;
					Receiver.state = RECEIVE_RECEIVING;
					break;
				
				default:
					Receiver.state = RECEIVE_RECEIVING;
					break;
			}		
		}
	}
}
	
ReceiveOrTimeoutState_t RF24DP :: Wait_for_Code (RF24Network& netw){
		
	switch( Receiver.state ) {
			case RECEIVE_CONFIG:
				delayInit( &(Receiver.delay), Receiver.timeout );
				Receiver.state = RECEIVE_RECEIVING;		
			break;
			case RECEIVE_RECEIVING:
				Read_Data(netw);
				break;

			case RECEIVE_RECEIVED_OK:
				if(!reset_request)
				{
					fixing=0;
					hopping=0;
					error=0;
				}
				Receiver.state = RECEIVE_CONFIG;				
				break;

			case RECEIVE_TIMEOUT:
				RF24ErrorManage();
				Receiver.state = RECEIVE_CONFIG;
				break;

			default:
				Receiver.state = RECEIVE_CONFIG;
				break;
   }
   return Receiver.state;
}

void RF24DP::RF24DPReset(void){
	Receiver.state = RECEIVE_CONFIG;			// It resets the MEF stage to receive this
    delayInit(&Receiver.delay,Receiver.timeout);		// Configure the UART Listening Process delay for the timeout measuring.
}

void RF24DP::RF24DPUpdate(RF24Network& netw){
	// Call network.update as usual to keep the network updated
	update();
	// In addition, keep the 'DHCP service' running on the master node so addresses will
	// be assigned to the sensor nodes
	DHCP();
	Receiver.state = Wait_for_Code( netw ); //Updates the MEF to receive the next byte.
}

char RF24DP::RF24DPRead(void){
	if( Receiver.state == RECEIVE_TIMEOUT ){  // Asks for the timeout flag. Must be called after
		return Comm_timeout;									// the "Update" (UARTUpdate).
	}
	else if( Receiver.state == RECEIVE_RECEIVED_OK){  //Asks for the received code flag. Must be
		return Comm_received;									// called after the "Update" (UARTUpdate).
	}
	else if( Receiver.state == RECEIVE_RECEIVING){  //Asks for the received code flag. Must be
		return 	Comm_receiving;
	}
	else{
		return 	Comm_receiving;
	}
}

int RF24DP::Get_Code(void){
	int rtn;
	rtn = Comm_Code(&RF_List);
	Active_nodes=RF_List.active_nodes;	
	return rtn;
}

int RF24DP::Get_Nodes(void){
	
	return Active_nodes;
}
