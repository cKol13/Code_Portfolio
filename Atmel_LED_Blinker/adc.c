/*
 * adc.c
 *
 * Created: 9/7/2017 8:10:30 PM
 *  Author: Chris Kolegraff
 */ 

#include "adc.h"

void initADC(){
	ADCSRA = 0;
	ADMUX = 0;
	
	// Configure the ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADCSRA |= (1 << ADEN);
	ADCSRA |= (1 << ADSC);
	
	ADMUX |= (1 << REFS0);
}

int32_t analogRead(uint8_t pin){// Analog 0-15 only{
	if (pin > 15) return -1;
	
	ADMUX &= 0xF0; // Clear last 4 bits
	ADMUX |= pin; // Choose pin to read
	
	ADCSRA |= (1 << ADEN);
	ADCSRA |= (1 << ADSC);
	
	// Need to delay or else quick consecutive analog reads are misordered
	_delay_ms(1); 
	
	return ADC;
}