#include <vector>
#include <strring>
#include "agent.h"
#include "socket.h"
#include "game.h"
#include "json/json.h"
#include "thread.h"
#include "debug.h"
#include <cassert>
#include "message.h"

using namespace std;

void Agent::Registation(int team_id)
{
    leg._team_id = team_id;
    RegMessage regs(team_id, "everest");
    string output = regs.toString();
    CONNECTOR->send(output);
}

string Agent::GetMsg(Json::Value& value)
{
    Json::Reader reader;
    string recv_str = CONNECTOR->recv();
    if(!reader.parse(recv_str, value))
    {
        return "";
    }
    if (!value.isMember(MSG_NAME))
        return "";

    return value[MSG_NAME].asString();
}

bool Agent::WaitLegStart()
{
    Json::Value value;
    string msg_type = GetMsg(value);

    if (msg_type == MSG_GAME_OVER)
        return false;

    if (msg_type != MSG_LEG_START)
        return false;

    WaitLastWorkingThreadEnd();

    IMessage::ProcessLegStart(leg, value);

    return true;
}

bool Agent::WaitRound()
{
    Json::Value value;
    string msg_type = GetMsg(value);

    if (msg_type == MSG_LEG_END)
        return false;

    if (msg_type != MSG_ROUND)
        return false;

    WaitLastWorkingThreadEnd();

    IMessage::ProcessRound(leg, value);

    return true;
}

bool Agent::Once(unsigned int time_out)
{
    Json::Value value;
    string msg_type = GetMsg(value);

    WaitLastWorkingThreadEnd();
    
    if (MSG_LEG_START== msg_type)
    {
        CONNECTOR->_longest_time = 0;
        IMessage::ProcessLegStart(leg, value);
    }
    else if (MSG_ROUND == msg_type)
    {
        IMessage::ProcessRound(leg, value);
#ifdef MULTI_THREAD_ROUND
        Round(time_out);
#else
        Round();
#endif
    }
    else if (MSG_LEG_END == msg_type)
    {
##if SHOW_TIME
        printf("Longest Round : %.3f ms\n",CONNECTOR->_longest_time/1000.0);
#endif
        log("Longest Round : %.3f ms\n", CONNECTOR->_longest_time/1000.0);
        log("Leg End!\n");
        CONNECTOR->_longest_time = 0;
    }
    else if (MSG_GAME_OVER == msg_type)
    {
        log("Game Over!\n");
        return false;
    }
    return true;
}

//#define HIGH_ACCURACY_CLOCK
#ifdef HIGH_ACCURACY_CLOCK
void Agent::Round(unsigned int time_out)
{
    LARGE_INTEGER tick_before;
    LARGE_INTEGER tick_now;
    LARGE_INTEGER tick_freq;
    QueryPerformanceFrequency(&tick_freq);
    double freq = tick_freq.QuadPart;

    QueryPerformanceCounter(&tick_before);
    unsigned long long before = tick_before.QuadPart;

    leg.SetActionGetted(false);

    if (round_thread!=NULL)
    {
        delete round_thread;
        round_thread = NULL;
    }
    round_thread = new RoundThread(_ai,leg);
    round_thread->Start();

    while(!round_thread->IsFinished())
    {
        QueryPerformanceCounter(&tick_now);
        unsigned long long now = tick_now.QuadPart;
        float delta = (now - before) * 1000 / freq;
        if (delta > time_out)
		{
            _ai.stop();
            break;
        }
        ::Sleep(1);
    }

    SendActions();
    KillWorkingThread();
}
#else
	void Agent::Round(unsigned int time_out)
{
    unsigned long tickLastTime = ::GetTickCount();
    unsigned long tickNow;

    leg.SetActionGetted(false);
   
    if (round_thread!=NULL)
    {
        delete round_thread;
        round_thread = NULL;
    }
    
    if (leg._map_info.FriendPlayerCount()!=0)
    {
        round_thread = new RoundThread(_ai,leg);
        round_thread->Start();

        while(!round_thread->IsFinished())
        {
            tickNow = ::GetTickCount();
            if(tickNow - tickLastTime > time_out)
            {
                _ai.stop();
                break;
            }
            ::Sleep(1);
        }
    }
    
    SendActions();
    KillWorkingThread();
}

void Agent::Round()
{
    leg.SetActionGetted(false);

    if (leg._map_info.FriendPlayerCount()!=0)
    {
        _ai.update(leg);
    }

    SendActions();
}

#endif

void Agent::SendActions()
{
#ifdef MULTI_THREAD_ROUND
    //进入临界区
    leg.EnterActionSection();
    //是否标记已获取结果
    if (leg.ActionGetted())
    {
        //退出临界区
        leg.LeaveActionSection();
        return;
    }
#endif

    leg.GetActions(_action_list);
    //标记已获取结果
    leg.SetActionGetted(true);

#ifdef MULTI_THREAD_ROUND
    //退出临界区
    leg.LeaveActionSection();
#endif

    ActionMessage msg(leg,_action_list);
    CONNECTOR->send(msg.toString());
}

void Agent::KillWorkingThread()
{
#ifdef MULTI_THREAD_ROUND
    if (round_thread != NULL && !round_thread->IsFinished())
    {
        leg.EnterActionSection();
        round_thread->Kill();
        leg.LeaveActionSection();
    }
#endif
}

void Agent::WaitLastWorkingThreadEnd()
{
#ifdef MULTI_THREAD_ROUND
    if (round_thread != NULL && !round_thread->IsFinished())
    {
        round_thread->Wait();
    }
#endif
}
