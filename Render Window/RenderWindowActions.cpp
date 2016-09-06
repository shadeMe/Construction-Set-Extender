#include "RenderWindowActions.h"
#include "RenderWindowInput.h"
#include "RenderWindowManager.h"
#include "Achievements.h"
#include "[Common]\CLIWrapper.h"
#include "AuxiliaryViewport.h"
#include "ToolbarOSDLayer.h"

namespace cse
{
	namespace renderWindow
	{
		namespace actions
		{
			IRenderWindowAction::IRenderWindowAction(std::string Name, std::string Desc, UInt8 Context) :
				Name(Name),
				Description(Desc),
				ExecutionContext(Context)
			{
				SME_ASSERT(ExecutionContext != NULL);
			}

			const char* IRenderWindowAction::GetName() const
			{
				return Name.c_str();
			}

			const char* IRenderWindowAction::GetDescription() const
			{
				return Description.c_str();
			}

			bool IRenderWindowAction::IsExecutableInLandscapeEdit() const
			{
				return ExecutionContext & kMode_LandscapeEdit;
			}

			bool IRenderWindowAction::IsExecutableInCurrentContext() const
			{
				bool Ref = *TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0;
				bool PathGrid = *TESRenderWindow::PathGridEditFlag;
				bool Landscape = *TESRenderWindow::LandscapeEditFlag;

				if (Ref && IsExecutableInReferenceEdit())
					return true;
				else if (PathGrid && IsExecutableInPathGridEdit())
					return true;
				else if (Landscape && IsExecutableInLandscapeEdit())
					return true;
				else
					return false;
			}

			bool IRenderWindowAction::HasSameExecutionContext(const IRenderWindowAction& RHS)
			{
				if (IsExecutableInReferenceEdit() && RHS.IsExecutableInReferenceEdit())
					return true;
				else if (IsExecutableInPathGridEdit() && RHS.IsExecutableInPathGridEdit())
					return true;
				else if (IsExecutableInLandscapeEdit() && RHS.IsExecutableInLandscapeEdit())
					return true;
				else
					return false;
			}

			bool IRenderWindowAction::IsExecutableInPathGridEdit() const
			{
				return ExecutionContext & kMode_PathGridEdit;
			}

			bool IRenderWindowAction::IsExecutableInReferenceEdit() const
			{
				return ExecutionContext & kMode_ReferenceEdit;
			}

			namespace impl
			{
				BasicRWA::BasicRWA(std::string Name, std::string Desc, ActionDelegateT Delegate) :
					IRenderWindowAction(Name, Desc, kMode_ReferenceEdit),
					Delegate(Delegate)
				{
					SME_ASSERT(Delegate);
				}

				BasicRWA::BasicRWA(std::string Name, std::string Desc, UInt8 Context, ActionDelegateT Delegate) :
					IRenderWindowAction(Name, Desc, Context),
					Delegate(Delegate)
				{
					SME_ASSERT(Delegate);
				}

				BasicRWA::~BasicRWA()
				{
					;//
				}

				bool BasicRWA::operator()()
				{
					if (IsExecutableInCurrentContext() == false)
						return false;

					Delegate();
					TESRenderWindow::Redraw();
					return true;
				}

				ToggleINISettingRWA::ToggleINISettingRWA(std::string Name, std::string Desc, UInt8 Context, SME::INI::INISetting& Setting) :
					IRenderWindowAction(Name, Desc, Context),
					Setting(Setting)
				{
					SME_ASSERT(Setting.GetType() == SME::INI::INISetting::kType_Integer ||
							   Setting.GetType() == SME::INI::INISetting::kType_Unsigned);
				}

				ToggleINISettingRWA::~ToggleINISettingRWA()
				{
					;//
				}

				bool ToggleINISettingRWA::operator()()
				{
					if (IsExecutableInCurrentContext() == false)
						return false;

					Setting.ToggleData();
					TESRenderWindow::Redraw();
					return true;
				}

				ToggleVisibilityRWA::ToggleVisibilityRWA(int Type) :
					IRenderWindowAction("", "", kMode_All),
					Type(Type)
				{
					SME_ASSERT(Type > kType__NONE && Type < kType__MAX);

					static const char* kNames[actions::impl::ToggleVisibilityRWA::kType__MAX] =
					{
						"Toggle Objects",
						"Toggle Markers",
						"Toggle Wireframe",
						"Toggle Bright Light",
						"Toggle Sky",
						"Toggle Solid Subspaces",
						"Toggle Collision Geometry",
						"Toggle Leaves",
						"Toggle Trees",
						"Toggle Water",
						"Toggle Cell Borders",
						"Toggle Landscape",
						"Toggle Light Radius",
						"Toggle Parent Child Indicator",
						"Toggle Path Grid Linked Reference Indicator",
						"Toggle Initially Disabled References",
						"Toggle Initially Disabled References' Children",
						"Toggle Grass Overlay"
					};

					static const char* kDescriptions[actions::impl::ToggleVisibilityRWA::kType__MAX] =
					{
						"Toggle visibility of non-land objects.",
						"Toggle visibility of markers.",
						"Toggle wireframe rendering.",
						"Toggle bright light rendering.",
						"Toggle visibility of the sky.",
						"Toggle visibility of solid subspaces.",
						"Toggle visibility of collision geometry.",
						"Toggle visibility of leaves.",
						"Toggle visibility of trees.",
						"Toggle visibility of water.",
						"Toggle visibility of cell borders.",
						"Toggle visibility of landscape.",
						"Toggle visibility of light radius.",
						"Toggle visibility of parent-child indicators.",
						"Toggle visibility of path grid point linked reference indicators.",
						"Toggle visibility of initially disabled references.",
						"Toggle visibility of initially disabled references' children.",
						"Toggle visibility of the grass overlay texture."
					};

					Name = kNames[Type];
					Description = kDescriptions[Type];
				}

				ToggleVisibilityRWA::~ToggleVisibilityRWA()
				{
					;//
				}

				bool ToggleVisibilityRWA::operator()()
				{
					if (IsExecutableInCurrentContext() == false)
						return false;

					switch (Type)
					{
					case kType_Objects:
						_RENDERWIN_MGR.GetHotKeyManager()->SendBuiltinHotKey('C', true, false);
						break;
					case kType_Markers:
						_RENDERWIN_MGR.GetHotKeyManager()->SendBuiltinHotKey('M', false, false);
						break;
					case kType_Wireframe:
						_RENDERWIN_MGR.GetHotKeyManager()->SendBuiltinHotKey('W', false, false);
						break;
					case kType_BrightLight:
						_RENDERWIN_MGR.GetHotKeyManager()->SendBuiltinHotKey('A', false, false);
						break;
					case kType_Sky:
						// using the key message is a bit of bother as it alters the stored Y key state
						// we could override it in the hotkey manager, but we just route the message to the main window (which is what the shortcut does anyway)
						TESCSMain::InvokeMainMenuTool(TESCSMain::kMainMenu_View_Sky);
						break;
					case kType_SolidSubspaces:
						{
							Setting* Setting = INISettingCollection::Instance->LookupByName("bDrawFullSubSpaceBox:General");
							Setting->value.b = Setting->value.b == false;
						}
						break;
					case kType_CollisionGeom:
						_RENDERWIN_MGR.GetHotKeyManager()->SendBuiltinHotKey(VK_F4, false, false);
						break;
					case kType_Leaves:
						(*BSTreeManager::Singleton)->drawLeaves = (*BSTreeManager::Singleton)->drawLeaves == false;
						break;
					case kType_Trees:
						(*BSTreeManager::Singleton)->drawTrees = (*BSTreeManager::Singleton)->drawTrees == false;
						break;
					case kType_Water:
						_RENDERWIN_MGR.GetHotKeyManager()->SendBuiltinHotKey('W', true, false);
						break;
					case kType_CellBorders:
						_RENDERWIN_MGR.GetHotKeyManager()->SendBuiltinHotKey('B', false, false);
						break;
					case kType_Land:
						_RENDERWIN_MGR.GetHotKeyManager()->SendBuiltinHotKey('L', true, false);
						break;
					case kType_LightRadius:
						_RENDERWIN_MGR.GetHotKeyManager()->SendBuiltinHotKey('L', false, false);
						break;
					case kType_ParentChildIndicator:
						settings::renderer::kParentChildVisualIndicator.ToggleData();
						break;
					case kType_PathGridLinkedRefIndicator:
						settings::renderer::kPathGridLinkedRefIndicator.ToggleData();
						break;
					case kType_InitiallyDisabledRefs:
						_RENDERWIN_XSTATE.ShowInitiallyDisabledRefs = _RENDERWIN_XSTATE.ShowInitiallyDisabledRefs == false;
						break;
					case kType_InitiallyDisabledRefsChildren:
						_RENDERWIN_XSTATE.ShowInitiallyDisabledRefChildren = _RENDERWIN_XSTATE.ShowInitiallyDisabledRefChildren == false;
						break;
					case kType_GrassOverlay:
						_RENDERWIN_XSTATE.UseGrassTextureOverlay = _RENDERWIN_XSTATE.UseGrassTextureOverlay == false;
						_TES->ReloadLandscapeTextures();
						break;
					}

					TESRenderWindow::Redraw(Type == kType_PathGridLinkedRefIndicator);
					return true;
				}

				bool ToggleVisibilityRWA::IsVisible(int Type)
				{
					SME_ASSERT(Type > kType__NONE && Type < kType__MAX);

					switch (Type)
					{
					case kType_Objects:
						return *TESRenderWindow::NonLandObjectsHiddenFlag == false;
					case kType_Markers:
						return *TESRenderWindow::StateFlags & TESRenderWindow::kRenderWindowState_ShowMarkers;
					case kType_Wireframe:
						return (*TESRender::Scenegraph::Singleton)->wireFrame->m_bWireframe;
					case kType_BrightLight:
						return *TESRenderWindow::FullBrightLightingFlag;
					case kType_Sky:
						return *TESRenderWindow::SkyFlag;
					case kType_SolidSubspaces:
						return INISettingCollection::Instance->LookupByName("bDrawFullSubSpaceBox:General")->value.b;
					case kType_CollisionGeom:
						{
							bhkWorldM* Havok = _TES->GetHavokWorld();
							if (Havok)
								return Havok->showCollisionGeom;
							else
								return false;
						}
					case kType_Leaves:
						return (*BSTreeManager::Singleton)->drawLeaves;
					case kType_Trees:
						return (*BSTreeManager::Singleton)->drawTrees;
					case kType_Water:
						return *TESRenderWindow::WaterHiddenFlag;
					case kType_CellBorders:
						return _TES->drawCellBorders;
					case kType_Land:
						return *TESRenderWindow::LandscapeHiddenFlag;
					case kType_LightRadius:
						return *TESRenderWindow::LightRadiusFlag;
					case kType_ParentChildIndicator:
						return settings::renderer::kParentChildVisualIndicator().i;
					case kType_PathGridLinkedRefIndicator:
						return settings::renderer::kPathGridLinkedRefIndicator().i;
					case kType_InitiallyDisabledRefs:
						return _RENDERWIN_XSTATE.ShowInitiallyDisabledRefs;
					case kType_InitiallyDisabledRefsChildren:
						return _RENDERWIN_XSTATE.ShowInitiallyDisabledRefChildren;
					case kType_GrassOverlay:
						return _RENDERWIN_XSTATE.UseGrassTextureOverlay;
					}

					return false;
				}

				AlignReferencesRWA::AlignReferencesRWA(UInt8 Alignment, UInt8 Axis) :
					IRenderWindowAction("", "", kMode_ReferenceEdit),
					Alignment(Alignment),
					Axis(Axis)
				{
					SME_ASSERT(Alignment == kAlign_Position || Alignment == kAlign_Rotation);

					if (Alignment == kAlign_Position)
					{
						Name = "Align Position";
						Description = "Align the reference's position to the target reference.";
					}
					else
					{
						Name = "Align Rotation";
						Description = "Align the reference's rotation to the target reference.";
					}
				}

				AlignReferencesRWA::~AlignReferencesRWA()
				{
					;//
				}

				bool AlignReferencesRWA::operator()()
				{
					if (IsExecutableInCurrentContext() == false)
						return false;

					bool AlignmentAxisX = Axis & kAxis_X;
					bool AlignmentAxisY = Axis & kAxis_Y;
					bool AlignmentAxisZ = Axis & kAxis_Z;

					if (_RENDERSEL->selectionCount)
					{
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

								if (Alignment == kAlign_Position)
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
								else
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

							NotificationOSDLayer::Instance.ShowNotification("Selection %s aligned to %08X", Alignment == kAlign_Position ? "position" : "rotation", AlignRef->formID);
							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefAlignment);
						}
					}

					return true;
				}
			}

			impl::BasicRWA InvertSelection("Invert Selection", "Invert the current reference selection.", []() {
				const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
				if (Refs.size())
				{
					TESRenderSelection* Buffer = TESRenderSelection::CreateInstance(_RENDERSEL);
					_RENDERSEL->ClearSelection(true);

					for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
					{
						TESObjectREFR* Ref = *Itr;

						if (Buffer->HasObject(Ref) == false)
							_RENDERSEL->AddToSelection(Ref, true);
					}

					Buffer->DeleteInstance();
				}
			});

			impl::BasicRWA SelectAll("Select All", "Select all references in the active cell/grid.", []() {
				const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
				_RENDERSEL->ClearSelection(true);

				for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
				{
					TESObjectREFR* Ref = *Itr;

					if (_RENDERSEL->HasObject(Ref) == false &&
						(Ref->formFlags & TESObjectREFR::kSpecialFlags_3DInvisible) == false &&
						(Ref->formFlags & TESObjectREFR::kSpecialFlags_Frozen) == false)
					{
						_RENDERSEL->AddToSelection(Ref, true);
					}
				}
			});

			impl::AlignReferencesRWA AlignPosition(impl::AlignReferencesRWA::kAlign_Position, impl::AlignReferencesRWA::kAxis_ALL);
			impl::AlignReferencesRWA AlignRotation(impl::AlignReferencesRWA::kAlign_Rotation, impl::AlignReferencesRWA::kAxis_ALL);

			impl::BasicRWA GroupSelection("Group", "Group references into a single selectable entity.", []() {
				ModalWindowProviderOSDLayer::ModalRenderDelegateT RenderModalNewGroup([](RenderWindowOSD*, ImGuiDX9*, void*)->bool {
					static char NewGroupNameBuffer[0x100] = { 0 };

					bool EnterKey = false;
					if (ImGui::InputText("Name", NewGroupNameBuffer, sizeof(NewGroupNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
						EnterKey = true;

					bool InvalidName = _RENDERWIN_MGR.GetGroupManager()->GetGroupExists(NewGroupNameBuffer);
					if (InvalidName)
						ImGui::Text("Group already exists!");
					else if (strlen(NewGroupNameBuffer) == 0)
						InvalidName = true;

					ImGui::Separator();
					bool Close = false;
					if (ImGui::Button("OK", ImVec2(120, 0)) || EnterKey)
					{
						if (InvalidName == false)
						{
							if (_RENDERWIN_MGR.GetGroupManager()->AddGroup(NewGroupNameBuffer, _RENDERSEL) == false)
								NotificationOSDLayer::Instance.ShowNotification("Couldn't add current selection to a new group.");
							else
								NotificationOSDLayer::Instance.ShowNotification("Created new selection group");

							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefGrouping);
							Close = true;
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0)))
						Close = true;

					if (Close)
					{
						// clear the buffer for the next use
						ZeroMemory(NewGroupNameBuffer, sizeof(NewGroupNameBuffer));
						return true;
					}
					else
						return false;
				});

				if (_RENDERWIN_MGR.GetGroupManager()->IsSelectionGroupable(_RENDERSEL))
				{
					ModalWindowProviderOSDLayer::Instance.ShowModal("New Reference Group",
																	RenderModalNewGroup,
																	nullptr,
																	ImGuiWindowFlags_AlwaysAutoResize);
				}
			});

			impl::BasicRWA UngroupSelection("Ungroup", "Dissolve a group into its individual references.", []() {
				if (_RENDERWIN_MGR.GetGroupManager()->RemoveGroup(_RENDERSEL) == false)
					NotificationOSDLayer::Instance.ShowNotification("Couldn't dissolve the current selection's group.");
				else
					NotificationOSDLayer::Instance.ShowNotification("Removed selection group");
			});

			impl::BasicRWA OrphanizeSelection("Orphanize", "Remove the reference from its parent group, if any.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					_RENDERWIN_MGR.GetGroupManager()->Orphanize(Ref);
				}
			});


			impl::BasicRWA FreezeSelection("Freeze", "Prevent references from being selected.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetFrozen(true);
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefFreezing);
			});

			impl::BasicRWA ThawSelection("Thaw", "Allow previously frozen references to be selected.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetFrozen(false);
				}
			});

			impl::BasicRWA ThawAll("Thaw All", "Defrost all frozen references.", []() {
				const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
				for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
				{
					TESObjectREFR* Ref = *Itr;
					Ref->SetFrozen(false);
				}

				NotificationOSDLayer::Instance.ShowNotification("Thawed all of the active cell/grid's references");
			});

			impl::BasicRWA ToggleFreezeInactive("Toggle Freeze Inactive", "Prevent references that don't belong to the active plugin from being selected.", []() {
				_RENDERWIN_XSTATE.FreezeInactiveRefs = _RENDERWIN_XSTATE.FreezeInactiveRefs == false;

				if (_RENDERWIN_XSTATE.FreezeInactiveRefs)
					NotificationOSDLayer::Instance.ShowNotification("Inactive references frozen");
				else
					NotificationOSDLayer::Instance.ShowNotification("Inactive references thawed");
			});


			impl::BasicRWA ToggleSelectionVisibility("Hide", "Hide the reference.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->ToggleInvisiblity();
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefVisibility);
			});

			impl::BasicRWA ToggleSelectionChildrenVisibility("Hide Children", "Hide the reference's enable-state children.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->ToggleChildrenInvisibility();
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefVisibility);
			});

			impl::BasicRWA RevealAll("Reveal All", "Show all hidden references.", []() {
				const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
				for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
				{
					TESObjectREFR* Ref = *Itr;

					if (Ref->GetInvisible())
						Ref->ToggleInvisiblity();

					if (Ref->GetChildrenInvisible())
						Ref->ToggleChildrenInvisibility();
				}

				NotificationOSDLayer::Instance.ShowNotification("Reset visibility flags on the active cell/grid's references");
			});


			impl::BasicRWA DimSelectionOpacity("Dim Opacity", "Make the reference partially transparent.", []() {
				float Alpha = settings::renderer::kRefToggleOpacityAlpha().f;
				if (Alpha < 0.1)
					Alpha = 0.1;

				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetAlpha(Alpha);
				}
			});

			impl::BasicRWA ResetSelectionOpacity("Reset Opacity", "Make the reference fully opaque.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetAlpha();
				}
			});


			impl::BasicRWA LinkPathGridSelection("Link Path Grid Reference", "Link the path grid point to a reference.",
												 IRenderWindowAction::kMode_PathGridEdit, []() {
				if (TESRenderWindow::SelectedPathGridPoints->IsEmpty())
					return;

				std::vector<TESPathGrid*> ParentGrids;
				TESObjectREFR* LinkRef = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, nullptr, true);

				if (LinkRef)
				{
					for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin();
						 !Itr.End() && Itr.Get();
						 ++Itr)
					{
						TESPathGridPoint* Point = Itr.Get();
						if (Point->linkedRef)
							Point->UnlinkFromReference();

						ParentGrids.push_back(Point->parentGrid);
						Point->LinkToReference(LinkRef);
					}

					for (auto Itr : ParentGrids)
						Itr->GenerateNiNode();
				}
			});

			impl::BasicRWA UnlinkPathGridSelection("Unlink Path Grid Reference", "Remove the path grid point's linked reference, if any",
												   IRenderWindowAction::kMode_PathGridEdit, []() {
				_RENDERWIN_MGR.GetPathGridUndoManager()->ResetRedoStack();

				if (TESRenderWindow::SelectedPathGridPoints->Count())
					_RENDERWIN_MGR.GetPathGridUndoManager()->RecordOperation(PathGridUndoManager::kOperation_DataChange, TESRenderWindow::SelectedPathGridPoints);

				for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
				{
					TESPathGridPoint* Point = Itr.Get();
					Point->UnlinkFromReference();
					Point->HideSelectionRing();
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_PathGridAdditions);
			});

			impl::BasicRWA ShowBatchEditor("Batch Editor", "Display the batch reference editor.", []() {
				const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
				UInt32 RefCount = Refs.size();

				if (RefCount > 1)
				{
					int i = 0;
					componentDLLInterface::CellObjectData* RefData = new componentDLLInterface::CellObjectData[RefCount];
					componentDLLInterface::BatchRefData* BatchData = new componentDLLInterface::BatchRefData();

					for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr, ++i)
					{
						TESObjectREFR* ThisRef = *Itr;
						componentDLLInterface::CellObjectData* ThisRefData = &RefData[i];

						ThisRefData->EditorID = (!ThisRef->editorID.c_str()) ? ThisRef->baseForm->editorID.c_str() : ThisRef->editorID.c_str();
						ThisRefData->FormID = ThisRef->formID;
						ThisRefData->TypeID = ThisRef->baseForm->formType;
						ThisRefData->Flags = ThisRef->formFlags;
						ThisRefData->Selected = false;

						for (TESRenderSelection::SelectedObjectsEntry* j = _RENDERSEL->selectionList; j != 0; j = j->Next)
						{
							if (j->Data && j->Data == ThisRef)
							{
								ThisRefData->Selected = true;
								break;
							}
						}

						ThisRefData->ParentForm = ThisRef;
					}

					BatchData->CellObjectListHead = RefData;
					BatchData->ObjectCount = RefCount;

					if (cliWrapper::interfaces::BE->ShowBatchRefEditorDialog(BatchData))
					{
						for (UInt32 k = 0; k < RefCount; k++)
						{
							TESObjectREFR* ThisRef = (TESObjectREFR*)RefData[k].ParentForm;
							componentDLLInterface::CellObjectData* ThisRefData = &RefData[k];
							bool Modified = false;

							if (ThisRefData->Selected)
							{
								if (BatchData->World3DData.UsePosX())	ThisRef->position.x = BatchData->World3DData.PosX, Modified = true;
								if (BatchData->World3DData.UsePosY())	ThisRef->position.y = BatchData->World3DData.PosY, Modified = true;
								if (BatchData->World3DData.UsePosZ())	ThisRef->position.z = BatchData->World3DData.PosZ, Modified = true;

								if (BatchData->World3DData.PosChanged())
									ThisRef->SetPosition(ThisRef->position.x, ThisRef->position.y, ThisRef->position.z);

								if (BatchData->World3DData.UseRotX())	ThisRef->rotation.x = BatchData->World3DData.RotX * PI / 180, Modified = true;
								if (BatchData->World3DData.UseRotY())	ThisRef->rotation.y = BatchData->World3DData.RotY * PI / 180, Modified = true;
								if (BatchData->World3DData.UseRotZ())	ThisRef->rotation.z = BatchData->World3DData.RotZ * PI / 180, Modified = true;

								if (BatchData->World3DData.RotChanged())
									ThisRef->SetRotation(ThisRef->rotation.x, ThisRef->rotation.y, ThisRef->rotation.z, true);

								if (BatchData->World3DData.UseScale())	ThisRef->SetScale(BatchData->World3DData.Scale), Modified = true;

								if (BatchData->Flags.UsePersistent())
									ThisRef->SetPersistent(BatchData->Flags.Persistent), Modified = true;

								if (BatchData->Flags.UseDisabled())
									ThisRef->SetInitiallyDisabled(BatchData->Flags.Disabled), Modified = true;

								if (BatchData->Flags.UseVWD())
									ThisRef->SetVWD(BatchData->Flags.VWD), Modified = true;

								if (BatchData->EnableParent.UseEnableParent())
								{
									TESObjectREFR* Parent = (TESObjectREFR*)BatchData->EnableParent.Parent;
									if (Parent != ThisRef)
									{
										ThisRef->extraData.ModExtraEnableStateParent(Parent);
										ThisRef->SetExtraEnableStateParentOppositeState(BatchData->EnableParent.OppositeState);
										Modified = true;
									}
								}

								if (BatchData->Ownership.UseOwnership() &&
									ThisRef->baseForm->formType != TESForm::kFormType_NPC &&
									ThisRef->baseForm->formType != TESForm::kFormType_Creature)
								{
									ThisRef->extraData.ModExtraGlobal(nullptr);
									ThisRef->extraData.ModExtraRank(-1);
									ThisRef->extraData.ModExtraOwnership(nullptr);

									TESForm* Owner = (TESForm*)BatchData->Ownership.Owner;
									ThisRef->extraData.ModExtraOwnership(Owner);

									if (BatchData->Ownership.UseNPCOwner())
										ThisRef->extraData.ModExtraGlobal((TESGlobal*)BatchData->Ownership.Global);
									else
										ThisRef->extraData.ModExtraRank(BatchData->Ownership.Rank);

									Modified = true;
								}

								if (BatchData->Extra.UseCharge())		ThisRef->ModExtraCharge((float)BatchData->Extra.Charge), Modified = true;
								if (BatchData->Extra.UseHealth())		ThisRef->ModExtraHealth((float)BatchData->Extra.Health), Modified = true;
								if (BatchData->Extra.UseTimeLeft())		ThisRef->ModExtraTimeLeft((float)BatchData->Extra.TimeLeft), Modified = true;
								if (BatchData->Extra.UseSoulLevel())	ThisRef->ModExtraSoul(BatchData->Extra.SoulLevel), Modified = true;
								if (BatchData->Extra.UseCount())
								{
									switch (ThisRef->baseForm->formType)
									{
									case TESForm::kFormType_Apparatus:
									case TESForm::kFormType_Armor:
									case TESForm::kFormType_Book:
									case TESForm::kFormType_Clothing:
									case TESForm::kFormType_Ingredient:
									case TESForm::kFormType_Misc:
									case TESForm::kFormType_Weapon:
									case TESForm::kFormType_Ammo:
									case TESForm::kFormType_SoulGem:
									case TESForm::kFormType_Key:
									case TESForm::kFormType_AlchemyItem:
									case TESForm::kFormType_SigilStone:
										ThisRef->extraData.ModExtraCount(BatchData->Extra.Count), Modified = true;

										break;
									case TESForm::kFormType_Light:
										TESObjectLIGH* Light = CS_CAST(ThisRef->baseForm, TESForm, TESObjectLIGH);
										if (Light)
										{
											if (Light->IsCarriable())
												ThisRef->extraData.ModExtraCount(BatchData->Extra.Count), Modified = true;
										}

										break;
									}
								}
							}

							if (Modified)
								ThisRef->SetFromActiveFile(true);

							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_BatchEditor);
						}
					}

					delete BatchData;
				}
			});

			impl::BasicRWA ShowUseInfo("Use Info", "Display the reference's usage info.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					TESDialog::ShowUseReportDialog(Ref);
				}
			});

			impl::BasicRWA ShowSearchReplace("Search/Replace", "Display the Search/Replace dialog.", []() {
				TESCSMain::InvokeMainMenuTool(TESCSMain::kMainMenu_Edit_SearchReplace);
			});


			impl::BasicRWA ToggleAuxViewport("Toggle Auxiliary Viewport", "Show the auxiliary viewport window.",
											 IRenderWindowAction::kMode_All, []() {
				AUXVIEWPORT->ToggleVisibility();
				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_AuxViewPort);
			});

			impl::ToggleINISettingRWA ToggleStaticCameraPivot("Toggle Static Camera Pivot", "Use a fixed camera pivot when rotating without a selection.",
															  IRenderWindowAction::kMode_All, settings::renderer::kFixedCameraPivot);
			impl::BasicRWA ToggleAlternateMovementSettings("Toggle Alternate Movement Settings", "Use the auxiliary movement speed settings.",
														   IRenderWindowAction::kMode_All, []() {
				_RENDERWIN_XSTATE.UseAlternateMovementSettings = (_RENDERWIN_XSTATE.UseAlternateMovementSettings == false);

				if (_RENDERWIN_XSTATE.UseAlternateMovementSettings == false)
					NotificationOSDLayer::Instance.ShowNotification("Using vanilla movement settings");
				else
					NotificationOSDLayer::Instance.ShowNotification("Using alternate movement settings");

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_AlternateRenderWindowMovement);
			});

			impl::BasicRWA TogglePathGridEditMode("Toggle Path Grid Mode", "Toggle path grid editing mode.",
												  IRenderWindowAction::kMode_All, []() {
				TESCSMain::InvokeMainMenuTool(TESCSMain::kToolbar_PathGridEdit);
			});

			impl::BasicRWA ToggleSnapToGrid("Toggle Snap-To-Grid", "Toggle reference movement snap to grid.",
											IRenderWindowAction::kMode_All, []() {
				UInt32 Flags = *TESRenderWindow::StateFlags;

				if ((Flags & TESRenderWindow::kRenderWindowState_SnapToGrid))
					Flags &= ~TESRenderWindow::kRenderWindowState_SnapToGrid;
				else
					Flags |= TESRenderWindow::kRenderWindowState_SnapToGrid;

				*TESRenderWindow::StateFlags = Flags;
			});

			impl::BasicRWA ToggleSnapToAngle("Toggle Snap-To-Angle", "Toggle reference rotation snap to angle.",
											 IRenderWindowAction::kMode_All, []() {
				UInt32 Flags = *TESRenderWindow::StateFlags;

				if ((Flags & TESRenderWindow::kRenderWindowState_SnapToAngle))
					Flags &= ~TESRenderWindow::kRenderWindowState_SnapToAngle;
				else
					Flags |= TESRenderWindow::kRenderWindowState_SnapToAngle;

				*TESRenderWindow::StateFlags = Flags;
			});

			impl::ToggleVisibilityRWA ToggleVisibility[impl::ToggleVisibilityRWA::kType__MAX] =
			{
				(impl::ToggleVisibilityRWA::kType_Objects),
				(impl::ToggleVisibilityRWA::kType_Markers),
				(impl::ToggleVisibilityRWA::kType_Wireframe),
				(impl::ToggleVisibilityRWA::kType_BrightLight),
				(impl::ToggleVisibilityRWA::kType_Sky),
				(impl::ToggleVisibilityRWA::kType_SolidSubspaces),
				(impl::ToggleVisibilityRWA::kType_CollisionGeom),
				(impl::ToggleVisibilityRWA::kType_Leaves),
				(impl::ToggleVisibilityRWA::kType_Trees),
				(impl::ToggleVisibilityRWA::kType_Water),
				(impl::ToggleVisibilityRWA::kType_CellBorders),
				(impl::ToggleVisibilityRWA::kType_Land),
				(impl::ToggleVisibilityRWA::kType_LightRadius),
				(impl::ToggleVisibilityRWA::kType_ParentChildIndicator),
				(impl::ToggleVisibilityRWA::kType_PathGridLinkedRefIndicator),
				(impl::ToggleVisibilityRWA::kType_InitiallyDisabledRefs),
				(impl::ToggleVisibilityRWA::kType_InitiallyDisabledRefsChildren),
				(impl::ToggleVisibilityRWA::kType_GrassOverlay)
			};


			impl::BasicRWA FocusOnRefFilter("Focus Ref Filter", "Move keyboard focus to the bottom toolbar's reference filter text input.",
											IRenderWindowAction::kMode_All, []() {
				if (ToolbarOSDLayer::Instance.IsEnabled())
					ToolbarOSDLayer::Instance.FocusOnRefFilter();
			});

			impl::BasicRWA JumpToExteriorCell("Jump To Exterior", "Warp to the exterior cell at the given coordinates.",
											  IRenderWindowAction::kMode_All, []() {
				bool OnLoad = true;
				ModalWindowProviderOSDLayer::ModalRenderDelegateT RenderModalJumpToCell([OnLoad](RenderWindowOSD*, ImGuiDX9*, void*) mutable->bool {
					static char CellCoordBuffer[0x100] = { 0 };
					if (OnLoad)
					{
						OnLoad = false;
						ImGui::SetKeyboardFocusHere(1);
					}
					bool EnterKey = false;
					if (ImGui::InputText("Coordinates", CellCoordBuffer, sizeof(CellCoordBuffer),
										 ImGuiInputTextFlags_EnterReturnsTrue))
					{
						EnterKey = true;
					}

					int X = 0, Y = 0;
					bool InvalidInput = sscanf_s(CellCoordBuffer, "%d %d", &X, &Y) != 2;
					if (strlen(CellCoordBuffer) == 0)
						InvalidInput = true;

					if (InvalidInput)
						ImGui::Text("X and Y coordinates must be separated by a whitespace");
					else
						ImGui::Text("Destination coordinates: %d, %d", X, Y);

					ImGui::Separator();
					bool Close = false;
					if (ImGui::Button("OK", ImVec2(120, 0)) || EnterKey)
					{
						if (InvalidInput == false)
						{
							Vector3 Coords((X << 12) + 2048.0, (Y << 12) + 2048.0, 0);
							_TES->LoadCellIntoViewPort(&Coords, nullptr);
							Close = true;
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0)))
						Close = true;

					if (Close)
					{
						ZeroMemory(CellCoordBuffer, sizeof(CellCoordBuffer));
						return true;
					}
					else
						return false;
				});

				if (_TES->currentInteriorCell)
				{
					NotificationOSDLayer::Instance.ShowNotification("This tool can only be used in an exterior worldspace");
				}
				else
				{
					ModalWindowProviderOSDLayer::Instance.ShowModal("Jump to Exterior Cell",
																	RenderModalJumpToCell,
																	nullptr,
																	ImGuiWindowFlags_AlwaysAutoResize);
				}
			});
		}

	}
}