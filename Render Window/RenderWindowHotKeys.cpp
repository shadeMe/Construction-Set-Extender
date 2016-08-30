#include "RenderWindowHotKeys.h"

namespace cse
{
	namespace renderWindow
	{

		void RenderWindowHotKeyManager::SetKeyStateOverride(UInt8 Type, bool NewState)
		{
			SME_ASSERT(Type < kKey__MAX);

			KeyStateOverride& Override = OverriddenKeyStates[Type];
			SME_ASSERT(Override.Active == false);

			Override.Active = true;
			Override.NewState = NewState;
			// also store the current state for restoring it later (ALT key state isn't stored by the editor)
			switch (Type)
			{
			case kKey_Shift:
				Override.OldState = *TESRenderWindow::KeyState_Shift;
				*TESRenderWindow::KeyState_Shift = NewState;
				break;
			case kKey_Control:
				Override.OldState = *TESRenderWindow::KeyState_Control;
				*TESRenderWindow::KeyState_Control = NewState;
				break;
			default:
				Override.OldState = false;
			}
		}

		void RenderWindowHotKeyManager::ResetKeyStateOverride(UInt8 Type)
		{
			SME_ASSERT(Type < kKey__MAX);

			KeyStateOverride& Override = OverriddenKeyStates[Type];
			SME_ASSERT(Override.Active);

			Override.Active = false;
			// restore previous state
			switch (Type)
			{
			case kKey_Shift:
				*TESRenderWindow::KeyState_Shift = Override.OldState;
				break;
			case kKey_Control:
				*TESRenderWindow::KeyState_Control = Override.OldState;
				break;
			}
		}

		RenderWindowHotKeyManager::RenderWindowHotKeyManager() :
			OverriddenKeyStates()
		{
			;//
		}

		RenderWindowHotKeyManager::~RenderWindowHotKeyManager()
		{
			;//
		}

		void RenderWindowHotKeyManager::SendDefaultHotKey(UInt8 VirtualKey, bool Shift, bool Control)
		{
			if (Shift)
				SetKeyStateOverride(kKey_Shift, Shift);

			if (Control)
				SetKeyStateOverride(kKey_Control, Control);

			BGSEEUI->GetSubclasser()->TunnelDialogMessage(*TESRenderWindow::WindowHandle,
														  WM_KEYDOWN,
														  VirtualKey,
														  NULL);

			if (Shift)
				ResetKeyStateOverride(kKey_Shift);

			if (Control)
				ResetKeyStateOverride(kKey_Control);
		}

	}
}