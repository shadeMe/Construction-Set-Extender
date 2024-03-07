#include "RenderWindowActions.h"
#include "RenderWindowInput.h"
#include "RenderWindowManager.h"
#include "Achievements.h"
#include "[Common]\CLIWrapper.h"
#include "AuxiliaryViewport.h"
#include "ToolbarOSDLayer.h"
#include "RenderWindowFlyCamera.h"

namespace cse
{
	namespace renderWindow
	{
		namespace actions
		{
			BasicRWA::BasicRWA(std::string Name, std::string Desc, ActionDelegateT Delegate) :
				IRenderWindowAction(Name, Desc, ExecutionContext::kMode_ReferenceEdit),
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
				if (Context.IsExecutable() == false)
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
				if (Context.IsExecutable() == false)
					return false;

				Setting.ToggleData();
				TESRenderWindow::Redraw();
				return true;
			}


			BuiltInKeyComboRWA::BuiltInKeyComboRWA(std::string Name, std::string Desc, UInt8 Context, input::BuiltIn::KeyBinding Binding, ActionDelegateT Delegate) :
				IRenderWindowAction(Name, Desc, Context),
				Key(Binding),
				Delegate(Delegate)
			{
				;//
			}

			BuiltInKeyComboRWA::~BuiltInKeyComboRWA()
			{
				;//
			}

			bool BuiltInKeyComboRWA::operator()()
			{
				if (Context.IsExecutable() == false)
					return false;

				if (Delegate)
					Delegate(Key);
				else
					Key.Trigger();

				return true;
			}

			const input::BuiltIn::KeyBinding& BuiltInKeyComboRWA::GetBuiltInBinding() const
			{
				return Key;
			}

			ToggleVisibilityRWA::ToggleVisibilityRWA(int Type) :
				IRenderWindowAction("", "", ExecutionContext::kMode_All),
				Type(Type)
			{
				SME_ASSERT(Type > kType__NONE && Type < kType__MAX);

				static const char* kNames[actions::ToggleVisibilityRWA::kType__MAX] =
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

				static const char* kDescriptions[actions::ToggleVisibilityRWA::kType__MAX] =
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
				if (Context.IsExecutable() == false)
					return false;

				switch (Type)
				{
				case kType_Objects:
					builtIn::ToggleObjects();
					break;
				case kType_Markers:
					builtIn::ToggleMarkers();
					break;
				case kType_Wireframe:
					builtIn::ToggleGlobalWireframe();
					break;
				case kType_BrightLight:
					builtIn::ToggleBrightLight();
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
					builtIn::ToggleCollisionGeom();
					break;
				case kType_Leaves:
					(*BSTreeManager::Singleton)->drawLeaves = (*BSTreeManager::Singleton)->drawLeaves == false;
					break;
				case kType_Trees:
					(*BSTreeManager::Singleton)->drawTrees = (*BSTreeManager::Singleton)->drawTrees == false;
					break;
				case kType_Water:
					builtIn::ToggleWater();
					break;
				case kType_CellBorders:
					builtIn::ToggleCellBorders();
					break;
				case kType_Land:
					builtIn::ToggleLandscape();
					break;
				case kType_LightRadius:
					builtIn::ToggleLightRadius();
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
					if (_RENDERWIN_XSTATE.GrassOverlayTexture == nullptr)
					{
						NotificationOSDLayer::Instance.ShowNotification("Set a valid grass overlay texture in CSE's preferences and restart the editor.");
						break;
					}

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
					return *TESRenderWindow::WaterHiddenFlag == 0;
				case kType_CellBorders:
					return _TES->drawCellBorders;
				case kType_Land:
					return *TESRenderWindow::LandscapeHiddenFlag == 0;
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
				IRenderWindowAction("", "", ExecutionContext::kMode_ReferenceEdit),
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
				if (Context.IsExecutable() == false)
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

			BasicRWA InvertSelection("Invert Selection", "Invert the current reference selection.", []() {
				const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
				if (Refs.size())
				{
					TESRenderSelection* Buffer = TESRenderSelection::CreateInstance(_RENDERSEL);
					_RENDERSEL->ClearSelection(true);

					for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
					{
						TESObjectREFR* Ref = *Itr;

						if (Buffer->HasObject(Ref) == false)
							ReferenceSelectionManager::AddToSelection(Ref, true);
					}

					Buffer->DeleteInstance();
				}
			});

			BasicRWA SelectAll("Select All", "Select all references in the active cell/grid.", []() {
				_RENDERSEL->ClearSelection(true);

				for (const auto& Ref : _RENDERWIN_MGR.GetActiveRefs())
					ReferenceSelectionManager::AddToSelection(Ref, true);
			});

			AlignReferencesRWA AlignPosition(AlignReferencesRWA::kAlign_Position, AlignReferencesRWA::kAxis_ALL);
			AlignReferencesRWA AlignRotation(AlignReferencesRWA::kAlign_Rotation, AlignReferencesRWA::kAxis_ALL);

			BasicRWA GroupSelection("Group", "Group references into a single selectable entity.", []() {
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
					if (ImGui::IsKeyReleased(VK_ESCAPE))
						Close = true;
					else
					{
						if (ImGui::Button("OK", ImVec2(120, 0)) || EnterKey)
						{
							if (InvalidName == false)
							{
								if (_RENDERWIN_MGR.GetGroupManager()->AddGroup(NewGroupNameBuffer, _RENDERSEL) == false)
									NotificationOSDLayer::Instance.ShowNotification("Couldn't add current selection to a new group.");
								else
									NotificationOSDLayer::Instance.ShowNotification("Created new selection group '%s'", NewGroupNameBuffer);

								achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefGrouping);
								Close = true;
							}
						}

						ImGui::SameLine();
						if (ImGui::Button("Cancel", ImVec2(120, 0)))
							Close = true;
					}

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

			BasicRWA UngroupSelection("Ungroup", "Dissolve a group into its individual references.", []() {
				_RENDERWIN_MGR.GetGroupManager()->RemoveSelectionGroups(_RENDERSEL);
			});

			BasicRWA OrphanizeSelection("Orphanize", "Remove the reference from its parent group, if any.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					_RENDERWIN_MGR.GetGroupManager()->Orphanize(Ref);
				}
			});


			BasicRWA FreezeSelection("Freeze", "Prevent references from being selected.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetFrozen(true);
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefFreezing);
			});

			BasicRWA ThawSelection("Thaw", "Allow previously frozen references to be selected.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetFrozen(false);
				}
			});

			BasicRWA ThawAll("Thaw All", "Defrost all frozen references.", []() {
				const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
				for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
				{
					TESObjectREFR* Ref = *Itr;
					Ref->SetFrozen(false);
				}

				NotificationOSDLayer::Instance.ShowNotification("Thawed all of the active cell/grid's references");
			});

			BasicRWA ToggleFreezeInactive("Toggle Freeze Inactive", "Prevent references that don't belong to the active plugin from being selected.", []() {
				_RENDERWIN_XSTATE.FreezeInactiveRefs = _RENDERWIN_XSTATE.FreezeInactiveRefs == false;

				if (_RENDERWIN_XSTATE.FreezeInactiveRefs)
					NotificationOSDLayer::Instance.ShowNotification("Inactive references frozen");
				else
					NotificationOSDLayer::Instance.ShowNotification("Inactive references thawed");
			});


			BasicRWA ToggleSelectionVisibility("Hide", "Hide the reference.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->ToggleInvisiblity();
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefVisibility);
			});

			BasicRWA ToggleSelectionChildrenVisibility("Hide Children", "Hide the reference's enable-state children.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->ToggleChildrenInvisibility();
				}

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefVisibility);
			});

			BasicRWA RevealAll("Reveal All", "Show all hidden references.", []() {
				const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
				for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
				{
					TESObjectREFR* Ref = *Itr;

					if (Ref->IsInvisible())
						Ref->ToggleInvisiblity();

					if (Ref->IsChildrenInvisible())
						Ref->ToggleChildrenInvisibility();
				}

				NotificationOSDLayer::Instance.ShowNotification("Reset visibility flags on the active cell/grid's references");
			});


			BasicRWA DimSelectionOpacity("Dim Opacity", "Make the reference partially transparent.", []() {
				float Alpha = settings::renderer::kRefToggleOpacityAlpha().f;
				if (Alpha < 0.1)
					Alpha = 0.1;

				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetAlpha(Alpha);
				}
			});

			BasicRWA ResetSelectionOpacity("Reset Opacity", "Make the reference fully opaque.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Ref->SetAlpha();
				}
			});

			BasicRWA UnlinkPathGridSelection("Unlink Path Grid Reference", "Remove the path grid point's linked reference, if any",
												   ExecutionContext::kMode_PathGridEdit, []() {
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

			BasicRWA ShowBatchEditor("Toggle Reference Batch Editor", "Display the batch reference editor.", []() {
				settings::renderWindowOSD::kShowRefBatchEditor.ToggleData();
			});

			BasicRWA ShowUseInfo("Use Info", "Display the reference's usage info.", []() {
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					TESDialog::ShowUseReportDialog(Ref);
				}
			});

			BasicRWA ShowSearchReplace("Search/Replace", "Display the Search/Replace dialog.", []() {
				TESCSMain::InvokeMainMenuTool(TESCSMain::kMainMenu_Edit_SearchReplace);
			});


			BasicRWA ToggleAuxViewport("Toggle Auxiliary Viewport", "Show the auxiliary viewport window.",
											 ExecutionContext::kMode_All, []() {
				AUXVIEWPORT->ToggleVisibility();
				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_AuxViewPort);
			});

			BasicRWA ToggleStaticCameraPivot("Toggle Static Camera Pivot", "Use a fixed camera pivot when rotating without a selection.",
											 ExecutionContext::kMode_All, []() {
				settings::renderer::kFixedCameraPivot.ToggleData();

				if (settings::renderer::kFixedCameraPivot().i)
					NotificationOSDLayer::Instance.ShowNotification("Using fixed camera pivot");
				else
					NotificationOSDLayer::Instance.ShowNotification("Using standard camera pivot");
			});
			BasicRWA ToggleAlternateMovementSettings("Toggle Alternate Movement Settings", "Use the auxiliary movement speed settings.",
														   ExecutionContext::kMode_All, []() {
				_RENDERWIN_XSTATE.UseAlternateMovementSettings = (_RENDERWIN_XSTATE.UseAlternateMovementSettings == false);

				if (_RENDERWIN_XSTATE.UseAlternateMovementSettings == false)
					NotificationOSDLayer::Instance.ShowNotification("Using vanilla movement settings");
				else
					NotificationOSDLayer::Instance.ShowNotification("Using alternate movement settings");

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_AlternateRenderWindowMovement);
			});

			BasicRWA TogglePathGridEditMode("Toggle Path Grid Mode", "Toggle path grid editing mode.",
												  ExecutionContext::kMode_All, []() {
				TESCSMain::InvokeMainMenuTool(TESCSMain::kToolbar_PathGridEdit);
			});

			BasicRWA ToggleSnapToGrid("Toggle Snap-To-Grid", "Toggle reference movement snap to grid.",
											ExecutionContext::kMode_All, []() {
				UInt32 Flags = *TESRenderWindow::StateFlags;

				if ((Flags & TESRenderWindow::kRenderWindowState_SnapToGrid))
					Flags &= ~TESRenderWindow::kRenderWindowState_SnapToGrid;
				else
					Flags |= TESRenderWindow::kRenderWindowState_SnapToGrid;

				*TESRenderWindow::StateFlags = Flags;
			});

			BasicRWA ToggleSnapToAngle("Toggle Snap-To-Angle", "Toggle reference rotation snap to angle.",
											 ExecutionContext::kMode_All, []() {
				UInt32 Flags = *TESRenderWindow::StateFlags;

				if ((Flags & TESRenderWindow::kRenderWindowState_SnapToAngle))
					Flags &= ~TESRenderWindow::kRenderWindowState_SnapToAngle;
				else
					Flags |= TESRenderWindow::kRenderWindowState_SnapToAngle;

				*TESRenderWindow::StateFlags = Flags;
			});

			ToggleVisibilityRWA ToggleVisibility[ToggleVisibilityRWA::kType__MAX] =
			{
				(ToggleVisibilityRWA::kType_Objects),
				(ToggleVisibilityRWA::kType_Markers),
				(ToggleVisibilityRWA::kType_Wireframe),
				(ToggleVisibilityRWA::kType_BrightLight),
				(ToggleVisibilityRWA::kType_Sky),
				(ToggleVisibilityRWA::kType_SolidSubspaces),
				(ToggleVisibilityRWA::kType_CollisionGeom),
				(ToggleVisibilityRWA::kType_Leaves),
				(ToggleVisibilityRWA::kType_Trees),
				(ToggleVisibilityRWA::kType_Water),
				(ToggleVisibilityRWA::kType_CellBorders),
				(ToggleVisibilityRWA::kType_Land),
				(ToggleVisibilityRWA::kType_LightRadius),
				(ToggleVisibilityRWA::kType_ParentChildIndicator),
				(ToggleVisibilityRWA::kType_PathGridLinkedRefIndicator),
				(ToggleVisibilityRWA::kType_InitiallyDisabledRefs),
				(ToggleVisibilityRWA::kType_InitiallyDisabledRefsChildren),
				(ToggleVisibilityRWA::kType_GrassOverlay)
			};


			BasicRWA FocusOnRefFilter("Focus Ref Filter", "Move keyboard focus to the bottom toolbar's reference filter text input.",
											ExecutionContext::kMode_All, []() {
				if (ToolbarOSDLayer::Instance.IsEnabled())
					ToolbarOSDLayer::Instance.FocusOnRefFilter();
			});

			BasicRWA JumpToExteriorCell("Jump To Exterior", "Warp to the exterior cell at the given coordinates.",
											  ExecutionContext::kMode_All, []() {
				ModalWindowProviderOSDLayer::ModalRenderDelegateT RenderModalJumpToCell([](RenderWindowOSD*, ImGuiDX9*, void*)->bool {
					static char CellCoordBuffer[0x100] = { 0 };

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
					if (ImGui::IsKeyReleased(VK_ESCAPE))
						Close = true;
					else
					{
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
					}

					if (Close)
					{
						ZeroMemory(CellCoordBuffer, sizeof(CellCoordBuffer));
						return true;
					}
					else
						return false;
				});

				if (TESRenderWindow::IsAnyCellLoaded() && !(*TESRenderWindow::ActiveCell)->IsInterior())
				{
					ModalWindowProviderOSDLayer::Instance.ShowModal("Jump to Exterior Cell",
																	RenderModalJumpToCell,
																	nullptr,
																	ImGuiWindowFlags_AlwaysAutoResize);
				}
				else
				{
					NotificationOSDLayer::Instance.ShowNotification("This tool can only be used in an exterior worldspace");

				}
			});

			BasicRWA ToggleFlyCamera("Toggle Fly Camera", "Enable/Disable free camera movement inside the render window.",
				ExecutionContext::kMode_All, []() {

				if (RenderWindowFlyCamera::Instance.IsActive())
					RenderWindowFlyCamera::Instance.Deactivate();
				else
				{
					RenderWindowFlyCamera::Instance.Activate();
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_FlyCamera);
				}
			});

			BasicRWA ToggleOSD("Toggle On-Screen-Display", "Show/Hide the render window on-screen-display.",
				ExecutionContext::kMode_All, []() {
				_RENDERWIN_MGR.GetOSD()->ToggleRendering();
			});

			BasicRWA CreateMeasureRuler("Create Ruler Measure", "Create a temporary ruler measure reference at the location of the cursor.",
				ExecutionContext::kMode_ReferenceEdit, []() {
				if (TESRenderWindow::IsAnyCellLoaded() == false)
					return;

				auto NewRef = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_REFR), TESForm, TESObjectREFR);
				NewRef->SetBaseForm(_RENDERWIN_XSTATE.MeasureBaseRuler);
				NewRef->MarkAsTemporary();

				TESRenderWindow::PlaceRefAtMousePos(NewRef, TESRenderWindow::LastMouseCoords->x, TESRenderWindow::LastMouseCoords->y);
				builtIn::TopCamera();

				NotificationOSDLayer::Instance.ShowNotification("Create new ruler measure");
			});

			BasicRWA CreateMeasureCircle("Create Circle Measure", "Create a temporary circle measure reference at the location of the cursor.",
				ExecutionContext::kMode_ReferenceEdit, []() {
				if (TESRenderWindow::IsAnyCellLoaded() == false)
					return;

				auto NewRef = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_REFR), TESForm, TESObjectREFR);
				NewRef->SetBaseForm(_RENDERWIN_XSTATE.MeasureBaseCircle);
				NewRef->MarkAsTemporary();

				TESRenderWindow::PlaceRefAtMousePos(NewRef, TESRenderWindow::LastMouseCoords->x, TESRenderWindow::LastMouseCoords->y);
				builtIn::TopCamera();

				NotificationOSDLayer::Instance.ShowNotification("Create new circle measure");
			});


			namespace builtIn
			{
				using namespace input;
				typedef BuiltIn::KeyBinding Key;

				BuiltInKeyComboRWA ReloadAllPathGrids("Reload All Path Grids", "Refresh all loaded path grids.",
													  ExecutionContext::kMode_All, Key('P', BuiltIn::kModifier_Control | BuiltIn::kModifier_Shift));
				BuiltInKeyComboRWA GenerateActiveExteriorLocalMap("Generate Current Exterior Local Map", "Save the local map of the current exterior cell.",
																  ExecutionContext::kMode_All, Key('P'));
				BuiltInKeyComboRWA ToggleBrightLight("Toggle Bright Light", "Toggle bright light rendering.", ExecutionContext::kMode_All, Key('A'));
				BuiltInKeyComboRWA ToggleLandscapeEditMode("Toggle Landscape Edit Mode", "Turn on landscape sculpting/painting mode.", ExecutionContext::kMode_All, Key('H'));
				BuiltInKeyComboRWA ToggleCollisionGeom("Toggle Collision Geometry", "Toggle visibility of collision geometry.", ExecutionContext::kMode_All, Key(VK_F4));
				BuiltInKeyComboRWA RefreshRenderWindow("Refresh Render Window", "Reload textures and refresh current cell", ExecutionContext::kMode_All, Key(VK_F5), [](Key& Key) {
					// prevent triggering the key when rendering the scene as it'll cause a deadlock
					SME_ASSERT(_RENDERWIN_MGR.IsRenderingScene() == false);
					Key.Trigger();
				});
				BuiltInKeyComboRWA MoveCamYAxisPos("Move Camera Y-Axis +", "Move camera in the positive Y-axis.", ExecutionContext::kMode_All, Key(VK_UP));
				BuiltInKeyComboRWA MoveCamYAxisNeg("Move Camera Y-Axis -", "Move camera in the negative Y-axis.", ExecutionContext::kMode_All, Key(VK_DOWN));
				BuiltInKeyComboRWA MoveCamXAxisPos("Move Camera X-Axis +", "Move camera in the positive X-axis.", ExecutionContext::kMode_All, Key(VK_RIGHT));
				BuiltInKeyComboRWA MoveCamXAxisNeg("Move Camera X-Axis -", "Move camera in the negative X-axis.", ExecutionContext::kMode_All, Key(VK_LEFT));
				BuiltInKeyComboRWA PitchCamPos("Pitch Camera +", "Increase camera pitch.", ExecutionContext::kMode_All, Key(VK_UP, BuiltIn::kModifier_Shift));
				BuiltInKeyComboRWA PitchCamNeg("Patch Camera -", "Decrease camera pitch.", ExecutionContext::kMode_All, Key(VK_DOWN, BuiltIn::kModifier_Shift));
				BuiltInKeyComboRWA YawCamPos("Yaw Camera +", "Increase camera yaw.", ExecutionContext::kMode_All, Key(VK_LEFT, BuiltIn::kModifier_Shift));
				BuiltInKeyComboRWA YawCamNeg("Yaw Camera -", "Decrease camera yaw.", ExecutionContext::kMode_All, Key(VK_RIGHT, BuiltIn::kModifier_Shift));
				BuiltInKeyComboRWA RollCamPos("Roll Camera +", "Increase camera roll.", ExecutionContext::kMode_All, Key(VK_HOME));
				BuiltInKeyComboRWA RollCamNeg("Roll Camera -", "Decrease camera roll.", ExecutionContext::kMode_All, Key(VK_PRIOR));
				BuiltInKeyComboRWA ShowLandUseDialog("Show Landscape Texture Usage", "Display the landscape texture use info dialog.", ExecutionContext::kMode_LandscapeEdit, Key('I'));
				BuiltInKeyComboRWA ShowSelectedRefInfo("Show Reference Selection Info", "Display the current selection's basic information.",
													   ExecutionContext::kMode_ReferenceEdit, Key('R', BuiltIn::kModifier_Control | BuiltIn::kModifier_Shift | BuiltIn::kModifier_Space));
				BuiltInKeyComboRWA LinkPathGridSelection("Link Path Grid Reference", "Link the path grid point to a reference.",
														 ExecutionContext::kMode_PathGridEdit, Key('R'), [](Key&) {
					// we recreate the code instead of tunneling the key
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
				BuiltInKeyComboRWA ToggleMarkers("Toggle Markers", "Toggle visibility of markers.", ExecutionContext::kMode_All, Key('M'));
				BuiltInKeyComboRWA ToggleWater("Toggle Water", "Toggle visibility of water.", ExecutionContext::kMode_All, Key('W', BuiltIn::kModifier_Shift));
				BuiltInKeyComboRWA ToggleGlobalWireframe("Toggle Global Wireframe Rendering", "Toggle global wireframe rendering.", ExecutionContext::kMode_All, Key('W'));
				BuiltInKeyComboRWA ToggleLandscape("Toggle Landscape", "Toggle visibility of landscape.", ExecutionContext::kMode_All, Key('L', BuiltIn::kModifier_Shift));
				BuiltInKeyComboRWA ToggleLightRadius("Toggle Light Radius", "Toggle visibility of light radius", ExecutionContext::kMode_All, Key('L'));
				BuiltInKeyComboRWA Fall("Fall Selection", "Move selection to the nearest collision plane in the negative Z-axis.",
										ExecutionContext::kMode_ReferenceEdit | ExecutionContext::kMode_PathGridEdit, Key('F'), [](Key& KeyBinding) {
					if (*TESRenderWindow::PathGridEditFlag)
					{
						// prevent the linked ref indicator trishape from interfering with the fall operation
						std::vector<NiAVObject*> Delinquents;

						for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
						{
							NiNode* Node = Itr.Get()->pointNiNode;
							if (Node)
							{
								for (int i = 0; i < Node->m_children.numObjs; i++)
								{
									NiAVObject* Child = Node->m_children.data[i];
									if (Child && Child->IsCulled() == false && TESRender::GetProperty(Child, NiWireframeProperty::kType))
									{
										// the bounding box trishape is the only child with the wireframe property
										Delinquents.push_back(Child);
										Child->SetCulled(true);
									}
								}
							}
						}

						// execute the operation
						KeyBinding.Trigger();

						// reset culled state
						for (auto Itr : Delinquents)
							Itr->SetCulled(false);
					}
					else
					{
						// execute as usual in ref edit mode
						KeyBinding.Trigger();
					}
				});
				BuiltInKeyComboRWA SavePlugin("Save Active Plugin", "Save the active plugin.", ExecutionContext::kMode_All, Key('S', BuiltIn::kModifier_Control));
				BuiltInKeyComboRWA Cut("Cut Selection", "Cut the reference selection.", ExecutionContext::kMode_ReferenceEdit, Key('X', BuiltIn::kModifier_Control));
				BuiltInKeyComboRWA Redo("Redo", "Redo the last change that was undone.", ExecutionContext::kMode_All, Key('Y', BuiltIn::kModifier_Control), [](Key& KeyBinding) {
					if (*TESRenderWindow::PathGridEditFlag)
					{
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_PathGridAdditions);
						_RENDERWIN_MGR.GetPathGridUndoManager()->PerformRedo();
					}
					else
						KeyBinding.Trigger();
				});
				BuiltInKeyComboRWA Undo("Undo", "Undo the last change that was performed.", ExecutionContext::kMode_All, Key('Z', BuiltIn::kModifier_Control), [](Key& KeyBinding) {
					if (*TESRenderWindow::PathGridEditFlag)
					{
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_PathGridAdditions);
						_RENDERWIN_MGR.GetPathGridUndoManager()->PerformUndo();
					}
					else
						KeyBinding.Trigger();
				});
				BuiltInKeyComboRWA UndoEx("Undo (Secondary)", "Undo the last change that was performed.", ExecutionContext::kMode_All, Key(VK_BACK), [](Key&) {
					Undo();
				});
				BuiltInKeyComboRWA CullLightSelection("Cull Light Selection", "Turn off light source.",
													  ExecutionContext::kMode_ReferenceEdit, Key('C', BuiltIn::kModifier_Control | BuiltIn::kModifier_Alt));
				BuiltInKeyComboRWA ToggleObjects("Toggle Objects", "Toggle visibility of non-land objects.", ExecutionContext::kMode_All, Key('C', BuiltIn::kModifier_Shift));
				BuiltInKeyComboRWA AutoLinkPathGridSelection("Auto-link Path Grid Point Selection", "Automatically link the selection with nearby path grid points.",
															 ExecutionContext::kMode_PathGridEdit, Key('C', BuiltIn::kModifier_Control));
				BuiltInKeyComboRWA Copy("Copy Selection", "Copy the reference selection to the clipboard.", ExecutionContext::kMode_ReferenceEdit, Key('C', BuiltIn::kModifier_Control));
				BuiltInKeyComboRWA FallCam("Floor Camera", "Move camera to the ground in exterior cells.", ExecutionContext::kMode_All, Key('C', BuiltIn::kModifier_Z), [](Key& KeyBinding) {
					// only in exteriors
					if (_TES->GetCurrentCell()->IsInterior())
						return;
					else
						KeyBinding.Trigger();
				});
				BuiltInKeyComboRWA CenterCam("Center Camera", "Isometrically center camera on selection/cursor.", ExecutionContext::kMode_All, Key('C'));
				BuiltInKeyComboRWA ToggleVertexColoring("Toggle Vertex Color Edit Mode", "Turn on landscape vertex color painting mode.", ExecutionContext::kMode_LandscapeEdit, Key('Q'));
				BuiltInKeyComboRWA TopCamera("Top Camera", "Focus camera on selection top.", ExecutionContext::kMode_All, Key('T'));
				BuiltInKeyComboRWA Duplicate("Duplicate Selection", "Duplicate the reference selection.", ExecutionContext::kMode_ReferenceEdit, Key('D', BuiltIn::kModifier_Control));
				BuiltInKeyComboRWA DeselectAll("Clear Current Selection", "Deselect all references/path grid points.",
											   ExecutionContext::kMode_ReferenceEdit | ExecutionContext::kMode_PathGridEdit, Key('D'), [](Key&) {
					_RENDERSEL->ClearSelection(true);

					for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin();
						 !Itr.End() && Itr.Get();
						 ++Itr)
					{
						TESPathGridPoint* Point = Itr.Get();
						Point->HideSelectionRing();
					}

					TESRenderWindow::SelectedPathGridPoints->RemoveAll();
				});
				BuiltInKeyComboRWA RunHavokSim("Toggle Havok Sim", "Turn on real-time Havok simulation mode.", ExecutionContext::kMode_ReferenceEdit, Key(VK_SCROLL));
				BuiltInKeyComboRWA Delete("Delete Selection", "Delete the current selection.",
										  ExecutionContext::kMode_ReferenceEdit | ExecutionContext::kMode_PathGridEdit, Key(VK_DELETE), [](Key& KeyBinding) {
					// clear the picked objects just in case they are about to be deleted
					_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
					_RENDERWIN_XSTATE.CurrentMousePathGridPoint = nullptr;

					KeyBinding.Trigger();
				});
				BuiltInKeyComboRWA ToggleSelectionWireframe("Toggle Selection Wireframe Rendering", "Toggle individual wireframe rendering.", ExecutionContext::kMode_ReferenceEdit, Key('1'));
				BuiltInKeyComboRWA CheckBounds("Check All Bounds", "Perform bounds check on loaded objects.",
											   ExecutionContext::kMode_All, Key('B', BuiltIn::kModifier_Control | BuiltIn::kModifier_Shift));
				BuiltInKeyComboRWA ToggleCellBorders("Toggle Cell Borders", "Toggle visibility of cell borders.", ExecutionContext::kMode_All, Key('B'));
				BuiltInKeyComboRWA Paste("Paste Selection", "Paste the reference selection in front of the camera.",
										 ExecutionContext::kMode_ReferenceEdit, Key('V', BuiltIn::kModifier_Control));
				BuiltInKeyComboRWA PasteInPlace("Paste Selection In Place", "Paste the reference selection at its original coordinates.",
										 ExecutionContext::kMode_ReferenceEdit, Key('V', BuiltIn::kModifier_Control | BuiltIn::kModifier_Shift));
			}

		}

	}
}