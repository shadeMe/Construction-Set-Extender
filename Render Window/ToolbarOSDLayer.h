#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class ToolbarOSDLayer : public IRenderWindowOSDLayer
		{
			static constexpr float			kTimeoutPopup = 0.1;		// in seconds
			enum : int
			{
				kPopup__NONE = -1,

				kPopup_SnapControls,
				kPopup_MovementControls,
				kPopup_VisibilityToggles,

				kPopup__MAX
			};


			int						ActivePopup;
			float					ActivePopupTimeout;
			bool					CloseActivePopup;
			bool					PreventActivePopupTicking;
			bool					PopupButtonHoverState[kPopup__MAX];			// true if the popup's button is being hovered, false otherwise
			StateData				PopupStateData[kPopup__MAX];
			ImGuiTextFilter			RefFilter;
			FormIDArrayT			FilterRefs;
			FormIDArrayT::const_iterator	PreviousFilterRef;
			bool					SetRefFilterFocus;

			void					CheckPopupButtonHoverChange(int PopupID, ImGuiDX9* GUI, void* ParentToolbarWindow,
														bool& OutHovering, bool& OutBeginHover, bool& OutEndHover);
			void					RenderPopup(int PopupID, ImGuiDX9* GUI, void* ParentToolbarWindow, const ImVec2& ButtonSize, const ImVec4* ButtonColor = nullptr);
			void					TickActivePopup();

			int						RefFilterCompletionCallback(ImGuiTextEditCallbackData* Data);
			void					HandleRefFilterChange();

			void					RenderMainToolbar(ImGuiDX9* GUI);
			void					RenderTopToolbar(ImGuiDX9* GUI);
		public:
			ToolbarOSDLayer();
			virtual ~ToolbarOSDLayer();

			virtual void			Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool			NeedsBackgroundUpdate();

			void					FocusOnRefFilter();		// moves the keyboard focus to the reference filter in the next frame

			static ToolbarOSDLayer			Instance;
		};

	}
}