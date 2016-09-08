#include "ActiveRefCollectionsOSDLayer.h"
#include "Render Window\RenderWindowManager.h"
#include "RenderWindowActions.h"

namespace cse
{
	namespace renderWindow
	{
		ActiveRefCollectionsOSDLayer	ActiveRefCollectionsOSDLayer::Instance;

		void ActiveRefCollectionsOSDLayer::RenderRefTableContents(int Tab)
		{
			const TESObjectREFRArrayT& ActiveRefs = _RENDERWIN_MGR.GetActiveRefs();
			std::vector<std::string> ActiveGroups;
			ActiveGroups.reserve(0x20);

			char FilterBuffer[0x200] = { 0 };
			char Label[0x100] = { 0 };

			for (auto Itr : ActiveRefs)
			{
				std::string EditorID(Helpers::GetRefEditorID(Itr));
				UInt32 FormID = Itr->formID;
				const char* Type = TESForm::GetFormTypeIDLongName(Itr->baseForm->formType);

				switch (Tab)
				{
				case kTab_Invisible:
				case kTab_Frozen:
					{
						FORMAT_STR(FilterBuffer, "%s %08X %s", EditorID.c_str(), FormID, Type);
						if (FilterHelper.PassFilter(FilterBuffer) == false)
							return;

						bool TruthCond = false;
						UInt32 InvisibleReasons = 0;
						if (Tab == kTab_Invisible)
							TruthCond = ReferenceVisibilityValidator::ShouldBeInvisible(Itr, InvisibleReasons);
						else
							TruthCond = Itr->IsFrozen();

						if (TruthCond)
						{
							FORMAT_STR(Label, "%08X-%d", FormID, Tab);
							ImGui::PushID(Label);
							{
								if (ImGui::Selectable(EditorID.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick) &&
									ImGui::IsMouseDoubleClicked(0))
								{
									// TODO select ref?
								}

								if (ImGui::BeginPopupContextItem(Tab == kTab_Invisible ? "Invisible_Popup" : "Frozen_Popup"))
								{
									if (Tab == kTab_Invisible)
									{
										if (ImGui::Selectable("Toggle Invisibility"))
											Itr->ToggleInvisiblity();
										if (ImGui::Selectable("Toggle Children Invisibility"))
											Itr->ToggleInvisiblity();
									}
									else
									{
										if (ImGui::Selectable("Thaw"))
											Itr->SetFrozen(false);
									}

									ImGui::EndPopup();
								}
							}
							ImGui::PopID();

							ImGui::NextColumn();
							char FormIDBuffer[0x10] = { 0 };
							FORMAT_STR(FormIDBuffer, "%08X", FormID);
							ImGui::Selectable(FormIDBuffer);
							ImGui::NextColumn();
							ImGui::Selectable(Type);
							ImGui::NextColumn();

							if (Tab == kTab_Invisible)
							{
								std::string FlagsBuffer;
								if ((InvisibleReasons & ReferenceVisibilityValidator::kReason_InvisibleSelf))
									FlagsBuffer.append("IS");

								if ((InvisibleReasons & ReferenceVisibilityValidator::kReason_InvisibleChild))
									FlagsBuffer.append(" IC");

								if ((InvisibleReasons & ReferenceVisibilityValidator::kReason_InitiallyDisabledSelf))
									FlagsBuffer.append(" DS");

								if ((InvisibleReasons & ReferenceVisibilityValidator::kReason_InitiallyDisabledChild))
									FlagsBuffer.append(" DC");

								boost::trim(FlagsBuffer);
								ImGui::Selectable(FlagsBuffer.c_str());
								ImGui::NextColumn();
							}
						}

					}

					break;
				case kTab_Groups:
					{
						const char* ParentGroup = _RENDERWIN_MGR.GetGroupManager()->GetParentGroupID(Itr);
						if (ParentGroup)
						{
							// cache unique hits
							std::string GroupID(ParentGroup);
							if (std::find(ActiveGroups.begin(), ActiveGroups.end(), GroupID) == ActiveGroups.end())
								ActiveGroups.push_back(GroupID);
						}
					}

					break;
				case kTab_Layers:
					{

					}

					break;
				}
			}

			switch (Tab)
			{
			case kTab_Groups:
				{
					if (ActiveGroups.size())
					{
						TESObjectREFRArrayT GroupMembers;
						GroupMembers.reserve(100);

						bool Dissolved = false;
						for (auto Itr : ActiveGroups)
						{
							if (_RENDERWIN_MGR.GetGroupManager()->GetGroupData(Itr.c_str(), GroupMembers))
							{
								SME_ASSERT(GroupMembers.size());

								// filter with members first
								int NonMatchCount = 0;
								for (auto Member : GroupMembers)
								{
									std::string EditorID(Helpers::GetRefEditorID(Member));
									UInt32 FormID = Member->formID;

									FORMAT_STR(FilterBuffer, "%s %08X", EditorID.c_str(), FormID);
									if (FilterHelper.PassFilter(FilterBuffer) == false)
										NonMatchCount++;
								}

								// if none of the members match, check the group name
								if (NonMatchCount == GroupMembers.size())
								{
									if (FilterHelper.PassFilter(Itr.c_str()) == false)
										continue;
								}

								FORMAT_STR(Label, "%s-group-%d", Itr.c_str(), Tab);
								ImGui::PushID(Label);
								{
									if (ImGui::Selectable(Itr.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick) &&
										ImGui::IsMouseDoubleClicked(0))
									{
										_RENDERWIN_MGR.GetGroupManager()->SelectAffiliatedGroup(GroupMembers[0], _RENDERSEL, true);
									}

									if (ImGui::BeginPopupContextItem("Group_Popup"))
									{
										if (ImGui::Selectable("Dissolve"))
										{
											_RENDERWIN_MGR.GetGroupManager()->RemoveParentGroup(GroupMembers[0]);
											Dissolved = true;
										}

										if (ImGui::Selectable("Edit Members"))
										{
											EditGroupMembersData* UserData = new EditGroupMembersData;
											UserData->GroupName = Itr;
											UserData->MemberList = GroupMembers;

											ModalWindowProviderOSDLayer::Instance.ShowModal("Edit Reference Group",
																							std::bind(&RenderModalEditGroupMembers, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
																							UserData,
																							ImGuiWindowFlags_AlwaysAutoResize);
										}

										ImGui::EndPopup();
									}
								}
								ImGui::PopID();

								char CountBuffer[0x100] = { 0 };
								std::string MemberList;
								int Count = 0;
								static constexpr int kMaxDiplayCount = 10;

								for (auto Member : GroupMembers)
								{
									std::string EditorID(Helpers::GetRefEditorID(Member));
									UInt32 FormID = Member->formID;

									FORMAT_STR(FilterBuffer, "%s (%08X)\n", EditorID.c_str(), FormID);
									MemberList.append(FilterBuffer);
									Count++;

									if (Count == kMaxDiplayCount)
										break;
								}

								if (Count == kMaxDiplayCount)
								{
									FORMAT_STR(CountBuffer, "\n+%d more references", GroupMembers.size() - Count);
									MemberList.append(CountBuffer);
								}

								FORMAT_STR(CountBuffer, "%d", GroupMembers.size());

								ImGui::NextColumn();
								ImGui::Selectable(CountBuffer);
								if (ImGui::IsItemHovered())
									ImGui::SetTooltip(MemberList.c_str());
								ImGui::NextColumn();

								if (Dissolved)
									break;
							}
						}
					}
				}

				break;
			case kTab_Layers:
				{

				}

				break;
			}
		}

		void ActiveRefCollectionsOSDLayer::RenderTabContents(int Tab)
		{
			SME_ASSERT(Tab >= kTab_Invisible && Tab < kTab__MAX);

			switch (Tab)
			{
			case kTab_Invisible:
			case kTab_Frozen:
				{
					ImGui::Columns(2, "ref_table_header", false);
					{
						if (Tab == kTab_Invisible)
						{
							ImGui::TextWrapped("Invisible References: (?)");
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("EditorIDs with an asterisk correspond to the reference's base form.\n\nRight click on an item to display the context menu.");
						}
						else
						{
							ImGui::TextWrapped("Frozen References: (?)");
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("Excluding references frozen with the \"Freeze Inactive Refs\" tool.EditorIDs with an asterisk correspond to the reference's base form.\n\nRight click on an item to display the context menu.");
						}

						ImGui::NextColumn();

						if (Tab == kTab_Invisible)
						{
							if (ImGui::Button("Reveal All", ImVec2(ImGui::GetColumnWidth() - 15, 20)))
								actions::RevealAll();
						}
						else
						{
							if (ImGui::Button("Thaw All", ImVec2(ImGui::GetColumnWidth() - 15, 20)))
								actions::ThawAll();
						}

						ImGui::NextColumn();
					}
					ImGui::Columns();

					ImGui::Columns(Tab == kTab_Invisible ? 4 : 3, "ref_table_invisible/frozen");
					{
						ImGui::Separator();
						ImGui::Text("EditorID"); ImGui::NextColumn();
						ImGui::Text("FormID"); ImGui::NextColumn();
						ImGui::Text("Type"); ImGui::NextColumn();

						if (Tab == kTab_Invisible)
						{
							ImGui::Text("Reason (?)");
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("DS - Initially Disabled\nDC - Child of Initially Disabled Parent\nIS - Invisible\nIC - Child of Parent with \"Invisible Children\" Flag");
							ImGui::NextColumn();
						}

						ImGui::Separator();
						RenderRefTableContents(Tab);
					}
					ImGui::Columns();
					ImGui::Separator();
				}

				break;
			case kTab_Groups:
				{
					ImGui::Columns(2, "ref_table_header", false);
					{
						ImGui::TextWrapped("Reference Groups: (?)");
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("EditorIDs with an asterisk correspond to the reference's base form.\n\nDouble click on an item to select the group.\nRight click on an item to display the context menu.\nHover the cursor over the \"Count\" column to view the first 10 members of the group.");
						ImGui::NextColumn();

						if (ImGui::Button("Group Current Selection", ImVec2(ImGui::GetColumnWidth() - 15, 20)))
							actions::GroupSelection();
						ImGui::NextColumn();
					}
					ImGui::Columns();

					ImGui::Columns(2, "ref_table_groups");
					{
						ImGui::Separator();
						ImGui::Text("Name"); ImGui::NextColumn();
						ImGui::Text("Count"); ImGui::NextColumn();

						ImGui::Separator();
						RenderRefTableContents(Tab);
					}
					ImGui::Columns();
					ImGui::Separator();
				}

				break;
			case kTab_Layers:
				{

				}

				break;
			}
		}

		bool ActiveRefCollectionsOSDLayer::RenderModalEditGroupMembers(RenderWindowOSD* OSD, ImGuiDX9* GUI, void* UserData)
		{
			static ImGuiTextFilter FilterHelper;

			EditGroupMembersData* Data = (EditGroupMembersData*)UserData;
			bool Close = false;
			char FilterBuffer[0x200] = { 0 };
			char Label[0x100] = { 0 };

			ImGui::Columns(3, "header", false);
			{
				int Count = Data->MemberList.size();
				if (Count <= 1)
					Count = 0;

				ImGui::Text("Group: %s", Data->GroupName.c_str()); ImGui::NextColumn();
				ImGui::Text("Count: %d", Count); ImGui::NextColumn();
				ImGui::TextDisabled("(?) ");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Right click on a reference to display the context menu.\nIf the group has only one member after removing another, it is automatically dissolved.");
				ImGui::NextColumn();
			}
			ImGui::Columns();

			FilterHelper.Draw();

			if (Data->MemberList.size() <= 1)
			{
				ImGui::TextDisabled("No Members (Dissolved).");
			}
			else
			{
				ImGui::BeginChild("subwindow", ImVec2(0, 300), false);
				ImGui::Columns(3, "ref_table");
				{
					ImGui::Separator();
					ImGui::Text("EditorID"); ImGui::NextColumn();
					ImGui::Text("FormID"); ImGui::NextColumn();
					ImGui::Text("Type"); ImGui::NextColumn();
					ImGui::Separator();

					int Count = 0;
					TESObjectREFRArrayT::iterator ToRemove = Data->MemberList.end();
					bool Removed = false;
					for (TESObjectREFRArrayT::iterator Member = Data->MemberList.begin(); Member != Data->MemberList.end(); ++Member)
					{
						std::string EditorID(Helpers::GetRefEditorID(*Member));
						UInt32 FormID = (*Member)->formID;
						const char* Type = TESForm::GetFormTypeIDLongName((*Member)->baseForm->formType);

						FORMAT_STR(FilterBuffer, "%s %08X %s", EditorID.c_str(), FormID, Type);
						if (FilterHelper.PassFilter(FilterBuffer) == false)
							continue;

						FORMAT_STR(Label, "%08X-%d", FormID, Count);
						ImGui::PushID(Label);
						{
							ImGui::Selectable(EditorID.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
							if (ImGui::BeginPopupContextItem("EditGroup_Popup"))
							{
								if (ImGui::Selectable("Remove"))
								{
									if (_RENDERWIN_MGR.GetGroupManager()->Orphanize(*Member))
									{
										ToRemove = Member;
										Removed = true;
									}
								}
								ImGui::EndPopup();
							}
						}
						ImGui::PopID();

						ImGui::NextColumn();
						char FormIDBuffer[0x10] = { 0 };
						FORMAT_STR(FormIDBuffer, "%08X", FormID);
						ImGui::Selectable(FormIDBuffer);
						ImGui::NextColumn();
						ImGui::Selectable(Type);
						ImGui::NextColumn();

						if (Removed)
							break;

						Count++;
					}

					// remove from the modal's cache
					if (Removed)
						Data->MemberList.erase(ToRemove);
				}
				ImGui::Columns();
				ImGui::EndChild();
				ImGui::Separator();
			}

			if (ImGui::Button("Close", ImVec2(0, 20)))
				Close = true;

			if (Close)
			{
				// release user data
				delete Data;
				FilterHelper.Clear();

				return true;
			}
			else
				return false;
		}

		ActiveRefCollectionsOSDLayer::ActiveRefCollectionsOSDLayer() :
			IRenderWindowOSDLayer(&settings::renderWindowOSD::kShowActiveRefCollections),
			CurrentTab(kTab_Invisible),
			FilterHelper()
		{
			;//
		}

		ActiveRefCollectionsOSDLayer::~ActiveRefCollectionsOSDLayer()
		{
			;//
		}

		void ActiveRefCollectionsOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			ImGui::SetNextWindowPos(ImVec2(10, 300), ImGuiSetCond_FirstUseEver);
			if (!ImGui::Begin("Active Reference Collections", nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				return;
			}

			ImGui::Columns(4, "Tab List", false);
			{
				static const ImColor kInactiveTabColor(ImColor::HSV(0, 0.2f, 0.2f));
				static const ImColor kActiveTabColor(ImColor::HSV(0, 0.6f, 0.6f));

				for (int i = kTab_Invisible; i < kTab__MAX; i++)
				{
					if (i != CurrentTab)
						ImGui::PushStyleColor(ImGuiCol_Button, kInactiveTabColor);
					else
						ImGui::PushStyleColor(ImGuiCol_Button, kActiveTabColor);
					{
						const char* TabTitle = nullptr;
						switch (i)
						{
						case kTab_Invisible:
							TabTitle = "Invisible##tab_invisible";
							break;
						case kTab_Frozen:
							TabTitle = "Frozen##tab_frozen";
							break;
						case kTab_Groups:
							TabTitle = "Groups##tab_groups";
							break;
						case kTab_Layers:
							TabTitle = "Layers##tab_layers";
							break;
						}

						if (ImGui::Button(TabTitle, ImVec2(ImGui::GetColumnWidth() - 15, 0)))
							CurrentTab = i;

						ImGui::NextColumn();
					}
					ImGui::PopStyleColor();
				}

			}
			ImGui::Columns(1);
			ImGui::Separator();
			FilterHelper.Draw();
			RenderTabContents(CurrentTab);

			ImGui::End();
		}

		bool ActiveRefCollectionsOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}
	}
}