#pragma once

// BGSEERenderWindowPainter - Render window text painter

namespace BGSEditorExtender
{
	class BGSEERenderWindowPainter;

	class BGSEERenderChannelBase
	{
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
		bool							Valid;
		Parameters						InputParams;

		BGSEERenderChannelBase(INT FontHeight,
			INT FontWidth,
			UINT FontWeight,
			const char* FontFace,
			D3DCOLOR Color,
			RECT* DrawArea,
			DWORD DrawFormat,
			UInt32 DrawAreaFlags = 0);

		virtual void					Render(void* Parameter, LPD3DXSPRITE RenderToSprite) = 0;
		bool							CreateD3D();
		void							ReleaseD3D();
		bool							GetIsValid() const;

		friend class					BGSEERenderWindowPainter;
	public:
		enum
		{
			kDrawAreaFlags_RightAligned		= 1 << 0,
			kDrawAreaFlags_BottomAligned	= 1 << 1,
		};

		virtual ~BGSEERenderChannelBase();
	};

	class BGSEEStaticRenderChannel : public BGSEERenderChannelBase
	{
	public:
		typedef bool					(* RenderHandler)(std::string& RenderedText);		// return false to skip rendering
	protected:
		std::string						RenderText;
		RenderHandler					RenderCallback;

		virtual void					Render(void* Parameter, LPD3DXSPRITE RenderToSprite);

		friend class					BGSEERenderWindowPainter;
	public:
		BGSEEStaticRenderChannel(INT FontHeight,
			INT FontWidth,
			UINT FontWeight,
			const char* FontFace,
			D3DCOLOR Color,
			RECT* DrawArea,
			DWORD DrawFormat,
			UInt32 DrawAreaFlags = 0,
			RenderHandler RenderCallback = NULL);

		virtual ~BGSEEStaticRenderChannel();
	};

	class BGSEEDynamicRenderChannel : public BGSEERenderChannelBase
	{
	protected:
		struct RenderTask
		{
			std::string					Text;
			float						RemainingTime;

			RenderTask(const char* Text, float SecondsToDisplay);
		};

		std::queue<RenderTask*>			TaskQueue;

		virtual void					Render(void* Parameter, LPD3DXSPRITE RenderToSprite);		// parameter's long double* - TimePassedSinceLastUpdate

		friend class					BGSEERenderWindowPainter;
	public:
		BGSEEDynamicRenderChannel(INT FontHeight,
			INT FontWidth,
			UINT FontWeight,
			const char* FontFace,
			D3DCOLOR Color,
			RECT* DrawArea,
			DWORD DrawFormat,
			UInt32 DrawAreaFlags = 0);

		virtual ~BGSEEDynamicRenderChannel();

		bool							Queue(float SecondsToDisplay, const char* Format, ...);
		UInt32							GetQueueSize() const;
	};

	class BGSEERenderWindowPainter
	{
		static BGSEERenderWindowPainter*		Singleton;
	protected:
		BGSEERenderWindowPainter();

		typedef std::list<BGSEERenderChannelBase*>	RenderChannelListT;

		RenderChannelListT						RegisteredChannels;
		SME::MiscGunk::ElapsedTimeCounter		TimeCounter;
		LPD3DXSPRITE							OutputSprite;
		D3D9DeviceGetter*						D3DDevice;
		HWNDGetter*								D3DWindow;
		bool									Enabled;
		bool									Initialized;

		bool									LookupRenderChannel(BGSEERenderChannelBase* Channel, RenderChannelListT::iterator& Match);
		bool									CreateD3D(void);
		void									ReleaseD3D(void);
	public:
		~BGSEERenderWindowPainter();

		enum
		{
			kDeviceReset_Release				= 0,
			kDeviceReset_Renew,
		};

		static BGSEERenderWindowPainter*		GetSingleton(void);

		bool									Initialize(HWND RenderWindowHandle, LPDIRECT3DDEVICE9 RendererD3DDevice);

		void									Render();
		bool									HandleD3DDeviceReset(UInt8 Operation);

		bool									RegisterRenderChannel(BGSEERenderChannelBase* Channel);		// painter takes ownership of the pointer, returns true if successful
		void									UnregisterRenderChannel(BGSEERenderChannelBase* Channel);	// releases the registered channel

		void									SetEnabled(bool State);
		bool									GetEnabled(void) const;

		bool									GetHasActiveTasks(void) const;
		HWND									GetD3DWindow(void) const;
		LPDIRECT3DDEVICE9						GetD3DDevice(void) const;
	};

#define BGSEERWPAINTER							BGSEditorExtender::BGSEERenderWindowPainter::GetSingleton()
}