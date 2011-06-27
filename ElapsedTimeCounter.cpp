#include "ElapsedTimeCounter.h"

void ElapsedTimeCounter::Update(void)
{
	QueryPerformanceCounter(&FrameBuffer);
	TimePassed = ((FrameBuffer.QuadPart - ReferenceFrame.QuadPart ) * 1000.0 / TimerFrequency.QuadPart);
	ReferenceFrame = FrameBuffer;
}