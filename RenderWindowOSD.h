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
			bool							MouseDoubleClicked[2];		// for the left and right mouse buttons
			bool							Initialized;

			static void						RenderDrawLists(ImDrawData* draw_data);

			bool							CreateFontsTexture();
			void							Shutdown();
		public:
			ImGuiDX9();
			~ImGuiDX9();

			bool			Initialize(HWND RenderWindow, IDirect3DDevice9* Device);
			void			NewFrame();
			void			Render();

			void			InvalidateDeviceObjects();
			bool			CreateDeviceObjects();

			bool			UpdateInputState(HWND, UINT msg, WPARAM wParam, LPARAM lParam);		// returns true if the message was processed
			bool			NeedsInput() const;		// returns true if the GUI needs mouse/keyboard input
			bool			IsInitialized() const;
			bool			IsDraggingWindow() const;
		};

		// queues ImGui drawcalls
		class IRenderWindowOSDLayer
		{
			const INISetting*				Toggle;
			UInt32							Priority;
		protected:
			enum
			{
				kPriority_Notifications = 1001,
				kPriority_MouseTooltip = 1000,
				kPriority_Toolbar = 999,

				kPriority_SelectionControls = 998,
				kPriority_CellLists = 997,

				kPriority_Debug = 2,
				kPriority_DefaultOverlay = 1,
				kPriority_ModalProvider = 0,
			};
		public:
			IRenderWindowOSDLayer(INISetting& Toggle, UInt32 Priority);
			IRenderWindowOSDLayer(UInt32 Priority);
			virtual ~IRenderWindowOSDLayer() = 0
			{
				;//
			}

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI) = 0;
			virtual bool					NeedsBackgroundUpdate() = 0;			// returns true if the layer needs to be rendered when the render window doesn't have input focus
			virtual UInt32					GetPriority() const;					// layers with higher priority get rendered first
			virtual bool					IsEnabled() const;
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
			bool			RenderingLayers;

			void			RenderLayers();
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
			DefaultOverlayOSDLayer();
			virtual ~DefaultOverlayOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			static DefaultOverlayOSDLayer		Instance;
		};

		class MouseOverTooltipOSDLayer : public IRenderWindowOSDLayer
		{
		public:
			MouseOverTooltipOSDLayer();
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

		class DebugOSDLayer : public IRenderWindowOSDLayer
		{
		public:
			DebugOSDLayer();
			virtual ~DebugOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			static DebugOSDLayer			Instance;
		};

		class ToolbarOSDLayer : public IRenderWindowOSDLayer
		{
			bool							BottomExpanded;

			void							RenderBottomToolbar();
		public:
			ToolbarOSDLayer();
			virtual ~ToolbarOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			static ToolbarOSDLayer			Instance;
		};

		class SelectionControlsOSDLayer : public IRenderWindowOSDLayer
		{
			bool					TextInputActive;
			bool					DragActive;
			int						LocalTransformation;
			bool					AlignmentAxisX;
			bool					AlignmentAxisY;
			bool					AlignmentAxisZ;

			void					EditReference(TESObjectREFR* Ref);
			void					EditBaseForm(TESObjectREFR* Ref);
			void					CheckTextInputChange(ImGuiDX9* GUI, bool& OutGotFocus, bool& OutLostFocus);
			void					CheckDragChange(ImGuiDX9* GUI, bool& OutsDragBegin, bool& OutDragEnd);
			void					DrawDragTrail();

			void					MoveSelection(bool X, bool Y, bool Z);
			void					RotateSelection(bool Local, bool X, bool Y, bool Z);
			void					ScaleSelection(bool Local);
			void					AlignSelection(bool Position, bool Rotation);
		public:
			SelectionControlsOSDLayer();
			virtual ~SelectionControlsOSDLayer();

			virtual void			Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool			NeedsBackgroundUpdate();

			static SelectionControlsOSDLayer			Instance;
		};

		class ModalWindowProviderOSDLayer : public IRenderWindowOSDLayer
		{
		public:
			typedef std::function<bool(RenderWindowOSD*, ImGuiDX9*)>		ModalRenderDelegateT;			// draws the modal's contents and returns true to close it, false otherwise
		private:
			struct ModalData
			{
				std::string						WindowName;		// salted with random integer to prevent collisions
				ModalRenderDelegateT			Delegate;
				ImGuiWindowFlags_				Flags;
				bool							Open;

				ModalData(const char* Name, ModalRenderDelegateT Delegate, ImGuiWindowFlags_ Flags);
			};

			typedef std::stack<ModalData>		ModalDataStackT;

			ModalDataStackT			OpenModals;
		public:
			ModalWindowProviderOSDLayer();
			virtual ~ModalWindowProviderOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			void							ShowModal(const char* Name, ModalRenderDelegateT Delegate, ImGuiWindowFlags_ Flags);

			static ModalWindowProviderOSDLayer		Instance;
		};
	}
}