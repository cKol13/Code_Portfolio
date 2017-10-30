/*************************************************************************//**
 * @file
 *
 * @brief Main file that holds the int main() function.
 *
 * @mainpage Program 3 - Verifying Zipf's Law
 *
 * @section Section M001: CSC 300
 *
 * @author Chris Kolegraff
 * @author Partner
 *
 * @date November 17, 2015
 *
 * @par Professor:
 *         Dr. Hinker
 *
 * @par Course:
 *         CSC 300 - M001 -  10:00am
 *
 * @par Location:
 *        McLaury - 313
 *
 * @section program_section Program Information
 *
 * @details
 * This program is meant to verify Zipf's Law which states that the
 * frequency of a word is inversely proportional to it's rank in a
 * frequency table when words are ranked according to frequency and
 * ties are all given the mean rank value.\n
 * The program will read a text file and split it into all of the words
 * which contain only alphabetic letters or apostrophes and will hash
 * them into a hashtable. The resulting table is then sorted first by
 * frequency and then alphabetically for a frequency.\n
 * Once the words are stored and sorted, the program outputs
 * data files showing what words have what frequencies/ranks.\n\n
 *
 * Analysis of results:
 * Looking at the included graph images, it appears that Zipf's law
 * seems to be true for the most part. When rank vs. frequency is
 * plotted on a logx - logy graph, the result is almost a line with
 * a slope of -1.
 * @section compile_section Compiling and Usage
 *
 * @par Compiling Instructions:
 *      Compiled using the included makefile and the command 'make'\n
 *      Use the command 'make time' to get more detailed information
 *      about the timing of the program.\n
 *      Use the command 'make data' to get a version that will output
 *      a file which can be plotted with gnuplot. Use the commans 'gnuplot',
 *      'set logscale xy' and 'plot file-rank_vs_frequency.data' to view the plot.
 *
 * @par Usage:
   @verbatim
   zipf [file]
   Example: zipf Shakespeare.txt
   @endverbatim
 *
 * @section todo_bugs_modification_section Todo, Bugs, and Modifications

 * @par Modifications and Development Timeline:
   @verbatim
   Date         Modification
   -----------  --------------------------------------------------------------
   Oct. 29      Initial commit with makefile and setting up repository
                Wrote simple hash function
   Oct. 30      Added timing mechanism - Runtime on Shakespeare ~220ms
                Wrote better hashfunction - Runtime on Shakespeare ~180ms
   Nov. 2       Wrote new tokenize function - Runtime on Shakespeare ~145 ms
                Started timing input/output in addition - Runtime on Shakespeare - ~170ms
   Nov. 4       Optimizations to bring total runtime on Shakespeare to ~165ms
   Nov. 9       Documented properly
   @endverbatim
 *
 *****************************************************************************/
#include "functions.h"
#include <ctime>
#include <fstream>

using namespace std;


/**************************************************************************//**
 * @author Partner
 * @author Chris Kolegraff
 *
 * @par Description:
 * Calls all the functions necessary to open the file, read and hash the words,
 * and output the stats. Times how long each step takes and outputs some
 * timing information at the end.
 *
 * @param[in]   argc - The number of command line arguments
 * @param[in]   argv - The command line arguments
 *
 * @returns -1 The program was not invoked correctly
 * @returns 1 The file for input couldn't be opened
 * @returns 0 The program ran successfully
 *****************************************************************************/
int main( int argc, char *argv[] )
{
    string file;
    string fileName;
    ifstream input;
    vector<string> lines;
    hashTable::hashNode* list = nullptr;


    //Make sure there are enough arguments and get the name of the file
    if(!getFileName(file, fileName, argc, argv))
    {
        cout << "Usage: zipf [file].txt\n";
        return -1;
    }

    //Open the file and check for errors
    input.open(file);
    if(!input)
    {
        cout << "Failed to open file " << file << "; exiting..." << endl;
        return 1;
    }

    //Create the hashtable
    hashTable words;

    //Read the text into a vector
    auto c1 = clock();
    readFile(input, lines);


#ifdef TIME
    auto c2 = clock();
#endif
    //Process the text
    processWords(lines, &words);

#ifdef TIME
    auto c3 = clock();
#endif
    //Sort the hashed words
    sortHash(&words, list);


#ifdef TIME
    auto c4 = clock();
#endif
    //Ouput to files
    outputFiles(list, fileName, words.getUniqueWords(), words.getTotalWords());

    auto c5 = clock();

    //Output timings; not including the time taken to call clock() (It can be a pretty significant time)
    outputTiming("Total Runtime: ", c5 - c1);

#ifdef TIME
    outputTiming("\tTime spent reading file: ", c2 - c1);
    outputTiming("\tTime spent processing text: ", (c4 - c2));
    outputTiming("\t\tTime spent inserting into hash: ", c3 - c2);
    outputTiming("\t\tTime spent sorting: ", c4-c3);
    outputTiming("\tTime spent writing files: ", c5-c4);
#endif

    delete[] list;
    input.close();

    return 0;
}
