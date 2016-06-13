#pragma once
#include <bgsee\RenderWindowPainter.h>

namespace cse
{
	namespace renderWindow
	{
		extern bgsee::DynamicRenderChannel*		RenderChannelNotifications;

		class SelectionInfoRenderChannel : public bgsee::StaticRenderChannel
		{
		protected:
			virtual bool							DrawText(std::string& OutText);

			SelectionInfoRenderChannel(const char* FontFace, UInt32 FontSize, RECT* DrawRect);
		public:
			virtual ~SelectionInfoRenderChannel();

			static SelectionInfoRenderChannel*	GetInstance(const char* FontFace = NULL, UInt32 FontSize = NULL);
		};

		class RAMUsageRenderChannel : public bgsee::StaticRenderChannel
		{
		protected:
			static VOID CALLBACK					TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			DWORD									TimerID;
			UInt32									RAMCounter;

			virtual bool							DrawText(std::string& OutText);

			RAMUsageRenderChannel(const char* FontFace, UInt32 FontSize, RECT* DrawRect);
		public:
			virtual ~RAMUsageRenderChannel();

			static RAMUsageRenderChannel*		GetInstance(const char* FontFace = NULL, UInt32 FontSize = NULL);
		};

		class MouseRefRenderChannel : public bgsee::StaticRenderChannel
		{
		protected:
			virtual bool							DrawText(std::string& OutText);

			MouseRefRenderChannel(const char* FontFace, UInt32 FontSize);
		public:
			virtual ~MouseRefRenderChannel();

			static MouseRefRenderChannel*	GetInstance(const char* FontFace = NULL, UInt32 FontSize = NULL);
		};

		class RenderWindowPainterOperator : public bgsee::RenderWindowPainterOperator
		{
		public:
			virtual ~RenderWindowPainterOperator();

			virtual LPDIRECT3DDEVICE9				GetD3DDevice(void);
			virtual HWND							GetD3DWindow(void);
			virtual void							RedrawRenderWindow(void);
		};
	}
};