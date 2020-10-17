#include "MouseOverTooltipOSDLayer.h"
#include "Render Window\RenderWindowManager.h"

namespace cse
{
	namespace renderWindow
	{
		MouseOverTooltipOSDLayer	MouseOverTooltipOSDLayer::Instance;

		MouseOverTooltipOSDLayer::MouseOverTooltipOSDLayer() :
			IRenderWindowOSDLayer()
		{

		}

		MouseOverTooltipOSDLayer::~MouseOverTooltipOSDLayer()
		{
			;//
		}

		void MouseOverTooltipOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			if (OSD->NeedsInput())
				return;
			else if (*TESRenderWindow::PathGridEditFlag == 0 && _RENDERWIN_XSTATE.CurrentMouseRef == nullptr)
				return;
			else if (*TESRenderWindow::PathGridEditFlag && _RENDERWIN_XSTATE.CurrentMousePathGridPoint == nullptr)
				return;
			else if (GetCapture() == *TESRenderWindow::WindowHandle && _RENDERWIN_MGR.GetMouseInputManager()->IsPaintingSelection() == false)
				return;

			SME_ASSERT((void*)_RENDERWIN_XSTATE.CurrentMouseRef != (void*)_RENDERWIN_XSTATE.CurrentMousePathGridPoint);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 2));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
			ImGui::BeginTooltip();
			{
				if (ImGui::BeginTable("##Tooltip Contents", 2))
				{
					ImGui::TableSetupColumn("First", ImGuiTableColumnFlags_WidthStretch, 100);
					ImGui::TableSetupColumn("Second", ImGuiTableColumnFlags_WidthAlwaysAutoResize);

					if (_RENDERWIN_XSTATE.CurrentMouseRef)
					{
						TESObjectREFR* Ref = _RENDERWIN_XSTATE.CurrentMouseRef;
						TESForm* Base = Ref->baseForm;

						ImGui::TableNextRow();
						{
							ImGui::TableNextColumn();
							{
								ImGui::Text("Object Reference:");
							}
							ImGui::TableNextColumn();
							{
								ImGui::Text("%s(%08X) %s", ((Ref->editorID.Size()) ? (Ref->editorID.c_str()) : ("")), Ref->formID, Ref->IsActive() ? "*" : "");
							}
						}
						ImGui::TableNextRow();
						{
							ImGui::TableNextColumn();
							{
								ImGui::Text("Base Form:");
							}
							ImGui::TableNextColumn();
							{
								ImGui::Text("%s (%08X)\n%s", ((Base->editorID.Size()) ? (Base->editorID.c_str()) : ("")), Base->formID, TESForm::GetFormTypeIDLongName(Ref->baseForm->formType));
							}
						}

						ImGui::TableNextRow();
						{
							ImGui::TableNextColumn();
							{
								ImGui::Text("Position:");
							}
							ImGui::TableNextColumn();
							{
								ImGui::Text("%.03f, %.03f, %.03f", Ref->position.x, Ref->position.y, Ref->position.z);
							}
						}

						ImGui::TableNextRow();
						{
							ImGui::TableNextColumn();
							{
								ImGui::Text("Rotation:");
							}
							ImGui::TableNextColumn();
							{
								ImGui::Text("%.03f, %.03f, %.03f", Ref->rotation.x * REFR_RAD2DEG, Ref->rotation.y * REFR_RAD2DEG, Ref->rotation.z * REFR_RAD2DEG);
							}
						}

						ImGui::TableNextRow();
						{
							ImGui::TableNextColumn();
							{
								ImGui::Text("Scale:");
							}
							ImGui::TableNextColumn();
							{
								ImGui::Text("%.03f", Ref->scale);
							}
						}

						std::string FlagStr;
						if (Ref->IsQuestItem())
							FlagStr += "Persistent\n";
						if (Ref->IsInitiallyDisabled())
							FlagStr += "Initially Disabled\n";
						if (Ref->IsVWD())
							FlagStr += "Visible When Distant\n";
						if (Ref->IsInvisible())
							FlagStr += "Invisible (Editor only)\n";
						if (Ref->IsChildrenInvisible())
							FlagStr += "Children Invisible (Editor only)\n";
						if (Ref->IsFrozen())
							FlagStr += "Frozen (Editor only)\n";

						if (!FlagStr.empty())
						{
							FlagStr.erase(FlagStr.begin() + FlagStr.size() - 1);
							ImGui::TableNextRow();
							{
								ImGui::TableNextColumn();
								{
									ImGui::Text("Flags:");
								}
								ImGui::TableNextColumn();
								{
									ImGui::Text("%s", FlagStr.c_str());
								}
							}
						}

						auto GroupID = _RENDERWIN_MGR.GetGroupManager()->GetParentGroupID(Ref);
						if (GroupID)
						{
							ImGui::TableNextRow();
							{
								ImGui::TableNextColumn();
								{
									ImGui::Text("Parent Group:");
								}
								ImGui::TableNextColumn();
								{
									ImGui::Text("%s", GroupID);
								}
							}
						}

						BSExtraData* xData = Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
						if (xData)
						{
							ImGui::TableNextRow();
							{
								ImGui::TableNextColumn();
								{
									ImGui::Text("Parent Reference:");
								}
								ImGui::TableNextColumn();
								{
									ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
									ImGui::Text("%s(%08X) %s",
										((xParent->parent->editorID.Size()) ? (xParent->parent->editorID.c_str()) : ("")),
										xParent->parent->formID,
										(xParent->oppositeState ? "[X]" : ""));
								}
							}
						}

						if (*TESRenderWindow::ActiveCell != Ref->parentCell && !Ref->parentCell->IsInterior())
						{
							ImGui::TableNextRow();
							{
								ImGui::TableNextColumn();
								{
									ImGui::Text("Parent Cell:");
								}
								ImGui::TableNextColumn();
								{
									ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
									ImGui::Text("%s(%08X) %d,%d",
										Ref->parentCell->GetEditorID(),
										Ref->parentCell->formID,
										Ref->parentCell->cellData.coords->x,
										Ref->parentCell->cellData.coords->y);
								}
							}
						}
					}
					else if (_RENDERWIN_XSTATE.CurrentMousePathGridPoint)
					{
						auto GridPoint = _RENDERWIN_XSTATE.CurrentMousePathGridPoint;
						TESObjectREFR* Ref = _RENDERWIN_XSTATE.CurrentMousePathGridPoint->linkedRef;
						TESForm* Base = Ref ? Ref->baseForm : nullptr;

						ImGui::TableNextRow();
						{
							ImGui::TableNextColumn();
							{
								ImGui::Text("Position:");
							}
							ImGui::TableNextColumn();
							{
								ImGui::Text("%.03f, %.03f, %.03f", GridPoint->position.x, GridPoint->position.y, GridPoint->position.z);
							}
						}

						ImGui::TableNextRow();
						{
							ImGui::TableNextColumn();
							{
								ImGui::Text("Linked Points:");
							}
							ImGui::TableNextColumn();
							{
								ImGui::Text("%d", GridPoint->linkedPoints.Count());
							}
						}

						if (Ref)
						{
							ImGui::TableNextRow();
							{
								ImGui::TableNextColumn();
								{
									ImGui::Text("Linked Reference:");
								}
								ImGui::TableNextColumn();
								{
									ImGui::Text("%s (%08X) %s", ((Ref->editorID.Size()) ? (Ref->editorID.c_str()) : ("")), Ref->formID, Ref->IsActive() ? "*" : "");
								}
							}

							ImGui::TableNextRow();
							{
								ImGui::TableNextColumn();
								{
									ImGui::Text("Base Form:");
								}
								ImGui::TableNextColumn();
								{
									ImGui::Text("%s (%08X)", ((Base->editorID.Size()) ? (Base->editorID.c_str()) : ("")), Base->formID);
								}
							}
						}

						std::string FlagStr;
						if (GridPoint->autoGenerated)
							FlagStr += "Auto-generated\n";
						if (GridPoint->IsPointPreferred())
							FlagStr += "Preferred\n";

						if (!FlagStr.empty())
						{
							FlagStr.erase(FlagStr.begin() + FlagStr.size() - 1);
							ImGui::TableNextRow();
							{
								ImGui::TableNextColumn();
								{
									ImGui::Text("Flags:");
								}
								ImGui::TableNextColumn();
								{
									ImGui::Text("%s", FlagStr.c_str());
								}
							}
						}
					}

					ImGui::EndTable();
				}
			}
			ImGui::EndTooltip();
			ImGui::PopStyleVar(2);
		}

		bool MouseOverTooltipOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		bool MouseOverTooltipOSDLayer::IsEnabled() const
		{
			int Enabled = settings::renderWindowOSD::kShowMouseRef.GetData().i;
			int ControlModified = settings::renderWindowOSD::kMouseRefCtrlModified.GetData().i;

			return Enabled && (ControlModified == false || GetAsyncKeyState(VK_CONTROL));
		}
	}
}