#include "Thread.h"
#include <process.h>

Thread::Thread():_is_finished(false), _hThread(INVALID_HANDLE_VALUE)
{
}

Thread::~Thread()
{
}

void Thread::Start()
{
	unsigned int thID1;
	_hThread = (HANDLE)::_beginthreadex(NULL,0,ThreadProc,this,0,&thID1);
}

unsigned int __stdcall Thread::ThreadProc(void *p)
{
	Thread *thread = (Thread*)p;
	thread->Run();
	thread->_is_finished = true;
	CloseHandle(thread->_hThread);
	thread->_hThread = INVALID_HANDLE_VALUE;
	return 0;
}

bool Thread::IsFinished()
{
	return _is_finished;
}

void Thread::Wait()
{
	if(_hThread != INVALID_HANDLE_VALUE)
	{
		if(WaitForSingleObject(_hThread,INFINITE) != WAIT_ABANDONED)
		{
			CloseHandle(_hThread);
		}
		_hThread = INVALID_HANDLE_VALUE;
	}
}

void Thread::Kill()
{
	if(_hThread != INVALID_HANDLE_VALUE)
	{
		TerminateThread(_hThread,1);
	}
}
