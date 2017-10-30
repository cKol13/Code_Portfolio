/*
File: notes.h
Author: Chris Kolegraff
Description:
Provides the notes interface to the user. The user can play a note by passing
in one of the defines found in Consts.S into the play_note() and stop_note() 
functions to play and stop playing specific notes. There is only 1 buzzer in
the kit, so only 1 note can be played at a time because these functions
implement a software PWM. The function uses pthreads to open a new thread to
run the software PWM from.
*/

// Begin playing a specific note (E4, C#4, A3, E3, or a raspberry)
void play_note(int note);

// Stop playing a specific note
void stop_note(int note);

// Stop playing all notes
void stop_all_notes();
