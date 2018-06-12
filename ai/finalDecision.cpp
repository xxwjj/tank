#include "ai.h"
#include "debug.h"
#include <map>
#include <algorithm>

enum PRIORITY_COMPARE
{
    PRIORITY_LOW,
    PRIORITY_EQUEUE,
    PRIORITY_HIGH
};

typedef PRIORITY_COMPARE (*FN_COMPARE)(ActionVecIt &a, ActionVecIt& b);
typedef std::map<int, std::map<Vector2D, int>> PlayerMoveActionMap;

static Vector2D dirArray[MAX_VEC_DIR_NUM] = {VECTOR_UP, VECTOR_DOWN, VECTOR_LEFT, VECTOR_RIGHT, VECTOR_ZERO};

bool compareAction(const Action &a, const Action &b)
{
    if (a.disable)
    {
        return false;
    }
    if (a.priEscape != b.priEscape
        && (a.priEscape > E_EVENT || b.priEscape > E_EVENT))
    {
        return a.priEscape < b.priEscape;
    }

    if (a.priDiamon != b.priDiamon)
    {
        return a.priDiamon > b.priDiamon;
    }

    if (a.priDeadEnd != b.priDeadEnd)
    {
        return a.priDeadEnd > b.priDeadEnd;
    }

    if (a.priEscape != b.priEscape)
    {
        return a.priEscape < b.priEscape;
    }

    if (a.priSiege != b.priSiege)
    {
        return a.priSiege > b.priSiege;
    }

    if (a.fire != b.fire)
    {
        return a.fire.LengthSq() > b.fire.LengthSq();
    }
    return false;
}

PRIORITY_COMPARE compareEscape(ActionVecIt &a, ActionVecIt& b)
{
    if (a->priEscape == b->priEscape)
    {
        return PRIORITY_EQUEUE;
    }

    if (a->priEscape <= E_EVENT && b->priEscape <= E_EVENT)
    {
        return PRIORITY_EQUEUE;
    }
    
    if (a->priEscape < b->priEscape)
    {
        return PRIORITY_HIGH;
    }
    return PRIORITY_LOW;
}

PRIORITY_COMPARE compareDiamon(ActionVecIt &a, ActionVecIt& b)
{
    if (a->priDiamon == b->priDiamon)
    {
        return PRIORITY_EQUEUE;
    }
    
    if (a->priDiamon > b->priDiamon)
    {
        return PRIORITY_HIGH;
    }

    return PRIORITY_LOW;
}

PRIORITY_COMPARE compareDeadEnd(ActionVecIt &a, ActionVecIt& b)
{
    if (a->priDeadEnd == b->priDeadEnd)
    {
        return PRIORITY_EQUEUE;
    }

    if (a->priDeadEnd > b->priDeadEnd)
    {
        return PRIORITY_HIGH;
    }

    return PRIORITY_LOW;
}

PRIORITY_COMPARE compareRisk(ActionVecIt &a, ActionVecIt& b)
{
    if (a->priEscape == b->priEscape)
    {
        return PRIORITY_EQUEUE;
    }

    if (a->priEscape < b->priEscape)
    {
        return PRIORITY_HIGH;
    }
    return PRIORITY_LOW;
}

PRIORITY_COMPARE compareSiege(ActionVecIt &a, ActionVecIt& b)
{
    if (a->priSiege == b->priSiege)
    {
        return PRIORITY_EQUEUE;
    }

    if (a->priSiege > b->priSiege)
    {
        return PRIORITY_HIGH;
    }

    return PRIORITY_LOW;
}

PRIORITY_COMPARE compareSuperBullet(ActionVecIt &a, ActionVecIt& b)
{
    if (!a->_is_super_bullet && b->_is_super_bullet)
    {
        return PRIORITY_HIGH;
    }
    if (a->_is_super_bullet && !b->_is_super_bullet)
    {
        return PRIORITY_LOW;
    }

    return PRIORITY_EQUEUE;
}

PRIORITY_COMPARE compareFire(ActionVecIt &a, ActionVecIt& b)
{
    if (a->fire.LengthSq() == b->fire.LengthSq())
    {
        return PRIORITY_EQUEUE;
    }

    if (a->fire.LengthSq() > b->fire.LengthSq())
    {
        return PRIORITY_HIGH;
	}
	
	return PRIORITY_LOW;
}

PRIORITY_COMPARE compareMove(ActionVecIt&a,ActionVecIt&b)
{
	if(a->move.LengthSq() == b->move.LengthSq())
	{
		return PRIORITY_EQUEUE;
	}
	if(a->move.LengthSq() > b->move.LengthSq())
	{
		return PRIORITY_HIGH;
	}
	return PRIORITY_LOW;
}

FN_COMPARE s_priorityStrategy[] = 
{
    compareEscape,
    compareDiamon,
    compareDeadEnd,
    compareRisk,
    compareSiege,
    compareSuperBullet,
    compareMove,
    compareFire,
    NULL,
};

bool checkMoveDireSole(std::map<int, map<Vector2D, int>> &playerValidMoveAction, int playerId)
{
    int validMoveDire = 0;
    for (int i = 0; i < MAX_VEC_DIR_NUM; i++)
    {
        if (playerValidMoveAction[playerId][dirArray[i]])
        {
            validMoveDire++;
        }
    }
    return 1 == validMoveDire;
}

bool checkActionConflict(Player &myPlayer, Action &action, PlayerVec &players, PlayerMoveActionMap &playerValidMoveAction)
{
    if (checkMoveDireSole(playerValidMoveAction, myPlayer._id))
    {
        return false;
    }

    Vector2D myPos = myPlayer._pos + action.move;
    Vector2D otherPos;

    for (PlayerVecIt f = players.begin(); f != players.end(); f++)
    {
        if (f->_id == myPlayer._id)
        {
            continue;
        }

        //如果对方行动位置冲突
        for (int i = 0; i < MAX_VEC_DIR_NUM; i++)
        {
            if (!playerValidMoveAction[f->_id][dirArray[i]])
            {
                continue;
            }
            otherPos = f->_pos + dirArray[i];
            if (otherPos == myPos)
            {
                log("WARN: Player (%d) conflict with player(%d).\n", myPlayer._id, f->_id);
                return true;
            }
        }
    }
    return false;
}

void disablePlayerMoveDire(Player &f, Vector2D dire)
{
    for (ActionVecIt a = f.actionList.begin(); a != f.actionList.end(); a++)
    {
        if (!a->disable && a->move == dire)
        {
            DISABLE_ACTION(a);
        }
    }
}


ActionVecIt dispalConflictAndChooseBest(Leg &leg, Player &player, PlayerMoveActionMap &playerValidMoveAction, FN_COMPARE strategy)
{
    PRIORITY_COMPARE pri;
    ActionVecIt bestActionIt = player.actionList.end();

    for (ActionVecIt a = player.actionList.begin(); a != player.actionList.end(); a++)
    {
        if (a->disable) {continue;}

        if (checkActionConflict(player, *a, leg._map_info._friend_players, playerValidMoveAction))
        {
            disablePlayerMoveDire(player, a->move);
            playerValidMoveAction[player._id][a->move] = 0;
            continue;
        }

        if (bestActionIt == player.actionList.end())
        {
            bestActionIt = a;
            continue;
        }

        pri = strategy(bestActionIt, a);
        if(PRIORITY_LOW == pri)
        {
            bestActionIt = a;
        }
    }
    return bestActionIt;
}

//根据指定局面进行决策
void runSpecificDecision(Leg &leg, PlayerMoveActionMap &playerValidMoveAction, FN_COMPARE* strategyArray)
{
    GameMap& mapInfo = leg._map_info;
    ActionVecIt bestActionIt;
    PRIORITY_COMPARE pri;

    for (int i = 0; NULL != strategyArray[i]; i++)
    {
        log("Stategy(%d):\n", i);
        for (PlayerVecIt f = mapInfo._friend_players.begin(); f != mapInfo._friend_players.end(); f++)
        {
            if (!f->_alive) {continue;}

            //去掉所有冲突的ACTION,并选择一个最好的ACTION作为PK目标
            bestActionIt = dispalConflictAndChooseBest(leg, *f, playerValidMoveAction, strategyArray[i]);

            //根据最好的ACTION来PK所有的ACTION
            for (ActionVecIt a = f->actionList.begin(); a != f->actionList.end(); a++)
            {
                if (a->disable) {continue;}
                
                pri = strategyArray[i](bestActionIt, a);
                switch(pri)
                {
                case PRIORITY_HIGH:
                    playerValidMoveAction[f->_id][a->move] --;
                   /* log("Winner Action Player:%d, fire(%d,%d,%d), move(%d,%d), priority(%d,%d,%d,%d) at strategy(%d).\n",
                        f->_id,
                        bestActionIt->fire.col, bestActionIt->fire.row, bestActionIt->_is_super_bullet,
                        bestActionIt->move.col,bestActionIt->move.row,
                        bestActionIt->priEscape, bestActionIt->priDiamon, bestActionIt->priDeadEnd, bestActionIt->priSiege, i
                        );*/
                    DISABLE_ACTION(a);
                    break;
                case PRIORITY_LOW:
                    /*log("Winner Action Player:%d, fire(%d,%d,%d), move(%d,%d), priority(%d,%d,%d,%d) at strategy(%d).\n",
                        f->_id,
                        a->fire.col, a->fire.row, a->_is_super_bullet,
                        a->move.col,a->move.row,
                        a->priEscape, a->priDiamon, a->priDeadEnd, a->priSiege, i
                        );*/
                    DISABLE_ACTION(bestActionIt);
                    playerValidMoveAction[f->_id][bestActionIt->move] --;
                    bestActionIt = a;
                    break;
                case PRIORITY_EQUEUE:
                    break;
                }
            }
        }
    }
}

void runFinalDecision(Leg&leg)
{
	std::map<int,map<Vector2D,int>> playerValidMoveAction;
	GameMap& mapInfo = leg._map_info;
    Vector2D myPos, otherPos;
    
    //初始化每个player每个MOVE方向的ACTION数量为0
    for (PlayerVecIt f = mapInfo._friend_players.begin(); f != mapInfo._friend_players.end(); f++)
    {
        for (int i = 0; i < MAX_VEC_DIR_NUM; i++)
        {
            playerValidMoveAction[f->_id][dirArray[i]] = 0;
        }
    }
    
    //刷新每个player每个MOVE方向的ACTION数量
    for (PlayerVecIt f = mapInfo._friend_players.begin(); f != mapInfo._friend_players.end(); f++)
    {
        if (!f->_alive)
        {
            continue;
        }
        std::sort(f->actionList.begin(), f->actionList.end(), compareAction);

        for (ActionVecIt a = f->actionList.begin(); a != f->actionList.end(); a++)
        {
            if (a->disable) {continue;}
            playerValidMoveAction[f->_id][a->move] ++;
        }
    }
	//根据当前局面进行决策
    switch(leg.sit)
    {
    case NORMAL_SITUATION:
    case SUICIDE:
    case NOT_ATTACK:
        runSpecificDecision(leg, playerValidMoveAction, s_priorityStrategy);
        break;
    default:
        log("ERROR: Invalid situation(%d).", leg.sit);
        break;
    }
    
    //根据当前局面进行决策
#ifdef _DEBUG
    for (PlayerVecIt f = mapInfo._friend_players.begin(); f != mapInfo._friend_players.end(); f++)
    {
        int validNum = 0;
        if (!f->_alive) {continue;}
        for (ActionVecIt a = f->actionList.begin(); a != f->actionList.end(); a++)
        {
            if (a->disable) {continue;}
            validNum++;
            log("Final decision Action Player:%d, fire(%d,%d,%d), move(%d,%d), priority(%d,%d,%d,%d)\n",
                f->_id,
                a->fire.col, a->fire.row, a->_is_super_bullet,
                a->move.col,a->move.row,
                a->priEscape, a->priDiamon, a->priDeadEnd, a->priSiege
                );
            Vector2D myPos = f->_pos + a->move;
            for (PlayerVecIt f1 = mapInfo._friend_players.begin(); f1 != mapInfo._friend_players.end(); f1++)
            {
                if (!f1->_alive || f == f1) {continue;}
                for (ActionVecIt a1 = f1->actionList.begin(); a1 != f1->actionList.end(); a1++)
                {
                    if (a1->disable) {continue;}
                    Vector2D hisPos = f1->_pos + a1->move;
                    if (myPos == hisPos)
                    {
                        log("[CRITICAL] Final decision Conflict:\n");
                        log("Action Player:%d, fire(%d,%d,%d), move(%d,%d), priority(%d,%d,%d,%d)\n",
                            f->_id,
                            a->fire.col, a->fire.row, a->_is_super_bullet,
                            a->move.col,a->move.row,
                            a->priEscape, a->priDiamon, a->priDeadEnd, a->priSiege
                            );
                        log("Action Player:%d, fire(%d,%d,%d), move(%d,%d), priority(%d,%d,%d,%d)\n",
                            f1->_id,
                            a1->fire.col, a1->fire.row, a1->_is_super_bullet,
                            a1->move.col,a1->move.row,
                            a1->priEscape, a1->priDiamon, a1->priDeadEnd, a1->priSiege
                            );
                    }                    
                }
            }
        }
        if (!validNum)
        {
            log("CRITICAL[BUG]: Player(%d) has none final descsion.\n",f->_id);
        }
    }
#endif
}


bool FinalDecision::run(Leg&leg)
{
	runFinalDecision(leg);
	return true;
}




