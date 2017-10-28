/*
 * pwm.c
 *
 * Created: 9/7/2017 8:05:50 PM
 *  Author: Chris Kolegraff
 */ 

#include "pwm.h"


#define TIMER1_MAX TIMER1_TOP
#define TIMER1_MIN 0

#define TIMER3_MAX TIMER3_TOP
#define TIMER3_MIN 0

#define TIMER4_MAX TIMER4_TOP
#define TIMER4_MIN 0

#define TIMER1_RANGE (TIMER1_MAX - TIMER1_MIN)
#define TIMER3_RANGE (TIMER3_MAX - TIMER3_MIN)
#define TIMER4_RANGE (TIMER4_MAX - TIMER4_MIN)

// Method for finding the constant value can be found here:
// https://embeddedgurus.com/stack-overflow/2009/06/division-of-integers-by-constants/
inline int32_t div100(int32_t num){ // Constant represents (1/100) * 2^38
	return ((int64_t)num * 0xA3D70A3E) >> 38;
}

// Counts [0,99], updates at a frequency of 100Hz
// Reaches top/bottom in 1 second, period = 2s
ISR (TIMER1_COMPA_vect){
	static int32_t count = 0;
	static int8_t dir = 1;
	
	setDutyCycle1(count);
	count += dir;
	if(count >= 99) dir = -1;
	else if(count <= 0) dir = 1;
}

// Counts [0, 199], updates at a frequency of 100Hz
// Reaches top/bottom in 2 seconds, period = 4s
ISR (TIMER3_COMPA_vect){
	static int32_t count = 0;
	static int8_t dir = 1;
	
	setDutyCycle3(count >> 1); // Divide by 2
	count += dir;
	if(count >= 199) dir = -1;
	else if(count <= 0) dir = 1;
}

// Counts [0, 399], updates at a frequency of 100Hz
// Reaches top/bottom in 4 seconds, period = 8s
ISR (TIMER4_COMPA_vect){
	static int32_t count = 0;
	static int8_t dir = 1;
	
	setDutyCycle4(count >> 2); // Divide by 4
	count += dir;
	if(count >= 399) dir = -1;
	else if(count <= 0) dir = 1;
}
	
void init16BitTimer4(){
	cli();
	PIN_OUTPUT(TIMER4_OUTPUT_PIN);
	
	// Fast PWM Mode, TOP = ICR4, Update of OCR4A at BOTTOM, TOV4 Flag Set on TOP
	// Set OC4A on compare match, prescalar of 8, non-inverting mode
	TCCR4A |= (1 << WGM41) | (1 << COM4A1);
	TCCR4B |= (1 << WGM43) | (1 << WGM42) | (1 << CS41);
	
	TCCR4C = 0;
	TCNT4 = 0;
	
	ICR4 = TIMER4_TOP; // TOP
	
	setDutyCycle4(0);
	TIMSK4 |= (1 << OCIE4A); // Timer/Counter, Output Compare A Match Interrupt Enable
	sei();
}

void init16BitTimer3(){
	cli();
	PIN_OUTPUT(TIMER3_OUTPUT_PIN);
	
	// Fast PWM Mode, TOP = ICR3, Update of OCR3A at BOTTOM, TOV3 Flag Set on TOP
	// Set OC3A on compare match, prescalar of 8, non-inverting mode
	TCCR3A |= (1 << WGM31) | (1 << COM3A1);
	TCCR3B |= (1 << WGM33) | (1 << WGM32) | (1 << CS31);
	
	TCCR3C = 0;
	TCNT3 = 0;
	
	ICR3 = TIMER3_TOP; // TOP
	
	setDutyCycle3(0);
	TIMSK3 |= (1 << OCIE3A); // Timer/Counter, Output Compare A Match Interrupt Enable
	sei();
}

void init16BitTimer1(){
	
	cli(); // Disable interrupts
	PIN_OUTPUT(TIMER1_OUTPUT_PIN);
	
	
	// Fast PWM Mode, TOP = ICR1, Update of OCR1A at BOTTOM, TOV1 Flag Set on TOP
	// Set OC1A on compare match, prescalar of 8, non-inverting mode
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11);
	
	TCCR1C = 0;
	TCNT1 = 0;
	
	ICR1 = TIMER1_TOP; // TOP #define TIMER1_TOP 0x4E1F // 100Hz
	
	setDutyCycle1(0);
	TIMSK1 |= (1 << OCIE1A); // Timer/Counter, Output Compare A Match Interrupt Enable
	sei(); // Enable interrupts
}

void setDutyCycle1(int32_t percent){
	if(percent >= 100) OCR1A = TIMER1_MAX;
	else if(percent <= 0) OCR1A = TIMER1_MIN;
	else OCR1A = div100(TIMER1_RANGE * percent);
}

void setDutyCycle3(int32_t percent){
	if(percent >= 100) OCR3A = TIMER3_MAX;
	else if(percent <= 0) OCR3A = TIMER3_MIN;
	else OCR3A = div100(TIMER3_RANGE * percent);
}

void setDutyCycle4(int32_t percent){
	if(percent >= 100) OCR4A = TIMER4_MAX;
	else if(percent <= 0) OCR4A = TIMER4_MIN;
	else OCR4A = div100(TIMER4_RANGE * percent);
}