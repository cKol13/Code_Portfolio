/**************************************************************************//**
* @file
*
* @brief CPP file for function definitions.
*****************************************************************************/
#include "functions.h"
#include <fstream>

using namespace std;

/*!
* @brief Typedef to scope the hashNode type correctly
*/
typedef hashTable::hashNode hashNode;

/**************************************************************************//**
 * @author Partner
 * @author Chris Kolegraff
 *
 * @par Description:
 * Takes the command line arguments and checks if they are valid.\n
 * If they are, parses out the name of the file to be opened.
 *
 * @param[out]  str - The full input file name
 * @param[out]  name - The input file name without the extension
 * @param[in]   argc - The number of params
 * @param[in]   argv - The param list
 *
 * @returns bool - Whether the command arguments were correct or not
 *****************************************************************************/
bool getFileName(string& str, string& name, int argc, char** argv)
{
    //Check argument number
    if(argc != 2)
    {
        return false;
    }
    else
    {
        //Parse the string; split at the last .
        str = argv[argc - 1];
        name = str.substr(0, str.rfind('.'));
        return true;
    }
}

/**************************************************************************//**
 * @author Partner
 * @author Chris Kolegraff
 *
 * @par Description:
 * Reads a text file into a vector of strings line by line.
 *
 * @param[in]   file - The text file to read
 * @param[out]  lines - The vector to read into
 *****************************************************************************/
void readFile(istream& file, vector<string>& lines)
{
    //Read the file line by line into a vector
    string line;
    while(getline(file, line))
        lines.push_back(line);
}

/**************************************************************************//**
 * @author Partner
 * @author Chris Kolegraff
 *
 * @par Description:
 * Splits lines of text into words and inserts them into a hashtable
 *
 * @param[in]   lines - The text to split
 * @param[out]  table - The hashtable to insert into
 *****************************************************************************/
void processWords(vector<string>& lines, hashTable* table)
{
    //For each line, tokenize it and insert all the words into the hash
    vector<string> myStack;

    for(vector<string>::iterator line = lines.begin(); line != lines.end(); line++)
    {
        tokAlpha(move(*line), myStack);

        while(!myStack.empty())
        {
            table->insertWord(move(myStack.back()));

            myStack.pop_back();
        }
    }
}

/**************************************************************************//**
 * @author Partner
 *
 * @par Description:
 * Splits a line of text into words containing only alphabetic characters
 * and apostrophes
 *
 * @param[in]   str - The line to tokenize
 * @param[out]  tokens - Storage or the words found
 *****************************************************************************/
void tokAlpha(string&& str, vector<string>& tokens)
{
    //Make the line lowercase
    lowerCase(str);

    unsigned int thsStart = 0;
    unsigned int thsEnd = 0;
    unsigned int lastGood = 0;

    unsigned int strSize = str.size();

    //Walk through the string
    while(thsStart < strSize)
    {
        //Find a char that is a-z
        while((str[thsStart] < 'a' || str[thsStart] > 'z'))
            if(++thsStart == strSize) return;

        //Find the last alpha character in the word (Including any ' that is surrounded by a-z chars)
        thsEnd = thsStart;
        while((str[thsEnd] >= 'a' && str[thsEnd] <= 'z') || str[thsEnd] == '\'')
        {
            if(thsEnd == strSize) break;
            if(str[thsEnd] != '\'') lastGood = thsEnd;
            thsEnd++;
        }

        //Substring and add to stack
        tokens.push_back(str.substr(thsStart, lastGood-thsStart+1));
        thsStart = thsEnd;
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Comparison function for std::qsort to sort hash nodes
 *
 * @param[in]   f1 - The first item to compare
 * @param[in]   f2 - The second item to compare
 *
 * @returns 1 - The first item belongs before the second
 * @returns 0 - The items belong in the same place
 * @returns -1 - The second item belongs before the first
 *****************************************************************************/
int qCompareNodes(const void* f1, const void* f2)
{
    //Cast nodes to correct type
    hashNode *node1 = (hashNode*)f1;
    hashNode *node2 = (hashNode*)f2;

    //Compare frequency
    if(node1->frequency < node2->frequency)
    {
        return 1;
    }
    else if(node1->frequency > node2->frequency)
    {
        return -1;
    }
    else
    {
        //If same frequency, compare text
        return node1->word.compare(node2->word);
    }
}

/**************************************************************************//**
 * @author Partner
 * @author Chris Kolegraff
 *
 * @par Description:
 * Copies the list from a hashtable and sorts it using qsort
 *
 * @param[in]   table - The hashtable to get nodes from
 * @param[out]  list - The sorted list of word/frequency pairs
 *****************************************************************************/
void sortHash(hashTable* table, hashTable::hashNode*& list)
{
    //Copy the hash table
    list = table->getTable();

    // Sort the contents of the array
    qsort(list, table->getUniqueWords(), sizeof(hashTable::hashNode), qCompareNodes);
}

/**************************************************************************//**
 * @author Partner
 *
 * @par Description:
 * Outputs two (possibly three) files containing stats about the sorted words.\n
 * Outputs a .wrd file which states what words were contained in what frequencies\n
 * Outputs a .csv file with frequency/rank/number of words triplets\n
 * If DATA is defined, then outputs a .data file which can be used
 * to graph rank vs. frequency in gnuplot.
 *
 * @param[in]   list - The list of word-frequency pairs
 * @param[in]   name - The name to base all files on
 * @param[in]   unique - The number of unique words (items in the list)
 * @param[in]   total - The total number of words read/hashed
 * @param[in]   cols - # of columns to output words in in the .wrd file
 *****************************************************************************/
void outputFiles(hashTable::hashNode* list, std::string name, int unique, int total, int cols)
{
    //Open files to output
    ofstream wrd(name+".wrd");
    ofstream csv(name+".csv");
#ifdef DATA
    ofstream d1(name+"-rank_vs_frequency.data");
#endif
    int longestLength = 0;
    int ind = 0;
    int tmpind = 0;
    int wordCount = 0;
    int rank = 0;
    int wWidth = 35;

    //Find the longest word
    while(list[ind].word != "" && ind < unique)
    {
        longestLength = max(longestLength, (int)list[ind].word.size());
        ind++;
    }

    //Calculate the width of the columns
    wWidth = max(wWidth, cols*(longestLength+2));

    wrd << setprecision(1) << fixed;
    csv << setprecision(1) << fixed;

    //Output .wrd header
    wrd << "Zipf's Law" << endl << string(10,'-') << endl;
    wrd << "File: " << name << ".txt" << endl;
    wrd << "Total number of words = " << total << endl;
    wrd << "Number of distinct words = " << unique << endl << endl;
    wrd << left << setw(wWidth) << "Word Frequencies" << '\t' << right << setw(10) << "Ranks" << '\t' << setw(10) << "Avg. Rank" << endl << left;
    wrd << left << string(wWidth, '-') << '\t' << right << string(10,'-') << '\t' << string(10, '-')<< endl;

    //Output .csv header
    csv << "Zipf's Law" << endl << string(10,'-') << endl;
    csv << "File: " << name << ".txt" << endl;
    csv << "Total number of words = " << total << endl;
    csv << "Number of distinct words = " << unique << endl << endl;
    csv << setw(15) << "Rank" << '\t' << setw(15) << "Frequency" << '\t' << setw(15) << "Rank*Frequency" << endl;
    csv << string(15, '-') << '\t' << string(15, '-') << '\t' << string(15, '-') << endl;

    ind = 0;
    rank = 1;
    //Go until empty word or end of list
    while(list[ind].frequency > 0 && ind < unique)
    {
        wordCount = 0;
        tmpind = ind;

        //Find how many words have this frequency
        while(list[tmpind].frequency == list[ind].frequency)
        {
            tmpind++;
            wordCount++;
        }

        //Output rank and frequency to files
        wrd << left << setw(wWidth) << "Words occurring " + to_string(list[ind].frequency) + " times: " << '\t';
        wrd << right << setw(10) << to_string(rank) + (wordCount==1?"":"-" + to_string(rank+wordCount-1)) << '\t';
        wrd << right << setw(10) << ((rank + rank+wordCount-1)/2.0) << left << endl;

        csv << setw(14) << right << ((rank + rank+wordCount-1)/2.0) << ",\t" << setw(14) << right << list[ind].frequency;
        csv << ",\t" << setw(15) << right << ((rank + rank+wordCount-1)/2.0)*list[ind].frequency << endl;

#ifdef DATA
        //Output to gnu plot file
        d1 << ((rank + rank+wordCount-1)/2.0) << ", " << list[ind].frequency << endl;
#endif

        //Output all words with the same frequency in columns
        tmpind = ind;
        while(list[tmpind].frequency == list[ind].frequency)
        {
            wrd << setw(wWidth/cols) << list[tmpind].word;
            tmpind++;
            if((tmpind-ind) % cols == 0 && ind != 0) wrd << endl;
        }

        if((tmpind+1-ind) % cols != 0 || ind == 0) wrd << endl;
        wrd << endl;

        rank+=wordCount;
        ind = tmpind;
    }

    //Close files
    wrd.close();
    csv.close();
#ifdef DATA
    d1.close();
#endif
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Outputs a number of ms taken to do a certain number of clock cycles
 *
 * @param[in]   txt - The text to output before the number
 * @param[in]   time - The number of clock cycles to convert to ms
 *****************************************************************************/
void outputTiming(string txt, double time)
{
    time = time * 1000.0 / CLOCKS_PER_SEC;
    cout << setprecision(1) << fixed << txt << time << "ms.\n";
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Checks if a number is prime or not
 *
 * @param[in]   num - The number to check for primeness
 *
 * @returns bool - Whether the number is prime or not
 *****************************************************************************/
bool isPrime(int &num)
{
    if(num == 2)
    {
        return true;
    }

    if(num < 2 || num % 2 == 0)
    {
        return false;
    }

    int stop = int(sqrt(double(num) + 1.0));

    for(int i = 3; i <= stop; i += 2)
    {
        if(num % i == 0)
        {
            return false;
        }
    }

    return true;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Finds the first prime that is after the given number
 *
 * @param[in, out]  num - The number to find the prime after
 *****************************************************************************/
void nextPrime(int &num)
{
    num++;
    int old = num;

    // Make sure num is odd
    if(num % 2 == 0)
    {
        num++;
    }

    while(true)
    {
        if(isPrime(num))
        {
            return;
        }
        num += 2;
    }

    num = old;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Converts any upper case letter to lower case
 *
 * @param[in, out]  lines - Holds the words to apply lowercase to.
 *****************************************************************************/
void lowerCase(std::string &lines)
{
    for(string::iterator it = lines.begin(); it != lines.end(); it++)
    {
        // Check if the letter is capitalized
        if(*it > 64 && *it < 91)
        {
            *it += ' ';
        }
    }
}
