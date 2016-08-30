#include "RenderWindowActions.h"
#include "RenderWindowHotKeys.h"
#include "RenderWindowManager.h"
#include "Achievements.h"
#include "[Common]\CLIWrapper.h"
#include "AuxiliaryViewport.h"

namespace cse
{
	namespace renderWindow
	{
		namespace actions
		{
			IRenderWindowAction::IRenderWindowAction(std::string Desc) :
				Description(Desc)
			{
				;//
			}

			const char* IRenderWindowAction::GetDescription() const
			{
				return Description.c_str();
			}

			namespace impl
			{

				BasicRWA::BasicRWA(std::string Desc, ActionDelegateT Delegate) :
					IRenderWindowAction(Desc),
					Delegate(Delegate)
				{
					SME_ASSERT(Delegate);
				}

				BasicRWA::~BasicRWA()
				{
					;//
				}

				void BasicRWA::operator()()
				{
					Delegate();
					TESRenderWindow::Redraw();
				}

				ToggleINISettingRWA::ToggleINISettingRWA(std::string Desc, SME::INI::INISetting& Setting) :
					IRenderWindowAction(Desc),
					Setting(Setting)
				{
					SME_ASSERT(Setting.GetType() == SME::INI::INISetting::kType_Integer ||
							   Setting.GetType() == SME::INI::INISetting::kType_Unsigned);
				}

				ToggleINISettingRWA::~ToggleINISettingRWA()
				{
					;//
				}

				void ToggleINISettingRWA::operator()()
				{
					Setting.ToggleData();
					TESRenderWindow::Redraw();
				}

				ToggleVisibilityRWA::ToggleVisibilityRWA(int Type) :
					IRenderWindowAction(""),
					Type(Type)
				{
					SME_ASSERT(Type > kType__NONE && Type < kType__MAX);

					Description = "Toggle visibility of ";
					switch (Type)
					{
					case kType_Objects:
						Description.append("non-land objects");
						break;
					case kType_Markers:
						Description.append("markers");
						break;
					case kType_Wireframe:
						Description = "Toggle wireframe rendering";
						break;
					case kType_BrightLight:
						Description = "Toggle bright light rendering";
						break;
					case kType_Sky:
						Description.append("the sky");
						break;
					case kType_SolidSubspaces:
						Description.append("solid subspaces");
						break;
					case kType_CollisionGeom:
						Description.append("collision geometry");
						break;
					case kType_Leaves:
						Description.append("leaves");
						break;
					case kType_Trees:
						Description.append("trees");
						break;
					case kType_Water:
						Description.append("water");
						break;
					case kType_CellBorders:
						Description.append("cell borders");
						break;
					case kType_Land:
						Description.append("landscape");
						break;
					case kType_LightRadius:
						Description.append("light radius");
						break;
					case kType_ParentChildIndicator:
						Description.append("parent-child indicators");
						break;
					case kType_PathGridLinkedRefIndicator:
						Description.append("path grid point linked reference indicators");
						break;
					case kType_InitiallyDisabledRefs:
						Description.append("initially disabled references");
						break;
					case kType_InitiallyDisabledRefsChildren:
						Description.append("initially disabled refrences' children");
						break;
					case kType_GrassOverlay:
						Description.append("the grass overlay texture");
						break;
					}

					Description.append(".");
				}

				ToggleVisibilityRWA::~ToggleVisibilityRWA()
				{
					;//
				}

				void ToggleVisibilityRWA::operator()()
				{
					switch (Type)
					{
					case kType_Objects:
						_RENDERWIN_MGR.GetHotKeyManager()->SendDefaultHotKey('C', true, false);
						break;
					case kType_Markers:
						_RENDERWIN_MGR.GetHotKeyManager()->SendDefaultHotKey('M', false, false);
						break;
					case kType_Wireframe:
						_RENDERWIN_MGR.GetHotKeyManager()->SendDefaultHotKey('W', false, false);
						break;
					case kType_BrightLight:
						_RENDERWIN_MGR.GetHotKeyManager()->SendDefaultHotKey('A', false, false);
						break;
					case kType_Sky:
						// using the key message is a bit of bother as it alters the stored Y key state
						// we could override it in the hotkey manager but we just route the message to the main window (which is what the shortcut does anyway)
						TESCSMain::InvokeMainMenuTool(TESCSMain::kMainMenu_View_Sky);
						break;
					case kType_SolidSubspaces:
						{
							Setting* Setting = INISettingCollection::Instance->LookupByName("bDrawFullSubSpaceBox:General");
							Setting->value.b = Setting->value.b == false;
						}
						break;
					case kType_CollisionGeom:
						_RENDERWIN_MGR.GetHotKeyManager()->SendDefaultHotKey(VK_F4, false, false);
						break;
					case kType_Leaves:
						(*BSTreeManager::Singleton)->drawLeaves = (*BSTreeManager::Singleton)->drawLeaves == false;
						break;
					case kType_Trees:
						(*BSTreeManager::Singleton)->drawTrees = (*BSTreeManager::Singleton)->drawTrees == false;
						break;
					case kType_Water:
						_RENDERWIN_MGR.GetHotKeyManager()->SendDefaultHotKey('W', true, false);
						break;
					case kType_CellBorders:
						_RENDERWIN_MGR.GetHotKeyManager()->SendDefaultHotKey('B', false, false);
						break;
					case kType_Land:
						_RENDERWIN_MGR.GetHotKeyManager()->SendDefaultHotKey('L', true, false);
						break;
					case kType_LightRadius:
						_RENDERWIN_MGR.GetHotKeyManager()->SendDefaultHotKey('L', false, false);
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
					}

					TESRenderWindow::Redraw(Type == kType_PathGridLinkedRefIndicator);
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
				}

			}

			impl::BasicRWA InvertSelection("Invert the current reference selection.", []() {
				if (*TESRenderWindow::PathGridEditFlag == 0)
				{
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
				}
			});

			impl::BasicRWA SelectAll("Select all references in the active cell/grid.", []() {
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

			impl::BasicRWA GroupSelection("Group references into a single selectable entity.", []() {
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

			impl::BasicRWA UngroupSelection("Dissolve a group into its individual references.", []() {
				if (_RENDERWIN_MGR.GetGroupManager()->RemoveGroup(_RENDERSEL) == false)
					NotificationOSDLayer::Instance.ShowNotification("Couldn't dissolve the current selection's group.");
				else
					NotificationOSDLayer::Instance.ShowNotification("Removed selection group");
			});

			impl::BasicRWA OrphanizeSelection("Remove the reference from its parent group, if any.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					_RENDERWIN_MGR.GetGroupManager()->Orphanize(Ref);
				}
			});


			impl::BasicRWA FreezeSelection("Prevent references from being selected.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetFrozen(true);
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefFreezing);
			});

			impl::BasicRWA ThawSelection("Allow previously frozen references to be selected.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetFrozen(false);
				}
			});

			impl::BasicRWA ThawAll("", []() {
				const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
				for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
				{
					TESObjectREFR* Ref = *Itr;
					Ref->SetFrozen(false);
				}

				NotificationOSDLayer::Instance.ShowNotification("Thawed all of the active cell/grid's references");
			});

			impl::BasicRWA ToggleFreezeInactive("Prevent references that don't belong to the active plugin from being selected.", []() {
				_RENDERWIN_XSTATE.FreezeInactiveRefs = _RENDERWIN_XSTATE.FreezeInactiveRefs == false;
			});


			impl::BasicRWA ToggleSelectionVisibility("Hide the reference.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->ToggleInvisiblity();
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefVisibility);
			});

			impl::BasicRWA ToggleSelectionChildrenVisibility("Hide the reference's enable-state children.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->ToggleChildrenInvisibility();
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefVisibility);
			});

			impl::BasicRWA RevealAll("Show all hidden references.", []() {
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


			impl::BasicRWA DimSelectionOpacity("Make the reference partially transparent.", []() {
				float Alpha = settings::renderer::kRefToggleOpacityAlpha().f;
				if (Alpha < 0.1)
					Alpha = 0.1;

				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetAlpha(Alpha);
				}
			});

			impl::BasicRWA ResetSelectionOpacity("Make the reference fully opaque.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetAlpha();
				}
			});


			impl::BasicRWA UnlinkPathGridSelection("Remove the path grid point's linked reference, if any", []() {
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

			impl::BasicRWA ShowBatchEditor("Display the batch reference editor.", []() {
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

			impl::BasicRWA ShowUseInfo("Display the reference's usage info.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					TESDialog::ShowUseReportDialog(Ref);
				}
			});

			impl::BasicRWA ShowSearchReplace("Display the Search/Replace dialog.", []() {
				TESCSMain::InvokeMainMenuTool(TESCSMain::kMainMenu_Edit_SearchReplace);
			});


			impl::BasicRWA ToggleAuxViewport("Toggle the auxiliary viewport window", []() {
				AUXVIEWPORT->ToggleVisibility();
			});

			impl::ToggleINISettingRWA ToggleStaticCameraPivot("Toggle the static camera pivot", settings::renderer::kFixedCameraPivot);
			impl::BasicRWA ToggleAlternateMovementSettings("Toggles the alternate movement settings", []() {
				_RENDERWIN_XSTATE.UseAlternateMovementSettings = _RENDERWIN_XSTATE.UseAlternateMovementSettings == false;
			});

			impl::BasicRWA TogglePathGridEditMode("", []() {

			});

			impl::BasicRWA ToggleSnapToGrid("", []() {

			});

			impl::BasicRWA ToggleSnapToAngle("", []() {

			});


			impl::ToggleVisibilityRWA ToggleVisibilityObjects;
			impl::ToggleVisibilityRWA ToggleVisibilityMarkers;
			impl::ToggleVisibilityRWA ToggleVisibilityWireframe;
			impl::ToggleVisibilityRWA ToggleVisibilityBrightLight;
			impl::ToggleVisibilityRWA ToggleVisibilitySky;
			impl::ToggleVisibilityRWA ToggleVisibilitySolidSubspaces;
			impl::ToggleVisibilityRWA ToggleVisibilityCollisionGeom;
			impl::ToggleVisibilityRWA ToggleVisibilityLeaves;
			impl::ToggleVisibilityRWA ToggleVisibilityTrees;
			impl::ToggleVisibilityRWA ToggleVisibilityWater;
			impl::ToggleVisibilityRWA ToggleVisibilityLandscape;
			impl::ToggleVisibilityRWA ToggleVisibilityCellBorders;
			impl::ToggleVisibilityRWA ToggleVisibilityParentChildIndicator;
			impl::ToggleVisibilityRWA ToggleVisibilityPathGridLinkedRefIndicator;
			impl::ToggleVisibilityRWA ToggleVisibilityInitiallyDisabledRefs;
			impl::ToggleVisibilityRWA ToggleVisibilityInitiallyDisabledRefsChildren;
			impl::ToggleVisibilityRWA ToggleVisibilityGrassOverlay;

			impl::BasicRWA FocusOnRefFilter("", []() {

			});

		}

	}
}