#pragma once 
#include <cstdlib>
#include <vector>
#include "define.h"
#include "vector2d.h"

#define DISABLE_ACTION(a) (a->disabled(__FUNCTION__, __LINE__))

enum PRIORITY_ESCAPE
{
	E_INFO,
		E_EVENT,
		E_WARNING,
		E_ERROR,
		E_EMERG	
};

enum PRIORITY_DIAMON
{
NOT_FORWARD_DIAMON,
	KEEP_FORWARD_DIAMOND,
	FORWARD_DIAMON
};

enum PRIORITY_DEAD_END
{
NO_ROAD,
	EXIST_ROAD
};

enum PRIORITY_BETTER_MOVE
{
E_NO_BETTER,
	E_BETTER_MOVE,
	E_GO_FOR_ENEMY
};

int const LINE_PRI = 10;
class Action {
	public:
		int player_id;
		bool disable;
		Vector2D fire;
		Vector2D move;
		bool _is_super_bullet;
		PRIORITY_ESCAPE priEscape;
		PRIORITY_DIAMON priDiamon;
		PRIORITY_DEAD_END priDeadEnd;
		PRIORITY_BETTER_MOVE priSiege;
		void disabled(const char *func, int line);
		Action():priEscape(E_INFO) , disable(false), player_id(0) {}
		Action(int player, Vector2D move_dir, Vector2D fire_dir, bool super_bullet = false):player_id(player), disable(false), priEscape(E_INFO), fire(fire_dir), move(move_dir),_is_super_bullet(super_bullet) {}

};


typedef std::vector<Action>::iterator ActionVecIt;


class ActionVec:public std::vector<Action>
{
	public:
		void DisableAction(ActionVecIt action)
		{
			if (!action->disable)
				{
				DISABLE_ACTION(action);
			}
			
		}

		int GetActionCount()
			{
			int count = 0;
			for (ActionVecIt iter = begin(); iter != end(); iter ++)
				{
				if (!iter->disable)
					count ++;
				
			}
			return count;
		}
		void Init(int player_id)
			{
			clear();
			push_back(Action(player_id, VECTOR_UP, VECTOR_UP));
			push_back(Action(player_id, VECTOR_UP, VECTOR_DOWN));
			push_back(Action(player_id, VECTOR_UP, VECTOR_LEFT));
			push_back(Action(player_id, VECTOR_UP, VECTOR_RIGHT));
			push_back(Action(player_id, VECTOR_UP, VECTOR_ZERO));
			push_back(Action(player_id, VECTOR_DOWN, VECTOR_UP));
			push_back(Action(player_id, VECTOR_DOWN, VECTOR_DOWN));
			push_back(Action(player_id, VECTOR_DOWN, VECTOR_LEFT));
			push_back(Action(player_id, VECTOR_DOWN, VECTOR_RIGHT));
			push_back(Action(player_id, VECTOR_DOWN, VECTOR_ZERO));
			push_back(Action(player_id, VECTOR_LEFT, VECTOR_UP));
			push_back(Action(player_id, VECTOR_LEFT, VECTOR_DOWN));
			push_back(Action(player_id, VECTOR_LEFT, VECTOR_LEFT));
			push_back(Action(player_id, VECTOR_LEFT, VECTOR_RIGHT));
			push_back(Action(player_id, VECTOR_LEFT, VECTOR_ZERO));
			push_back(Action(player_id, VECTOR_RIGHT, VECTOR_UP));
			push_back(Action(player_id, VECTOR_RIGHT, VECTOR_DOWN));
			push_back(Action(player_id, VECTOR_RIGHT, VECTOR_LEFT));
			push_back(Action(player_id, VECTOR_RIGHT, VECTOR_RIGHT));
			push_back(Action(player_id, VECTOR_RIGHT, VECTOR_ZERO));
			push_back(Action(player_id, VECTOR_ZERO, VECTOR_UP));
			push_back(Action(player_id, VECTOR_ZERO, VECTOR_DOWN));
			push_back(Action(player_id, VECTOR_ZERO, VECTOR_LEFT));
			push_back(Action(player_id, VECTOR_ZERO, VECTOR_RIGHT));
			push_back(Action(player_id, VECTOR_ZERO, VECTOR_ZERO));
			
			
			push_back(Action(player_id, VECTOR_UP, VECTOR_UP,true));
			push_back(Action(player_id, VECTOR_UP, VECTOR_DOWN,true));
			push_back(Action(player_id, VECTOR_UP, VECTOR_LEFT,true));
			push_back(Action(player_id, VECTOR_UP, VECTOR_RIGHT,true));
			//push_back(Action(player_id, VECTOR_UP, VECTOR_ZERO,true));
			push_back(Action(player_id, VECTOR_DOWN, VECTOR_UP,true));
			push_back(Action(player_id, VECTOR_DOWN, VECTOR_DOWN,true));
			push_back(Action(player_id, VECTOR_DOWN, VECTOR_LEFT,true));
			push_back(Action(player_id, VECTOR_DOWN, VECTOR_RIGHT,true));
			//push_back(Action(player_id, VECTOR_DOWN, VECTOR_ZERO,true));
			push_back(Action(player_id, VECTOR_LEFT, VECTOR_UP,true));
			push_back(Action(player_id, VECTOR_LEFT, VECTOR_DOWN,true));
			push_back(Action(player_id, VECTOR_LEFT, VECTOR_LEFT,true));
			push_back(Action(player_id, VECTOR_LEFT, VECTOR_RIGHT,true));
			//push_back(Action(player_id, VECTOR_LEFT, VECTOR_ZERO,true));
			push_back(Action(player_id, VECTOR_RIGHT, VECTOR_UP,true));
			push_back(Action(player_id, VECTOR_RIGHT, VECTOR_DOWN,true));
			push_back(Action(player_id, VECTOR_RIGHT, VECTOR_LEFT,true));
			push_back(Action(player_id, VECTOR_RIGHT, VECTOR_RIGHT,true));
			//push_back(Action(player_id, VECTOR_RIGHT, VECTOR_ZERO,true));
			push_back(Action(player_id, VECTOR_ZERO, VECTOR_UP,true));
			push_back(Action(player_id, VECTOR_ZERO, VECTOR_DOWN,true));
			push_back(Action(player_id, VECTOR_ZERO, VECTOR_LEFT,true));
			push_back(Action(player_id, VECTOR_ZERO, VECTOR_RIGHT,true));
			//push_back(Action(player_id, VECTOR_ZERO, VECTOR_ZERO,true));
			
		}
			
		void Reset(bool have_super_bullet = false)
		{
			for (ActionVecIt iter = begin(); iter != end(); iter ++)
			{				
				if (iter->_is_super_bullet)
				{
					iter->disable =!have_super_bullet;
				} else {
					iter->disable =false;
				}
				
			iter->priEscape = E_INFO;
			iter->priDiamon = NOT_FORWARD_DIAMON;
			iter->priDeadEnd = NO_ROAD;
			iter->priSiege = E_NO_BETTER;			
		    }
		}
};