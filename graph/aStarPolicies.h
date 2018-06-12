#pragma once
#pragma warning (disable:4786)

#include "grid.h"

//-----------------------------------------------------------------------------
//the euclidian heuristic (straight-line distance)
//-----------------------------------------------------------------------------
class Heuristic_Manhattan
{
public:

    Heuristic_Manhattan(){}

    //calculate the straight line distance from node nd1 to node nd2
    template <class graph_type>
    static int Calculate(const graph_type& G, int nd1, int nd2)
    {
        return ManhattanDistance(G.GetPos(nd1), G.GetPos(nd2));
    }
};


//-----------------------------------------------------------------------------
//you can use this class to turn the A* algorithm into Dijkstra's search.
//this is because Dijkstra's is equivalent to an A* search using a heuristic
//value that is always equal to zero.
//-----------------------------------------------------------------------------
class Heuristic_Dijkstra
{
public:

    template <class graph_type>
    static int Calculate(const graph_type& G, int nd1, int nd2)
    {
        return 0;
    }
};