#include "RenderWindowCommon.h"

namespace cse
{
	namespace renderWindow
	{
		ExecutionContext::ExecutionContext(UInt8 Context) :
			Context(Context)
		{
			SME_ASSERT(Context);
		}

		bool ExecutionContext::HasLandscapeEdit() const
		{
			return Context & kMode_LandscapeEdit;
		}

		bool ExecutionContext::HasPathGridEdit() const
		{
			return Context & kMode_PathGridEdit;
		}

		bool ExecutionContext::HasReferenceEdit() const
		{
			return Context & kMode_ReferenceEdit;
		}

		bool ExecutionContext::IsExecutable() const
		{
			bool Ref = *TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0;
			bool PathGrid = *TESRenderWindow::PathGridEditFlag;
			bool Landscape = *TESRenderWindow::LandscapeEditFlag;

			if (Ref && HasReferenceEdit())
				return true;
			else if (PathGrid && HasPathGridEdit())
				return true;
			else if (Landscape && HasLandscapeEdit())
				return true;
			else
				return false;
		}

		bool ExecutionContext::operator!=(const ExecutionContext& RHS)
		{
			return (*this == RHS) == false;
		}

		bool ExecutionContext::operator==(const ExecutionContext& RHS)
		{
			if (HasReferenceEdit() && RHS.HasReferenceEdit())
				return true;
			else if (HasPathGridEdit() && RHS.HasPathGridEdit())
				return true;
			else if (HasLandscapeEdit() && RHS.HasLandscapeEdit())
				return true;
			else
				return false;
		}

		IRenderWindowAction::IRenderWindowAction(std::string Name, std::string Desc, UInt8 Context) :
			Context(Context),
			Name(Name),
			Description(Desc)
		{
			;//
		}

		IRenderWindowAction::~IRenderWindowAction() = default;

		const char* IRenderWindowAction::GetName() const
		{
			return Name.c_str();
		}

		const char* IRenderWindowAction::GetDescription() const
		{
			return Description.c_str();
		}

		const ExecutionContext& IRenderWindowAction::GetExecutionContext() const
		{
			return Context;
		}


		namespace input
		{

			BuiltIn::ModifierOverride	BuiltIn::ModifierOverride::Instance;

			BuiltIn::ModifierOverride::ModifierOverride() :
				Data()
			{
				for (int i = kKey_CTRL; i < kKey__MAX; i++)
				{
					State& Current = Data[i];
					switch (i)
					{
					case kKey_CTRL:
						Current.BaseState = TESRenderWindow::KeyState_Control;
						break;
					case kKey_SHIFT:
						Current.BaseState = TESRenderWindow::KeyState_Shift;
						break;
					case kKey_ALT:
						Current.BaseState = nullptr;
						break;
					case kKey_SPACE:
						Current.BaseState = TESRenderWindow::KeyState_SpaceMMB;
						break;
					case kKey_Z:
						Current.BaseState = TESRenderWindow::KeyState_Z;
						break;
					}

					Current.Active = Current.NewState = false;
				}
			}

			BuiltIn::ModifierOverride::~ModifierOverride()
			{
				for (int i = kKey_CTRL; i < kKey__MAX; i++)
				{
					State& Current = Data[i];
					SME_ASSERT(Current.Active == false);
				}
			}

			bool BuiltIn::ModifierOverride::IsActive(UInt8 Modifier) const
			{
				SME_ASSERT(Modifier);

				if (Modifier & kModifier_Control)
					return Data[kKey_CTRL].Active;
				else if (Modifier & kModifier_Shift)
					return Data[kKey_SHIFT].Active;
				else if (Modifier & kModifier_Alt)
					return Data[kKey_ALT].Active;
				else if (Modifier & kModifier_Space)
					return Data[kKey_SPACE].Active;
				else if (Modifier & kModifier_Z)
					return Data[kKey_Z].Active;
				else
					return false;
			}

			bool BuiltIn::ModifierOverride::GetOverrideState(UInt8 Modifier) const
			{
				SME_ASSERT(Modifier);

				const State* Current = nullptr;
				if (Modifier & kModifier_Control)
					Current = &Data[kKey_CTRL];
				else if (Modifier & kModifier_Shift)
					Current = &Data[kKey_SHIFT];
				else if (Modifier & kModifier_Alt)
					Current = &Data[kKey_ALT];
				else if (Modifier & kModifier_Space)
					Current = &Data[kKey_SPACE];
				else if (Modifier & kModifier_Z)
					Current = &Data[kKey_Z];
				else
					SME_ASSERT(Current);

				SME_ASSERT(Current->Active);
				return Current->NewState;
			}

			void BuiltIn::ModifierOverride::Activate(UInt8 Modifier, bool NewState)
			{
				SME_ASSERT(Modifier);

				for (int i = kKey_CTRL; i < kKey__MAX; i++)
				{
					State& Current = Data[i];
					if (Modifier & (1 << i))
					{
						SME_ASSERT(Current.Active == false);

						Current.Active = true;
						Current.NewState = NewState;

						if (Current.BaseState)
						{
							Current.StateBuffer = *Current.BaseState;
							*Current.BaseState = NewState;
						}
					}
				}
			}

			void BuiltIn::ModifierOverride::Deactivate(UInt8 Modifier)
			{
				SME_ASSERT(Modifier);

				for (int i = kKey_CTRL; i < kKey__MAX; i++)
				{
					State& Current = Data[i];
					if (Modifier & (1 << i))
					{
						SME_ASSERT(Current.Active);

						Current.Active = false;

						if (Current.BaseState)
						{
							*Current.BaseState = Current.StateBuffer;
						}
					}
				}
			}

			BuiltIn::KeyBinding::KeyBinding(SHORT Key, UInt8 Modifiers) :
				KeyCode(Key),
				Modifiers(Modifiers)
			{
				SME_ASSERT(KeyCode);
			}

			void BuiltIn::KeyBinding::Trigger() const
			{
				bool Control = Modifiers & kModifier_Control;
				bool Shift = Modifiers & kModifier_Shift;
				bool Alt = Modifiers & kModifier_Alt;
				bool Space = Modifiers & kModifier_Space;
				bool Z = Modifiers & kModifier_Z;

				ModifierOverride::Instance.Activate(kModifier_Control, Control);
				ModifierOverride::Instance.Activate(kModifier_Shift, Shift);
				ModifierOverride::Instance.Activate(kModifier_Alt, Alt);
				ModifierOverride::Instance.Activate(kModifier_Space, Space);
				ModifierOverride::Instance.Activate(kModifier_Z, Z);

				BGSEEUI->GetSubclasser()->TunnelDialogMessage(*TESRenderWindow::WindowHandle,
															  WM_KEYDOWN,
															  KeyCode,
															  NULL);

				ModifierOverride::Instance.Deactivate(kModifier_Control);
				ModifierOverride::Instance.Deactivate(kModifier_Shift);
				ModifierOverride::Instance.Deactivate(kModifier_Alt);
				ModifierOverride::Instance.Deactivate(kModifier_Space);
				ModifierOverride::Instance.Deactivate(kModifier_Z);
			}


			bool BuiltIn::KeyBinding::IsActivated(SHORT Key) const
			{
				bool Control = ((Modifiers & kModifier_Control) && GetAsyncKeyState(kHoldable_Control)) || ((Modifiers & kModifier_Control) == false && GetAsyncKeyState(kHoldable_Control) == NULL);
				bool Shift = ((Modifiers & kModifier_Shift) && GetAsyncKeyState(kHoldable_Shift)) || ((Modifiers & kModifier_Shift) == false && GetAsyncKeyState(kHoldable_Shift) == NULL);
				bool Alt = ((Modifiers & kModifier_Alt) && GetAsyncKeyState(VK_MENU)) || ((Modifiers & kModifier_Alt) == false && GetAsyncKeyState(VK_MENU) == NULL);
				bool Space = ((Modifiers & kModifier_Space) && GetAsyncKeyState(kHoldable_Space)) || ((Modifiers & kModifier_Space) == false && GetAsyncKeyState(kHoldable_Space) == NULL);
				bool Z = ((Modifiers & kModifier_Z) && GetAsyncKeyState(kHoldable_Z)) || ((Modifiers & kModifier_Z) == false && GetAsyncKeyState(kHoldable_Z) == NULL);

				if (Key == KeyCode)
				{
					if (Control && Shift && Alt && Space && Z)
						return true;
				}

				return false;
			}


			SHORT BuiltIn::KeyBinding::GetKeyCode() const
			{
				return KeyCode;
			}

			UInt8 BuiltIn::KeyBinding::GetModifiers(bool StripSpecialModifiers) const
			{
				UInt8 Out = Modifiers;

				if (StripSpecialModifiers)
				{
					Out &= ~kModifier_Space;
					Out &= ~kModifier_Z;
				}

				return Out;
			}
		}
	}
}