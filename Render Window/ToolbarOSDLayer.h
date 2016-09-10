#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class ToolbarOSDLayer : public IRenderWindowOSDLayer
		{
			MouseOverPopupProvider					PopupProvider;
			MouseOverPopupProvider::PopupIDT		PopupSnapControls;
			MouseOverPopupProvider::PopupIDT		PopupMovementControls;
			MouseOverPopupProvider::PopupIDT		PopupVisibilityToggles;
			MouseOverPopupProvider::PopupIDT		PopupOSDLayerToggles;

			ImGuiTextFilter							RefFilter;
			FormIDArrayT							FilterRefs;
			FormIDArrayT::const_iterator			PreviousFilterRef;
			bool									SetRefFilterFocus;

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