#include <vector>
#include <string>
#include "json/json.h"
#include "define.h"
#include "game.h"
#include "ai.h"
#include "thread.h"

using namespace std;

#define AGENT (Agent::getInstance())

class RoundThread : public Thread
{
  public :
  	 RoundThread(AI &ai,Leg&leg):_ai(ai),_leg(leg)
  	 {
  	 
  	 	}
    void Run()
    	{
    	 _ai.update(_leg);
    	}
 
 private:
  AI &_ai;
  Leg &_leg;
};

class Agent
{
	public:
	Agent():round_thread(NULL)
	{
		_action_list.reserve(FRIEND_NUM);
	}
	
	static Agent* getInstance()
	{
		static Agent instance;
		return &instance;
	}
	
	void Registation(int team_id);
	bool Once(unsigned int time_out);
	
	bool WaitLegStart();
	bool WaitRound();
	
	void Round(unsigned int time_out);
	void Round();
	int ProcessMsg(char*buf,int size);
	string GetMsg(Json::Value &value);
	void ProcessLegStart(Json::Value &value);
	void ProcessRound(Json::Value &value);
	void KillWorkingThread();
	void WaitLastWorkingThreadEnd();
	
	void SendActions();
	
	Leg leg;
	AI _ai;
	vector<Action> _action_list;
	RoundThread *round_thread;
};



