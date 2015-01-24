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

		if (DrawArea == NULL)
			ZeroMemory(&this->DrawArea, sizeof(RECT));
		else
			memcpy(&this->DrawArea, DrawArea, sizeof(RECT));
	}

	BGSEERenderChannelBase::BGSEERenderChannelBase( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags ) :
		Font(NULL),
		Valid(false),
		InputParams(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags)
	{
		;//
	}

	bool BGSEERenderChannelBase::CreateD3D(LPDIRECT3DDEVICE9 Device, HWND Window)
	{
		Valid = false;

		memcpy(&RenderArea, &InputParams.DrawArea, sizeof(RECT));
		if (InputParams.DrawAreaFlags)
		{
			RECT RenderWindowBounds;
			GetClientRect(Window, &RenderWindowBounds);

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

		SME_ASSERT(Font == NULL);
		HRESULT OpResult = D3DXCreateFont(Device,
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

	BGSEEStaticRenderChannel::BGSEEStaticRenderChannel(INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags /*= kDrawAreaFlags_Default*/) :
		BGSEERenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags),
		LastRenderedText("")
	{
		;//
	}

	BGSEEStaticRenderChannel::~BGSEEStaticRenderChannel()
	{
		;//
	}

	void BGSEEStaticRenderChannel::Render(LPD3DXSPRITE RenderToSprite)
	{
		if (Valid == false)
			return;

		std::string Drawable;
		if (DrawText(Drawable) == false)
			return;

		LastRenderedText = Drawable;
		Font->DrawTextA(RenderToSprite, LastRenderedText.c_str(), -1, &RenderArea, InputParams.DrawFormat, InputParams.Color);
	}

	VOID CALLBACK BGSEEDynamicRenderChannelScheduler::UpdateTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	{
		BGSEEDynamicRenderChannelScheduler::GetSingleton()->UpdateTasks(dwTime);
	}

	void BGSEEDynamicRenderChannelScheduler::UpdateTasks(DWORD CurrentTickCount)
	{
		for each (auto Itr in TaskRegistry)
		{
			Itr->UpdateTasks(CurrentTickCount);
		}
	}

	BGSEEDynamicRenderChannelScheduler::BGSEEDynamicRenderChannelScheduler() :
		TaskRegistry(),
		TimerID(0)
	{
		TimerID = SetTimer(NULL, NULL, kTimerPeriod, &UpdateTimerProc);
		SME_ASSERT(TimerID);
	}

	BGSEEDynamicRenderChannelScheduler::~BGSEEDynamicRenderChannelScheduler()
	{
		TaskRegistry.clear();
	}

	bool BGSEEDynamicRenderChannelScheduler::Register(BGSEEDynamicRenderChannel* Channel)
	{
		SME_ASSERT(Channel);

		for each (auto Itr in TaskRegistry)
		{
			if (Itr == Channel)
				return false;
		}

		TaskRegistry.push_back(Channel);
		return true;
	}

	void BGSEEDynamicRenderChannelScheduler::Unregister(BGSEEDynamicRenderChannel* Channel)
	{
		SME_ASSERT(Channel);

		for (UpdateCallbackListT::const_iterator Itr = TaskRegistry.begin(); Itr != TaskRegistry.end(); Itr++)
		{
			if (*Itr == Channel)
			{
				TaskRegistry.erase(Itr);
				return;
			}
		}
	}

	BGSEEDynamicRenderChannelScheduler* BGSEEDynamicRenderChannelScheduler::GetSingleton(void)
	{
		static BGSEEDynamicRenderChannelScheduler kInstance;

		return &kInstance;
	}

	BGSEEDynamicRenderChannel::Task::Task(const char* Text, DWORD Duration) :
		Drawable(Text),
		Duration(Duration),
		StartTickCount(0)
	{
		;//
	}

	void BGSEEDynamicRenderChannel::Render(LPD3DXSPRITE RenderToSprite)
	{
		if (Valid == false)
			return;
		else if (QueuedItems.size() == 0)
			return;

		Task& Current = QueuedItems.front();
		Font->DrawTextA(RenderToSprite, Current.Drawable.c_str(), -1, &RenderArea, InputParams.DrawFormat, InputParams.Color);
	}

	void BGSEEDynamicRenderChannel::UpdateTasks(DWORD CurrentTickCount)
	{
		if (QueuedItems.size())
		{
			Task& Current = QueuedItems.front();
			if (Current.StartTickCount == 0)
			{
				// new task
				Current.StartTickCount = CurrentTickCount;
				BGSEERWPAINTER->Redraw();
			}
			else if (CurrentTickCount - Current.StartTickCount > Current.Duration)
			{
				// elapsed
				QueuedItems.pop();
				if (QueuedItems.size())
				{
					Current = QueuedItems.front();
					SME_ASSERT(Current.StartTickCount == 0);
					Current.StartTickCount = CurrentTickCount;
				}

				BGSEERWPAINTER->Redraw();
			}
		}
	}

	BGSEEDynamicRenderChannel::BGSEEDynamicRenderChannel(INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags /*= kDrawAreaFlags_Default*/) :
		BGSEERenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags),
		QueuedItems()
	{
		BGSEEDynamicRenderChannelScheduler::GetSingleton()->Register(this);
	}

	BGSEEDynamicRenderChannel::~BGSEEDynamicRenderChannel()
	{
		while (QueuedItems.size())
			QueuedItems.pop();

		BGSEEDynamicRenderChannelScheduler::GetSingleton()->Unregister(this);
	}

	bool BGSEEDynamicRenderChannel::Queue(float DurationInSeconds, const char* Format, ...)
	{
		if (Valid == false || Format == NULL)
			return false;

		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		if (strlen(Buffer) == 0 || DurationInSeconds < 1 || DurationInSeconds > 10)
			return false;

		QueuedItems.push(Task(Buffer, DurationInSeconds * 1000));
		return true;
	}

	BGSEERenderWindowPainter* BGSEERenderWindowPainter::Singleton = NULL;

	BGSEERenderWindowPainter::BGSEERenderWindowPainter() :
		RegisteredChannels(),
		Operator(NULL),
		OutputSprite(NULL),
		Enabled(true),
		Initialized(false)
	{
		;//
	}

	BGSEERenderWindowPainter::~BGSEERenderWindowPainter()
	{
		ReleaseD3D();
		delete Operator;

		for each (auto Itr in RegisteredChannels)
			Itr->ReleaseD3D();

		Singleton = NULL;
	}

	bool BGSEERenderWindowPainter::Initialize(BGSEERenderWindowPainterOperator* Operator)
	{
		if (Initialized)
			return false;

		SME_ASSERT(Operator);

		Initialized = true;
		this->Operator = Operator;
		Initialized = CreateD3D();
		return Initialized;
	}

	bool BGSEERenderWindowPainter::CreateD3D( void )
	{
		SME_ASSERT(OutputSprite == NULL);

		bool Result = true;
		HRESULT OpResult = D3DXCreateSprite(Operator->GetD3DDevice(), &OutputSprite);

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
		if (Initialized == false)
			return;
		else if (Enabled == false || OutputSprite == NULL)
			return;

		OutputSprite->Begin(D3DXSPRITE_ALPHABLEND|D3DXSPRITE_SORT_TEXTURE);

		for each (auto Itr in RegisteredChannels)
			Itr->Render(OutputSprite);

		OutputSprite->End();
	}

	bool BGSEERenderWindowPainter::HandleReset(bool Release, bool Renew)
	{
		if (Initialized == false)
			return false;

		SME_ASSERT(Release != Renew);

		bool Result = true;

		if (Release)
			ReleaseD3D();
		else
			Result = CreateD3D();

		for each (auto Itr in RegisteredChannels)
		{
			if (Release)
				Itr->ReleaseD3D();
			else if (Itr->CreateD3D(Operator->GetD3DDevice(), Operator->GetD3DWindow()) == false)
				Result = false;
		}

		return Result;
	}

	bool BGSEERenderWindowPainter::RegisterRenderChannel( BGSEERenderChannelBase* Channel )
	{
		SME_ASSERT(Initialized && Channel);
		for each (auto Itr in RegisteredChannels)
		{
			if (Itr == Channel)
				return false;
		}

		RegisteredChannels.push_back(Channel);
		Channel->CreateD3D(Operator->GetD3DDevice(), Operator->GetD3DWindow());
		return true;
	}

	void BGSEERenderWindowPainter::UnregisterRenderChannel( BGSEERenderChannelBase* Channel )
	{
		SME_ASSERT(Initialized && Channel);

		for (RenderChannelListT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
		{
			if (*Itr == Channel)
			{
				RegisteredChannels.erase(Itr);
				Channel->ReleaseD3D();
				return;
			}
		}
	}

	void BGSEERenderWindowPainter::SetEnabled( bool State )
	{
		if (Initialized == false)
			return;

		Enabled = State;
	}

	bool BGSEERenderWindowPainter::GetEnabled( void ) const
	{
		if (Initialized == false)
			return false;

		return Enabled;
	}

	void BGSEERenderWindowPainter::Redraw(void) const
	{
		Operator->RedrawRenderWindow();
	}
}