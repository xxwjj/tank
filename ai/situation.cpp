#include "define.h"
#include "game.h"
#include "ai.h"
#include "debug.h"
#include "situation.h"

bool checkNotAttackSituation(Leg &leg)
{
    int aliveFirend = 0;
    int aliveEnemy = 0;

    for (PlayerVecIt f = leg._map_info._friend_players.begin(); f != leg._map_info._friend_players.end(); f++)
    {
        if (!f->_alive) { continue;}
        aliveFirend++;
    }

    for (PlayerVecIt foe = leg._map_info._enemy_players.begin(); foe != leg._map_info._enemy_players.end(); foe++)
    {
        if (!foe->_alive) { continue;}
        aliveEnemy++;
    }

    if (1 == aliveEnemy && aliveFirend >= aliveEnemy)
    {
        return true;
    }

    return false;
}

bool checkSuicideSituation(Leg &leg)
{
    int aliveFirend = 0;
    int aliveEnemy = 0;
    int mySuperBulletNum = 0;

    for (PlayerVecIt f = leg._map_info._friend_players.begin(); f != leg._map_info._friend_players.end(); f++)
    {
        if (!f->_alive) { continue;}
        aliveFirend++;
        if (f->_have_super_bullet) mySuperBulletNum++;
    }

    for (PlayerVecIt foe = leg._map_info._enemy_players.begin(); foe != leg._map_info._enemy_players.end(); foe++)
    {
        if (!foe->_alive) { continue;}
        aliveEnemy++;
    }
    
    //如果己方人数多于1个或者持有超级子弹，不自杀
    if (1 != aliveFirend || mySuperBulletNum)
    {
        return false;
    }
    
    //如果地图上有超级子弹，不自杀
    if (leg._map_info._stars.size() > 0)
    {
        return false;
    }

    return true;
}

void runSituation(Leg &leg)
{
    if (checkNotAttackSituation(leg))
    {
        leg.sit = NOT_ATTACK;
        return ;
    }

    if (checkSuicideSituation(leg))
    {
        leg.sit = SUICIDE;
        return ;
    }

    leg.sit = NORMAL_SITUATION;
    return ;
}

bool CalcSituation::run(Leg & leg)
{
    runSituation(leg);
    return true;
}