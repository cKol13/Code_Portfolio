/**************************************************************************//**
* @file
*
* @brief .H file for function declarations
*****************************************************************************/
#pragma once
#include <stack>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>

#include "hashTable.h"

///Read all the text into a file into a vector
void readFile(std::istream& file, std::vector<std::string>& lines);

///Hash all the words in a vector
void processWords(std::vector<std::string>& lines, hashTable* table);

///Sort the word/frequency pairs in a hashtable
void sortHash(hashTable* table, hashTable::hashNode*& list);

///Output all the stats about the hashed words
void outputFiles(hashTable::hashNode* list, std::string name, int unique, int total, int cols = 4);

///Split a string into all of the words in it
void tokAlpha(std::string &&str, std::vector<std::string> &tokens);

///Check command line arguments and get the file name
bool getFileName(std::string& str, std::string& name, int argc, char** argv);

///Compare function for qsort to use
int qCompareNodes(const void* f1, const void* f2);

///Output a number of clock cycles in ms
void outputTiming(std::string txt, double time);

/// Checks number for primeness
bool isPrime(int &num);

/// Gets the next prime
void nextPrime(int &num);

/// Converts letters to lower case
void lowerCase(std::string &lines);
