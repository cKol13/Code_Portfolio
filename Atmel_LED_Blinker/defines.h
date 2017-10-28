/*
 * defines.h
 *
 * Created: 9/6/2017 9:16:27 PM
 *  Author: Chris Kolegraff
 *
 *  Description: Defines to configure the AtMega 2560 board
 */ 


#ifndef DEFINES_H_
#define DEFINES_H_


#define F_CPU 16000000L
#define USART_BAUDRATE 57600
#define BAUD_PRESCALE ((F_CPU / (USART_BAUDRATE * 16UL)) - 1)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#endif /* DEFINES_H_ */