/*************************************************************************//**
 * @file
 * @brief .cpp file holds the definitions of general use functions
 **************************************************************************/

#include "functions.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

/**************************************************************************//**
 * @author VvV
 *
 * @par Description:
 * This is a function specific for Linux that emulates Windows' "_getch()" function.
 * The _getch() function pulls input from the keyboard every key press, instead of
 * waiting for the user to enter the end line character. This function is used to
 * add to the user interface by allowing quick navigation of the menu.
 *
 * The code's origin can be found here:
 * http://cboard.cprogramming.com/faq-board/27714-faq-there-getch-conio-equivalent-linux-unix.html
 *
 * @returns char What key was pressed
 *****************************************************************************/
char getch()
{
    struct termios oldt, newt;
    char character;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    character = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return character;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Processes the command line arguments. It will get the name of an input file,
 * and the name of a starting actor/movie, in double quotes, if it was entered.
 *
 * @param[in]   argc - Number of command line arguments
 * @param[in]   argv - Command line arguments
 * @param[out]  fileName - Name of the input file
 * @param[out]  name - Name of the starting node, either an actor or a movie name
 *
 * @returns true Function was successful
 * @returns false Error occurred
 ****************************************************************************/
bool getNames(int argc, char** argv, string &fileName, string &name)
{
    if(argc < 2 || argc > 3)
    {
        return false;
    }

    switch(argc)
    {
    case 2:
    {
        fileName = argv[1];
        name = "Bacon, Kevin";
        break;
    }

    case 3:
    {
        fileName = argv[1];
        name = argv[2];
        break;
    }
    default:
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
 * Handles the input for the main loop of the program. It uses a switch statement
 * to determine what option the user has chosen.
 *
 * @param[in,out]   movSet - class that contains information about actors and movies
 * @param[in,out]   quit - Holds whether or not the program should quit
 * @param[in]       input - Single character that the user entered
 *
 * @returns true Function was successful
 * @returns false Invalid input was entered
 *****************************************************************************/
bool handleInput(movieSet &movSet, char input, bool &quit)
{
    string name;
    switch(input)
    {
    // Play Six Degrees of Kevin Bacon
    case '1':
    {
        cout << "Enter the name of an actor/actress or a movie: ";
        getline(cin, name);
        cout << endl;

        if(movSet.KnownActor(name) || movSet.KnownMovie(name))
        {
            movSet.PlayBaconGame(name);
        }
        else
        {
            cout << name << " is an invalid actor/actress/movie\n";
        }
        break;
    }

    // Output histogram of actors' Bacon Numbers
    case '2':
    {
        movSet.OutputHist();
        break;
    }

    // Change the starting node
    case '3':
    {
        cout << "Enter name of an actor/actress or movie: ";
        getline(cin, name);

        // Only run Reassign if there is a new node name
        if(name != movSet.StartNodeName() && !movSet.ReassignStartNode(name))
        {
            cout << name << " is an invalid starting node" << endl;
        }

        break;
    }

    // Output actors with largest Bacon Number
    case '4':
    {
        movSet.OutputLongestPaths();
        break;
    }

    // Output
    case '5':
    {
        movSet.OutputVector(movSet.StartNodeName());
        break;
    }
    // Exit Program
    case '6':
    {
        quit = true;
        break;
    }

    default:
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
 * Main user interaction loop that handles menu navigation and selecting options
 * for operations to use on the movie/actor graph
 *
 * @param[in,out]   movieActorSet - movieSet variable that holds all of
 * the movie/actor names and connections
 *****************************************************************************/
void mainLoop(movieSet &movieActorSet)
{
    bool quit = false;
    char input = ' ';
    string name;

    while(!quit)
    {
        cout << endl;
        outputInstructions();

        name = movieActorSet.StartNodeName();

        if(movieActorSet.KnownActor(name))
            cout << "Starting actor: " << name << endl;

        else if(movieActorSet.KnownMovie(name))
            cout << "Starting movie: " << name << endl;
        name.clear();

        cout << "Option: ";
        input = getch();
        cout << input << endl << endl;

        if(!handleInput(movieActorSet, input, quit))
            cout << "Invalid input" << endl;

        if(quit)
            cout << "Exiting..." << endl;
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Counts the length of a decimal number by continually dividing the number by 10
 *
 * @param[in]   num - Number to count the length
 *
 * @returns int The length of the number
 *****************************************************************************/
int numberLength(int num)
{
    int count = 0;

    while(num > 0)
    {
        num /= 10;
        count++;
    }

    return count;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Opens a .txt file for reading in actors and movies
 *
 * @param[in]   fileName - Name of the input file
 * @param[in]   fin - File input handler
 *
 * @returns true File opened successfully
 * @returns false File did not open
 *****************************************************************************/
bool openFile(string fileName, ifstream &fin)
{
    fin.open(fileName.c_str());

    if(!fin)
    {
        return false;
    }

    return true;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Outputs the instructions for using the menu in the main program loop
 *****************************************************************************/
void outputInstructions()
{
    cout << "Select an option: \n";
    cout << "1. Play Six Degrees of Kevin Bacon\n";
    cout << "2. Output histogram of actors' Bacon Degrees\n";
    cout << "3. Change the starting node\n";
    cout << "4. Output actors with largest Bacon Number\n";
    cout << "5. Output list of start node's actors/movies\n";
    cout << "6. Exit Program\n";
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Reads an input file, and inserts the movie/actor names into a movieSet container.
 *
 * @param[in,out]   fin - File input handler
 * @param[in,out]   movie - movieSet container
 *****************************************************************************/
void readFile(ifstream &fin, movieSet &movie)
{
    string line;
    vector<string> names;

    while(getline(fin, line))
    {
        tokenNames(line, names);

        for(vector<string>::iterator it = names.begin(); it != names.end(); it++)
        {
            if(it == names.begin())
            {
                movie.Insert(*it, true);
            }
            else
            {
                movie.Insert(*it);
            }
        }

        names.clear();
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Tokenize function that will separate all names with the '/' character.
 * It will return a vector of strings. The first (0th index) string in the
 * vector is the movie's name. All following names are the actors' names.
 *
 * @param[in]   line - Line that contains all the information, the movie's and actors' names
 * @param[out]  names - Vector of the names that were tokenized
 *****************************************************************************/
void tokenNames(string &line, vector<string> &names)
{
    unsigned int begin = 0;

    for(unsigned int i = 0; i < line.size(); i++)
    {
        if(line[i] == '/')
        {
            names.push_back(line.substr(begin, i - begin));

            // Skip '/'
            begin = i + 1;
        }

        // Get the last name
        if(i == line.size() - 1)
        {
            names.push_back(line.substr(begin, i));
        }
    }
}
