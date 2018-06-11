#include "ai.h"

bool ContiDecision::run(Leg & leg)
{
    for (PlayerVecIt iter = leg._map_info._friend_players.begin();iter!=leg._map_info._friend_players.end();iter++)
    {
        if (iter->_alive)
        {
            iter->MaintainList();
        }
    }

    CheckConflictActions(leg);

    std::vector<Player>::iterator iter;
    iter = leg._map_info._friend_players.begin();
    for (; iter != leg._map_info._friend_players.end(); iter++)
    {
        if (!iter->_alive)
            continue;
        if ((*iter).actionList.GetActionCount() > 1)
            return true;
    }
    return false;
}
 
void ContiDecision::CheckConflictActions(Leg & leg)
{
    for (PlayerVecIt iter = leg._map_info._friend_players.begin();iter!=leg._map_info._friend_players.end();iter++)
    {
        if (iter->_alive && iter->moveList.size()==1)
        {
            Vector2D only_pos = iter->_pos + (*iter->moveList.begin());
            GivePosTo(leg, only_pos, iter);
        }
    }
}

void ContiDecision::GivePosTo(Leg& leg, Vector2D pos,PlayerVecIt player)
{
    for (PlayerVecIt player_iter = leg._map_info._friend_players.begin();player_iter!=leg._map_info._friend_players.end();player_iter++)
    {
        if (!player_iter->_alive)
            continue;

        if (player_iter == player)
            continue;

        Vector2D expect_move = pos - player_iter->_pos;
        //不在移动范围内
        if (expect_move.LengthSq()>1)
            continue;

        if (player_iter->moveList.count(expect_move) > 0 && player_iter->moveList.size() ==1)
        {
            //TODO:两个player抢占一个位置
            continue;
        }

        //包含了冲突move
        if (player_iter->moveList.count(expect_move) > 0)
        {
            if (player_iter->actionList.GetActionCount() <=1)
                continue;
            
            for (ActionVecIt action_iter = player_iter->actionList.begin();action_iter!=player_iter->actionList.end();action_iter++)
            {                
                if (action_iter->disable)
                    continue;
                if (action_iter->move == expect_move && player_iter->actionList.GetActionCount() > 1)
                    player_iter->actionList.DisableAction(action_iter);
            }
            player_iter->moveList.erase(expect_move);
            
            //如果产生了新的唯一移动的player
            if (player_iter->moveList.size()==1)
            {
                Vector2D next_pos = player_iter->_pos + (*player_iter->moveList.begin());
                GivePosTo(leg,next_pos,player_iter);
            }
        }
    } 
}