// David Hart - 2012

#include "Timer.h"

Timer::Timer()
{
	Start();
}

void Timer::Start()
{
	QueryPerformanceCounter(&_startTime);
	QueryPerformanceFrequency(&_freq);
}

double Timer::GetTime()
{
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);
	
	return (double)(endTime.QuadPart-_startTime.QuadPart)/_freq.QuadPart;
}
