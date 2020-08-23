#include <cstdlib>						 // BIBLIOTECA DE FORMATOS STANDARD 
#include <string>		
#include <unistd.h>
#include <stdbool.h>
#include "RF24Mesh.h"					// BIBLIOTECA DE RF24 POR TMRH20
#include "Channel_Hooping.h"
#include <RF24DP.h>
#include <delay.h>

/*
void RF24ChannelHop(RF24MChannel_t * RFChannel, RF24DP FireComm){
	//CUIDADO CON BLOQUEO POR TIEMPOS PROLONGADOS
	printf("New Channel Assignation: %d \n",channels1[RFChannel->channelCounter]);
	FireComm.Set_Channel(channels1[RFChannel->channelCounter],MESH_TIME);
	RFChannel->channelCounter++;
	if(RFChannel->channelCounter>=3){
		RFChannel->channelCounter=0;
	}
	RFChannel->ErrorCounter=0;
}
*/
/*
void RF24ErrorManage(RF24MChannel_t * RFChannel, RF24DP FireComm){
	RFChannel->ErrorCounter++;
	if(RFChannel->ErrorCounter>=CHANNEL_ERROR_TOLERANCE){
		RF24ChannelHop(RFChannel,);
	}
}
*/
// It sets initial conditions for the RFChannel Hopping Scheme
/*
bool RF24ChannelInit(RF24MChannel_t * RFChannel, tick_t time, RF24DP FireComm)
{
	if(NULL==RFChannel)
		return 0;
	RFChannel->channelCounter=0;
	RFChannel->ErrorCounter=0;
	return 1;
}

*/

// The MEFS logic, execute the actions related to the
/*
bool RF24Control(RF24MChannel_t * RFChannel){

	if(NULL==RFChannel)
		return 0;

	switch( RFChannel->state ) {

		case TRANSMITTING:
			//TODO SI TODO VA BIEN
			break;
		case ERROR:
			RF24ErrorManage(RFChannel);
			break;
		case CHANNEL_HOPPING:
			RF24ChannelHop(RFChannel);
			break;
		default:
			RFChannel->state=ERROR;
		}
	return 1;
}
*/


