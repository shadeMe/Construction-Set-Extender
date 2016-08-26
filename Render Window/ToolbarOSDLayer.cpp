#include "ToolbarOSDLayer.h"
#include "Render Window\RenderWindowManager.h"
#include "Construction Set Extender_Resource.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"

namespace cse
{
	namespace renderWindow
	{
#define PUSH_TRANSPARENT_BUTTON_COLORS				ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0, 0, 0, 0)); \
													ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0, 0, 0, 0)); \
													ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0, 0, 0, 0))
#define POP_TRANSPARENT_BUTTON_COLORS				ImGui::PopStyleColor(3);
#define TOOLBAR_BUTTON_SIZE							ImVec2(30, 0)

		ToolbarOSDLayer				ToolbarOSDLayer::Instance;

		void ToolbarOSDLayer::RenderPopup(int PopupID, ImGuiDX9* GUI, void* ParentToolbarWindow, const ImVec2& ButtonSize, const ImVec4* ButtonColor)
		{
			SME_ASSERT(PopupID > kPopup__NONE && PopupID < kPopup__MAX);

			bool Hovering = false, BeginHover = false, EndHover = false;
			const ImVec4 MainColor = ImColor::HSV(4 / 7.0f, 0.6f, 0.6f);
			if (ButtonColor == nullptr)
				ButtonColor = &MainColor;
			const char* PopupStrID = nullptr;

			ImGui::PushStyleColor(ImGuiCol_Button, *ButtonColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, *ButtonColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, *ButtonColor);

			switch (PopupID)
			{
			case kPopup_SnapControls:
				ImGui::Button(ICON_MD_GRID_ON "##popupbtn_snap_controls", ButtonSize);
				PopupStrID = "popup_snap_controls";
				break;
			case kPopup_MovementControls:
				ImGui::Button(ICON_MD_ZOOM_OUT_MAP "##popupbtn_movement_controls", ButtonSize);
				PopupStrID = "popup_movement_controls";
				break;
			case kPopup_VisibilityToggles:
				ImGui::Button(ICON_MD_REMOVE_RED_EYE "##popupbtn_visibility_toggles", ButtonSize);
				PopupStrID = "popup_visibility_toggles";
				break;
			}

			ImGui::PopStyleColor(3);

			// just render the button if there are open modals
			if (ModalWindowProviderOSDLayer::Instance.HasOpenModals())
				return;

			StateData& CurrentState = PopupStateData[PopupID];
			bool ActiveStateDragging = false;
			if (ActivePopup != kPopup__NONE && PopupStateData[ActivePopup].DragInput.Active)
				ActiveStateDragging = true;

			CheckPopupButtonHoverChange(PopupID, GUI, ParentToolbarWindow, Hovering, BeginHover, EndHover);

			if (BeginHover && ActiveStateDragging == false)
			{
				ActivePopup = PopupID;
				ActivePopupTimeout = 0.f;
			}
			else if (EndHover && ActiveStateDragging == false)
			{
				ActivePopupTimeout = kTimeoutPopup;
			}

			if (ActivePopup == PopupID)
			{
				if (BeginHover)
					ImGui::OpenPopup(PopupStrID);

				if (ImGui::BeginPopup(PopupStrID))
				{
					CurrentState.Update(GUI);

					if (CloseActivePopup)
					{
						CloseActivePopup = false;
						ActivePopupTimeout = 0.f;
						ActivePopup = kPopup__NONE;
						ImGui::CloseCurrentPopup();
					}
					else
					{
						// render the contents of the current popup
						switch (PopupID)
						{
						case kPopup_SnapControls:
							{
								UInt32 Flags = *TESRenderWindow::StateFlags;
								bool SnapGrid = Flags & TESRenderWindow::kRenderWindowState_SnapToGrid;
								bool SnapAngle = Flags & TESRenderWindow::kRenderWindowState_SnapToAngle;
								int GridVal = *(UInt32*)TESRenderWindow::SnapGridDistance;
								int AngleVal = *(UInt32*)TESRenderWindow::SnapAngle;
								TESObjectREFR* SnapRef = *TESRenderWindow::SnapReference;

								if (ImGui::Checkbox("Snap Grid   ", &SnapGrid))
								{
									if (SnapGrid)
										Flags |= TESRenderWindow::kRenderWindowState_SnapToGrid;
									else
										Flags &= ~TESRenderWindow::kRenderWindowState_SnapToGrid;
								}
								ImGui::SameLine();
								ImGui::PushItemWidth(40);
								ImGui::DragInt("##gridDist", &GridVal, 1, 0, 5000);
								ImGui::PopItemWidth();

								ImGui::Separator();

								if (ImGui::Checkbox("Snap Angle ", &SnapAngle))
								{
									if (SnapAngle)
										Flags |= TESRenderWindow::kRenderWindowState_SnapToAngle;
									else
										Flags &= ~TESRenderWindow::kRenderWindowState_SnapToAngle;
								}
								ImGui::SameLine();
								ImGui::PushItemWidth(40);
								ImGui::DragInt("##anglVal", &AngleVal, 1, 0, 500);
								ImGui::PopItemWidth();

								ImGui::Separator();

								ImGui::Text("Snap Reference:");
								if (SnapRef)
								{
									const char* SnapRefID = SnapRef->GetEditorID();
									ImGui::TextWrapped("%s%s%08X%s", (SnapRefID ? SnapRefID : ""), (SnapRefID ? " (" : ""), SnapRef->formID, (SnapRefID ? ")" : ""));
								}
								else
									ImGui::Text("None");

								if (ImGui::Button("Set", ImVec2(70, 25)))
								{
									TESObjectREFR* NewSnap = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, SnapRef, true);
									if (NewSnap)
										SnapRef = NewSnap;
								}
								ImGui::SameLine();
								if (ImGui::Button("Clear", ImVec2(70, 25)))
									SnapRef = nullptr;

								*(UInt32*)TESRenderWindow::SnapGridDistance = GridVal;
								*(UInt32*)TESRenderWindow::SnapAngle = AngleVal;
								*TESRenderWindow::StateFlags = Flags;
								*TESRenderWindow::SnapReference = SnapRef;
							}

							break;
						case kPopup_MovementControls:
							{
								float CamPan = *TESRenderWindow::CameraPanSpeed;
								float CamZoom = *TESRenderWindow::CameraZoomSpeed;
								float CamRot = *TESRenderWindow::CameraRotationSpeed;
								float RefMov = *TESRenderWindow::RefMovementSpeed;
								float RefRot = *TESRenderWindow::RefRotationSpeed;

								ImGui::PushItemWidth(150);
								ImGui::Text("Camera:");
								ImGui::DragFloat("##Cam. Pan", &CamPan, 0.05f, 0.01, 10, "Pan: %.3f");
								ImGui::DragFloat("##Cam. Rotation", &CamRot, 0.05f, 0.01, 10, "Rotation: %.3f");
								ImGui::DragFloat("##Cam. Zoom", &CamZoom, 0.05f, 0.01, 10, "Zoom: %.3f");

								ImGui::Text("Reference:");
								ImGui::DragFloat("##Ref. Move", &RefMov, 0.05f, 0.01, 10, "Movement: %.3f");
								ImGui::DragFloat("##Ref. Rotation", &RefRot, 0.05f, 0.01, 10, "Rotation: %.3f");
								ImGui::PopItemWidth();

								*TESRenderWindow::CameraPanSpeed = CamPan;
								*TESRenderWindow::CameraZoomSpeed = CamZoom;
								*TESRenderWindow::CameraRotationSpeed = CamRot;
								*TESRenderWindow::RefMovementSpeed = RefMov;
								*TESRenderWindow::RefRotationSpeed = RefRot;
							}

							break;
						}

						if (Hovering == false && GUI->IsPopupHovered())
						{
							// reset the timeout every frame if the mouse is hovering over the popup
							ActivePopupTimeout = kTimeoutPopup;
						}
					}

					ImGui::EndPopup();
				}
			}
		}

		void ToolbarOSDLayer::CheckPopupButtonHoverChange(int PopupID, ImGuiDX9* GUI, void* ParentToolbarWindow,
														  bool& OutHovering, bool& OutBeginHover, bool& OutEndHover)
		{
			SME_ASSERT(PopupID > kPopup__NONE && PopupID < kPopup__MAX);

			// assuming the last item was one of the buttons
			if (ImGui::IsItemHoveredRect() && GUI->GetHoveredWindow() == ParentToolbarWindow)
			{
				OutHovering = true;
				if (PopupButtonHoverState[PopupID] == false)
				{
					PopupButtonHoverState[PopupID] = true;
					OutBeginHover = true;
				}
			}
			else
			{
				if (PopupButtonHoverState[PopupID])
				{
					PopupButtonHoverState[PopupID] = false;
					OutEndHover = true;
				}
			}
		}

		void ToolbarOSDLayer::TickActivePopup()
		{
			if (ModalWindowProviderOSDLayer::Instance.HasOpenModals())
				return;

			if (ActivePopup != kPopup__NONE && ActivePopupTimeout != 0.f && CloseActivePopup == false)
			{
				StateData& ActiveState = PopupStateData[ActivePopup];
				if (ActiveState.DragInput.Active == false)
				{
					ActivePopupTimeout -= ImGui::GetIO().DeltaTime;
					if (ActivePopupTimeout <= 0.f)
					{
						CloseActivePopup = true;
						ActivePopupTimeout = 0.f;
					}
				}
			}
		}

		int ToolbarOSDLayer::RefFilterCompletionCallback(ImGuiTextEditCallbackData* Data)
		{
			if ((Data->EventFlag & ImGuiInputTextFlags_CallbackCompletion))
			{
				if (Data->BufTextLen)
				{
					// cycle through the filtered refs
					while (FilterRefs.size())
					{
						if (PreviousFilterRef != FilterRefs.end())
						{
							// locate and select the reference
							TESForm* Form = TESForm::LookupByFormID(*PreviousFilterRef);
							TESObjectREFR* NextRef = nullptr;
							if (Form && Form->IsDeleted() == false)
							{
								SME_ASSERT(Form->formType == TESForm::kFormType_ACHR || Form->formType == TESForm::kFormType_ACRE || Form->formType == TESForm::kFormType_REFR);

								NextRef = CS_CAST(Form, TESForm, TESObjectREFR);
							}
							else
							{
								// invalid ref, move to the next one
								PreviousFilterRef = FilterRefs.erase(PreviousFilterRef);
								continue;
							}

							SME_ASSERT(NextRef);
							_TES->LoadCellIntoViewPort(nullptr, NextRef);

							++PreviousFilterRef;
						}
						else if (PreviousFilterRef == FilterRefs.end())
						{
							PreviousFilterRef = FilterRefs.begin();
							continue;
						}

						break;
					}
				}
			}

			return 0;
		}

		void ToolbarOSDLayer::HandleRefFilterChange()
		{
			if (strlen(RefFilter.InputBuf))
			{
				// refresh the filter
				FilterRefs.clear();
				RefFilter.Build();

				for (auto Itr : _RENDERWIN_MGR.GetActiveRefs())
				{
					std::string EditorID(Helpers::GetRefEditorID(Itr));
					UInt32 FormID = Itr->formID;
					const char* Type = TESForm::GetFormTypeIDLongName(Itr->baseForm->formType);

					char FilterBuffer[0x200] = { 0 };
					FORMAT_STR(FilterBuffer, "%s %08X %s", EditorID.c_str(), FormID, Type);
					if (RefFilter.PassFilter(FilterBuffer))
						FilterRefs.push_back(FormID);
				}

				PreviousFilterRef = FilterRefs.begin();
			}
			else
			{
				FilterRefs.clear();
				PreviousFilterRef = FilterRefs.end();
			}
		}

		void ToolbarOSDLayer::RenderMainToolbar(ImGuiDX9* GUI)
		{
			static const int kRegularHeight = 40;

			int XSize = *TESRenderWindow::ScreeWidth;
			int YPos = *TESRenderWindow::ScreeHeight - kRegularHeight;
			int YSize = kRegularHeight;

			ImGui::SetNextWindowPos(ImVec2(0, YPos));
			ImGui::SetNextWindowSize(ImVec2(XSize, YSize));

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7, 7));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
			if (!ImGui::Begin("Main Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus |
							  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar))
			{
				ImGui::End();
				ImGui::PopStyleVar(4);
				return;
			}

			void* CurrentToolbarWindow = GUI->GetCurrentWindow();

			float TOD = _TES->GetSkyTOD();
			float FOV = settings::renderer::kCameraFOV().f;

			ImGui::PushAllowKeyboardFocus(false);
			ImGui::Columns(4, "toolbar_contents", false);
			{
				ImGui::TextWrapped("Movement Controls: ");
				ImGui::SameLine(0, 10);
				RenderPopup(kPopup_SnapControls, GUI, CurrentToolbarWindow, TOOLBAR_BUTTON_SIZE);
				ImGui::SameLine(0, 10);
				RenderPopup(kPopup_MovementControls, GUI, CurrentToolbarWindow, TOOLBAR_BUTTON_SIZE);
				ImGui::SameLine(0, 20);

				ImGui::NextColumn();
				PUSH_TRANSPARENT_BUTTON_COLORS;
				ImGui::Button(ICON_MD_SEARCH "##find_ref_mouseover", TOOLBAR_BUTTON_SIZE);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Type in the text box to find references matching the filter string.\nCycle through matches with the TAB key.");
				POP_TRANSPARENT_BUTTON_COLORS;
				ImGui::SameLine(0, 10);
				if (ImGui::InputText("##find_ref_textbox", RefFilter.InputBuf, sizeof(RefFilter.InputBuf),
									 ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCompletion,
									 [](ImGuiTextEditCallbackData* Data)->int { ToolbarOSDLayer* Parent = (ToolbarOSDLayer*)Data->UserData; return Parent->RefFilterCompletionCallback(Data); },
									 this))
				{
					HandleRefFilterChange();
				}
				GUI->WhitelistItemForMouseEvents();
				ImGui::SameLine(0, 10);
				if (strlen(RefFilter.InputBuf))
					ImGui::Text(ICON_MD_FILTER_LIST " %d", FilterRefs.size());
				else
					ImGui::Dummy(ImVec2(5, 5));

				ImGui::NextColumn();
				const char* FreezeInactiveCaption = nullptr;
				const char* FreezeInactiveToolTip = nullptr;
				if (_RENDERWIN_XSTATE.FreezeInactiveRefs)
				{
					FreezeInactiveCaption = ICON_MD_LOCK " " ICON_MD_STAR "##freeze_inactive_refs";
					FreezeInactiveToolTip = "Inactive References Frozen";

					ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0, 0.6f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0, 0.8f, 0.8f));
				}
				else
				{
					FreezeInactiveCaption = ICON_MD_LOCK_OPEN " " ICON_MD_STAR "##freeze_inactive_refs";
					FreezeInactiveToolTip = "Inactive References not Frozen";

					ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));
				}

				if (ImGui::Button(FreezeInactiveCaption, ImVec2(45, 0)))
					_RENDERWIN_XSTATE.FreezeInactiveRefs = _RENDERWIN_XSTATE.FreezeInactiveRefs == false;

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(FreezeInactiveToolTip);

				ImGui::PopStyleColor(3);
				ImGui::SameLine(0, 20);

				if (ImGui::Button(ICON_MD_LOCK_OPEN "##thaw_all_refs", TOOLBAR_BUTTON_SIZE))
					_RENDERWIN_MGR.InvokeContextMenuTool(IDC_RENDERWINDOWCONTEXT_THAWALLINCELL);

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Thaw all Frozen References");

				ImGui::SameLine(0, 20);

				if (ImGui::Button(ICON_MD_FLIP_TO_FRONT "##reveal_all_refs", TOOLBAR_BUTTON_SIZE))
					_RENDERWIN_MGR.InvokeContextMenuTool(IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL);

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Reveal all Invisible References");

				ImGui::NextColumn();

				PUSH_TRANSPARENT_BUTTON_COLORS;
				ImGui::Button(ICON_MD_ACCESS_TIME " ", TOOLBAR_BUTTON_SIZE);
				ImGui::SameLine(0, 5);
				ImGui::PushItemWidth(70);
				ImGui::DragFloat("##TOD", &TOD, 0.25f, 0.f, 24.f, "%.2f");
				if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
					ImGui::SetTooltip("Time of Day");
				ImGui::PopItemWidth();
				ImGui::SameLine(0, 35);

				ImGui::Button(ICON_MD_PANORAMA_HORIZONTAL " ", TOOLBAR_BUTTON_SIZE);
				ImGui::SameLine(0, 5);
				ImGui::PushItemWidth(70);
				ImGui::DragFloat("##FOV", &FOV, 1.f, 50.f, 120.f, "%.0f");
				if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
					ImGui::SetTooltip("Camera Field-of-Vision");
				ImGui::PopItemWidth();
				POP_TRANSPARENT_BUTTON_COLORS;

				ImGui::NextColumn();
			}
			ImGui::Columns();
			ImGui::PopAllowKeyboardFocus();

			if (TOD < 0 || TOD > 24)
				TOD = 10;

			if (TOD != _TES->GetSkyTOD())
				_TES->SetSkyTOD(TOD);

			if (FOV < 50 || FOV > 120)
				FOV = 75;

			if (settings::renderer::kCameraFOV().f != FOV)
			{
				settings::renderer::kCameraFOV.SetFloat(FOV);
				_RENDERWIN_MGR.RefreshFOV();
			}

			ImGui::End();
			ImGui::PopStyleVar(4);
		}

		void ToolbarOSDLayer::RenderTopToolbar(ImGuiDX9* GUI)
		{
			int XSize = *TESRenderWindow::ScreeWidth;

			ImGui::SetNextWindowPos(ImVec2(XSize - 45, 10));

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0, 0, 0, 100));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(7, 7));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7, 7));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
			if (!ImGui::Begin("Top Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus |
							  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::End();
				ImGui::PopStyleVar(3);
				ImGui::PopStyleColor();
				return;
			}

			void* CurrentToolbarWindow = GUI->GetCurrentWindow();

			ImColor VisibilityButtonColor(0, 0, 0, 0);
			RenderPopup(kPopup_VisibilityToggles, GUI, CurrentToolbarWindow, ImVec2(0, 0), &VisibilityButtonColor.Value);

			ImGui::End();
			ImGui::PopStyleVar(3);
			ImGui::PopStyleColor();
		}

		ToolbarOSDLayer::ToolbarOSDLayer() :
			IRenderWindowOSDLayer(settings::renderWindowOSD::kShowToolbar, IRenderWindowOSDLayer::kPriority_Toolbar),
			PopupStateData(),
			RefFilter(),
			FilterRefs()
		{
			ActivePopup = kPopup__NONE;
			ActivePopupTimeout = 0.f;
			CloseActivePopup = false;

			for (int i = kPopup__NONE + 1; i < kPopup__MAX; i++)
				PopupButtonHoverState[i] = false;

			FilterRefs.reserve(100);
			PreviousFilterRef = FilterRefs.end();
		}

		ToolbarOSDLayer::~ToolbarOSDLayer()
		{
			;//
		}

		void ToolbarOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			TickActivePopup();

			RenderMainToolbar(GUI);
			RenderTopToolbar(GUI);
		}

		bool ToolbarOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}
	}
}