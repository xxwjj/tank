#pragma once

#include "define.h"
#include "constants.h"
#include <vector>
#include <list>
#include <set>
#include <map>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "matrix_map.h"
#include "action.h"
#include "tick_bullet.h"
#include "event.h"
#include "situation.h"

class Player
{
public:
    Vector2D _pos;
    Vector2D _born_pos;
    int _id;
    int _team;
    bool _alive;
    bool _have_super_bullet;

    bool _ownBulletCalcSuper;
    Vector2D _ownBulletCalcDir;

    int _mostDangerousDistance;
    Vector2D _dangerousDir;

    int _foeDistance;
    Vector2D _foeDir;
	
	int _foeBulletDistance;
	Vector2D _foeBulletDir;

    int _totalEnemyNum[MAX_VEC_DIR_NUM];
    int _totalFriendNum[MAX_VEC_DIR_NUM];
    std::vector<Player>::iterator _nearest_foe;
    int _nearest_foe_dist;
	
	set<Vector2D> _move_to_nearest_foe;
	bool _target_choosed;
	
    std::set<Vector2D> moveList;
    std::set<Vector2D> fireList;
    ActionVec actionList;
    EventList event_list;
	
    void MaintainList()
    {
        moveList.clear();
        fireList.clear();
        for (ActionVecIt iter = actionList.begin(); iter != actionList.end(); iter++)
        {
			if(!iter->disable)
			{
              moveList.insert(iter->move);
              fireList.insert(iter->fire);
			}
        }
    }
    void GenerateActionList()
    {
        actionList.Init(_id);
    }

    void ResetActionList()
    {
        actionList.Reset(_have_super_bullet);
        moveList.insert(VECTOR_UP);
		moveList.insert(VECTOR_DOWN);
        moveList.insert(VECTOR_LEFT);
        moveList.insert(VECTOR_RIGHT);
        moveList.insert(VECTOR_ZERO);
        fireList.insert(VECTOR_UP);
        fireList.insert(VECTOR_DOWN);
        fireList.insert(VECTOR_LEFT);
        fireList.insert(VECTOR_RIGHT);
    }

    Player() :_id(0),_team(0),_pos(0,0), _alive(false), _foeDistance(MaxMapDistance), _dangerousDir(0,0)
    {
        actionList.reserve(MAX_DIR_NUM * MAX_DIR_NUM * 2);
        _born_pos = VECTOR_LEFT;

    }

    void SendEventTo(Player & target_player, EVENT_TYPE type, int value);
    int GetEvent(EVENT_TYPE type, vector<Event>& result);
    void ClearEvent();
    void ClearEvent(EVENT_TYPE type);

};


typedef  std::vector<Player> PlayerVec;
typedef  std::vector<Player>::iterator PlayerVecIt;

class Diamond
{
public:
    Diamond(){}
    Diamond(int col, int row):_pos(col, row) {}
    Vector2D _pos;
    int _count;
};
typedef  std::vector<Diamond> DiamondVec;
typedef  std::vector<Diamond>::iterator DiamondVecIt;

class Star
{
public:
    Star(){}
    Star(int col, int row):_pos(col, row){}
    Vector2D _pos;
};

typedef  std::vector<Star> StarVec;
typedef  std::vector<Star>::iterator StarVecIt;

class Bullet
{
public:
    Vector2D _pos;
    Vector2D _direction;

    int _team;
    bool _super_bullet;

    bool _deleted;

    Bullet():_pos(0,0), _direction(0,0), _team(0),_deleted(false), _super_bullet(false){}
    Bullet(Vector2D pos, Vector2D dir, int team, bool super)
    {
        _pos = pos;
        _direction = dir;
        _team = team;
        _deleted = false;
        _super_bullet = super;
    }
};

typedef  std::vector<Bullet> BulletVec;
typedef  std::vector<Bullet>::iterator BulletVecIt;

class Wall
{
public:
    Wall(){}
    Wall(int col, int row):_pos(col, row) {}
    Vector2D _pos;
};

typedef  std::vector<Wall> WallVec;
typedef  std::vector<Wall>::iterator WallVecIt;

class River
{
public:
    River(){}
    River(int col, int row):_pos(col, row) {}
    Vector2D _pos;
};

typedef  std::vector<River> RiverVec;
typedef  std::vector<River>::iterator RiverVecIt;

class Brick
{
public:
    Brick(){}
    Brick(int col, int row):_pos(col, row) {}
    Vector2D _pos;
};


typedef  std::vector<Brick> BrickVec;
typedef  std::vector<Brick>::iterator BrickVecIt;

class GameMap
{
public:
    std::vector<Player> _friend_players;
    std::vector<Player> _enemy_players;
    std::vector<Wall> _walls;
    std::vector<Brick > _bricks;
    std::vector<River> _rivers;
    std::vector<Diamond> _diamonds;
    std::vector<Star> _stars;
    std::vector<Bullet> _ownBullets;
    std::vector<Bullet> _foeBullets;
    GameMap()
    {
        _friend_players.reserve(FRIEND_NUM);
        _enemy_players.reserve(ENEMY_NUM);
    }

    int _height;
    int _width;
    int _max_distance;
    Player *getPlayer(int id)
    {
        std::vector<Player>::iterator iter;
        iter = _friend_players.begin();
        for (; iter != _friend_players.end(); iter++)
        {
            if ((*iter)._id == id)
                return &(*iter);
        }

        iter = _enemy_players.begin();
        for(;iter != _enemy_players.end(); iter++)
        {
            if((*iter)._id == id)
                return &(*iter);
        }
        return NULL;

    }

    PlayerVecIt getEnemy(int id)
    {
        PlayerVecIt  iter = _enemy_players.begin();
        for (;iter != _enemy_players.end(); iter++)
        {
            if (iter->_id == id)
            {
                return iter;
            }
        }
		return _enemy_players.end();
    }

    bool findOppositeBulletAtPos(Vector2D &pos, Vector2D &dir,E_BULLET type);
    bool findOppositeBulletAtPos(Vector2D &pos, Vector2D &dir, E_BULLET type, bool isSuperBullet);
    bool findStarAtPos(Vector2D &pos);

    bool HaveSuperBullet();
    int FriendPlayerCount();

    void clear()
    {
        _friend_players.clear();
        _enemy_players.clear();
        _bricks.clear();
        _walls.clear();
        _diamonds.clear();
        _stars.clear();
        _ownBullets.clear();
        _foeBullets.clear();
        _height = 0;
        _width = 0;
    };
};

class Team
{
public:
    int _id;
    int _point;
    std::vector<int> _players;
    int _remain_life;
};


class Leg
{
public:
    Leg()
    {
        InitializeCriticalSection(&action_section);
    }
    ~Leg()
    {
        DeleteCriticalSection(&action_section);
    }
    GameMap _map_info;
    MatrixMap _matrix_map;
    TickBullet _tick_bullet;
    std::map<int, Team> _teams;
    int _round_id;
    int _team_id;
    SITUATION sit;
    void clear() {
        _map_info.clear();
        _matrix_map.m_free();
        _teams.clear();
        _round_id = 0;

    };

    CRITICAL_SECTION action_section;
    void GetActions(std::vector<Action> & actions)
    {
        actions.clear();
        std::vector<Player>::iterator iter;
        iter = _map_info._friend_players.begin();
        for(;iter != _map_info._friend_players.end(); iter ++)
        {
            if (!iter->_alive)
                continue;
            bool action_getted = false;
            for (ActionVecIt act_iter = (*iter).actionList.begin(); act_iter != (*iter).actionList.end();act_iter++)
            {
                if (!act_iter->disable)
                {
                    actions.push_back(*act_iter);
                    action_getted = true;
                    break;
                }
            }

            if (!action_getted)
            {
                Action default_action;
                default_action.disable = false;
                default_action.player_id = iter->_id;
                default_action.fire = VECTOR_ZERO;
                default_action.move = VECTOR_ZERO;
                default_action._is_super_bullet = false;
                actions.push_back(default_action);
            }

        }
    }
    void EnterActionSection()
    {
#ifdef MULTI_THREAD_ROUND
        EnterCriticalSection(&action_section);
#endif
    }
    void LeaveActionSection()
    {
#ifdef MULTI_THREAD_ROUND
        LeaveCriticalSection(&action_section);
#endif
    }
    bool _action_getted;
    void SetActionGetted(bool action_getted)
    {
        _action_getted = action_getted;
    }

    bool ActionGetted()
    {
        return _action_getted;
    }
    int getNearestBulletDistance(Vector2D &pos, Vector2D &dir, E_BULLET type);
    bool haveSuperBulletInDir(Vector2D &pos, Vector2D &dir, E_BULLET type, int dis);
    int haveBulletNumInDir(Vector2D &pos, Vector2D &dir, E_BULLET type, int dis);
    int getSuperPlayerNum(Vector2D &pos);
};

