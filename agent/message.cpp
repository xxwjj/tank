#include "json/json.h"
#include "message.h"
#include "debug.h"
#include "situation.h"
#include <cassert>

void IMessage::ProcessLegStart(Leg& leg, Json::Value value)
{
    log("Leg Start!\n");
#ifdef _DEBUG
    printf("Leg Start!\n");
#endif
    leg.clear();
    Json::Value map_node = value[MSG_DATA]["map"];

    leg._map_info._height = map_node["height"].asInt();
    leg._map_info._width = map_node["width"].asInt();
    leg._map_info._max_distance = leg._map_info._height * leg._map_info._width ;

    assert(leg._map_info._width <= MaxMapWidth); //这里需要注意适配坐标
    assert(leg._map_info._height <= MaxMapHeight);
    leg._matrix_map.m_alloc(leg._map_info._width, leg._map_info._height);
    leg._tick_bullet.m_alloc(leg._map_info._width, leg._map_info._height);

    leg._teams.clear();
    Json::Value teams_node = value[MSG_DATA]["teams"];
    for (unsigned int i = 0; i < teams_node.size(); ++i)
    {
        Team team;
        Json::Value team_node = teams_node[i];
        team._id = team_node["id"].asInt();

        Json::Value team_players_node = teams_node[i]["players"];
        for (unsigned int j = 0;j<team_players_node.size();++j)
        {
            int player_id = team_players_node[j].asInt();
            team._players.push_back(player_id);
            Player temp;
            temp._id = player_id;
            temp._team = team._id;
            temp._alive = true;
            if (team._id == leg._team_id)
            {
                leg._map_info._friend_players.push_back(temp);
                leg._map_info._friend_players.rbegin()->GenerateActionList();
            }
            else
                leg._map_info._enemy_players.push_back(temp);
        }
        leg._teams.insert(make_pair(team._id,team));
    }
}

void IMessage::ProcessRound(Leg& leg, Json::Value value)
{
    leg._round_id = value[MSG_DATA]["round_id"].asInt();
    leg.sit = NORMAL_SITUATION;
    log("Round:%d \n",leg._round_id);
#ifdef _DEBUG
    printf("Round:%d \n",leg._round_id);
#endif
    //log("Recv: %s",value.toStyledString().c_str());
    //printf("Recv: %s",value.toStyledString().c_str());
    leg._matrix_map.clearVolatileTerrain();
    leg._tick_bullet.reset();

    leg._map_info._ownBullets.clear();
    leg._map_info._foeBullets.clear();
    Json::Value bombs_node = value[MSG_DATA]["bullets"];
    for (unsigned int i = 0; i < bombs_node.size(); ++i)
    {
        Bullet bullet;
        Json::Value boom_node = bombs_node[i];
        bullet._pos.col = boom_node["x"].asInt();
        bullet._pos.row = boom_node["y"].asInt();
        bullet._team = boom_node["team"].asInt();
        bullet._super_bullet = (boom_node["type"].asInt() == 1);
        
        string dir = boom_node["direction"].asString();
        bullet._direction = String2Vector(dir);

        if (bullet._team == leg._team_id)
        {
            leg._map_info._ownBullets.push_back(bullet);
            leg._matrix_map.setBullet(bullet._pos.col,bullet._pos.row,OWE_BULLET);
        }
        else
        {
            leg._map_info._foeBullets.push_back(bullet);
            leg._matrix_map.setBullet(bullet._pos.col,bullet._pos.row,FOE_BULLET);
        }
    }
	
	leg._map_info._bricks.clear();
	Json::Value boxes_node = value[MSG_DATA]["brick_walls"];
	for(unsigned int i = 0; i < boxes_node.size(); ++i)
	{
	  Brick box;
	  Json::Value box_node = boxes_node[i];
	  box._pos.col = box_node["x"].asInt();
	  box._pos.row = box_node["y"].asInt();
	  leg._map_info._bricks.push_back(box);
	  //刷新matrix地图信息
	  leg._matrix_map.setTerrain(box._pos.col,box._pos.row,E_BRICK);
	  assert(leg._matrix_map.getTerrain(box._pos.col,box._pos.row) == E_BRICK);
	}
	
	leg._map_info._walls.clear();
	Json::Value walls_node = value[MSG_DATA]["iron_walls"];
	for(unsigned int i = 0; i < walls_node.size(); ++i)
	{
	  Wall wall;
	  Json::Value wall_node = walls_node[i];
	  wall._pos.col = wall_node["x"].asInt();
	  wall._pos.row = wall_node["y"].asInt();
	  leg._map_info._walls.push_back(wall);
	  //刷新matrix地图信息
	  leg._matrix_map.setTerrain(wall._pos.col,wall._pos.row,E_WALL);
	}
	
	leg._map_info._rivers.clear();
	Json::Value rivers_node = value[MSG_DATA]["river"];
	for(unsigned int i = 0; i < rivers_node.size(); ++i)
	{
	  River river;
	  Json::Value river_node = rivers_node[i];
	  river._pos.col = river_node["x"].asInt();
	  river._pos.row = river_node["y"].asInt();
	  leg._map_info._rivers.push_back(river);
	  //刷新matrix地图信息
	  leg._matrix_map.setTerrain(river._pos.col,river._pos.row,E_RIVER);
	}
	
	leg._map_info._diamonds.clear();
	Json::Value diamonds_node = value[MSG_DATA]["coins"];
	for(unsigned int i = 0; i < diamonds_node.size(); ++i)
	{
	  Diamond diamond;
	  Json::Value diamond_node = diamonds_node[i];
	  diamond._pos.col = diamond_node["x"].asInt();
	  diamond._pos.row = diamond_node["y"].asInt();
	  diamond._count = diamond_node["point"].asInt();
	  leg._map_info._diamonds.push_back(diamond);
	  //刷新matrix地图信息
	  //leg._matrix_map.setTerrain(river._pos.col,river._pos.row,E_RIVER);
	}
	
	leg._map_info._stars.clear();
	Json::Value stars_node = value[MSG_DATA]["stars"];
	for(unsigned int i = 0; i < stars_node.size(); ++i)
	{
	  Star star;
	  Json::Value star_node = stars_node[i];
	  star._pos.col = star_node["x"].asInt();
	  star._pos.row = star_node["y"].asInt();
	  leg._map_info._stars.push_back(star);
	  //刷新matrix地图信息
	  //leg._matrix_map.setTerrain(star._pos.col,star._pos.row,E_STAR);
	}
	
	//所有player设置为死亡
	for(PlayerVecIt iter = leg._map_info._friend_players.begin(); iter != leg._map_info._friend_players.end(); iter++)
	{
		iter->_alive = false;
		iter->_target_choosed = false;
	}
	
	for(PlayerVecIt iter = leg._map_info._enemy_players.begin(); iter != leg._map_info._enemy_players.end(); iter++)
	{
		iter->_alive = false;
		iter->_target_choosed = false;
	}
	
	Json::Value players_node = value[MSG_DATA]["players"];
	for(unsigned int i = 0; i < players_node.size(); ++i)
	{
		Json::Value player_node = players_node[i];
		
		int id = player_node["id"].asInt();
		int team_id =player_node["team"].asInt();
		int col = player_node["x"].asInt();
		int row = player_node["y"].asInt();
		bool have_super_bullet = player_node["super_bullet"].asBool();
		PlayerVec &player_vec = (team_id == leg._team_id)? leg._map_info._friend_players:leg._map_info._enemy_players;
		PlayerVecIt iter;
		for(iter = player_vec.begin(); iter != player_vec.end(); iter++)
		{
			if(iter->_id == id)
				break;
		}
		
		if(iter == player_vec.end())
		{
			//一个round新出现了player,理论上不会走这个分支
			Player tmp;
			tmp._id = id;
			tmp._team = team_id;
			tmp._alive = true;
			tmp._pos.col = col;
			tmp._pos.row = row;
			tmp._born_pos = tmp._pos;
			tmp._have_super_bullet = have_super_bullet;
			player_vec.push_back(tmp);
			if(team_id == leg._team_id)
			{
				player_vec.rbegin()->GenerateActionList();
				player_vec.rbegin()->ResetActionList();
				player_vec.rbegin()->ClearEvent();
			}
		}
		else
		{
			iter->_alive = true;
			iter->_pos.col = col;
			iter->_pos.row = row;
			if( iter->_born_pos == VECTOR_LEFT)//born_pos init for left
				iter->_born_pos = iter->_pos;
			iter->_have_super_bullet = have_super_bullet;
			iter->_move_to_nearest_foe.clear();
			iter->ResetActionList();
			iter->ClearEvent();
		}
		
		if(team_id == leg._team_id)
		   leg._matrix_map.setTerrain(col,row,E_FRIEND);
	    else
		   leg._matrix_map.setTerrain(col,row,E_FOE);
	}
	
	Json::Value teams_node = value[MSG_DATA]["teams"];
	for(unsigned int i = 0; i < teams_node.size(); ++i)
	{
	Json::Value team_node = teams_node[i];
	int id = team_node["id"].asInt();
	leg._teams[id]._point = team_node["point"].asInt();
	leg._teams[id]._remain_life = team_node["remain_life"].asInt();
#ifdef _DEBUG
#endif
	}
}

std::string IMessage::Vector2String(Vector2D vec)
{
	if(vec == VECTOR_UP)
	   return "up";
   else if(vec == VECTOR_DOWN)
	   return "down";
   else if(vec == VECTOR_LEFT)
	   return "left";
   else if(vec == VECTOR_RIGHT)
	   return "right";
   else
	   return "";
}

Vector2D IMessage::String2Vector(string dir)
{
	if(dir == "up")
	   return VECTOR_UP;
	else if(dir == "down")
	   return VECTOR_DOWN;
   else if(dir == "left")
	   return VECTOR_LEFT;
   else if(dir == "right")
	   return VECTOR_RIGHT;
   else
	   return VECTOR_ZERO;
}

ActionMessage::ActionMessage(Leg &leg, vector<Action> & action_list)
{
	value[MSG_NAME] = MSG_ACTION;
	value[MSG_DATA]["round_id"] = leg._round_id;
	
	const string move_directions[4] = {"up","down","left","right"};
	
	Json::Value action_array;
	action_array.resize(0);
	for(vector<Action>::iterator iter = action_list.begin(); iter!=action_list.end(); iter++)
	{
		Json::Value action_node;
		action_node["team"] = leg._map_info.getPlayer(iter->player_id)->_team;
		action_node["player_id"] = iter->player_id;
		action_node["move"].resize(0);
		action_node["fire"].resize(0);
		action_node["bullet_type"] = iter->_is_super_bullet ? 1:0;
		
		string move = Vector2String(iter->move);
		if(move != "")
		{
			action_node["move"][0] = move;
		}
		string fire = Vector2String(iter->fire);
		if(fire != "")
		{
			action_node["fire"][0] = fire;
		}
		
		action_array.append(action_node);
	}
	value[MSG_DATA]["actions"] = action_array;
	
	log("Action: %s\n",value.toStyledString().c_str());
}

RegMessage::RegMessage(int team_id,string team_name) : _team_id(team_id),_team_name(team_name)
{
	value["msg_name"] = "registration";
	value["msg_data"]["team_id"] = _team_id;
	value["msg_data"]["team_name"] = _team_name;
};
