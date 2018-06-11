#include "ai.h"
#include "debug.h"

static Vector2D dirArray[MAX_VEC_DIR_NUM] = {VECTOR_UP, VECTOR_DOWN, VECTOR_LEFT, VECTOR_RIGHT, VECTOR_ZERO};

void RiskAssess::procFoeBullet(Leg & leg)
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
            int totalBullet = 0;///leg._tick_bullet.getFoeBulletNum(nextPos,1); //TODO:
            totalBullet += leg._tick_bullet.getFoeBulletNum(nextPos,2);
            totalBullet += leg._tick_bullet.getFoeBulletNum(nextPos,3);
#ifdef _DEBUG
            log("Play id=%d, dir=%d, total=%d,tick0=%d,tick1=%d,tick2=%d,tick3=%d\n",iter->_id, i,totalBullet,\
                leg._tick_bullet.getFoeBulletNum(nextPos,0),\
                leg._tick_bullet.getFoeBulletNum(nextPos,1),\
                leg._tick_bullet.getFoeBulletNum(nextPos,2),\
                leg._tick_bullet.getFoeBulletNum(nextPos,3));
#endif
            if (totalBullet>2)
            {
                pri = E_EMERG;
            }
            else if (totalBullet>0)
            {
                pri = E_ERROR;
            }
            else
            {
                pri = E_INFO;
                //如果移动方向与原点tick3记录的子弹方向一样，这这种情况不能向子弹方向移动
                if (dirArray[i]==leg._tick_bullet.getFoeBulletDir(iter->_pos, 3))
                {
#ifdef _DEBUG
                    log("leg._tick_bullet.getFoeBulletDir  hahaha\n");
#endif
                    pri = E_WARNING;//将优先级升为错误
                }
            }
            for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
            {
                if (a->disable) {continue;}
                if (a->move == dirArray[i])
                {
                    if (pri==E_INFO && iter->_ownBulletCalcDir!=VECTOR_ZERO  && iter->_ownBulletCalcDir==dirArray[i])
                    {//表示这个方向的子弹为0，但是需要继续考虑是否这个方向是子弹对消方向
                        if (iter->_ownBulletCalcSuper && a->fire==iter->_ownBulletCalcDir && !a->_is_super_bullet)
                        {
                            DISABLE_ACTION(a);
                            continue;
                        }
                        if (a->fire != iter->_ownBulletCalcDir)
                        {
                            a->priEscape = E_ERROR;//
                            continue;
                        }
                    }
                    a->priEscape = pri;
                    continue;
                }
            }
        }
    }
}


//根据敌我个数悬殊，设定位置的危险程度
void RiskAssess::procFoeOwnDiff(Leg & leg)
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
            if (iter->_totalEnemyNum-iter->_totalFriendNum>2)
            {
                pri = E_EMERG;
            }
            else if (iter->_totalEnemyNum-iter->_totalFriendNum>1)
            {
                pri = E_ERROR;
            }
            else if (iter->_totalEnemyNum-iter->_totalFriendNum>0)
            {
                pri = E_WARNING;
            }
            else if (iter->_totalEnemyNum==iter->_totalFriendNum)
            {
                pri = E_EVENT;
            }
            else
            {
                pri = E_INFO;
            }
            for (ActionVecIt a = iter->actionList.begin(); a != iter->actionList.end(); a++)
            {
                if (a->disable) {continue;}
                if (a->move == dirArray[i])
                {
                    if (pri > a->priEscape)
                    {
                        a->priEscape = pri;
                    }
                    continue;
                }
            }
        }
    }
}

bool RiskAssess::run(Leg & leg)
{
    log("ListNextSteps\n");

    //根据周边敌人子弹数刷新权重值
    procFoeBullet(leg);


    //检测并处理周边位置的敌我悬殊情况，刷新权重值
    procFoeOwnDiff(leg);


#if _DEBUG
    std::vector<Player>::iterator iter;
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
    return true;
}