#include "RenderTimeManager.h"

RenderTimeManager					g_RenderTimeManager;

void RenderTimeManager::Update(void)
{
	QueryPerformanceCounter(&FrameBuffer);
	TimePassed = ((LONGLONG)((FrameBuffer.QuadPart - ReferenceFrame.QuadPart ) * 1000 / TimerFrequency.QuadPart)) / 1000.0;
	ReferenceFrame = FrameBuffer;
}