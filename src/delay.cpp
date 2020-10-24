#include <delay.h>
#include <RF24/RF24.h>

/* ---- Non Blocking Delay ---- */

//Validation of the number of ticks
unsigned int tickRead(void){
	return millis();
}

//Init of the delay object
void delayInit( delay_t * delay, tick_t duration )
{
   delay->duration = duration;
   delay->running = 0;
}

//Validation of the timeout event of the delay
bool delayRead( delay_t * delay )
{
   bool timeArrived = 0;

   if( !delay->running ) {
      delay->startTime = tickRead();
      delay->running = 1;
   } else {
      if ( (tick_t)(tickRead() - delay->startTime) >= delay->duration ) {
         timeArrived = 1;
         delay->running = 0;
      }
   }

   return timeArrived;
}

//Modification of the delay object 
void delayWrite( delay_t * delay, tick_t duration )
{
   delay->duration = duration;
}
