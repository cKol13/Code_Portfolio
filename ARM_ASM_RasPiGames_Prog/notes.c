/*
File: notes.c
Author: Chris Kolegraff
Description:
Contains definitions for the notes interface. These functions use pthreads to
create a software PWM that produces a note (E4, C#4, A3, or E3) using a piezo
buzzer.
*/

// Defines to wait x nanoseconds
#define E4_50 310000
#define CS4_50 385000
#define A3_50 491000
#define E3_50 680000
#define RS_50 2000000

#include "notes.h"
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

// Can't use 'ticker' delay in these functions, otherwise program stalls
struct timespec ts; // Used to add another delay function

pthread_t tids[5]; // Holds pthread information
int stops[5] = {1, 1, 1, 1, 1}; // Used to stop a particular thread
const int duties[5] = {E4_50, CS4_50, A3_50, E3_50, RS_50}; // 50% duty cycles

void buzzer_on(); // ASM function declaration to turn buzzer on
void buzzer_off(); // ASM function delcaration to turn buzzer off


/*
Function: note_thread
Author: Chris Kolegraff
Description:
This function will be used in a pthread. It will initalize a different
timer that will allow the thread to pause without changing the 'ticker' 
interface's pause function.
*/
void *note_thread(void *args)
{
    int note = (int) args;
    if(note < 0)note = 0;
    if(note > 4) note = 4;
    
    ts.tv_sec = 0;
    ts.tv_nsec = duties[note];
    
    while(stops[note] != 1)
    {
        buzzer_on();
        nanosleep(&ts, NULL);
        buzzer_off();
        nanosleep(&ts, NULL);
    }
    
    return NULL;
}

/*
Function: play_note
Author: Chris Kolegraff
Description:
Begins playing a note by starting a pthread and using note_thread().
It will pass in the given note as an argument so that note_thread() will know
which note to play.
*/
void play_note(int note)
{
    if(note < 0) note = 0;
    if(note > 4) note = 4;
    stops[note] = 0;
    pthread_create(&tids[note], NULL, note_thread, (void*) note);
}

/*
Function: stop_note
Author: Chris Kolegraff
Description:
Stops a note playing by setting the stop value to 1 and called the pthread join
function which will wait until note_thread() returns from executing in 
the pthread.
*/
void stop_note(int note)
{
    if(note < 0) note = 0;
    if(note > 4) note = 4;
    stops[note] = 1;
    pthread_join(tids[note], NULL);
    buzzer_off();
}

/*
Function: stop_all_notes
Author: Chris Kolegraff
Description:
Stops all notes from playing by setting all stop values to 1. Used mostly for
when we can't easily know which note is playing. So iterating through all notes
to see which are playing helps keep the code simpler.
*/
void stop_all_notes()
{
    int i;
    for(i = 0; i < 5; i++)
    {
        if(stops[i] == 0)
        {
            stops[i] = 1;
            pthread_join(tids[i], NULL);
            buzzer_off();
        }
    }
}

