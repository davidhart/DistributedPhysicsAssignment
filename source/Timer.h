#pragma once

#include <Windows.h>

class Timer
{

public:

	Timer();
	void Start();
	double GetTime();

private:

	LARGE_INTEGER _startTime;
	LARGE_INTEGER _freq;
};