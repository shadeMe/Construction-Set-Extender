#pragma once
#include "ExtenderInternals.h"

class RenderTimeManager
{
	LARGE_INTEGER				ReferenceFrame;
	LARGE_INTEGER				FrameBuffer;
	LARGE_INTEGER				TimerFrequency;
	long double					TimePassed;					// in seconds
public:

	RenderTimeManager()
	{
		QueryPerformanceCounter(&ReferenceFrame);
		QueryPerformanceFrequency(&TimerFrequency);
	}

	void								Update(void);
	long double							GetTimePassedSinceLastFrame(void) { return TimePassed; }
};

extern RenderTimeManager		g_RenderTimeManager;