#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "BGSEERenderWindowPainter.h"

namespace BGSEditorExtender
{
	BGSEERenderChannelBase::Parameters::Parameters( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags )
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

	BGSEERenderChannelBase::BGSEERenderChannelBase( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags ) :
		Font(NULL),
		Valid(false),
		InputParams(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags)
	{
		CreateD3D();
	}

	bool BGSEERenderChannelBase::CreateD3D(void)
	{
		Valid = false;

		memcpy(&RenderArea, &InputParams.DrawArea, sizeof(RECT));
		if (InputParams.DrawAreaFlags)
		{
			RECT RenderWindowBounds;
			GetClientRect(BGSEERWPAINTER->GetD3DWindow(), &RenderWindowBounds);

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

		HRESULT OpResult = D3DXCreateFont(BGSEERWPAINTER->GetD3DDevice(),
										InputParams.FontHeight, InputParams.FontWidth,
										InputParams.FontWeight, 0, FALSE,
										DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
										DEFAULT_PITCH|FF_DONTCARE,
										(LPCTSTR)InputParams.FontFace, &Font);
		if (FAILED(OpResult))
			BGSEECONSOLE_MESSAGE("Failed to create font for RenderChannelBase\n\tResult = %d; %08X", OpResult, OpResult);
		else
			Valid = true;

		return Valid;
	}

	void BGSEERenderChannelBase::ReleaseD3D(void)
	{
		SAFERELEASE_D3D(Font);
		Valid = false;
	}

	bool BGSEERenderChannelBase::GetIsValid() const
	{
		return Valid;
	}

	BGSEERenderChannelBase::~BGSEERenderChannelBase()
	{
		ReleaseD3D();
	}

	BGSEEStaticRenderChannel::BGSEEStaticRenderChannel( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags /*= 0*/, RenderHandler RenderCallback ) :
		BGSEERenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags),
		RenderText(""),
		RenderCallback(RenderCallback)
	{
		;//
	}

	BGSEEStaticRenderChannel::~BGSEEStaticRenderChannel()
	{
		RenderText.clear();
		RenderCallback = NULL;
	}

	void BGSEEStaticRenderChannel::Render(void* Parameter, LPD3DXSPRITE RenderToSprite)
	{
		if (Valid == false || RenderCallback == NULL)
			return;

		RenderText.clear();
		if (RenderCallback(RenderText) == false)
			return;

		Font->DrawTextA(RenderToSprite, RenderText.c_str(), -1, &RenderArea, InputParams.DrawFormat, InputParams.Color);
	}

	BGSEEDynamicRenderChannel::RenderTask::RenderTask( const char* Text, float SecondsToDisplay ) :
		Text(Text),
		RemainingTime(SecondsToDisplay)
	{
		;//
	}

	BGSEEDynamicRenderChannel::BGSEEDynamicRenderChannel( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags /*= 0*/ ) :
		BGSEERenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags),
		TaskQueue()
	{
		;//
	}

	BGSEEDynamicRenderChannel::~BGSEEDynamicRenderChannel()
	{
		while (TaskQueue.size())
		{
			RenderTask* CurrentTask = TaskQueue.front();
			delete CurrentTask;
			TaskQueue.pop();
		}
	}

	void BGSEEDynamicRenderChannel::Render(void* Parameter, LPD3DXSPRITE RenderToSprite)
	{
		if (Valid == false)
			return;
		else if (TaskQueue.size() < 1 || Parameter == NULL)
			return;

		long double TimePassed = *((long double*)Parameter);
		RenderTask* CurrentTask = TaskQueue.front();

		if (CurrentTask->RemainingTime > 0.0)
		{
			Font->DrawTextA(RenderToSprite, CurrentTask->Text.c_str(), -1, &RenderArea, InputParams.DrawFormat, InputParams.Color);
			CurrentTask->RemainingTime -= TimePassed / 1000.0;
		}
		else
		{
			delete CurrentTask;
			TaskQueue.pop();
		}
	}

	bool BGSEEDynamicRenderChannel::Queue( float SecondsToDisplay, const char* Format, ... )
	{
		if (Valid == false || Format == NULL)
			return false;

		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		if (strlen(Buffer) && SecondsToDisplay > 0)
			TaskQueue.push(new RenderTask(Buffer, SecondsToDisplay));
		else
			return false;

		return true;
	}

	UInt32 BGSEEDynamicRenderChannel::GetQueueSize() const
	{
		return TaskQueue.size();
	}

	BGSEERenderWindowPainter::BGSEERenderWindowPainter()
	{
		OutputSprite = NULL;
		D3DDevice = NULL;
		D3DWindow = NULL;
		Enabled = false;
		Initialized = false;
	}

	BGSEERenderWindowPainter::~BGSEERenderWindowPainter()
	{
		Singleton = NULL;

		ReleaseD3D();
		for (RenderChannelListT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
			delete *Itr;

		RegisteredChannels.clear();
		SAFEDELETE(D3DDevice);
		SAFEDELETE(D3DWindow);
	}

	bool BGSEERenderWindowPainter::Initialize( HWND RenderWindowHandle, LPDIRECT3DDEVICE9 RendererD3DDevice )
	{
		if (Initialized)
			return false;

		SME_ASSERT(RenderWindowHandle &&  RendererD3DDevice);
		Initialized = true;

		SAFEDELETE(D3DDevice);
		SAFEDELETE(D3DWindow);
		D3DWindow = new HWNDGetter(RenderWindowHandle);
		D3DDevice = new D3D9DeviceGetter(RendererD3DDevice);

		Initialized = CreateD3D();
		return Initialized;
	}

	bool BGSEERenderWindowPainter::LookupRenderChannel( BGSEERenderChannelBase* Channel, RenderChannelListT::iterator& Match )
	{
		bool Result = false;

		for (RenderChannelListT::iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
		{
			if (*Itr == Channel)
			{
				Match = Itr;
				Result = true;
				break;
			}
		}

		return Result;
	}

	bool BGSEERenderWindowPainter::CreateD3D( void )
	{
		bool Result = true;
		HRESULT OpResult = D3DXCreateSprite(D3DDevice->operator()(), &OutputSprite);

		if (FAILED(OpResult))
		{
			BGSEECONSOLE_MESSAGE("Failed to create output sprite for RenderTextWindowPainter\n\tResult = %d; %08X", OpResult, OpResult);
			Result = false;
		}

		return Result;
	}

	void BGSEERenderWindowPainter::ReleaseD3D( void )
	{
		SAFERELEASE_D3D(OutputSprite);
	}

	BGSEERenderWindowPainter* BGSEERenderWindowPainter::GetSingleton(void)
	{
		if (Singleton == NULL)
			Singleton = new BGSEERenderWindowPainter();

		return Singleton;
	}

	void BGSEERenderWindowPainter::Render()
	{
		SME_ASSERT(Initialized);

		TimeCounter.Update();

		if (Enabled == false || OutputSprite == NULL)
			return;

		long double TimePassed = TimeCounter.GetTimePassed();

		OutputSprite->Begin(D3DXSPRITE_ALPHABLEND|D3DXSPRITE_SORT_TEXTURE);

		for (RenderChannelListT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
			(*Itr)->Render(&TimePassed, OutputSprite);

		OutputSprite->End();
	}

	bool BGSEERenderWindowPainter::HandleD3DDeviceReset( UInt8 Operation )
	{
		bool Result = true;

		switch (Operation)
		{
		case kDeviceReset_Release:
			ReleaseD3D();
			break;
		case kDeviceReset_Renew:
			Result = CreateD3D();
			break;
		}

		for (RenderChannelListT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
		{
			switch (Operation)
			{
			case kDeviceReset_Release:
				(*Itr)->ReleaseD3D();
				break;
			case kDeviceReset_Renew:
				if ((*Itr)->CreateD3D() == false)
					Result = false;

				break;
			}
		}

		return Result;
	}

	bool BGSEERenderWindowPainter::RegisterRenderChannel( BGSEERenderChannelBase* Channel )
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

	void BGSEERenderWindowPainter::UnregisterRenderChannel( BGSEERenderChannelBase* Channel )
	{
		RenderChannelListT::iterator Match;
		if (LookupRenderChannel(Channel, Match))
		{
			delete Channel;
			RegisteredChannels.erase(Match);
		}
	}

	void BGSEERenderWindowPainter::SetEnabled( bool State )
	{
		Enabled = State;
	}

	bool BGSEERenderWindowPainter::GetEnabled( void ) const
	{
		return Enabled;
	}

	bool BGSEERenderWindowPainter::GetHasActiveTasks( void ) const
	{
		for (RenderChannelListT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
		{
			BGSEEDynamicRenderChannel* Dynamic = dynamic_cast<BGSEEDynamicRenderChannel*>(*Itr);
			if (Dynamic && Dynamic->GetQueueSize())
				return true;
		}

		return false;
	}

	HWND BGSEERenderWindowPainter::GetD3DWindow( void ) const
	{
		SME_ASSERT(D3DWindow);
		return D3DWindow->operator()();
	}

	LPDIRECT3DDEVICE9 BGSEERenderWindowPainter::GetD3DDevice( void ) const
	{
		SME_ASSERT(D3DDevice);
		return D3DDevice->operator()();
	}
}