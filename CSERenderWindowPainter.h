#pragma once
#include <BGSEERenderWindowPainter.h>
#include <BGSEERenderWindowFlyCamera.h>

namespace ConstructionSetExtender
{
	namespace RenderWindowPainter
	{
		extern BGSEditorExtender::BGSEEStaticRenderChannel*						RenderChannelSelectionStats;
		extern BGSEditorExtender::BGSEEDynamicRenderChannel*					RenderChannelNotifications;

		class CSERAMUsageRenderChannel : public BGSEditorExtender::BGSEEStaticRenderChannel
		{
		protected:
			static CSERAMUsageRenderChannel*		Singleton;

			static VOID CALLBACK					TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
			static bool								RenderChannelCallback(std::string& RenderedText);

			DWORD									TimerID;
			UInt32									RAMCounter;

			CSERAMUsageRenderChannel(INT FontHeight,
				INT FontWidth,
				UINT FontWeight,
				const char* FontFace,
				D3DCOLOR Color,
				RECT* DrawArea,
				DWORD DrawFormat,
				UInt32 DrawAreaFlags);
		public:
			virtual ~CSERAMUsageRenderChannel();

			static CSERAMUsageRenderChannel*		GetSingleton();
		};

		class CSEMouseRefRenderChannel : public BGSEditorExtender::BGSEEStaticRenderChannel
		{
		protected:
			static CSEMouseRefRenderChannel*		Singleton;

			static bool								RenderChannelCallback(std::string& RenderedText);

			virtual void							Render(void* Parameter, LPD3DXSPRITE RenderToSprite);

			CSEMouseRefRenderChannel(INT FontHeight,
				INT FontWidth,
				UINT FontWeight,
				const char* FontFace,
				D3DCOLOR Color,
				DWORD DrawFormat);
		public:
			virtual ~CSEMouseRefRenderChannel();

			static CSEMouseRefRenderChannel*		GetSingleton();
		};

		void																	Initialize(void);
	}
};