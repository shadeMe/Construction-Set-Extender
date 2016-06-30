#pragma once
#include "IMGUI\imgui.h"

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowOSD;

		class ImGuiDX9
		{
			LPDIRECT3DVERTEXBUFFER9			VertexBuffer;
			LPDIRECT3DINDEXBUFFER9			IndexBuffer;

			int								VertexBufferSize;
			int								IndexBufferSize;

			LPDIRECT3DTEXTURE9				FontTexture;

			INT64							Time;
			INT64							TicksPerSecond;

			HWND							RenderWindowHandle;
			IDirect3DDevice9*				D3DDevice;
			bool							Initialized;

			static void						RenderDrawLists(ImDrawData* draw_data);

			bool							CreateFontsTexture();
			void							Shutdown();
		public:
			ImGuiDX9();
			~ImGuiDX9();

			bool			Initialize(HWND RenderWindow, IDirect3DDevice9* Device);
			void			NewFrame();

			void			InvalidateDeviceObjects();
			bool			CreateDeviceObjects();

			bool			UpdateInputState(HWND, UINT msg, WPARAM wParam, LPARAM lParam);		// returns true if the message was processed
			bool			NeedsInput() const;		// returns true if the GUI needs mouse/keyboard input
			bool			IsInitialized() const;
		};

		// queues ImGui drawcalls
		class IRenderWindowOSDLayer
		{
		public:
			virtual ~IRenderWindowOSDLayer() = 0
			{
				;//
			}

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI) = 0;
			virtual bool					NeedsBackgroundUpdate() = 0;			// returns true if the layer needs to be rendered when the render window doesn't have input focus
		};

		class RenderWindowOSD
		{
			class DialogExtraData : public bgsee::WindowExtraData
			{
			public:
				RenderWindowOSD*		Parent;

				DialogExtraData(RenderWindowOSD* OSD);
				virtual ~DialogExtraData();

				enum { kTypeID = 'XOSD' };
			};

			struct GUIState
			{
				bool					MouseInClientArea;					// set to true when the mouse is inside the window

				GUIState();
			};

			static LRESULT CALLBACK		OSDSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);

			typedef std::vector<IRenderWindowOSDLayer*>			LayerArrayT;

			ImGuiDX9*		Pipeline;
			GUIState		State;
			LayerArrayT		AttachedLayers;
			bool			Initialized;

			void			RenderSelectionControls();

			void			GUILogic();
			bool			NeedsBackgroundUpdate() const;
		public:
			RenderWindowOSD();
			~RenderWindowOSD();

			bool			Initialize();
			void			Deinitialize();

			void			Render();
			void			AttachLayer(IRenderWindowOSDLayer* Layer);
			void			DetachLayer(IRenderWindowOSDLayer* Layer);

			void			HandleD3DRelease();
			void			HandleD3DRenew();

			bool			NeedsInput() const;			// returns true if the OSD requires mouse or keyboard input
		};


		class DefaultOverlayOSDLayer : public IRenderWindowOSDLayer
		{
		public:
			virtual ~DefaultOverlayOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			static DefaultOverlayOSDLayer		Instance;
		};

		class MouseOverTooltipOSDLayer : public IRenderWindowOSDLayer
		{
		public:
			virtual ~MouseOverTooltipOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			static MouseOverTooltipOSDLayer		Instance;
		};


		class NotificationOSDLayer : public IRenderWindowOSDLayer
		{
			struct Notification
			{
				static const int		kNotificationDisplayTime = 2.5 * 1000;		// in ms

				std::string		Message;
				ULONGLONG		StartTickCount;

				Notification(std::string Message);

				bool			HasElapsed();
			};

			typedef std::queue<Notification>			NotificationQueueT;

			NotificationQueueT		Notifications;

			bool					HasNotifications();
			const Notification&		GetNextNotification() const;
		public:
			NotificationOSDLayer();
			virtual ~NotificationOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			void							ShowNotification(const char* Format, ...);

			static NotificationOSDLayer		Instance;
		};

	}
}