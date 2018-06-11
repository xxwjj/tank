#pragma once
#pragma warning (disable:4786)


#include <math.h>
#include "debug.h"
#include "define.h"

class Grid
{
public:
    int row;
    int col;
    Grid(){}
    Grid(int a, int b) :row(a), col(b) {}
};


inline int ManhattanDistance(Grid& a, Grid & b)
{
    return abs(a.row - b.row) + abs (a.col - b.col);
}