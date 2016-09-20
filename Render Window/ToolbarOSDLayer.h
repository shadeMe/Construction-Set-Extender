#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class ToolbarOSDLayer : public IRenderWindowOSDLayer
		{
			typedef std::vector<MouseOverPopupProvider::PopupIDT>		PopupIDArrayT;

			MouseOverPopupProvider					BottomToolbarPopupProvider;
			MouseOverPopupProvider::PopupIDT		PopupSnapControls;
			MouseOverPopupProvider::PopupIDT		PopupMovementControls;
			MouseOverPopupProvider::PopupIDT		PopupVisibilityToggles;

			MouseOverPopupProvider					TopToolbarPopupProvider;
			PopupIDArrayT							TopToolbarPopupIDs;

			ImGuiTextFilter							RefFilter;
			FormIDArrayT							FilterRefs;
			int										CurrentFilterRefIndex;
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
			void					RegisterTopToolbarButton(const char* PopupID,
															 MouseOverPopupProvider::RenderDelegateT DrawButton,
															 MouseOverPopupProvider::RenderDelegateT DrawPopup);

			static ToolbarOSDLayer			Instance;
		};

	}
}