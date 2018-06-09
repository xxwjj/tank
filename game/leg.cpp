#include "game.h"
#include "debug.h"

using namespace std;

int Leg::getSuperPlayerNum(Vector2D &pos) {
    int cnt = 0;
    for (int i = pos.col -2; i<= pos.col +2; i++) {
        for (int j = pos.row - 2; j <= pos.row + 2; j++)
        {
            if (!_matirx_map.isValidCoord(i,j))
            {
                continue;
            }
            if (_matrix_map.getTerrain(i,j) == E_FOE && (abs(pos.col -i)+abs(pos.row-j)<=3))
            {
                for (PlayerVecIt iter = _map_info._enemy_players.begin(); iter != _map_info._enemy_players.end(); iter++)
                {
                    if (!iter->_alive)
                        continue;
                    if (iter->_have_super_bullet && iter->_pos.col == i && iter->_pos.row == j)
                    {
                        cnt++;
                    }
                }
            }
        }
    }
    return cnt;
}

int Leg::getNearestBulletDistance(Vector2D &pos, Vector2D &dir, E_BULLET type)
{
    int distance = 0;
    Vector2D nextPos = pos;
    nextPos += dir;
    Cell *pCell = _matrix_map.getCell(&nextPos);
    while (NULL != pCell)
    {
        distance++;
        if (pCell->_bullet = type)
        {
            if (_map_info.findOppositeBulletAtPos(nextPos, dir, type))
            {
                return distance;
            }
            if (pCell->_type == E_WALL || pCell->type == E_BRICK)
            {
                break;
            }
            nextPos += dir;
            pCell = _matrix_map.getCell(&nextPos);
        }
    }
    return 0;
}

bool Leg::haveSuperBulletInDir(Vector2D &pos, Vector2D &dir, E_BULLET type, int dis) {
    Vector2D nextPos = pos;
    nextPos += dir;
    int distance = 0;
    Cell *pCell =_matrix_map.getCell(nextPos);
    while (NULL != pCell && distance < dis)
    {
        distance++;
        if (pCell->_bullet == type)
        {
            if (_map_info.findOppositeBulletAtPos(nextPos, dir,type, true)){
                return true;
            }
        }
        if (pCell->type == E_WALL || pCell->_type == E_BRICK)
        {// yu dao chang ai , bu ji xu zha xun
            break;
        }
        nextPos += dir;
        pCell = _matrix_map.getCell(&nextPos);
    }
    return false;
}

int Leg::haveBulletNumInDir(Vector2D &pos, Vector2D &dir, E_BULLET type, int dis) {
    Vector2D nextPos = pos;
    nextPos += dir;
    int bulletNum = 0;
    int distance =0;

    Cell * pCell = _matrix_map.getCell(&nextPos);
    while (NULL != pCell && distance < dis)
    {
        distance++;
        if (pCell->_bullet == type)
        {
            if (_map_info.findOppositeBulletAtPos(nextPos, dir ,type, false))// normal bullet
            {
                bulletNum++;
                continue;
            }
        }
        if (pCell->type == E_WALL || pCell->_type == E_BRICK)
        {// yu dao chang ai , bu ji xu zha xun
            break;
        }
        nextPos += dir;
        pCell = _matrix_map.getCell(&nextPos);
    }
    return bulletNum;
}