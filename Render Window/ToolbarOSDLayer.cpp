#include "ToolbarOSDLayer.h"
#include "Render Window\RenderWindowManager.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"
#include "RenderWindowActions.h"

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

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
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
			ImGui::Columns(5, "toolbar_contents", false);
			{
				ImGui::TextWrapped("Movement Controls: ");
				ImGui::SameLine(0, 10);
				BottomToolbarPopupProvider.Draw(PopupSnapControls, GUI, CurrentToolbarWindow);
				ImGui::SameLine(0, 10);
				BottomToolbarPopupProvider.Draw(PopupMovementControls, GUI, CurrentToolbarWindow);
				ImGui::SameLine(0, 20);

				ImGui::NextColumn();
				PUSH_TRANSPARENT_BUTTON_COLORS;
				ImGui::Button(ICON_MD_SEARCH "##find_ref_mouseover", TOOLBAR_BUTTON_SIZE);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Type in the text box to find references matching the filter string.\nCycle through matches with the TAB key.");
				POP_TRANSPARENT_BUTTON_COLORS;
				ImGui::SameLine(0, 10);

				if (SetRefFilterFocus)
				{
					SetRefFilterFocus = false;
					ImGui::SetKeyboardFocusHere();
				}
				if (ImGui::InputText("##find_ref_textbox", RefFilter.InputBuf, sizeof(RefFilter.InputBuf),
									 ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCompletion,
									 [](ImGuiTextEditCallbackData* Data)->int { ToolbarOSDLayer* Parent = (ToolbarOSDLayer*)Data->UserData; return Parent->RefFilterCompletionCallback(Data); },
									 this))
				{
					HandleRefFilterChange();
				}
				ImGui::SameLine(0, 10);
				if (strlen(RefFilter.InputBuf))
				{
					ImGui::Text(ICON_MD_FILTER_LIST " %d", FilterRefs.size());
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("%d references match the filter.", FilterRefs.size());
				}
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
					actions::ThawAll();

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Thaw all Frozen References");

				ImGui::SameLine(0, 20);

				if (ImGui::Button(ICON_MD_FLIP_TO_FRONT "##reveal_all_refs", TOOLBAR_BUTTON_SIZE))
					actions::RevealAll();

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
				ImGui::SameLine(0, 25);

				ImGui::Button(ICON_MD_PANORAMA_HORIZONTAL " ", TOOLBAR_BUTTON_SIZE);
				ImGui::SameLine(0, 5);
				ImGui::PushItemWidth(70);
				ImGui::DragFloat("##FOV", &FOV, 1.f, 50.f, 120.f, "%.0f");
				if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
					ImGui::SetTooltip("Camera Field-of-Vision");
				ImGui::PopItemWidth();
				POP_TRANSPARENT_BUTTON_COLORS;

				ImGui::NextColumn();
				ImGui::SetColumnOffset(-1, XSize - 40);
				BottomToolbarPopupProvider.Draw(PopupVisibilityToggles, GUI, CurrentToolbarWindow);

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
			int Width = 5 + TopToolbarPopupIDs.size() * 40;
			ImGui::SetNextWindowPos(ImVec2(XSize - Width, 10));

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

			for (auto Itr : TopToolbarPopupIDs)
			{
				TopToolbarPopupProvider.Draw(Itr, GUI, GUI->GetCurrentWindow());
				ImGui::SameLine(0, 10);
			}

			ImGui::End();
			ImGui::PopStyleVar(3);
			ImGui::PopStyleColor();
		}

		ToolbarOSDLayer::ToolbarOSDLayer() :
			IRenderWindowOSDLayer(&settings::renderWindowOSD::kShowToolbar)
		{
			PopupSnapControls = BottomToolbarPopupProvider.RegisterPopup("popup_snap_controls",
															[]() {
				const ImVec4 MainColor = ImColor::HSV(4 / 7.0f, 0.6f, 0.6f);

				ImGui::PushStyleColor(ImGuiCol_Button, MainColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, MainColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, MainColor);

				ImGui::Button(ICON_MD_GRID_ON "##popupbtn_snap_controls", TOOLBAR_BUTTON_SIZE);

				ImGui::PopStyleColor(3);
			},
															[]() {
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
			});

			PopupMovementControls= BottomToolbarPopupProvider.RegisterPopup("popup_movement_controls",
															   []() {
				const ImVec4 MainColor = ImColor::HSV(4 / 7.0f, 0.6f, 0.6f);

				ImGui::PushStyleColor(ImGuiCol_Button, MainColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, MainColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, MainColor);

				ImGui::Button(ICON_MD_ZOOM_OUT_MAP "##popupbtn_movement_controls", TOOLBAR_BUTTON_SIZE);

				ImGui::PopStyleColor(3);
			},
															   []() {
				float CamPan = *TESRenderWindow::CameraPanSpeed;
				float CamZoom = *TESRenderWindow::CameraZoomSpeed;
				float CamRot = *TESRenderWindow::CameraRotationSpeed;
				float RefMov = *TESRenderWindow::RefMovementSpeed;
				float RefRot = *TESRenderWindow::RefRotationSpeed;

				float AltCamPan = settings::renderer::kAltCamPanSpeed().f;
				float AltCamZoom = settings::renderer::kAltCamZoomSpeed().f;
				float AltCamRot = settings::renderer::kAltCamRotationSpeed().f;
				float AltRefMov = settings::renderer::kAltRefMovementSpeed().f;
				float AltRefRot = settings::renderer::kAltRefRotationSpeed().f;

				ImGui::PushItemWidth(125);
				ImGui::Text("Default"); ImGui::SameLine(160); ImGui::Text("Alternate");
				ImGui::Separator();

				ImGui::Text("Camera:");
				ImGui::DragFloat("##DefCam. Pan", &CamPan, 0.05f, 0.01, 10, "Pan: %.3f"); ImGui::SameLine(0, 20);
				ImGui::DragFloat("##AltCam. Pan", &AltCamPan, 0.05f, 0.01, 10, "Pan: %.3f");
				ImGui::DragFloat("##DefCam. Rotation", &CamRot, 0.05f, 0.01, 10, "Rotation: %.3f"); ImGui::SameLine(0, 20);
				ImGui::DragFloat("##AltCam. Rotation", &AltCamRot, 0.05f, 0.01, 10, "Rotation: %.3f");
				ImGui::DragFloat("##DefCam. Zoom", &CamZoom, 0.05f, 0.01, 10, "Zoom: %.3f"); ImGui::SameLine(0, 20);
				ImGui::DragFloat("##AltCam. Zoom", &AltCamZoom, 0.05f, 0.01, 10, "Zoom: %.3f");

				ImGui::Text("Reference:");
				ImGui::DragFloat("##DefRef. Move", &RefMov, 0.05f, 0.01, 10, "Movement: %.3f"); ImGui::SameLine(0, 20);
				ImGui::DragFloat("##AltRef. Move", &AltRefMov, 0.05f, 0.01, 10, "Movement: %.3f");
				ImGui::DragFloat("##DefRef. Rotation", &RefRot, 0.05f, 0.01, 10, "Rotation: %.3f"); ImGui::SameLine(0, 20);
				ImGui::DragFloat("##AltRef. Rotation", &AltRefRot, 0.05f, 0.01, 10, "Rotation: %.3f");
				ImGui::PopItemWidth();

				*TESRenderWindow::CameraPanSpeed = CamPan;
				*TESRenderWindow::CameraZoomSpeed = CamZoom;
				*TESRenderWindow::CameraRotationSpeed = CamRot;
				*TESRenderWindow::RefMovementSpeed = RefMov;
				*TESRenderWindow::RefRotationSpeed = RefRot;

				settings::renderer::kAltCamPanSpeed.SetFloat(AltCamPan);
				settings::renderer::kAltCamZoomSpeed.SetFloat(AltCamZoom);
				settings::renderer::kAltCamRotationSpeed.SetFloat(AltCamRot);
				settings::renderer::kAltRefMovementSpeed.SetFloat(AltRefMov);
				settings::renderer::kAltRefRotationSpeed.SetFloat(AltRefRot);
			});

			PopupVisibilityToggles = BottomToolbarPopupProvider.RegisterPopup("popup_visibility_toggles",
																 []() {
				const ImVec4 MainColor = ImColor::HSV(4 / 7.0f, 0.6f, 0.6f);

				ImGui::PushStyleColor(ImGuiCol_Button, MainColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, MainColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, MainColor);

				ImGui::Button(ICON_MD_REMOVE_RED_EYE "##popupbtn_visibility_toggles", TOOLBAR_BUTTON_SIZE);

				ImGui::PopStyleColor(3);
			},
																 []() {
				ImGui::PushID("visibility_toggle_menu_item");
				{
					static const char* kNames[actions::ToggleVisibilityRWA::kType__MAX] =
					{
						"Objects",
						"Markers",
						"Wireframe",
						"Bright Light",
						"Sky",
						"Solid Subspaces",
						"Collision Geometry",
						"Leaves",
						"Trees",
						"Water",
						"Cell Borders",
						"Landscape",
						"Light Radius",
						"Parent Child Indicator",
						"Path Grid Linked Reference Indicator",
						"Initially Disabled References",
						"Initially Disabled References' Children",
						"Grass Overlay"
					};

					bool Toggles[actions::ToggleVisibilityRWA::kType__MAX] = { false };
					for (int i = 0; i < actions::ToggleVisibilityRWA::kType__MAX; i++)
					{

						Toggles[i] = actions::ToggleVisibilityRWA::IsVisible(i);
						if (ImGui::Checkbox(kNames[i], &Toggles[i]))
							actions::ToggleVisibility[i]();
					}
				}
				ImGui::PopID();
			});

			FilterRefs.reserve(100);
			PreviousFilterRef = FilterRefs.end();
			SetRefFilterFocus = false;
		}

		ToolbarOSDLayer::~ToolbarOSDLayer()
		{
			;//
		}

		void ToolbarOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			BottomToolbarPopupProvider.Update();
			TopToolbarPopupProvider.Update();

			RenderMainToolbar(GUI);
			RenderTopToolbar(GUI);
		}

		bool ToolbarOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		void ToolbarOSDLayer::FocusOnRefFilter()
		{
			SetRefFilterFocus = true;
		}

		void ToolbarOSDLayer::RegisterTopToolbarButton(const char* PopupID,
													   MouseOverPopupProvider::RenderDelegateT DrawButton,
													   MouseOverPopupProvider::RenderDelegateT DrawPopup)
		{
			MouseOverPopupProvider::PopupIDT ID = TopToolbarPopupProvider.RegisterPopup(PopupID, DrawButton, DrawPopup);
			TopToolbarPopupIDs.push_back(ID);
		}

	}
}