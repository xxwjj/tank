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
		for(map<PlayerVecIt,map<Vector2D,int>>::iterator foe_dist_iter = foe_dist_map.begin();foe_dist_iter != foe_dist_map();foe_dist_iter++)
		{
			if(!player->_alive)
				continue;
			
			map<PlayerVecIt,map<Vector2D,int>> &foe_dist_map = dist_map[player];
			player->_nearest_foe = leg._map_info._enemy_players.end();
			player->_nearest_foe_dist = leg._map_info._max_distance;
			int min_dist = leg._map_info._max_distance;
			for(map<PlayerVecIt,map<Vector2D,int>>::iterator foe_dist_iter = foe_dist_map.begin();foe_dist_iter != foe_dist_map.end();foe_dist_iter++)
			{
				if(foe_dist_iter->second[Vector2D] < min_dist)
				{
					min_dist = foe_dist_iter->second[VECTOR_ZERO];
					player->_nearest_foe = foe_dist_iter->first;
					player->_nearest_foe_dist = min_dist;
				}
			}
			if(player->_nearest_foe != leg._map_info._enemy_players.end())
			{
				//calc min move
				map<Vector2D,int> &cost_map = foe_dist_map[(player->_nearest_foe)];
				ChooseBetterMove(cost_map,player->_move_to_nearest_foe);
			}
			list<PlayerVecIt>::iterator list_iter = process_list.begin();
			for(;list_iter!=process_list.end();list_iter++)
			{
				if(player->_nearest_foe_dist < (*list_iter)->_nearest_foe_dist)
				{
					process_list.push_front(player);
					break;
				}
			}
			if(list_iter == process_list.end())
			{
				process_list.push_back(player);
			}
		}
	}
	
	void ProcessSuperBulletPlayerByFoe(Leg &leg,SiegeGraph &graph,map<PlayerVecIt,map<PlayerVecIt,map<Vector2D,int>>> &dist_map)
	{
		if(!leg._map_info.HaveSuperBullet())
			return;
		for(PlayerVecIt iter = leg._map_info._friend_players.begin();iter!=leg._map_info._friend_players.end();iter++)
		{
		if(!iter->_alive)
			continue;
		if(iter->_nearest_foe == leg._map_info._enemy_players.end())
			continue;
		for(set<Vector2D>::iterator move_iter = iter->_move_to_nearest_foe.begin();move_iter != iter->_move_to_nearest_foe.end();move_iter++)
		{
			//如果已在攻击范围内 不再追
		if(dist_map[iter][iter->_nearest_foe][*move_iter] <= 2 && (iter->_nearest_foe->_pos.row == iter->_pos.row || iter->_nearest_foe->_pos.col == iter->_pos.col))	
			continue;
			PriAction(&(*iter)),*move_iter,E_GO_FOR_ENEMY);
			iter->_target_choosed = true;
		}
		}
	}
	
	void ChooseTarget(PlayerVecIt player,PlayerVecIt enemy,map<Vector2D,int> enemy_dist)
	{
		set<Vector2D> better_move;
		if(!ChooseBetterMove(enemy_dist,better_move))
			return;
		for(set<Vector2D>::iterator dir_iter = better_move.begin();dir_iter!=better_move.end();dir_iter++)
		{
			PriAction(&(*player),*dir_iter,E_BETTER_MOVE);
			player->_target_choosed = true;
		}
	}
	
	void NoticeNearFriend(PlayerVecIt player,PlayerVecIt foe,map<PlayerVecIt,map<PlayerVecIt,map<Vector2D,int>>> dist_map,Leg &leg)
	{
		PlayerVecIt next_player = leg._map_info._friend_players.end();
		int min_dist = leg._map_info._max_distance;
		for(map<PlayerVecIt,map<PlayerVecIt,map<Vector2D,int>>>::iterator frend_map_iter = dist_map.begin();frend_map_iter != dist_map.end();frend_map_iter++)
		{
			if(frend_map_iter->first == player)
				continue;
			//已经有目标的朋友不通知
			if(frend_map_iter->first->_target_choosed)
				continue;
			map<Vector2D,int> &cost_map = frend_map_iter->second[foe];
			int dist = cost_map[VECTOR_ZERO];
			if(dist < min_dist)
			{
				min_dist = dist;
				next_player = frend_map_iter->first;
			}
		}
		
		//找到才通知
		if(next_player != leg._map_info._friend_players.end())
		{
			player->SendEventTo((*next_player),E_COOR_ATTACK,player->_nearest_foe->_id);
		}
	}
	
	bool CoorAttack(PlayerVecIt player,Leg&leg,map<PlayerVecIt,map<PlayerVecIt,map<Vector2D,int>>> dist_map)
	{
		vector<Event> attack_event;
		if(player->GetEvent(E_COOR_ATTACK,attack_event) != 0)
		{
			PlayerVecIt near_foe_player = leg._map_info.getEnemy(attack_event.begin()->_value);
			if(near_foe_player != leg._map_info._enemy_players.end())
			{
				leg.EnterActionSection();
				ChooseTarget(player,near_foe_player,dist_map[player][near_foe_player]);
				leg.LeaveActionSection();
			}
		}
		else
		{
			leg.EnterActionSection();
			ChooseTarget(player,player->_nearest_foe,dist_map[player][player->_nearest_foe]);
			leg.LeaveActionSection();
			NoticeNearFriend(player,player->_nearest_foe,dist_map,leg);
		}
		return true;
	}
	
	bool GotoCenter(PlayerVecIt player,Leg &leg,SiegeGraphAStar &astar)
	{
		const int SHORT_ENOUGH = 3;
		const int MAX_DIST = leg._matrix_map._height * leg._matrix_map._width;
		int current_dist = astar.GetCostToPos(leg._matrix_map.coord2index(player->_pos.col,player->_pos.row));
		if(current_dist < SHORT_ENOUGH)
			return true;
		
		map<Vector2D,int> cost_map;
		for(set<Vector2D>::iterator move_iter = player->moveList.begin(); move_iter!=player->moveList.end();move_iter++)
		{
			if(*move_iter ==VECTOR_ZERO)
				continue;
			Vector2D pos = player->_pos + *move_iter;
			int dist = astar.GetCostToPos(leg._matrix_map.coord2index(pos.col,pos.row));
			const_map[*move_iter] = dist == -1 ?MAX_DIST :dist;
		}
		set<Vector2D> better_move;
		if(!ChooseBetterMove(cost_map,better_move))
			return true;
		leg.EnterActionSection();
		for(set<Vector2D>::iterator dir_iter = better_move.begin();dir_iter != better_move.end(); dir_iter++)
		{
			PriAction(&(*player),*dir_iter,E_BETTER_MOVE);
		}
		leg.LeaveActionSection();
		return true;
	}

	void runSiege(Leg &leg)
	{
		//每个player计算最近敌人
		map<PlayerVecIt,map<PlayerVecIt,map<Vector2D,int>>> dist_map;//[friend][enemry][friend move direc][dist]
		list<PlayerVecIt>  process_list;
		Vector2D center = CalcCenter(leg);
		PlayerVecIt center_player = ChooseCenterPlayer(leg,center);
		Vector2D center_target = center_player->_pos;
		SiegeGraph graph;
		graph.LoadFromMatrix(leg._matrix_map._cells,leg._matrix_map._height,leg._matrix_map._width);
		CalcFoeDist(leg,graph,dist_map);
		GetNearestFoe(leg,dist_map,process_list);
		ProcessSuperBulletPlayerByFoe(leg,graph,dist_map);
		//协作攻击
		for(list<PlayerVecIt>::iterator player_iter = process_list.begin();player_iter!=process_list.end();player_iter++)
		{
			//有超级子弹  直接找敌人
			if((*player_iter)->_have_super_bullet)
				continue;
			CoorAttack(*player_iter,leg,dist_map);
		}
		//向中心集中
		SiegeGraphAStar astar(graph,leg._matrix_map.coord2index(center_target.col,center_target.row),-1);
		for(PlayerVecIt iter = leg._map_info._friend_players.begin();iter!=leg._map_info._friend_players.end();iter++)
		{
			if(!iter->_alive)
				continue;
			if(iter->moveList.size() <= 1)
				continue;
			if(iter->_target_choosed)
				continue;
			GotoCenter(iter,leg,astar);
		}
	}
	
	bool Siege::run(Leg&leg)
	{
		runSiege(leg);
		return true;
	}
	