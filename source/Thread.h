// David Hart - 2012

#pragma once

#include <Windows.h>

class ThreadStart
{

public:

	virtual ~ThreadStart() { }
	virtual unsigned Start() = 0;

};

class Thread
{

public:

	Thread();
	~Thread();

	void Start(ThreadStart& start);
	void Join();

private:

	static unsigned __stdcall ThreadStartBootstrap(void* data);
	HANDLE _threadHandle;

};