/**************************************************************************//**
* @file
*
* @brief .H file for the hashTable class declarations
*****************************************************************************/
#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

/**************************************************************************//**
* @class hashTable
*
* @brief A hashtable class for hashing strings
*
* @brief This class supports O(1) insertion, removal,
* and lookup in a hashtable. If a word is inserted
* multiple times, it's frequency is incremented rather
* than a second copy put in. When the load factor
* of the table reaches > 0.75, the table will
* find the next prime greater than currentsize*2
* and rehash into a table that big.
*****************************************************************************/
class hashTable
{
public:
    //!A node object for the table to hash
    /*!
    * Contains a word and a frequency associated with that word
    */
    struct hashNode
    {
        /// The frequency of the word
        int frequency = 0;

        /// The stored word
        std::string word = "";
    };

    ///Default constructor
    hashTable();

    ///Deconstructor
    ~hashTable();

    ///Get the size of the table
    int getTableSize();

    /// get number of total words
    int getTotalWords();

    ///Get number of unique words
    int getUniqueWords();

    ///Check the load factor of the table
    double getLoadFactor();

    ///Get how many times a string is in the table(frequency of string text)
    int find(std::string text);

    ///Insert string text into the table freq times
    void insertWord(std::string &&text, int freq=1);

    ///Remove a word from the table
    void deleteWord(std::string text);

    ///Remove multiple words from the table
    void deleteWords(std::string text);

    ///Get a condensed copy of the hashtable
    hashNode* getTable();

private:

    /// Amount of space in the table
    int tableSize;

    /// Number of items in the table
    int currentSize;

    /// Number of Unique words in the hash table
    int uniqueWords;

    /// Sum of all frequencies stored
    int totalWords;

    /// Node array used to implement hash table
    hashNode* hTable;



    /// Resize and rehash the table
    void resize();

    /// Clear the dynamic array
    void deleteArray(hashNode*& arr);

    /// Get hash and compress a word to an index
    int hashWord(std::string &word);

    /// Get an index, using a probe if needed
    int linearProbe(int &start, std::string &word, bool lookup = true);
};



