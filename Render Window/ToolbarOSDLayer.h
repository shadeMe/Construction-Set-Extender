#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class ToolbarOSDLayer : public IRenderWindowOSDLayer
		{
			static constexpr float			kPopupTimeout = 0.2;		// in seconds
			enum : int
			{
				kPopup__NONE = -1,

				kPopup_SnapControls,
				kPopup_CameraMult,
				kPopup_ReferenceMult,

				kPopup__MAX
			};

			int				ActivePopupID;
			float			ActivePopupTimeout;
			bool			ActivePopupClosing;
			bool			PopupButtonHoverState[kPopup__MAX];			// true if the popup's button is being hovered, false otherwise

			void			RenderPopup(int PopupID, ImGuiDX9* GUI);
			void			CheckPopupButtonHoverChange(int PopupID, bool& OutHovering, bool& OutBeginHover, bool& OutEndHover);
			void			TickActivePopup();

			bool							BottomExpanded;

			void							RenderBottomToolbar(ImGuiDX9* GUI);
		public:
			ToolbarOSDLayer();
			virtual ~ToolbarOSDLayer();

			virtual void					Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool					NeedsBackgroundUpdate();

			static ToolbarOSDLayer			Instance;
		};

	}
}