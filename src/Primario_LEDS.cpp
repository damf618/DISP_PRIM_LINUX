/*=============================================================================
 * Copyright (c) 2020, DANIEL MARQUEZ <DAMF618@GMAIL.COM>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2020/04/12
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "Primario_LEDS.h"
#include <GPIO.h>
#include <cstdbool>
#include <RF24/RF24.h>
#include <stdio.h>


/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/
//Configures the corresponding modes for each GPIO
void gpioSet(void){
	
	GPIOMode(RED_LIGHT,OUTM,PULLOFF);
	GPIOMode(YELLOW_LIGHT,OUTM,PULLOFF);
	GPIOMode(GREEN_LIGHT,OUTM,PULLOFF);
	
	GPIOMode(ALARM_BUTTON,INPUTM,PULLUP);
	GPIOMode(FAIL_BUTTON,INPUTM,PULLUP);
	GPIOMode(NORMAL_BUTTON,INPUTM,PULLUP);	
}

// Sets the GPIO indicated as lamp, HIGH.
void turnOn(char lamp){
	digitalWrite(lamp , HIGH );
}

// Sets the GPIO indicated as lamp, LOW.
void turnOff(char lamp){
	digitalWrite(lamp, LOW );
}

// Reads the GPIO digital status.
uint8_t gpioRead(uint8_t lamp){
	return digitalRead (lamp);
}
