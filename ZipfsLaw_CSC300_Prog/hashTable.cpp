/**************************************************************************//**
* @file
*
* @brief .CPP file holds hashTable definitions
*****************************************************************************/
#include <ctime>
#include "hashTable.h"
#include "functions.h"

using namespace std;

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Constructor for the hashtable. Sets the table to have 1009 slots
 *****************************************************************************/
hashTable::hashTable()
{
    currentSize = uniqueWords = totalWords = 0;
    tableSize = 1009;
    hTable = new hashNode [tableSize];
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Deconstructor for the hashtable; deletes the array used by the table
 *****************************************************************************/
hashTable::~hashTable()
{
    deleteArray(hTable);
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Computes the current load factor of the hashtable
 *
 * @returns double - The current load factor of the table
 *****************************************************************************/
double hashTable::getLoadFactor()
{
    return double(currentSize)/tableSize;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Gets the maximum number of spaces in the hashtable
 *
 * @returns int - Amount of space in the hashtable
 *****************************************************************************/
int hashTable::getTableSize()
{
    return tableSize;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 * @author Andrew Stelter
 *
 * @par Description:
 * Finds a string in the hashtable using linear probing
 *
 * @param[in]   text - The string to find
 *
 * @returns int - The number of times the word has been hashed into the table
 * @returns 0   - The string was not found in the hash table
 *****************************************************************************/
int hashTable::find(string text)
{
    //Find ideal location for word
    int start = hashWord(text);

    //Linear probe to it or the next empty spot
    //If the item is in the ideal location, it'll be returned
    int index = linearProbe(start, text, true);

    //If the item is not in the table, or it's full
    //For some reason, return
    if(index == -1) return 0;

    //This will be the correct frequency if the item was found
    //It will be 0 if it wasn't found
    return hTable[index].frequency;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 * @author Andrew Stelter
 *
 * @par Description:
 * Hashes a word into the hashtable using linear probing
 *
 * @param[in]   text - The word to hash
 * @param[in]   freq - The number of times to add it (Defaults to 1)
 *****************************************************************************/
void hashTable::insertWord(string &&text, int freq)
{
    //Check where it is ideally
    int start = hashWord(text);

    //Linear probe to the item or next empty spot
    //If the item is in the ideal position, that'll be returned
    int index = linearProbe(start, text, false);

    //If the item is not in the table, and it's full
    //For some reason, return
    if(index == -1) return;

    //Store the item in the given location

    //If the word is in the hashTable
    if(hTable[index].word == text)
    {
        totalWords += freq;
        hTable[index].frequency += freq;
    }
    else
    {
        uniqueWords++;
        totalWords += freq;
        hTable[index].frequency = move(freq);
        hTable[index].word = move(text);
        currentSize++;

        // Check to see if the load factor is too high
        if(getLoadFactor() > 0.75)
        {
            resize();
        }
    }
}

/**************************************************************************//**
 * @author Andrew Stelter
 *
 * @par Description:
 * Removes a word from the hashtable and sets the frequency of that location to
 * -1 so that the linear probe will know to skip it if in lookup mode
 *
 * @param[in]   text - The word to remove
 *****************************************************************************/
void hashTable::deleteWord(string text)
{
    lowerCase(text);

    //Check the ideal location for the word
    int start = hashWord(text);

    //Linear probe to the word or next empty spot
    int index = linearProbe(start, text, true);

    //If not a -1, reset that spot
    if((index != -1) && hTable[index].frequency > 0)
    {
        cout << "Deleting word: " << hTable[index].word << endl;
        uniqueWords--;
        totalWords-=hTable[index].frequency;
        hTable[index].word = "";
        hTable[index].frequency = 0;
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Deletes multiple words in the hash table.
 *
 * @param[in]   text - The words to remove
 *****************************************************************************/
void hashTable::deleteWords(std::string text)
{
    vector<string> words;

    tokAlpha(move(text), words);

    while(!words.empty())
    {
        deleteWord(words.back());
        words.pop_back();
    }
}


/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Makes the hashtable bigger - (nextPrime(tableSize*2))- and hashes all of the values into it again
 *****************************************************************************/
void hashTable::resize()
{
    totalWords = uniqueWords = currentSize = 0;

    int oldSize = tableSize;
    tableSize *= 2;
    nextPrime(tableSize);

    hashNode* placeHolder = hTable;
    hTable = new hashNode [tableSize];

    // Rehashing function
    for(int i = 0; i < oldSize; i++)
    {
        if(placeHolder[i].frequency > 0)
        {
            insertWord(move(placeHolder[i].word), placeHolder[i].frequency);
        }
    }

    cout << "Hashtable too full!\nRehashing from " << oldSize << " items to " << tableSize << " items\n";
    deleteArray(placeHolder);
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Deallocates an array
 *
 * @param[in]   arr - The array to deallocate
 *****************************************************************************/
void hashTable::deleteArray(hashNode*& arr)
{
    delete[] arr;
    arr = nullptr;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Creates an index for the hash table using bitwise operations based on the letters
 * in the word.
 *
 * @param[in]   word - The word to create a hashcode for
 *
 * @returns int - The hashcode for the word
 *****************************************************************************/

int hashTable::hashWord(string& word)
{
    unsigned long hash = 234557;

    // 10, 181, and 89 were found by using nested for-loops to iterate
    // through most of the possible combinations. We ran the main program
    // ~10 times per combination to get an average.
    for(string::iterator it = word.begin(); it != word.end(); it++)
    {
        hash = (hash << 10) ^ (hash * 181 * (*it) + 89 * (*it));
    }

    int ret = hash % tableSize;

    return ret;
}

/**************************************************************************//**
 * @author Andrew Stelter
 *
 * @par Description:
 * Starting from a location, walks through the hashtable until the given
 * word or an empty space is found. Will by default count spaces left
 * by removed words as non-empty unless lookup is set to false
 *
 * @param[in]   start - Where to start the probe
 * @param[in]   word - The word to find
 * @param[in]   lookup - Whether to pass over removed spaces or not
 *
 * @returns int - The location of the word/next empty space
 * @returns -1 - If the table is entirely full and the word is not in it
 *****************************************************************************/
int hashTable::linearProbe(int &start, std::string &word, bool lookup)
{
    int i = start;
    hashNode* curr = hTable + start;
    //Search for the next item that is the word or empty
    //Skips spaces set to -1 if lookup is true
    while(curr->word != word && (curr->frequency > 0 || (lookup && curr->frequency == 0)))
    {
        i = (i+1) % tableSize;
        curr = hTable + i;
        //If this loops around, that's an issue
        //If everything runs properly, it'll never return -1
        if(i==start) return -1;
    }
    return i;
}

/**************************************************************************//**
 * @author Andrew Stelter
 * @author Chris Kolegraff
 *
 * @par Description:
 * Copies all full items from the table into an array for use elsewhere
 *
 * @returns hashNode* - A dynamically allocated copy of all items in the table
 *****************************************************************************/
hashTable::hashNode* hashTable::getTable()
{
    //Set up a copy into array, + 10 spaces for safety
    hashNode* copy = new hashNode[currentSize + 10];
    int index = 0;

    //Copy the array's contents over
    for(int i=0; i<tableSize; i++)
    {
        // Only copies if there is something to copy
        if(hTable[i].frequency > 0)
        {
            copy[index] = hTable[i];
            index++;
        }
    }

    //Return the copy
    return copy;
}

/**************************************************************************//**
 * @author Andrew Stelter
 *
 * @par Description:
 * Gets the total number of words inserted into the table
 *
 * @returns int - The number of words inserted
 *****************************************************************************/
int hashTable::getTotalWords()
{
    return totalWords;
}

/**************************************************************************//**
 * @author Andrew Stelter
 *
 * @par Description:
 * Gets the total number of unique words inserted into the table
 *
 * @returns int - The number of unique words inserted
 *****************************************************************************/
int hashTable::getUniqueWords()
{
    return uniqueWords;
}
