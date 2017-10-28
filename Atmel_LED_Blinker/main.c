/*
 * main.c
 *
 * Created: 9/6/2017 7:49:35 PM
 *  Author: Chris Kolegraff
 *
 *  Description: This mini-project is just to test various functions for use in other projects
 *     later on. This project will blink 3 LED's on and off using PWM to alter the brightness
 *     of the LEDs. The LEDs flash with different periods and offsets from the first LED.
 *     The program will also read in an analog and digital pin, and send those pin values out
 *     to a UART console.
 *
 *     Embedded systems concepts used in this project: 
 *				UART, PWM, ADC, GPIO, Interrupts, Timers
 *
 *  Note: Some functions/Interfaces have been copied over from the Embedded Project Code folder
 *        (ArduinoCode.c) and adapted to fit the purposes of this project
 */ 

#include "adc.h"
#include "uart.h"
#include "pinIO.h"
#include "pwm.h"

void init();

int main(void){
	init();
	
	uint8_t str[50];
	int32_t num = 0;
	
	init16BitTimer1(); // Slowly turn LED on/off with T = 2s
	_delay_ms(200); 
	init16BitTimer3(); // Slowly turn LED on/off with T = 4s, offset of 200ms
	_delay_ms(400);
	init16BitTimer4(); // Slowly turn LED on/off with T = 8s, offset of 600ms
	
    while (1){
		// Read analog pin and write value out to console
		num = analogRead(0);
		serialWriteString(intToStr(str, num));
		
		// Read pin state, then write state out to console
		num = PIN_READ(PIN28);
		serialWriteString(intToStr(str, num));
		serialWriteString((uint8_t*)"Test\n");
		
		// Turn On-Board LED on/off based on user button input
		if(num) PIN_SET(OBLED); 
		else    PIN_CLR(OBLED);
		
		_delay_ms(100);
    }
}

void init(){
	// Set OBLED to output
	PIN_OUTPUT(OBLED);
	
	// Set DP28 to input, pullup enabled
	PIN_INPUT(PIN28);
	PIN_SET(PIN28);
	
	// Set Analog 0 to input
	PIN_INPUT(PINA00);
	
	initUSART();
	initADC();
}