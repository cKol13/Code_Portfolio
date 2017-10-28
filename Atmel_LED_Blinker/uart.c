/*
 * uart.c
 *
 * Created: 9/6/2017 9:24:20 PM
 *  Author: Chris Kolegraff
 */ 

#include "uart.h"

inline uint32_t div10(uint32_t num){ // Constant represents (1/10) * 2^35
	return ((uint64_t)num * 0xCCCCCCCD) >> 35;
}

inline uint32_t mod10(uint32_t num){
	return num - 10 * div10(num);
}

uint8_t* intToStr(uint8_t* buff, int32_t num){
	register uint32_t i = (num < 0) ? 1 : 0; // If negative, i = 1, else 0
	// If the number was negative, set first character to '-'
	if(i){
		num = -num;
		buff[0] = '-';
	}
	
	if(num){
		uint8_t tBuff[MAX_BUFFER_SIZE];
		register uint32_t j = 0;
		// Convert integer to string, result is backwards
		do{
			tBuff[j++] = '0' + mod10(num);
			num = div10(num);
		}while(num);
		
		while(j > 0) buff[i++] = tBuff[--j]; // Reverse the string
	}
	else{
		buff[i++] = '0';
	}
	
	buff[i] = 0;
	return buff;
}

void serialWriteByte(uint8_t byte){
	while(!(UCSR0A & (1 << UDRE0))); // Wait to finish transmitting
	UDR0 = byte;
}

void serialWriteString(uint8_t *sentence){
	register uint32_t i = 0;
	while(sentence[i] != '\0')
		serialWriteByte(sentence[i++]);
}

void initUSART(){
	// USART data sheet page 200 atmega 2560
	UBRR0H = (uint8_t)(BAUD_PRESCALE>>8);
	UBRR0L = (uint8_t) BAUD_PRESCALE;
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
}

