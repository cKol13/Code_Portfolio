/*************************************************************************//**
 * @file
 * @brief .cpp file holds the definitions of the movieSet class
 **************************************************************************/



#include "movieSet.h"

#include <iomanip>

using namespace std;

/// Unordered map's iterator for actor nodes, actor Iterator
typedef unordered_map<string, actor*>::iterator aIter;

/// Unordered map's iterator for movie nodes, movie Iterator
typedef unordered_map<string, movie*>::iterator mIter;

/// Vector's iterator for actor nodes, actor vector Iterator
typedef vector<actor*>::iterator avIter;

/// Vector's iterator for movie nodes, movie vector Iterator
typedef vector<movie*>::iterator mvIter;

//##################################################//
// PUBLIC FUNCTIONS
//##################################################//

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Basic constructor for the movieSet class. Sets unordered_map variables' max load factor to 0.75.
 *****************************************************************************/
movieSet::movieSet()
{
    knownActors.max_load_factor(0.75f);
    knownMovies.max_load_factor(0.75f);

    targetMovie = selectedMovie = startingMovie = nullptr;
    targetActor = startingActor = nullptr;

    updated = false;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Basic destructor for the movieSet class. Calls a delete function which will
 * delete all dynamically allocated nodes.
 *****************************************************************************/
movieSet::~movieSet()
{
    DeleteGraph();
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Inserts either a new movie or a new actor node into their respective unordered_maps, or hash tables.
 * After an insertion, the function will make the movie node and actor node point to each other, creating a graph.
 * The function will check if an actor is already in the actor hash table, and if it is it will set the pointers.
 *
 * @param[in]   name - Name of the actor/movie being inserted
 * @param[in]   isMovie - Whether or not the incoming name is a movie
 *****************************************************************************/
void movieSet::Insert(string &name, bool isMovie)
{
    if(isMovie)
    {
        // Add a new movie with no actors in its vector. Set it as the selected movie
        movie* tempMovie = new movie;
        tempMovie->name = name;
        selectedMovie = tempMovie;
        knownMovies.insert(make_pair<string, movie*>(move(name), move(tempMovie)));
        tempMovie = nullptr;
    }
    else
    {
        // If the actor is known, push the selected movie onto that actor's movie vector
        if(KnownActor(name))
        {
            // Find the actor, get the list of movies the actor's been in, and add the selected movie
            actor* tempActor = FindActor(name);
            tempActor->movies.push_back(selectedMovie);
            tempActor->numMovies++;

            // Take the selected movie, and add the actor to the list of that movie's actors
            selectedMovie->actors.push_back(tempActor);
            selectedMovie->numActors++;
        }
        // If the actor isn't known, create a new actor
        else
        {
            // Create a new actor
            actor* temp = new actor;

            // Set the name of the actor
            temp->name = name;
            temp->numMovies++;

            // Add the current movie to the list of movies the actor's been in
            temp->movies.push_back(selectedMovie);

            // Add the actor to the list of actors that are in the selected movie
            selectedMovie->actors.push_back(temp);
            selectedMovie->numActors++;

            // Make the actor known
            knownActors.insert(make_pair<string, actor*>(move(name), move(temp)));
            temp = nullptr;
        }
    }
    name.clear();
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * This function will return whether or not an actor is in the knownActors hash table.
 *
 * @param[in]   name - Name of the actor to find
 *
 * @returns true Actor was in the hash table
 * @returns false Actor was not in the hash table
 *****************************************************************************/
bool movieSet::KnownActor(string &name)
{
    if(knownActors.find(name) == knownActors.end())
    {
        return false;
    }

    return true;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * This function will return whether or not a movie is in the knownMovies hash table.
 *
 * @param[in]   name - Name of the movie to find
 *
 * @returns true Movie was in the hash table
 *****************************************************************************/
bool movieSet::KnownMovie(string &name)
{
    if(knownMovies.find(name) == knownMovies.end())
    {
        return false;
    }

    return true;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Makes either an actor or a movie the middle, or starting position in the graph
 * for calculating the Bacon Numbers. If the function cannot make an actor or a movie
 * the starting node, no changes will occur.
 *
 * @param[in]   name - Name of the actor/movie to make the starting node
 *
 * @returns true  Actor/movie was made the starting node
 * @returns false Actor/movie was not made the starting node.
 *****************************************************************************/
bool movieSet::MakeStartNode(string name)
{
    if(KnownActor(name) || KnownMovie(name))
    {
        startingActor = FindActor(name);
        startingMovie = FindMovie(name);
        return true;
    }

    return false;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Assigns Bacon Numbers to movies and actors. This is an interface function that will
 * first set up the algorithm by varifying there is a valid starting node, as well as
 * making sure that actor/movie has at least one movie/actor. It will then call a
 * recursive function that will do the assignments.
 *
 * @returns true The assignments worked
 * @return false The assignments did not work
 *****************************************************************************/
bool movieSet::NumberActors()
{
    // If the start node is an actor
    if(startingActor != nullptr && startingMovie == nullptr)
    {
        startingActor->baconNumber = 0;

        if(startingActor->movies.empty())
        {
            cout << startingActor->name << " was not in any movies" << endl;
            return false;
        }

        // Continue to run until all nodes have the shortest possible path
        do
        {
            // Reset all visted values to false
            ResetVisited();

            updated = false;
            NumberActors(startingActor, 0);
        }
        while(updated);
    }
    // If the start node is a movie
    else if(startingActor == nullptr && startingMovie != nullptr)
    {
        startingMovie->depth = 0;

        if(startingMovie->actors.empty())
        {
            cout << startingMovie->name << " does not have any actors" << endl;
            return false;
        }

        // Continue to run until all nodes have the shortest possible path
        do
        {
            // Reset all visted values to false
            ResetVisited();

            updated = false;
            NumberActors(startingMovie, 1);
        }
        while(updated);
    }
    else
    {
        cout << "Error: No starting node found." << endl;
        return false;
    }

    return true;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Outputs a histogram of the actors' Bacon Numbers. Only actors will have their numbers
 * printed. Movies will not, so if a starting node is a movie, the first spot, 0, will
 * have a frequency of 0.
 *****************************************************************************/
void movieSet::OutputHist()
{
    int freq = FindMaxFreq();
    int sum = 0;
    int count = 0;
    double average = 0.0;
    int *buckets = nullptr;

    // + 3 for safety
    buckets = new int [freq + 3];

    for(int i = 0; i < freq + 3; i++)
    {
        buckets[i] = 0;
    }

    // Count frequencies
    for(aIter it = knownActors.begin(); it != knownActors.end(); it++)
    {
        if(it->second->baconNumber != INF)
        {
            buckets[it->second->baconNumber]++;
            sum += it->second->baconNumber;
            count++;
        }
    }

    cout << "***********************Histogram***********************\n";
    for(int i = 0; i <= freq; i++)
    {
        cout << left << i << setw(16 - numberLength(i)) << right << buckets[i] << endl;
    }

    int infs = CountInfinites();

    cout << left << "Inf. " << setw(16 - numberLength(infs)) << right << infs  << endl;
    average = double(sum) / count;
    cout << endl << "Average: " << average << endl;
    delete[] buckets;
    buckets = nullptr;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Outputs a list of actors to the console that have the highest Bacon Numbers.
 *****************************************************************************/
void movieSet::OutputLongestPaths()
{
    int freq = FindMaxFreq();

    cout << "Actors with Bacon Number of: " << freq << endl << endl;

    for(aIter it = knownActors.begin(); it != knownActors.end(); it++)
    {
        if(it->second->baconNumber == freq)
        {
            cout << it->second->name << endl;
        }
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Outputs how many movies an actor was in, or how many actors a movie has.
 * Also outputs a list of movies the actor was in, or a list of actors the movie casts.
 *
 * @param[in]   name - Name of the actor/movie to output
 *****************************************************************************/
void movieSet::OutputVector(std::string name)
{
    actor* act = nullptr;
    movie* mov = nullptr;

    if(KnownActor(name))
    {
        act = FindActor(name);

        cout << act->numMovies << " " << act->name << " movies: " << endl;

        for(mvIter it = act->movies.begin(); it != act->movies.end(); it++)
        {
            cout << "\t" << (*it)->name << endl;
        }
    }
    else if(KnownMovie(name))
    {
        mov = FindMovie(name);

        cout << mov->numActors << " " << mov->name << " performers" << endl;

        for(avIter it = mov->actors.begin(); it != mov->actors.end(); it++)
        {
            cout << "\t" << (*it)->name << endl;
        }
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Interface function that will initialize the Six Degrees
 * of Kevin Bacon game. It will find the node where the user wishes to start, and checks
 * for validity. If the startName is valid, it will reset every node's visited value to false,
 * and then call the recursive function that will navigate to the starting node.
 *
 * @param[in]   startName - Name of the node the user wishes to start
 *****************************************************************************/
void movieSet::PlayBaconGame(string startName)
{
    // Make sure that the actor/actress or movie is related to the start node
    movie* mTemp = FindMovie(startName);
    actor* aTemp = FindActor(startName);

    // Check to make sure startName's node is related to starting actor/movie
    if(startingActor != nullptr)
    {
        if((aTemp != nullptr && aTemp->baconNumber == INF) ||
                (mTemp != nullptr && mTemp->depth == INF))
        {
            cout << startName << " is not related to " << startingActor->name << endl;
            return;
        }
    }
    else if(startingMovie != nullptr)
    {
        if((aTemp != nullptr && aTemp->baconNumber == INF) ||
                (mTemp != nullptr && mTemp->depth == INF))
        {
            cout << startName << " is not related to " << startingMovie->name << endl;
            return;
        }
    }
    else
    {
        cout << "Error: Invalid starting actor/movie" << endl;
        return;
    }

    // Reset all visted values to false, then play the game
    ResetVisited();
    targetActor = aTemp;
    targetMovie = mTemp;
    FindTheBacon(aTemp, mTemp);
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * This function will reassign the starting node for the graph navigation.
 * If the name is valid, it will regenerate the Bacon Numbers for the actors and
 * movies.
 *
 * @param[in]   name - Name of the new starting node
 *
 * @returns true Function reassigned the starting node
 * @returns false Function did not reassign the starting node
 *****************************************************************************/
bool movieSet::ReassignStartNode(string name)
{
    if(!MakeStartNode(name))
    {
        return false;
    }

    updated = true;
    ResetBaconNumbers();
    NumberActors();

    return true;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Returns the name of the starting node. If the starting node is null, the
 * function will return an empty string, "".
 *
 * @returns string Name of the starting node
 * @returns "" Starting node was invalid
 *****************************************************************************/
string movieSet::StartNodeName()
{
    if(startingMovie != nullptr)
    {
        return startingMovie->name;
    }
    else if(startingActor != nullptr)
    {
        return startingActor->name;
    }
    else
    {
        return "";
    }
}


//##################################################//
// PRIVATE FUNCTIONS
//##################################################//

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Counts the number of actors that are not related to the starting node.
 *
 * @returns int Number of actors not related to the starting node
 *****************************************************************************/
int movieSet::CountInfinites()
{
    int count = 0;
    for(aIter it = knownActors.begin(); it != knownActors.end(); it++)
    {
        if(it->second->baconNumber == INF)
        {
            count++;
        }
    }

    return count;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Deletes the dynamically allocated actor and movie structs in their hashtables.
 *****************************************************************************/
void movieSet::DeleteGraph()
{
    for(aIter it = knownActors.begin(); it != knownActors.end(); it++)
    {
        delete it->second;
        it->second = nullptr;
    }

    for(mIter it = knownMovies.begin(); it != knownMovies.end(); it++)
    {
        delete it->second;
        it->second = nullptr;
    }

    knownActors.clear();
    knownMovies.clear();
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Finds the highest Bacon Number in the list of actors
 *
 * @returns int Max frequency found
 *****************************************************************************/
int movieSet::FindMaxFreq()
{
    int max = -1;

    for(aIter it = knownActors.begin(); it != knownActors.end(); it++)
    {
        if(it->second->baconNumber != INF && it->second->baconNumber > max)
        {
            max = it->second->baconNumber;
        }
    }

    return max;
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Recursive function that will find its way back to the starting node.
 * It will search the surrounding nodes, and it will pick the node with the
 * smallest depth or Bacon Number.
 *
 * @param[in]   act Points to the most current actor
 * @param[in]   mov Points to the most current movie
 *****************************************************************************/
void movieSet::FindTheBacon(actor *act, movie *mov)
{
    // Output arrow to all the names beneath the target node
    if(act != targetActor || mov != targetMovie)
        cout << "     V" << endl;

    if(act != nullptr)
    {
        cout << act->baconNumber << ". " << act->name << endl;
    }
    else if(mov != nullptr)
    {
        cout << mov->name << endl;
    }


    movie* chosenMovie = nullptr;
    actor* chosenActor = nullptr;

    int minNum = 1000000;

    // If a movie is chosen
    if(act == nullptr && mov != nullptr)
    {
        // Check if we've found the target
        if(mov == startingMovie)
        {
            return;
        }

        // Find the minimum Bacon Number and that actor
        for(avIter i = mov->actors.begin(); i != mov->actors.end(); i++)
        {
            if(!(*i)->visited && (*i)->baconNumber < minNum)
            {
                minNum = (*i)->baconNumber;
                chosenActor = *i;
            }
        }

        chosenActor->visited = true;
        // Recurse using that actor
        FindTheBacon(chosenActor, nullptr);
    }
    // If an actor is chosen
    else if(act != nullptr && mov == nullptr)
    {
        // Check if we've found the target
        if(act == startingActor)
        {
            return;
        }

        // Find the minimum depth and that movie
        for(mvIter i = act->movies.begin(); i != act->movies.end(); i++)
        {
            if(!(*i)->visited && (*i)->depth < minNum)
            {
                minNum = (*i)->depth;
                chosenMovie = *i;
            }
        }

        chosenMovie->visited = true;
        // Recurse using that movie
        FindTheBacon(nullptr, chosenMovie);
    }
    else
    {
        cout << "Error finding node" << endl;
        return;
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * This function will try and find an actor in the actor hash table. If it is
 * there, it will return a pointer to that actor.
 *
 * @param[in]   name - Name of the actor to find
 *
 * @returns actor* The function found the actor
 * @returns nullptr The function did not find the actor
 *****************************************************************************/
actor* movieSet::FindActor(string &name)
{
    aIter it = knownActors.find(name);

    if(it == knownActors.end())
    {
        return nullptr;
    }
    else
    {
        return it->second;
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * This function will try and find a movie in the movie hash table. If it is
 * there, it will return a pointer to that movie.
 *
 * @param[in]   name - Name of the movie to find
 *
 * @returns movie* The function found the movie
 * @returns nullptr The function did not find the movie
 *****************************************************************************/
movie* movieSet::FindMovie(string &name)
{
    mIter it = knownMovies.find(name);

    if(it == knownMovies.end())
    {
        return nullptr;
    }
    else
    {
        return it->second;
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * One of 2 recursive functions for generating the Bacon Numbers for actors. It
 * will validate the actor, then update the neighbours, then recurse to all
 * surrounding movie nodes, if it hasn't already been visited.
 *
 * @param[in]   act - Pointer to the current actor node
 * @param[in]   distance - Current Bacon Number
 *****************************************************************************/
void movieSet::NumberActors(actor *act, int distance)
{
    if(act == nullptr)
    {
        return;
    }

    act->visited = true;

    // Update the neighbours if they need to be updated
    for(mvIter mov = act->movies.begin(); mov != act->movies.end(); mov++)
    {
        if(distance < (*mov)->depth)
        {
            (*mov)->depth = distance;
            updated = true;
        }
    }

    // Go to all of the actors' movies
    for(mvIter mov = act->movies.begin(); mov != act->movies.end(); mov++)
    {
        if(!(*mov)->visited)
        {
            if(startingMovie != nullptr)
            {
                NumberActors(*mov, (*mov)->depth);
            }
            else
            {
                NumberActors(*mov, (*mov)->depth + 1);
            }
        }
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * One of 2 recursive functions for generating the Bacon Numbers for actors. It
 * will validate the movie, then update the neighbours, then recurse to all
 * surrounding actor nodes, if it hasn't already been visited.
 *
 * @param[in]   mov - Pointer to the current movie node
 * @param[in]   distance - Current Bacon Number
 *****************************************************************************/
void movieSet::NumberActors(movie *mov, int distance)
{
    if(mov == nullptr)
    {
        return;
    }

    mov->visited = true;

    // Update the neighbours if they need to be updated
    for(avIter act = mov->actors.begin(); act != mov->actors.end(); act++)
    {
        if(distance < (*act)->baconNumber)
        {
            (*act)->baconNumber = distance;
            updated = true;
        }
    }

    // Go to all other actors
    for(avIter act = mov->actors.begin(); act != mov->actors.end(); act++)
    {
        if(!(*act)->visited)
        {
            if(startingMovie != nullptr)
            {
                NumberActors(*act, (*act)->baconNumber + 1);
            }
            else
            {
                NumberActors(*act, (*act)->baconNumber);
            }
        }
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Resets the Bacon Number and depth of all actor and movie nodes to INF,
 * which basically represents infinite length.
 *****************************************************************************/
void movieSet::ResetBaconNumbers()
{
    for(aIter it = knownActors.begin(); it != knownActors.end(); it++)
    {
        it->second->visited = false;
        it->second->baconNumber = INF;
    }

    for(mIter it = knownMovies.begin(); it != knownMovies.end(); it++)
    {
        it->second->visited = false;
        it->second->depth = INF;
    }
}

/**************************************************************************//**
 * @author Chris Kolegraff
 *
 * @par Description:
 * Resets the visited variable for all of the actors and movies to false.
 *****************************************************************************/
void movieSet::ResetVisited()
{
    for(aIter it = knownActors.begin(); it != knownActors.end(); it++)
    {
        it->second->visited = false;
    }

    for(mIter it = knownMovies.begin(); it != knownMovies.end(); it++)
    {
        it->second->visited = false;
    }
}
