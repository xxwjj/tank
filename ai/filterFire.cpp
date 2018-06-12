#include "ai.h"
#include "debug.h"
#include <cstdlib>
#include "graphSearchAStar.h"
#include "denseGraph.h"
#include <algorithm>
#include <map>

bool checkLineDire(Leg &leg,Vector2D &pos,Vector2D &fire,bool & needSuper)
{
	GameMap &mapInfo = leg._map_info;
	MatrixMap &matrix = leg._matrix_map;
	Vector2D newPos;
	
	for(int i = 1;;i++)
	{
		newPos = pos + fire*i;
		if(!matrix.isValidCoord(newPos))
		{
			break;
		}
		//遇墙停止，但是允许超级子弹发送
		if(E_WALL == matrix.getTerrain(newPos))
		{
			needSuper = true;
			break;
		}
		//线上有砖
		if(E_BRICK == matrix.getTerrain(newPos))
		{
			return true;
		}
		//线上有敌人
		if(E_FOE == matrix.getTerrain(newPos))
		{
			return true;
		}
		//线上有相同子弹
		for(BulletVecIt b = mapInfo._foeBullets.begin(); b != mapInfo._foeBullets.end(); b++)
		{
			if(b->_pos == newPos && b->_direction *(-1) == fire)
			{
				return true;
			}
		}
	}
	return false;
}

bool checkDiagDire(Leg &leg,Vector2D &pos,Vector2D &fire)
{
	GameMap &mapInfo = leg._map_info;
	MatrixMap &matrix = leg._matrix_map;
	Vector2D newPos;
	Vector2D verticalDire[2];
	if(fire.col == 0)
	{
		verticalDire[0].col = -1;
		verticalDire[1].col = 1;
		verticalDire[0].row = 0;
		verticalDire[1].row = 0;
	}
	else if(fire.row == 0)
	{
		verticalDire[0].row = -1;
		verticalDire[1].row = 1;
		verticalDire[0].col = 0;
		verticalDire[1].col = 0;
	}
	else
	{
		//wrong fire dir
		return true;
	}
	
	Vector2D bulletPos;
	for(int i = 1;;i++)
	{
		bulletPos = pos + fire*i;
		if(!matrix.isValidCoord(bulletPos))
		{
			break;
		}
		if(E_WALL == matrix.getTerrain(bulletPos))
		{
			break;
		}
		for(int d = 0; d < 2;d++)
		{
			int j;
			for(j = 1;j < i;j++)
			{
				newPos = bulletPos + verticalDire[d]*j;
				if(!matrix.isValidCoord(newPos))
				{
					break;
				}
				if(E_WALL == matrix.getTerrain(newPos))
				{
					break;
				}
			}
			if( j < i)
			{
				continue;
			}
			newPos = bulletPos + verticalDire[d] * i;
			for(BulletVecIt b = mapInfo._foeBullets.begin(); b!=mapInfo._foeBullets.end();b++)
			{
				if(b->_pos == newPos && b->_direction == -1 * verticalDire[d])
				{
					return true;
				}
			}
			
			for(PlayerVecIt foe = mapInfo._enemy_players.begin();foe != mapInfo._enemy_players.end(); foe++)
			{
				if(foe->_pos == newPos)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool checkAcentDire(Leg &leg,Vector2D &pos,Vector2D &fire)
{
	GameMap &mapInfo = leg._map_info;
	MatrixMap &matrix = leg._matrix_map;
	Vector2D newPos;
	Vector2D verticalDire[2];
	if(fire.col == 0)
	{
		verticalDire[0].col = -1;
		verticalDire[1].col = 1;
		verticalDire[0].row = 0;
		verticalDire[1].row = 0;
	}
	else if(fire.row == 0)
	{
		verticalDire[0].row = -1;
		verticalDire[1].row = 1;
		verticalDire[0].col = 0;
		verticalDire[1].col = 0;
	}
	
	Vector2D bulletPos;
	//测试火力范围内有无可攻击敌人
	for(int i = 1;;i++)
	{
		bulletPos = pos + fire*i;
		if(!matrix.isValidCoord(newPos))
		{
			break;
		}
		if(E_WALL == matrix.getTerrain(bulletPos))
		{
			break;
		}
		int step = int(i/3.0+ 0.5);
		for(int d = 0; d < 2;d++)
		{
			int j;
			for(j = 1;j <= step;j++)
			{
				newPos = bulletPos + verticalDire[d]*j;
				if(!matrix.isValidCoord(newPos))
				{
					break;
				}
				if(E_WALL == matrix.getTerrain(newPos)
					|| E_RIVER == matrix.getTerrain(newPos))
				{
					break;
				}
			
			for(PlayerVecIt foe = mapInfo._enemy_players.begin(); foe!=mapInfo._enemy_players.end();foe++)
			{
				if(foe->_pos == newPos)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void filterUselessFire(Leg &leg)
{
	GameMap &mapInfo = leg._map_info;
	MatrixMap &matrix = leg._matrix_map;
	int playerId;
	bool superFire = false;
	Vector2D dire[DIRECTION_NUM] = {VECTOR_DOWN,VECTOR_RIGHT,VECTOR_LEFT,VECTOR_UP};
	
	for(PlayerVecIt f = mapInfo._friend_players.begin();f != mapInfo._friend_players.end();f++)
	{
		if(!f->_alive)
		{
			continue;
		}
		
		playerId = f->_id;
		for(int i = 0; i < DIRECTION_NUM;i++)
		{
			superFire = false;
			if(checkLineDire(leg,f->_pos,dire[i],superFire))
			{
				continue;
			}
			if(checkDiagDire(leg,f->_pos,dire[i]))
			{
				continue;
			}
			if(checkAcentDire(leg,f->_pos,dire[i]))
			{
				continue;
			}
			leg.EnterActionSection();
			for(ActionVecIt a = f->actionList.begin(); a != f->actionList.end(); a++)
			{
				if(a->disable)
				{
					continue;
				}
				if(superFire && a->_is_super_bullet)
				{
					continue;
				}
				if(a->fire == dire[i])
				{
					DISABLE_ACTION(a);
				}
			}
			leg.LeaveActionSection();
		}
	}
}

void filterConflictMoveFire(Leg &leg)
{
	GameMap &mapInfo = leg._map_info;
	MatrixMap &matrix = leg._matrix_map;
	Vector2D newPos;
	for(PlayerVecIt f = mapInfo._friend_players.begin(); f!= mapInfo._friend_players.end(); f++)
	{
		if(!f->_alive)
		{
			continue;
		}
		f->MaintainList();
		
		for(std::set<Vector2D>::iterator m = f->moveList.begin(); m != f->moveList.end(); m++)
		{
			newPos = *m + f->_pos;
			if(!matrix.isValidCoord(newPos))
			{
				continue;
			}
			if(E_BRICK == matrix.getTerrain(newPos))
			{
				leg.EnterActionSection();
				for(ActionVecIt a = f->actionList.begin(); a!= f->actionList.end();a++)
				{
					if(a->disable || a->move != *m)
					{
						continue;
					}
					if(a->fire != *m)
					{
						DISABLE_ACTION(a);
					}
				}
				leg.LeaveActionSection();
			}
			if(E_WALL == matrix.getTerrain(newPos))
			{
				leg.EnterActionSection();
				for(ActionVecIt a = f->actionList.begin(); a!= f->actionList.end();a++)
				{
					if(a->disable || a->move != *m)
					{
						continue;
					}
					if(a->fire != *m || !a->_is_super_bullet)
					{
						DISABLE_ACTION(a);
					}
				}
				leg.LeaveActionSection();
			}
		}
	}
}

void runFilterFire(Leg &leg)
{
	filterUselessFire(leg);
	filterConflictMoveFire(leg);
}

bool FilterFire::run(Leg &leg)
{
	runFilterFire(leg);
	return true;
}


