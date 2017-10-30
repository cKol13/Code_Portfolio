/*
File: ticker.h
Author: Chris Kolegraff
Description:
Provides the ticker interface to the user. The user can first call tickerInit()
to initialize the delay, then call wait() to pause the program for a set
amount of time.
*/

// Initialize the delay function with a delay value in seconds and microseconds
void tickerInit(int sec, int usec);

// Call the delay function and wait for the specified amount of time.
void wait();
