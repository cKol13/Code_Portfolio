/*************************************************************************//**
 * @file main.cpp file holds int main()
 *
 * @brief Main file that holds the int main() function.
 *
 * @mainpage Program 4 - Six Degrees of Kevin Bacon
 *
 * @section Section M001: CSC 300
 *
 * @author Chris Kolegraff
 *
 * @date December 7, 2015
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
 * This program will read in an input file with '/' separated fields. The first name
 * is the movie name, and the following names are the actors that are in that movie.
 * The graph that will hold the data read in from the file consists of two structs,
 * an actor and a movie struct. Movie structs point to actors, and actors point to movies.
 *
 * Once the initial graph is ready, the user will be prompted to input options for
 * using this program. Pressing 1, for example, will ask the user for a name. The user
 * can enter a name, not in double quotes, and it can be either an actor's name, or a
 * movie's name. Once a name has been entered and validated, the program will play
 * Six Degrees of Kevin Bacon. It will recursively find the start node, with the
 * default node being Kevin Bacon. The program will output a path it took to find
 * the starting node.
 *
 * Six Degrees of Kevin Bacon can be played by selecting a start actor or movie. It will output
 * a list which is the path to the starting node. You can output a histogram which will
 * show how closely connected the rest of the actors are to that actor. You can output a list
 * of the actors that are the furthest away from the starting node. you can reassign the starting
 * node to either another actor or a movie.
 *
 * @section compile_section Compiling and Usage
 *
 * @par Compiling Instructions:
 *
 *      To compile, enter "make".
 *
 *      To create Doxygen documentation, enter "doxygen Doxyfile"
 *
 * @par Usage:
   @verbatim
   To use the program, enter [Bacon_Number] followed by a text file [fileName.txt] to read in that has '/' separated fields.

   Additionally, you can enter a name, ["Actor/Movie Name"] to change the initial starting node. If no name is given, the program
   will default to using "Bacon, Kevin" as the starting node. The name that is written in has to be in double quotes.

   Examples:
            Bacon_Number action06.txt
            Bacon_Number all06.txt "Connery, Sean"
            Bacon_Number all06.txt "Zoo (2007)"
   @endverbatim
 *
 * @section todo_bugs_modification_section Todo, Bugs, and Modifications
 *
 * @bug No known bugs.
 *
 * @todo Nothing to do.
 *
 * @par Modifications and Development Timeline:
   @verbatim
   Date         Modification
   -----------  --------------------------------------------------------------
   Nov. 15      Wrote frame work for program. Began reading in/tokenizing names
   Nov. 16      Wrote most of the movieSet class. Began writing functions to update bacon numbers
   Nov. 17      Debugging and fixing bacon number functions
   Nov. 20      Debugging and cleaning code
   Nov. 21      Debugging / Doxygen / Cleaning
   Dec. 4       Finalized and submitted program
   @endverbatim
 *
 *****************************************************************************/

#include "functions.h"

using namespace std;


/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * This function will start the main program. It will read in the file into a movieSet, and
 * perform opertions with it based on user input in a menu system. Various error checks
 * will occur making sure that the program runs successfully.
 *
 * @param[in]   argc - Number of command line arguments
 * @param[in]   argv - Contains command line arguments
 *
 * @returns  0 Program ran successfully
 * @returns -1 Error with input arguments
 * @returns -2 Error opening input file
 * @returns -3 Could not make the actor/movie the start node
 *****************************************************************************/
int main(int argc, char** argv)
{
    string fileName;
    string startNode;

    // Handle the incoming arguments
    if(!getNames(argc, argv, fileName, startNode))
    {
        cout << "Usage: Bacon_Number textFile.txt [Optional] \"Additional Name\" " << endl;
        return -1;
    }

    ifstream fin;
    if(!openFile(fileName, fin))
    {
        cout << "Could not open file: " << fileName << endl;
        return -2;
    }

    movieSet actorSet;

    // Read in from the file into the actorSet
    readFile(fin, actorSet);
    fin.close();

    if(!actorSet.MakeStartNode(startNode))
    {
        cout << "Could not make the actor/movie named [" << startNode << "] the starting node.\n";
        return -3;
    }

    // Calculate "Bacon Number" and depth for each actor/movie
    actorSet.NumberActors();
    mainLoop(actorSet);
    return 0;
}
