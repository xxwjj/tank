#pragma once
#include <cstdlib>
#include <cassert>
#include <vector>
#include "define.h"
#include "vector2d.h"
#include "constants.h"

#define MAX_TICK_NUM (6)

//Tick信息依赖于MatrixMap,因为MatrixMap可能会被各个策略模块复制多次，而bullet信息在每个round是不变的。
//为了减少不必要的内存消耗，将这块内存单独丢出来
class TickBullet
{
	int _width;
	int _height;
	char _ownBullets[MaxMapWidth][MaxMapHeight][MAX_TICK_NUM];
	
    char _foeBullets[MaxMapWidth][MaxMapHeight][MAX_TICK_NUM];
    Vector2D _foeBulletDir[MaxMapWidth][MaxMapHeight][MAX_TICK_NUM];//记录每个时刻的敌方子弹方向
	
public:
 TickBullet(int col,int row){_width = col, _height = row;}
 void m_alloc(int col,int row){reset(), _width = col, _height = row;}
 void reset()
 {
	 memset(_ownBullets,0, sizeof(_ownBullets));
	 memset(_foeBullets,0, sizeof(_foeBullets));
	 memset(_foeBulletDir,0, sizeof(_foeBulletDir));
 }
 
 void incFoeBulletNum(Vector2D &pos, int tick)
 {
	 if(pos.col < 0 || pos.row < 0|| pos.col >= _width || pos.row >= _height)
		 return;
	 if(tick < 0 || tick >= MAX_TICK_NUM)
		 return;
	 _foeBullets[pos.col][pos.row][tick] += 1;
 }
 
 void recordFoeBulletDir(Vector2D &pos ,Vector2D &dir, int tick)//如果有多个只记录最后一个
 {
   if(pos.col < 0 || pos.row < 0 || pos.col >= _width || pos.row >= _height)
	 return;
   if(tick < 0|| tick >= MAX_TICK_NUM)
	return;
   _foeBulletDir[pos.col][pos.row][tick] = dir;
 }
 
 Vector2D getFoeBulletDir(Vector2D &pos , int tick)
 {
   if(pos.col < 0 || pos.row < 0 || pos.col >= _width || pos.row >= _height)
	 return Vector2D(0,0);
   if(tick < 0|| tick >= MAX_TICK_NUM)
	return Vector2D(0,0);
   return _foeBulletDir[pos.col][pos.row][tick];
 }

  int getFoeBulletNum(Vector2D &pos, int tick)
 {
	 if(pos.col < 0 || pos.row < 0|| pos.col >= _width || pos.row >= _height)
		 return 0;
	 if(tick < 0 || tick >= MAX_TICK_NUM)
		 return 0;
	 return _foeBullets[pos.col][pos.row][tick];
 }
 
  void incOwnBulletNum(Vector2D &pos, int tick)
 {
	 if(pos.col < 0 || pos.row < 0|| pos.col >= _width || pos.row >= _height)
		 return;
	 if(tick < 0 || tick >= MAX_TICK_NUM)
		 return;
	 _ownBullets[pos.col][pos.row][tick] += 1;
 }
 
   int getOwnBulletNum(Vector2D &pos, int tick)
 {
	 if(pos.col < 0 || pos.row < 0|| pos.col >= _width || pos.row >= _height)
		 return 0;
	 if(tick < 0 || tick >= MAX_TICK_NUM)
		 return 0;
	 return _ownBullets[pos.col][pos.row][tick];
 }
 
 ~TickBullet(){}
 TickBullet():_width(0),_height(0){}
};