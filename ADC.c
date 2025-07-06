#include <avr\io.h>


static uint8_t offset=0; //Error de sezgo o medicion
static uint8_t gain=0; //Error de gain
void ADC_Ini ( void )
{
	
	//ADC calibracion
	ADCSRA=(1<<ADEN) |(7<<ADPS0); //Conf anterior
	ADMUX |= (0x1F<<MUX0)| (3<<REFS0); //Vref=2.56V ,VIN=0 (Osea lo de MUX)
	ADCSRB &= ~(1<<MUX5);
	//Start Conversion
	ADCSRA |= 1<<ADSC;
	while(ADCSRA & (1<<ADSC));
	offset=(uint8_t)ADC; //tenemos el offset
	
	
	//Conf de ADC
	ADMUX=(1<<REFS0); //Vref=5V
	ADCSRA=(1<<ADEN)|(7<<ADPS0); //Habilito ADC,FreqADC=125Khz
	ADCSRB=0;
    return;
}
uint16_t ADC_Read( uint8_t channel )
{
    if(channel == 0 || channel >=16){
		DIDR0= (1<<ADC0D); //Para canal 0 o por defecto
	}else{
		if(channel < 8){ //Estructura para ver que canal es
			ADMUX |= (channel<<MUX0);  //Canal menor a 8
			DIDR0 |= (1<<channel);
		}else{
			ADMUX |= ((channel-8)<<MUX0); //Canal es >=8
			ADCSRB |= (1<<MUX5);
			DIDR2  |=(1<<(channel-8));
		}
	}
	
	ADCSRA |=(1<<ADSC);
	while((ADCSRA & (1<<ADSC))) 
		;
	return (ADC-offset);
}
