#pragma once

#include <Windows.h>

class Thread
{
	public:
	Thread();
	virtual ~Thread();
	
	virtual void Run() =0;
	void Start();
	bool IsFinished();
	void Wait();
	void Kill();
	
	protected:
	static unsigned int __stdcall ThreadProc(void*p);
	
	private:
	bool _is_finished;
	HANDLE _hThread;
};

	