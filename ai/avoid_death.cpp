#include "ai.h"
#include "debug.h"
#include <cstdlib>
#include "graphSearchAStar.h"
#include "denseGraph.h"
#include <algorithm>
#include <map>

static Vector2D dirArray[MAX_VEC_DIR_NUM] = {VECTOR_UP, VECTOR_DOWN, VECTOR_LEFT, VECTOR_RIGHT, VECTOR_ZERO};

//防必死局面
void procDedead(Leg & leg)
{
    std::vector<Player>::iterator iter;
    iter = leg._map_info._friend_players.begin();
    for (;iter!= leg._map_info._friend_players.end();iter++)
    {
        if (iter->_alive != true)
        {
            continue;
        }
        for (int i=0; i<MAX_VEC_DIR_NUM; i++)
        {
            Vector2D nextPos;
            enum PRIORITY_ESCAPE pri=E_INFO;
            nextPos = iter->_pos + dirArray[i];
            if (leg._matrix_map.getTerrainNumInDistance(nextPos, E_FOE)>=2
                || (!iter->_have_super_bullet&&leg.getSuperPlayerNum(nextPos)>0))
            {
                for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
                {
                    if (a->disable) {continue;}
                    if (a->move == dirArray[i])
                    {
                        a->priEscape = E_WARNING;
                        continue;
                    }
                }
            }
        }
    }
}


bool AvoidDeath::run(Leg &leg)
{
    log("AI:AvoidDeath:\n");
    procDedead(leg);
    return true;
}

