#include "ai.h"
#include "debug.h"
#include "vector2D.h"
#include <cstdlib>
#include "graphSearchAStar.h"
#include "denseGraph.h"
#include <set>
#include <map>

#define MY_TEAM (4012)
class EscapeGraph: public DenseGraph<Cell>
{
public:
    virtual bool isNodeReachable(int nid) const
    {
        const Cell& cell = GetNode(nid);
        if (E_WALL == cell._type || E_RIVER == cell._type)
        {
            return false;
        }
        return true; 
    }
};

typedef Graph_SearchAStar<EscapeGraph, Heuristic_Manhattan> EscapeGraphAStar;

static inline void updateActionPriority(Action &action, bool existRoad)
{
    if (existRoad)
    {
        action.priDeadEnd = EXIST_ROAD;
    }
    else
    {
        log("\tEscape Dead Road: update player(%d) move(%d, %d) no road.\n", action.player_id, action.move.col, action.move.row);
        action.priDeadEnd = NO_ROAD;
    }
}

void runEscapeFromDeadEnd(Leg &leg)
{
    GameMap &mapInfo = leg._map_info;
    MatrixMap &matrix = leg._matrix_map;
    int playerPosIdx = 0;
    Vector2D nextPos;
    EscapeGraph graph;
    Cell cell;
    std::map<Vector2D, bool> foeReachMap;
    
    graph.LoadFromMatrix(matrix._cells, matrix._height, matrix._width);

    for (PlayerVecIt f = mapInfo._friend_players.begin(); f != mapInfo._friend_players.end(); f++)
    {
        if (!f->_alive) {continue;}
        foeReachMap.clear();
        cell._point = f->_pos;
        cell._type = E_WALL;
        playerPosIdx = matrix.pos2Index(f->_pos);
        graph.SetNode(playerPosIdx, cell);
        log("\tPlayer %d, pos(%d, %d):\n", f->_id, f->_pos.col, f->_pos.row);
        leg.EnterActionSection();
        for (ActionVecIt a = f->actionList.begin(); a != f->actionList.end(); a++)
        {
            if (a->disable) {continue;}
            if (foeReachMap.end() == foeReachMap.find(a->move))
            {
                nextPos = f->_pos + a->move;
                foeReachMap[a->move] = false;
                for (PlayerVecIt foe = mapInfo._enemy_players.begin(); foe != mapInfo._enemy_players.end(); foe++)
                {
                    if (!foe->_alive) {continue;}
                    EscapeGraphAStar astar(graph, matrix.pos2Index(nextPos), matrix.pos2Index(foe->_pos));
                    if (INVALID_COST != astar.GetCostToTarget())
                    {
                        foeReachMap[a->move] = true;
                        break;
                    }
                }
            }
            
            updateActionPriority(*a, foeReachMap[a->move]);
        }
        cell._type = E_FRIEND;
        graph.SetNode(playerPosIdx, cell);
        leg.LeaveActionSection();
    }
}
bool EsacpeDeadEnd::run(Leg &leg)
{
    log("AI:EsacpeDeadEnd:\n");
    runEscapeFromDeadEnd(leg);
    return true;
}

