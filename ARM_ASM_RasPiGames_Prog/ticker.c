/*
File: ticker.c
Author: Chris Kolegraff
Description:
Contains definitions for the ticker interface. The waiting function uses
sigpause(SIGALARM) to pause execution and save CPU.
*/

#include "ticker.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

struct itimerval timeout;
struct sigaction action;

void alarm_func(int nothing){} // Used by OS for handling the interrupts
void sigpause(); // Need to include this line to turn off a warning

/*
Function: tickerInit
Author: Chris Kolegraff
Description:
Initialize the timeout struct with the given data, then setup the OS waiting
systems to pause the program.
*/
void tickerInit(int sec, int usec)
{
    timeout.it_interval.tv_sec = sec;
    timeout.it_interval.tv_usec = usec;
    timeout.it_value.tv_sec = sec;
    timeout.it_value.tv_usec = usec;
    
    action.sa_handler = alarm_func;
    sigemptyset(&action.sa_mask);
    action.sa_flags=SA_RESTART; // probably not needed
    sigaction(SIGALRM, &action, NULL);
    
    setitimer(ITIMER_REAL, &timeout, NULL);
    return;
}


/*
Function: wait
Author: Chris Kolegraff
Description:
Call sigpause(SIGALARM) to wait the set amount of time.
*/
void wait()
{
    sigpause(SIGALRM);
    return;
}
