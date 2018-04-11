#include "ToolbarOSDLayer.h"
#include "Render Window\RenderWindowManager.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"
#include "RenderWindowActions.h"

namespace cse
{
	namespace renderWindow
	{
#define PUSH_TRANSPARENT_BUTTON_COLORS				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0, 0, 0)); \
													ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0, 0, 0)); \
													ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0, 0, 0))
#define POP_TRANSPARENT_BUTTON_COLORS				ImGui::PopStyleColor(3);
#define TOOLBAR_BUTTON_SIZE							ImVec2(30, 0)

		ToolbarOSDLayer				ToolbarOSDLayer::Instance;
		constexpr int				kFilterRefsReset = -9;

		int ToolbarOSDLayer::RefFilterCompletionCallback(ImGuiTextEditCallbackData* Data)
		{
			if ((Data->EventFlag & ImGuiInputTextFlags_CallbackCompletion))
			{
				if (Data->BufTextLen)
				{
					bool CycleBack = ImGui::GetIO().KeyShift;
					FormIDArrayT::const_iterator Bookend = CycleBack ? FilterRefs.begin() : FilterRefs.end();

					// cycle through the filtered refs
					while (FilterRefs.size())
					{
						if (CurrentFilterRefIndex == kFilterRefsReset)
						{
							// first selection, init the index
							if (FilterRefs.size() == 1)
								CurrentFilterRefIndex = 0;
							else if (CycleBack)
								CurrentFilterRefIndex = FilterRefs.size() - 1;		// last item
							else
								CurrentFilterRefIndex = 0;							// first item
						}
						else if (CycleBack)
							CurrentFilterRefIndex--;
						else
							CurrentFilterRefIndex++;

						if (CurrentFilterRefIndex > -1 && CurrentFilterRefIndex < FilterRefs.size())
						{
							// locate and select the reference
							TESForm* Form = TESForm::LookupByFormID(FilterRefs.at(CurrentFilterRefIndex));
							TESObjectREFR* NextRef = nullptr;
							if (Form && Form->IsDeleted() == false)
							{
								SME_ASSERT(Form->formType == TESForm::kFormType_ACHR || Form->formType == TESForm::kFormType_ACRE || Form->formType == TESForm::kFormType_REFR);

								NextRef = CS_CAST(Form, TESForm, TESObjectREFR);
							}
							else
							{
								// invalid ref, remove it and move to the next one
								FilterRefs.erase(FilterRefs.begin() + CurrentFilterRefIndex);
								continue;
							}

							SME_ASSERT(NextRef);
							_TES->LoadCellIntoViewPort(nullptr, NextRef);
						}
						else
						{
							CurrentFilterRefIndex = kFilterRefsReset;
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
			}
			else
				FilterRefs.clear();

			CurrentFilterRefIndex = kFilterRefsReset;
		}

		void ToolbarOSDLayer::RenderBottomToolbars(ImGuiDX9* GUI)
		{
			static const int kRegularHeight = 45;
			static const int kMinWidthRefFilter = 250;

			int XSize = kMinWidthRefFilter;
			int XDel = *TESRenderWindow::ScreenWidth - XSize;
			int XPos = XDel / 2;
			int YPos = *TESRenderWindow::ScreenHeight - kRegularHeight - 8;
			int YSize = kRegularHeight;

			if (BeginToolbarWindow("Bottom Ref Filter", XPos, YPos, XSize, YSize, ImVec2(7, 10), ImVec2(5, 5), ImVec2(5, 5)) == false)
				return;
			else
			{
				TransparentButton(ICON_MD_SEARCH "##find_ref_mouseover", TOOLBAR_BUTTON_SIZE);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Type in the text box to find references matching the filter string.\nCycle through matches with the (SHIFT+)TAB key.");
				ImGui::SameLine(0, 10);

				if (SetRefFilterFocus)
				{
					SetRefFilterFocus = false;
					ImGui::SetKeyboardFocusHere();
				}

				bool HasFilter = strlen(RefFilter.InputBuf);
				ImGui::PushItemWidth(HasFilter ? 140 : 175);
				if (ImGui::InputText("##find_ref_textbox", RefFilter.InputBuf, sizeof(RefFilter.InputBuf),
									 ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCompletion,
									 [](ImGuiTextEditCallbackData* Data)->int { ToolbarOSDLayer* Parent = (ToolbarOSDLayer*)Data->UserData; return Parent->RefFilterCompletionCallback(Data); },
									 this))
				{
					HandleRefFilterChange();
				}
				ImGui::PopItemWidth();

				ImGui::SameLine(0, 10);
				if (HasFilter)
				{
					ImGui::Text(ICON_MD_FILTER_LIST " %d ", FilterRefs.size());
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("%d references match the filter.", FilterRefs.size());
				}
				else
					ImGui::Dummy(ImVec2(5, 0));

				EndToolbarWindow();
			}

			static const int kMinWidthMiscControls = 165;
			XSize = kMinWidthMiscControls;
			XPos = *TESRenderWindow::ScreenWidth - XSize - 10;

			if (BeginToolbarWindow("Bottom Misc Popups", XPos, YPos, XSize, YSize, ImVec2(7, 10), ImVec2(5, 5), ImVec2(5, 5)) == false)
				return;
			else
			{
				void* CurrentToolbarWindow = GUI->GetCurrentWindow();

				BottomToolbarPopupProvider.Draw(PopupSnapControls, GUI, CurrentToolbarWindow);
				ImGui::SameLine(0, 10);
				BottomToolbarPopupProvider.Draw(PopupMovementControls, GUI, CurrentToolbarWindow);
				ImGui::SameLine(0, 10);
				BottomToolbarPopupProvider.Draw(PopupVisibilityToggles, GUI, CurrentToolbarWindow);
				ImGui::SameLine(0, 10);
				BottomToolbarPopupProvider.Draw(PopupMiscControls, GUI, CurrentToolbarWindow);

				EndToolbarWindow();
			}
		}

		void ToolbarOSDLayer::RenderTopToolbar(ImGuiDX9* GUI)
		{
			int XSize = *TESRenderWindow::ScreenWidth;
			int Width = 5 + TopToolbarPopupIDs.size() * 40;

			if (BeginToolbarWindow("Top Dock", XSize - Width, 10, -1, -1, ImVec2(7, 7), ImVec2(5, 5), ImVec2(7, 7)) == false)
				return;
			else
			{
				for (auto Itr : TopToolbarPopupIDs)
				{
					TopToolbarPopupProvider.Draw(Itr, GUI, GUI->GetCurrentWindow());
					ImGui::SameLine(0, 10);
				}

				EndToolbarWindow();
			}
		}

		bool ToolbarOSDLayer::BeginToolbarWindow(const char* Name,
												 int XPos, int YPos, int Width, int Height,
												 const ImVec2& WindowPadding, const ImVec2& FramePadding, const ImVec2& ItemSpacing)
		{
			bool AutoResize = Width == -1 && Height == -1;
			ImGui::SetNextWindowPos(ImVec2(XPos, YPos));
			if (AutoResize == false)
				ImGui::SetNextWindowSize(ImVec2(Width, Height));

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ItemSpacing);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, WindowPadding);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, FramePadding);
			if (!ImGui::Begin(Name, nullptr,
							  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus |
							  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar |
							  (AutoResize ? ImGuiWindowFlags_AlwaysAutoResize : NULL)))
			{
				EndToolbarWindow();
				return false;
			}
			else
				return true;
		}

		void ToolbarOSDLayer::EndToolbarWindow()
		{
			ImGui::End();
			ImGui::PopStyleVar(3);
		}

		ToolbarOSDLayer::ToolbarOSDLayer() :
			IRenderWindowOSDLayer(&settings::renderWindowOSD::kShowToolbar)

		{
			auto SnapControlsButton = []() {
				PUSH_TRANSPARENT_BUTTON_COLORS;
				ImGui::Button(ICON_MD_GRID_ON "##popupbtn_snap_controls", TOOLBAR_BUTTON_SIZE);
				POP_TRANSPARENT_BUTTON_COLORS;

				if (ImGui::IsItemHoveredRect())
					ImGui::SetTooltip("Snap Controls");
			};
			auto SnapControlsPopup = []() {
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
			};

			PopupSnapControls = BottomToolbarPopupProvider.RegisterPopup("popup_snap_controls",
																		 SnapControlsButton,
																		 SnapControlsPopup,
																		MouseOverPopupProvider::kPosition_Relative,
																		ImVec2(-85, -175));


			auto MovementControlsButton = []() {
				const ImVec4 MainColor = ImColor::HSV(4 / 7.0f, 0.6f, 0.6f);

				PUSH_TRANSPARENT_BUTTON_COLORS;
				ImGui::Button(ICON_MD_ZOOM_OUT_MAP "##popupbtn_movement_controls", TOOLBAR_BUTTON_SIZE);
				POP_TRANSPARENT_BUTTON_COLORS;

				if (ImGui::IsItemHoveredRect())
					ImGui::SetTooltip("Movement Controls");
			};
			auto MovementControlsPopup = []() {
				bool Alternate = _RENDERWIN_XSTATE.UseAlternateMovementSettings;

				float CamPan = *TESRenderWindow::CameraPanSpeed;
				float CamZoom = *TESRenderWindow::CameraZoomSpeed;
				float CamRot = *TESRenderWindow::CameraRotationSpeed;
				float RefMov = *TESRenderWindow::RefMovementSpeed;
				float RefRot = *TESRenderWindow::RefRotationSpeed;

				if (Alternate)
				{
					CamPan = settings::renderer::kAltCamPanSpeed().f;
					CamZoom = settings::renderer::kAltCamZoomSpeed().f;
					CamRot = settings::renderer::kAltCamRotationSpeed().f;
					RefMov = settings::renderer::kAltRefMovementSpeed().f;
					RefRot = settings::renderer::kAltRefRotationSpeed().f;
				}

				ImGui::NewLine();

				ImGui::SameLine(0, 30);
				ImGui::Text(ICON_MD_PHOTO_CAMERA);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Camera");

				ImGui::SameLine(0, 80);

				ImGui::Text(ICON_MD_PERSON);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Reference");

				ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(4 / 7.0f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(4 / 7.0f, 0.6f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(4 / 7.0f, 0.7f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(4 / 7.0f, 0.9f, 0.9f));
				{
					ImGui::VSliderFloat("##cam_pan", ImVec2(18, 160), &CamPan, 0.01f, 10.0f, "");
					if (ImGui::IsItemActive() || ImGui::IsItemHovered())
						ImGui::SetTooltip("%.3f", CamPan);

					ImGui::SameLine(0, 10);

					ImGui::VSliderFloat("##cam_zoom", ImVec2(18, 160), &CamZoom, 0.01f, 10.0f, "");
					if (ImGui::IsItemActive() || ImGui::IsItemHovered())
						ImGui::SetTooltip("%.3f", CamZoom);

					ImGui::SameLine(0, 10);

					ImGui::VSliderFloat("##cam_rot", ImVec2(18, 160), &CamRot, 0.01f, 10.0f, "");
					if (ImGui::IsItemActive() || ImGui::IsItemHovered())
						ImGui::SetTooltip("%.3f", CamRot);

					ImGui::SameLine(0, 40);

					ImGui::VSliderFloat("##ref_mov", ImVec2(18, 160), &RefMov, 0.01f, 10.0f, "");
					if (ImGui::IsItemActive() || ImGui::IsItemHovered())
						ImGui::SetTooltip("%.3f", RefMov);

					ImGui::SameLine(0, 10);

					ImGui::VSliderFloat("##ref_rot", ImVec2(18, 160), &RefRot, 0.01f, 10.0f, "");
					if (ImGui::IsItemActive() || ImGui::IsItemHovered())
						ImGui::SetTooltip("%.3f", RefRot);
				}
				ImGui::PopStyleColor(4);
				ImGui::Dummy(ImVec2(10, 5));

				ImGui::Text(ICON_MD_PAN_TOOL); ImGui::SameLine(0, 15);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Pan");

				ImGui::Text(ICON_MD_ZOOM_IN); ImGui::SameLine(0, 15);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Zoom");

				ImGui::Text(ICON_MD_3D_ROTATION); ImGui::SameLine(0, 1);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Rotation");

				ImGui::SameLine(0, 30);

				ImGui::Text(ICON_MD_ZOOM_OUT_MAP); ImGui::SameLine(0, 15);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Movement");

				ImGui::Text(ICON_MD_3D_ROTATION);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Rotation");

				if (Alternate)
				{
					settings::renderer::kAltCamPanSpeed.SetFloat(CamPan);
					settings::renderer::kAltCamZoomSpeed.SetFloat(CamZoom);
					settings::renderer::kAltCamRotationSpeed.SetFloat(CamRot);
					settings::renderer::kAltRefMovementSpeed.SetFloat(RefMov);
					settings::renderer::kAltRefRotationSpeed.SetFloat(RefRot);
				}
				else
				{
					*TESRenderWindow::CameraPanSpeed = CamPan;
					*TESRenderWindow::CameraZoomSpeed = CamZoom;
					*TESRenderWindow::CameraRotationSpeed = CamRot;
					*TESRenderWindow::RefMovementSpeed = RefMov;
					*TESRenderWindow::RefRotationSpeed = RefRot;
				}

				ImGui::Dummy(ImVec2(10, 5));
				if (ImGui::Checkbox("Alternate Settings", &Alternate))
					actions::ToggleAlternateMovementSettings();
			};
			PopupMovementControls= BottomToolbarPopupProvider.RegisterPopup("popup_movement_controls",
																			MovementControlsButton,
																			MovementControlsPopup,
																			MouseOverPopupProvider::kPosition_Relative,
																			ImVec2(-75, -290));


			auto VisibilityButtons = []() {
				PUSH_TRANSPARENT_BUTTON_COLORS;
				ImGui::Button(ICON_MD_REMOVE_RED_EYE "##popupbtn_visibility_toggles", TOOLBAR_BUTTON_SIZE);
				POP_TRANSPARENT_BUTTON_COLORS;

				if (ImGui::IsItemHoveredRect())
					ImGui::SetTooltip("Visibility Toggles");
			};
			auto VisibilityPopup = []() {
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
			};
			PopupVisibilityToggles = BottomToolbarPopupProvider.RegisterPopup("popup_visibility_toggles",
																			  VisibilityButtons,
																			  VisibilityPopup,
																			  MouseOverPopupProvider::kPosition_Relative,
																			  ImVec2(-10, -595));

			auto MiscButton = []() {
				PUSH_TRANSPARENT_BUTTON_COLORS;
				ImGui::Button(ICON_MD_MORE_HORIZ "##popupbtn_misc_controls", TOOLBAR_BUTTON_SIZE);
				POP_TRANSPARENT_BUTTON_COLORS;
			};
			auto MiscPopup = []() {

				float TOD = _TES->GetSkyTOD();
				float FOV = settings::renderer::kCameraFOV().f;

				PUSH_TRANSPARENT_BUTTON_COLORS;
				ImGui::Button(ICON_MD_ACCESS_TIME " ");
				ImGui::SameLine(0, 2);
				ImGui::PushItemWidth(50);
				ImGui::DragFloat("Time of Day##TOD", &TOD, 0.25f, 0.f, 24.f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::Button(ICON_MD_PANORAMA_HORIZONTAL " ");
				ImGui::SameLine(0, 2);
				ImGui::PushItemWidth(50);
				ImGui::DragFloat("Field-of-Vision##FOV", &FOV, 1.f, 50.f, 120.f, "%.0f");
				ImGui::PopItemWidth();
				POP_TRANSPARENT_BUTTON_COLORS;

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
			};
			PopupMiscControls = BottomToolbarPopupProvider.RegisterPopup("popup_misc_controls",
																		 MiscButton,
																		 MiscPopup,
																		 MouseOverPopupProvider::kPosition_Relative,
																		 ImVec2(-35, -20));

			FilterRefs.reserve(100);
			CurrentFilterRefIndex = -1;
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

			RenderBottomToolbars(GUI);
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
			MouseOverPopupProvider::PopupIDT ID = TopToolbarPopupProvider.RegisterPopup(PopupID, DrawButton, DrawPopup,
																						MouseOverPopupProvider::kPosition_Relative,
																						ImVec2(0, 25));
			TopToolbarPopupIDs.push_back(ID);
		}

		void ToolbarOSDLayer::TransparentButton(const char* Name, const ImVec2& Size)
		{
			PUSH_TRANSPARENT_BUTTON_COLORS;
			ImGui::Button(Name, Size);
			POP_TRANSPARENT_BUTTON_COLORS;
		}

	}
}