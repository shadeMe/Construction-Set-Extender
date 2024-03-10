#pragma once

#include "UICommon.h"

namespace bgsee
{
	class GenericModelessDialog
	{
		static LRESULT CALLBACK			DefaultDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	protected:
		typedef LRESULT (CALLBACK*		MessageCallback)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);

		struct DlgUserData
		{
			GenericModelessDialog*		Instance;
			LPARAM						ExtraData;					// upon init, holds the InitParam parameter (if any)
			bool						Initialized;				// set once the WM_INITDIALOG message is sent to the dlg
		};

		HWND							DialogHandle;
		HWND							ParentHandle;
		HMENU							ContextMenuHandle;			// all menu items must be a part of a sub-menu at index 0
		HMENU							ContextMenuParentHandle;	// handle of the base menu
		HINSTANCE						ResourceInstance;
		ResourceTemplateOrdinalT		DialogTemplateID;
		ResourceTemplateOrdinalT		DialogContextMenuID;
		MessageCallback					CallbackDlgProc;
		bool							Visible;
		bool							Topmost;
		float							AspectRatio;
		bool							LockAspectRatio;

		virtual void					INILoadUIState(INISetting* Top, INISetting* Left, INISetting* Right, INISetting* Bottom, INISetting* Visible);
		virtual void					INISaveUIState(INISetting* Top, INISetting* Left, INISetting* Right, INISetting* Bottom, INISetting* Visible);

		virtual bool					SetVisibility(bool State);
		virtual bool					SetTopmost(bool State);

		virtual void					Create(LPARAM InitParam, bool Hide, bool OverrideCreation = false);

		GenericModelessDialog();
	public:
		GenericModelessDialog(HWND Parent, HINSTANCE Resource, ResourceTemplateOrdinalT DialogTemplate, ResourceTemplateOrdinalT ContextMenuTemplate, MessageCallback CallbackProc, float AspectRatio = 0.0f);
		virtual ~GenericModelessDialog();

		virtual bool					ToggleVisibility(void);		// returns the new state
		virtual bool					ToggleTopmost(void);

		bool							IsVisible(void) const;
		bool							GetTopmost(void) const;
		bool							GetInitialized(void) const;
	};
}
