#ifndef DELAY_H_
#define DELAY_H_

#include <cstdbool>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int tick_t;

typedef struct{
   tick_t startTime;
   tick_t duration;
   bool running;
} delay_t;

unsigned int tickRead(void);
void delayInit( delay_t * delay, tick_t duration );
bool delayRead( delay_t * delay );
void delayWrite( delay_t * delay, tick_t duration );


#ifdef __cplusplus
}
#endif

#endif
