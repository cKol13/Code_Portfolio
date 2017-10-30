/*************************************************************************//**
 * @file
 * @brief .h file holds the declaractions of general use functions
 **************************************************************************/

#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include "movieSet.h"

class movieSet;

/// Used for instantaneous keyboard input
char getch();

/// Reads in command line arguments
bool getNames(int argc, char** argv, std::string& fileName, std::string& name);

/// Handles the input for the main loop function
bool handleInput(movieSet &movSet, char input, bool &quit);

/// Main program loop for using the program
void mainLoop(movieSet &movieActorSet);

/// Get the length of a number in decimal
int numberLength(int num);

/// Opens the input file
bool openFile(std::string fileName, std::ifstream &fin);

/// Outputs menu instruction
void outputInstructions();

/// Reads in contents of a file into the graph
void readFile(std::ifstream &fin, movieSet &movie);

/// Tokenize the names that are read in
void tokenNames(std::string &line, std::vector<std::string> &names);


