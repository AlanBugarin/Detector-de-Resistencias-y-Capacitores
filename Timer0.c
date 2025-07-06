#include <avr/io.h>
#include <avr/interrupt.h>
#include "Timer0.h"
//#ticks=1ms/1/(FCPU/PS) PS=256 ticks=62

uint8_t msFlag=0;
void Timer0_Ini(void)
{
	TCCR0A=(2<<WGM00); //Modo 2 CTC
	TCCR0B=(1<<CS02); //PS=256
	OCR0A=62-1; //1ms
	TCNT0=0;
	TIMSK0=(1<<OCIE0A); //Comparacion Modo A
	sei();
}

ISR(TIMER0_COMPA_vect)
{
	static uint16_t millis=0;
	millis++;
	if(millis==1){
	    msFlag=1;
		millis=0;
	}
}

uint8_t msFlagFunc(void)
{
	if(msFlag)
	{
		msFlag=0;
		return 1;
	}else {
		return 0;
	}
	
	
}