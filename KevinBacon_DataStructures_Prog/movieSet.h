/*************************************************************************//**
 * @file
 * @brief .h file holds the declarations of the movieSet class, and movie/actor structs
 **************************************************************************/

#pragma once
#include <unordered_map>
#include "functions.h"

// Forward declaration so actor struct knows the movie struct exists
struct movie;

/// Actor struct. Points to movies
/**************************************************************************//**
* @brief Actor struct will point to its surrounding movies. It has a bacon number,
* a visited value, and a name.
*****************************************************************************/
struct actor
{
    /// Name of the actor
    std::string name = "";

    /// Array of pointers to surrounding movies
    std::vector<movie*> movies;

    /// Distance from starting node
    int baconNumber = 999999;

    /// Whether or not the node has been visited
    bool visited = false;

    /// Tracks how many movies the actor is in
    int numMovies = 0;
};

/// Movie struct. Points to actors
/**************************************************************************//**
* @brief Movie struct will point to its surrounding actors. It has a depth,
* a visited value, and a name.
*****************************************************************************/
struct movie
{
    /// Name of the movie
    std::string name = "";

    /// Array of pointers to surrounding actors
    std::vector<actor*> actors;

    /// Distance from starting node
    int depth = 999999;

    /// Whether or not the node has been visited
    bool visited = false;

    /// Tracks how many actors the movie has
    int numActors = 0;
};

/**************************************************************************//**
* @class movieSet
*
* @brief movieSet class holds movies and actors
*
* @brief movieSet class uses unordered_maps to hold a list of actors and a list
* of movies. Each actor will point to a movie that that actor was in, and each
* movie will point to an actor that had cast that actor. This creates a graph
* that can be used to play the Six Degrees of Kevin Bacon game. Other operations are
* also available that can be used to get more information about the created graph.
*****************************************************************************/
class movieSet
{
public:
    /// movieSet constructor
    movieSet();

    /// movieSet destructor
    ~movieSet();

    /// Insert a movie or actor/actress into hash tables, creates actor/movie graph
    void Insert(std::string &name, bool isMovie = false);

    /// Finds if an actor is in the list
    bool KnownActor(std::string &name);

    /// Find if a movie is in the list
    bool KnownMovie(std::string &name);

    /// Makes the entered movie/actor the starting node
    bool MakeStartNode(std::string name);

    /// Generates bacon numbers
    bool NumberActors();

    /// Outputs a histogram of connectivity to the console
    void OutputHist();

    /// Outputs the actors who have the highest bacon numbers
    void OutputLongestPaths();

    /// Outputs a list of movies that that actor has been in
    void OutputVector(std::string name);

    /// Finds the starting node, outputting the names of each node it passes
    void PlayBaconGame(std::string startName);

    /// Reassigns the starting node to the actor/movie
    bool ReassignStartNode(std::string name);

    /// Gets the starting node's name
    std::string StartNodeName();

private:

    /// Count the number of actors that are not related to the starting node
    int CountInfinites();

    /// Delete all the allocated memory
    void DeleteGraph();

    /// Finds the max frequency of the actors
    int FindMaxFreq();

    /// Recursively finds the starting node, given a start position
    void FindTheBacon(actor* act, movie* mov);

    /// Finds an actor in the hash table
    actor* FindActor(std::string &name);

    /// Finds a movie in the hash table
    movie* FindMovie(std::string &name);

    /// Recursively generate bacon numbers for surrounding movies
    void NumberActors(actor *act, int distance);

    /// Recursively generate bacon numbers for surrounding actors
    void NumberActors(movie *mov, int distance);

    /// Reset the bacon numbers for actors/movies
    void ResetBaconNumbers();

    /// Reset the visited bools for actors/movies
    void ResetVisited();


    //##################################################//
    // PRIVATE VARIABLES
    //##################################################//

    /// Holds the output file's name
    std::string fileName;

    /// Holds the actors that have been read in
    std::unordered_map<std::string, actor*> knownActors;

    /// Holds the movies that have been read in
    std::unordered_map<std::string, movie*> knownMovies;

    /// Holds the current in-use movie for inserting
    movie* selectedMovie;

    /// Holds the starting actor for generating bacon numbers
    actor* startingActor;

    /// Holds the starting movie for generating bacon numbers
    movie* startingMovie;

    /// Holds the target movie, used for outputting Six Degrees arrows
    movie* targetMovie;

    /// Holds the target actor, used for outputting Six Degrees arrows
    actor* targetActor;

    /// Tracks if there were updates while generating bacon numbers
    bool updated;

    /// Represents a node with an infinite distance from the start node
    const int INF = 999999;
};
