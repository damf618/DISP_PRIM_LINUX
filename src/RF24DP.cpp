#include <cstdlib>						 // BIBLIOTECA DE FORMATOS STANDARD 
#include <string>		
#include <unistd.h>
#include <stdbool.h>
#include "RF24Mesh.h"					// BIBLIOTECA DE RF24 POR TMRH20
#include <RF24DP.h>
#include <delay.h>

uint8_t channels1[3] = {117,120,122};

int Lectura_Mesh(RF24Network& netw ){
	int dat=EMPTY_CODE;
	if(netw.available()){
		RF24NetworkHeader header;
		netw.peek(header);
		netw.read(header,&dat,sizeof(dat));
		/*
		switch(header.type){
		// Display the code received from the secundary nodes
		case 'M': netw.read(header,&dat,sizeof(dat)); 
			printf("Rcv %u from 0%o\n",dat,header.from_node);
	        break;
		default:  netw.read(header,0,0); 
	        printf("Rcv bad type %d from 0%o\n",header.type,header.from_node); 
	        break;
		}
		* */
		
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
}

int RF24DP::Comm_Status(void){
	int rtn=0;
	
	// OK      -> 0
	// HOPPING -> 1
	// FIXING  -> 2
	// ERROR_RF   -> 3
	if(hopping)
	{
		rtn=1;
	}
	if(fixing)
	{
		rtn=rtn+1;
	}
	if(fixing)
	{
		rtn=rtn+1;
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
		Status = Lectura_Mesh(netw);	
		
		if(Status==(int)ALARM_FAIL_CODE){
			Code=Alarm_Fail_Mode;
			Receiver.state = RECEIVE_RECEIVED_OK;
			FireComm_Channel.ErrorCounter=0;
		}	
		if(Status==(int)ALARM_CODE){
			Code=Alarm_Mode;
			Receiver.state = RECEIVE_RECEIVED_OK;
			FireComm_Channel.ErrorCounter=0;
		}
		else if(Status==(int)FAIL_CODE){
			Code=Fail_Mode;
			Receiver.state = RECEIVE_RECEIVED_OK;
			FireComm_Channel.ErrorCounter=0;
		}	
		else if(Status==(int)NORMAL_CODE){
			Code=Normal_Mode;
			Receiver.state = RECEIVE_RECEIVED_OK;
			FireComm_Channel.ErrorCounter=0;
		}
		else if(Status==(int)EMPTY_CODE){
			//better luck next time
			Receiver.state = RECEIVE_RECEIVING;
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
				fixing=0;
				hopping=0;
				error=0;
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
	return Code;
}
