#ifndef _ADC_H
#define _ADC_H

#include <inttypes.h>

/* Esta función inicializa para 10 bits de resolución y habilita el ADC del microcontrolador de
   forma genérica. Encontrar el desplazamiento (offset) de la medición y almacenarla. */
void ADC_Ini ( void );

/* Está función lo que realiza es una lectura del ADC usando el canal correcto y retornando el
   valor de 10 bits acorde a la aplicación (ver Figura 2), compensando el desplazamiento de la medición. */
uint16_t ADC_Read( uint8_t channel );

#endif