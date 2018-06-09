#include "ai.h"
#include "debug.h"
#include <cassert>
#include "denseGraph.h"
#include "graphSearchAStar.h"
#include "event.h"

class SiegeGraph : public DenseGraph<Cell>
{
	public:
	virtual bool isNodeReachable(int nid) const
	{
		const Cell &cell = GetNode(nid);
		if(E_WALL == cell._type || E_RIVER == cell._type)
		{
			return false;
		}
		return true;
	}
};

typedef Graph_SearchAStar<SiegeGraph, Heuristic_Manhattan> SiegeGraphAStar;

void PriAction(Player*player, Vector2D dir, PRIOPITY_BETTER_MOVE pri)
{
	for(ActionVecIt iter = player=>actionList.begin(); iter != player->actionList.end(); iter++)
	{
		if(iter->disable)
			continue;
		if(iter->move == dir && pri> iter->priSiege)
		{
			iter->priSiege = pri;
		}
	}
}

void PriFireAction(Player*player,Vector2D dir,PRIOPITY_BETTER_MOVE pri)
{
	for(ActionVecIt iter = player=>actionList.begin(); iter != player->actionList.end(); iter++)
	{
		if(iter->disable)
			continue;
		if(iter->fire == dir && pri> iter->priSiege)
		{
			iter->priSiege = pri;
		}
	}
}

bool ChooseBetterMove(map<Vector2D,int>&cost_map,set<Vector2D>&better_dir)
{
	better_dir.clear();
	if(const_map.size() <= 1)
	   return false;
    int min_dist = cost_map.begin()->second;
	for(map<Vector2D,int>::iterator iter = cost_map.begin(); iter!= cost_map.end();iter++)
	{
		if(iter->second == min_dist)
		   better_dir.insert(iter->first);
	    else if( iter->second < min_dist)
		{
			min_dist = iter->second;
			better_dir.clear();
			better_dir.insert(iter->first);
		}
	}
	if(better_dir.size() == cost_map.size())
		return false;
	
	return true;
}

bool GetMinMoveDir(SiegeGraphAStar &graph,MatrixMap &mtxmap,Vector2D source,set<Vector2D> &move_dirs,set<Vector2D>&best_moves)
{
	const int MAX_DIST = mtxmap._width*mtxmap._height;
	map<Vector2D,int> cost_map;
	for(set<Vector2D>::iterator move_iter = move_dirs.begin(); move_iter != move_dirs.end(); move_iter++)
	{
		if( *move_iter == VECTOR_ZERO)
			continue;
		Vector2D pos = source + *move_iter;
		int dist = graph.GetCostToPos(mtxmap.coord2index(pos.col,pos.row));
		cost_map[*move_iter] = dist==-1? MAX_DIST : dist;
	}
	return ChooseBetterMove(cost_map,best_moves);
}

Vector2D CalcCenter(Leg &leg)
{
	bool have_super_bullet = false;
	int player_count = 0;
	int super_player_count = 0;
	Vector2D sum,super_sum;
	PlayerVec &friends = leg._map_info._friend_players;
	for(PlayerVecIt iter = friends.begin(); iter != friends.end(); iter++)
	{
		if(!iter->_alive)
			continue;
		sum += iter->_pos;
		player_count++;
		if(iter->_have_super_bullet)
		{
			super_sum += iter->_pos;
			super_player_count++;
			have_super_bullet = true;
		}
	}
	assert(player_count != 0);
	if(player_count == 0)
	{
		return Vector2D(leg._map_info._width/2,leg._map_info._height/2);
	}
	return have_super_bullet? (super_sum/super_player_count) : (sum/player_count);
}

PlayerVecIt ChooseCenterPlayer(Leg&leg,Vector2D center)
{
	PlayerVec &friends = leg._map_info._friend_players;
	PlayerVecIt target = friends.begin();
	int min_dist_sqr = Vector2D(leg._map_info._width,leg._map_info._height).LengthSq();
	for(PlayerVecIt iter = friends.begin(); iter != friends.end(); iter++)
	{
		if (!iter->_alive)
			continue;
		Vector2D dir = center - iter->_pos;
		int dir_dist_sqr = dir.LengthSq();
		if(dir_dist_sqr < min_dist_sqr)
		{
			min_dist_sqr = dir.LengthSq();
			target = iter;
		}
	}
	return target;
}

void CalcFoeDist(Leg &leg,SiegeGraph &graph,map<PlayerVecIt,map<PlayerVecIt,map<Vector2D,int>>> &dist_map)
{
	const int MAX_DIST = leg._matrix_map._height * leg._matrix_map._width;
	for(PlayerVecIt foe_iter = leg._map_info._enemy_players.begin(); foe_iter!=leg._map_info._enemy_players.end(); foe_iter++)
	{
		if(!foe_iter->_alive)
			continue;
		
		SiegeGraphAStar foe_astar(graph,leg._matrix_map.coord2index(foe_iter->_pos.col,foe_iter->_pos.row),-1);
		//计算最近的敌人
		for(PlayerVecIt friend_iter = leg._map_info._friend_players.begin(); friend_iter!= leg._map_info._friend_players.end();friend_iter++)
		{
			if(!friend_iter -> _alive)
				continue;
			if(friend_iter->moveList.size() <= 1)
				continue;
			
			int foe_dist = foe_astar.GetCostToPos(leg._matrix_map.coord2index(friend_iter->_pos.col,friend_iter->_pos.row));
			dist_map[friend_iter][foe_iter][VECTOR_ZERO] = foe_dist==-1?MAX_DIST:foe_dist;
			
			for(set<Vector2D>::iterator dir_iter = friend_iter->moveList.begin(); dir_iter!=friend_iter->moveList.end();dir_iter++)
			{
				Vector2D dir = friend_iter->_pos + *dir_iter;
				int foe_dist = foe_astar.GetCostToPos(leg._matrix_map.coord2index(dir.col,dir.row));
				dist_map[friend_iter][foe_iter][*dir_iter] = foe_dist==-1?MAX_DIST:foe_dist;
			}
		}
	}
}

void GetNearestFoe(Leg &leg,map<PlayerVecIt,map<PlayerVecIt,map<Vector2D,int>>>&dist_map,list<PlayerVecIt> &process_list)
{
	//计算最近foe
	for(PlayerVecIt player = leg._map_info._friend_players.begin(); player!=leg._map_info._friend_players.end(); player++)
	{
		if(!player -> _alive)
			continue;
		
		map<PlayerVecIt,map<Vector2D,int>> &foe_dist_map = dist_map[player];
		player->_nearest_foe = leg._map_info._enemy_players.end();
		player->_nearest_foe_dist = leg._map_info._max_distance;
		int min_dist = leg._map_info._max_distance;
		for(map<PlayerVecIt,map<Vector2D,int>>::iterator foe_dist_iter 

