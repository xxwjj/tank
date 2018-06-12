#include "ai.h"
#include "debug.h"

static Vector2D dirArray[MAX_VEC_DIR_NUM] = {VECTOR_UP, VECTOR_DOWN, VECTOR_LEFT, VECTOR_RIGHT, VECTOR_ZERO};
void ExcludeFoolSteps::calcPlayerNumInVision(Leg & leg)
{
    std::vector<Player>::iterator iter;
    for (iter = leg._map_info._friend_players.begin();iter!= leg._map_info._friend_players.end();iter++)
    {
        if (iter->_alive != true)
        {
            continue;
        }
        for (int i = 0; i<MAX_VEC_DIR_NUM; i++)
        {
            iter->_totalEnemyNum[i] = leg._matrix_map.getTerrainNumInVision(iter->_pos+dirArray[i], E_FOE);
            iter->_totalFriendNum[i] = leg._matrix_map.getTerrainNumInVision(iter->_pos+dirArray[i], E_FRIEND);
        }
    }
}

bool ExcludeFoolSteps::run(Leg & leg)
{
    log("ExcludeFoolSteps\n");
    calcPlayerNumInVision(leg);
    std::vector<Player>::iterator iter;
    leg.EnterActionSection();
    for (iter = leg._map_info._friend_players.begin();iter!= leg._map_info._friend_players.end();iter++)
    {
        if (iter->_alive != true)
        {
            continue;
        }
        Vector2D pos = iter->_pos;
        Vector2D nextPos;
#ifdef _DEBUG
        log("Player:%d,  action num=%d\n",iter->_id, iter->actionList.GetActionCount());
#endif
        for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
        {
            if (a->disable) {continue;}
            //move 方向有效性检查
            nextPos = pos + a->move;
            if (!leg._matrix_map.isValidCoord(nextPos))
            {
                DISABLE_ACTION(a);
                continue;
            }
            if (E_RIVER == leg._matrix_map.getTerrain(nextPos))
            {
                DISABLE_ACTION(a);
                continue;
            }
            if (E_WALL == leg._matrix_map.getTerrain(nextPos))
            {
                DISABLE_ACTION(a);
                continue;
            }
            if (E_FRIEND == leg._matrix_map.getTerrain(nextPos))
            {
                a->priEscape = E_EVENT;
                continue;
            }
        }
#if 0 //这个分支放开跟叶小节pk不占优，不要开放
        if (iter->_mostDangerousDistance >0 && iter->_mostDangerousDistance<=6)
        {//
            for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
            {
                if (a->disable) {continue;}
                if (a->fire != iter->_foeDir)
                {
                    a->priEscape = E_WARNING;
                    continue;
                }
            }
       }
#endif
        if (iter->_foeDistance>0 && iter->_foeDistance <= 3)
        {//如果敌人距离小于3，优先攻击敌人
            for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
            {
                if (a->disable) {continue;}
                if (a->fire != iter->_foeDir)
                {
                    DISABLE_ACTION(a);
                    continue;
                }
            }
        }
        else if (iter->_foeBulletDistance >0 && iter->_foeBulletDistance <=3)
        {//如果子弹的距离小于2，对消子弹
            for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
            {
                if (a->disable) {continue;}
                if (a->fire != iter->_foeBulletDir)
                {
                    DISABLE_ACTION(a);
                    continue;
                }
            }
        }
		
		 if (iter->_have_super_bullet && iter->_foeDistance >0 && iter->_foeDistance <=2)
       {
#ifdef _DEBUG
            log("Player:%d,  send super bullet\n",iter->_id);
#endif
            for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
            {
                if (a->disable) {continue;}
                if (!a->_is_super_bullet)
                {
                    DISABLE_ACTION(a);
                    continue;
                }
            }
       }
       else if(iter->_have_super_bullet )
       {//如果移动的方向前面是星星，并且周围没有其他坦克，包括敌、我，就发超级子弹
           for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
           {
               if (a->disable) {continue;}
               if (!a->_is_super_bullet //没有发送超级子弹
                   && leg._map_info.findStarAtPos(iter->_pos+a->move)  //目标位置是超级子弹道具
                   && leg._matrix_map.getPlayerNumInOneStep(iter->_pos+a->move)==1)//目标位置周围一步的坦克个数大于一（确保自己能够抢到道具）
               {
#ifdef _DEBUG
                   log("is Star=%d, player num=%d\n",leg._map_info.findStarAtPos(iter->_pos+a->move),leg._matrix_map.getPlayerNumInOneStep(iter->_pos+a->move));
#endif
                   DISABLE_ACTION(a);
                   continue;
               }
           }
       }
    }

    leg.LeaveActionSection();

#if _DEBUG
#if 0
    if (iterFriend->actionList.GetActionCount()==0)
    {
        log("Actions: Player:%d,  no action!!!!!\n",iterFriend->_id);
    }
#else
    iter = leg._map_info._friend_players.begin();
    for (;iter!= leg._map_info._friend_players.end();iter++)
    {
        for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
        {
            if (a->disable) {continue;}
            log("Round(%d): Player:%d, Action fire(%d,%d), move(%d,%d), priority(%d,%d,%d,%d)\n",
                leg._round_id, iter->_id,
                a->fire.col,a->fire.row,
                a->move.col,a->move.row,
                a->priEscape, a->priDiamon, a->priDeadEnd, a->priSiege
                );
        }
    }
#endif
#endif



    return true;
}
