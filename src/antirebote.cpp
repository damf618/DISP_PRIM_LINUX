/* Copyright 2018, Eric Pernia.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <delay.h>
#include "antirebote.h"
#include <cstdbool>
#include <Primario_LEDS.h>

#define WAITING_TIME 40

// FSM DECLARATIONS

// FSM state names


// FSM functions
/*void fsmInit( void );
void fsmUpdate( void );
*/

// FSM IMPLEMENTATIONS

// Variable that hold the current state

// FSM Error Handler Function
/*void fsmError( void )
{
   // Error handler, example, restart FSM:
   fsmState = INIT;
}
*/

// FSM Initialize Function, resets the released button flag and associates the
// indicated GPIO
void fsmInit(antirebote_t * pbutton, char boton)
{
	pbutton->flag=0;
	pbutton->fsmState = DOWN;   // Set initial state
	pbutton->button= boton;

}

// It sets the Released Flag On, and makes the transition to the next state.
void fsmReleased( antirebote_t * pbutton )
{
	pbutton->fsmState = DOWN;   // Set next state
	pbutton->flag=1;		  // Button released flag
}

// It returns high if the button was released
bool get_State( antirebote_t * pbutton )
{
	if(pbutton->flag==0)	 //
		return 0;
	else {
		if(pbutton->fsmState != DOWN){
			pbutton->flag=0;
		}
		return 1;
	}
}

// FSM Update State Function,
void fsmUpdate( antirebote_t * pbutton )
{

   switch( pbutton->fsmState ){

      case INIT:						//	INITIAL STATE
    	  pbutton->fsmState=DOWN;
      break;

      case DOWN:							//	UP STATE

         /* UPDATE OUTPUTS */

         /* CHECK TRANSITION CONDITIONS */
    	  if(gpioRead( pbutton->button )){
    		  	  pbutton->fsmState = RISING;
    			  delayInit(&pbutton->delay,WAITING_TIME);
    	  }
      break;

      case FALLING:						//	FALLING STATE

         /* UPDATE OUTPUTS */

         /* CHECK TRANSITION CONDITIONS */
    	 if(delayRead(&pbutton->delay)){
    		 if(!gpioRead( pbutton->button )){
    			 pbutton->fsmState = DOWN;
    			 fsmReleased(pbutton);
    		 	 delayWrite(&pbutton->delay,WAITING_TIME);
    		 }
     		 else{
     			pbutton->fsmState = UP;
     		 }

    	 }
      break;

      case UP:						//	DOWN STATE

         /* UPDATE OUTPUTS */

         /* CHECK TRANSITION CONDITIONS */
    	  if(!gpioRead( pbutton->button )){
    		  	  pbutton->fsmState = FALLING;
    	  	  	  delayWrite(&pbutton->delay,WAITING_TIME);
    	  }
      break;

      case RISING:						// RISING STATE

         /* UPDATE OUTPUTS */

         /* CHECK TRANSITION CONDITIONS */
    	  if(delayRead(&pbutton->delay)){
    		  if(gpioRead( pbutton->button )){
     			 pbutton->fsmState = UP;
     			 delayWrite(&pbutton->delay,WAITING_TIME);
     		 }
     		 else{
     			pbutton->fsmState = DOWN;
     		 }
     	 }
      break;

      default:							//	ERROR HANDLING STATE
    	  pbutton->fsmState=UP;
      break;
   }
}
