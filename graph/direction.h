#pragma once
#pragma warning (disable:4786)

class Direction
{
public:
    Direction(int x, int y):dRow(x), dCol(y){}
    int dRow;
    int dCol;
};

inline  Direction * getNeighbourDirection(int d)
{
    static Direction g_neightbourDirection4[] = {Direction(-1, 0),Direction(0, -1),Direction(1, 0),Direction(0,1),};
    static Direction g_neightbourDirection6[] = {Direction(1, 1),Direction(-1, -1),Direction(0, -2),Direction(1,-1),Direction(1,1),Direction(0,2)};
    static Direction g_neightbourDirection8[] = {Direction(-1,0), Direction(-1,1), Direction(0,-1), Direction(1,-1),
                                                 Direction(1,0), Direction(1,1), Direction(0,1), Direction(-1,1)};
    if (d == 4)
    {
        return g_neightbourDirection4;
    }
    if (d == 6)
    {
        return g_neightbourDirection6;
    }
    if (d == 8)
    {
        return g_neightbourDirection8;
    }
    return NULL;
}