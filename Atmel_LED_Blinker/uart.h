/*
 * uart.h
 *
 * Created: 9/6/2017 9:21:49 PM
 *  Author: Chris Kolegraff
 *  
 *  Description: UART interface for communicating with a UART device.
 *     Provides a function to convert an integer to a c string.
 *     
 *  TODO: Implement printf functionality? (vargs) Maybe too slow?
 *
 */ 


#ifndef UART_H_
#define UART_H_

#include MAX_BUFFER_SIZE 50

#include "defines.h"

void serialWriteString(uint8_t *sentence);
void initUSART();

// Convert an int to string, returns ptr to original input buffer
uint8_t* intToStr(uint8_t* buff, int32_t num); 




#endif /* UART_H_ */