#include "SelectionControlsOSDLayer.h"
#include "Render Window\RenderWindowManager.h"
#include "Achievements.h"
#include "Construction Set Extender_Resource.h"

namespace cse
{
	namespace renderWindow
	{
		SelectionControlsOSDLayer		SelectionControlsOSDLayer::Instance;

		void SelectionControlsOSDLayer::EditReference(TESObjectREFR* Ref)
		{
			TESDialog::ShowFormEditDialog(Ref);
		}

		void SelectionControlsOSDLayer::EditBaseForm(TESObjectREFR* Ref)
		{
			TESDialog::ShowFormEditDialog(Ref->baseForm);
		}

		void SelectionControlsOSDLayer::DrawDragTrail()
		{
			if (ImGui::IsItemActive())
			{
				ImDrawList* DrawList = ImGui::GetWindowDrawList();
				DrawList->PushClipRectFullScreen();
				DrawList->AddLine(ImGui::CalcItemRectClosestPoint(ImGui::GetIO().MousePos, true, -2.0f),
								  ImGui::GetIO().MousePos,
								  ImColor(ImGui::GetStyle().Colors[ImGuiCol_Button]),
								  4.0f);
				DrawList->PopClipRect();
			}
		}

		void SelectionControlsOSDLayer::MoveSelection(bool X, bool Y, bool Z)
		{
			if (ImGui::IsItemActive() == false)
				return;

			ImVec2 Delta = ImGui::GetIO().MouseDelta;
			_PRIMARYRENDERER->MoveReferenceSelection(Delta.x, Delta.x, X, Y, Z);
		}

		void SelectionControlsOSDLayer::RotateSelection(bool Local, bool X, bool Y, bool Z)
		{
			if (ImGui::IsItemActive() == false)
				return;

			UInt32 FlagsState = *TESRenderWindow::StateFlags;
			UInt32 FlagsBuffer = FlagsState & ~TESRenderWindow::kRenderWindowState_UseWorld;
			if (Local == false)
				FlagsBuffer |= TESRenderWindow::kRenderWindowState_UseWorld;

			ImVec2 Delta = ImGui::GetIO().MouseDelta;

			*TESRenderWindow::StateFlags = FlagsBuffer;
			_PRIMARYRENDERER->RotateReferenceSelection(Delta.x, X, Y, Z);
			*TESRenderWindow::StateFlags = FlagsState;
		}

		void SelectionControlsOSDLayer::ScaleSelection(bool Local)
		{
			if (ImGui::IsItemActive() == false)
				return;

			ImVec2 Delta = ImGui::GetIO().MouseDelta;
			_PRIMARYRENDERER->ScaleReferenceSelection(Delta.x, Local == false);
		}

		void SelectionControlsOSDLayer::AlignSelection(bool Position, bool Rotation)
		{
			if (AlignmentAxisX == false && AlignmentAxisY == false && AlignmentAxisZ == false)
				return;

			TESObjectREFR* AlignRef = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, nullptr, false);
			if (AlignRef)
			{
				const Vector3& AlignPos = *AlignRef->GetPosition();
				const Vector3& AlignRot = *AlignRef->GetRotation();

				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					if (Ref == AlignRef)
						continue;

					if (Position)
					{
						Vector3 NewPos(*Ref->GetPosition());
						if (AlignmentAxisX)
							NewPos.x = AlignPos.x;
						if (AlignmentAxisY)
							NewPos.y = AlignPos.y;
						if (AlignmentAxisZ)
							NewPos.z = AlignPos.z;

						Ref->SetPosition(NewPos);
					}

					if (Rotation)
					{
						Vector3 NewRot(*Ref->GetRotation());
						if (AlignmentAxisX)
							NewRot.x = AlignRot.x;
						if (AlignmentAxisY)
							NewRot.y = AlignRot.y;
						if (AlignmentAxisZ)
							NewRot.z = AlignRot.z;

						Ref->SetRotation(NewRot, true);
					}

					Ref->SetFromActiveFile(true);
				}

				NotificationOSDLayer::Instance.ShowNotification("Selection %s aligned to %08X", Position ? "position" : "rotation", AlignRef->formID);
				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefAlignment);
			}
		}

		SelectionControlsOSDLayer::SelectionControlsOSDLayer() :
			IRenderWindowOSDLayer(settings::renderWindowOSD::kShowSelectionControls, IRenderWindowOSDLayer::kPriority_SelectionControls),
			WindowState()
		{
			LocalTransformation = 1;
			AlignmentAxisX = AlignmentAxisY = AlignmentAxisZ = true;
		}

		SelectionControlsOSDLayer::~SelectionControlsOSDLayer()
		{
			;//
		}

#define CURRENT_COLWIDTH_MINUS_10	ImGui::GetColumnWidth() - 10

		void SelectionControlsOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			if (_RENDERSEL->selectionCount == 0 || _RENDERSEL->selectionList == nullptr || _RENDERSEL->selectionList->Data == nullptr)
				return;
			else if (TESDialog::GetActiveFormEditDialog(_RENDERSEL->selectionList->Data))
				return;

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (_RENDERSEL->selectionCount == 0 ? 0.1f : 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
			ImGui::SetNextWindowPosCenter(ImGuiSetCond_FirstUseEver);
			if (!ImGui::Begin("Quick Controls", nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				ImGui::PopStyleVar(2);
				return;
			}

			WindowState.Update(GUI);

			bool SingleSel = _RENDERSEL->selectionCount == 1;

			if (WindowState.TextInput.GotFocus || WindowState.DragInput.DragBegin)
			{
				if (_RENDERSEL->selectionCount)
				{
					// recording just once crashes the editor. also, this code only seems to support undos
					// ### investigate
					_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_RefChange3D, _RENDERSEL->selectionList);
					_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_RefChange3D, _RENDERSEL->selectionList);
				}
			}

			bool ReturnEarly = false;
			if (SingleSel)
			{
				TESObjectREFR* ThisRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
				SME_ASSERT(ThisRef);

				Vector3 Position(ThisRef->position);
				Vector3 Rotation(ThisRef->rotation);
				float Scale = ThisRef->scale;
				Rotation.Scale(57.2957763671875);

				ImGui::Columns(2, "Ref Info", false);
				{
					ImGui::TextWrapped("%s(%08X) [%s]", ThisRef->editorID.Size() ? ThisRef->editorID.c_str() : "",
									   ThisRef->formID,
									   TESForm::GetFormTypeIDLongName(ThisRef->baseForm->formType));
					ImGui::NextColumn();

					float ColWidth = ImGui::GetColumnWidth() - 20;
					ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0, 0.6f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0, 0.8f, 0.8f));
					{
						if (ImGui::Button("Reference##edit_ref", ImVec2(ColWidth / 2 - 10, 20)))
						{
							EditReference(ThisRef);

							ImGui::PopStyleColor(3);
							ImGui::End();
							ImGui::PopStyleVar(2);
							return;
						}
						ImGui::SameLine();

						if (ImGui::Button("Base Form##edit_base", ImVec2(ColWidth / 2 - 10, 20)))
							EditBaseForm(ThisRef);
					}
					ImGui::PopStyleColor(3);

					ImGui::SameLine(); ImGui::TextDisabled("(?)  ");
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Click and drag the buttons to transform the selection.\nCTRL + click to directly edit the values.\n\nThe \"Reference\" and \"Base Form\" buttons open their corresponding edit dialogs.");

					ImGui::NextColumn();
				}
				ImGui::Columns(1);
				ImGui::NewLine();

				ImGui::DragFloat3("Position##single_pos", &Position.x, 1, 0, 0, "%.2f");
				ImGui::DragFloat3("Rotation##single_rot", &Rotation.x, 1, 0, 0, "%.2f");
				ImGui::SliderFloat("Scale##single_scale", &Scale, 0.01f, 10.f);
				ImGui::NewLine();

				if (WindowState.DragInput.Active || WindowState.TextInput.Active ||
					WindowState.TextInput.LostFocus || WindowState.DragInput.DragEnd)
				{
					ThisRef->SetPosition(Position);
					ThisRef->SetRotation(Rotation);
					ThisRef->SetScale(Scale);
				}

				if (WindowState.TextInput.LostFocus || WindowState.DragInput.DragEnd)
					ThisRef->SetFromActiveFile(true);
			}
			else if (_RENDERSEL->selectionCount)
			{
				ImGui::Text("Transformation:"); ImGui::SameLine(0, 15);
				ImGui::RadioButton("Local", &LocalTransformation, 1); ImGui::SameLine(0, 10); ImGui::RadioButton("Global", &LocalTransformation, 0);
				ImGui::SameLine(ImGui::GetWindowWidth() - 40); ImGui::TextDisabled("(?)");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Click and drag the buttons to transform the selection");
				ImGui::NewLine();

				float Width = 0;
				ImGui::Columns(4, "Position + Rotation", false);
				{
					ImGui::Button("X##multi_pos_x", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)); MoveSelection(true, false, false); DrawDragTrail(); ImGui::NextColumn();
					ImGui::Button("Y##multi_pos_y", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)); MoveSelection(false, true, false); DrawDragTrail(); ImGui::NextColumn();
					ImGui::Button("Z##multi_pos_z", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)); MoveSelection(false, false, true); DrawDragTrail(); ImGui::NextColumn();

					ImGui::Text("Position"); ImGui::NextColumn();

					ImGui::Button("X##multi_rot_x", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)); RotateSelection(LocalTransformation, true, false, false);
					DrawDragTrail(); ImGui::NextColumn();
					ImGui::Button("Y##multi_rot_y", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)); RotateSelection(LocalTransformation, false, true, false);
					DrawDragTrail(); ImGui::NextColumn();
					ImGui::Button("Z##multi_rot_z", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)); RotateSelection(LocalTransformation, false, false, true);
					DrawDragTrail(); ImGui::NextColumn();

					Width = ImGui::GetColumnOffset();
					ImGui::Text("Rotation"); ImGui::NextColumn();
				}
				ImGui::Columns(1);

				ImGui::Button("Scale##multi_scale", ImVec2(Width - 10, 20)); ScaleSelection(LocalTransformation);
				ImGui::NewLine();

				ImGui::Columns(3, "Grouping", false);
				{
					ImGui::Text("Grouping:"); ImGui::NextColumn();
					if (ImGui::Button("Group##group_sel", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
						_RENDERWIN_MGR.InvokeContextMenuTool(IDC_RENDERWINDOWCONTEXT_GROUP);
					ImGui::NextColumn();
					if (ImGui::Button("Ungroup##ungroup_sel", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
						_RENDERWIN_MGR.InvokeContextMenuTool(IDC_RENDERWINDOWCONTEXT_UNGROUP);
					ImGui::NextColumn();
				}
				ImGui::Columns(1);
			}

			ImGui::Columns(3, "Alignment", false);
			{
				ImGui::Text("Alignment: "); ImGui::NextColumn();
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Aligns the selection to the position/rotation of another reference");
				if (ImGui::Button("Postion##align_pos", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
					AlignSelection(true, false);
				ImGui::NextColumn();
				if (ImGui::Button("Rotation##align_rot", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
					AlignSelection(false, true);
				ImGui::NextColumn();

				ImGui::Checkbox("Align X-axis", &AlignmentAxisX); ImGui::NextColumn();
				ImGui::Checkbox("Align Y-axis", &AlignmentAxisY); ImGui::NextColumn();
				ImGui::Checkbox("Align Z-axis", &AlignmentAxisZ); ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::Separator();

			ImGui::Columns(4, "Flags", false);
			{
				ImGui::Text("Flags: "); ImGui::NextColumn();
				if (SingleSel)
				{
					TESObjectREFR* ThisRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
					SME_ASSERT(ThisRef);

					bool Persistent = ThisRef->IsQuestItem();
					bool VWD = ThisRef->IsVWD();
					bool Disabled = ThisRef->IsInitiallyDisabled();
					bool Changed = false;

					if (ImGui::Checkbox("Persistent##flag_persistent", &Persistent))
						ThisRef->SetPersistent(Persistent), Changed = true;
					ImGui::NextColumn();
					if (ImGui::Checkbox("VWD##flag_vwd", &VWD))
						ThisRef->SetVWD(VWD), Changed = true;
					ImGui::NextColumn();
					if (ImGui::Checkbox("Disabled##flag_disabled", &Disabled))
						ThisRef->SetInitiallyDisabled(Disabled), Changed = true;
					ImGui::NextColumn();

					if (Changed)
						ThisRef->SetFromActiveFile(true);
				}
				else
				{
					bool Persistent = true, VWD = true, Disabled = true;
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						if (Ref->IsQuestItem() == false)
							Persistent = false;
						if (Ref->IsVWD() == false)
							VWD = false;
						if (Ref->IsInitiallyDisabled() == false)
							Disabled = false;
					}

					bool SetPersistent = false, SetVWD = false, SetDisabled = false;
					if (ImGui::Checkbox("Persistent##flag_persistent", &Persistent))
						SetPersistent = true;
					ImGui::NextColumn();
					if (ImGui::Checkbox("VWD##flag_vwd", &VWD))
						SetVWD = true;
					ImGui::NextColumn();
					if (ImGui::Checkbox("Disabled##flag_disabled", &Disabled))
						SetDisabled = true;
					ImGui::NextColumn();

					if (SetPersistent || SetDisabled || SetVWD)
					{
						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
						{
							TESObjectREFR* ThisRef = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
							if (SetPersistent)
								ThisRef->SetPersistent(Persistent);
							else if (SetVWD)
								ThisRef->SetVWD(VWD);
							else if (SetDisabled)
								ThisRef->SetInitiallyDisabled(Disabled);

							ThisRef->SetFromActiveFile(true);
						}
					}
				}
			}
			ImGui::Columns(1);

			if (ImGui::CollapsingHeader("Enable State Parent##linkedref_header"))
			{
				bool Modified = false;
				if (SingleSel)
				{
					TESObjectREFR* ThisRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
					SME_ASSERT(ThisRef);

					BSExtraData* xData = ThisRef->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
					ExtraEnableStateParent* xParent = nullptr;
					if (xData)
						xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);

					ImGui::Columns(3, "Enable State Parent##linkedref_cols_single", false);
					{
						ImGui::Text("Linked Ref:"); ImGui::NextColumn();
						if (xParent)
						{
							ImGui::Text("%s(%08X)",
								((xParent->parent->editorID.Size()) ? (xParent->parent->editorID.c_str()) : ("")), xParent->parent->formID);
						}
						else
							ImGui::Text("None");
						ImGui::NextColumn();

						if (xParent)
							ImGui::Text("Opposite: %s   ", (xParent->oppositeState ? "Yes" : "No"));
						else if (ImGui::Button("Set", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
						{
							TESObjectREFR* NewParent = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, xParent ? xParent->parent : nullptr, true);
							if (NewParent)
								ThisRef->extraData.ModExtraEnableStateParent(NewParent), Modified = true;
						}
						ImGui::NextColumn();

						if (xParent && ImGui::Button("Change", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
						{
							TESObjectREFR* NewParent = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, xParent ? xParent->parent : nullptr, true);
							if (NewParent)
								ThisRef->extraData.ModExtraEnableStateParent(NewParent), Modified = true;
						}
						ImGui::NextColumn();

						if (xParent)
						{
							if (ImGui::Button("Clear", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
								ThisRef->extraData.ModExtraEnableStateParent(nullptr), Modified = true;
						}
						ImGui::NextColumn();

						if (xParent)
						{
							if (ImGui::Button("Toggle Opposite", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
								ThisRef->SetExtraEnableStateParentOppositeState(xParent->oppositeState == 0), Modified = true;
						}
						ImGui::NextColumn();
					}
					ImGui::Columns(1);

					if (Modified)
						ThisRef->SetFromActiveFile(true);
				}
				else
				{
					ImGui::Columns(4, "Enable State Parent##linkedref_cols_multi", false);
					{
						bool SameParent = true;
						TESObjectREFR* ParentRefMark = nullptr;
						bool OppositeStateMark = false;
						bool MarksSet = false;

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
						{
							TESObjectREFR* ParentComp = nullptr;
							bool OppositeComp = false;

							TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
							BSExtraData* xData = Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
							if (xData)
							{
								ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
								ParentComp = xParent->parent;
								OppositeComp = xParent->oppositeState == 1;
							}

							if (MarksSet == false)
							{
								ParentRefMark = ParentComp;
								OppositeStateMark = OppositeComp;
								MarksSet = true;
							}

							if (ParentRefMark != ParentComp || OppositeStateMark != OppositeComp)
							{
								SameParent = false;
								break;
							}
						}

						ImGui::Text("Linked Ref:"); ImGui::NextColumn();
						if (SameParent && ParentRefMark)
						{
							ImGui::Text("%s(%08X)",
								((ParentRefMark->editorID.Size()) ? (ParentRefMark->editorID.c_str()) : ("")), ParentRefMark->formID);
						}
						else if (SameParent && ParentRefMark == nullptr)
							ImGui::Text("None");
						else if (SameParent == false)
							ImGui::Text("(multiple)");
						ImGui::NextColumn();
						if (SameParent)
							ImGui::Text("Opposite: %s   ", (OppositeStateMark ? "Yes" : "No"));
						ImGui::NextColumn();
						ImGui::NextColumn();

						char Buffer[0x100] = { 0 };
						FORMAT_STR(Buffer, "%s", (SameParent && ParentRefMark ? "Change" : "Set"));
						TESObjectREFR* NewParent = nullptr;
						bool ClearParent = false;
						bool ToggleOpposite = false;
						bool NewOpposite = false;

						if (ImGui::Button(Buffer, ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
							NewParent = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, SameParent ? ParentRefMark : nullptr, true);
						ImGui::NextColumn();

						if (ImGui::Button("Clear", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
							ClearParent = true;
						ImGui::NextColumn();

						if (ImGui::Button("Set Opposite", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
							ToggleOpposite = NewOpposite = true;
						ImGui::NextColumn();

						if (ImGui::Button("Clear Opposite", ImVec2(CURRENT_COLWIDTH_MINUS_10, 20)))
							ToggleOpposite = true;
						ImGui::NextColumn();

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
						{
							TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
							if (NewParent)
								Ref->extraData.ModExtraEnableStateParent(NewParent);

							if (ClearParent)
								Ref->extraData.ModExtraEnableStateParent(nullptr);

							if (ToggleOpposite)
								Ref->SetExtraEnableStateParentOppositeState(NewOpposite);

							if (NewParent || ClearParent || ToggleOpposite)
								Ref->SetFromActiveFile(true);
						}
					}
					ImGui::Columns(1);
				}
			}


			ImGui::End();
			ImGui::PopStyleVar(2);
		}

		bool SelectionControlsOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}
	}
}