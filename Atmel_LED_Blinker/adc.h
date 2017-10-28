/*
 * adc.h
 *
 * Created: 9/7/2017 8:09:42 PM
 *  Author: Chris Kolegraff
 *
 *  Description: Interface for handling the ADC, includes an init function
 *               and an analog read function, user needs to set the analog
 *               pin to input before called analog read.
 */ 

#ifndef ADC_H
#define ADC_H


#include "defines.h"

void initADC();
int32_t analogRead(uint8_t pin);


#endif /* ADC_H */