#pragma once
#include <Windows.h>
#include "thread.h"

class WatchDog: public Thread
{
	typedef void(CALLBACK*Timerfunc)(void *P);
	typedef Timerfunc TimerHandler;
	
public:
	WatchDog(Agent &agent,unsigned int mill_sec):_agent(agent),_stop_flag(false),_mill_sec(mill_sec)
	{
		
	}
	
	void Run()
	{
		unsigned long tickNow = ::GetTickCount();
		unsigned long tickLastTime = tickNow;
		
		while(!_stop_flag)
		{
			tickNow = ::GetTickCount();
			if(tickNow - tickLastTime > _mill_sec)
			{
				TimeUp();
				return;
			}
			::Sleep(1);
		}	
	}
	
	void Stop()
	{
		_stop_flag = true;
	}
	
	void TimeUp()
	{
		_agent.SendActions();
	}
	
	private:
	Agent &_agent;
	bool _stop_flag;
	unsigned int _mill_sec;
};

	