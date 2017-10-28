/*
 * pinIO.h
 *
 * Created: 9/7/2017 2:08:24 PM
 *  Author: Chris Kolegraff
 *
 *	Description: Easy-use interface for accessing pins
 *   Provides digital read/write functions, {set, clr, toggle, DDR_in, DDR_out, read}
 *  Example:
	PIN_OUTPUT(PIN07); // Set Pin 7 to Output
	PIN_SET(PIN07); // Set Pin 7 to HIGH
 *
 */ 


#ifndef PINIO_H_
#define PINIO_H_

#include "defines.h"

#define HIGH 1
#define LOW 0

#define OBLED PORTB, DDRB, PINB, PB7

// Analog Pins
#define PINA00 PORTF, DDRF, PINF, PF0
#define PINA01 PORTF, DDRF, PINF, PF1
#define PINA02 PORTF, DDRF, PINF, PF2
#define PINA03 PORTF, DDRF, PINF, PF3
#define PINA04 PORTF, DDRF, PINF, PF4
#define PINA05 PORTF, DDRF, PINF, PF5
#define PINA06 PORTF, DDRF, PINF, PF6
#define PINA07 PORTF, DDRF, PINF, PF7
#define PINA08 PORTK, DDRK, PINK, PK0
#define PINA09 PORTK, DDRK, PINK, PK1
#define PINA10 PORTK, DDRK, PINK, PK2
#define PINA11 PORTK, DDRK, PINK, PK3
#define PINA12 PORTK, DDRK, PINK, PK4
#define PINA13 PORTK, DDRK, PINK, PK5
#define PINA14 PORTK, DDRK, PINK, PK6
#define PINA15 PORTK, DDRK, PINK, PK7

// Digital Pins
#define PIN00 PORTE, DDRE, PINE, PE0
#define PIN01 PORTE, DDRE, PINE, PE1
#define PIN02 PORTE, DDRE, PINE, PE4
#define PIN03 PORTE, DDRE, PINE, PE5
#define PIN04 PORTG, DDRG, PING, PG5
#define PIN05 PORTE, DDRE, PINE, PE3
#define PIN06 PORTH, DDRH, PINH, PH3
#define PIN07 PORTH, DDRH, PINH, PH4
#define PIN08 PORTH, DDRH, PINH, PH5
#define PIN09 PORTH, DDRH, PINH, PH6
#define PIN10 PORTB, DDRB, PINB, PB4
#define PIN11 PORTB, DDRB, PINB, PB5
#define PIN12 PORTB, DDRB, PINB, PB6
#define PIN13 PORTB, DDRB, PINB, PB7
#define PIN14 PORTJ, DDRJ, PINJ, PJ1
#define PIN15 PORTJ, DDRJ, PINJ, PJ0
#define PIN16 PORTH, DDRH, PINH, PH1
#define PIN17 PORTH, DDRH, PINH, PH0
#define PIN18 PORTD, DDRD, PIND, PD3
#define PIN19 PORTD, DDRD, PIND, PD2
#define PIN20 PORTD, DDRD, PIND, PD1
#define PIN21 PORTD, DDRD, PIND, PD0
#define PIN22 PORTA, DDRA, PINA, PA0
#define PIN23 PORTA, DDRA, PINA, PA1
#define PIN24 PORTA, DDRA, PINA, PA2
#define PIN25 PORTA, DDRA, PINA, PA3
#define PIN26 PORTA, DDRA, PINA, PA4
#define PIN27 PORTA, DDRA, PINA, PA5
#define PIN28 PORTA, DDRA, PINA, PA6
#define PIN29 PORTA, DDRA, PINA, PA7
#define PIN30 PORTC, DDRC, PINC, PC7
#define PIN31 PORTC, DDRC, PINC, PC6
#define PIN32 PORTC, DDRC, PINC, PC5
#define PIN33 PORTC, DDRC, PINC, PC4
#define PIN34 PORTC, DDRC, PINC, PC3
#define PIN35 PORTC, DDRC, PINC, PC2
#define PIN36 PORTC, DDRC, PINC, PC1
#define PIN37 PORTC, DDRC, PINC, PC0
#define PIN38 PORTD, DDRD, PIND, PD7
#define PIN39 PORTG, DDRG, PING, PG2
#define PIN40 PORTG, DDRG, PING, PG1
#define PIN41 PORTG, DDRG, PING, PG0
#define PIN42 PORTL, DDRL, PINL, PL7
#define PIN43 PORTL, DDRL, PINL, PL6
#define PIN44 PORTL, DDRL, PINL, PL5
#define PIN45 PORTL, DDRL, PINL, PL4
#define PIN46 PORTL, DDRL, PINL, PL3
#define PIN47 PORTL, DDRL, PINL, PL2
#define PIN48 PORTL, DDRL, PINL, PL1
#define PIN49 PORTL, DDRL, PINL, PL0
#define PIN50 PORTB, DDRB, PINB, PB3
#define PIN51 PORTB, DDRB, PINB, PB2
#define PIN52 PORTB, DDRB, PINB, PB1
#define PIN53 PORTB, DDRB, PINB, PB0

#define GETPORT(w, x, y, z) w
#define GETDDR(w, x, y, z) x
#define GETPIN(w, x, y, z) y
#define GETNUM(w, x, y, z) z

#define SET(ADDR, BIT) (ADDR |= (1 << BIT))
#define CLR(ADDR, BIT) (ADDR &= ~(1 << BIT))
#define TOGGLE(ADDR, BIT) (ADDR ^= (1 << BIT))
#define READ(ADDR, BIT) (ADDR & (1 << BIT))

#define PIN_SET(pin) SET(GETPORT(pin), GETNUM(pin))
#define PIN_CLR(pin) CLR(GETPORT(pin), GETNUM(pin))
#define PIN_TOGGLE(pin) TOGGLE(GETPORT(pin), GETNUM(pin))
#define PIN_OUTPUT(pin) SET(GETDDR(pin), GETNUM(pin))
#define PIN_INPUT(pin) CLR(GETDDR(pin), GETNUM(pin))
#define PIN_READ(pin) READ(GETPIN(pin), GETNUM(pin))

#endif /* PINIO_H_ */