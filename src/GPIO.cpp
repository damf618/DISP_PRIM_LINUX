#include <bcm2835.h>
#include <stdio.h>
#include <GPIO.h>

RPiGPIOPin value=RPI_BPLUS_GPIO_J8_40;

void conversion (char Param){
	  /* RPi B+ J8 header, also RPi 2 40 pin GPIO header */
	switch(Param){
	case 2:
		value= RPI_BPLUS_GPIO_J8_03;
		break;
	case 3:
		value= RPI_BPLUS_GPIO_J8_05;
		break;
	case 4:
		value= RPI_BPLUS_GPIO_J8_07;
		break;
	case 14:
		value= RPI_BPLUS_GPIO_J8_08;
		break;
	case 15:
		value= RPI_BPLUS_GPIO_J8_10;
		break;
	case 17:
		value= RPI_BPLUS_GPIO_J8_11;
		break;
	case 18:
		value= RPI_BPLUS_GPIO_J8_12;
		break;
	case 27:
		value= RPI_BPLUS_GPIO_J8_13;
		break;
	case 22:
		value= RPI_BPLUS_GPIO_J8_15;
		break;
	case 23:
		value= RPI_BPLUS_GPIO_J8_16;
		break;
	case 24:
		value= RPI_BPLUS_GPIO_J8_18;
		break;
	case 10:
		value= RPI_BPLUS_GPIO_J8_19;
		break;
	case 9:
		value= RPI_BPLUS_GPIO_J8_21;
		break;
	case 25:
		value= RPI_BPLUS_GPIO_J8_22;
		break;
	case 11:
		value= RPI_BPLUS_GPIO_J8_23;
		break;
	case 8:
		value= RPI_BPLUS_GPIO_J8_24;
		break;
	case 7:
		value= RPI_BPLUS_GPIO_J8_26;
		break;
	case 5:
		value= RPI_BPLUS_GPIO_J8_29;
		break;
	case 6:
		value= RPI_BPLUS_GPIO_J8_31;
		break;
	case 12:
		value= RPI_BPLUS_GPIO_J8_32;
		break;
	case 13:
		value= RPI_BPLUS_GPIO_J8_33;
		break;
	case 19:
		value= RPI_BPLUS_GPIO_J8_35;
		break;
	case 16:
		value= RPI_BPLUS_GPIO_J8_36;
		break;
	case 26:
		value= RPI_BPLUS_GPIO_J8_37;
		break;
	case 20:
		value= RPI_BPLUS_GPIO_J8_38;
		break;
	case 21:
		value= RPI_BPLUS_GPIO_J8_40;
		break;
	default:
		value= RPI_BPLUS_GPIO_J8_40;
		break;\
	}	
}

void GPIOMode(char PIN,char Mode,char Res){
	conversion (PIN);
	 printf("value: %d\n", value);
	if ((bcm2835_init()) || (value!=0)){   
    // Set RPI pin P1-15 to be an input
    if (Mode==INPUTM){
		bcm2835_gpio_fsel(value, BCM2835_GPIO_FSEL_INPT);
		 printf("INPUT");
	}
	else if(Mode==OUTM){
		bcm2835_gpio_fsel(value, BCM2835_GPIO_FSEL_OUTP);
		printf("OUTPUT");
	}
		
    //  with a pullup
    if (Res==PULLUP){
		bcm2835_gpio_set_pud(value, BCM2835_GPIO_PUD_UP);
		 printf("PULLUP");
	}
	else if(Res==PULLDOWN){
		bcm2835_gpio_set_pud(value, BCM2835_GPIO_PUD_DOWN);
		printf("PULLDOWN");
	}
	else{
		bcm2835_gpio_set_pud(value, BCM2835_GPIO_PUD_OFF);
		printf("PULLOFF");
	}
	}
}

uint8_t digitalRead(uint8_t PIN){
	conversion(PIN);
	if(value==0)
		return 7;
	else
		return bcm2835_gpio_lev(value);
}
