#include "InfoOverlayOSDLayer.h"
#include "Render Window\RenderWindowManager.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"

namespace cse
{
	namespace renderWindow
	{
		InfoOverlayOSDLayer		InfoOverlayOSDLayer::Instance;

		InfoOverlayOSDLayer::InfoOverlayOSDLayer() :
			IRenderWindowOSDLayer(&settings::renderWindowOSD::kShowInfoOverlay)
		{
			;//
		}

		InfoOverlayOSDLayer::~InfoOverlayOSDLayer()
		{
			;//
		}

		void InfoOverlayOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			static const float FirstColumnWidth = 130;

			ImGui::SetNextWindowPos(ImVec2(10, 10));
			ImGui::SetNextWindowSizeConstraints(ImVec2(-1, -1), ImVec2(-1, 300));

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3, 6));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 10));
			if (!ImGui::Begin("Default Info Overlay", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				ImGui::PopStyleVar(2);
				return;
			}

			TESObjectCELL* CurrentCell = _TES->currentInteriorCell;
			if (CurrentCell == nullptr)
				CurrentCell = *TESRenderWindow::ActiveCell;

			if (ImGui::BeginTable("##Info Overlay Data", 2))
			{
				ImGui::TableSetupColumn("First", ImGuiTableColumnFlags_WidthStretch, FirstColumnWidth);
				ImGui::TableSetupColumn("Second", ImGuiTableColumnFlags_WidthAlwaysAutoResize);

				ImGui::TableNextRow();
				{
					ImGui::TableNextColumn();
					{
						PROCESS_MEMORY_COUNTERS_EX MemCounter = { 0 };
						GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter));
						UInt32 CurrentRAMCounter = MemCounter.WorkingSetSize / (1024 * 1024);		// in megabytes
						ImGui::Text("RAM: %d MB", CurrentRAMCounter);
					}
					ImGui::TableNextColumn();
					{
						ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
					}
				}

				ImGui::Dummy(ImVec2(1, 7));

				if (CurrentCell)
				{
					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							ImGui::Text("Current Cell: ");
							ImGui::SameLine();
							ImGui::TextDisabled(" (?)");
							{
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
							}
						}
						ImGui::TableNextColumn();
						{
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
							ImGui::Text("%s", EditModeIcon);
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("%s Edit Mode", EditModeLabel);

							ImGui::SameLine(0, 5);

							char Buffer[0x50] = { 0 };
							if (CurrentCell->IsInterior())
								FORMAT_STR(Buffer, "%s (%08X)", CurrentCell->GetEditorID(), CurrentCell->formID);
							else
							{
								FORMAT_STR(Buffer, "%s %d,%d (%08X)", CurrentCell->GetEditorID(), CurrentCell->cellData.coords->x,
									CurrentCell->cellData.coords->y, CurrentCell->formID);
							}

							TEXT_WITH_TOOLTIP("%s", Buffer);
						}
					}

					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							ImGui::Text("Camera:");
						}
						ImGui::TableNextColumn();
						{
							const NiVector3* CameraCoords = _PRIMARYRENDERER->GetCameraWorldTranslate();
							TEXT_WITH_TOOLTIP("%.0f, %0.f, %0.f", CameraCoords->x, CameraCoords->y, CameraCoords->z);
						}
					}
				}

				if (_RENDERSEL->selectionCount)
				{
					ImGui::Dummy(ImVec2(1, 5));

					std::vector<std::string> Groups;
					int GroupCount = _RENDERWIN_MGR.GetGroupManager()->GetSelectionGroups(_RENDERSEL, Groups);

					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							ImGui::Text("%d Object%s Selected", _RENDERSEL->selectionCount, (_RENDERSEL->selectionCount > 1 ? "s" : ""));
						}
						ImGui::TableNextColumn();
						{
							if (GroupCount)
							{
								ImGui::Text("%d Group%s ", GroupCount, GroupCount == 1 ? "" : "s");
								ImGui::SameLine(0, 5);
								ImGui::TextDisabled(" (?) ");
								if (ImGui::IsItemHovered())
								{
									char CountBuffer[0x100] = { 0 };
									char Buffer[0x100] = { 0 };
									std::string MouseOver;
									int Count = 0;
									constexpr int kMaxDiplayCount = 10;

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

									ImGui::SetTooltip(MouseOver.c_str());
								}
							}
						}
					}

					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							ImGui::Text("Nominal Center:");
						}
						ImGui::TableNextColumn();
						{
							TEXT_WITH_TOOLTIP("%.3f, %.3f, %.3f",
								_RENDERSEL->selectionPositionVectorSum.x,
								_RENDERSEL->selectionPositionVectorSum.y,
								_RENDERSEL->selectionPositionVectorSum.z);
						}
					}
				}

				ImGui::EndTable();
			}

			ImGui::End();
			ImGui::PopStyleVar(2);
		}

		bool InfoOverlayOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

	}
}