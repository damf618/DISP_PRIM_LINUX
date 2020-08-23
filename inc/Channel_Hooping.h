#include <cstdlib>						 // BIBLIOTECA DE FORMATOS STANDARD 
#include <string>		
#include <unistd.h>
#include <stdbool.h>
#include "RF24Mesh.h"					// BIBLIOTECA DE RF24 POR TMRH20
#include <RF24DP.h>
#include <delay.h>
/*
#define CHANNEL_ERROR_TOLERANCE 5
//#define TIME_CHECK 5
#define MESH_TIME 2000

// Possible states for the MEFs
typedef enum{ TRANSMITTING , ERROR, CHANNEL_HOPPING} rf24ch_state_t;

uint8_t channels1[] = {117,120,122};
uint8_t channels2[] = {110,113,115};

// Structure with the different data types to generate an independent Monitor
typedef struct{
	uint8_t channelCounter;
	uint8_t ErrorCounter;
}RF24MChannel_t;

bool RF24ChannelInit(RF24MChannel_t * RFChannel);
void RF24ErrorManage(RF24MChannel_t * RFChannel, RF24DP FireComm);
void RF24ChannelHop(RF24MChannel_t * RFChannel, RF24DP FireComm);
*/

