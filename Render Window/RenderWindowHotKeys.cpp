#include "RenderWindowHotKeys.h"

namespace cse
{
	namespace renderWindow
	{
		KeyCombo::KeyCombo()
		{
			Modifiers = NULL;
			KeyCode = NULL;		// some key that doesn't exist, or does it? *eerie music plays as Stephen Fry's face fades into view from the darkness*
		}

		KeyCombo::KeyCombo(SHORT Key, bool Control, bool Shift, bool Alt)
		{
			Set(Key, Control, Shift, Alt);
		}

		KeyCombo::KeyCombo(SHORT Key, UInt8 Modifiers)
		{
			Set(Key, Modifiers);
		}

		KeyCombo::KeyCombo(const KeyCombo& RHS)
		{
			Modifiers = RHS.Modifiers;
			KeyCode = RHS.KeyCode;
		}

		std::string KeyCombo::GetDescription() const
		{
			std::string Out;

			if ((Modifiers & kModifier_Control))
				Out.append("CTRL + ");

			if ((Modifiers & kModifier_Shift))
				Out.append("SHIFT + ");

			if ((Modifiers & kModifier_Alt))
				Out.append("ALT + ");

			Out.append(GetKeyName(KeyCode));

			return Out;
		}

		bool KeyCombo::IsActivated(SHORT Key) const
		{
			if (Key == KeyCode)
			{
				if ((Modifiers & kModifier_Control) == false || GetAsyncKeyState(VK_CONTROL))
				{
					if ((Modifiers & kModifier_Shift) == false || GetAsyncKeyState(VK_SHIFT))
					{
						if ((Modifiers & kModifier_Alt) == false || GetAsyncKeyState(VK_MENU))
						{
							return true;
						}
					}
				}
			}

			return false;
		}

		bool KeyCombo::IsValid() const
		{
			return KeyCode != NULL;
		}

		void KeyCombo::Set(SHORT Key, bool Control, bool Shift, bool Alt)
		{
			SME_ASSERT(Key != NULL);

			KeyCode = Key;
			Modifiers = NULL;

			if (Control)
				Modifiers |= kModifier_Control;

			if (Shift)
				Modifiers |= kModifier_Shift;

			if (Alt)
				Modifiers |= kModifier_Alt;
		}


		void KeyCombo::Set(SHORT Key, UInt8 Modifiers)
		{
			this->KeyCode = Key;
			this->Modifiers = Modifiers;

			SME_ASSERT(IsValid());
		}

		bool KeyCombo::HasControl() const
		{
			return Modifiers & kModifier_Control;
		}

		bool KeyCombo::HasShift() const
		{
			return Modifiers & kModifier_Shift;
		}

		bool KeyCombo::HasAlt() const
		{
			return Modifiers & kModifier_Alt;
		}

		KeyCombo& KeyCombo::operator=(const KeyCombo& RHS)
		{
			KeyCode = RHS.KeyCode;
			Modifiers = RHS.Modifiers;
			return *this;
		}

		int KeyCombo::Serialize(const KeyCombo& Combo)
		{
			SME_ASSERT(Combo.KeyCode != NULL);

			int Out = (Combo.KeyCode & 0xFFFF) << 16 | (Combo.Modifiers & 0xFFFF);
			return Out;
		}

		cse::renderWindow::KeyCombo KeyCombo::Deserialize(int Serialized)
		{
			SHORT KeyCode = (Serialized >> 16) & 0xFFFF;
			UInt8 Modifiers = (Serialized) & 0xFFFF;

			return KeyCombo(KeyCode, Modifiers);
		}

		std::string KeyCombo::GetKeyName(SHORT Key)
		{
			unsigned int ScanCode = MapVirtualKey(Key, MAPVK_VK_TO_VSC);

			// because MapVirtualKey strips the extended bit for some keys
			switch (Key)
			{
			case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
			case VK_PRIOR: case VK_NEXT: // page up and page down
			case VK_END: case VK_HOME:
			case VK_INSERT: case VK_DELETE:
			case VK_DIVIDE: // numpad slash
			case VK_NUMLOCK:
				{
					ScanCode |= 0x100; // set extended bit
					break;
				}
			}

			char Buffer[0x100] = { 0 };
			if (GetKeyNameText(ScanCode << 16, Buffer, sizeof(Buffer)))
				return Buffer;
			else
				return "<unknown-key>";
		}

		bool KeyCombo::operator==(const KeyCombo& RHS)
		{
			if (KeyCode == RHS.KeyCode && Modifiers == RHS.Modifiers)
				return true;
			else
				return false;
		}

		bool RenderWindowHotKey::IsOverride() const
		{
			return BuiltinKey.IsValid();
		}

		void RenderWindowHotKey::SetActiveCombo(SHORT Key, UInt8 Modifiers)
		{
			ActiveKey.Set(Key, Modifiers);
		}

		void RenderWindowHotKey::Save(bgsee::INIManagerSetterFunctor& INI, const char* Section) const
		{
			char Buffer[0x100] = { 0 };
			FORMAT_STR(Buffer, "%08X", KeyCombo::Serialize(ActiveKey));
			INI(GUID.c_str(), Section, Buffer);
		}

		bool RenderWindowHotKey::Load(bgsee::INIManagerGetterFunctor& INI, const char* Section)
		{
			char Buffer[0x100] = { 0 };
			if (INI(GUID.c_str(), Section, nullptr, Buffer, sizeof(Buffer)))
			{
				if (strlen(Buffer))
				{
					int Data = 0;
					if (sscanf_s(Buffer, "%08X", &Data))
					{
						ActiveKey = KeyCombo::Deserialize(Data);
						if (ActiveKey.IsValid())
							return true;
					}
				}
			}

			// reset to the default combo if loading failed
			ActiveKey = DefaultKey;
			return false;
		}

		const char* RenderWindowHotKey::GetName() const
		{
			return BoundAction.GetName();
		}

		const char* RenderWindowHotKey::GetDescription() const
		{
			return BoundAction.GetDescription();
		}

		RenderWindowHotKey::RenderWindowHotKey(const char* GUID, actions::IRenderWindowAction& Action, KeyCombo Active) :
			GUID(GUID),
			BoundAction(Action),
			ActiveKey(Active),
			DefaultKey(Active),
			BuiltinKey()
		{
			SME_ASSERT(ActiveKey.IsValid());
		}

		RenderWindowHotKey::RenderWindowHotKey(const char* GUID, actions::IRenderWindowAction& Action, KeyCombo Active, KeyCombo Builtin) :
			GUID(GUID),
			BoundAction(Action),
			ActiveKey(Active),
			DefaultKey(Active),
			BuiltinKey(Builtin)
		{
			SME_ASSERT(ActiveKey.IsValid());
			SME_ASSERT(BuiltinKey.IsValid());
		}

		bool RenderWindowHotKey::operator==(const KeyCombo& RHS)
		{
			if (ActiveKey == RHS)
				return true;
			else
				return false;
		}

		bool RenderWindowHotKey::HandleActiveKeyCombo(SHORT Key)
		{
			if (ActiveKey.IsActivated(Key))
			{
				BoundAction();
				return true;
			}
			else
				return false;
		}

		bool RenderWindowHotKey::HandleBuiltinKeyCombo(SHORT Key)
		{
			if (IsOverride())
				return BuiltinKey.IsActivated(Key);
			else
				return false;
		}

		const char*		RenderWindowHotKeyManager::kINISection = "RenderWindowHotKeys";

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


		RenderWindowHotKey* RenderWindowHotKeyManager::LookupHotKey(KeyCombo Key)
		{
			for (auto& Itr : HotKeys)
			{
				if (Itr == Key)
					return &Itr;
			}

			return nullptr;
		}

		void RenderWindowHotKeyManager::RegisterHotKey(const char* GUID, actions::IRenderWindowAction& Action, KeyCombo& Combo)
		{
			SME_ASSERT(LookupHotKey(Combo) == nullptr);

			HotKeys.push_back(RenderWindowHotKey(GUID, Action, Combo));
		}

		void RenderWindowHotKeyManager::RegisterHotKey(const char* GUID, actions::IRenderWindowAction& Action, KeyCombo& Combo, KeyCombo& Default)
		{
			SME_ASSERT(LookupHotKey(Combo) == nullptr);

			HotKeys.push_back(RenderWindowHotKey(GUID, Action, Combo, Default));
		}

		void RenderWindowHotKeyManager::SaveToINI() const
		{
			bgsee::INIManagerSetterFunctor Setter(BGSEEMAIN->INISetter());

			for (auto& Itr : HotKeys)
				Itr.Save(Setter, kINISection);
		}

		void RenderWindowHotKeyManager::LoadFromINI()
		{
			bgsee::INIManagerGetterFunctor Getter(BGSEEMAIN->INIGetter());

			bool Errors = false;
			for (auto& Itr : HotKeys)
			{
				if (Itr.Load(Getter, kINISection) == false)
					Errors = true;
			}

			if (Errors)
				BGSEECONSOLE_MESSAGE("Encountered errors when loading hotkeys. Falling back to defaults");
		}

		bool RenderWindowHotKeyManager::RenderModalHotKeyEditor(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			static ImGuiTextFilter Filter;

			Filter.Draw();
			ImGui::TextWrapped("Double click on a hotkey to remap it.");

			ImGui::Columns(3, "hotkey_list");
			{
				int i = 0;
				RenderWindowHotKey* Selection = nullptr;
				for (auto& Itr : HotKeys)
				{
					const char* Name = Itr.GetName();
					const char* Desc = Itr.GetDescription();
					std::string Key(Itr.ActiveKey.GetDescription());

					if (Filter.PassFilter(Name) == false && Filter.PassFilter(Desc) == false)
						continue;

					char Buffer[0x100] = {0};
					FORMAT_STR(Buffer, "%s %s-%d", Name, Desc, i);
					bool Selected = false;

					ImGui::PushID(Buffer);
					{
						if (ImGui::Selectable(Name, false,
											  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups) &&
							ImGui::IsMouseDoubleClicked(0))
						{
							Selection = &Itr;
						}

						ImGui::NextColumn();
						ImGui::Selectable(Key.c_str());
						ImGui::NextColumn();
						ImGui::Selectable(Desc, false, NULL, ImVec2(200, 0));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip(Desc);
						ImGui::NextColumn();
					}
					ImGui::PopID();

					i++;
				}

				if (Selection)
				{
					ModalWindowProviderOSDLayer::Instance.ShowModal("Edit Hotkey Binding",
																	std::bind(&RenderWindowHotKeyManager::RenderModalBindingEditor, this,
																			  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
																	Selection,
																	ImGuiWindowFlags_AlwaysAutoResize);
				}
			}
			ImGui::Columns();

			if (ImGui::Button("Close", ImVec2(120, 0)))
			{
				Filter.Clear();
				return true;
			}
			else
				return false;
		}

		bool RenderWindowHotKeyManager::RenderModalBindingEditor(RenderWindowOSD* OSD, ImGuiDX9* GUI, void* UserData)
		{
			static SHORT HotKeyBuffer = NULL;
			static bool Shift = false, Control = false, Alt = false;
			static bool DoInit = false;

			RenderWindowHotKey* CurrentHotKey = (RenderWindowHotKey*)UserData;

			if (DoInit == false)
			{
				DoInit = true;

				HotKeyBuffer = NULL;
				Shift = CurrentHotKey->ActiveKey.HasShift();
				Control = CurrentHotKey->ActiveKey.HasControl();
				Alt = CurrentHotKey->ActiveKey.HasAlt();
			}

			SHORT FirstKeyDown = NULL;
			bool ClearBuffer = false;
			for (int i = NULL; i < 256; i++)
			{
				bool Skip = false;
				switch (i)
				{
				case VK_ESCAPE:
					if (ImGui::IsKeyPressed(i))
						ClearBuffer = true;
				case VK_SHIFT:
				case VK_CONTROL:
				case VK_MENU:
				case VK_LSHIFT:
				case VK_RSHIFT:
				case VK_LCONTROL:
				case VK_RCONTROL:
				case VK_LMENU:
				case VK_RMENU:
				case VK_SPACE:
					Skip = true;		// skip special keys
					break;
				}

				if (Skip)
					continue;

				if (ImGui::IsKeyReleased(i))
				{
					FirstKeyDown = i;

					Shift = ImGui::IsKeyDown(VK_SHIFT);
					Control = ImGui::IsKeyDown(VK_CONTROL);
					Alt = ImGui::IsKeyDown(VK_MENU);

					break;
				}
			}

			if (ClearBuffer)
				HotKeyBuffer = NULL;
			else if (FirstKeyDown)
				HotKeyBuffer = FirstKeyDown;

			KeyCombo NewBinding;
			if (HotKeyBuffer)
				NewBinding.Set(HotKeyBuffer, Control, Shift, Alt);

			const ImVec4 RedColor = ImColor::HSV(0, 0.6f, 0.6f);
			bool KeyConflict = false;
			RenderWindowHotKey* Existing = LookupHotKey(NewBinding);
			if (Existing && Existing != CurrentHotKey)
				KeyConflict = true;

			ImGui::Text("Press the key combination to update the binding for '%s'", CurrentHotKey->GetName());
			ImGui::Separator();
			ImGui::Text("Current Binding: %s", CurrentHotKey->ActiveKey.GetDescription().c_str());
			if (NewBinding.IsValid() == false)
				ImGui::TextColored(RedColor, "New Binding: Invalid");
			else
			{
				if (KeyConflict)
					ImGui::TextColored(RedColor, "New Binding: %s (conflicts with %s)", NewBinding.GetDescription().c_str(), Existing->GetName());
				else
					ImGui::Text("New Binding: %s", NewBinding.GetDescription().c_str());
			}

			ImGui::Columns(3, "modifiers", false);
			{
				ImGui::Checkbox("Control", &Control); ImGui::NextColumn();
				ImGui::Checkbox("Shift", &Shift); ImGui::NextColumn();
				ImGui::Checkbox("Alt", &Alt); ImGui::NextColumn();
			}
			ImGui::Columns();
			ImGui::Separator();

			bool Close = false;
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				if (KeyConflict == false)
				{
					if (!(NewBinding == CurrentHotKey->ActiveKey))
					{
						CurrentHotKey->ActiveKey = NewBinding;
					}

					Close = true;
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
				Close = true;

			if (Close)
			{
				DoInit = false;
				return true;
			}
			else
				return false;
		}

		RenderWindowHotKeyManager::RenderWindowHotKeyManager() :
			HotKeys(),
			OverriddenKeyStates()
		{
			Initialized = false;
		}

		RenderWindowHotKeyManager::~RenderWindowHotKeyManager()
		{
			;//
		}

		void RenderWindowHotKeyManager::Initialize()
		{
			SME_ASSERT(Initialized == false);

			// register the hotkeys first and then load state from the INI file
			RegisterHotKey("{6EC6F5E2-AE52-4475-AC27-5418375B5FB8}", actions::InvertSelection, KeyCombo('V', KeyCombo::kModifier_CTRL_SHIFT));

			RegisterHotKey("{F2BF346D-EC8F-4A33-975D-0E7A9C7424EE}", actions::SelectAll, KeyCombo('A', KeyCombo::kModifier_Control));

			RegisterHotKey("{A1FE6205-5F69-4808-BD4B-F0D297AA215C}", actions::GroupSelection, KeyCombo('G', KeyCombo::kModifier_Control));
			RegisterHotKey("{C028EA1C-76CF-4984-A820-1240E9001679}", actions::UngroupSelection, KeyCombo('U', KeyCombo::kModifier_Control));
			RegisterHotKey("{2C7E9F71-E42E-4455-8069-46EAD79541D1}", actions::OrphanizeSelection, KeyCombo('U', KeyCombo::kModifier_CTRL_ALT));

			RegisterHotKey("{30A153E0-6E49-42C0-8346-3F0EE5DDEE75}", actions::FreezeSelection, KeyCombo('W', KeyCombo::kModifier_Control));
			RegisterHotKey("{24780B09-E7DB-4020-A3D2-4656465CC828}", actions::ThawSelection, KeyCombo('E', KeyCombo::kModifier_Control));
			RegisterHotKey("{44431BAA-E7C9-4D36-93E8-8DE5A7F5A498}", actions::ThawAll, KeyCombo(VK_F2, NULL));
			RegisterHotKey("{0C513C52-86FE-45D5-87AF-461EC2A9FA1B}", actions::ToggleFreezeInactive, KeyCombo('E', KeyCombo::kModifier_CTRL_SHIFT));

			RegisterHotKey("{765B64A2-572D-4D9B-BBDC-197225E6E76C}", actions::ToggleSelectionVisibility, KeyCombo('2', NULL));
			RegisterHotKey("{C6D5E817-81C1-415A-ADDE-6A7CD41C12A0}", actions::ToggleSelectionChildrenVisibility, KeyCombo('3', NULL));
			RegisterHotKey("{F54BA759-71C1-4E09-ADD7-EA9D88D63CBB}", actions::RevealAll, KeyCombo(VK_F3, NULL));

			RegisterHotKey("{9726EE0A-C9C1-4BCA-8B2E-3B72BDBB513D}", actions::DimSelectionOpacity, KeyCombo(VK_OEM_MINUS, NULL));
			RegisterHotKey("{247D9B08-3BFE-4E1C-B268-AED18D4B633B}", actions::ResetSelectionOpacity, KeyCombo(VK_OEM_PLUS, NULL));

			RegisterHotKey("{6D68753D-AEE1-4573-9B8A-4A9E4B17497C}", actions::UnlinkPathGridSelection, KeyCombo('R', KeyCombo::kModifier_Shift));

			RegisterHotKey("{F42F86FA-8BE7-4CC8-A6BA-779A963EEF89}", actions::ShowBatchEditor, KeyCombo(VK_OEM_PERIOD, NULL));
			RegisterHotKey("{D233EB54-BED6-4720-96A6-86442A97E6A6}", actions::ShowUseInfo, KeyCombo(VK_F1, NULL));
			RegisterHotKey("{F11C7C9E-CF74-4C2D-877D-39427F7C81CD}", actions::ShowSearchReplace, KeyCombo('F', KeyCombo::kModifier_CTRL_SHIFT));

			RegisterHotKey("{BBA57061-EE4C-4C0E-8598-C7CE0706E957}", actions::ToggleAuxViewport, KeyCombo(VK_OEM_3, KeyCombo::kModifier_CTRL_SHIFT));

			RegisterHotKey("{2C35CC8B-2CA1-4867-92FD-A64ED6DE1148}", actions::ToggleAlternateMovementSettings, KeyCombo('Q', KeyCombo::kModifier_Shift));
			RegisterHotKey("{E2240B02-1D0B-4885-931F-137DCD6E6D90}", actions::TogglePathGridEditMode, KeyCombo('G', NULL));
			RegisterHotKey("{1A6CD699-8265-48BB-964F-8F6CF55E0103}", actions::ToggleSnapToGrid, KeyCombo('S', KeyCombo::kModifier_Shift));
			RegisterHotKey("{DECA2E00-6FB7-4638-8563-108326891DEA}", actions::ToggleSnapToAngle, KeyCombo('D', KeyCombo::kModifier_Shift));

			RegisterHotKey("{F5862ECB-7413-4C56-A5AF-E5EED401FC17}", actions::FocusOnRefFilter, KeyCombo('F', KeyCombo::kModifier_Control));
			RegisterHotKey("{959D48C0-43FE-47B5-BD61-9D78EEA277FC}", actions::JumpToExteriorCell, KeyCombo('J', KeyCombo::kModifier_Control));


			LoadFromINI();
			Initialized = true;
		}

		void RenderWindowHotKeyManager::Deinitialize()
		{
			SME_ASSERT(Initialized);

			SaveToINI();
			Initialized = false;
		}

		bool RenderWindowHotKeyManager::HandleInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			bool Handled = false;

			if (uMsg == WM_KEYDOWN)
			{
				// walk through the hotkeys and check for activation
				bool NoHandlers = true;
				for (auto& Itr : HotKeys)
				{
					if (Itr.HandleActiveKeyCombo(wParam))
					{
						NoHandlers = false;
						break;
					}
				}

				if (NoHandlers == false)
					Handled = true;
				else
				{
					// check if any of the hotkeys are overrides and consume the input accordingly
					for (auto& Itr : HotKeys)
					{
						if (Itr.IsOverride() && Itr.BuiltinKey.IsActivated(wParam))
						{
							Handled = true;
							break;
						}
					}
				}
			}

			return Handled;
		}

		void RenderWindowHotKeyManager::ShowHotKeyEditor()
		{
			ModalWindowProviderOSDLayer::Instance.ShowModal("Render Window Keyboard Mapping",
															std::bind(&RenderWindowHotKeyManager::RenderModalHotKeyEditor, this, std::placeholders::_1, std::placeholders::_2),
															nullptr,
															ImGuiWindowFlags_AlwaysAutoResize);
		}

		void RenderWindowHotKeyManager::SendBuiltinHotKey(SHORT Key, bool Shift, bool Control)
		{
			if (Shift)
				SetKeyStateOverride(kKey_Shift, Shift);

			if (Control)
				SetKeyStateOverride(kKey_Control, Control);

			BGSEEUI->GetSubclasser()->TunnelDialogMessage(*TESRenderWindow::WindowHandle,
														  WM_KEYDOWN,
														  Key,
														  NULL);

			if (Shift)
				ResetKeyStateOverride(kKey_Shift);

			if (Control)
				ResetKeyStateOverride(kKey_Control);
		}

		SHORT RenderWindowHotKeyManager::HandleDefaultWndProcAsyncKeyState(int vKey)
		{
			KeyStateOverride* Override = nullptr;
			switch (vKey)
			{
			case VK_SHIFT:
			case VK_CONTROL:
			case VK_MENU:
				{
					if (vKey == VK_SHIFT)
						Override = &OverriddenKeyStates[kKey_Shift];
					else if (vKey == VK_CONTROL)
						Override = &OverriddenKeyStates[kKey_Control];
					else
						Override = &OverriddenKeyStates[kKey_Alt];

					if (Override->Active)
					{
						SHORT Result = 0;
						if (Override->NewState)
							Result |= 1 << 15;

						return Result;
					}
				}

				break;
			}

			// key not handled/overridden, call the original function
			return GetAsyncKeyState(vKey);
		}
	}
}