// David Hart - 2012

#pragma once

#include <Windows.h>

namespace Threading
{

	class Thread
	{

	public:

		Thread();
		~Thread();

		void Start();

		virtual unsigned ThreadMain() = 0;

		void Join();

	private:

		static unsigned __stdcall ThreadStartBootstrap(void* data);
		HANDLE _threadHandle;

	};

	class Event
	{

	public:

		Event();
		~Event();
		void Wait();
		void Raise();
		void Reset();

	private:

		HANDLE _handle;

	};

}