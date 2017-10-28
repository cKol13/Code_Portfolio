#define F_CPU 16000000 // Specify oscillator frequency
#define USART_BAUDRATE 57600
#define BAUD_PRESCALE ((F_CPU / (USART_BAUDRATE * 16UL)) - 1)

// DC PWM Limits
#define PWM_UPPER 0xC000
#define PWM_LOWER 0x2000

// DC Motor Pins
#define MOTOR_B_DIR_PORT PORTB // Pin 51, direction B
#define MOTOR_B_DIR_DDR DDRB // We had to pull out some pins from the motor shield
#define MOTOR_B_DIR_PIN 2    // and rewire them to other pins on the Arduino for it to work
#define MOTOR_BRAKE_PORT PORTG
#define MOTOR_BRAKE_DDR DDRG
#define MOTOR_BRAKE_A_PIN 2
#define MOTOR_BRAKE_B_PIN 0

// Sensor pins
#define TEMPC_PIN 2 // Analog
#define LIGHT_PIN 3

// Sonar info
#define SONAR_PIN 7
#define SONAR_DDR DDRC
#define SONAR_PORT PORTC
#define SONAR_IN PINC

// Flash light info
#define FLASH_PIN 5
#define FLASH_DDR DDRC
#define FLASH_PORT PORTC
#define FLASH_THRESH 50
#define DELTA_LIGHT 3 // Prevent flashlight flicker at value ~= FLASH_THRESH

// Camera motion limits
#define UD_UPPER_LIMIT 30
#define UD_LOWER_LIMIT 10
#define LR_RIGHT_LIMIT 0xD000
#define LR_LEFT_LIMIT 0x8000

// DC motor limits
#define RIGHT_DC_UPPER 0xBE00
#define RIGHT_DC_LOWER 0x1E00
#define LEFT_DC_UPPER 0xC000
#define LEFT_DC_LOWER 0x2000

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "Arduino.h"

unsigned int DC_PWM = (PWM_UPPER + PWM_LOWER) / 2;
const unsigned int DC_CALIB = 0x200;
unsigned int percentPWM = 0;
unsigned int FlashLightValue = 0;


int myMap(long val, long fromLow, long fromHigh, long toLow, long toHigh)
{
	// Based on Arduino's map() function
	if (val > fromHigh)
		val = fromHigh;
	else if (val < fromLow)
		val = fromLow;
	
	return (val - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

// SERIAL FUNCTIONS
unsigned char SerialReadByte()
{
	while(!(UCSR0A & (1 << RXC0))); // Wait to finish reading serial
	return UDR0;
}
void SerialWriteByte(char byte)
{
	while(!(UCSR0A & (1<<UDRE0)));// Wait to finish transmitting
	UDR0 = byte;
}
void SerialFlush()
{
	unsigned char temp;
	while (UCSR0A & (1<<RXC0))
	{
		temp = UDR0;
		temp = temp; // Gets rid of warning
	}
}
void SerialWriteString(char sentence[])
{
	int i = 0;
	while(sentence[i] != '\0')
	{
		SerialWriteByte(sentence[i++]);
	}
}
char* SerialReadString(char* str)
{
	char byte = ' ';
	int bytesRead = 0;
	
	while(byte != '\r' && byte != '\n' && byte != '\0')
	{
		byte = SerialReadByte();
		str[bytesRead++] = byte;
	}
	str[bytesRead] = '\0';
	
	SerialFlush();
	return str;
}
void initUSART()
{ 
	// USART data sheet page 200
	UBRR0H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR0L = (unsigned char)BAUD_PRESCALE;
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
}

// ADC FUNCTIONS
void initADC()
{
	ADCSRA = 0;
	ADMUX = 0;
	
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADCSRA |= (1 << ADEN);
	ADCSRA |= (1 << ADSC);
	
	ADMUX |= (1 << REFS0);
}
int AnalogRead(char pin)// Analog 0-15 only
{
	if (pin > 15 || pin < 0)
	{
		return -1;
	}
	
	ADMUX &= 0xF0; // Clear last 4 bits
	ADMUX |= pin; // Choose pin to read
	
	ADCSRA |= (1 << ADEN);
	ADCSRA |= (1 << ADSC);
	
	_delay_ms(5); // Need to delay or else pins are read in wrong order
	
	return ADC;
}

void setMotors(int en)
{
	cli();
	if(en == 1)
	{
		TCCR0B |= (1<<CS02) | (1<<CS00);
		TCCR4B |= (1<<CS40);
	}
	else
	{
		TCCR0B &= ~((1<<CS02)|(1<<CS00));
		TCCR4B &= ~(1<<CS40);
	}
	
	sei();
}

// MOTOR FUNCTIONS
void initServoMotors()
{
	DDRB = 0xFF;
	DDRG = 0xFF;
	DDRE = 0xFF;
	DDRH = 0xFF;
	PORTG = 0;
	PORTB = 0;
	PORTE = 0;
	PORTH = 0;
	sei();
	
	
	TCCR3A = 0;
	TCCR3B = 0;
	TCCR4A = 0;
	TCCR4B = 0;
	
	TCCR4A |= (1<<WGM41) | (1<<COM4A0) | (1<<COM4A1);
	TCCR4B |= (1<<WGM43) | (1<<WGM42) | (1<<CS40);
	
	TCCR4C = 0;
	TCNT4 = 0;
	OCR4A = 0xA000;
	ICR4 = 0xFFFF;
	
	TCCR0A = 0xA3;
	TCCR0B = (1<<WGM02) | (1<<CS02) | (1<<CS00);
	TCNT0 = 0;
	OCR0A = 156;
	OCR0B = 20;
	_delay_ms(1000);
	setMotors(0);
	
	sei();
}
void initDCMotors()
{
	DDRB |= (1<<PB2) | (1<<PB5) | (1<<PB6) | (1<<PB7);
	DDRE |= (1<<PE5);
	MOTOR_BRAKE_DDR |= (1<<MOTOR_BRAKE_B_PIN) | (1<<MOTOR_BRAKE_B_PIN);
	MOTOR_BRAKE_PORT &= ~((1<<MOTOR_BRAKE_B_PIN) | (1<<MOTOR_BRAKE_B_PIN));
	PORTB = 0;
	PORTE = 0;
	TCCR1A |= (1<<WGM11) | (1<<COM1A0) | (1<<COM1A1);
	TCCR1B |= (1<<WGM13) | (1<<WGM12) | (1<<CS10);
	TCCR3A |= (1<<WGM31) | (1<<COM3C0) | (1<<COM3C1);
	TCCR3B |= (1<<WGM33) | (1<<WGM32) | (1<<CS30);
	
	cli();
	TCCR1C = 0;
	TCCR3C = 0;
	TCNT1 = 0;
	TCNT3 = 0;
	PORTB |= (1<<PB7); // Go forward left
	PORTB |= (1<<PB6); // Go forward right
	OCR1A = 0xFFFF; // Control speed
	OCR3C = 0xFFFF;
	ICR1 = 0xFFFF;
	ICR3 = 0xFFFF;
	sei();
}

// ROBOT CONTROL FUNCTIONS
void handleDCMotor(char state)
{
	unsigned int leftPWM = 0;
	unsigned int rightPWM = 0;
	int direction = 0;
	unsigned int brakes = 0;
	cli();
	switch (state)
	{
		case '0':
			direction = (1<<PB2) | (1<<PB6);
			leftPWM = 0xFFFF;
			rightPWM = 0xFFFF;
			brakes |= (1<<MOTOR_BRAKE_B_PIN) | (1<<MOTOR_BRAKE_A_PIN);
			break;
		case '1':
			direction = (1<<PB6) | (1<<PB2);
			leftPWM = DC_PWM;
			rightPWM = DC_PWM + DC_CALIB;
			break;
		case '2':
			leftPWM = DC_PWM;
			rightPWM = DC_PWM + DC_CALIB;
			break;
		case '3':
			direction = (1<<PB6);
			leftPWM = DC_PWM;
			rightPWM = DC_PWM + DC_CALIB;
			break;
		case '4':
			direction = (1<<PB2);
			leftPWM = DC_PWM;
			rightPWM = DC_PWM + DC_CALIB;
			break;
		case '5':
			direction = (1<<PB2) | (1<<PB6);
			leftPWM = 0xFFFF;
			rightPWM = DC_PWM + DC_CALIB;
			break;
		case '6':
			direction = (1<<PB2) | (1<<PB6);
			leftPWM = DC_PWM;
			rightPWM = 0xFFFF;
			break;
		case '7':
			leftPWM = DC_PWM;
			rightPWM = 0xFFFF;
			break;
		case '8':
			leftPWM = 0xFFFF;
			rightPWM = (DC_PWM + DC_CALIB);
			break;
		default:
			break;
	}
	if (leftPWM != 0xFFFF)
	{
		if (leftPWM < LEFT_DC_LOWER)
		{
			leftPWM = LEFT_DC_LOWER;
		}
		else if (leftPWM > LEFT_DC_UPPER)
		{
			leftPWM = LEFT_DC_UPPER;
		}
	}
	
	if (rightPWM != 0xFFF)
	{
		if (rightPWM < RIGHT_DC_LOWER)
		{
			rightPWM = RIGHT_DC_LOWER;
		}
		else if (rightPWM > RIGHT_DC_UPPER)
		{
			rightPWM = RIGHT_DC_UPPER;
		}
	}
	
	MOTOR_BRAKE_PORT = brakes;
	PORTB = direction;
	cli();
	OCR1A = leftPWM;
	OCR3C = rightPWM;
	sei();
}
void handleFlashLight(char state)
{
	if (state == 'F' || FlashLightValue < (FLASH_THRESH - DELTA_LIGHT)) // Turn LED on
	{
		FLASH_PORT |= (1<<FLASH_PIN);
	}
	else if (state == '0' && FlashLightValue > (FLASH_THRESH + DELTA_LIGHT)) // Turn LED off
	{
		FLASH_PORT &= ~(1<<FLASH_PIN);
	}
}
void handleServos(char state)
{
	// Servo0 -> Pin 4 LR
	// Servo1 -> Pin 5 UD, servo holding the camera
	int tempLR = 0;
	int tempUD = 0;
	
	if (state >= '0' || state <= '8')
	{
		setMotors(1);
	}
	switch(state)
	{
		case '0':
			setMotors(0);
			break;
		case '1': // Tilt up
			tempUD = OCR0B + 1;
			if(tempUD > UD_UPPER_LIMIT)
				tempUD = UD_UPPER_LIMIT;
			OCR0B = tempUD;
			break;
		case '2': // Tilt down
			tempUD = OCR0B - 1;
			if(tempUD < UD_LOWER_LIMIT)
				tempUD = UD_LOWER_LIMIT;
			OCR0B = tempUD;
			break;
		case '3': // Left
			tempLR = OCR4A - 0x300;
			if(tempLR < LR_LEFT_LIMIT)
				tempLR = LR_LEFT_LIMIT;
			OCR4A = tempLR;
			break;
		case '4': // Right
			tempLR = OCR4A + 0x300;
			if(tempLR > LR_RIGHT_LIMIT)
				tempLR = LR_RIGHT_LIMIT;
			OCR4A = tempLR;
			break;
		case '5': // Up/Left
			tempLR = OCR4A - 0x300;
			tempUD = OCR0B + 1;
			if(tempLR < LR_LEFT_LIMIT)
				tempLR = LR_LEFT_LIMIT;
			if(tempUD > UD_UPPER_LIMIT)
				tempUD = UD_UPPER_LIMIT;
			OCR4A = tempLR;
			OCR0B = tempUD;
			break;
		case '6': // Up/Right
			tempLR = OCR4A + 0x300;
			tempUD = OCR0B + 1;
			if(tempLR > LR_RIGHT_LIMIT)
				tempLR = LR_RIGHT_LIMIT;
			if(tempUD > UD_UPPER_LIMIT)
				tempUD = UD_UPPER_LIMIT;
			OCR4A = tempLR;
			OCR0B = tempUD;
			break;
		case '7': // Down/Left
			tempLR = OCR4A - 0x300;
			tempUD = OCR0B - 1;
			if(tempUD < UD_LOWER_LIMIT)
				tempUD = UD_LOWER_LIMIT;
			if(tempLR < LR_LEFT_LIMIT)
				tempLR = LR_LEFT_LIMIT;
			OCR4A = tempLR;
			OCR0B = tempUD;
			break;
		case '8': // Down/Right
			tempLR = OCR4A + 0x300;
			tempUD = OCR0B - 1;
			if(tempLR > LR_RIGHT_LIMIT)
				tempLR = LR_RIGHT_LIMIT;
			if(tempLR < LR_LEFT_LIMIT)
				tempLR = LR_LEFT_LIMIT;	
			OCR0B = tempUD;
			OCR4A = tempLR;
			
			break;
		default:
			break;
	}
}
void handleDCPWM(char state)
{
	if (state == 'D')
	{
		DC_PWM += 0x100;
		if (DC_PWM > PWM_UPPER)
			DC_PWM = PWM_UPPER;
		percentPWM = myMap(DC_PWM, PWM_LOWER, PWM_UPPER, 100, 0);
	}
	else if (state == 'U')
	{
		DC_PWM -= 0x100;
		if (DC_PWM < PWM_LOWER)
			DC_PWM = PWM_LOWER;
		percentPWM = myMap(DC_PWM, PWM_LOWER, PWM_UPPER, 100, 0);
	}
}
void handleSerialInput(char* str)
{
	handleFlashLight(str[1]);
	handleServos(str[2]);
	handleDCPWM(str[3]);
	handleDCMotor(str[0]);
}

// SENSOR READ FUNCTIONS
float readTemperature()
{
	float volts = (AnalogRead(TEMPC_PIN) * 5.0f) / 1024.0f;
	return (volts - 0.5f) * 100.0f;// In C
}
int readLight()
{
	double lightVoltage = AnalogRead(LIGHT_PIN);
	return myMap(lightVoltage, 0, 1023, 0, 100);
}
int readSonar()
{	
	// Code based off of \Arduino\hardware\arduino\cores\arduino\wiring_pulse.c
	SONAR_DDR |= (1<<SONAR_PIN);// Set to output
	SONAR_PORT &= ~(1<<SONAR_PIN);// Write low
	_delay_us(4);
	SONAR_PORT |= (1<<SONAR_PIN);// Write high
	_delay_us(7);
	SONAR_PORT &= ~(1<<SONAR_PIN);// Low
	
	SONAR_DDR &= ~(1<<SONAR_PIN);// Set to input
	
	// PulseIn() function
	uint8_t bit = (1<<SONAR_PIN);
	unsigned long width = 0;
	unsigned long numloops = 0;
	unsigned long maxloops = microsecondsToClockCycles(1000000L) / 16;
	
	// Wait for previous pulse to end
	while ((SONAR_IN & bit) != 0)
		if(numloops++ >= maxloops)
			return 1;
	
	// Wait for new pulse to start
	while ((SONAR_IN & bit) == 0)
		if(numloops++ >= maxloops)
			return 2;
	
	// Wait for pulse to stop
	while ((SONAR_IN & bit) != 0)
	{
		if(numloops++ >= maxloops)
			return 3;
		width++;
	}
	
	long duration = clockCyclesToMicroseconds(width*20 + 16);
	
	return (duration / 40) / 2;//return cm 40->Calibration
}

void init()
{
	DDRB = 0;
	DDRD = 0;
	PORTB = 0;
	PORTD = 0;
	PORTE = 0;
	FLASH_DDR |= (1<<FLASH_PIN);
	initUSART();
	initADC();
	initServoMotors();
	initDCMotors();
}

char* createSerialMessage(char* buff, char buffers[][15], float* tempC, int* distance)
{
	strcpy(buff, itoa(*tempC, buffers[0], 10));
	strcat(buff, "C,");
	strcat(buff, itoa(FlashLightValue, buffers[1], 10));
	strcat(buff, "%,");
	strcat(buff, itoa(*distance, buffers[2], 10));
	strcat(buff, "cm,");
	strcat(buff, itoa(percentPWM, buffers[3], 10));
	strcat(buff, "%PWM\r\n");
	
	return buff;
}

/*
	Arduino pin placement
	A3. Light
	A2. Temperature
	5. Servo T3A
	6. Servo T4A
	30. Sonar In/Out
	32. Flashlight LED
	
	Don't use:
	2. Servo Timer3B
	3. Servo Timer3C
	7. Servo Timer4B
	8. Servo Timer4C
	11. Servo1A
	12. 
	13. Servo0A
*/


int main()
{
	init();
	
	char serialOutBuffer[30];
	char serialBuffer[10];
	char sensorBuffers[5][15];
	
	float tempC = 0.0f;
	int distance = 0;
	char* recSerial;
	char* serialOut;
	
	percentPWM = myMap(DC_PWM, PWM_LOWER, PWM_UPPER, 0, 100);
	
	while(1)
	{
		recSerial = SerialReadString(serialBuffer);
		if (strlen(recSerial) > 3)
		{
			tempC = readTemperature();
			FlashLightValue = readLight();
			distance = readSonar();
			
			handleSerialInput(recSerial);
			serialOut = createSerialMessage(serialOutBuffer, sensorBuffers, &tempC, &distance);
			SerialWriteString(serialOut);
		}
	}
}