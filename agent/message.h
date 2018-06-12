#pragma once

#include "json/json.h"
#include <string>
#include "game.h"

using namespace std;

const string MSG_NAME = "msg_name";
const string MSG_REGISTRATION = "registration";
const string MSG_LEG_START = "leg_start";
const string MSG_ROUND = "round";
const string MSG_ACTION = "action";
const string MSG_LEG_END = "leg_end";
const string MSG_GAME_OVER = "game_over";
const string MSG_DATA = "msg_data";

class IMessage
{
	public:
	string toString()
	{
		Json::FastWriter writer;
		return writer.write(value);
	};
	
	static void ProcessLegStart(Leg &l,Json::Value value);
	static void ProcessRound(Leg &l,Json::Value value);
protected:
    Json::Value value;
	string string_value;
	static string Vector2String(Vector2D vec);
	static Vector2D String2Vector(string dir);
};

class RegMessage: public IMessage
{
	public:
	RegMessage(int team_id,string team_name);
	private:
	int _team_id;
	string _team_name;
};

class ActionMessage:public IMessage
{
	public:
	ActionMessage(Leg &l,vector<Action> & action_list);
};

