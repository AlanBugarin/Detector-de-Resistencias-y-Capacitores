#include <avr/io.h>
#include <avr/interrupt.h>
#include "UART.h"
// Prototypes
// Initialization
void UART_Ini(uint8_t com,uint32_t baudrate,uint8_t size,uint8_t parity,uint8_t stop);
void UART0_AutoBaudRate(void);

// Send
void UART_puts(uint8_t com,char *str);
void UART_putchar(uint8_t com,char data);

// Received
uint8_t UART_available(uint8_t com);
char UART_getchar(uint8_t com );
void UART_gets(uint8_t com,char *str);

// Escape sequences
void UART_clrscr( uint8_t com );
void UART_setColor(uint8_t com,uint8_t color);
void UART_gotoxy(uint8_t com,uint8_t x,uint8_t y);

// Utils
void itoa(uint16_t number,char* str,uint8_t base);
uint16_t atoi(char *str);


void UART0_AutoBaudRate(void)
{
    /* Implementar */
    //Autobaud [8000bps :200,000 bps]
    //		   [125us   : 5us]  Saber cuanto tiempo voy a contar
    //T=1/F
    //#ticks =125us * (16Mhz/PS) = 250 //CON PS=8
    TCCR0B=0;//Desabilitar Timer
    TCNT0=0;//Inicializar contador
    TCCR0A=0;
	while(PINE & (1<<PE0)); //verificar el cambio de alto a bajo
	TCCR0B=2<<CS00;//PS=8
	
	while(!(PINE & (1<<PE0)));
	TCCR0B=0;//Parar contador
	
	UBRR0=TCNT0; //Velocidad doble mejor para este caso
}


typedef struct{
	union{
		uint8_t UCSRA;
		struct{
			uint8_t mpcm:1;
			uint8_t u2x:1;
			uint8_t upe:1;
			uint8_t dor:1;
			uint8_t fe:1;
			uint8_t udre:1;
			uint8_t txc:1;
			uint8_t rxc:1;
		};
	};
	union{
		uint8_t UCSRB;
		struct{
			uint8_t txb8:1;
			uint8_t rxb8:1;
			uint8_t ucsz2:1;
			uint8_t txen:1;
			uint8_t rxen:1;
			uint8_t udrie:1;
			uint8_t txcie:1;
			uint8_t rxcie:1;			
		};
	};
	union{
		uint8_t UCSRC;
		struct{
			uint8_t ucpol:1;
			uint8_t ucsz0:1;
			uint8_t ucsz1:1;
			uint8_t usbs:1;
			uint8_t upm0:1;
			uint8_t upm1:1;
			uint8_t umsel0:1;
			uint8_t umsel1:1;			
		};
	};
	union{
		uint8_t res;
	};
	union{
		uint16_t UBRR;
		struct{
			uint8_t ubrrl:8;
			uint8_t ubrrh:8;
		};
	};
	
}UART_Regs_t;


uint8_t *uart_offsets[]={
	&UCSR0A,
	&UCSR1A,
	&UCSR2A,
	&UCSR3A,	
};

void UART_Ini(uint8_t com,uint32_t baudrate,uint8_t size,uint8_t parity,uint8_t stop)
{
	UART_Regs_t *myUart = uart_offsets[com];
	
	//Calculo para sacar baudaje normal
	uint16_t velNormal=16000000/16/baudrate-1;
	uint16_t baudNormal=16000000/16/(velNormal+1);

	//calculo para sacar baudaje Doble velocidad
	uint16_t velDoble=16000000/8/baudrate-1;
	uint16_t baudDoble=16000000/8/(velDoble+1);

	//Sacamos el error
	uint16_t errorNormal = ((baudNormal - baudrate) * 100) / baudrate;
    uint16_t errorDoble = ((baudDoble - baudrate) * 100) / baudrate;
		

	
	//Registro A	
	if(errorDoble<=errorNormal){
		myUart->u2x=1;
		myUart->ubrrh=(unsigned char)(velDoble>>8);
		myUart->ubrrl=(unsigned char)velDoble;
	}else{
		myUart->ubrrh=(unsigned char)(velNormal>>8);
		myUart->ubrrl=(unsigned char)velNormal;	
	}

	//Registro B
	myUart->txen=1;
	myUart->rxen=1;
		//Activo las interrupciones en perifericos
	myUart->rxcie=1;
	myUart->udrie=1;

	//Registro C
	if(size==6){ //para el tamano de los bits
		myUart->ucsz0=1;	
	}else if(size==7){
		myUart->ucsz1=1;
	}else if(size==8){
		myUart->ucsz0=1;
		myUart->ucsz1=1;
	}
	if(parity==1){ //configuracion de paridad
		myUart->upm1=1;
	}else if(parity==2){
		myUart->upm1=1;
		myUart->upm0=1;
	}
	if(stop==2){
		myUart->usbs=1;
	}
	
	sei();


}

ISR(USART0_UDRE_vect){ //ISR para extraer dato de la cola en UART0
   if(!IS_BUFFER_EMPTY(tx_buffer0)) {
        UDR0 = tx_buffer0.buffer[tx_buffer0.out_idx];
        INC(tx_buffer0.out_idx);
		        // Verificamos si el buffer tiene datos
        if (!IS_BUFFER_EMPTY(tx_buffer0)) {
            UCSR0B |= (1 << UDRIE0);  // la interrupcion de transmision se habilitada
        }
    } else {
        UCSR0B &= ~(1 << UDRIE0);  //Si esta vacio, deshabilita la interrupcion de transmision 
    }
}

ISR(USART0_RX_vect){  //ISR para enviar dato de la cola en UART0
    char data = UDR0;
    if (!IS_BUFFER_FULL(rx_buffer0)) {
        rx_buffer0.buffer[rx_buffer0.in_idx] = data;
        INC(rx_buffer0.in_idx);
    }
}

ISR(USART1_UDRE_vect){ //ISR para extraer dato de la cola en UART1
   if(!IS_BUFFER_EMPTY(tx_buffer1)) {
        UDR1 = tx_buffer1.buffer[tx_buffer1.out_idx];
        INC(tx_buffer1.out_idx);
		        // Verificamos si el buffer tiene datos
        if (!IS_BUFFER_EMPTY(tx_buffer1)) {
            UCSR1B |= (1 << UDRIE1);  // la interrupcion de transmision se habilitada
        }
    } else {
        UCSR1B &= ~(1 << UDRIE1);  //Si esta vacio, deshabilita la interrupcion de transmision 
    }
}

ISR(USART1_RX_vect){ //ISR para enviar dato de la cola en UART1
    char data = UDR1;
    if (!IS_BUFFER_FULL(rx_buffer1)) {
        rx_buffer1.buffer[rx_buffer1.in_idx] = data;
        INC(rx_buffer1.in_idx);
    }
}

ISR(USART2_UDRE_vect){ //ISR para extraer dato de la cola en UART2
  if(!IS_BUFFER_EMPTY(tx_buffer2)) {
        UDR2 = tx_buffer2.buffer[tx_buffer2.out_idx];
        INC(tx_buffer2.out_idx);
		        // Verificamos si el buffer tiene datos
        if (!IS_BUFFER_EMPTY(tx_buffer2)) {
            UCSR2B |= (1 << UDRIE2);  // la interrupcion de transmision se habilitada
        }
    } else {
        UCSR2B &= ~(1 << UDRIE2);  //Si esta vacio, deshabilita la interrupcion de transmision 
    }
}

ISR(USART2_RX_vect){ //ISR para enviar dato de la cola en UART2
    char data = UDR2;
    if (!IS_BUFFER_FULL(rx_buffer2)) {
        rx_buffer2.buffer[rx_buffer2.in_idx] = data;
        INC(rx_buffer2.in_idx);
    }
}

ISR(USART3_UDRE_vect){ //ISR para extraer dato de la cola en UART3
	if(!IS_BUFFER_EMPTY(tx_buffer3)) {
        UDR3 = tx_buffer3.buffer[tx_buffer3.out_idx];
        INC(tx_buffer3.out_idx);
		        // Verificamos si el buffer tiene datos
        if (!IS_BUFFER_EMPTY(tx_buffer3)) {
            UCSR3B |= (1 << UDRIE3);  // la interrupcion de transmision se habilitada
        }
    } else {
        UCSR3B &= ~(1 << UDRIE3);  //Si esta vacio, deshabilita la interrupcion de transmision 
    }
}

ISR(USART3_RX_vect){  //ISR para enviar dato de la cola en UART3
    char data = UDR3;
    if (!IS_BUFFER_FULL(rx_buffer3)) {
        rx_buffer3.buffer[rx_buffer3.in_idx] = data;
        INC(rx_buffer3.in_idx);
    }
}

void UART_puts(uint8_t com, char *str){
	while(*str){
		UART_putchar(com,*str);
		str++;
	}
}

void UART_putchar(uint8_t com,char data){
 
	switch(com){
		case 0:
			while(IS_BUFFER_FULL(tx_buffer0));
			tx_buffer0.buffer[tx_buffer0.in_idx]=data;
			INC(tx_buffer0.in_idx);
			if(!(UCSR0B &(1<<UDRIE0))){
				UCSR0B |= (1<<UDRIE0);
			}
			break;
		case 1:
			while(IS_BUFFER_FULL(tx_buffer1));
			tx_buffer1.buffer[tx_buffer1.in_idx]=data;
			INC(tx_buffer1.in_idx);
			if(!(UCSR1B &(1<<UDRIE1))){
				UCSR1B |= (1<<UDRIE1);
			}
			break;
		case 2:
			while(IS_BUFFER_FULL(tx_buffer2));
			tx_buffer2.buffer[tx_buffer2.in_idx]=data;
			INC(tx_buffer2.in_idx);
			if(!(UCSR2B &(1<<UDRIE2))){
				UCSR2B |= (1<<UDRIE2);
			}
			break;
		case 3:
			while(IS_BUFFER_FULL(tx_buffer3));
			tx_buffer3.buffer[tx_buffer3.in_idx]=data;
			INC(tx_buffer3.in_idx);
			if(!(UCSR3B &(1<<UDRIE3))){
				UCSR3B |= (1<<UDRIE3);
			}
			break;
	
	}
}

uint8_t UART_available(uint8_t com){
	uint8_t estado=0;
	switch(com){
		case 0:
			estado= !IS_BUFFER_EMPTY(rx_buffer0);
			break;
		case 1:
			estado=!IS_BUFFER_EMPTY(rx_buffer1);
			break;
		case 2:
			estado=!IS_BUFFER_EMPTY(rx_buffer2);
			break;
		case 3:
			estado=!IS_BUFFER_EMPTY(rx_buffer3);
			break;
	}
	return estado;
}

char UART_getchar(uint8_t com){
	 char data;
	 switch(com){
	 	case 0:
		     while(IS_BUFFER_EMPTY(rx_buffer0));
			 data=rx_buffer0.buffer[rx_buffer0.out_idx];
			 INC(rx_buffer0.out_idx);
			break;
		case 1:
		     while(IS_BUFFER_EMPTY(rx_buffer1));
			 data=rx_buffer1.buffer[rx_buffer1.out_idx];
			 INC(rx_buffer1.out_idx);
			break;
		case 2:
		     while(IS_BUFFER_EMPTY(rx_buffer2));
			 data=rx_buffer2.buffer[rx_buffer2.out_idx];
			 INC(rx_buffer2.out_idx);
			break;
		case 3:
		     while(IS_BUFFER_EMPTY(rx_buffer3));
			 data=rx_buffer3.buffer[rx_buffer3.out_idx];
			 INC(rx_buffer3.out_idx);
			break;	
	 }
	 return data;
}


void UART_gets(uint8_t com,char *str){
	char caracter;
	uint16_t idx=0;
	while(1){
			caracter=UART_getchar(com); //obtenemos el carac
	
			if(caracter==13){ //enter
				if(idx==0){
				 str[idx]='0';
				 idx++;
				}
				break;
			}else if(caracter==8 || caracter == 127){ //backspace
				if(idx>0){
					UART_putchar(com,'\b');
					UART_putchar(com,' ');
					UART_putchar(com,'\b');
					idx--;
				}
			}else if(idx<19){
				str[idx]=caracter;
				UART_putchar(com,caracter);
				idx++;
			}
	}
	str[idx]='\0';

}

void UART_clrscr(uint8_t com){
	UART_puts(com,"\x1b[2J");
}

void UART_setColor(uint8_t com,uint8_t color){
	char comandoColor[16];
	itoa(color,comandoColor,10);
	
	UART_puts(com,"\033["); //poner el color en  uart
	UART_puts(com,comandoColor);
	UART_puts(com,"m");

}

void UART_gotoxy(uint8_t com,uint8_t x,uint8_t y){
	char buffer[10];
	// Formar la secuencia de escape ANSI
    UART_puts(com,"\x1b[");  // Secuencia de escape inicial
    // Convertir la coordenada Y (fila) a cadena
    itoa(y,buffer, 10);
    UART_puts(com,buffer);           // Enviar el valor de Y
    UART_puts(com,";");  // Separador entre coordenadas
    // Convertir la coordenada X (columna) a cadena
    itoa(x,buffer, 10);
    UART_puts(com,buffer);           // Enviar el valor de X
	UART_puts(com,"H");  // Final de la secuencia

}

void itoa(uint16_t number,char* str,uint8_t base){
    uint16_t i = 0;
	uint16_t copia=i; //paa inverti la cadena
	    //en caso de que el numero sea cero, solo se agrega el carc nulo y se agrega el carac de cero
    if (number == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
	  do {
	    uint32_t residuo = number % base;  //Obtenemos el residuo
	    if (residuo <= 9) {
		    str[i++] = residuo + '0'; // Si el residuo es menor o igual a 9, es un digito
	    } else {
		    str[i++] = residuo - 10 + 'A'; // Si el residuo es mayor que 9, es una letra
	    }
	    number = number/ base;     //Obtenemos el entero
	  } while (number != 0);
	  
	    // Colocamos el  caracter nulo
  		str[i] = '\0';
		//invertimos la cadena resuttante
		uint16_t end = i - 1;
	    while (copia < end) {
	        char temp = str[copia];
	        str[copia] = str[end];
	        str[end] = temp;
	        copia++;
	        end--;
	    }
}

uint16_t atoi(char *str){
	uint16_t dato=0;
   
    while (*str >= '0' && *str <= '9') { //Mientras sea un digito
        dato = dato * 10 + (*str - '0'); //lo convertimos a numeo
        str++;   //avanzamos al siguiente caracter
    }
	return dato;
}

