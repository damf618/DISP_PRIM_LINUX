#ifndef RF24DP_H
#define RF24DP_H

//We need the C++ Compiler
/*
#ifdef __cplusplus
extern "C" {
#endif
*/

#include <delay.h>
#include <stdio.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include "primario4.h"

#define MASTER_ID 0
#define RF_DEFAULT_CHANNEL 120

#define CE_pin_default 			22
#define CSN_pin_default			0
#define Number_default 			1
#define Contacto_A_default		19
#define Contacto_F_default		26
#define Status_Comm_default		0
#define Reset_Pin_default		20

#define read_timeout_default	DEF_TIMEOUT-500
#define write_timeout_default	100

#define Comm_timeout 0
#define Comm_received 1
#define Comm_receiving 2

#define EMPTY_CODE 333
#define NORMAL_CODE 321
#define FAIL_CODE 654
#define ALARM_CODE 987
#define ALARM_FAIL_CODE 258

#define OK_MSG 0
#define ERROR_MSG 1
#define HOPPING_MSG 2
#define FIXING_MSG 3

#define WRITING_MAX_TIME 1000
#define READING_MAX_TIME 200

#define RETRIES_MAX_NUMBER 15			//How many retries before giving up, max 15*
#define RETRIES_MAX_SPACING 15			//0 means 250us, 15 means 4000us, max 15*  

#define NORMALIZATION_TIME 10			//min 300 possible time to send amessage after the reading.
#define RESET_TIME 5000					//MAX Time for stablish communication response.

#define CHANNEL_ERROR_TOLERANCE 5
//#define TIME_CHECK 5
#define MESH_TIME read_timeout_default



// Structure with the different data types to generate an independent Monitor
typedef struct{
	uint8_t ChannelCounter;
	uint8_t ErrorCounter;
}RF24MChannel_t;

enum Code_t{
   Normal_Mode=0,
   Fail_Mode,
   Alarm_Mode,
   Alarm_Fail_Mode,
   Receiving_Mode
}; 

typedef enum{
   RECEIVE_CONFIG,
   RECEIVE_RECEIVING,
   RECEIVE_RECEIVED_OK,
   RECEIVE_FULL_BUFFER,
   RECEIVE_TIMEOUT
} ReceiveOrTimeoutState_t;

typedef struct{
	ReceiveOrTimeoutState_t state;
	char*    string;
	uint16_t stringSize;
	uint16_t stringIndex;
	tick_t   timeout;
	delay_t  delay;
} ReceiveOrTimeout_t;

int Lectura_Mesh(RF24Network& netw );

class RF24DP : public RF24Mesh	{
	
	private:
		char CE_pin = CE_pin_default; 			//22;
		char CSN_pin = CSN_pin_default;	 		//0;
		char Number = Number_default;			//1;
		char Contacto_A = Contacto_A_default;	//19; 
		char Contacto_F = Contacto_F_default;	//26;
		int Status_Comm = Status_Comm_default;	//0;
		char Reset_Pin = Reset_Pin_default;		//20;
		uint64_t counter=0;
		ReceiveOrTimeout_t Receiver;
		Code_t Code=Receiving_Mode;
		RF24MChannel_t FireComm_Channel;
		bool error = 0;
		bool hopping = 0;
		
	public:
		
		RF24DP(RF24& _radio, RF24Network& _network):RF24Mesh{_radio,_network}{
			RF24Mesh( _radio , _network);  
		}	
		void Init(void);
		void Set_Up(void);
		void Set_Channel(uint8_t Channel, uint32_t timeout); 
		void Init(char CE, char CSN, char Num, char CA, char CF, char SC, char Reset, unsigned int timeout);
		void Set_Up(char intentos, char tiempo, char Canal);
		void Writing_Data(void); 
		bool Read_Status(void);
		void Normalization_Delay(void);
		void Read_Data (RF24Network& netw);
		ReceiveOrTimeoutState_t  Wait_for_Code (RF24Network& netw);
		void RF24DPReset(void);
		void RF24DPUpdate(RF24Network& netw);
		char RF24DPRead(void);
		void RF24ChannelHop(void);
		void RF24ErrorManage(void);
		int Get_Code(void);
		int Comm_Status(void);
};

/*
#ifdef __cplusplus
}
#endif
*/

#endif 
