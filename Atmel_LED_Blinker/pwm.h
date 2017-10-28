/*
 * pwm.h
 *
 * Created: 9/7/2017 8:03:45 PM
 *  Author: Chris Kolegraff
 *
 *  Description: Interface for using the timers.
 *    Provides initializing functions and a function to set the duty cycle
 *    Currently timers are set to only operate @ 100Hz
 *
 *    TODO: Add functions to change frequency of timers
 *          Allow for dynamically changing the ISR of each timer
 *			  (ISR would call a function ptr, possibly unsafe?)
 */ 


#ifndef PWM_H_
#define PWM_H_

#include "pinIO.h"

#define TIMER1_TOP 0x4E1F // 100Hz
#define TIMER3_TOP 0x4E1F
#define TIMER4_TOP 0x4E1F

#define TIMER1_OUTPUT_PIN PIN11
#define TIMER3_OUTPUT_PIN PIN05
#define TIMER4_OUTPUT_PIN PIN06

void init16BitTimer1();
void init16BitTimer3();
void init16BitTimer4();
void setDutyCycle1(int32_t percent);
void setDutyCycle3(int32_t percent);
void setDutyCycle4(int32_t percent);

#endif /* PWM_H_ */