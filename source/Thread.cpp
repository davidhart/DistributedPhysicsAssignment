#include "Thread.h"
#include <process.h>

Thread::Thread() :
	_threadHandle(0)
{
}

Thread::~Thread()
{
}

void Thread::Start(ThreadStart& start)
{
	_threadHandle = (HANDLE)_beginthreadex(NULL, 0, &ThreadStartBootstrap, &start, 0, NULL);
	// TODO: test for errror
}

void Thread::Join()
{
	WaitForSingleObject(_threadHandle, INFINITE);
}

unsigned __stdcall Thread::ThreadStartBootstrap(void* data)
{
	ThreadStart* threadstart = (ThreadStart*)data;

	return threadstart->Start();
}