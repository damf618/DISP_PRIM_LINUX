#include <delay.h>
#include <RF24/RF24.h>

/* ---- Non Blocking Delay ---- */
unsigned int tickRead(void){
	return millis();
}

void delayInit( delay_t * delay, tick_t duration )
{
   delay->duration = duration;
   delay->running = 0;
}

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

void delayWrite( delay_t * delay, tick_t duration )
{
   delay->duration = duration;
}
