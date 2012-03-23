// David Hart - 2012

#include "Threading.h"
#include <process.h>

using namespace Threading;

Thread::Thread() :
	_threadHandle(0)
{
}

Thread::~Thread()
{
}

void Thread::Start()
{
	_threadHandle = (HANDLE)_beginthreadex(NULL, 0, &ThreadStartBootstrap, this, 0, NULL);
	// TODO: test for errror
}

void Thread::Join()
{
	WaitForSingleObject(_threadHandle, INFINITE);
	// TODO: argument for wait duration
}

unsigned __stdcall Thread::ThreadStartBootstrap(void* data)
{
	Thread* thread = (Thread*)data;

	return thread->ThreadMain();
}

Event::Event()
{
	_handle = CreateEvent(NULL,
				TRUE, // Manually reset event
				FALSE, // Initial state: unset
				NULL);  // Event has no name
}

Event::~Event()
{
	CloseHandle(_handle);
}

void Event::Wait()
{
	WaitForSingleObject(_handle, INFINITE);
}

void Event::Raise()
{
	SetEvent(_handle);
}

void Event::Reset()
{
	ResetEvent(_handle);
}