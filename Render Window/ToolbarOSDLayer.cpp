#include "ToolbarOSDLayer.h"
#include "Render Window\RenderWindowManager.h"
#include "Construction Set Extender_Resource.h"

namespace cse
{
	namespace renderWindow
	{
		ToolbarOSDLayer				ToolbarOSDLayer::Instance;

		void ToolbarOSDLayer::RenderPopup(int PopupID, ImGuiDX9* GUI)
		{

			SME_ASSERT(PopupID > kPopup__NONE && PopupID < kPopup__MAX);

			bool Hovering = false, BeginHover = false, EndHover = false;
			switch (PopupID)
			{
			case kPopup_SnapControls:
				ImGui::Button("Snap Controls##popupbtn_snap_controls");
				break;
			case kPopup_CameraMult:
				ImGui::Button("Camera Movement##popupbtn_cam_move");
				break;
			case kPopup_ReferenceMult:
				ImGui::Button("Reference Movement##popupbtn_ref_move");
				break;
			}

			// just render the button if there are open modals
			if (ModalWindowProviderOSDLayer::Instance.HasOpenModals())
				return;

			CheckPopupButtonHoverChange(PopupID, Hovering, BeginHover, EndHover);
			bool NoActivePopup = ActivePopupID == kPopup__NONE;

			if (EndHover)
			{
				if (ActivePopupClosing == false)
				{
					SME_ASSERT(NoActivePopup || ActivePopupID == PopupID);
					ActivePopupClosing = true;
					ActivePopupTimeout = kPopupTimeout;
				}
			}

			bool CurrentPopupClosing = ActivePopupClosing && ActivePopupID == PopupID && ActivePopupID != kPopup__NONE;
			bool AnotherPopupClosing = ActivePopupClosing && ActivePopupID != PopupID && ActivePopupID != kPopup__NONE;
			SME_ASSERT(NoActivePopup || !(CurrentPopupClosing && AnotherPopupClosing));

			if (Hovering || CurrentPopupClosing)
			{
				if (AnotherPopupClosing)
				{
					// close the other popup immediately
					ImGui::CloseCurrentPopup();
					ActivePopupClosing = false;
					ActivePopupID = kPopup__NONE;
				}

				SME_ASSERT(NoActivePopup || ActivePopupID == PopupID);

				if (BeginHover)
					ActivePopupID = PopupID;

				// render the contents of the current popup
				bool CurrentPopupHovering = false;
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

						const char* PopupID = "popup_snap_controls";
						ImGui::OpenPopup(PopupID);
						if (ImGui::BeginPopup(PopupID))
						{
							if (ImGui::Checkbox("Snap Grid", &SnapGrid))
							{
								if (SnapGrid)
									Flags |= TESRenderWindow::kRenderWindowState_SnapToGrid;
								else
									Flags &= ~TESRenderWindow::kRenderWindowState_SnapToGrid;
							}
							ImGui::SameLine();
							ImGui::PushItemWidth(30);
							ImGui::DragInt("##gridDist", &GridVal, 1, 0, 5000);
							ImGui::PopItemWidth();

							ImGui::Separator();

							if (ImGui::Checkbox("Snap Angle", &SnapAngle))
							{
								if (SnapAngle)
									Flags |= TESRenderWindow::kRenderWindowState_SnapToAngle;
								else
									Flags &= ~TESRenderWindow::kRenderWindowState_SnapToAngle;
							}
							ImGui::SameLine();
							ImGui::PushItemWidth(30);
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

							ImGui::PushItemWidth(30);
							if (ImGui::Button("Set"))
								SnapRef = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, SnapRef, true);
							ImGui::SameLine();
							if (ImGui::Button("Clear"))
								SnapRef = nullptr;
							ImGui::PopItemWidth();

							if (CurrentPopupClosing && GUI->IsPopupHovered())
							{
								// reset the timeout every frame if the mouse is hovering over the popup
								ActivePopupTimeout = kPopupTimeout;
							}
							ImGui::EndPopup();
						}

						*(UInt32*)TESRenderWindow::SnapGridDistance = GridVal;
						*(UInt32*)TESRenderWindow::SnapAngle = AngleVal;
						*TESRenderWindow::StateFlags = Flags;
						*TESRenderWindow::SnapReference = SnapRef;
					}

					break;
				case kPopup_CameraMult:
					{
						float CamPan = *TESRenderWindow::CameraPanSpeed;
						float CamZoom = *TESRenderWindow::CameraZoomSpeed;
						float CamRot = *TESRenderWindow::CameraRotationSpeed;

						const char* PopupID = "popup_cam_move";
						ImGui::OpenPopup(PopupID);
						if (ImGui::BeginPopup(PopupID))
						{

							if (CurrentPopupClosing && GUI->IsPopupHovered())
								ActivePopupTimeout = kPopupTimeout;

							ImGui::EndPopup();
						}

						*TESRenderWindow::CameraPanSpeed = CamPan;
						*TESRenderWindow::CameraZoomSpeed = CamZoom;
						*TESRenderWindow::CameraRotationSpeed = CamRot;
					}

					break;
				case kPopup_ReferenceMult:
					{
						float RefMov = *TESRenderWindow::RefMovementSpeed;
						float RefRot = *TESRenderWindow::RefRotationSpeed;

						const char* PopupID = "popup_ref_move";
						ImGui::OpenPopup(PopupID);
						if (ImGui::BeginPopup(PopupID))
						{
							if (CurrentPopupClosing && GUI->IsPopupHovered())
								ActivePopupTimeout = kPopupTimeout;

							ImGui::EndPopup();
						}

						*TESRenderWindow::RefMovementSpeed = RefMov;
						*TESRenderWindow::RefRotationSpeed = RefRot;
					}

					break;
				}
			}

		}

		void ToolbarOSDLayer::CheckPopupButtonHoverChange(int PopupID, bool& OutHovering, bool& OutBeginHover, bool& OutEndHover)
		{
			SME_ASSERT(PopupID > kPopup__NONE && PopupID < kPopup__MAX);

			// assuming the last item was the buttons
			if (ImGui::IsItemHoveredRect())
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
			// can't call CloseCurrentPopup() when there are active modals present
			if (ModalWindowProviderOSDLayer::Instance.HasOpenModals())
				return;

			if (ActivePopupClosing)
			{
				SME_ASSERT(ActivePopupID != kPopup__NONE);
				ActivePopupTimeout -= ImGui::GetIO().DeltaTime;

				if (ActivePopupTimeout <= 0)
				{
					ActivePopupID = kPopup__NONE;
					ActivePopupClosing = false;
					ImGui::CloseCurrentPopup();
				}
			}
		}

		void ToolbarOSDLayer::RenderBottomToolbar(ImGuiDX9* GUI)
		{
			static const int kExpandedHeight = 70, kRegularHeight = 40;

			int XSize = *TESRenderWindow::ScreeWidth;
			int YPos = *TESRenderWindow::ScreeHeight - (BottomExpanded ? kExpandedHeight : kRegularHeight);
			int YSize = BottomExpanded ? kExpandedHeight : kRegularHeight;

			ImGui::SetNextWindowPos(ImVec2(0, YPos));
			ImGui::SetNextWindowSize(ImVec2(XSize, YSize));

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(7, 7));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7, 7));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
			if (!ImGui::Begin("Bottom Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus |
							  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar))
			{
				ImGui::End();
				ImGui::PopStyleVar(4);
				return;
			}

			State.Update(GUI);

			RenderPopup(kPopup_SnapControls, GUI);
			TickActivePopup();

			float CamPan = *TESRenderWindow::CameraPanSpeed;
			float CamZoom = *TESRenderWindow::CameraZoomSpeed;
			float CamRot = *TESRenderWindow::CameraRotationSpeed;
			float RefMov = *TESRenderWindow::RefMovementSpeed;
			float RefRot = *TESRenderWindow::RefRotationSpeed;
			float TOD = _TES->GetSkyTOD();
			UInt32 Flags = *TESRenderWindow::StateFlags;
			bool SnapGrid = Flags & TESRenderWindow::kRenderWindowState_SnapToGrid;
			bool SnapAngle = Flags & TESRenderWindow::kRenderWindowState_SnapToAngle;
			int GridVal = *(UInt32*)TESRenderWindow::SnapGridDistance;
			int AngleVal = *(UInt32*)TESRenderWindow::SnapAngle;
			TESObjectREFR* SnapRef = *TESRenderWindow::SnapReference;
			float FOV = settings::renderer::kCameraFOV().f;

			if (ImGui::Checkbox("Snap Grid", &SnapGrid))
			{
				if (SnapGrid)
					Flags |= TESRenderWindow::kRenderWindowState_SnapToGrid;
				else
					Flags &= ~TESRenderWindow::kRenderWindowState_SnapToGrid;
			} ImGui::SameLine();
			ImGui::PushItemWidth(30); ImGui::DragInt("##gridDist", &GridVal, 1, 0, 5000); ImGui::PopItemWidth(); ImGui::SameLine(0, 20);

			if (ImGui::Checkbox("Snap Angle", &SnapAngle))
			{
				if (SnapAngle)
					Flags |= TESRenderWindow::kRenderWindowState_SnapToAngle;
				else
					Flags &= ~TESRenderWindow::kRenderWindowState_SnapToAngle;
			} ImGui::SameLine();
			ImGui::PushItemWidth(30); ImGui::DragInt("##anglVal", &AngleVal, 1, 0, 500); ImGui::PopItemWidth(); ImGui::SameLine(0, 35);

			char Buffer[0x100] = { 0 };
			if (SnapRef)
			{
				const char* SnapRefID = SnapRef->GetEditorID();
				FORMAT_STR(Buffer, "%s%s%08X%s", (SnapRefID ? SnapRefID : ""), (SnapRefID ? " (" : ""), SnapRef->formID, (SnapRefID ? ")" : ""));
			}
			else
				FORMAT_STR(Buffer, "Snap Reference");

			ImGui::PushItemWidth(100);
			if (ImGui::Button(Buffer))
				SnapRef = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, SnapRef, true);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(Buffer);
			ImGui::PopItemWidth(); ImGui::SameLine(0, 35);


			ImGui::PushItemWidth(100);
			ImGui::DragFloat("##TOD", &TOD, 0.25f, 0.f, 24.f, "TOD: %.2f");
			if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
				ImGui::SetTooltip("Time of Day");
			ImGui::PopItemWidth();

			ImGui::SameLine(0, 30);
			ImGui::Checkbox("Freeze Inactive Refs", &_RENDERWIN_XSTATE.FreezeInactiveRefs);

			ImGui::SameLine(0, 15);
			if (ImGui::Button("Thaw All"))
				_RENDERWIN_MGR.InvokeContextMenuTool(IDC_RENDERWINDOWCONTEXT_THAWALLINCELL);

			ImGui::SameLine(0, 15);
			if (ImGui::Button("Reveal All"))
				_RENDERWIN_MGR.InvokeContextMenuTool(IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL);

			ImGui::SameLine(XSize - 35);
			ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0, 0.8f, 0.8f));
			if (ImGui::Button((BottomExpanded ? " V " : " ^ ")))
				BottomExpanded = BottomExpanded == false;

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(BottomExpanded ? "Close" : "Expand");

			ImGui::PopStyleColor(3);

			ImGui::SameLine(XSize - 60);
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Click and drag the values to change them. CTRL + click to directly edit them.");

			if (BottomExpanded)
			{
				ImGui::Columns(8, "cam/ref controls", false);
				{
					ImGui::Text("Camera:"); ImGui::SameLine(0, 10); ImGui::NextColumn();

					ImGui::DragFloat("##Cam. Pan", &CamPan, 0.05f, 0.01, 10, "Pan: %.3f"); ImGui::NextColumn();
					ImGui::DragFloat("##Cam. Zoom", &CamZoom, 0.05f, 0.01, 10, "Zoom: %.3f"); ImGui::NextColumn();
					ImGui::DragFloat("##Cam. Rotation", &CamRot, 0.05f, 0.01, 10, "Rotation: %.3f"); ImGui::NextColumn();

					ImGui::Text("Reference:"); ImGui::SameLine(0, 10); ImGui::NextColumn();
					ImGui::DragFloat("##Ref. Move", &RefMov, 0.05f, 0.01, 10, "Movement: %.3f"); ImGui::NextColumn();
					ImGui::DragFloat("##Ref. Rotation", &RefRot, 0.05f, 0.01, 10, "Rotation: %.3f"); ImGui::NextColumn();

					ImGui::DragFloat("##FOV", &FOV, 1.f, 50.f, 120.f, "FOV: %.0f");
					if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
						ImGui::SetTooltip("Camera FOV");

					ImGui::NextColumn();
				}
				ImGui::Columns();
			}

			*TESRenderWindow::CameraPanSpeed = CamPan;
			*TESRenderWindow::CameraZoomSpeed = CamZoom;
			*TESRenderWindow::CameraRotationSpeed = CamRot;
			*TESRenderWindow::RefMovementSpeed = RefMov;
			*TESRenderWindow::RefRotationSpeed = RefRot;
			*(UInt32*)TESRenderWindow::SnapGridDistance = GridVal;
			*(UInt32*)TESRenderWindow::SnapAngle = AngleVal;
			*TESRenderWindow::StateFlags = Flags;
			*TESRenderWindow::SnapReference = SnapRef;

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

		ToolbarOSDLayer::ToolbarOSDLayer() :
			IRenderWindowOSDLayer(settings::renderWindowOSD::kShowToolbar, IRenderWindowOSDLayer::kPriority_Toolbar)
		{
			ActivePopupID = kPopup__NONE;
			ActivePopupTimeout = kPopupTimeout;
			ActivePopupClosing = false;

			for (int i = kPopup__NONE + 1; i < kPopup__MAX; i++)
				PopupButtonHoverState[i] = false;

			BottomExpanded = false;
		}

		ToolbarOSDLayer::~ToolbarOSDLayer()
		{
			;//
		}

		void ToolbarOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			State.Update(GUI);
			RenderBottomToolbar(GUI);
		}

		bool ToolbarOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}
	}
}