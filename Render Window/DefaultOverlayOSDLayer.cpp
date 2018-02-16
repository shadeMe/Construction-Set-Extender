#include "DefaultOverlayOSDLayer.h"
#include "Render Window\RenderWindowManager.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"

namespace cse
{
	namespace renderWindow
	{
		DefaultOverlayOSDLayer		DefaultOverlayOSDLayer::Instance;

		DefaultOverlayOSDLayer::DefaultOverlayOSDLayer() :
			IRenderWindowOSDLayer(&settings::renderWindowOSD::kShowInfoOverlay)
		{
			;//
		}

		DefaultOverlayOSDLayer::~DefaultOverlayOSDLayer()
		{
			;//
		}

		void DefaultOverlayOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			static const float FirstCoulmnWidth = 150;

			ImGui::SetNextWindowPos(ImVec2(10, 10));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 6));
			if (!ImGui::Begin("Default Info Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				ImGui::PopStyleVar();
				return;
			}

			TESObjectCELL* CurrentCell = _TES->currentInteriorCell;
			if (CurrentCell == nullptr)
				CurrentCell = *TESRenderWindow::ActiveCell;

			int R = 0, G = 0, B = 0;
			const NiVector3* CameraCoords = _PRIMARYRENDERER->GetCameraWorldTranslate();

			ImGui::Columns(2, "Cell and Cam Data", false);
			{
				if (CurrentCell)
				{
					char Buffer[0x50] = { 0 };
					if (CurrentCell->IsInterior())
						FORMAT_STR(Buffer, "%s (%08X)   ", CurrentCell->GetEditorID(), CurrentCell->formID);
					else
					{
						FORMAT_STR(Buffer, "%s %d,%d (%08X)   ", CurrentCell->GetEditorID(), CurrentCell->cellData.coords->x,
								   CurrentCell->cellData.coords->y, CurrentCell->formID);
					}

					ImGui::Text("Current Cell:"); ImGui::SameLine(); ImGui::TextDisabled("  (?)"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
					if (ImGui::IsItemHovered())
					{
						std::string Geom(TESRenderWindow::GetCellGeomDescription(CurrentCell).c_str());
						SME::StringHelpers::Tokenizer Extractor(Geom.c_str(), ",");
						std::string CurrentArg, Extract;
						int Count = 0;
						while (Extractor.NextToken(CurrentArg) != -1)
						{
							switch (Count)
							{
							case 1:
							case 2:
							case 3:
								Extract += "  " + CurrentArg.substr(1) + "\n";
								break;
							}

							Count++;
						}
						ImGui::SetTooltip("Geometry:\n%s", Extract.c_str());
					}
					ImGui::Text("%s  ", Buffer);
					const char* EditModeIcon = nullptr;
					const char* EditModeLabel = nullptr;

					if (*TESRenderWindow::PathGridEditFlag)
					{
						EditModeIcon = " " ICON_MD_LINEAR_SCALE;
						EditModeLabel = "Path Grid";
					}
					else if (*TESRenderWindow::LandscapeEditFlag)
					{
						EditModeIcon = " " ICON_MD_LANDSCAPE;
						EditModeLabel = "Landscape";
					}
					else
					{
						EditModeIcon = " " ICON_MD_PERSON;
						EditModeLabel = "Reference";
					}

					ImGui::SameLine(0, 5);
					ImGui::Text("%s  ", EditModeIcon);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("%s Edit Mode", EditModeLabel);
					ImGui::NextColumn();

					ImGui::Text("Camera:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
					ImGui::Text("%.0f, %0.f, %0.f  ", CameraCoords->x, CameraCoords->y, CameraCoords->z); ImGui::NextColumn();
					ImGui::Separator();
				}
			}
			ImGui::Columns(1);
			SME::StringHelpers::GetRGB(settings::renderWindowOSD::kColorSelectionStats().s, R, G, B);

			ImGui::Columns(2, "Selection Data", false);
			{
				if (_RENDERSEL->selectionCount)
				{
					char Buffer[0x200] = { 0 };

					if (_RENDERSEL->selectionCount > 1)
					{
						std::vector<std::string> Groups;
						int GroupCount = _RENDERWIN_MGR.GetGroupManager()->GetSelectionGroups(_RENDERSEL, Groups);

						ImGui::TextColored(ImColor(R, G, B), "%d Objects Selected", _RENDERSEL->selectionCount); ImGui::NextColumn();
						if (GroupCount)
						{
							ImGui::TextColored(ImColor(R, G, B), "%d Group%s", GroupCount, GroupCount == 1 ? "" : "s"); ImGui::SameLine(0, 5);
							ImGui::TextDisabled("(?)   ");
							char CountBuffer[0x100] = { 0 };
							char Buffer[0x100] = { 0 };
							std::string MouseOver;
							int Count = 0;
							static constexpr int kMaxDiplayCount = 10;
							for (auto& Itr : Groups)
							{
								FORMAT_STR(Buffer, "%s\n", Itr.c_str());
								MouseOver.append(Buffer);
								Count++;

								if (Count == kMaxDiplayCount)
									break;
							}

							if (Count == kMaxDiplayCount)
							{
								FORMAT_STR(CountBuffer, "\n+%d more groups", Groups.size() - Count);
								MouseOver.append(CountBuffer);
							}

							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(MouseOver.c_str());
						}

						ImGui::NextColumn();
						ImGui::TextColored(ImColor(R, G, B), "Nominal Center:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
						ImGui::TextColored(ImColor(R, G, B), "%.3f, %.3f, %.3f   ", _RENDERSEL->selectionPositionVectorSum.x,
										   _RENDERSEL->selectionPositionVectorSum.y, _RENDERSEL->selectionPositionVectorSum.z); ImGui::NextColumn();
					}
					else
					{
						TESObjectREFR* Selection = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
						if (Selection)	// in the off-chance that the selection contains a regular form
						{
							ImGui::TextColored(ImColor(R, G, B), "%s(%08X)%s", ((Selection->editorID.Size()) ? (Selection->editorID.c_str()) : ("")), Selection->formID, Selection->IsActive() ? "*" : ""); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%s", TESForm::GetFormTypeIDLongName(Selection->baseForm->formType)); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Base Form:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%s(%08X)",
								((Selection->baseForm->editorID.Size()) ? (Selection->baseForm->editorID.c_str()) : ("")), Selection->baseForm->formID); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Position:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%.03f, %.03f, %.03f   ", Selection->position.x, Selection->position.y, Selection->position.z); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Rotation:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%.03f, %.03f, %.03f   ",
											   Selection->rotation.x * REFR_RAD2DEG,
											   Selection->rotation.y * REFR_RAD2DEG,
											   Selection->rotation.z * REFR_RAD2DEG); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Scale:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%0.3f", Selection->scale); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Flags:");  ImGui::SameLine(); ImGui::TextDisabled("  (?)"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("P  - Persistent\nD  - Initially Disabled\nV  - Visible When Distant\nI  - Invisible (Editor only)\nCI - Children Invisible (Editor only)\nF  - Frozen (Editor only)");
							ImGui::TextColored(ImColor(R, G, B), "%s %s %s %s %s %s   ",
								((Selection->formFlags & TESForm::kFormFlags_QuestItem) ? ("P") : ("-")),
											   ((Selection->formFlags & TESForm::kFormFlags_Disabled) ? ("D") : ("-")),
											   ((Selection->formFlags & TESForm::kFormFlags_VisibleWhenDistant) ? ("V") : ("-")),
											   (Selection->GetInvisible() ? ("I") : ("-")),
											   (Selection->GetChildrenInvisible() ? ("CI") : ("-")),
											   (Selection->IsFrozen() ? ("F") : ("-"))); ImGui::NextColumn();

							BSExtraData* xData = Selection->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
							char xBuffer[0x50] = { 0 };
							if (xData)
							{
								ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
								ImGui::TextColored(ImColor(R, G, B), "Parent:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
								ImGui::TextColored(ImColor(R, G, B), "%s(%08X) %s   ",
									((xParent->parent->editorID.Size()) ? (xParent->parent->editorID.c_str()) : ("")),
												   xParent->parent->formID,
												   (xParent->oppositeState ? "[X]" : "")); ImGui::NextColumn();
								if (ImGui::IsItemClicked())
									_TES->LoadCellIntoViewPort(nullptr, xParent->parent);
							}

							char cBuffer[0x50] = { 0 };
							if (Selection->parentCell->IsInterior() == false)
							{
								ImGui::TextColored(ImColor(R, G, B), "Parent Cell:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
								ImGui::TextColored(ImColor(R, G, B), "%s(%08X) %d,%d   ", Selection->parentCell->GetEditorID(), Selection->parentCell->formID,
												   Selection->parentCell->cellData.coords->x, Selection->parentCell->cellData.coords->y); ImGui::NextColumn();
							}
						}

					}

					ImGui::Separator();
				}

				if (TESRenderWindow::SelectedPathGridPoints->Count() == 1)
				{
					TESPathGridPoint* Point = TESRenderWindow::SelectedPathGridPoints->Head()->Item();
					ImGui::TextColored(ImColor(R, G, B), "Position:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
					ImGui::TextColored(ImColor(R, G, B), "%.03f, %.03f, %.03f   ", Point->position.x, Point->position.y, Point->position.z); ImGui::NextColumn();
					if (Point->linkedRef)
					{
						ImGui::TextColored(ImColor(R, G, B), "Linked Reference:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
						ImGui::TextColored(ImColor(R, G, B), "%s(%08X)", ((Point->linkedRef->editorID.Size()) ? (Point->linkedRef->editorID.c_str()) : ("")), Point->linkedRef->formID); ImGui::NextColumn();
					}

					ImGui::Separator();
				}
			}
			ImGui::Columns(1);

			ImGui::Columns(2, "Misc Data", false);
			{
				PROCESS_MEMORY_COUNTERS_EX MemCounter = { 0 };
				SME::StringHelpers::GetRGB(settings::renderWindowOSD::kColorRAMUsage().s, R, G, B);
				if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter)))
				{
					UInt32 CurrentRAMCounter = MemCounter.WorkingSetSize / (1024 * 1024);		// in megabytes
					ImGui::TextColored(ImColor(R, G, B), "RAM Usage:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
					ImGui::TextColored(ImColor(R, G, B), "%d MB  ", CurrentRAMCounter); ImGui::NextColumn();
				}

				ImGui::Text("FPS:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
				ImGui::Text("%.1f  ", ImGui::GetIO().Framerate); ImGui::NextColumn();
			}
			ImGui::Columns(1);

			ImGui::End();
			ImGui::PopStyleVar();
		}

		bool DefaultOverlayOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

	}
}