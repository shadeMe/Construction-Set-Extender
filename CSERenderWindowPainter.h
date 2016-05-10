#pragma once
#include <BGSEERenderWindowPainter.h>
#include <BGSEERenderWindowFlyCamera.h>

namespace ConstructionSetExtender
{
	namespace RenderWindowPainter
	{
		extern bgsee::BGSEEDynamicRenderChannel*	RenderChannelNotifications;

		class CSESelectionInfoRenderChannel : public bgsee::BGSEEStaticRenderChannel
		{
		protected:
			virtual bool							DrawText(std::string& OutText);

			CSESelectionInfoRenderChannel(const char* FontFace, UInt32 FontSize, RECT* DrawRect);
		public:
			virtual ~CSESelectionInfoRenderChannel();

			static CSESelectionInfoRenderChannel*	GetInstance(const char* FontFace = NULL, UInt32 FontSize = NULL);
		};

		class CSERAMUsageRenderChannel : public bgsee::BGSEEStaticRenderChannel
		{
		protected:
			static VOID CALLBACK					TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			DWORD									TimerID;
			UInt32									RAMCounter;

			virtual bool							DrawText(std::string& OutText);

			CSERAMUsageRenderChannel(const char* FontFace, UInt32 FontSize, RECT* DrawRect);
		public:
			virtual ~CSERAMUsageRenderChannel();

			static CSERAMUsageRenderChannel*		GetInstance(const char* FontFace = NULL, UInt32 FontSize = NULL);
		};

		class CSEMouseRefRenderChannel : public bgsee::BGSEEStaticRenderChannel
		{
		protected:
			virtual bool							DrawText(std::string& OutText);

			CSEMouseRefRenderChannel(const char* FontFace, UInt32 FontSize);
		public:
			virtual ~CSEMouseRefRenderChannel();

			static CSEMouseRefRenderChannel*	GetInstance(const char* FontFace = NULL, UInt32 FontSize = NULL);
		};

		class CSERenderWindowPainterOperator : public bgsee::BGSEERenderWindowPainterOperator
		{
		public:
			virtual ~CSERenderWindowPainterOperator();

			virtual LPDIRECT3DDEVICE9				GetD3DDevice(void);
			virtual HWND							GetD3DWindow(void);
			virtual void							RedrawRenderWindow(void);
		};

		void Initialize(void);
		void Deinitialize(void);
	}
};