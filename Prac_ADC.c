/*
    Example code written during the Lab Session given only as reference, 
    Please update the function to your own implementation in order to compile.
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "UART.h"
#include "ADC.h"
#include "Timer0.h"

extern void Delay(uint8_t ms);
void DescargarCapacitor(uint16_t adcv1);
void esCapacitor(void);
void esResistencia(void);
#define NUM_MUESTRAS 8
// ADC1, VRef = Vcc, 10 bits

// ADC Test example
#define R1 470000UL
#define R2 680UL
int main(void)
{
	rx_buffer0.in_idx=0; //Inicializacion del Buffer
	rx_buffer0.out_idx=0;
	tx_buffer0.in_idx=0;
	tx_buffer0.out_idx=0;

	Timer0_Ini();
	UART_Ini(0,115200,8,0,1);
	ADC_Ini();
	UART_clrscr(0);
	UART_getchar(0);
	DDRA =(1<<PA2);	
	DDRA |= (1<<PA1);
	PORTA &= ~(1<<PA1);

	while(1)
	{	
		uint16_t adcvp1=0;
		uint16_t adcvp2=0;
		uint32_t mSs=0; //ms para ver si es resitencia o capacitor
		uint16_t vA=0;
		
		UART_puts(0,"Presione un boton cuando ingrese el componente a calcular");
		UART_puts(0,"\n\r");
		UART_getchar(0);
		adcvp1=ADC_Read(1); //Tomamos la primera muestra
		DescargarCapacitor(adcvp1);
		// Iniciar carga con PA1 en alto
		DDRA |= (1 << PA1);
		PORTA |= (1 << PA1);

		// Primera lectura
		adcvp1 = ADC_Read(1);
		
		while(mSs<3){
			if(msFlagFunc()){
				mSs++;
				if(mSs == 3){
					adcvp2=ADC_Read(1); //Tomo la segunda muestra
					if(adcvp2>adcvp1){
						vA=adcvp2-adcvp1;
					}else{
						vA=adcvp1-adcvp2;
					}
				}
			}
		}
		
		if(vA>4){
			 UART_puts(0,"Es un Capacitor \n\r");
	    	 UART_getchar(0);
	    	 esCapacitor();
		}else{
			UART_puts(0,"Es una Resistencia\n\r");
		    UART_getchar(0);
		    esResistencia();
		}

		
	}

	return 1;
}

void DescargarCapacitor(uint16_t adcv1){
		char strq[15];
		if(adcv1>200){
			UART_puts(0,"Descargando Componente");
			UART_puts(0,"\n\r");
			DDRA  |= (1<<PA0);
			PORTA &= ~(1<<PA0);
			DDRA |= (1<<PA1);
			PORTA &= ~(1<<PA1);
			while(adcv1>0){
				adcv1=ADC_Read(1); //Espero hasta que se descarga
				/*
				itoa(adcv1,strq,10);
				UART_puts(0,strq);
				UART_puts(0,"\n\r");
				*/
			}
		}
}

void esCapacitor(void)
{
		char strq[15];
		uint16_t ms=0;
		uint16_t adcv1=0;
		uint32_t resConocidaC=R2;
		adcv1=ADC_Read(1); //Leo si esta cargado
		adcv1=ADC_Read(1); //Leo si esta cargado
		DescargarCapacitor(adcv1);
		
		UART_puts(0,"Cargando Componente");
		UART_puts(0,"\n\r");
		DDRA &= ~(1<<PA0);
		PORTA &= ~(1<<PA0);
		DDRA  |= (1<<PA1); //Cargando con R 680
		PORTA |=(1<<PA1);
		while(1){
			if(msFlagFunc()){
				adcv1=ADC_Read(1);
				if(ms==100 && adcv1<600){ //si ha pasado 100ms y aun no carga al limite entonces switchea a 470k
						UART_puts(0,"Cambio a R 470K");
						UART_puts(0,"\n\r");
						resConocidaC=R1; //Res 470k
						
						DescargarCapacitor(adcv1);
						UART_puts(0,"Cargando Componente");
						UART_puts(0,"\n\r");
						DDRA |= (1<<PA0); //Activando carga por R1
						PORTA |= (1<<PA0);
						DDRA  &= ~(1<<PA1);
						PORTA &= ~(1<<PA1);
						ms=0;
						continue;
				}
				if(adcv1>=1000){
					break;
				}
				itoa(adcv1,strq,10);
				UART_puts(0,strq);
				UART_puts(0,"\n\r");
				ms++;
			}
		}
		UART_puts(0,"tiempo (ms):");
		UART_puts(0,"\n\r");
		itoa(ms,strq,10);
		UART_puts(0,strq);
		UART_puts(0,"\n\r");
		uint32_t temp=(1000UL * ms);
		uint16_t C= temp/(4 * resConocidaC);
		UART_puts(0,"Capacitancia:");
		UART_puts(0,"\n\r");
		itoa(C,strq,10);
		UART_puts(0,strq);
		UART_puts(0,"\n\r");	
		
}
void esResistencia(void)
{
		char str[15];
		uint32_t vccR=0;
		uint16_t valorProm=0;
		uint32_t res=0;
		uint32_t resConocida=R1;
		uint16_t value=0;
		valorProm = 0;
		
		//Primero con R 470K
		DDRA  |= (1<<PA0);
		PORTA |=(1<<PA0);
		PORTA &= ~(1<<PA1);
		DDRA &= ~(1<<PA1);
		PORTA &= ~(1<<PA2);
		UART_puts(0,"Calculando con R de 470K");
		UART_puts(0,"\n\r");
		
		for(uint8_t j=0;j<NUM_MUESTRAS;j++){
			value = ADC_Read(1);
			for(uint8_t i=0; i < (value>>4); i++)
			{
				UART_putchar(0,'.');
			}
			itoa(value,str,10);
			UART_puts(0,str);
			UART_puts(0,"\n\r");
			valorProm=valorProm+value;			
		}
		valorProm=valorProm/NUM_MUESTRAS; //ADC promedio
		vccR=(valorProm*5000UL)/1023UL; //Voltaje calculado en el punto
		
		if(vccR<=300)//Cambiar a R2
		{
			UART_puts(0,"Calculando con R de 680");
			UART_puts(0,"\n\r");
			valorProm=0;
			resConocida=R2;
			PORTA &= ~(1<<PA0);
			DDRA &= ~(1<<PA0);
			DDRA  |= (1<<PA1);
			PORTA |= (1<<PA1);
			PORTA &= ~(1<<PA2);
			
			for(uint8_t j=0;j<NUM_MUESTRAS;j++){
				value = ADC_Read(1);
				for(uint8_t i=0; i < (value>>4); i++)
				{
					UART_putchar(0,'.');
				}
				itoa(value,str,10);
				UART_puts(0,str);
				UART_puts(0,"\n\r");
				valorProm=valorProm+value;			
			}
			valorProm=valorProm/NUM_MUESTRAS; //ADC promedio
			vccR=(valorProm*5000UL)/1023UL; //Voltaje calculado en el punto
		}
		
		UART_puts(0,"Voltaje: ");
		itoa(vccR,str,10);
		UART_puts(0,str);
		UART_puts(0,"\n\r");
		
		res=(resConocida * vccR)/(5000UL - vccR); //Calc resistencia
		UART_puts(0,"Resistencia: ");
		itoa(res,str,10);
		UART_puts(0,str);
		UART_puts(0,"\n\r");	
	
}