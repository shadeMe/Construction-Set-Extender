#include "RenderWindowTextPainter.h"

RenderWindowTextPainter*			RenderWindowTextPainter::Singleton = NULL;

RenderChannelBase::Parameters::Parameters( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags )
{
	this->FontHeight = FontHeight;
	this->FontWidth = FontWidth;
	this->FontWeight = FontWeight;
	this->Color = Color;
	this->DrawAreaFlags = DrawAreaFlags;
	this->DrawFormat = DrawFormat;

	sprintf_s(this->FontFace, sizeof(this->FontFace), "%s", FontFace);
	memcpy(&this->DrawArea, DrawArea, sizeof(RECT));
}

RenderChannelBase::RenderChannelBase( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags )
	: Font(NULL), Valid(false), InputParams(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags)
{
	Create();
}

bool RenderChannelBase::Create()
{
	Valid = false;

	memcpy(&RenderArea, &InputParams.DrawArea, sizeof(RECT));
	if (InputParams.DrawAreaFlags)
	{
		tagRECT RenderWindowBounds;
		GetClientRect(*g_HWND_RenderWindow, &RenderWindowBounds);
		UInt32 RenderWindowWidth = RenderWindowBounds.right,
				RenderWindowHeight = RenderWindowBounds.bottom;

		if ((InputParams.DrawAreaFlags & kDrawAreaFlags_RightAligned))
		{
			RenderArea.left =  RenderWindowWidth + InputParams.DrawArea.left;
			RenderArea.right =  RenderWindowWidth;
		}

		if ((InputParams.DrawAreaFlags & kDrawAreaFlags_BottomAligned))
		{
			RenderArea.top = RenderWindowHeight + InputParams.DrawArea.top;
			RenderArea.bottom = RenderWindowHeight;
		}
	}

	if (FAILED(D3DXCreateFont((*g_CSRenderer)->device, InputParams.FontHeight, InputParams.FontWidth, InputParams.FontWeight, 0, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, (LPCTSTR)InputParams.FontFace, &Font)))
	{
		DebugPrint("Failed to create font for RenderChannelBase %08X!", this);
	}
	else 
		Valid = true;

	return Valid;
}

void RenderChannelBase::Release()
{
	SAFERELEASE_D3D(Font);
	Valid = false;
}

bool RenderChannelBase::GetIsValid() const
{
	return Valid;
}

RenderChannelBase::~RenderChannelBase()
{
	Release();
}

StaticRenderChannel::StaticRenderChannel( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags /*= 0*/, RenderHandler RenderCallback )
	: RenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags), RenderText(""), RenderCallback(RenderCallback)
{
	;//
}

StaticRenderChannel::~StaticRenderChannel()
{
	;//
}

void StaticRenderChannel::Render(void* Parameter, LPD3DXSPRITE RenderToSprite)
{
	if (Valid == false || RenderCallback == NULL)
		return;

	RenderText.clear();
	if (RenderCallback(RenderText) == false)
		return;

	Font->DrawTextA(RenderToSprite, RenderText.c_str(), -1, &RenderArea, InputParams.DrawFormat, InputParams.Color);
}

DynamicRenderChannel::QueueTask::QueueTask( const char* Text, float SecondsToDisplay )
	: Text(Text), RemainingTime(SecondsToDisplay)
{
	;//
}

DynamicRenderChannel::DynamicRenderChannel( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags /*= 0*/ )
	: RenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags), RenderQueue()
{
	;//
}

DynamicRenderChannel::~DynamicRenderChannel()
{
	Release();
}

void DynamicRenderChannel::Render(void* Parameter, LPD3DXSPRITE RenderToSprite)
{
	if (Valid == false)
		return;
	else if (RenderQueue.size() < 1 || Parameter == NULL)
		return;

	long double TimePassed = *((long double*)Parameter);
	QueueTask* CurrentTask = RenderQueue.front();

	if (CurrentTask->RemainingTime > 0.0)
	{
		Font->DrawTextA(RenderToSprite, CurrentTask->Text.c_str(), -1, &RenderArea, InputParams.DrawFormat, InputParams.Color);
		CurrentTask->RemainingTime -= TimePassed / 1000.0;
	}
	else
	{
		delete CurrentTask;
		RenderQueue.pop();
	}
}

void DynamicRenderChannel::Release()
{
	if (Valid == false)
		return;

	while (RenderQueue.size())
	{
		QueueTask* CurrentTask = RenderQueue.front();
		delete CurrentTask;
		RenderQueue.pop();
	}

	RenderChannelBase::Release();
}

bool DynamicRenderChannel::Queue( float SecondsToDisplay, const char* Format, ... )
{
	if (Valid == false || Format == NULL)
		return false;
	
	char Buffer[0x200] = {0};
	va_list Args;
	va_start(Args, Format);
	vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
	va_end(Args);

	if (strlen(Buffer) && SecondsToDisplay > 0)
		RenderQueue.push(new QueueTask(Buffer, SecondsToDisplay));
	else
		return false;

	return true;
}

UInt32 DynamicRenderChannel::GetQueueSize() const
{
	return RenderQueue.size();
}

RenderWindowTextPainter::RenderWindowTextPainter()
	: RegisteredChannels(), RenderWindowTimeCounter(), RenderToSprite(NULL), Enabled(true)
{
	ReleaseSprite(true);
	RenderWindowTimeCounter.Update();
}

bool RenderWindowTextPainter::LookupRenderChannel( RenderChannelBase* Channel, RenderChannelListT::iterator& MatchIterator )
{
	bool Result = false;

	for (RenderChannelListT::iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
	{
		if (*Itr == Channel)
		{
			MatchIterator = Itr;
			Result = true;
			break;
		}
	}

	return Result;
}

RenderWindowTextPainter* RenderWindowTextPainter::GetSingleton(void)
{
	if (Singleton == NULL)
		Singleton = new RenderWindowTextPainter();

	return Singleton;
}

void RenderWindowTextPainter::Render()
{
	RenderWindowTimeCounter.Update();

	if (Enabled == false || RenderToSprite == NULL)
		return;

	long double TimePassed = RenderWindowTimeCounter.GetTimePassedSinceLastUpdate();

	RenderToSprite->Begin(D3DXSPRITE_ALPHABLEND|D3DXSPRITE_SORT_TEXTURE);
	for (RenderChannelListT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
		(*Itr)->Render(&TimePassed, RenderToSprite);
	RenderToSprite->End();
}

bool RenderWindowTextPainter::Release( bool Recreate )
{
	bool Result = ReleaseSprite(Recreate);

	for (RenderChannelListT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
	{
		(*Itr)->Release();

		if (Recreate)
		{
			if ((*Itr)->Create() == false)
				Result = false;
		}
	}

	return Result;
}

bool RenderWindowTextPainter::RegisterRenderChannel( RenderChannelBase* Channel )
{
	RenderChannelListT::iterator Match;
	if (LookupRenderChannel(Channel, Match) == false)
	{
		RegisteredChannels.push_back(Channel);
		return true;
	}
	else
		return false;
}

bool RenderWindowTextPainter::UnregisterRenderChannel( RenderChannelBase* Channel )
{
	RenderChannelListT::iterator Match;
	if (LookupRenderChannel(Channel, Match))
	{
		delete Channel;
		RegisteredChannels.erase(Match);
		return true;
	}
	else
		return false;
}

void RenderWindowTextPainter::SetEnabled( bool State )
{
	Enabled = State;
}

bool RenderWindowTextPainter::GetEnabled( void ) const
{
	return Enabled;
}

bool RenderWindowTextPainter::GetHasActiveTasks( void ) const
{
	for (RenderChannelListT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
	{
		DynamicRenderChannel* Dynamic = dynamic_cast<DynamicRenderChannel*>(*Itr);
		if (Dynamic && Dynamic->GetQueueSize())
			return true;
	}

	return false;
}

void RenderWindowTextPainter::Deinitialize()
{
	for (RenderChannelListT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
		delete *Itr;

	RegisteredChannels.clear();
}

bool RenderWindowTextPainter::ReleaseSprite( bool Recreate /*= true*/ )
{
	bool Result = true;
	SAFERELEASE_D3D(RenderToSprite);

	if (Recreate)
	{
		if (FAILED(D3DXCreateSprite(_RENDERER->device, &RenderToSprite)))
		{
			DebugPrint("Failed to create sprite for RenderTextWindowPainter");
			Result = false;
		}
	}
	return Result;
}
