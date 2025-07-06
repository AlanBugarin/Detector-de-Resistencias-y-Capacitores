// Prototypes
#include <avr/io.h>
#include <avr/interrupt.h>
#ifndef UART_H
#define UART_H
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
void UART_clrscr(uint8_t com);
void UART_setColor(uint8_t com,uint8_t color);
void UART_gotoxy(uint8_t com,uint8_t x,uint8_t y);

#define YELLOW  33 // Fixme 
#define GREEN   32 // Fixme 
#define BLUE    34 // Fixme 
#define BLACK   30 

// Utils
void itoa(uint16_t number,char* str,uint8_t base); 
uint16_t atoi(char *str);

#define BUFFER_SIZE 64
#define MOD(n) ((n) & (BUFFER_SIZE-1))
#define INC(n) n=MOD((n)+1)
#define IS_BUFFER_EMPTY(buf) (buf.in_idx==buf.out_idx)
#define IS_BUFFER_FULL(buf) (buf.in_idx==MOD((buf.out_idx)-1))

typedef struct {
 	 char buffer[BUFFER_SIZE];
	 volatile unsigned char in_idx;
	 volatile unsigned char out_idx;
 }ring_buffer_t;

 ring_buffer_t tx_buffer0; //Salida
 ring_buffer_t rx_buffer0; //Entrada
 
 ring_buffer_t tx_buffer1;
 ring_buffer_t rx_buffer1;

 ring_buffer_t tx_buffer2;
 ring_buffer_t rx_buffer2;
 
 ring_buffer_t tx_buffer3;
 ring_buffer_t rx_buffer3;
#endif
