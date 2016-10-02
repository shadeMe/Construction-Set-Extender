#include "RenderWindowInput.h"
#include "RenderWindowManager.h"
#include "ObjectPaletteManager.h"
#include "Hooks\Hooks-Renderer.h"

namespace cse
{
	namespace renderWindow
	{
		namespace input
		{
			BasicKeyBinding::BasicKeyBinding()
			{
				Modifiers = NULL;
				KeyCode = NULL;		// some key that doesn't exist, or does it? *eerie music plays as Stephen Fry's face fades into view from the darkness*
			}

			BasicKeyBinding::BasicKeyBinding(SHORT Key, bool Control, bool Shift, bool Alt)
			{
				Set(Key, Control, Shift, Alt);
			}

			BasicKeyBinding::BasicKeyBinding(SHORT Key, UInt8 Modifiers)
			{
				Set(Key, Modifiers);
			}

			BasicKeyBinding::BasicKeyBinding(const BasicKeyBinding& RHS)
			{
				Modifiers = RHS.Modifiers;
				KeyCode = RHS.KeyCode;
			}

			BasicKeyBinding::BasicKeyBinding(const BuiltIn::KeyBinding& BuiltIn)
			{
				Set(BuiltIn.GetKeyCode(), BuiltIn.GetModifiers(true));
			}

			std::string BasicKeyBinding::GetDescription() const
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

			bool BasicKeyBinding::IsActivated(SHORT Key) const
			{
				bool Control = ((Modifiers & kModifier_Control) && GetAsyncKeyState(VK_CONTROL)) || ((Modifiers & kModifier_Control) == false && GetAsyncKeyState(VK_CONTROL) == NULL);
				bool Shift = ((Modifiers & kModifier_Shift) && GetAsyncKeyState(VK_SHIFT)) || ((Modifiers & kModifier_Shift) == false && GetAsyncKeyState(VK_SHIFT) == NULL);
				bool Alt = ((Modifiers & kModifier_Alt) && GetAsyncKeyState(VK_MENU)) || ((Modifiers & kModifier_Alt) == false && GetAsyncKeyState(VK_MENU) == NULL);

				if ((Key == NULL && GetAsyncKeyState(KeyCode)) || (Key && Key == KeyCode))
				{
					if (Control && Shift && Alt)
						return true;
				}

				return false;
			}

			bool BasicKeyBinding::IsValid() const
			{
				return KeyCode != NULL;
			}

			void BasicKeyBinding::Set(SHORT Key, bool Control, bool Shift, bool Alt)
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


			void BasicKeyBinding::Set(SHORT Key, UInt8 Modifiers)
			{
				this->KeyCode = Key;
				this->Modifiers = Modifiers;

				SME_ASSERT(IsValid());
			}



			bool BasicKeyBinding::HasModifiers() const
			{
				return Modifiers != NULL;
			}

			bool BasicKeyBinding::HasControl() const
			{
				return Modifiers & kModifier_Control;
			}

			bool BasicKeyBinding::HasShift() const
			{
				return Modifiers & kModifier_Shift;
			}

			bool BasicKeyBinding::HasAlt() const
			{
				return Modifiers & kModifier_Alt;
			}

			SHORT BasicKeyBinding::GetKeyCode() const
			{
				return KeyCode;
			}

			BasicKeyBinding& BasicKeyBinding::operator=(const BasicKeyBinding& RHS)
			{
				KeyCode = RHS.KeyCode;
				Modifiers = RHS.Modifiers;
				return *this;
			}

			int BasicKeyBinding::Serialize(const BasicKeyBinding& Combo)
			{
				SME_ASSERT(Combo.KeyCode != NULL);

				int Out = (Combo.KeyCode & 0xFFFF) << 16 | (Combo.Modifiers & 0xFFFF);
				return Out;
			}

			BasicKeyBinding BasicKeyBinding::Deserialize(int Serialized)
			{
				SHORT KeyCode = (Serialized >> 16) & 0xFFFF;
				UInt8 Modifiers = (Serialized) & 0xFFFF;

				return BasicKeyBinding(KeyCode, Modifiers);
			}

			std::string BasicKeyBinding::GetKeyName(SHORT Key)
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

			bool BasicKeyBinding::operator==(const BasicKeyBinding& RHS)
			{
				if (KeyCode == RHS.KeyCode && Modifiers == RHS.Modifiers)
					return true;
				else
					return false;
			}

			bool BasicKeyBinding::operator!=(const BasicKeyBinding& RHS)
			{
				return !(*this == RHS);
			}

			IKeyboardEventHandler::~IKeyboardEventHandler() = default;

			IHotKey::IHotKey(const char* GUID, bool Editable, UInt8 Context) :
				IKeyboardEventHandler(),
				GUID(GUID),
				DefaultBinding(),
				ActiveBinding(),
				Context(Context),
				Editable(Editable)
			{
				;//
			}


			IHotKey::IHotKey(const char* GUID, bool Editable, const ExecutionContext& Context) :
				IKeyboardEventHandler(),
				GUID(GUID),
				DefaultBinding(),
				ActiveBinding(),
				Context(Context),
				Editable(Editable)
			{
				;//
			}

			IHotKey::~IHotKey() = default;

			const BasicKeyBinding& IHotKey::GetActiveBinding() const
			{
				return ActiveBinding;
			}

			void IHotKey::SetActiveBinding(const BasicKeyBinding& NewBinding)
			{
				SME_ASSERT(NewBinding.IsValid());
				ActiveBinding = NewBinding;
			}

			bool IHotKey::IsActiveBindingTriggered(SHORT Key) const
			{
				return ActiveBinding.IsActivated(Key);
			}

			void IHotKey::Save(bgsee::INIManagerSetterFunctor& INI, const char* Section) const
			{
				if (Editable == false)
					return;

				char Buffer[0x100] = { 0 };
				FORMAT_STR(Buffer, "%08X", BasicKeyBinding::Serialize(GetActiveBinding()));
				INI(GUID.c_str(), Section, Buffer);
			}

			bool IHotKey::Load(bgsee::INIManagerGetterFunctor& INI, const char* Section)
			{
				if (Editable == false)
					return true;

				char Buffer[0x100] = { 0 };
				if (INI(GUID.c_str(), Section, nullptr, Buffer, sizeof(Buffer)))
				{
					if (strlen(Buffer))
					{
						int Data = 0;
						if (sscanf_s(Buffer, "%08X", &Data))
						{
							BasicKeyBinding Loaded = BasicKeyBinding::Deserialize(Data);
							if (Loaded.IsValid())
							{
								SetActiveBinding(Loaded);
								return true;
							}
						}
					}
				}

				// reset to the default combo if loading failed
				SetActiveBinding(DefaultBinding);
				return false;
			}

			bool IHotKey::IsEditable() const
			{
				return Editable;
			}

			const ExecutionContext& IHotKey::GetExecutionContext() const
			{
				return Context;
			}

			bool HoldableKeyHandler::IsActiveBindingTriggered(SHORT Key /*= NULL*/) const
			{
				if (Key)
					return ActiveBinding.GetKeyCode() == Key;
				else
					return GetAsyncKeyState(ActiveBinding.GetKeyCode());
			}

			void HoldableKeyHandler::NotifyListeners(UInt8 Event) const
			{
				for (auto& Itr : EventListeners)
					Itr(Event);
			}

			HoldableKeyHandler::HoldableKeyHandler(const char* GUID, const char* Name, const char* Desc, BasicKeyBinding Default) :
				IHotKey(GUID),
				Name(Name),
				Description(Desc),
				EventListeners()
			{
				SetActiveBinding(Default);
				DefaultBinding = ActiveBinding;
			}

			bool HoldableKeyHandler::IsHeldDown() const
			{
				return IsActiveBindingTriggered();
			}

			void HoldableKeyHandler::RegisterListener(HoldableKeyEventDelegateT Delegate)
			{
				SME_ASSERT(Delegate);

				EventListeners.push_back(Delegate);
			}

			const char* HoldableKeyHandler::GetName() const
			{
				return Name.c_str();
			}

			const char* HoldableKeyHandler::GetDescription() const
			{
				return Description.c_str();
			}

			void HoldableKeyHandler::SetActiveBinding(const BasicKeyBinding& NewBinding)
			{
				SME_ASSERT(NewBinding.HasModifiers() == false);
				SME_ASSERT(NewBinding.GetKeyCode() != VK_CONTROL && NewBinding.GetKeyCode() != VK_MENU);		// special keys
				ActiveBinding = NewBinding;
			}

			const UInt8 HoldableKeyHandler::GetHandlerType() const
			{
				return kType_Stateful;
			}

			IKeyboardEventHandler::EventResult HoldableKeyHandler::HandleActive(UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				EventResult Result;
				switch (uMsg)
				{
				case WM_KEYDOWN:
				case WM_KEYUP:
					if (IsActiveBindingTriggered(wParam))
					{
						Result.Triggered = true;
						if (GetExecutionContext().IsExecutable())
						{
							Result.Success = true;
							if (uMsg == WM_KEYDOWN)
								NotifyListeners(kEvent_KeyDown);
							else
								NotifyListeners(kEvent_KeyUp);
						}
						else
							Result.InvalidContext = true;
					}

					break;
				}

				return Result;
			}

			IKeyboardEventHandler::EventResult HoldableKeyHandler::HandleBuiltIn(UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				return EventResult();
			}

			HoldableKeyOverride::HoldableKeyOverride(const char* GUID, SHORT BuiltIn, bool Editable) :
				HoldableKeyHandler(GUID, "", ""),
				BuiltInKey(BuiltIn)
			{
				SME_ASSERT(BuiltInKey == BuiltIn::kHoldable_Control ||
						   BuiltInKey == BuiltIn::kHoldable_Shift ||
						   BuiltInKey == BuiltIn::kHoldable_Space ||
						   BuiltInKey == BuiltIn::kHoldable_X ||
						   BuiltInKey == BuiltIn::kHoldable_Y ||
						   BuiltInKey == BuiltIn::kHoldable_Z ||
						   BuiltInKey == BuiltIn::kHoldable_S ||
						   BuiltInKey == BuiltIn::kHoldable_V);

				DefaultBinding.Set(BuiltInKey, NULL);
				ActiveBinding = DefaultBinding;

				switch (BuiltInKey)
				{
				case BuiltIn::kHoldable_Control:
					Name = "Unbound Action";
					break;
				case BuiltIn::kHoldable_Shift:
					Name = "Rotate Camera";
					break;
				case BuiltIn::kHoldable_Space:
					Name = "Pan Camera";
					break;
				case BuiltIn::kHoldable_X:
					Name = "Transform along X-Axis";
					break;
				case BuiltIn::kHoldable_Y:
					Name = "Transform along Y-Axis";
					break;
				case BuiltIn::kHoldable_Z:
					Name = "Transform along Z-Axis";
					break;
				case BuiltIn::kHoldable_S:
					Name = "Scale";
					break;
				case BuiltIn::kHoldable_V:
					Name = "Zoom Camera";
					break;
				}

				this->Editable = Editable;
			}

			UInt8* HoldableKeyOverride::GetBaseState() const
			{
				switch (BuiltInKey)
				{
				case BuiltIn::kHoldable_Control:
					return TESRenderWindow::KeyState_Control;
				case BuiltIn::kHoldable_Shift:
					return TESRenderWindow::KeyState_Shift;
				case BuiltIn::kHoldable_Space:
					return TESRenderWindow::KeyState_SpaceMMB;
				case BuiltIn::kHoldable_X:
					return TESRenderWindow::KeyState_X;
				case BuiltIn::kHoldable_Y:
					return TESRenderWindow::KeyState_Y;
				case BuiltIn::kHoldable_Z:
					return TESRenderWindow::KeyState_Z;
				case BuiltIn::kHoldable_S:
					return TESRenderWindow::KeyState_S;
				case BuiltIn::kHoldable_V:
					return TESRenderWindow::KeyState_V;
				}

				return nullptr;
			}

			SHORT HoldableKeyOverride::GetBuiltInKey() const
			{
				return BuiltInKey;
			}

			IKeyboardEventHandler::EventResult HoldableKeyOverride::HandleActive(UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				EventResult Result = HoldableKeyHandler::HandleActive(uMsg, wParam, lParam);

				if (Result.Success)
				{
					// tunnel the built-in key on success
					BGSEEUI->GetSubclasser()->TunnelDialogMessage(*TESRenderWindow::WindowHandle,
																  uMsg,
																  BuiltInKey,
																  NULL);
				}

				return Result;
			}

			IKeyboardEventHandler::EventResult HoldableKeyOverride::HandleBuiltIn(UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				EventResult Result;
				switch (uMsg)
				{
				case WM_KEYDOWN:
				case WM_KEYUP:
					if (wParam == BuiltInKey)
						Result.Triggered = Result.Success = true;

					break;
				}

				return Result;
			}


			ComboKeyOverride::ComboKeyOverride(const char* GUID, actions::BuiltInKeyComboRWA& Action) :
				IHotKey(GUID, true, Action.GetExecutionContext()),
				BuiltInCombo(Action.GetBuiltInBinding()),
				BoundAction(Action)
			{
				DefaultBinding.Set(BuiltInCombo.GetKeyCode(), BuiltInCombo.GetModifiers(true));
				ActiveBinding = DefaultBinding;
			}

			ComboKeyOverride::ComboKeyOverride(const char* GUID, actions::BuiltInKeyComboRWA& Action, BasicKeyBinding OverrideKey) :
				IHotKey(GUID, true, Action.GetExecutionContext()),
				BuiltInCombo(Action.GetBuiltInBinding()),
				BoundAction(Action)
			{
				SME_ASSERT(OverrideKey.IsValid());
				DefaultBinding = ActiveBinding = OverrideKey;
			}

			const char* ComboKeyOverride::GetName() const
			{
				return BoundAction.GetName();
			}

			const char* ComboKeyOverride::GetDescription() const
			{
				return BoundAction.GetDescription();
			}

			const UInt8 ComboKeyOverride::GetHandlerType() const
			{
				return kType_Stateless;
			}

			IKeyboardEventHandler::EventResult ComboKeyOverride::HandleActive(UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				EventResult Result;

				if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
				{
					if (IsActiveBindingTriggered(wParam))
					{
						Result.Triggered = true;
						if (GetExecutionContext().IsExecutable())
							Result.Success = BoundAction();
						else
							Result.InvalidContext = true;
					}
				}

				return Result;
			}

			IKeyboardEventHandler::EventResult ComboKeyOverride::HandleBuiltIn(UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				EventResult Result;

				if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
				{
					if (BuiltInCombo.IsActivated(wParam))
						Result.Triggered = Result.Success = true;
				}

				return Result;
			}

			ActionableKeyHandler::ActionableKeyHandler(const char* GUID, IRenderWindowAction& Action, BasicKeyBinding Default) :
				IHotKey(GUID, true, Action.GetExecutionContext()),
				BoundAction(Action)
			{
				DefaultBinding = Default;
				ActiveBinding = DefaultBinding;
			}

			const char* ActionableKeyHandler::GetName() const
			{
				return BoundAction.GetName();
			}

			const char* ActionableKeyHandler::GetDescription() const
			{
				return BoundAction.GetDescription();
			}

			const UInt8 ActionableKeyHandler::GetHandlerType() const
			{
				return kType_Stateless;
			}

			IKeyboardEventHandler::EventResult ActionableKeyHandler::HandleActive(UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				EventResult Result;

				if (uMsg == WM_KEYDOWN)
				{
					if (IsActiveBindingTriggered(wParam))
					{
						Result.Triggered = true;
						if (GetExecutionContext().IsExecutable())
							Result.Success = BoundAction();
						else
							Result.InvalidContext = true;
					}
				}

				return Result;
			}

			IKeyboardEventHandler::EventResult ActionableKeyHandler::HandleBuiltIn(UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				return EventResult();
			}


			const char*		RenderWindowKeyboardManager::kINISection_HotKeys = "RenderWindowHotKeys";

			IHotKey* RenderWindowKeyboardManager::LookupHotKey(BasicKeyBinding Key, UInt8 HandlerType, ExecutionContext Context, bool OnlyMatchKeyCode)
			{
				SME_ASSERT(HandlerType == IKeyboardEventHandler::kType_Stateful || HandlerType == IKeyboardEventHandler::kType_Stateless);
				SME_ASSERT(Key.IsValid());

				for (auto& Itr : HotKeys)
				{
					if (((OnlyMatchKeyCode && Key.GetKeyCode() == Itr->GetActiveBinding().GetKeyCode()) || (!OnlyMatchKeyCode && Key == Itr->GetActiveBinding())) &&
						HandlerType == Itr->GetHandlerType() &&
						Context == Itr->GetExecutionContext())
					{
						return Itr.get();
					}
				}

				return nullptr;
			}

			HoldableKeyOverride* RenderWindowKeyboardManager::RegisterHoldableOverride(const char* GUID, SHORT HoldableKey, bool Editable)
			{
				SME_ASSERT(LookupHotKey(BasicKeyBinding(HoldableKey, NULL), IKeyboardEventHandler::kType_Stateful) == nullptr);

				std::unique_ptr<HoldableKeyOverride> Temp(new HoldableKeyOverride(GUID, HoldableKey, Editable));
				HoldableKeyOverride* Out = Temp.get();
				HotKeys.push_back(std::move(Temp));
				return Out;
			}

			ComboKeyOverride* RenderWindowKeyboardManager::RegisterComboKeyOverride(const char* GUID, actions::BuiltInKeyComboRWA& Action, BasicKeyBinding OverrideKey)
			{
				if (OverrideKey.IsValid() == false)
					OverrideKey = BasicKeyBinding(Action.GetBuiltInBinding());

				SME_ASSERT(LookupHotKey(OverrideKey, IKeyboardEventHandler::kType_Stateless, Action.GetExecutionContext()) == nullptr);

				std::unique_ptr<ComboKeyOverride> Temp(new ComboKeyOverride(GUID, Action, OverrideKey));
				ComboKeyOverride* Out = Temp.get();
				HotKeys.push_back(std::move(Temp));
				return Out;
			}

			ActionableKeyHandler* RenderWindowKeyboardManager::RegisterActionableKeyHandler(const char* GUID, IRenderWindowAction& Action, BasicKeyBinding Default)
			{
				SME_ASSERT(LookupHotKey(Default, IKeyboardEventHandler::kType_Stateless, Action.GetExecutionContext()) == nullptr);

				std::unique_ptr<ActionableKeyHandler> Temp(new ActionableKeyHandler(GUID, Action, Default));
				ActionableKeyHandler* Out = Temp.get();
				HotKeys.push_back(std::move(Temp));
				return Out;
			}

			HoldableKeyHandler* RenderWindowKeyboardManager::RegisterHoldableHandler(const char* GUID, const char* Name, const char* Desc, BasicKeyBinding Default)
			{
				SME_ASSERT(LookupHotKey(Default, IKeyboardEventHandler::kType_Stateful) == nullptr);

				std::unique_ptr<HoldableKeyHandler> Temp(new HoldableKeyHandler(GUID, Name, Desc, Default));
				HoldableKeyHandler* Out = Temp.get();
				HotKeys.push_back(std::move(Temp));
				return Out;
			}

			void RenderWindowKeyboardManager::SaveToINI() const
			{
				bgsee::INIManagerSetterFunctor Setter(BGSEEMAIN->INISetter());

				for (auto& Itr : HotKeys)
					Itr->Save(Setter, kINISection_HotKeys);
			}

			void RenderWindowKeyboardManager::LoadFromINI()
			{
				bgsee::INIManagerGetterFunctor Getter(BGSEEMAIN->INIGetter());

				bool Errors = false;
				for (auto& Itr : HotKeys)
				{
					if (Itr->Load(Getter, kINISection_HotKeys) == false)
						Errors = true;
				}

				if (Errors)
					BGSEECONSOLE_MESSAGE("Encountered errors when loading hotkeys. Falling back to defaults");
			}

			bool RenderWindowKeyboardManager::RenderModalHotKeyEditor(RenderWindowOSD* OSD, ImGuiDX9* GUI)
			{
				static ImGuiTextFilter Filter;

				Filter.Draw("Filter", 350); ImGui::SameLine(0, 20);
				ImGui::Text("Double click on a hotkey to remap it.");

				ImGui::Separator();
				ImGui::Columns(3, "hotkey_list", false);
				{
					ImGui::Text("Action"); ImGui::NextColumn();
					ImGui::Text("Binding"); ImGui::NextColumn();
					ImGui::Text("Description"); ImGui::NextColumn();
					ImGui::Separator();
				}
				ImGui::Columns();

				ImGui::BeginChild("hotkey_child_frame", ImVec2(0, 455));
				{
					ImGui::Columns(3, "hotkey_list", false);
					{
						int i = 0;
						IHotKey* Selection = nullptr;
						for (auto& Itr : HotKeys)
						{
							const char* Name = Itr->GetName();
							const char* Desc = Itr->GetDescription();
							std::string Key(Itr->GetActiveBinding().GetDescription());

							if (Itr->IsEditable() == false)
								continue;
							else if (Filter.PassFilter(Name) == false && Filter.PassFilter(Desc) == false)
								continue;

							char Buffer[0x100] = { 0 };
							FORMAT_STR(Buffer, "%s %s-%d", Name, Desc, i);
							bool Selected = false;

							ImGui::PushID(Buffer);
							{
								if (ImGui::Selectable(Name, false,
													  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups) &&
									ImGui::IsMouseDoubleClicked(0))
								{
									Selection = Itr.get();
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
							ModalWindowProviderOSDLayer::Instance.ShowModal("Edit Binding",
																			std::bind(&RenderWindowKeyboardManager::RenderModalBindingEditor, this,
																					  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
																			Selection,
																			ImGuiWindowFlags_AlwaysAutoResize);
						}
					}
					ImGui::Columns();
				}
				ImGui::EndChild();

				if (ImGui::Button("Close", ImVec2(75, 0)))
				{
					Filter.Clear();
					return true;
				}
				else
					return false;
			}

			bool RenderWindowKeyboardManager::RenderModalBindingEditor(RenderWindowOSD* OSD, ImGuiDX9* GUI, void* UserData)
			{
				static SHORT HotKeyBuffer = NULL;
				static bool Shift = false, Control = false, Alt = false;
				static bool DoInit = false;

				IHotKey* CurrentHotKey = (IHotKey*)UserData;
				bool HoldableKey = CurrentHotKey->GetHandlerType() == IKeyboardEventHandler::kType_Stateful;

				if (DoInit == false)
				{
					DoInit = true;

					HotKeyBuffer = NULL;
					Shift = CurrentHotKey->GetActiveBinding().HasShift();
					Control = CurrentHotKey->GetActiveBinding().HasControl();
					Alt = CurrentHotKey->GetActiveBinding().HasAlt();
				}

				SHORT FirstKeyDown = NULL;
				bool ClearBuffer = false;
				for (int i = NULL; i < 256; i++)
				{
					bool Skip = false;
					switch (i)
					{
					case VK_ESCAPE:
						if (ImGui::IsKeyPressed(i) || ImGui::IsKeyReleased(i))
						{
							ClearBuffer = true;
							Skip = true;
						}

						break;
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
						if (HoldableKey == false)
							Skip = true;		// use modifiers only when mapping holdable keys
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

				BasicKeyBinding NewBinding;
				bool InvalidNewHoldableKey = false;
				if (HotKeyBuffer)
				{
					if (HoldableKey == false)
						NewBinding.Set(HotKeyBuffer, Control, Shift, Alt);
					else
					{
						NewBinding.Set(HotKeyBuffer, NULL);
						switch (HotKeyBuffer)
						{
						case VK_CONTROL:
						case VK_MENU:
						case VK_LCONTROL:
						case VK_RCONTROL:
						case VK_LMENU:
						case VK_RMENU:
							InvalidNewHoldableKey = true;
							break;
						}
					}
				}
				const ImVec4 RedColor = ImColor::HSV(0, 0.8f, 0.8f);
				const ImVec4 YellowColor = ImColor::HSV(1 / 7.f, 0.8f, 0.8f);
				bool KeyConflict = false;
				IHotKey* Existing = nullptr;
				IHotKey* StatelessBroadMatch = nullptr;
				if (NewBinding.IsValid())
				{
					Existing = LookupHotKey(NewBinding, CurrentHotKey->GetHandlerType(), CurrentHotKey->GetExecutionContext());
					if (HoldableKey)
						StatelessBroadMatch = LookupHotKey(NewBinding, IKeyboardEventHandler::kType_Stateless, ExecutionContext(ExecutionContext::kMode_All), true);
				}
				if (Existing && Existing != CurrentHotKey)
					KeyConflict = true;

				ImGui::Text("Press the key combination to update the binding for '%s'", CurrentHotKey->GetName());
				ImGui::Separator();
				ImGui::Text("Current Binding: %s", CurrentHotKey->GetActiveBinding().GetDescription().c_str());
				if (NewBinding.IsValid() == false)
					ImGui::TextColored(RedColor, "New Binding: Invalid");
				else
				{
					if (HoldableKey && InvalidNewHoldableKey)
						ImGui::TextColored(RedColor, "New Binding: %s (Invalid key)", NewBinding.GetDescription().c_str());
					else if (KeyConflict)
						ImGui::TextColored(RedColor, "New Binding: %s (Conflicts with '%s')", NewBinding.GetDescription().c_str(), Existing->GetName());
					else if (StatelessBroadMatch)
						ImGui::TextColored(YellowColor, "New Binding: %s (Overlaps with '%s')", NewBinding.GetDescription().c_str(), StatelessBroadMatch->GetName());
					else
						ImGui::Text("New Binding: %s", NewBinding.GetDescription().c_str());
				}

				if (HoldableKey == false)
				{
					ImGui::Columns(3, "modifiers", false);
					{
						ImGui::Checkbox("Control", &Control); ImGui::NextColumn();
						ImGui::Checkbox("Shift", &Shift); ImGui::NextColumn();
						ImGui::Checkbox("Alt", &Alt); ImGui::NextColumn();
					}
					ImGui::Columns();
				}
				ImGui::Separator();

				bool Close = false;
				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					if (KeyConflict == false)
					{
						if (NewBinding != CurrentHotKey->GetActiveBinding())
						{
							CurrentHotKey->SetActiveBinding(NewBinding);
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

			void RenderWindowKeyboardManager::PerformConsistencyChecks(UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				// ensure that the holdable override base states correspond to the active state of the override key
				for (auto& Itr : HotKeys)
				{
					if (Itr->GetHandlerType() == IKeyboardEventHandler::kType_Stateful)
					{
						HoldableKeyOverride* Override = dynamic_cast<HoldableKeyOverride*>(Itr.get());
						if (Override == nullptr)
							continue;

						UInt8* BaseState = Override->GetBaseState();
						SHORT BuiltInKey = Override->GetBuiltInKey();
						if (Override->IsHeldDown())
						{
							if (*BaseState == 0)
							{
								// this ought never happen (except when triggering the MMB and Space at the same time)
								// however, GetAsyncKeyState might return true if the key was pressed sometime after its previous call
								// so we just ignore this eventuality
							}
						}
						else
						{
							if (*BaseState == 1)
							{
								// special case for Space as the base state is set when holding down the middle mouse button too
								if (BuiltInKey != BuiltIn::kHoldable_Space)
								{
#ifndef NDEBUG
									BGSEECONSOLE->PrintToMessageLogContext(MessageLogContext, false, "Reset Holdable Key: Name: '%s' | %s",
																		   Itr->GetName(),
																		   Itr->GetActiveBinding().GetDescription().c_str());
#endif
									// reset the base state
									*BaseState = 0;
								}
								else if (GetAsyncKeyState(VK_MBUTTON) == NULL)
								{
									// MMB isn't down either, reset
									*BaseState = 0;
								}

							}
						}
					}
				}
			}

			RenderWindowKeyboardManager::RenderWindowKeyboardManager() :
				HotKeys(),
				DeletedBindings(),
				Shared(),
				MessageLogContext(nullptr)
			{
				Initialized = false;
			}

			RenderWindowKeyboardManager::~RenderWindowKeyboardManager()
			{
				HotKeys.clear();
			}

			void RenderWindowKeyboardManager::Initialize()
			{
				SME_ASSERT(Initialized == false);
#ifndef NDEBUG
				MessageLogContext = BGSEECONSOLE->RegisterMessageLogContext("Render Window Keyboard Input");
#endif
				DeletedBindings.push_back(BuiltIn::KeyBinding('O'));
				DeletedBindings.push_back(BuiltIn::KeyBinding('I', BuiltIn::kModifier_Space));

				RegisterHoldableOverride("D17061A6-7FD7-4E79-BCCB-AC4899A14B47", BuiltIn::kHoldable_Control, false);		// not editable as it just functions like a regular modifier
				Shared.RotateCamera = RegisterHoldableOverride("206281B3-6694-4C99-9AA2-E1B94720C9BC", BuiltIn::kHoldable_Shift, true);
				Shared.PanCamera = RegisterHoldableOverride("4C52B079-91D6-4BA0-854B-B7028A293C49", BuiltIn::kHoldable_Space, true);
				RegisterHoldableOverride("76C93A6E-2777-42DB-B80B-82C0A0578E79", BuiltIn::kHoldable_X, true);
				RegisterHoldableOverride("1CAF7AAF-4E5E-44DE-9877-0428040829E2", BuiltIn::kHoldable_Y, true);
				RegisterHoldableOverride("8A16BDBA-709B-4464-8B01-5748C0392635", BuiltIn::kHoldable_Z, true);
				RegisterHoldableOverride("123363FB-D801-400F-B8B5-3CA8F68A2797", BuiltIn::kHoldable_S, true);
				Shared.ZoomCamera = RegisterHoldableOverride("5B69B450-F18B-467A-8AF7-5615F835F265", BuiltIn::kHoldable_V, true);

				RegisterComboKeyOverride("141C71A5-CF32-4C83-9F30-F87B082AB078", actions::builtIn::ReloadAllPathGrids);
				RegisterComboKeyOverride("127C4534-8779-4640-8EF3-D9FF83D5C19C", actions::builtIn::GenerateActiveExteriorLocalMap);
				RegisterComboKeyOverride("0E1CC780-527D-4483-8A39-7788EA1BA10E", actions::builtIn::ToggleBrightLight);
				RegisterComboKeyOverride("F7EC3361-AA15-480F-BD10-5678F7A9C842", actions::builtIn::ToggleLandscapeEditMode);
				RegisterComboKeyOverride("27E4F082-1C7C-4BCD-873C-3664A75825A3", actions::builtIn::ToggleCollisionGeom);
				RegisterComboKeyOverride("0ACC2CA5-D63A-46CE-888D-1697F6DDA0DD", actions::builtIn::RefreshRenderWindow);
				RegisterComboKeyOverride("2433E786-F83B-4E78-8001-EB82637C42CA", actions::builtIn::MoveCamYAxisPos);
				RegisterComboKeyOverride("5505FE39-7033-4EA0-BC88-65E0FDC9DB59", actions::builtIn::MoveCamYAxisNeg);
				RegisterComboKeyOverride("E1560D3D-6B7E-40D8-A056-205BCA4D9539", actions::builtIn::MoveCamXAxisPos);
				RegisterComboKeyOverride("D848842F-FAA0-4854-A053-8BFCFBF807EB", actions::builtIn::MoveCamXAxisNeg);
				RegisterComboKeyOverride("A8F4891C-96ED-4E6B-A0F5-1E2CF855DAD8", actions::builtIn::PitchCamPos);
				RegisterComboKeyOverride("BBA51901-A005-4244-BF87-A30B14C0D5D2", actions::builtIn::PitchCamNeg);
				RegisterComboKeyOverride("71C74CD9-9A9A-4856-9BA7-DAEDB13F4BD7", actions::builtIn::YawCamPos);
				RegisterComboKeyOverride("E7A02E6D-1F94-416A-A5D3-3762014EF21D", actions::builtIn::YawCamNeg);
				RegisterComboKeyOverride("63AFCE02-C30E-45A2-AA13-A74033857427", actions::builtIn::RollCamPos);
				RegisterComboKeyOverride("81D9BC2B-2EB6-41FD-9718-9DC2D2D3B4E3", actions::builtIn::RollCamNeg);
				RegisterComboKeyOverride("43A2842C-7A49-4518-BD91-CCCB03419041", actions::builtIn::ShowLandUseDialog);
				RegisterComboKeyOverride("82E80B48-BD5D-4360-80A2-56277E847563", actions::builtIn::ShowSelectedRefInfo, BasicKeyBinding('R', BasicKeyBinding::kModifier_CTRL_SHIFT_ALT));
				RegisterComboKeyOverride("427EB49C-04DE-4255-B67F-B6DA7999F112", actions::builtIn::LinkPathGridSelection);
				RegisterComboKeyOverride("563A59A2-D079-4CB2-ABE5-9DC39017A974", actions::builtIn::ToggleMarkers);
				RegisterComboKeyOverride("D9C00DC1-32E8-481E-A616-29F348D96339", actions::builtIn::ToggleWater);
				RegisterComboKeyOverride("0E1882E2-F867-4CEF-886E-C783BBF9A6E3", actions::builtIn::ToggleGlobalWireframe);
				RegisterComboKeyOverride("46E3CA09-BC55-4CC0-B7AF-41F4510111E7", actions::builtIn::ToggleLandscape);
				RegisterComboKeyOverride("95A62CEA-D549-42AA-96A4-ED59F143747F", actions::builtIn::ToggleLightRadius);
				RegisterComboKeyOverride("6E4F8B1B-F056-4022-9FD9-176A07134B4D", actions::builtIn::Fall);
				RegisterComboKeyOverride("BA518E5E-8CAB-439C-9984-F82C1BF47D17", actions::builtIn::SavePlugin);
				RegisterComboKeyOverride("98B7AFF2-AF80-4FE6-BC80-8AB74EF7627E", actions::builtIn::Cut);
				RegisterComboKeyOverride("D94078C0-EC4A-4FD1-AFAA-B685A1FD80E0", actions::builtIn::Redo);
				RegisterComboKeyOverride("69242E32-3DCC-4585-B3FE-8E060302873C", actions::builtIn::Undo);
				RegisterComboKeyOverride("9610ACA9-48AB-4298-B412-1CF95D0839B3", actions::builtIn::UndoEx);
				RegisterComboKeyOverride("4DEDA44A-881C-4F9E-807D-B95E91F7F600", actions::builtIn::CullLightSelection);
				RegisterComboKeyOverride("C338CF36-B724-43E5-B000-AF042FC86180", actions::builtIn::ToggleObjects);
				RegisterComboKeyOverride("DEE22990-E33B-4002-9EBC-8E84342ECB8A", actions::builtIn::AutoLinkPathGridSelection);
				RegisterComboKeyOverride("43DEAA35-D3D7-4099-9CD6-AC44A23E48D8", actions::builtIn::Copy);
				RegisterComboKeyOverride("A929F12D-D0AA-42A9-9B3C-5A3C3F3F685C", actions::builtIn::FallCam, BasicKeyBinding('C', BasicKeyBinding::kModifier_SHIFT_ALT));
				RegisterComboKeyOverride("41ABFF79-BC52-4D55-9DCC-168F738617D2", actions::builtIn::CenterCam);
				RegisterComboKeyOverride("680AD60E-3632-47BE-A190-9502C445C60F", actions::builtIn::ToggleVertexColoring);
				RegisterComboKeyOverride("74F30579-C528-4AC4-8876-E424B01AC945", actions::builtIn::TopCamera);
				RegisterComboKeyOverride("FE9B55E7-860F-42DE-9605-5E2532312BDA", actions::builtIn::Duplicate);
				RegisterComboKeyOverride("8DADBA23-89AE-4B15-9A6C-5AD9489B5C40", actions::builtIn::DeselectAll);
				RegisterComboKeyOverride("719C3CCA-DEB7-4627-A437-9B99E592A86A", actions::builtIn::RunHavokSim);
				RegisterComboKeyOverride("87F57D0A-26D4-4785-8B53-FB84D4A2FBDD", actions::builtIn::Delete);
				RegisterComboKeyOverride("B519BADD-0127-4DCE-9557-ADE428BE7F62", actions::builtIn::ToggleSelectionWireframe);
				RegisterComboKeyOverride("8014F8CD-6B41-413D-803F-8B22BE6EF412", actions::builtIn::CheckBounds);
				RegisterComboKeyOverride("B2AA4948-41C0-45A3-A1B3-8AFBDC55D459", actions::builtIn::ToggleCellBorders);
				RegisterComboKeyOverride("B292405B-F209-49A2-9017-ED2AC49DE5BD", actions::builtIn::Paste);
				RegisterComboKeyOverride("4B73B74C-7015-44A8-8C2C-105B5A67272C", actions::builtIn::PasteInPlace);

				RegisterActionableKeyHandler("6EC6F5E2-AE52-4475-AC27-5418375B5FB8", actions::InvertSelection, BasicKeyBinding('D', BasicKeyBinding::kModifier_CTRL_SHIFT));
				RegisterActionableKeyHandler("F2BF346D-EC8F-4A33-975D-0E7A9C7424EE", actions::SelectAll, BasicKeyBinding('A', BasicKeyBinding::kModifier_Control));
				RegisterActionableKeyHandler("CADFE174-ED60-4B9A-8562-34ED58DB2882", actions::AlignPosition, BasicKeyBinding('X', BasicKeyBinding::kModifier_Shift));
				RegisterActionableKeyHandler("82E93C1E-CAEE-4E27-8B3F-7719F7E980D6", actions::AlignRotation, BasicKeyBinding('Z', BasicKeyBinding::kModifier_Shift));
				RegisterActionableKeyHandler("A1FE6205-5F69-4808-BD4B-F0D297AA215C", actions::GroupSelection, BasicKeyBinding('G', BasicKeyBinding::kModifier_Control));
				RegisterActionableKeyHandler("C028EA1C-76CF-4984-A820-1240E9001679", actions::UngroupSelection, BasicKeyBinding('U', BasicKeyBinding::kModifier_Control));
				RegisterActionableKeyHandler("2C7E9F71-E42E-4455-8069-46EAD79541D1", actions::OrphanizeSelection, BasicKeyBinding('U', BasicKeyBinding::kModifier_CTRL_ALT));
				RegisterActionableKeyHandler("30A153E0-6E49-42C0-8346-3F0EE5DDEE75", actions::FreezeSelection, BasicKeyBinding('W', BasicKeyBinding::kModifier_Control));
				RegisterActionableKeyHandler("24780B09-E7DB-4020-A3D2-4656465CC828", actions::ThawSelection, BasicKeyBinding('E', BasicKeyBinding::kModifier_Control));
				RegisterActionableKeyHandler("44431BAA-E7C9-4D36-93E8-8DE5A7F5A498", actions::ThawAll, BasicKeyBinding(VK_F2, NULL));
				RegisterActionableKeyHandler("0C513C52-86FE-45D5-87AF-461EC2A9FA1B", actions::ToggleFreezeInactive, BasicKeyBinding('E', BasicKeyBinding::kModifier_CTRL_SHIFT));
				RegisterActionableKeyHandler("765B64A2-572D-4D9B-BBDC-197225E6E76C", actions::ToggleSelectionVisibility, BasicKeyBinding('2', NULL));
				RegisterActionableKeyHandler("C6D5E817-81C1-415A-ADDE-6A7CD41C12A0", actions::ToggleSelectionChildrenVisibility, BasicKeyBinding('3', NULL));
				RegisterActionableKeyHandler("F54BA759-71C1-4E09-ADD7-EA9D88D63CBB", actions::RevealAll, BasicKeyBinding(VK_F3, NULL));
				RegisterActionableKeyHandler("9726EE0A-C9C1-4BCA-8B2E-3B72BDBB513D", actions::DimSelectionOpacity, BasicKeyBinding(VK_OEM_MINUS, NULL));
				RegisterActionableKeyHandler("247D9B08-3BFE-4E1C-B268-AED18D4B633B", actions::ResetSelectionOpacity, BasicKeyBinding(VK_OEM_PLUS, NULL));
				RegisterActionableKeyHandler("6D68753D-AEE1-4573-9B8A-4A9E4B17497C", actions::UnlinkPathGridSelection, BasicKeyBinding('R', BasicKeyBinding::kModifier_Shift));
				RegisterActionableKeyHandler("F42F86FA-8BE7-4CC8-A6BA-779A963EEF89", actions::ShowBatchEditor, BasicKeyBinding(VK_OEM_PERIOD, NULL));
				RegisterActionableKeyHandler("D233EB54-BED6-4720-96A6-86442A97E6A6", actions::ShowUseInfo, BasicKeyBinding(VK_F1, NULL));
				RegisterActionableKeyHandler("F11C7C9E-CF74-4C2D-877D-39427F7C81CD", actions::ShowSearchReplace, BasicKeyBinding('F', BasicKeyBinding::kModifier_CTRL_SHIFT));
				RegisterActionableKeyHandler("0A6D7417-7C81-4479-B977-E3D32EE485DF", actions::ToggleStaticCameraPivot, BasicKeyBinding('T', BasicKeyBinding::kModifier_Shift));
				RegisterActionableKeyHandler("BBA57061-EE4C-4C0E-8598-C7CE0706E957", actions::ToggleAuxViewport, BasicKeyBinding(VK_OEM_3, BasicKeyBinding::kModifier_CTRL_SHIFT));
				RegisterActionableKeyHandler("2C35CC8B-2CA1-4867-92FD-A64ED6DE1148", actions::ToggleAlternateMovementSettings, BasicKeyBinding('Q', BasicKeyBinding::kModifier_Shift));
				RegisterActionableKeyHandler("E2240B02-1D0B-4885-931F-137DCD6E6D90", actions::TogglePathGridEditMode, BasicKeyBinding('G', NULL));
				RegisterActionableKeyHandler("1A6CD699-8265-48BB-964F-8F6CF55E0103", actions::ToggleSnapToGrid, BasicKeyBinding('S', BasicKeyBinding::kModifier_Shift));
				RegisterActionableKeyHandler("DECA2E00-6FB7-4638-8563-108326891DEA", actions::ToggleSnapToAngle, BasicKeyBinding('D', BasicKeyBinding::kModifier_Shift));
				RegisterActionableKeyHandler("F5862ECB-7413-4C56-A5AF-E5EED401FC17", actions::FocusOnRefFilter, BasicKeyBinding('F', BasicKeyBinding::kModifier_Control));
				RegisterActionableKeyHandler("959D48C0-43FE-47B5-BD61-9D78EEA277FC", actions::JumpToExteriorCell, BasicKeyBinding('J', BasicKeyBinding::kModifier_Control));

				Shared.MoveCameraWithSelection = RegisterHoldableHandler("5AE9F3BA-A336-450C-89B5-C278294A97C1",
																		 "Move Camera With References",
																		 "",
																		 BasicKeyBinding('N'));

				LoadFromINI();
				Initialized = true;
			}

			void RenderWindowKeyboardManager::Deinitialize()
			{
				SME_ASSERT(Initialized);

				SaveToINI();
#ifndef NDEBUG
				BGSEECONSOLE->UnregisterMessageLogContext(MessageLogContext);
				MessageLogContext = nullptr;
#endif
				Initialized = false;
			}

			bool RenderWindowKeyboardManager::HandleInput(HWND, UINT uMsg, WPARAM wParam, LPARAM lParam, RenderWindowManager*)
			{
				bool ConsumeMessage = false;
				const char* MessageName = nullptr;

				switch (uMsg)
				{
				case WM_SYSKEYDOWN:
					MessageName = "WM_SYSKEYDOWN";
					break;
				case WM_SYSKEYUP:
					MessageName = "WM_SYSKEYUP";
					break;
				case WM_KEYDOWN:
					MessageName = "WM_KEYDOWN";
					break;
				case WM_KEYUP:
					MessageName = "WM_KEYUP";
					break;
				}

				switch (uMsg)
				{
				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:
				case WM_KEYDOWN:
				case WM_KEYUP:
					{
						// execute stateful handlers first
						for (auto& Itr : HotKeys)
						{
							if (Itr->GetHandlerType() == IKeyboardEventHandler::kType_Stateful)
							{
								IKeyboardEventHandler::EventResult Output = Itr->HandleActive(uMsg, wParam, lParam);
								// holdable key events aren't restricted by context
								SME_ASSERT(Output.InvalidContext == false);
								if (Output.Triggered)
								{
#ifndef NDEBUG
									BGSEECONSOLE->PrintToMessageLogContext(MessageLogContext, false, "Handled Stateful Event (MSG=%s): Name: '%s' | %s",
														 MessageName,
														 Itr->GetName(),
														 Itr->GetActiveBinding().GetDescription().c_str());
#endif
									// activation implictly means success
									SME_ASSERT(Output.Success);
									ConsumeMessage = true;
									break;
								}
							}
						}

						// execute stateless handlers next
						IKeyboardEventHandler::EventResult StatelessResult;
						IHotKey* InvalidContext = nullptr;
						for (auto& Itr : HotKeys)
						{
							if (Itr->GetHandlerType() == IKeyboardEventHandler::kType_Stateless)
							{
								IKeyboardEventHandler::EventResult Output = Itr->HandleActive(uMsg, wParam, lParam);
								if (Output.Triggered)
								{
									StatelessResult.Triggered = true;
									ConsumeMessage = true;
									if (Output.Success)
									{
#ifndef NDEBUG
										BGSEECONSOLE->PrintToMessageLogContext(MessageLogContext, false, "Handled Stateless Event (MSG=%s): Name: '%s' | %s",
															 MessageName,
															 Itr->GetName(),
															 Itr->GetActiveBinding().GetDescription().c_str());
#endif
										StatelessResult.Success = true;
										break;
									}
									else if (Output.InvalidContext)
									{
										StatelessResult.InvalidContext = true;
										InvalidContext = Itr.get();
									}

								}
							}
						}

						static IHotKey* LastInvalidContext = nullptr;
						if (StatelessResult.Triggered && StatelessResult.Success == false && StatelessResult.InvalidContext)
						{
							if (InvalidContext != LastInvalidContext)
							{
								LastInvalidContext = InvalidContext;
								if (settings::renderer::kNotifyOnInvalidExecutionContext().i)
								{
									NotificationOSDLayer::Instance.ShowNotification("'%s' cannot be performed in the current edit mode.",
																					LastInvalidContext->GetName());
								}
							}
						}

						// consume built-in events
						for (auto& Itr : HotKeys)
						{
							IKeyboardEventHandler::EventResult Output = Itr->HandleBuiltIn(uMsg, wParam, lParam);
							if (Output.Triggered)
							{
#ifndef NDEBUG
								BGSEECONSOLE->PrintToMessageLogContext(MessageLogContext, false, "Consumed Built-In Event (MSG=%s): Name: '%s' | %s",
													 MessageName,
													 Itr->GetName(),
													 Itr->GetActiveBinding().GetDescription().c_str());
#endif
								SME_ASSERT(Output.Success);
								ConsumeMessage = true;
								break;;
							}
						}

						// finally, consume deleted bindings
						if (ConsumeMessage == false)
						{
							for (auto& Itr : DeletedBindings)
							{
								if (Itr.IsActivated(wParam))
								{
									ConsumeMessage = true;
									break;
								}
							}
						}
					}

					break;
				}

				// performed at the end as potential KEYDOWN events need to be handled beforehand
				PerformConsistencyChecks(uMsg, wParam, lParam);
				return ConsumeMessage;
			}

			void RenderWindowKeyboardManager::ShowHotKeyEditor()
			{
				ModalWindowProviderOSDLayer::Instance.ShowModal("Render Window Keyboard Bindings",
																std::bind(&RenderWindowKeyboardManager::RenderModalHotKeyEditor, this, std::placeholders::_1, std::placeholders::_2),
																nullptr,
																ImGuiWindowFlags_NoResize, ImVec2(700, 600), ImGuiSetCond_Once);
			}


			SharedBindings& RenderWindowKeyboardManager::GetSharedBindings()
			{
				return Shared;
			}

			POINT RenderWindowMouseManager::CenterCursor(HWND hWnd, bool UpdateBaseCoords)
			{
				int X, Y, W, H;
				GetWindowMetrics(hWnd, X, Y, W, H);

				POINT Center = { X + W / 2, Y + H / 2 };
				POINT Out(Center);
				SetCursorPos(Center.x, Center.y);

				if (UpdateBaseCoords)
				{
					ScreenToClient(hWnd, &Center);
					TESRenderWindow::LastMouseCoords->x = Center.x;
					TESRenderWindow::LastMouseCoords->y = Center.y;
				}

				return Out;
			}

			bool RenderWindowMouseManager::IsCenteringCursor(HWND hWnd, LPARAM lParam) const
			{
				// ### HACK, kludge to workaround the WM_MOUSEMOVE feedback loop we get when allowing free movement
				int X, Y, W, H;
				GetWindowMetrics(hWnd, X, Y, W, H);

				POINT Center = { X + W / 2, Y + H / 2 };
				ScreenToClient(hWnd, &Center);

				int PosX = GET_X_LPARAM(lParam);
				int PosY = GET_Y_LPARAM(lParam);

				if (PosX == Center.x && PosY == Center.y)
					return true;
				else
					return false;
			}

			void RenderWindowMouseManager::GetWindowMetrics(HWND hWnd, int& X, int& Y, int& Width, int& Height) const
			{
				RECT WindowRect = { 0 };
				GetWindowRect(hWnd, &WindowRect);

				Width = WindowRect.right - WindowRect.left;
				Height = WindowRect.bottom - WindowRect.top;

				X = WindowRect.left;
				Y = WindowRect.top;
			}

			void RenderWindowMouseManager::ToggleCellViewUpdate(bool State)
			{
				if (*TESRenderWindow::ActiveCell && (*TESRenderWindow::ActiveCell)->IsInterior() == false)
				{
					if (CellViewUpdatesDeferred == false && State == false)
					{
						// defer updating the cell view in exterior worldspaces until we release captue
						hooks::_MemHdlr(CellViewSetCurrentCell).WriteUInt8(0xC3);		// write an immediate retn
						CellViewUpdatesDeferred = true;
					}
					else if (CellViewUpdatesDeferred && State)
					{
						hooks::_MemHdlr(CellViewSetCurrentCell).WriteBuffer();			// write original instruction
						CellViewUpdatesDeferred = false;

						// update the cell view to the current renderwindow cell
						Vector3 PosCoord;
						PosCoord.x = ((*TESRenderWindow::ActiveCell)->cellData.coords->x << 12) + 2048;
						PosCoord.y = ((*TESRenderWindow::ActiveCell)->cellData.coords->y << 12) + 2048;
						TESCellViewWindow::SetCurrentCell(&PosCoord, true);
					}
				}
			}

			void RenderWindowMouseManager::ToggleFreeMouseMovement(HWND hWnd, bool State)
			{
				if (settings::renderer::kUnrestrictedMouseMovement().i == 0)
					return;

				if (State)
				{
					if (FreeMouseMovement == false)
					{
						GetCursorPos(&MouseDownCursorPos);
						// hide the cursor and reset it to the center
						FreeMouseMovement = true;
						CenterCursor(hWnd, true);
						while (ShowCursor(FALSE) > 0)
							;//
					}
				}
				else
				{
					if (FreeMouseMovement)
					{
						// restore the cursor
						FreeMouseMovement = false;
						SetCursorPos(MouseDownCursorPos.x, MouseDownCursorPos.y);
						while (ShowCursor(TRUE) < 0)
							;//
					}
				}
			}

			void RenderWindowMouseManager::HandleFreeMouseMovementKeyEvent(UInt8 Type)
			{
				ToggleFreeMouseMovement(*TESRenderWindow::WindowHandle, Type == HoldableKeyHandler::kEvent_KeyDown);
				ToggleCellViewUpdate(Type != HoldableKeyHandler::kEvent_KeyDown);
			}

			RenderWindowMouseManager::RenderWindowMouseManager() :
				CurrentMouseCoord{ 0 },
				PaintingSelection(false),
				SelectionPaintingMode(kSelectionPainting_NotSet),
				MouseDownCursorPos{ 0 },
				FreeMouseMovement(false),
				CellViewUpdatesDeferred(false)
			{
				;//
			}

			void RenderWindowMouseManager::Initialize(SharedBindings& Shared)
			{
				// register free mouse movement event listeners
				HoldableKeyHandler::HoldableKeyEventDelegateT Listener(std::bind(&RenderWindowMouseManager::HandleFreeMouseMovementKeyEvent, this, std::placeholders::_1));
				Shared.ZoomCamera->RegisterListener(Listener);
				Shared.PanCamera->RegisterListener(Listener);
				Shared.RotateCamera->RegisterListener(Listener);
			}

			void RenderWindowMouseManager::Deinitialize()
			{
				;//
			}

			bool RenderWindowMouseManager::HandleInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, RenderWindowManager* Manager)
			{
				bool Handled = false;

				switch (uMsg)
				{
				case WM_LBUTTONDBLCLK:
					{
						if (*TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0)
						{
							TESObjectREFR* Ref = TESRender::PickRefAtCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
							if (Ref)
							{
								if (ReferenceSelectionManager::IsSelectable(Ref, PaintingSelection) == false)
								{
									// preempt the vanilla handler
									Handled = true;
								}
							}
						}
					}

					break;
				case WM_SETCURSOR:
					{
						if (GetCapture() != hWnd)
						{
							HCURSOR Icon = *TESRenderWindow::CursorArrow;

							if (Manager->GetOSD()->NeedsInput() == false)
							{
								if (*TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0)
								{
									TESObjectREFR* MouseRef = _RENDERWIN_XSTATE.CurrentMouseRef;
									if (MouseRef)
									{
										UInt32 SelectionReason = 0;
										if (ReferenceSelectionManager::IsSelectable(MouseRef, SelectionReason, PaintingSelection))
										{
											if (_RENDERSEL->HasObject(MouseRef))
												Icon = *TESRenderWindow::CursorMove;
											else
												Icon = *TESRenderWindow::CursorSelect;
										}
										else if ((SelectionReason & (ReferenceSelectionManager::kReason_FrozenInactive |
																	 ReferenceSelectionManager::kReason_FrozenSelf |
																	 ReferenceSelectionManager::kReason_ParentLayerFrozen)) != NULL)
										{
											Icon = LoadCursor(nullptr, IDC_NO);
										}
									}
								}

								if (Icon != *TESRenderWindow::CursorArrow)
								{
									HCURSOR CurrentCursor = GetCursor();
									if (Icon && CurrentCursor != Icon)
										SetCursor(Icon);

									Handled = true;
								}
							}
						}
					}

					break;
				case WM_MOUSELEAVE:
				case WM_NCMOUSELEAVE:
					_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
					_RENDERWIN_XSTATE.CurrentMousePathGridPoint = nullptr;

					break;
				case WM_MOUSEMOVE:
					{
						Handled = true;

						POINT MousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
						POINT MouseDelta = { CurrentMouseCoord.x - MousePos.x, CurrentMouseCoord.y - MousePos.y };
						POINT LastMouseCoord(CurrentMouseCoord);
						CurrentMouseCoord.x = MousePos.x;
						CurrentMouseCoord.y = MousePos.y;

						if (FreeMouseMovement && IsCenteringCursor(hWnd, lParam))
							break;

						_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
						_RENDERWIN_XSTATE.CurrentMousePathGridPoint = nullptr;

						if (*TESRenderWindow::LandscapeEditFlag == 0)
						{
							if (*TESRenderWindow::PathGridEditFlag == 0)
							{
								_RENDERWIN_XSTATE.CurrentMouseRef = TESRender::PickRefAtCoords(MousePos.x, MousePos.y);
								if (_RENDERWIN_XSTATE.CurrentMouseRef)
								{
									if (ReferenceVisibilityManager::IsCulled(_RENDERWIN_XSTATE.CurrentMouseRef) ||
										ReferenceVisibilityManager::ShouldBeInvisible(_RENDERWIN_XSTATE.CurrentMouseRef))
									{
										_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
									}
								}
							}
							else
							{
								_RENDERWIN_XSTATE.CurrentMousePathGridPoint = TESRender::PickPathGridPointAtCoords(MousePos.x,
																												   MousePos.y);
							}
						}

						if (PaintingSelection)
						{
							// paint only when the alt key is held down
							if (GetAsyncKeyState(VK_MENU))
							{
								TESObjectREFR* MouseRef = _RENDERWIN_XSTATE.CurrentMouseRef;
								if (MouseRef)
								{
									if (SelectionPaintingMode == kSelectionPainting_NotSet)
									{
										if (_RENDERSEL->HasObject(MouseRef))
											SelectionPaintingMode = kSelectionPainting_Deselect;
										else
											SelectionPaintingMode = kSelectionPainting_Select;
									}

									if (SelectionPaintingMode == kSelectionPainting_Select)
										ReferenceSelectionManager::AddToSelection(MouseRef, true, PaintingSelection);
									else
										ReferenceSelectionManager::RemoveFromSelection(MouseRef, true);
								}
							}

							break;
						}

						// handle free movement
						if (FreeMouseMovement)
						{
							// center the mouse and tunnel just the offset
							CenterCursor(hWnd, false);

							POINT ClientPos;
							ClientPos.x = TESRenderWindow::LastMouseCoords->x - MouseDelta.x;
							ClientPos.y = TESRenderWindow::LastMouseCoords->y - MouseDelta.y;
							lParam = MAKELPARAM(ClientPos.x, ClientPos.y);
						}

						bool MoveCameraWithSel = Manager->GetKeyboardInputManager()->GetSharedBindings().MoveCameraWithSelection->IsHeldDown();
						Vector3 PrePivot, PostPivot;
						if (MoveCameraWithSel)
						{
							if (_RENDERWIN_XSTATE.DraggingPathGridPoints)
								TESRenderWindow::CalculatePathGridPointPositionVectorSum(PrePivot);
							else
							{
								_RENDERSEL->CalculatePositionVectorSum();
								PrePivot = _RENDERSEL->selectionPositionVectorSum;
							}
						}

						// tunnel the message
						BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, wParam, lParam);

						if (MoveCameraWithSel)
						{
							if (_RENDERWIN_XSTATE.DraggingPathGridPoints)
								TESRenderWindow::CalculatePathGridPointPositionVectorSum(PostPivot);
							else
							{
								_RENDERSEL->CalculatePositionVectorSum();
								PostPivot = _RENDERSEL->selectionPositionVectorSum;
							}

							Vector3 Diff = PostPivot - PrePivot;
							Vector3* CameraRootLocalTranslate = (Vector3*)&_PRIMARYRENDERER->primaryCameraParentNode->m_localTranslate;
							*CameraRootLocalTranslate += Diff;
						}
					}

					break;
				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
					Handled = true;

					if (uMsg == WM_RBUTTONDOWN)
					{
						if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL))
						{
							// handle it for the button up event, early out
							break;
						}
					}

					if (uMsg == WM_LBUTTONDOWN)
					{
						if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL) &&
							*TESRenderWindow::LandscapeEditFlag == 0 && *TESRenderWindow::PathGridEditFlag == 0)
						{
							SME_ASSERT(PaintingSelection == false && SelectionPaintingMode == kSelectionPainting_NotSet);

							PaintingSelection = true;
							NotificationOSDLayer::Instance.ShowNotification("Painting reference selection...");

							SetCapture(hWnd);
							break;
						}
					}

					// tunnel the message to the original proc and check if we need to allow free mouse movement
					BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, wParam, lParam);

					_RENDERWIN_XSTATE.DraggingPathGridPoints = *TESRenderWindow::PathGridEditFlag &&
																_RENDERWIN_XSTATE.CurrentMousePathGridPoint &&
																GetCapture() == hWnd &&
																uMsg == WM_LBUTTONDOWN;
					if (*TESRenderWindow::DraggingSelection || *TESRenderWindow::RotatingSelection || _RENDERWIN_XSTATE.DraggingPathGridPoints)
					{
						// landscape edit mode isn't supported as the land coords are calculated from the mouse coords, not their offset
						SME_ASSERT(*TESRenderWindow::LandscapeEditFlag == 0);
						if (GetCapture() == hWnd)
						{
							// begin free movement handling
							ToggleFreeMouseMovement(hWnd, true);
							ToggleCellViewUpdate(false);
						}
					}
					else if (*TESRenderWindow::LandscapeEditFlag)
						ToggleCellViewUpdate(false);

					break;
				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
					Handled = true;

					if (uMsg == WM_RBUTTONUP)
					{
						if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL))
						{
							// place palette object, if any
							objectPalette::ObjectPaletteManager::Instance.PlaceObject(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
							break;
						}
					}

					if (uMsg == WM_LBUTTONUP)
					{
						if (PaintingSelection)
						{
							PaintingSelection = false;
							SelectionPaintingMode = kSelectionPainting_NotSet;

							ReleaseCapture();
							break;
						}
					}

					_RENDERWIN_XSTATE.DraggingPathGridPoints = false;
					BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, wParam, lParam);

					// end free movement handling
					ToggleFreeMouseMovement(hWnd, false);
					ToggleCellViewUpdate(true);

					break;
				}

				return Handled;
			}

			bool RenderWindowMouseManager::IsPaintingSelection() const
			{
				return PaintingSelection;
			}
		}
	}
}