#include "ReferenceEditorOSDLayer.h"
#include "Render Window\RenderWindowManager.h"
#include "Achievements.h"
#include "RenderWindowActions.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"

namespace cse
{
	namespace renderWindow
	{
		ReferenceEditorOSDLayer		ReferenceEditorOSDLayer::Instance;


		IReferenceEditComponent::DrawContext::DrawContext(const OSDLayerStateData& ParentState, RenderWindowOSD* OSD, ImGuiDX9* GUI, const TESObjectREFRArrayT& Selection) :
			ParentState(ParentState), OSD(OSD), GUI(GUI), Selection(Selection)
		{
			;//
		}

		bool IReferenceEditComponent::DrawContext::SelectionHasInvalidType(UInt8 ValidFormType) const
		{
			for (const auto& Ref : Selection)
			{
				if (Ref->formType != ValidFormType)
					return true;
			}

			return false;
		}

		bool IReferenceEditComponent::DrawContext::HasMultipleSelection() const
		{
			return Selection.size() > 1;
		}

		bool Reference3DEditComponent::Vector3Equals(const Vector3& a, const Vector3& b) const
		{
			return FloatEqual(a.x, b.x) && FloatEqual(a.y, b.y) && FloatEqual(a.z, b.z);
		}

		void Reference3DEditComponent::BuildForSingleSelection(DrawContext* Context)
		{
			SME_ASSERT(Context->HasMultipleSelection() == false);

			auto ThisRef = CS_CAST(Context->Selection.at(0), TESForm, TESObjectREFR);
			SME_ASSERT(ThisRef);

			Vector3 Position(ThisRef->position);
			Vector3 Rotation(ThisRef->rotation);
			float Scale = ThisRef->scale;
			Rotation.Scale(REFR_RAD2DEG);

			bool PositionChanged = ImGui::DragFloat3("Position##single_pos", &Position.x, 1.f, 0, 0, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			bool RotationChanged = ImGui::DragFloat3("Rotation##single_rot", &Rotation.x, 0.5f, 0, 360.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			bool ScaleChanged = ImGui::SliderFloat("Scale##single_scale", &Scale, 0.01f, 10.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::NewLine();

			if (PositionChanged && !Vector3Equals(Position, ThisRef->position))
			{
				ThisRef->SetPosition(Position);
				ThisRef->SetFromActiveFile(true);
			}

			if (RotationChanged)
			{
				Vector3 RotationBuf(ThisRef->rotation);
				RotationBuf.Scale(REFR_RAD2DEG);
				if (!Vector3Equals(Rotation, RotationBuf))
				{
					ThisRef->SetRotation(Rotation);
					ThisRef->SetFromActiveFile(true);
				}
			}

			if (ScaleChanged && !FloatEqual(Scale, ThisRef->scale))
			{
				ThisRef->SetScale(Scale);
				ThisRef->SetFromActiveFile(true);
			}
		}

		void Reference3DEditComponent::BuildForMultipleSelection(DrawContext* Context)
		{
			SME_ASSERT(Context->HasMultipleSelection());

			ImGui::TextUnformatted("Transformation:");
			ImGui::SameLine(0, 15);
			ImGui::RadioButton("Local", &TransformationMode, kTransformationMode_Local);
			ImGui::SameLine(0, 10);
			ImGui::RadioButton("Global", &TransformationMode, kTransformationMode_Global);

			ImGui::NewLine();

			if (ImGui::BeginTable("##transform_table", 4, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableNextRow();
				{
					ImGui::TableNextColumn();
					{
						ImGui::Button("X##multi_pos_x", ImVec2(-FLT_MIN, 0.0f));
						MoveSelection(true, false, false);
					}
					ImGui::TableNextColumn();
					{
						ImGui::Button("Y##multi_pos_y", ImVec2(-FLT_MIN, 0.0f));
						MoveSelection(false, true, false);
					}
					ImGui::TableNextColumn();
					{
						ImGui::Button("Z##multi_pos_z", ImVec2(-FLT_MIN, 0.0f));
						MoveSelection(false, false, true);
					}
					ImGui::TableNextColumn();
					{
						ImGui::TextUnformatted("Position");
					}
				}

				ImGui::TableNextRow();
				{
					ImGui::TableNextColumn();
					{
						ImGui::Button("X##multi_rot_x", ImVec2(-FLT_MIN, 0.0f));
						RotateSelection(TransformationMode, true, false, false);
					}
					ImGui::TableNextColumn();
					{
						ImGui::Button("Y##multi_rot_y", ImVec2(-FLT_MIN, 0.0f));
						RotateSelection(TransformationMode, false, true, false);
					}
					ImGui::TableNextColumn();
					{
						ImGui::Button("Z##multi_rot_z", ImVec2(-FLT_MIN, 0.0f));
						RotateSelection(TransformationMode, false, false, true);
					}
					ImGui::TableNextColumn();
					{
						ImGui::TextUnformatted("Rotation");
					}
				}

				ImGui::EndTable();
			}

			if (ImGui::BeginTable("##scale_table", 1, ImGuiTableFlags_SizingPolicyStretchX | ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableNextRow();
				{
					ImGui::TableNextColumn();
					{
						ImGui::Button("Scale##multi_scale", ImVec2(-FLT_MIN, 0.0f));
						ScaleSelection(TransformationMode == kTransformationMode_Local);
					}
				}

				ImGui::EndTable();
			}

			ImGui::NewLine();

			if (ImGui::BeginTable("##group_table", 3, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableNextRow();
				{
					ImGui::TableNextColumn();
					{
						ImGui::TextUnformatted("Grouping:");
					}
					ImGui::TableNextColumn();
					{
						if (ImGui::Button("Group##group_sel", ImVec2(-FLT_MIN, 0.0f)))
							actions::GroupSelection();
					}
					ImGui::TableNextColumn();
					{
						if (ImGui::Button("Ungroup##ungroup_sel", ImVec2(-FLT_MIN, 0.0f)))
							actions::UngroupSelection();
					}
				}

				ImGui::EndTable();
			}
		}

		void Reference3DEditComponent::MoveSelection(bool X, bool Y, bool Z) const
		{
			if (ImGui::IsItemActive() == false)
				return;

			ImVec2 Delta = ImGui::GetIO().MouseDelta;
			_PRIMARYRENDERER->MoveReferenceSelection(Delta.x, Delta.x, X, Y, Z);
		}

		void Reference3DEditComponent::RotateSelection(bool Local, bool X, bool Y, bool Z) const
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

		void Reference3DEditComponent::ScaleSelection(bool Local) const
		{
			if (ImGui::IsItemActive() == false)
				return;

			ImVec2 Delta = ImGui::GetIO().MouseDelta;
			_PRIMARYRENDERER->ScaleReferenceSelection(Delta.x, Local == false);
		}

		void Reference3DEditComponent::AlignSelection(bool Position, bool Rotation) const
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

		void Reference3DEditComponent::SaveSelectionToUndoStack() const
		{
			if (_RENDERSEL->selectionCount)
			{
				// also, this code only seems to support undos
				// ### investigate
				_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_RefChange3D, _RENDERSEL->selectionList);
				_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_RefChange3D, _RENDERSEL->selectionList);
			}
		}

		void Reference3DEditComponent::DrawDragTrail(DrawContext* Context) const
		{
			auto& io = ImGui::GetIO();

			if (Context->ParentState.DragInput.Active)
				ImGui::GetForegroundDrawList()->AddLine(io.MouseClickedPos[ImGuiMouseButton_Left], io.MousePos, ImGui::GetColorU32(ImGuiCol_PlotLines), 4.0f); // Draw a line between the button and the mouse cursor
		}

		Reference3DEditComponent::Reference3DEditComponent()
		{
			TransformationMode = kTransformationMode_Local;
			AlignmentAxisX = AlignmentAxisY = AlignmentAxisZ = true;
		}

		bool Reference3DEditComponent::SupportsReference(TESObjectREFR* Ref) const
		{
			return true;
		}

		void Reference3DEditComponent::Draw(DrawContext* Context)
		{
			if (ImGui::BeginTabItem("3D Data"))
			{
				ImGui::ShowHelpPopup("Click and drag the buttons to transform the selection.%s", !Context->HasMultipleSelection() ? "\nCTRL + click to directly edit the values." : "");

				if (Context->ParentState.TextInput.GotFocus || Context->ParentState.DragInput.DragBegin)
					SaveSelectionToUndoStack();

				if (Context->HasMultipleSelection())
					BuildForMultipleSelection(Context);
				else
					BuildForSingleSelection(Context);

				if (ImGui::BeginTable("##alignment_table", 3, ImGuiTableFlags_NoSavedSettings))
				{
					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							ImGui::TextUnformatted("Alignment:");
							ImGui::SameLine(0, 3);
							ImGui::ShowHelpPopup("Aligns the selection to the position/rotation of another reference");
						}
						ImGui::TableNextColumn();
						{
							if (ImGui::Button("Position##align_pos", ImVec2(-FLT_MIN, 0.0f)))
								AlignSelection(true, false);
						}
						ImGui::TableNextColumn();
						{
							if (ImGui::Button("Rotation##align_rot", ImVec2(-FLT_MIN, 0.0f)))
								AlignSelection(false, true);
						}
					}

					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						ImGui::Checkbox("Align X-axis", &AlignmentAxisX);
						ImGui::TableNextColumn();
						ImGui::Checkbox("Align Y-axis", &AlignmentAxisY);
						ImGui::TableNextColumn();
						ImGui::Checkbox("Align Z-axis", &AlignmentAxisZ);

					}

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}
		}

		bool ReferenceFlagsEditComponent::SupportsReference(TESObjectREFR* Ref) const
		{
			return true;
		}

		void ReferenceFlagsEditComponent::Draw(DrawContext* Context)
		{
			if (ImGui::BeginTabItem("Flags"))
			{
				int InitiallyDisabled = -1, VWD = -1, Persistent = -1;
				for (int i = 0; i < Context->Selection.size(); ++i)
				{
					auto Ref = Context->Selection[i];

					bool ThisDisabled = Ref->IsInitiallyDisabled();
					bool ThisVWD = Ref->IsVWD();
					bool ThisPersistent = Ref->IsQuestItem();

					if (i == 0)
					{
						InitiallyDisabled = ThisDisabled;
						VWD = ThisVWD;
						Persistent = ThisPersistent;
						continue;
					}

					if (InitiallyDisabled != static_cast<int>(ThisDisabled))
						InitiallyDisabled = -1;
					if (VWD != static_cast<int>(ThisVWD))
						VWD = -1;
					if (Persistent != static_cast<int>(ThisPersistent))
						Persistent = -1;
				}

				bool SetPersistent = ImGui::CheckBoxTristate("Persistent", &Persistent);
				bool SetVWD = ImGui::CheckBoxTristate("Visible When Distant", &VWD);
				bool SetDisabled = ImGui::CheckBoxTristate("Initially Disabled", &InitiallyDisabled);

				for (const auto& Ref : Context->Selection)
				{
					if (SetPersistent)
						Ref->SetQuestItem(Persistent == 1);

					if (SetVWD)
						Ref->SetVWD(VWD == 1);

					if (SetDisabled)
						Ref->SetInitiallyDisabled(InitiallyDisabled == 1);

					if (SetPersistent || SetVWD || SetDisabled)
						Ref->SetFromActiveFile(true);
				}

				ImGui::EndTabItem();
			}
		}

		bool ReferenceEnableParentEditComponent::SupportsReference(TESObjectREFR* Ref) const
		{
			return true;
		}

		void ReferenceEnableParentEditComponent::Draw(DrawContext* Context)
		{
			TESObjectREFR* NewParent = nullptr;
			TESObjectREFR* ViewParent = nullptr;
			bool ClearParent = false;
			bool ToggleOpposite = false;
			bool NewOpposite = false;

			if (ImGui::BeginTabItem("Enable Parent"))
			{
				if (ImGui::BeginTable("##enable_parent_table", 2, ImGuiTableFlags_NoSavedSettings))
				{
					constexpr auto FirstColumnWidth = 75, SecondColumnWidth = 150;
					ImGui::TableSetupColumn("First", ImGuiTableColumnFlags_NoResize, FirstColumnWidth);
					ImGui::TableSetupColumn("Second", ImGuiTableColumnFlags_NoResize, SecondColumnWidth);

					bool SameParent = true, SameOpposite = true;
					TESObjectREFR* ParentRefMark = nullptr;
					bool OppositeStateMark = false;
					bool MarksSet = false;

					for (const auto& Ref : Context->Selection)
					{
						TESObjectREFR* ParentComp = nullptr;
						bool OppositeComp = false;

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

						if (ParentRefMark != ParentComp)
							SameParent = false;

						if (OppositeStateMark != OppositeComp)
							SameOpposite = false;
					}

					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							ImGui::TextWrapped("Linked Reference:");
						}
						ImGui::TableNextColumn();
						{
							char Buffer[100];
							if (SameParent && ParentRefMark)
							{
								FORMAT_STR(Buffer, "%s (%08X)",
										IRenderWindowOSDLayer::Helpers::GetRefEditorID(ParentRefMark).c_str(),
										ParentRefMark->formID);
							}
							else if (SameParent && ParentRefMark == nullptr)
								FORMAT_STR(Buffer, "(None)");
							else if (SameParent == false)
								FORMAT_STR(Buffer, "(Multiple)");

							if (ImGui::Button(Buffer, ImVec2(SecondColumnWidth, 0.0f)))
							{
								NewParent = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle,
																				SameParent ? ParentRefMark : nullptr, true);
							}

							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(Buffer);
						}
					}

					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						ImGui::TableNextColumn();
						{
							auto CanClear = !(SameParent && ParentRefMark == nullptr);
							auto CanView = SameParent && ParentRefMark;

							if (CanClear)
							{
								if (ImGui::Button("Clear##clear_linked_ref", ImVec2(CanView ? SecondColumnWidth / 2 : SecondColumnWidth, 0.0f)))
								{
									ClearParent = true;
								}
							}

							if (SameParent && ParentRefMark)
							{
								ImGui::SameLine(0, 1);
								if (ImGui::Button("View##view_linked_ref", ImVec2(CanClear ? SecondColumnWidth / 2 : SecondColumnWidth, 0.0f)))
								{
									ViewParent = ParentRefMark;
								}
							}
						}
					}

					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							ImGui::TextWrapped("Set Enable State to Opposite of Parent:");
						}
						ImGui::TableNextColumn();
						{
							int CheckboxState = SameOpposite ? OppositeStateMark : -1;
							if (ImGui::CheckBoxTristate("##opposite_state", &CheckboxState))
							{
								ToggleOpposite = true;
								NewOpposite = CheckboxState == 1;
							}
						}
					}

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}

			for (auto& Ref : Context->Selection)
			{
				if (NewParent)
					Ref->extraData.ModExtraEnableStateParent(NewParent);

				if (ClearParent)
					Ref->extraData.ModExtraEnableStateParent(nullptr);

				if (ToggleOpposite)
					Ref->SetExtraEnableStateParentOppositeState(NewOpposite);

				if (NewParent || ClearParent || ToggleOpposite)
					Ref->SetFromActiveFile(true);
			}

			if (ViewParent)
			{
				_RENDERWIN_MGR.GetDeferredExecutor()->QueueTask([=]() {
					_TES->LoadCellIntoViewPort(ViewParent->GetPosition(), ViewParent);
				});
			}
		}

		void ReferenceOwnershipEditComponent::UpdateCachedData(DrawContext* Context)
		{
			CachedData.RefSelectionChanged = false;
			if (CachedData.LastRefSelection.size() != Context->Selection.size())
				CachedData.RefSelectionChanged = true;

			if (!CachedData.RefSelectionChanged)
			{
				for (const auto Ref : Context->Selection)
				{
					if (CachedData.LastRefSelection.find(Ref) == CachedData.LastRefSelection.cend())
					{
						CachedData.RefSelectionChanged = true;
						break;
					}
				}
			}

			if (CachedData.RefSelectionChanged)
			{
				CachedData.LastRefSelection.clear();
				for (const auto Ref : Context->Selection)
					CachedData.LastRefSelection.emplace(Ref);

				CachedData.LastOwnershipSelection = kOwnership_None;

				CachedData.BaselineOwnershipData.Owner = nullptr;
				CachedData.BaselineOwnershipData.Global = nullptr;
				CachedData.BaselineOwnershipData.Rank = -1;
			}
		}

		void ReferenceOwnershipEditComponent::BeginSelectionPopup(SelectionType Type, TESForm* FormParam)
		{
			SME_ASSERT(SelectionState.PopupState == SelectionPopupState::NotShowing);

			SelectionState.PopupState = SelectionPopupState::Showing;
			SelectionState.Type = Type;
			SelectionState.SelectedItem.Form = nullptr;
			SelectionState.SelectedItem.Rank = -1;
			SelectionState.FormParam = FormParam;
			SelectionState.Filter.Clear();
			SelectionState.PopupName.clear();

			switch (Type)
			{
			case SelectionType::NPC:
				SelectionState.PopupName = "Select NPC";
				break;
			case SelectionType::Faction:
				SelectionState.PopupName = "Select Faction";
				break;
			case SelectionType::Global:
				SelectionState.PopupName = "Select Global";
				break;
			case SelectionType::Rank:
				SelectionState.PopupName = "Select Faction Rank";
				break;

			}

			SelectionState.PopupName += "##SelectionPopup";
			ImGui::OpenPopup(SelectionState.PopupName.c_str());
		}

		bool ReferenceOwnershipEditComponent::DrawSelectionPopup()
		{
			bool SelectionMade = false;
			auto InsertRowForForm = [&](TESForm* Form) {
				std::string EditorID("(None)");
				char FormID[9];
				FORMAT_STR(FormID, "(None)");
				if (Form)
				{
					EditorID = IRenderWindowOSDLayer::Helpers::GetFormEditorID(Form);
					FORMAT_STR(FormID, "%08X", Form->formID);
				}

				if (Form && !SelectionState.Filter.PassFilter(IRenderWindowOSDLayer::Helpers::GetFormEditorID(Form).c_str()) && !SelectionState.Filter.PassFilter(FormID))
					return;

				ImGui::PushID(Form ? Form->formID : 0);
				ImGui::TableNextRow();
				{
					bool RowSelected = SelectionState.SelectedItem.Form == Form;
					ImGui::TableNextColumn();
					{
						std::string EditorID("(None)");
						if (Form)
							EditorID = IRenderWindowOSDLayer::Helpers::GetFormEditorID(Form);

						if (ImGui::Selectable(EditorID.c_str(), &RowSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_DontClosePopups))
						{
							SelectionState.SelectedItem.Form = Form;
							RowSelected = true;
						}
					}
					ImGui::TableNextColumn();
					{
						ImGui::Selectable(FormID, &RowSelected);
					}

					if (!SelectionMade && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						SME_ASSERT(SelectionState.PopupState == SelectionPopupState::Showing);
						SelectionMade = true;
						SelectionState.PopupState = SelectionPopupState::OK;
					}
				}
				ImGui::PopID();
			};
			auto InsertRowForRank = [&](TESFaction::RankData* RankData, SInt32 Rank) {
				if (RankData && RankData->maleRank.Size() && !SelectionState.Filter.PassFilter(RankData->maleRank.c_str()))
					return;
				else if (RankData && RankData->femaleRank.Size() && !SelectionState.Filter.PassFilter(RankData->femaleRank.c_str()))
					return;

				ImGui::PushID(Rank);
				ImGui::TableNextRow();
				{
					bool RowSelected = SelectionState.SelectedItem.Rank == Rank;
					ImGui::TableNextColumn();
					{
						auto DisplayText = "(None)";
						if (RankData)
							DisplayText = RankData->maleRank.c_str();

						if (ImGui::Selectable(DisplayText, &RowSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_DontClosePopups))
						{
							SelectionState.SelectedItem.Rank = Rank;
							RowSelected = true;
						}
					}
					ImGui::TableNextColumn();
					{
						auto DisplayText = "(None)";
						if (RankData)
							DisplayText = RankData->femaleRank.c_str();

						ImGui::Selectable(DisplayText, &RowSelected);
					}

					if (!SelectionMade && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						SME_ASSERT(SelectionState.PopupState == SelectionPopupState::Showing);
						SelectionMade = true;
						SelectionState.PopupState = SelectionPopupState::OK;
					}
				}
				ImGui::PopID();
			};

			ImGui::SetNextWindowPosCenter(ImGuiCond_Appearing);
			if (ImGui::BeginPopupModal(SelectionState.PopupName.c_str(), nullptr))
			{
				SME_ASSERT(SelectionState.PopupState == SelectionPopupState::Showing);

				SelectionState.Filter.Draw();
				ImGui::Dummy(ImVec2(0, 5));

				bool RankSelection = SelectionState.Type == SelectionType::Rank;

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 2));
				ImGui::BeginChild("##child_window", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
				ImGui::PopStyleVar();
				{
					if (ImGui::BeginTable("##selection_popup_table", 2,
						ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Scroll | ImGuiTableFlags_Borders,
						ImVec2(-5, -35)))
					{
						ImGui::TableSetupColumn(RankSelection ? "Male Title" : "Editor ID", ImGuiTableColumnFlags_WidthFixed, 200);
						ImGui::TableSetupColumn(RankSelection ? "Female Title" : "Form ID", ImGuiTableColumnFlags_WidthFixed, 75);

						ImGui::TableSetupScrollFreeze(0, 2);
						ImGui::TableHeadersRow();

						// insert 'None' item
						if (SelectionState.Type == SelectionType::Rank)
							InsertRowForRank(nullptr, -1);
						else
							InsertRowForForm(nullptr);

						switch (SelectionState.Type)
						{
						case SelectionType::NPC:
						{
							for (auto Itr = _DATAHANDLER->objects->first; Itr; Itr = Itr->next)
							{
								if (Itr->formType == TESForm::kFormType_NPC)
									InsertRowForForm(Itr);
							}

							break;
						}
						case SelectionType::Faction:
						case SelectionType::Global:
						{
							tList<TESForm>* SourceList = nullptr;
							if (SelectionState.Type == SelectionType::Faction)
								SourceList = reinterpret_cast<tList<TESForm>*>(&_DATAHANDLER->factions);
							else
								SourceList = reinterpret_cast<tList<TESForm>*>(&_DATAHANDLER->globals);

							for (auto Itr = SourceList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
								InsertRowForForm(Itr.Get());

							break;
						}
						case SelectionType::Rank:
						{
							auto Faction = CS_CAST(SelectionState.FormParam, TESForm, TESFaction);
							SME_ASSERT(Faction);

							int RankIndex = 0;
							for (auto Itr = Faction->rankDataList.Begin(); !Itr.End() && Itr.Get(); ++Itr, ++RankIndex)
							{
								auto RankData = Itr.Get();
								bool NoRankTitles = RankData->maleRank.Size() == 0 && RankData->femaleRank.Size() == 0;

								if (NoRankTitles)
									continue;

								InsertRowForRank(RankData, RankIndex);
							}

							break;
						}
						}

						ImGui::EndTable();
					}

					if (ImGui::Button("OK", ImVec2(75, 0)))
					{
						SME_ASSERT(SelectionState.PopupState == SelectionPopupState::Showing);
						SelectionMade = true;
						SelectionState.PopupState = SelectionPopupState::OK;
					}

					ImGui::SameLine(0, 5);

					if (ImGui::Button("Cancel", ImVec2(75, 0)))
					{
						SME_ASSERT(SelectionState.PopupState == SelectionPopupState::Showing);
						SelectionMade = true;
						SelectionState.PopupState = SelectionPopupState::Cancel;
					}
				}
				ImGui::EndChild();

				if (SelectionMade)
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
			}

			return SelectionMade;
		}

		void ReferenceOwnershipEditComponent::EndSelectionPopup()
		{
			SME_ASSERT(SelectionState.PopupState == SelectionPopupState::OK || SelectionState.PopupState == SelectionPopupState::Cancel);
			SelectionState.PopupState = SelectionPopupState::NotShowing;
		}

		bool ReferenceOwnershipEditComponent::SupportsReference(TESObjectREFR* Ref) const
		{
			switch (Ref->baseForm->formType)
			{
			case TESForm::FormType::kFormType_LeveledCreature:
			case TESForm::FormType::kFormType_NPC:
			case TESForm::FormType::kFormType_Static:
			case TESForm::FormType::kFormType_Tree:
				return false;
			default:
				return true;
			}
		}

		void ReferenceOwnershipEditComponent::Draw(DrawContext* Context)
		{
			static const char* const kOwnershipTypeStrings[kOwnership__MAX] {
				"None", "NPC", "Faction", "(Multiple)"
			};

			static const char* const kOwnershipSecondParamStrings[kSecondParam__MAX] {
				"Global", "Rank"
			};

			auto RemoveOwner = [=]() {
				for (const auto& Ref : Context->Selection)
				{
					if (!SupportsReference(Ref))
						continue;

					if (Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Ownership)
						|| Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Global)
						|| Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Rank))
						Ref->SetFromActiveFile(true);

					Ref->extraData.ModExtraOwnership(nullptr);
					Ref->extraData.ModExtraGlobal(nullptr);
					Ref->extraData.ModExtraRank(-1);
				}
			};
			auto SetOwner = [=](TESForm* Owner) {
				for (const auto& Ref : Context->Selection)
				{
					if (!SupportsReference(Ref))
						continue;

					auto xOwner = CS_CAST(Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Ownership), BSExtraData, ExtraOwnership);
					if (xOwner == nullptr && Owner == nullptr)
						continue;
					else if (xOwner && xOwner->owner == Owner)
						continue;

					Ref->extraData.ModExtraOwnership(Owner);
					if (Owner)
					{
						if (Owner->formType != TESForm::kFormType_NPC)
							Ref->extraData.ModExtraGlobal(nullptr);
						else
							Ref->extraData.ModExtraRank(-1);
					}
					else
					{
						Ref->extraData.ModExtraGlobal(nullptr);
						Ref->extraData.ModExtraRank(-1);
					}

					Ref->SetFromActiveFile(true);
				}
			};
			auto SetGlobal = [=](TESGlobal* Global) {
				for (const auto& Ref : Context->Selection)
				{
					if (!SupportsReference(Ref))
						continue;

					auto xOwner = CS_CAST(Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Ownership), BSExtraData, ExtraOwnership);
					if (Global && (xOwner == nullptr || xOwner->owner->formType != TESForm::kFormType_NPC))
						continue;

					auto xGlobal = CS_CAST(Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Global), BSExtraData, ExtraGlobal);
					if (xGlobal == nullptr && Global == nullptr)
						continue;
					else if (xGlobal && xGlobal->globalVar == Global)
						continue;

					Ref->extraData.ModExtraGlobal(Global);
					Ref->extraData.ModExtraRank(-1);
					Ref->SetFromActiveFile(true);
				}
			};
			auto SetRank = [=](SInt32 Rank) {
				for (const auto& Ref : Context->Selection)
				{
					if (!SupportsReference(Ref))
						continue;

					auto xOwner = CS_CAST(Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Ownership), BSExtraData, ExtraOwnership);
					if (Rank != -1 && (xOwner == nullptr || xOwner->owner->formType != TESForm::kFormType_Faction))
						continue;

					auto xRank = CS_CAST(Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Rank), BSExtraData, ExtraRank);
					if (xRank == nullptr && Rank == -1)
						continue;
					else if (xRank && xRank->rank == Rank)
						continue;

					Ref->extraData.ModExtraRank(Rank);
					Ref->extraData.ModExtraGlobal(nullptr);
					Ref->SetFromActiveFile(true);
				}
			};

			if (ImGui::BeginTabItem("Ownership"))
			{
				UpdateCachedData(Context);

				auto& BaselineOwnershipData = CachedData.BaselineOwnershipData;
				bool InitBaseline = true;
				bool MultipleValues = false;

				// since the calls to OpenPopup and Begin(Modal)Popup need to be at the same (window stack) level
				// we need to hoist the call to the former out of its original call site
				std::function<void()> BeginSelectionPopupCallable;

				if (CachedData.RefSelectionChanged)
				{
					for (const auto& Ref : Context->Selection)
					{
						if (!SupportsReference(Ref))
							continue;

						auto xOwner = CS_CAST(Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Ownership), BSExtraData, ExtraOwnership);
						auto xGlobal = CS_CAST(Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Global), BSExtraData, ExtraGlobal);
						auto xRank = CS_CAST(Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_Rank), BSExtraData, ExtraRank);

						if (InitBaseline)
						{
							InitBaseline = false;

							if (xOwner)
								BaselineOwnershipData.Owner = xOwner->owner;
							if (xGlobal)
								BaselineOwnershipData.Global = xGlobal->globalVar;
							if (xRank)
								BaselineOwnershipData.Rank = xRank->rank;
						}

						if ((xOwner && BaselineOwnershipData.Owner != xOwner->owner) || (xOwner == nullptr && BaselineOwnershipData.Owner))
						{
							MultipleValues = true;
							break;
						}

						if ((xGlobal && BaselineOwnershipData.Global != xGlobal->globalVar) || (xGlobal == nullptr && BaselineOwnershipData.Global))
						{
							MultipleValues = true;
							break;
						}

						if ((xRank && BaselineOwnershipData.Rank != xRank->rank) || (xRank == nullptr && BaselineOwnershipData.Rank != -1))
						{
							MultipleValues = true;
							break;
						}
					}
				}

				SME_ASSERT(CachedData.RefSelectionChanged == false || InitBaseline == false);

				auto CurrentOwnershipType = CachedData.LastOwnershipSelection;
				if (CachedData.RefSelectionChanged)
				{
					if (MultipleValues)
						CurrentOwnershipType = kOwnership_Multiple;
					else if (BaselineOwnershipData.Owner && BaselineOwnershipData.Owner->formType == TESForm::kFormType_Faction)
						CurrentOwnershipType = kOwnership_Faction;
					else if (BaselineOwnershipData.Owner && BaselineOwnershipData.Owner->formType == TESForm::kFormType_NPC)
						CurrentOwnershipType = kOwnership_NPC;

					CachedData.LastOwnershipSelection = CurrentOwnershipType;
				}

				int OwnershipTypeSelection = kOwnership__MAX;

				if (ImGui::BeginTable("##ownership_table", 2, ImGuiTableFlags_NoSavedSettings))
				{
					constexpr auto FirstColumnWidth = 75, SecondColumnWidth = 200;
					ImGui::TableSetupColumn("First", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, FirstColumnWidth);
					ImGui::TableSetupColumn("Second", ImGuiTableColumnFlags_WidthAlwaysAutoResize | ImGuiTableColumnFlags_NoResize, SecondColumnWidth);

					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							ImGui::TextWrapped("Owner:");
						}
						ImGui::TableNextColumn();
						{
							ImGui::PushItemWidth(SecondColumnWidth);
							if (ImGui::BeginCombo("##ownership_type", kOwnershipTypeStrings[CurrentOwnershipType]))
							{
								for (int i = kOwnership_None; i < kOwnership__MAX; ++i)
								{
									// the Multiple option is only used in the preview
									if (i == kOwnership_Multiple)
										continue;

									ImGui::PushID(i);
									if (ImGui::Selectable(kOwnershipTypeStrings[i], i == CurrentOwnershipType))
										OwnershipTypeSelection = i;

									ImGui::PopID();
								}

								ImGui::EndCombo();
							}
							ImGui::PopItemWidth();
						}
					}


					if (CurrentOwnershipType == kOwnership_None || CurrentOwnershipType == kOwnership_Multiple)
						goto ENDTABLE;

					// everything beyond this point will only be displayed for (single) refs with the same ownership data
					SME_ASSERT(MultipleValues == false);

					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							ImGui::TextWrapped("%s:", kOwnershipTypeStrings[CurrentOwnershipType]);
						}
						ImGui::TableNextColumn();
						{
							char Buffer[100];
							FORMAT_STR(Buffer, "(None)");

							if (BaselineOwnershipData.Owner)
							{
								FORMAT_STR(Buffer, "%s (%08X)",
									IRenderWindowOSDLayer::Helpers::GetFormEditorID(BaselineOwnershipData.Owner).c_str(),
									BaselineOwnershipData.Owner->formID);
							}

							std::string DisplayText(Buffer);
							DisplayText += "##owner_form";
							if (ImGui::Button(DisplayText.c_str(), ImVec2(SecondColumnWidth, 0.0f)))
								BeginSelectionPopupCallable = [&]() { BeginSelectionPopup(CurrentOwnershipType == kOwnership_NPC ? SelectionType::NPC : SelectionType::Faction); };

							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(Buffer);
						}
					}

					if (BaselineOwnershipData.Owner == nullptr)
						goto ENDTABLE;

					ImGui::TableNextRow();
					{
						int SecondParam = CurrentOwnershipType == kOwnership_NPC ? kSecondParam_Global : kSecondParam_Rank;

						ImGui::TableNextColumn();
						{
							ImGui::TextWrapped("%s:", kOwnershipSecondParamStrings[SecondParam]);
						}
						ImGui::TableNextColumn();
						{
							char Buffer[100];
							FORMAT_STR(Buffer, "(None)");
							bool NoValue = true;

							switch (SecondParam)
							{
							case kSecondParam_Global:
							{
								if (BaselineOwnershipData.Global)
								{
									NoValue = false;
									FORMAT_STR(Buffer, "%s (%08X)",
											IRenderWindowOSDLayer::Helpers::GetFormEditorID(BaselineOwnershipData.Global).c_str(),
											BaselineOwnershipData.Global->formID);
								}

								break;
							}
							case kSecondParam_Rank:
							{
								if (BaselineOwnershipData.Rank != -1)
								{
									NoValue = false;
									auto Faction = CS_CAST(BaselineOwnershipData.Owner, TESForm, TESFaction);
									SME_ASSERT(Faction && BaselineOwnershipData.Rank < Faction->rankDataList.Count());

									auto RankMaleTitle = Faction->rankDataList.GetNthItem(BaselineOwnershipData.Rank)->maleRank.c_str();
									if (RankMaleTitle == nullptr)
										RankMaleTitle = "<No_Male_Rank_Title>";

									FORMAT_STR(Buffer, "%s (%d)", RankMaleTitle, BaselineOwnershipData.Rank);
								}

								break;
							}
							}

							std::string DisplayText(Buffer);
							DisplayText += "##second_param";
							if (ImGui::Button(DisplayText.c_str(), ImVec2(SecondColumnWidth, 0.0f)))
								BeginSelectionPopupCallable = [&]() { BeginSelectionPopup(SecondParam == kSecondParam_Global ? SelectionType::Global : SelectionType::Rank, BaselineOwnershipData.Owner); };

							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(Buffer);
						}
					}

				ENDTABLE:
					ImGui::EndTable();
				}

				if (OwnershipTypeSelection != kOwnership__MAX && CurrentOwnershipType != OwnershipTypeSelection)
				{
					RemoveOwner();
					CachedData.LastOwnershipSelection = OwnershipTypeSelection;
					BaselineOwnershipData.Owner = nullptr;
					BaselineOwnershipData.Global = nullptr;
					BaselineOwnershipData.Rank = -1;
				}

				if (BeginSelectionPopupCallable)
					BeginSelectionPopupCallable();

				if (DrawSelectionPopup())
				{
					if (SelectionState.PopupState == SelectionPopupState::OK)
					{
						switch (SelectionState.Type)
						{
						case SelectionType::NPC:
						case SelectionType::Faction:
							BaselineOwnershipData.Owner = SelectionState.SelectedItem.Form;
							SetOwner(BaselineOwnershipData.Owner);
							break;
						case SelectionType::Global:
							BaselineOwnershipData.Global = CS_CAST(SelectionState.SelectedItem.Form, TESForm, TESGlobal);
							SetGlobal(BaselineOwnershipData.Global);
							break;
						case SelectionType::Rank:
							BaselineOwnershipData.Rank = SelectionState.SelectedItem.Rank;
							SetRank(BaselineOwnershipData.Rank);
							break;
						}
					}

					EndSelectionPopup();
				}

				ImGui::EndTabItem();
			}
		}

		bool ReferenceExtraDataEditComponent::HasExtraDataCharge(TESObjectREFR* Ref) const
		{
			return CS_CAST(Ref->baseForm, TESForm, TESEnchantableForm) != nullptr;
		}

		bool ReferenceExtraDataEditComponent::HasExtraDataHealth(TESObjectREFR* Ref) const
		{
			switch (Ref->baseForm->formType)
			{
			case TESForm::kFormType_Creature:
			case TESForm::kFormType_NPC:
				return false;
			default:
				return CS_CAST(Ref->baseForm, TESForm, TESHealthForm) != nullptr;
			}
		}

		bool ReferenceExtraDataEditComponent::HasExtraDataTimeLeft(TESObjectREFR* Ref) const
		{
			auto LightForm = CS_CAST(Ref->baseForm, TESForm, TESObjectLIGH);
			return LightForm && LightForm->IsCarriable();
		}

		bool ReferenceExtraDataEditComponent::HasExtraDataCount(TESObjectREFR* Ref) const
		{
			switch (Ref->baseForm->formType)
			{
			case TESForm::kFormType_Tree:
			case TESForm::kFormType_Static:
			case TESForm::kFormType_Container:
			case TESForm::kFormType_LeveledCreature:
			case TESForm::kFormType_Activator:
			case TESForm::kFormType_Creature:
			case TESForm::kFormType_NPC:
			case TESForm::kFormType_Door:
				return false;
			default:
				return true;
			}
		}

		bool ReferenceExtraDataEditComponent::HasExtraDataSoul(TESObjectREFR* Ref) const
		{
			return Ref->baseForm->formType == TESForm::FormType::kFormType_SoulGem;
		}

		bool ReferenceExtraDataEditComponent::SupportsReference(TESObjectREFR* Ref) const
		{
			return HasExtraDataCharge(Ref) || HasExtraDataHealth(Ref) || HasExtraDataTimeLeft(Ref) || HasExtraDataCount(Ref) || HasExtraDataSoul(Ref);
		}

#define INITIALIZE_EXTRADATA_VALUES(ref_variable, extra_data_type_name, bs_extra_data_value_member_name)	\
		if (HasExtraData##extra_data_type_name(ref_variable)) \
		{ \
			auto& Value = SelectionValues.at(ExtraDataType::##extra_data_type_name); \
			auto xData = CS_CAST(Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_##extra_data_type_name), BSExtraData, Extra##extra_data_type_name); \
			if (Value.Type == SelectionValueType::Single) \
			{ \
				if (xData == nullptr || xData->##bs_extra_data_value_member_name != Value.##extra_data_type_name) \
					Value.Type = SelectionValueType::Multiple; \
			} \
			else if (Value.Type == SelectionValueType::Invalid) \
			{ \
				if (xData == nullptr) \
					Value.Type = SelectionValueType::None; \
				else \
				{ \
					Value.Type = SelectionValueType::Single; \
					Value.##extra_data_type_name = xData->##bs_extra_data_value_member_name; \
				} \
			} \
		}

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)
#define DRAW_EXTRADATA_INPUT_EDIT_CONTROLS(extra_data_type_name, desc_string, format_string, min_val, max_val)	\
		{ \
			auto& ExtraDataSelValue = SelectionValues.at(ExtraDataType::##extra_data_type_name); \
			if (ExtraDataSelValue.Type != SelectionValueType::Invalid) \
			{ \
				ImGui::TableNextRow(); \
				{ \
					float CurrentValue = ExtraDataSelValue.##extra_data_type_name; \
					const char* FormatString = ExtraDataSelValue.Type == SelectionValueType::Multiple ? "(Multiple)" : format_string; \
					char Buffer[100]; \
					bool Modified = false; \
					ImGui::TableNextColumn(); \
					{ \
						ImGui::TextWrapped("%s:", STRINGIZE(desc_string)); \
					} \
					ImGui::TableNextColumn(); \
					{ \
						ImGui::PushItemWidth(SecondColumnWidth); \
						FORMAT_STR(Buffer, "##%s_input", STRINGIZE(extra_data_type_name)); \
						if (ImGui::DragFloat(Buffer, &CurrentValue, 1.f, min_val, max_val, FormatString)) \
							Modified = true; \
						ImGui::PopItemWidth(); \
					} \
					ImGui::TableNextColumn(); \
					{ \
						FORMAT_STR(Buffer, "Default##%s_default", STRINGIZE(extra_data_type_name)); \
						if (ImGui::Button(Buffer)) \
						{ \
							Modified = true; \
							CurrentValue = ExtraDataValue().##extra_data_type_name; \
						} \
					}\
					if (Modified) \
					{ \
						if (CurrentValue < min_val) \
							CurrentValue = min_val; \
						else if (CurrentValue > max_val) \
							CurrentValue = max_val; \
						if (!FloatEqual(CurrentValue, ExtraDataSelValue.extra_data_type_name)) \
						{ \
							for (const auto& Ref : Context->Selection) \
							{ \
								if (!HasExtraData##extra_data_type_name(Ref)) \
									continue; \
								Ref->ModExtra##extra_data_type_name(CurrentValue); \
								Ref->SetFromActiveFile(true); \
							} \
						} \
					} \
				} \
			} \
		}


		void ReferenceExtraDataEditComponent::Draw(DrawContext* Context)
		{
			static const char* const kSoulLevelStrings[TESSoulGem::kSoulLevel__MAX] {
				"None", "Petty", "Lesser", "Common", "Greater", "Grand",
			};

			enum class SelectionValueType { Invalid, None, Single, Multiple };
			enum class ExtraDataType
			{
				Count,
				Health,
				Charge,
				TimeLeft,
				Soul
			};

			struct ExtraDataValue
			{
				SelectionValueType	Type = SelectionValueType::Invalid;
				SInt16				Count = 1;
				float				Health = 100.f;
				float				Charge = 0.f;
				float				TimeLeft = -1.f;
				SInt8				Soul = TESSoulGem::kSoulLevel_None;
			};

			std::map<ExtraDataType, ExtraDataValue> SelectionValues {
				{ ExtraDataType::Count, ExtraDataValue() },
				{ ExtraDataType::Health, ExtraDataValue() },
				{ ExtraDataType::Charge, ExtraDataValue() },
				{ ExtraDataType::TimeLeft, ExtraDataValue() },
				{ ExtraDataType::Soul, ExtraDataValue() }
			};

			if (ImGui::BeginTabItem("Extra Data"))
			{
				for (const auto& Ref : Context->Selection)
				{
					INITIALIZE_EXTRADATA_VALUES(Ref, Count, count);
					INITIALIZE_EXTRADATA_VALUES(Ref, Health, health);
					INITIALIZE_EXTRADATA_VALUES(Ref, Charge, charge);
					INITIALIZE_EXTRADATA_VALUES(Ref, TimeLeft, time);

					// soul gems need to be handled slightly differently
					if (HasExtraDataSoul(Ref))
					{
						auto& Value = SelectionValues.at(ExtraDataType::Soul);
						auto SoulLevel = Ref->GetSoulLevel();	// soul gems will always fallback to the soul level of the base form
						if (Value.Type == SelectionValueType::Single)
						{
							if (SoulLevel != Value.Soul)
								Value.Type = SelectionValueType::Multiple;
						}
						else if (Value.Type == SelectionValueType::Invalid)
						{
							Value.Type = SelectionValueType::Single;
							Value.Soul = SoulLevel;
						}
					}
				}

				ImGui::ShowHelpPopup("Click and drag to edit value.\nHold SHIFT/ALT for faster/slower edit.\nDouble-click or CTRL+click to input value.");

				if (ImGui::BeginTable("##extradata_table", 3, ImGuiTableFlags_NoSavedSettings))
				{
					constexpr auto FirstColumnWidth = 75, SecondColumnWidth = 150, ThirdColumnWidth = 50;
					ImGui::TableSetupColumn("First", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, FirstColumnWidth);
					ImGui::TableSetupColumn("Second", ImGuiTableColumnFlags_WidthAlwaysAutoResize | ImGuiTableColumnFlags_NoResize, SecondColumnWidth);
					ImGui::TableSetupColumn("Third", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoResize, ThirdColumnWidth);

					DRAW_EXTRADATA_INPUT_EDIT_CONTROLS(Count, Count, "%.0f", 1, 65535);
					DRAW_EXTRADATA_INPUT_EDIT_CONTROLS(Health, Health, "%.3f", 0, 4294967300);
					DRAW_EXTRADATA_INPUT_EDIT_CONTROLS(Charge, Charge, "%.0f", 0, 65535);
					DRAW_EXTRADATA_INPUT_EDIT_CONTROLS(TimeLeft, Time Left, "%.3f", -1, FLT_MAX);

					// soul level is displayed separately as a dropdown
					{
						auto& ExtraDataSelValue = SelectionValues.at(ExtraDataType::Soul);
						if (ExtraDataSelValue.Type != SelectionValueType::Invalid)
						{
							ImGui::TableNextRow();
							{
								bool SelectionMade = false;
								auto OldValue = ExtraDataSelValue.Soul;

								ImGui::TableNextColumn();
								{
									ImGui::TextWrapped("Soul Level:");
								}
								ImGui::TableNextColumn();
								{
									ImGui::PushItemWidth(SecondColumnWidth);
									auto Preview = ExtraDataSelValue.Type != SelectionValueType::Multiple ? kSoulLevelStrings[ExtraDataSelValue.Soul] : "(Multiple)";
									if (ImGui::BeginCombo("##soul_level", Preview))
									{
										for (int i = TESSoulGem::kSoulLevel_None; i < TESSoulGem::kSoulLevel__MAX; ++i)
										{
											ImGui::PushID(i);
											if (ImGui::Selectable(kSoulLevelStrings[i], i == ExtraDataSelValue.Soul && ExtraDataSelValue.Type != SelectionValueType::Multiple))
											{
												SelectionMade = true;
												ExtraDataSelValue.Soul = i;
											}
											ImGui::PopID();
										}

										ImGui::EndCombo();
									}
									ImGui::PopItemWidth();
								}
								ImGui::TableNextColumn();
								{
									if (ImGui::Button("Default##soul_default"))
									{
										SelectionMade = true;
										ExtraDataSelValue.Soul = ExtraDataValue().Soul;
									}
								}

								if (SelectionMade)
								{
									if (ExtraDataSelValue.Soul != OldValue)
									{
										for (const auto& Ref : Context->Selection)
										{
											if (!HasExtraDataSoul(Ref))
												continue;

											Ref->ModExtraSoul(ExtraDataSelValue.Soul);
											Ref->SetFromActiveFile(true);
										}
									}
								}
							}
						}
					}

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}
		}

#undef INITIALIZE_EXTRADATA_VALUES
#undef DRAW_EXTRADATA_EDIT_CONTROLS

		ReferenceEditorOSDLayer::ReferenceEditorOSDLayer() :
			IRenderWindowOSDLayer(&settings::renderWindowOSD::kShowRefBatchEditor),
			WindowState(),
			RefListOnlyShowSelection(false),
			RefListContextMenuSelection(nullptr),
			HideRefList(false)
		{
			EditComponents.emplace_back(new Reference3DEditComponent);
			EditComponents.emplace_back(new ReferenceFlagsEditComponent);
			EditComponents.emplace_back(new ReferenceEnableParentEditComponent);
			EditComponents.emplace_back(new ReferenceOwnershipEditComponent);
			EditComponents.emplace_back(new ReferenceExtraDataEditComponent);
			CurrentSelection.reserve(0x100);
		}

		ReferenceEditorOSDLayer::~ReferenceEditorOSDLayer()
		{
			;//
		}


		void ReferenceEditorOSDLayer::UpdateCurrentSelection()
		{
			CurrentSelection.clear();
			for (const auto& Ref : _RENDERWIN_MGR.GetActiveRefs())
				CurrentSelection.emplace_back(Ref, _RENDERSEL->HasObject(Ref));
		}

		void ReferenceEditorOSDLayer::BuildReferenceListChildWindow(std::vector<std::pair<TESObjectREFR*, bool>>& References)
		{
			TESObjectREFR* RefToShow = nullptr;
			TESObjectREFR* ContextMenuRef = nullptr;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 2));
			ImGui::BeginChild("##reference_list_child_window", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);
			ImGui::PopStyleVar();
			{
				ImGui::ShowHelpPopup("CTRL + click on a reference below to center the camera on it");
				ImGui::SameLine(0, 5);
				RefListFilterHelper.Draw("Filter");
				ImGui::Dummy(ImVec2(0, 5));

				constexpr auto NumColumns = 3;
				if (ImGui::BeginTable("##references_table", NumColumns,
					ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Hideable | ImGuiTableFlags_Scroll,
					ImVec2(-5, -35)))
				{
					ImGui::TableSetupColumn("Reference", ImGuiTableColumnFlags_WidthStretch, 225);
					ImGui::TableSetupColumn("Form ID", ImGuiTableColumnFlags_WidthStretch, 25);
					ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 75);

					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableHeadersRow();

					char Buffer[0x100];
					auto HoveredColumn = ImGui::TableGetHoveredColumn();

					for (auto& Itr : References)
					{
						auto Ref = Itr.first;
						if (RefListOnlyShowSelection && !Itr.second)
							continue;

						auto EditorID = Ref->GetEditorID() ? Ref->GetEditorID() : Ref->baseForm->GetEditorID();
						if (EditorID == nullptr)
							EditorID = "<no_editor_id>";

						FORMAT_STR(Buffer, "%s %08X %s", EditorID, Ref->formID, TESForm::GetFormTypeIDLongName(Ref->baseForm->formType));
						if (RefListFilterHelper.PassFilter(Buffer) == false)
							continue;

						ImGui::PushID(Ref->formID);
						{
							ImGui::TableNextRow();

							ImGui::TableNextColumn();
							{
								FORMAT_STR(Buffer, "%s##selection_toggle", EditorID);
								bool OldValue = Itr.second;
								if (ImGui::Checkbox(Buffer, &Itr.second) && RefToShow == nullptr)
								{
									if (ImGui::GetIO().KeyCtrl)
									{
										Itr.second = OldValue;
										RefToShow = Ref;
									}
									else if (Itr.second == true)
										_RENDERSEL->AddToSelection(Ref, true);
									else
										_RENDERSEL->RemoveFromSelection(Ref, true);
								}
								if (ImGui::IsItemHovered())
								{
									if (ContextMenuRef == nullptr && HoveredColumn != -1 && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
										ContextMenuRef = Ref;
									else
										ImGui::SetTooltip(EditorID);
								}
							}

							ImGui::TableNextColumn();
							{
								FORMAT_STR(Buffer, "%08X", Ref->formID);
								ImGui::Text("%s", Buffer);
								if (ImGui::IsItemHovered() && ImGui::TableGetColumnIsVisible())
									ImGui::SetTooltip(Buffer);
							}

							ImGui::TableNextColumn();
							{
								auto FormType = TESForm::GetFormTypeIDLongName(Ref->baseForm->formType);
								ImGui::Text("%s", FormType);
								if (ImGui::IsItemHovered() && ImGui::TableGetColumnIsVisible())
									ImGui::SetTooltip(FormType);
							}

							if (!ImGui::IsAnyItemHovered() && ContextMenuRef == nullptr && HoveredColumn != -1 && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
								ContextMenuRef = Ref;
						}
						ImGui::PopID();
					}

					for (int Col = 0; Col < NumColumns + 1; ++Col)
					{
						ImGui::PushID(Col);
						{
							if (HoveredColumn == Col && ContextMenuRef)
							{
								ImGui::OpenPopup("##context_menu");
								RefListContextMenuSelection = ContextMenuRef;
							}

							if (ImGui::BeginPopup("##context_menu"))
							{
								SME_ASSERT(RefListContextMenuSelection);

								if (ImGui::Selectable("Open Reference Edit Dialog"))
									TESDialog::ShowFormEditDialog(RefListContextMenuSelection);
								if (ImGui::Selectable("Open Base Form Edit Dialog"))
									TESDialog::ShowFormEditDialog(RefListContextMenuSelection->baseForm);

								ImGui::Separator();

								if (ImGui::Selectable("Center Camera"))
									RefToShow = RefListContextMenuSelection;

								ImGui::EndPopup();
							}
						}
						ImGui::PopID();
					}

					ImGui::EndTable();
				}

				ImGui::Dummy(ImVec2(0, 5));
				ImGui::Checkbox("Hide unselected references", &RefListOnlyShowSelection);
			}
			ImGui::EndChild();

			if (RefToShow)
			{
				_RENDERWIN_MGR.GetDeferredExecutor()->QueueTask([=]() {
					// create a new selection with just the one reference
					// needed for the correct positioning of the camera
					auto TempSelection = TESObjectSelection::CreateInstance();
					auto OrgSelection = _RENDERSEL;

					TempSelection->AddToSelection(RefToShow);

					// prevent the current selection from being modified during the load process
					*TESRenderWindow::CenteringCamera = 1;

					_RENDERSEL = TempSelection;
					_TES->LoadCellIntoViewPort(RefToShow->GetPosition(), RefToShow);
					_RENDERSEL = OrgSelection;

					TempSelection->DeleteInstance();
				});
			}
		}

		bool ReferenceEditorOSDLayer::BuildEditComponentTabs(RefSelectionArrayT& References, RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			std::vector<bool> ComponentEnabled;
			for (int i = 0; i < EditComponents.size(); ++i)
				ComponentEnabled.emplace_back(false);

			TESObjectREFRArrayT SelectedRefs;
			for (const auto& Itr : References)
			{
				if (Itr.second)
				{
					for (int i = 0; i < EditComponents.size(); ++i)
					{
						if (!ComponentEnabled[i] && EditComponents[i]->SupportsReference(Itr.first))
							ComponentEnabled[i] = true;
					}
					SelectedRefs.emplace_back(Itr.first);
				}
			}

			if (SelectedRefs.empty())
				return false;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 5));
			if (ImGui::BeginTabBar("##EditComponentTabs", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip))
			{
				IReferenceEditComponent::DrawContext DC(WindowState, OSD, GUI, SelectedRefs);
				for (int i = 0; i < EditComponents.size(); ++i)
				{
					if (ComponentEnabled[i])
						EditComponents[i]->Draw(&DC);
				}

				ImGui::EndTabBar();
			}
			ImGui::PopStyleVar();

			return true;
		}

		void ReferenceEditorOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			bool NothingToSee = *TESRenderWindow::ActiveCell == nullptr;
			if (NothingToSee)
				return;

			ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(1280, 720));
			if (!ImGui::Begin("Reference Editor", nullptr, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar))
			{
				ImGui::End();
				return;
			}

			WindowState.Update(GUI);

			UpdateCurrentSelection();

			if (ImGui::BeginTable("##main_content_table", HideRefList ? 1 : 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingPolicyStretchX ))
			{
				if (!HideRefList)
					ImGui::TableSetupColumn("RefList", ImGuiTableColumnFlags_WidthStretch, 300);
				ImGui::TableSetupColumn("EditComponents", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				{
					if (!HideRefList)
					{
						ImGui::TableNextColumn();
						{
							BuildReferenceListChildWindow(CurrentSelection);
						}
					}

					ImGui::TableNextColumn();
					{
						ImGui::Dummy(ImVec2(5, 0));
						ImGui::SameLine();

						ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 2));
						ImGui::BeginChild("##edit_component_child_window", ImVec2(-5, 0), ImGuiWindowFlags_AlwaysUseWindowPadding);
						ImGui::PopStyleVar();
						{
							if (ImGui::ArrowButton("##ToggleRefList", HideRefList ? ImGuiDir_Right : ImGuiDir_Left))
								HideRefList = HideRefList == false;
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("%s Reference List", HideRefList ? "Show" : "Hide");

							ImGui::SameLine(0, 15);
							if (_RENDERSEL->selectionCount == 1)
							{
								auto ThisRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
								char FormIdStr[100];
								FORMAT_STR(FormIdStr, "%08X", ThisRef->formID);
								ImGui::TextWrapped("%s - %s%s (%s)",
									TESForm::GetFormTypeIDLongName(ThisRef->baseForm->formType),
									ThisRef->editorID.Size() ? ThisRef->editorID.c_str() : FormIdStr,
									ThisRef->IsActive() ? "*" : "",
									ThisRef->baseForm->editorID.Size() ? ThisRef->baseForm->editorID.c_str() : "<no_editor_id>");
							}
							else if (_RENDERSEL->selectionCount > 1)
							{
								ImGui::Text("%d references selected", _RENDERSEL->selectionCount);
							}
							else
								ImGui::NewLine();


							if (!BuildEditComponentTabs(CurrentSelection, OSD, GUI))
							{
								ImGui::Text("No references selected...");
							}
						}
						ImGui::EndChild();
					}
				}

				ImGui::EndTable();
			}

			ImGui::End();
		}

		bool ReferenceEditorOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}


	}
}