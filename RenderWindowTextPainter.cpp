#include "RenderWindowTextPainter.h"
#include "ElapsedTimeCounter.h"

RenderWindowTextPainter*			RenderWindowTextPainter::Singleton = NULL;

void RenderWindowTextPainter::StaticRenderChannel::Render()
{
	if (Valid == false)
		return;
	else if (TextToRender.length() < 1)
		return;

	Font->DrawTextA(NULL, TextToRender.c_str(), -1, &DrawArea, 0, Color);
}

void RenderWindowTextPainter::StaticRenderChannel::Queue(const char* Text)
{
	if (Valid == false)
		return;

	if (Text)
		TextToRender = Text;
	else
		TextToRender.clear();
}

void RenderWindowTextPainter::DynamicRenderChannel::Render()
{
	if (Valid == false)
		return;
	else if (DrawQueue.size() < 1)
		return;

	QueueTask* CurrentTask = DrawQueue.front();

	if (CurrentTask->RemainingTime > 0)
	{
		Font->DrawTextA(NULL, CurrentTask->Text.c_str(), -1, &DrawArea, 0, Color);
		CurrentTask->RemainingTime -= g_RenderWindowTimeManager.GetTimePassedSinceLastUpdate() / 1000.0;
	}
	else
	{
		delete CurrentTask;
		DrawQueue.pop();
	}
}

void RenderWindowTextPainter::DynamicRenderChannel::Queue(const char* Text, long double SecondsToDisplay)
{
	if (Valid == false)
		return;

	if (GetQueueSize() == 0)
		g_RenderWindowTimeManager.Update();

	if (Text && SecondsToDisplay > 0)
		DrawQueue.push(new QueueTask(Text, SecondsToDisplay));
}

void RenderWindowTextPainter::DynamicRenderChannel::Release()
{
	if (Valid == false)
		return;

	while (DrawQueue.size())
	{
		QueueTask* CurrentTask = DrawQueue.front();
		delete CurrentTask;
		DrawQueue.pop();
	}

	RenderChannelBase::Release();
}

RenderWindowTextPainter::RenderWindowTextPainter()
{
	RenderChannel1 = NULL;
	RenderChannel2 = NULL;
	Valid = false;
}

RenderWindowTextPainter* RenderWindowTextPainter::GetSingleton(void)
{
	if (Singleton == NULL)
		Singleton = new RenderWindowTextPainter();
	return Singleton;
}

bool RenderWindowTextPainter::Initialize()
{
	if (Valid)
		return true;

	CONSOLE->Indent();

	RECT DrawRect;
	DrawRect.left = 3;
	DrawRect.top = 3;
	DrawRect.right = 1280;
	DrawRect.bottom = 600;
	RenderChannel1 = new StaticRenderChannel(12, 7, FW_THIN, "Lucida Console", D3DCOLOR_ARGB(220, 189, 237, 99), &DrawRect);

	DrawRect.top += 350;
	RenderChannel2 = new DynamicRenderChannel(12, 7, FW_THIN, "Lucida Console", D3DCOLOR_ARGB(255, 190, 35, 47), &DrawRect);

	if (RenderChannel1->GetIsValid() == false || RenderChannel2->GetIsValid() == false)
		Valid = false;
	else
		Valid = true;

	CONSOLE->Exdent();
	return Valid;
}

void RenderWindowTextPainter::Release()
{
	if (!RenderChannel1 || !RenderChannel2)
		return;

	RenderChannel1->Release();
	RenderChannel2->Release();

	delete RenderChannel1;
	delete RenderChannel2;

	Valid = false;
}

void RenderWindowTextPainter::Render()
{
	if (Valid == false)
		return;

	RenderChannel1->Render();
	RenderChannel2->Render();
}

void RenderWindowTextPainter::QueueDrawTask(UInt8 Channel, const char* Text, long double SecondsToDisplay)
{
	if (Valid == false)
		return;

	switch (Channel)
	{
	case kRenderChannel_1:
		RenderChannel1->Queue(Text);
		break;
	case kRenderChannel_2:
		RenderChannel2->Queue(Text, SecondsToDisplay);
		break;
	}
}

UInt32 RenderWindowTextPainter::GetRenderChannelQueueSize(UInt8 Channel)
{
	if (Valid == false)
		return 0;

	switch (Channel)
	{
	case kRenderChannel_1:
		return RenderChannel1->GetQueueSize();
	case kRenderChannel_2:
		return RenderChannel2->GetQueueSize();
	}

	return 0;
}