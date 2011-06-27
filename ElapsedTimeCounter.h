#pragma once
#include "ExtenderInternals.h"

class ElapsedTimeCounter
{
	LARGE_INTEGER				ReferenceFrame;
	LARGE_INTEGER				FrameBuffer;
	LARGE_INTEGER				TimerFrequency;
	long double					TimePassed;					// in ms
public:

	ElapsedTimeCounter()
	{
		QueryPerformanceCounter(&ReferenceFrame);
		QueryPerformanceFrequency(&TimerFrequency);
	}

	void								Update(void);
	long double							GetTimePassedSinceLastUpdate(void) const { return TimePassed; }
};