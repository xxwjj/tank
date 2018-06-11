#include "ai.h"
#include "debug.h"
#include <cstdlib>
#include "graphSearchAStar.h"
#include "denseGraph.h"
#include <algorithm>
#include <map>

class digBoxInfo
{
    public:
        digBoxInfo(int a, int b, int c, int d, int s):playerId(a), diamonPos(b), cost(c), boxType(d), score(s){}
        digBoxInfo(){}
        bool operator < (const digBoxInfo b) const
        {
            if (boxType != b.boxType)
            {
                return boxType == E_STAR;
            }
            if (cost != b.cost)
            {
                return cost < b.cost;
            }
            if (score != b.score)
            {
                return score > b.score;
            }
            if (playerId != b.playerId)
            {
                return playerId < b.playerId;
            }
            return diamonPos < b.diamonPos;
        }
    public:
        int playerId;
        int diamonPos;
        int cost;
        int boxType;
        int score;
};

class DiamondGraph: public DenseGraph<Cell>
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

typedef Graph_SearchAStar<DiamondGraph, Heuristic_Manhattan> CellDenseGraphAStar;

static inline void updateActionPriority(Action &action, PRIORITY_DIAMON forward)
{
    action.priDiamon = forward;
    if (NOT_FORWARD_DIAMON != forward)
    {
        log("\t\tDig diamond: update player(%d) move(%d, %d) forward diamond.\n", action.player_id, action.move.col, action.move.row);
    }
}

void calcDigBoxesInfo(Leg &leg, std::vector<digBoxInfo> &digInfo)
{
    GameMap &mapInfo = leg._map_info;
    MatrixMap &matrix = leg._matrix_map;
    DiamondVec &boxes = mapInfo._diamonds;
    StarVec stars = mapInfo._stars;

    int diamonPosId = 0;
    int starPosId = 0;

    int movePosId = 0;
    std::list<int> path;
    int cost;
    
    DiamondGraph graph;
    graph.LoadFromMatrix(matrix._cells, matrix._height, matrix._width);
    
    for (PlayerVecIt f = mapInfo._friend_players.begin(); f != mapInfo._friend_players.end(); f++)
    {
        if (!f->_alive) {continue;}
        if (f->_have_super_bullet)
        {
            continue;
        }

        CellDenseGraphAStar astar(graph, matrix.pos2Index(f->_pos), -1);
        //计算金币的花费信息
        for (size_t j = 0; j < boxes.size(); j++)
        {
            diamonPosId = matrix.pos2Index(boxes[j]._pos);
            cost = astar.GetCostToPos(diamonPosId);
            if (INVALID_COST != cost)
            {
                digInfo.push_back(digBoxInfo(f->_id, diamonPosId, cost, E_DIAMOND, boxes[j]._count));
            }            
        }
        
        //计算超级子弹的花费信息
        for (size_t j = 0; j < stars.size(); j++)
        {
            starPosId = matrix.pos2Index(stars[j]._pos);
            cost = astar.GetCostToPos(starPosId);
            if (INVALID_COST != cost)
            {
                digInfo.push_back(digBoxInfo(f->_id, starPosId, cost, E_STAR, 10));
            }            
        }
    }
    std::sort(digInfo.begin(), digInfo.end());
}

PRIORITY_DIAMON getCostPriority(MatrixMap &matrix, int start, int end, int minCost)
{
    DiamondGraph graph;
    graph.LoadFromMatrix(matrix._cells, matrix._height, matrix._width);

    CellDenseGraphAStar astar(graph, start, end);
    if (astar.GetCostToTarget() == minCost)
    {
        return FORWARD_DIAMON;
    }
    if (astar.GetCostToTarget() == minCost + 1)
    {
        return KEEP_FORWARD_DIAMOND;
    }
    return NOT_FORWARD_DIAMON;
}

void dispatchDiamonByCost(std::vector<digBoxInfo> &digInfo, std::map<int, digBoxInfo>& playerDiamond, MatrixMap &matrix, size_t num)
{
    std::set<int> diamonSet;
    Cell *cell;

    for (std::vector<digBoxInfo>::iterator it = digInfo.begin(); it != digInfo.end(); it++)
    {
        if (0 == it->cost)
        {
            continue;
        }
        if (playerDiamond.size() >= num)
        {
            return;
        }
        if (diamonSet.end() != diamonSet.find(it->diamonPos))
        {
            continue;
        }

        cell = matrix.getCell(it->diamonPos);
        //如果宝物地图上有TANK，不发送
        if (E_FRIEND == cell->_type || E_FOE == cell->_type)
        {
            continue;
        }

        if (playerDiamond.end() != playerDiamond.find(it->playerId))
        {
            continue;
        }
        diamonSet.insert(it->diamonPos);
        playerDiamond[it->playerId] = *it;
    }
}

void runDigDiamond(Leg &leg)
{
    GameMap &mapInfo = leg._map_info;
    MatrixMap &matrix = leg._matrix_map;
    int playerId;

    std::vector<digBoxInfo> digInfo;
    std::map<int, digBoxInfo> playerDiamond;
    std::map<Vector2D, PRIORITY_DIAMON> moveMap;

    calcDigBoxesInfo(leg, digInfo);
    dispatchDiamonByCost(digInfo, playerDiamond, matrix, mapInfo._friend_players.size());

    for (PlayerVecIt f = mapInfo._friend_players.begin(); f != mapInfo._friend_players.end(); f++)
    {
        if (!f->_alive) {continue;}
        if (playerDiamond.end() == playerDiamond.find(f->_id))
        {
            continue;
        }
        log("\tPlayer %d, pos(%d, %d):\n", f->_id, f->_pos.col, f->_pos.row);
        playerId = f->_id;
        moveMap.clear();
        Vector2D pos = f->_pos;
        Vector2D move = pos - f->_pos;
        Vector2D diamondPos = matrix.index2Pos(playerDiamond[playerId].diamonPos);
        log("\t\tGoal Diamon: pos(%d, %d), type(%d).\n", diamondPos.col, diamondPos.row, playerDiamond[playerId].boxType);

        leg.EnterActionSection();
        for (ActionVecIt a = f->actionList.begin(); a != f->actionList.end(); a++)
        {
            if (a->disable) {continue;}
            if (moveMap.end() == moveMap.find(a->move))
            {
                moveMap[a->move] = getCostPriority(matrix, 
                                                    matrix.pos2Index(pos + a->move), 
                                                    playerDiamond[playerId].diamonPos, 
                                                    playerDiamond[playerId].cost - 1);
            }
            updateActionPriority(*a, moveMap[a->move]);
        }
        leg.LeaveActionSection();
    }
}

bool DigDiamond::run(Leg &leg)
{
    log("AI:DigDiamond:\n");
    runDigDiamond(leg);
    return true;
}
