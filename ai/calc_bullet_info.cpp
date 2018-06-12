#include "ai.h"
#include "debug.h"

static Vector2D dirArray[MAX_VEC_DIR_NUM] = {VECTOR_UP,VECTOR_DOWN,VECTOR_LEFT,VECTOR_RIGHT,VECTOR_ZERO};

void CalcBullet::calcPlayerDangerousInfo(Leg &leg)
{
	std::vector<Player>::iterator iter;
	for(iter = leg._map_info._friend_players.begin();iter!=leg._map_info._friend_players.end();iter++)
	{
		if(iter->_alive != true)
		{
			continue;
		}
		
		int dist = 0;
		(*iter)._dangerousDir = VECTOR_ZERO;
		(*iter)._mostDangerousDistance = MaxMapDistance;
		(*iter)._foeDir = VECTOR_ZERO;
		(*iter)._foeDistance = MaxMapDistance;
		(*iter)._foeBulletDir = VECTOR_ZERO;
		(*iter)._foeBulletDistance = MaxMapDistance;
		for(int i = 0; i < MAX_VEC_DIR_NUM - 1;i++)
		{
			//先判断坦克，这相等条件下，危险方向指向的是坦克
			dist = leg._matrix_map.getNearestTerrainDistance((*iter)._pos,dirArray[i],E_FOE);
			if(dist != 0 &&dist<(*iter)._foeDistance)
			{
				(*iter)._foeDistance = dist;
				(*iter)._foeDir = dirArray[i];
			}
			
			if(dist != 0 && dist< (*iter)._mostDangerousDistance)
			{
				(*iter)._mostDangerousDistance = dist;
				(*iter)._dangerousDir = dirArray[i];
			}
			dist = leg.getNearestBulletDistance((*iter)._pos,dirArray[i],FOE_BULLET);
			if(dist != 0 && dist <(*iter)._foeBulletDistance)
			{
				(*iter)._foeBulletDistance = dist;
				(*iter)._foeBulletDir = dirArray[i];
			}
			if(dist != 0 && dist < (*iter)._mostDangerousDistance)
			{
				(*iter)._mostDangerousDistance = dist;
				(*iter)._dangerousDir = dirArray[i];
			}
		}
		
		if((*iter)._foeDistance == MaxMapDistance)
		{
			(*iter)._foeDistance = 0;//输出0，表示没有找到
		}
		if((*iter)._foeBulletDistance == MaxMapDistance)
		{
			(*iter)._foeBulletDistance = 0;//输出0，表示没有找到
		}
		if((*iter)._mostDangerousDistance == MaxMapDistance)
		{
			(*iter)._mostDangerousDistance = 0;//输出0，表示没有找到
		}
	}
}
		
bool CalcBullet::bulletDestroy(Bullet &friendBullet,Bullet &enemyBullet)
{
	if(friendBullet._super_bullet == enemyBullet._super_bullet)
	{
		friendBullet._deleted = true;
		enemyBullet._deleted = true;
		return false;
	}
	if(friendBullet._super_bullet)
	{
		enemyBullet._deleted = true;
		return true;
	}
	if(enemyBullet._super_bullet)
	{
		friendBullet._deleted = true;
		return false;
	}
	return false;
}

bool CalcBullet::run(Leg &leg)
{
	MatrixMap tmpMap;
	leg._matrix_map.clone(tmpMap);
	std::vector<Bullet> _tmpOwnBullets =leg._map_info._ownBullets;
	std::vector<Bullet> _tmpFoeBullets =leg._map_info._foeBullets;
	
	tmpMap.dump();
	calcPlayerDangerousInfo(leg);
	
	//根据敌人信息增加bullet
	std::vector<Player>::iterator iterEnemy;
	iterEnemy = leg._map_info._enemy_players.begin();
	for(;iterEnemy!=leg._map_info._enemy_players.end();iterEnemy++)
	{
		if((*iterEnemy)._alive != true)
		{
			continue;
		}
		_tmpFoeBullets.push_back(Bullet((*iterEnemy)._pos,VECTOR_UP,(*iterEnemy)._team,(*iterEnemy)._have_super_bullet));
		_tmpFoeBullets.push_back(Bullet((*iterEnemy)._pos,VECTOR_DOWN,(*iterEnemy)._team,(*iterEnemy)._have_super_bullet));
		_tmpFoeBullets.push_back(Bullet((*iterEnemy)._pos,VECTOR_LEFT,(*iterEnemy)._team,(*iterEnemy)._have_super_bullet));
		_tmpFoeBullets.push_back(Bullet((*iterEnemy)._pos,VECTOR_RIGHT,(*iterEnemy)._team,(*iterEnemy)._have_super_bullet));
	}

	//根据友军信息增加bullet
	std::vector<Player>::iterator iterFriend;
	iterFriend = leg._map_info._friend_players.begin();
	for(;iterFriend != leg._map_info._friend_players.end();iterFriend++)
	{
		if((*iterFriend)._alive != true)
		{
			continue;
		}
		if((*iterFriend)._foeDistance == 0)
		{
			iterFriend->_ownBulletCalcDir = VECTOR_ZERO;
			iterFriend->_ownBulletCalcSuper = false;
			continue;
		}
		
		if(leg.haveSuperBulletInDir(iterFriend->_pos,VECTOR_UP,FOE_BULLET,2)
			|| leg.haveBulletNumInDir(iterFriend->_pos,VECTOR_UP,FOE_BULLET,2) > 1)
		{
			_tmpOwnBullets.push_back(Bullet((*iterFriend)._pos,VECTOR_UP,(*iterFriend)._team,iterFriend->_have_super_bullet));	
			iterFriend->_ownBulletCalcSuper = iterFriend->_have_super_bullet;
            iterFriend->_ownBulletCalcDir = VECTOR_UP;
		}
		else if(leg.haveSuperBulletInDir(iterFriend->_pos,VECTOR_DOWN,FOE_BULLET,2)
			|| leg.haveBulletNumInDir(iterFriend->_pos,VECTOR_DOWN,FOE_BULLET,2) > 1)
		{
			_tmpOwnBullets.push_back(Bullet((*iterFriend)._pos,VECTOR_DOWN,(*iterFriend)._team,iterFriend->_have_super_bullet));	
			iterFriend->_ownBulletCalcSuper = iterFriend->_have_super_bullet;
            iterFriend->_ownBulletCalcDir = VECTOR_DOWN;
		}else if(leg.haveSuperBulletInDir(iterFriend->_pos,VECTOR_LEFT,FOE_BULLET,2)
			|| (leg.haveBulletNumInDir(iterFriend->_pos,VECTOR_LEFT,FOE_BULLET,2) > 1))
		{
			_tmpOwnBullets.push_back(Bullet((*iterFriend)._pos,VECTOR_LEFT,(*iterFriend)._team,iterFriend->_have_super_bullet));	
			iterFriend->_ownBulletCalcSuper = iterFriend->_have_super_bullet;
            iterFriend->_ownBulletCalcDir = VECTOR_LEFT;
		}else if(leg.haveSuperBulletInDir(iterFriend->_pos,VECTOR_RIGHT,FOE_BULLET,2)
			|| (leg.haveBulletNumInDir(iterFriend->_pos,VECTOR_RIGHT,FOE_BULLET,2) > 1))
		{
			_tmpOwnBullets.push_back(Bullet((*iterFriend)._pos,VECTOR_RIGHT,(*iterFriend)._team,iterFriend->_have_super_bullet));	
			iterFriend->_ownBulletCalcSuper = iterFriend->_have_super_bullet;
            iterFriend->_ownBulletCalcDir = VECTOR_RIGHT;
		}
		 else
        {
            _tmpOwnBullets.push_back(Bullet((*iterFriend)._pos, (*iterFriend)._dangerousDir, (*iterFriend)._team,false));
            iterFriend->_ownBulletCalcSuper = false;
            iterFriend->_ownBulletCalcDir = iterFriend->_dangerousDir;
        }
	}
	
	int tick = 0;
	for(tick = 0;tick < MAX_TICK_NUM;tick++)
	{
		std::vector<Bullet>::iterator iterFriend;
		std::vector<Bullet>::iterator iterEnemy;
		
		for(iterFriend = _tmpOwnBullets.begin();iterFriend != _tmpOwnBullets.end();iterFriend++)
		{
			if((*iterFriend)._deleted == true)
			{
				continue;
			}
			leg._tick_bullet.incOwnBulletNum((*iterFriend)._pos,tick);
			leg._tick_bullet.recordFoeBulletDir((*iterFriend)._pos,(*iterFriend)._direction,tick);
		}
		for(iterEnemy = _tmpFoeBullets.begin();iterEnemy!= _tmpFoeBullets.end();iterEnemy++)
		{
			if((*iterEnemy)._deleted == true)
			{
				continue;
			}
			leg._tick_bullet.incFoeBulletNum((*iterEnemy)._pos,tick);
		}
		tmpMap.clearDeletedTerrain();
		
		for(iterFriend = _tmpOwnBullets.begin();iterFriend != _tmpOwnBullets.end(); iterFriend++)
		{
			if((*iterFriend)._deleted == true)
			{
				continue;
			}
			Vector2D friendPos = (*iterFriend)._pos;
			//遍历敌方子弹列表
			for(iterEnemy = _tmpFoeBullets.begin();iterEnemy!= _tmpFoeBullets.end();iterEnemy++)
			{
				if((*iterEnemy)._deleted == true)
				{
					continue;
				}
				//相同位置			
				if(((*iterFriend)._pos == (*iterEnemy)._pos)
					&& (((*iterFriend)._direction+(*iterEnemy)._direction).isZero()
				||iterFriend->_direction.isPerp(iterEnemy->_direction)))
				{
					if(bulletDestroy((*iterFriend),(*iterEnemy)))
						continue;
					else
						goto NEXT_BULLET;
				}
			}
			if(tmpMap.getTerrain(friendPos) == E_WALL)
			{
				(*iterFriend)._deleted = true;
				goto NEXT_BULLET;
			}
			if(tmpMap.getTerrain(friendPos) == E_BRICK)
			{
				(*iterFriend)._deleted = true;
				Cell *pCell = tmpMap.getCell(&friendPos);//我方子弹遇到砖认为始终撞砖消失
				pCell->_deleted = true;
				goto NEXT_BULLET;
			}
			if(tmpMap.getTerrain(friendPos) ==E_FOE)
			{
				//我方子弹杀死敌方坦克
				(*iterFriend)._deleted = true;
				Cell *pCell = tmpMap.getCell(&friendPos);
				pCell->_deleted = true;
				goto NEXT_BULLET;
			}
	NEXT_BULLET:
	;
		}
		
		for(iterEnemy = _tmpFoeBullets.begin();iterEnemy != _tmpFoeBullets.end();iterEnemy++)
		{
			if((*iterEnemy)._deleted == true)
			{
				continue;
			}
			Vector2D foePos = (*iterEnemy)._pos;
			if(!tmpMap.isValidCoord(foePos))
			{
				(*iterEnemy)._deleted = true;
				continue;
			}
			if(tmpMap.getTerrain(foePos) == E_WALL)
			{
				(*iterEnemy)._deleted = true;
				continue;
			}
			Cell *pCell = tmpMap.getCell(&foePos);
			if(tmpMap.getTerrain(foePos) == E_BRICK && !pCell->_deleted)
			{
				//
				(*iterEnemy)._deleted = true;
				pCell->_deleted = true;
				continue;
			}
			if(tmpMap.getTerrain(foePos) == E_FRIEND)
			{
				(*iterEnemy)._deleted = true;
				pCell->_deleted = true;
				continue;
			}
		}
		
		for(iterFriend = _tmpOwnBullets.begin();iterFriend != _tmpOwnBullets.end();iterFriend++)
		{
			if((*iterFriend)._deleted == true)
			{
				continue;
			}
			Vector2D friendPos = (*iterFriend)._pos + iterFriend->_direction;
			if(!tmpMap.isValidCoord(friendPos))
			{
				(*iterFriend)._deleted = true;
				goto P_NEXT_BULLET;
			}
			
			for(iterEnemy = _tmpFoeBullets.begin();iterEnemy != _tmpFoeBullets.end();iterEnemy++)
		   {
			   if((*iterEnemy)._deleted == true)
			   {
				   continue;
			   }
			   //相隔位置字段对消
			   if((friendPos == (*iterEnemy)._pos)
				   && ((*iterFriend)._direction + (*iterEnemy)._direction).isZero())
				   {//这个地方不判断垂直对消
					   if(bulletDestroy((*iterFriend),(*iterEnemy)))
						   continue;
					   else
						   goto P_NEXT_BULLET;
				   }
		   }
		  P_NEXT_BULLET:
		  ;
		}
			
		//子弹位置前进一
		for(iterFriend = _tmpOwnBullets.begin();iterFriend != _tmpOwnBullets.end();iterFriend++)
		{
			if((*iterFriend)._deleted == true)
			{
				continue;
			}
			(*iterFriend)._pos+=(*iterFriend)._direction;
			if(!tmpMap.isValidCoord(iterFriend->_pos))
			{
				(*iterFriend)._deleted = true;
			}
		}
		for(iterEnemy = _tmpFoeBullets.begin();iterEnemy != _tmpFoeBullets.end();iterEnemy++)
		{
			if((*iterEnemy)._deleted == true)
			{
				continue;
			}
			(*iterEnemy)._pos += (*iterEnemy)._direction;
			if(!tmpMap.isValidCoord(iterEnemy->_pos))
			{
				(*iterEnemy)._deleted = true;
			}
		}
	}
	return true;
}

			