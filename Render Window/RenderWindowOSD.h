#pragma once
#include "IMGUI\imgui.h"

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowOSD;
		class IRenderWindowOSDLayer;

		class ImGuiDX9
		{
			typedef std::vector<ImGuiID>		ImGuiWidgetIDArrayT;

			LPDIRECT3DVERTEXBUFFER9			VertexBuffer;
			LPDIRECT3DINDEXBUFFER9			IndexBuffer;

			int								VertexBufferSize;
			int								IndexBufferSize;

			LPDIRECT3DTEXTURE9				FontTexture;

			INT64							Time;
			INT64							TicksPerSecond;

			HWND							RenderWindowHandle;
			IDirect3DDevice9*				D3DDevice;
			bool							MouseDoubleClicked[2];			// for the left and right mouse buttons
			bool							ConsumeNextMouseRButtonDown;

			// when the active widget is whitelisted, input events are allowed to be handled by the org wnd proc
			ImGuiWidgetIDArrayT				PassthroughWhitelistMouseEvents;

			bool							Initialized;

			static void						RenderDrawLists(ImDrawData* draw_data);

			bool							CreateFontsTexture();
			void							Shutdown();

			bool							IsActiveItemInWhitelist(const ImGuiWidgetIDArrayT& Whitelist) const;
			bool							HasActiveItem() const;
		public:
			ImGuiDX9();
			~ImGuiDX9();

			bool			Initialize(HWND RenderWindow, IDirect3DDevice9* Device);
			void			NewFrame();
			void			Render();

			void			InvalidateDeviceObjects();
			bool			CreateDeviceObjects();

			void			ResetInputState(bool ConsumeNextRButtonDown);
			bool			UpdateInputState(HWND, UINT msg, WPARAM wParam, LPARAM lParam);		// returns true if the message was processed
			void			NeedsInput(bool& OutNeedsMouse, bool& OutNeedsKeyboard, bool& OutNeedsTextInput) const;
			void			WhitelistItemForMouseEvents();										// adds the last item to the mouse event whitelist
			bool			CanAllowInputEventPassthrough(UINT msg, WPARAM wParam, LPARAM lParam, bool& OutNeedsMouse, bool& OutNeedsKeyboard) const;		// returns false if the message/input event needs to be handled exclusively by the GUI

			bool			IsInitialized() const;
			bool			IsDraggingWindow() const;
			bool			IsPopupHovered() const;												// must be called inside a BeginPopup block
			bool			IsHoveringWindow() const;
			bool			IsChildWindowHovering(void* RootWindow) const;						// pass output of GetCurrent/HoveredWindow()
			bool			HasRootWindow(void* ChildWindow, void* RootWindow) const;			// returns true if the root window is found in the child's parent hierarchy

			void*			GetLastItemID() const;
			void*			GetMouseHoverItemID() const;

			void*			GetCurrentWindow() const;
			void*			GetHoveredWindow() const;
			void*			GetCurrentPopup() const;		// returns the topmost/open/persistent popup
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
				bool		RedrawSingleFrame;
				bool		MouseInClientArea;				// true when the mouse is inside the window
				bool		ConsumeMouseInputEvents;		// true when the OSD wnd proc needs exclusive access to the mouse
				bool		ConsumeKeyboardInputEvents;		// same as above but for the keyboard
				bool		MouseHoveringOSD;				// true if the mouse is hovering over an OSD element

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

			void			Draw();			// draws the attached layers
			void			Render();		// rendered the queued draw calls from the last Draw() call

			void			AttachLayer(IRenderWindowOSDLayer* Layer);
			void			DetachLayer(IRenderWindowOSDLayer* Layer);

			void			HandleD3DRelease();
			void			HandleD3DRenew();

			bool			NeedsInput(UINT uMsg) const;			// returns true if the OSD requires mouse or keyboard input
			bool			NeedsInput() const;
		};

		// queues ImGui drawcalls
		class IRenderWindowOSDLayer
		{
			const INISetting*		Toggle;
		public:
			struct Helpers
			{
				static std::string			GetRefEditorID(TESObjectREFR* Ref);
				static bool					ResolveReference(UInt32 FormID, TESObjectREFR*& OutRef);		// returns false if the formID didn't resolve to a valid ref, true otherwise
			};

			IRenderWindowOSDLayer(const INISetting* Toggle = nullptr);
			virtual ~IRenderWindowOSDLayer() = 0
			{
				;//
			}

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI) = 0;
			virtual bool					NeedsBackgroundUpdate() = 0;			// returns true if the layer needs to be rendered when the render window doesn't have input focus
			virtual bool					IsEnabled() const;
		};

		// tracked over multiple frames
		struct OSDLayerStateData
		{
			struct TextInputData
			{
				bool	Active;
				bool	GotFocus;
				bool	LostFocus;

				void	Update(ImGuiDX9* GUI);
			};

			struct DragInputData
			{
				bool	Active;
				bool	DragBegin;
				bool	DragEnd;

				void	Update(ImGuiDX9* GUI);
			};

			TextInputData		TextInput;
			DragInputData		DragInput;

			OSDLayerStateData();

			void				Update(ImGuiDX9* GUI);							// must be called inside a ImGui::Begin() block
		};

		class MouseOverPopupProvider
		{
		public:
			typedef int							PopupIDT;
			typedef std::function<void()>		RenderDelegateT;

			enum
			{
				kPosition_Default = 0,		// at mouse coords
				kPosition_Absolute,			// in screen coords
				kPosition_Relative,			// relative to the mouse position
			};
		private:
			static constexpr float				kTimeout = 0.25;		// in seconds
			static const PopupIDT				kInvalidID = -1;

			struct PopupData
			{
				std::string			PopupName;
				bool				ButtonHoverState;	// true if the popup's buttons is being hovered
				OSDLayerStateData	PopupState;
				RenderDelegateT		DrawPopup;			// draws the popup contents
				RenderDelegateT		DrawButton;			// draws (just) the button
				ImVec2				Position;			// position of the popup on appearance
				UInt8				PositionType;

				PopupData(const char* Name,
						  RenderDelegateT DrawButton, RenderDelegateT DrawPopup,
						  UInt8 PositionType, ImVec2& Pos);

				void				CheckButtonHoverChange(ImGuiDX9* GUI, void* ParentWindow,
														   bool& OutHovering, bool& OutBeginHover, bool& OutEndHover);
			};

			typedef std::vector<PopupData>		PopupDataArrayT;

			PopupDataArrayT			RegisteredPopups;
			PopupIDT				ActivePopup;
			float					ActivePopupTimeout;
			bool					CloseActivePopup;
			bool					PreventActivePopupTicking;
		public:
			MouseOverPopupProvider();

			PopupIDT				RegisterPopup(const char* Name,
												  RenderDelegateT DrawButton, RenderDelegateT DrawPopup,
												  UInt8 PositionType = kPosition_Default, ImVec2& Pos = ImVec2(0, 0));
			void					Draw(PopupIDT ID, ImGuiDX9* GUI, void* ParentWindow);
			void					Update();		// called at the start of the layer's Draw method
		};

		class NotificationOSDLayer : public IRenderWindowOSDLayer
		{
			struct Notification
			{
				static const int		kNotificationDisplayTime = 1.75 * 1000;

				std::string		Message;
				ULONGLONG		StartTickCount;
				int				Duration;				// in ms

				Notification(std::string Message, int Duration = kNotificationDisplayTime);

				bool			HasElapsed();
				ULONGLONG		GetRemainingTicks() const;
			};

			typedef std::queue<Notification>			NotificationQueueT;

			NotificationQueueT		Queue;

			bool					Tick();		// returns true if there are pending notifications
			const Notification&		GetNextNotification() const;
		public:
			NotificationOSDLayer();
			virtual ~NotificationOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			void							ShowNotification(const char* Format, ...);
			void							ClearNotificationQueue();		// removes all but the currently showing notification

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

		class ModalWindowProviderOSDLayer : public IRenderWindowOSDLayer
		{
		public:
			typedef std::function<bool(RenderWindowOSD*, ImGuiDX9*, void*)>		ModalRenderDelegateT;			// draws the modal's contents and returns true to close it, false otherwise
		private:
			struct ModalData
			{
				std::string						WindowName;		// salted with a random integer to prevent collisions
				ModalRenderDelegateT			Delegate;
				void*							UserData;
				ImGuiWindowFlags				Flags;
				ImVec2							WindowSize;
				ImGuiSetCond					SizeSetCondition;

				bool							Open;
				bool							HasCustomSize;

				ModalData(const char* Name, ModalRenderDelegateT Delegate, void* UserData,
						  ImGuiWindowFlags Flags, const ImVec2& Size, ImGuiSetCond SizeCond);
			};

			typedef std::stack<ModalData>		ModalDataStackT;

			ModalDataStackT			OpenModals;
		public:
			ModalWindowProviderOSDLayer();
			virtual ~ModalWindowProviderOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			void							ShowModal(const char* Name, ModalRenderDelegateT Delegate, void* UserData,
													  ImGuiWindowFlags Flags, const ImVec2& Size = ImVec2(0, 0), ImGuiSetCond SizeCond = NULL);
			bool							HasOpenModals() const;

			static ModalWindowProviderOSDLayer		Instance;
		};
	}
}