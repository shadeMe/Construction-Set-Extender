#pragma once

// RenderWindowPainter - Render window text painter

namespace bgsee
{
	class RenderWindowPainter;
	class RenderWindowPainterOperator;
	class DynamicRenderChannel;

	class RenderChannelBase
	{
		friend class					RenderWindowPainter;

		virtual void					Render(LPD3DXSPRITE RenderToSprite) = 0;
	protected:
		struct Parameters
		{
			INT								FontHeight;
			INT								FontWidth;
			UINT							FontWeight;
			char							FontFace[0x100];
			D3DCOLOR						Color;
			RECT							DrawArea;
			UInt32							DrawAreaFlags;
			DWORD							DrawFormat;

			Parameters(INT FontHeight,
				INT FontWidth,
				UINT FontWeight,
				const char* FontFace,
				D3DCOLOR Color,
				RECT* DrawArea,
				DWORD DrawFormat,
				UInt32 DrawAreaFlags);
		};

		LPD3DXFONT						Font;
		RECT							RenderArea;
		Parameters						InputParams;
		bool							Valid;

		bool							CreateD3D(LPDIRECT3DDEVICE9 Device, HWND Window);
		void							ReleaseD3D();
		bool							GetIsValid() const;
	public:
		enum
		{
			kDrawAreaFlags_Default = 0,
			kDrawAreaFlags_RightAligned = 1 << 0,
			kDrawAreaFlags_BottomAligned = 1 << 1,
		};

		RenderChannelBase(INT FontHeight,
							   INT FontWidth,
							   UINT FontWeight,
							   const char* FontFace,
							   D3DCOLOR Color,
							   RECT* DrawArea,
							   DWORD DrawFormat,
							   UInt32 DrawAreaFlags = kDrawAreaFlags_Default);

		virtual ~RenderChannelBase() = 0;
	};

	class StaticRenderChannel : public RenderChannelBase
	{
		friend class					RenderWindowPainter;

		std::string						LastRenderedText;

		virtual void					Render(LPD3DXSPRITE RenderToSprite);
	protected:
		virtual bool					DrawText(std::string& OutText) = 0;				// return false to skip rendering
	public:
		StaticRenderChannel(INT FontHeight,
			INT FontWidth,
			UINT FontWeight,
			const char* FontFace,
			D3DCOLOR Color,
			RECT* DrawArea,
			DWORD DrawFormat,
			UInt32 DrawAreaFlags = kDrawAreaFlags_Default);

		virtual ~StaticRenderChannel();
	};

	class DynamicRenderChannelScheduler
	{
		static const UInt32				kTimerPeriod = 50;		// in ms

		static VOID CALLBACK			UpdateTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

		typedef std::vector<DynamicRenderChannel*>		UpdateCallbackListT;

		UpdateCallbackListT				TaskRegistry;
		UINT_PTR						TimerID;

		void							UpdateTasks(DWORD CurrentTickCount);
	public:
		DynamicRenderChannelScheduler();
		~DynamicRenderChannelScheduler();

		bool							Register(DynamicRenderChannel* Channel);
		void							Unregister(DynamicRenderChannel* Channel);

		static DynamicRenderChannelScheduler*				GetSingleton(void);
	};

	class DynamicRenderChannel : public RenderChannelBase
	{
		friend class					RenderWindowPainter;
		friend class					DynamicRenderChannelScheduler;

		virtual void					Render(LPD3DXSPRITE RenderToSprite);

		struct Task
		{
			std::string					Drawable;
			DWORD						Duration;			// in ms
			DWORD						StartTickCount;

			Task(const char* Text, DWORD Duration);
		};

		typedef std::queue<Task>		TaskQueueT;

		TaskQueueT						QueuedItems;

		void							UpdateTasks(DWORD CurrentTickCount);
	public:
		DynamicRenderChannel(INT FontHeight,
			INT FontWidth,
			UINT FontWeight,
			const char* FontFace,
			D3DCOLOR Color,
			RECT* DrawArea,
			DWORD DrawFormat,
			UInt32 DrawAreaFlags = kDrawAreaFlags_Default);

		virtual ~DynamicRenderChannel();

		bool							Queue(float DurationInSeconds, const char* Format, ...);		// duration must be b'ween 1 and 10 seconds
	};

	class RenderWindowPainterOperator
	{
	public:
		virtual ~RenderWindowPainterOperator() = 0
		{
			;//
		}

		virtual LPDIRECT3DDEVICE9				GetD3DDevice(void) = 0;
		virtual HWND							GetD3DWindow(void) = 0;
		virtual void							RedrawRenderWindow(void) = 0;
	};

	class RenderWindowPainter
	{
		static RenderWindowPainter*		Singleton;

		RenderWindowPainter(RenderWindowPainterOperator* Operator);
		~RenderWindowPainter();

		typedef std::vector<RenderChannelBase*>	RenderChannelArrayT;

		RenderChannelArrayT						RegisteredChannels;
		RenderWindowPainterOperator*			Operator;
		LPD3DXSPRITE							OutputSprite;
		bool									Enabled;
		bool									Initialized;

		bool									CreateD3D(void);
		void									ReleaseD3D(void);
	public:
		static RenderWindowPainter*				Get(void);
		static bool								Initialize(RenderWindowPainterOperator* Operator);		// takes ownership of pointer
		static void								Deinitialize();

		bool									RegisterRenderChannel(RenderChannelBase* Channel);		// caller retains ownership of pointer; returns true if successful
		void									UnregisterRenderChannel(RenderChannelBase* Channel);

		void									Render(void);
		bool									HandleReset(bool Release, bool Renew);
		void									Redraw(void) const;											// force a redraw of current render window scene

		void									SetEnabled(bool State);
		bool									GetEnabled(void) const;
	};

#define BGSEERWPAINTER							bgsee::RenderWindowPainter::Get()
}