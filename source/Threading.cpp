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

	unsigned ret = thread->ThreadMain();

	thread->_threadHandle = 0;

	return ret;
}

bool Thread::IsRunning()
{
	return _threadHandle != 0;
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

Mutex::Mutex()
{
	_handle = CreateMutex(NULL, FALSE, NULL);
}

Mutex::~Mutex()
{
	CloseHandle(_handle);
}

void Mutex::Enter()
{
	WaitForSingleObject(_handle, INFINITE);
}

void Mutex::Exit()
{
	ReleaseMutex(_handle);
}

ScopedLock::ScopedLock(Mutex& mutex) :
	_mutex(mutex)
{
	_mutex.Enter();
}

ScopedLock::~ScopedLock()
{
	_mutex.Exit();
}