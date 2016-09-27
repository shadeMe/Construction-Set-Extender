#include "Main.h"
#include "Console.h"
#include "RenderWindowPainter.h"

namespace bgsee
{
	RenderChannelBase::Parameters::Parameters( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags )
	{
		this->FontHeight = FontHeight;
		this->FontWidth = FontWidth;
		this->FontWeight = FontWeight;
		this->Color = Color;
		this->DrawAreaFlags = DrawAreaFlags;
		this->DrawFormat = DrawFormat;

		sprintf_s(this->FontFace, sizeof(this->FontFace), "%s", FontFace);

		if (DrawArea == nullptr)
			ZeroMemory(&this->DrawArea, sizeof(RECT));
		else
			memcpy(&this->DrawArea, DrawArea, sizeof(RECT));
	}

	RenderChannelBase::RenderChannelBase( INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags ) :
		Font(nullptr),
		Valid(false),
		InputParams(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags)
	{
		;//
	}

	bool RenderChannelBase::CreateD3D(LPDIRECT3DDEVICE9 Device, HWND Window)
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

		SME_ASSERT(Font == nullptr);
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

	void RenderChannelBase::ReleaseD3D(void)
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
		ReleaseD3D();
	}

	StaticRenderChannel::StaticRenderChannel(INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags /*= kDrawAreaFlags_Default*/) :
		RenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags),
		LastRenderedText("")
	{
		;//
	}

	StaticRenderChannel::~StaticRenderChannel()
	{
		;//
	}

	void StaticRenderChannel::Render(LPD3DXSPRITE RenderToSprite)
	{
		if (Valid == false)
			return;

		std::string Drawable;
		if (DrawText(Drawable) == false)
			return;

		LastRenderedText = Drawable;
		Font->DrawTextA(RenderToSprite, LastRenderedText.c_str(), -1, &RenderArea, InputParams.DrawFormat, InputParams.Color);
	}

	VOID CALLBACK DynamicRenderChannelScheduler::UpdateTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	{
		DynamicRenderChannelScheduler::GetSingleton()->UpdateTasks(dwTime);
	}

	void DynamicRenderChannelScheduler::UpdateTasks(DWORD CurrentTickCount)
	{
		for (auto Itr : TaskRegistry)
			Itr->UpdateTasks(CurrentTickCount);
	}

	DynamicRenderChannelScheduler::DynamicRenderChannelScheduler() :
		TaskRegistry(),
		TimerID(0)
	{
		TimerID = SetTimer(nullptr, 0, kTimerPeriod, &UpdateTimerProc);
		SME_ASSERT(TimerID);
	}

	DynamicRenderChannelScheduler::~DynamicRenderChannelScheduler()
	{
		TaskRegistry.clear();
	}

	bool DynamicRenderChannelScheduler::Register(DynamicRenderChannel* Channel)
	{
		SME_ASSERT(Channel);

		for (auto Itr : TaskRegistry)
		{
			if (Itr == Channel)
				return false;
		}

		TaskRegistry.push_back(Channel);
		return true;
	}

	void DynamicRenderChannelScheduler::Unregister(DynamicRenderChannel* Channel)
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

	DynamicRenderChannelScheduler* DynamicRenderChannelScheduler::GetSingleton(void)
	{
		static DynamicRenderChannelScheduler kInstance;

		return &kInstance;
	}

	DynamicRenderChannel::Task::Task(const char* Text, DWORD Duration) :
		Drawable(Text),
		Duration(Duration),
		StartTickCount(0)
	{
		;//
	}

	void DynamicRenderChannel::Render(LPD3DXSPRITE RenderToSprite)
	{
		if (Valid == false)
			return;
		else if (QueuedItems.size() == 0)
			return;

		Task& Current = QueuedItems.front();
		Font->DrawTextA(RenderToSprite, Current.Drawable.c_str(), -1, &RenderArea, InputParams.DrawFormat, InputParams.Color);
	}

	void DynamicRenderChannel::UpdateTasks(DWORD CurrentTickCount)
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

	DynamicRenderChannel::DynamicRenderChannel(INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, D3DCOLOR Color, RECT* DrawArea, DWORD DrawFormat, UInt32 DrawAreaFlags /*= kDrawAreaFlags_Default*/) :
		RenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea, DrawFormat, DrawAreaFlags),
		QueuedItems()
	{
		DynamicRenderChannelScheduler::GetSingleton()->Register(this);
	}

	DynamicRenderChannel::~DynamicRenderChannel()
	{
		while (QueuedItems.size())
			QueuedItems.pop();

		DynamicRenderChannelScheduler::GetSingleton()->Unregister(this);
	}

	bool DynamicRenderChannel::Queue(float DurationInSeconds, const char* Format, ...)
	{
		if (Valid == false || Format == nullptr)
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

	RenderWindowPainter* RenderWindowPainter::Singleton = nullptr;

	RenderWindowPainter::RenderWindowPainter(RenderWindowPainterOperator* Operator) :
		RegisteredChannels(),
		Operator(nullptr),
		OutputSprite(nullptr),
		Enabled(true),
		Initialized(false)
	{
		SME_ASSERT(Singleton == nullptr);
		Singleton = this;

		SME_ASSERT(Operator);

		Initialized = true;
		this->Operator = Operator;
		Initialized = CreateD3D();
	}

	RenderWindowPainter::~RenderWindowPainter()
	{
		ReleaseD3D();
		delete Operator;

		for (auto Itr : RegisteredChannels)
			Itr->ReleaseD3D();

		Singleton = nullptr;
	}

	bool RenderWindowPainter::Initialize(RenderWindowPainterOperator* Operator)
	{
		if (Singleton)
			return false;

		RenderWindowPainter* Buffer = new RenderWindowPainter(Operator);
		return Buffer->Initialized;
	}

	void RenderWindowPainter::Deinitialize()
	{
		SME_ASSERT(Singleton);
		delete Singleton;
	}

	bool RenderWindowPainter::CreateD3D(void)
	{
		SME_ASSERT(OutputSprite == nullptr);

		bool Result = true;
		HRESULT OpResult = D3DXCreateSprite(Operator->GetD3DDevice(), &OutputSprite);

		if (FAILED(OpResult))
		{
			BGSEECONSOLE_MESSAGE("Failed to create output sprite for RenderTextWindowPainter\n\tResult = %d; %08X", OpResult, OpResult);
			Result = false;
		}

		return Result;
	}

	void RenderWindowPainter::ReleaseD3D( void )
	{
		SAFERELEASE_D3D(OutputSprite);
	}

	RenderWindowPainter* RenderWindowPainter::Get(void)
	{
		return Singleton;
	}

	void RenderWindowPainter::Render()
	{
		if (Enabled == false || OutputSprite == nullptr)
			return;

		OutputSprite->Begin(D3DXSPRITE_ALPHABLEND|D3DXSPRITE_SORT_TEXTURE);

		for (auto Itr : RegisteredChannels)
			Itr->Render(OutputSprite);

		OutputSprite->End();
	}

	bool RenderWindowPainter::HandleReset(bool Release, bool Renew)
	{
		SME_ASSERT(Release != Renew);

		bool Result = true;

		if (Release)
			ReleaseD3D();
		else
			Result = CreateD3D();

		for (auto Itr : RegisteredChannels)
		{
			if (Release)
				Itr->ReleaseD3D();
			else if (Itr->CreateD3D(Operator->GetD3DDevice(), Operator->GetD3DWindow()) == false)
				Result = false;
		}

		return Result;
	}

	bool RenderWindowPainter::RegisterRenderChannel( RenderChannelBase* Channel )
	{
		SME_ASSERT(Channel);
		for (auto Itr : RegisteredChannels)
		{
			if (Itr == Channel)
				return false;
		}

		RegisteredChannels.push_back(Channel);
		Channel->CreateD3D(Operator->GetD3DDevice(), Operator->GetD3DWindow());
		return true;
	}

	void RenderWindowPainter::UnregisterRenderChannel( RenderChannelBase* Channel )
	{
		SME_ASSERT(Channel);

		for (RenderChannelArrayT::const_iterator Itr = RegisteredChannels.begin(); Itr != RegisteredChannels.end(); Itr++)
		{
			if (*Itr == Channel)
			{
				RegisteredChannels.erase(Itr);
				Channel->ReleaseD3D();
				return;
			}
		}
	}

	void RenderWindowPainter::SetEnabled( bool State )
	{
		Enabled = State;
	}

	bool RenderWindowPainter::GetEnabled( void ) const
	{
		return Enabled;
	}

	void RenderWindowPainter::Redraw(void) const
	{
		Operator->RedrawRenderWindow();
	}
}