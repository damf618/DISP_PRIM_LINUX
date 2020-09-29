#include <cstdlib>						 // BIBLIOTECA DE FORMATOS STANDARD 
#include <string>		
#include <unistd.h>
#include <stdbool.h>
#include "RF24Mesh.h"					// BIBLIOTECA DE RF24 POR TMRH20
#include <RF24DP.h>
#include <delay.h>
#include <Cuaima_Testing.h>

uint8_t channels1[3] = {117,120,122};

//If theres a new nodeID detected, we ask for memory and save it together with the
// code received in the corresponding array
/** The RF system is based on an array of RF_Device_t that allows to detect how many
 * actives nodes are in the system. This system allows too identify if there are enough
 * nodes interacting with the system or not 
	
	@param int id, ID of the new RF Device detected.
	@param int Code Code transmitted by this new Device.
	@param RF_List_t* RF_List Array of RF_Device_t to keep track of the memory required
	from the system
	
	@see Header_Validation,Comm_Code
**/
static bool Insert(int id, int Code,RF_List_t* RF_List){
	
	RF_Device_t * Mem_Block;
	Mem_Block = (RF_Device_t *)malloc(sizeof(RF_Device_t));
	bool rtn=0;
	
	//If there is space availble, we add the Device to the RF_LIst array
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

//Allows to check if a message  is received, is it from a  new RF device or an old one
/** If we received a RF message with a valid code, we save the id and the code sent. This
 * code allows us to verify if the message received was from a new RF Device or not.
 * 
	
	@param int id, ID of the new RF Device detected.
	@param RF_List_t* RF_List Array of RF_Device_t to keep track of the memory required
	from the system
	
	@see Header_Validation,Comm_Code
**/
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

//Allows to check if a message  is received, is it from a  new RF device or an old one
/** If we received a RF message with a valid code, we save the id and the code sent. This
 * function allows us to verify if the message received was from a new RF Device or not.
 * In case we have already received a message from this RF Device before, we update the 
 * code associated in the RF_List
	
	@param int id, ID of the new RF Device detected.
	@param RF_List_t* RF_List Array of RF_Device_t to keep track of the memory required
	from the system
	
	@see Comm_Code
**/
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

void Comm_Nodes(RF_List_t* RF_List){
	int i;
	int counter=0;
	RF_Device_t * Mem_Block;
	
	for(i=0;i<RF_List->counter;i++)
	{
		Mem_Block=RF_List->RF_Devices[i];
		if(Mem_Block[0].updated)
		{
			counter++;
			Mem_Block[0].updated=0;
		}
	}
	printf("# N Nodes----Get_Node: %d\n",counter);
	RF_List->active_nodes = counter;
}

//The link between RF Communication and primario4, this funciton returns the corresponding
//code based on the RF_List generated between calls.
/** If we received a RF message with a valid code, we save the id and the code sent. This
 * code allows us to verify if the message received was from a new RF Device or not.
 * In case we have already received a message from this RF Device before, we update the 
 * code associated in the RF_List.
 * Calling this function evaluate the different codes saved and based on it's priority it
 * returns the corresponding code, it only takes on consideration those codes associated
 * to RF_Devices which codes were updated between calls of this function, once this function
 * is called it "disables" the id until Header_Validation enables it through the updated 
 * flag.
 * This scheme also give us information about the number of nodes interacting with us, 
 * beside the number of nodes in the system.
	
	@param RF_List_t* RF_List Array of RF_Device_t to keep track of the memory required
	from the system
	
	@see Header_Validation.
**/
int Comm_Code(RF_List_t* RF_List){
	int Final_Code;
	bool Alarm=0;
	bool Fail=0;
	int i;
	int counter=0;
	RF_Device_t * Mem_Block;
	printf("Number of Devices: %d\n",RF_List->counter);
	
	for(i=0;i<RF_List->counter;i++)
	{
		Mem_Block=RF_List->RF_Devices[i];
		printf("#%d Device: %d\n",i,Mem_Block[0].Node_ID);
		if(Mem_Block[0].updated)
		{
			printf("#%d Device: %d,State: %d\n",i,Mem_Block[0].Node_ID,Mem_Block[0].RF_Code);
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

//Clean the RF_List and free of the memory used to allocate the RF_Devices
/** If for some reasoon there is an error or we are stopping the execution of the program
 * the call of this function allows to restart the RF_List by freeing the memory  allocated.
	
	
	@note ususally called by Kill_Them_All or RF_Maintenance_thread.
**/
void RF24DP:: Clean_RFDevices(){
	int k;
	
	for(k=0;k<RF_List.counter;k++)
	{
		free(RF_List.RF_Devices[k]);
		RF_List.counter=0;
	}
}

// Poolling on the RF Network to check if there is a new message
/** This function is called to verufy if there are new RF messages, if there 
 * are, it checks if the message has the correct header and if it does, it
 * checks if it is a new RF_Device or not, checking if it was already added to
 * the RF_List of devices, finally it also verifies if the maximum the number of 
 * devices present in the list in case we are out of memory.
	
	@param RF24Network& netw RF24Network class associated to the system
 	@param RF_List_t * RF_Devices List  of RF_Devices used to register code received
 	and actives nodes.
	@see Header_Validation
	@note MUst be called frequently to make sure at least one message from all the
	RF_Devices is received in orderto process the RF_Code accordingly.
**/
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
				printf("Error Nro Max Alcanzado\n");
				dat = RESET_REQUEST;
			}
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

// Poolling on the RF Network to check if there is a new message
/** One of the methods to restablish communication using RF is by changing the frequency,
 *  maybe moving the channel we are transmitting/receiving, the are three possible
 *  channels to move to. HOw does this works? if we are sending in channel 1 and we 
 *  have failed to receive a message in the next 5 attempts, then we change to a new channel
 *  this goes one more time until we reach the fifth try on the channel 3 in case, if
 *  we have not received any message we send a notfication to start the RF_Maintenance_thread
	
	@see RF_Maintenance_thread
**/
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

// Reset the flags associated to comm_status
/** There are some variables that works as flag to indicate in which status we are currently
 *  this functions clears those variables and reset the RF_System to initial conditions.  
 * 	
	@see RF_Maintenance_thread
**/
void RF24DP::Maintenance_clean(void){
	error=0;
	hopping=0;
	fixing=0;
	reset_request=0;
	RF_List.counter=0;
}

// Return which is the status of the RF Communication 
/** This function is used to indicate which is the status of the RF System based.
 * 
 *  @note there is no "Incomplete" state because it is a different type of error, and
 *  it is handled differently.

	@see CurrentState
**/
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

// Wait for a specified number of tries before changing the RF Channel 
/** Using this function we give some error tolerance before having to use the hopping
 * protocol, if do not receive any meesage on CHANNEL_ERROR_TOLERANCE tries, then we can
 * start the hopping protocol.
 * 
 *  @note there is no "Incomplete" state because it is a different type of error, and
 *  it is handled differently.

	@see RF24ChannelHop
**/
void RF24DP:: RF24ErrorManage(void){
	FireComm_Channel.ErrorCounter++;
	printf("\r\n Numero de Errores Timeout: %d \r\n",FireComm_Channel.ErrorCounter);
	if(FireComm_Channel.ErrorCounter>=CHANNEL_ERROR_TOLERANCE){
		RF24ChannelHop();
	}
}

// Setup configurations for the RF24Mesh Class 
/** Setting of initial conditions an required calls from the RF24Mesh library.

	@see RF24Mesh
**/
void RF24DP:: Set_Up(void) {
	setNodeID(MASTER_ID);
	begin(RF_DEFAULT_CHANNEL);
}	

// Set new RF CHannel 
/** Function to correctly change channel without affecting other parameters.
 * 
 *  @param Channel Value from 1 to 125, to indicate the frequency selected to work with.
 *  @param max_time Amount of time to wait for a connection in the new channel.
	@see RF24Mesh
**/
void RF24DP:: Set_Channel(uint8_t Channel, uint32_t max_time) {
	begin(Channel,RF24_1MBPS,max_time);	
}	

// Initialization of the RF24DP class 
/** Setting of initial conditions for the custom RF24DP library based on RF24Mesh.
 * 
	@see RF24Mesh
**/
void RF24DP:: Init(void){
	Receiver.timeout= read_timeout_default;
	Receiver.state= RECEIVE_CONFIG;
	delayInit( &Receiver.delay,Receiver.timeout);
	FireComm_Channel.ChannelCounter=0;
	FireComm_Channel.ErrorCounter=0;
	RF_List.counter=0;
	RF_List.n_nodes=0;
	RF_List.active_nodes=0;
	Active_nodes=0;
}

// Indicate the number of actives nodes obtain during the RF_List check
/** Using this function we give some error tolerance before having to use the hopping
 * protocol, if do not receive any meesage on CHANNEL_ERROR_TOLERANCE tries, then we can
 * start the hopping protocol.
 * 
 *  

	@see Comm_Code
**/
void RF24DP:: NnodesUpdate(int nnodes){
	RF_List.n_nodes=nnodes;	
}

// Manual set up of the different parameters used by the RF System
/** This function gives the possiibility to modify the default pins for some of the 
 * GPIO configurations.
	
	@param CE RF24 Configuration.
 	@param CSN RF24 Configuration.
 	@param Node Address.
 	@param CA Alarm_Contact pin.
 	@param CF Fail_Contact pin.
 	@param SC .
 	@param Reset .
 	@param timeout timeout event period.
	@see Header_Validation
	@note MUst be called frequently to make sure at least one message from all the
	RF_Devices is received in orderto process the RF_Code accordingly.
**/
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

//Could be some Improvements here

// Manual set up of the different parameters used by the RF System
/** Reads the RF Message and updates the state of the RF System FSM, checks if there are any
 * RF messages and if there are updates the RF_List.
	
	@param netw RF24Network& RF24Network intance of the system
**/	
void RF24DP::Read_Data(RF24Network& netw){
	int Status=0;
	if (delayRead(&Receiver.delay)) 
	{
		Receiver.state = RECEIVE_TIMEOUT;
	}else
	{
		Status = Lectura_Mesh(netw,&RF_List);
		//If we received an empty code and there previous code was also empty. 
		if((EMPTY_CODE==Status)&&(EMPTY_CODE!=Code))
		{
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

//FSM in control of thee entire RF_System	
/** The entire logic of the system is handled here, based on the current state, the FSM
 *  will decide what actions should be taken. It is called frequently in order to response
 *  as fast as possible.
	
	@param netw RF24Network& RF24Network intance of the system
**/	
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

// COMENTAR
void RF24DP::RF24DPReset(void){
	Receiver.state = RECEIVE_CONFIG;			// It resets the MEF stage to receive this
    delayInit(&Receiver.delay,Receiver.timeout);		// Configure the UART Listening Process delay for the timeout measuring.
}

// COMENTAR
void RF24DP::RF24DPUpdate(RF24Network& netw){
	// Call network.update as usual to keep the network updated
	update();
	// In addition, keep the 'DHCP service' running on the master node so addresses will
	// be assigned to the sensor nodes
	DHCP();
	Receiver.state = Wait_for_Code( netw ); //Updates the MEF to receive the next byte.
}

// COMENTAR
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

// COMENTAR
int RF24DP::Get_Code(void){
	int rtn;
	rtn = Comm_Code(&RF_List);
	Active_nodes=RF_List.active_nodes;
	return rtn;
}

// COMENTAR
int RF24DP::Get_Nodes(void){
	//Comm_Nodes(&RF_List);
	//Active_nodes=RF_List.active_nodes;
	return Active_nodes;
}
