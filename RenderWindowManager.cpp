#include "RenderWindowManager.h"
#include "Construction Set Extender_Resource.h"
#include "AuxiliaryViewport.h"
#include "ObjectPaletteManager.h"
#include "ObjectPrefabManager.h"
#include "Achievements.h"
#include "GlobalClipboard.h"
#include "[Common]\CLIWrapper.h"
#include "Hooks\Hooks-Renderer.h"

namespace cse
{
	namespace renderWindow
	{
		IRenderWindowSceneGraphModifier::RenderData::RenderData(NiNode* SceneGraph, NiNode* ExtraNode) :
			SceneGraph(SceneGraph),
			ExtraNode(ExtraNode),
			LoadedRefs()
		{
			TESRenderWindow::GetActiveCellObjects(LoadedRefs);
		}



		RenderWindowSceneGraphManager::RenderWindowSceneGraphManager() :
			Modifiers()
		{
			Modifiers.reserve(10);
		}

		RenderWindowSceneGraphManager::~RenderWindowSceneGraphManager()
		{
			for each (auto Itr in Modifiers)
				delete Itr;

			Modifiers.clear();
		}

		void RenderWindowSceneGraphManager::AddModifier(IRenderWindowSceneGraphModifier* Mod)
		{
			SME_ASSERT(Mod);

			if (std::find(Modifiers.begin(), Modifiers.end(), Mod) == Modifiers.end())
				Modifiers.push_back(Mod);
			else
				BGSEECONSOLE_MESSAGE("Attempting to re-add the same scenegraph modifier");
		}

		void RenderWindowSceneGraphManager::HandleRender(NiCamera* Camera, NiNode* SceneGraph, NiCullingProcess* CullingProc, BSRenderedTexture* RenderTarget)
		{
			NiNode* ExtraFittingsNode = TESRender::CreateNiNode();
			TESRender::AddToNiNode(SceneGraph, ExtraFittingsNode);
			IRenderWindowSceneGraphModifier::RenderData Data(SceneGraph, ExtraFittingsNode);

			for each (auto Itr in Modifiers)
				Itr->PreRender(Data);

			TESRender::UpdateAVObject(ExtraFittingsNode);
			cdeclCall<void>(0x00700240, Camera, SceneGraph, CullingProc, RenderTarget);

			for each (auto Itr in Modifiers)
				Itr->PostRender(Data);

			bool Freed = TESRender::RemoveFromNiNode(SceneGraph, ExtraFittingsNode);
			SME_ASSERT(Freed == true);
		}



		bool ReferenceParentChildIndicator::EnableParentIndicatorVisitor(TESObjectREFR* Ref)
		{
			ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
			if (xParent && xParent->parent)
				return true;
			else
				return false;
		}

		ReferenceParentChildIndicator::ReferenceParentChildIndicator()
		{
			VertexColor = (NiVertexColorProperty*)FormHeap_Allocate(sizeof(NiVertexColorProperty));
			thisCall<void>(0x00410C50, VertexColor);
			VertexColor->flags |= NiVertexColorProperty::kSrcMode_Emissive;

			Wireframe = (NiWireframeProperty*)FormHeap_Allocate(sizeof(NiWireframeProperty));
			thisCall<void>(0x00417BE0, Wireframe);
			Wireframe->m_bWireframe = 0;

			// increment ref counts so that they don't get released elsewhere
			VertexColor->m_uiRefCount = 1;
			Wireframe->m_uiRefCount = 1;
		}

		ReferenceParentChildIndicator::~ReferenceParentChildIndicator()
		{
			SME_ASSERT(Wireframe->m_uiRefCount == 1);
			TESRender::DeleteNiRefObject(Wireframe);

			SME_ASSERT(VertexColor->m_uiRefCount == 1);
			TESRender::DeleteNiRefObject(VertexColor);
		}

		void ReferenceParentChildIndicator::PreRender(RenderData& Data)
		{
			if (settings::renderer::kParentChildVisualIndicator().i &&
				*TESRenderWindow::PathGridEditFlag == 0 &&
				*TESRenderWindow::LandscapeEditFlag == 0)
			{
				CellObjectListT CurrentRefs;
				if (TESRenderWindow::GetActiveCellObjects(CurrentRefs, &EnableParentIndicatorVisitor))
				{
					std::vector<TESObjectREFR*> EnumeratedParents;
					for each (auto Itr in CurrentRefs)
					{
						NiNode* RefNode = Itr->GetNiNode();
						if (RefNode)
						{
							ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)Itr->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
							SME_ASSERT(xParent && xParent->parent);

							if (TESRenderWindow::GetCellInActiveGrid(xParent->parent->parentCell))
							{
								bool NewParentNode = false;
								if (std::find(EnumeratedParents.begin(), EnumeratedParents.end(), xParent->parent) == EnumeratedParents.end())
								{
									EnumeratedParents.push_back(xParent->parent);
									NewParentNode = true;
								}

								NiColorAlpha ColorConnector = { 0 }, ColorIndicator = { 0 };
								ColorConnector.r = ColorIndicator.r = 0.5f;
								ColorConnector.b = ColorIndicator.b = 0.7f;
								ColorConnector.g = ColorIndicator.g = 1.0f;

								if (xParent->oppositeState)
								{
									ColorConnector.r = ColorConnector.b = 1.0f;
									ColorConnector.g = 0.f;
								}

								Vector3 Point(xParent->parent->position);
								Point -= Itr->position;
								Vector3 Zero(0, 0, 0);

								NiLines* NodeLineConnector = cdeclCall<NiLines*>(0x004AD0C0, &Point, &ColorConnector, &Zero, &ColorConnector);
								SME_ASSERT(NodeLineConnector);
								NodeLineConnector->m_localTranslate.x = NodeLineConnector->m_worldTranslate.x = Itr->position.x;
								NodeLineConnector->m_localTranslate.y = NodeLineConnector->m_worldTranslate.y = Itr->position.y;
								NodeLineConnector->m_localTranslate.z = NodeLineConnector->m_worldTranslate.z = Itr->position.z;

								TESRender::AddProperty(NodeLineConnector, VertexColor);
								TESRender::AddToNiNode(Data.ExtraNode, NodeLineConnector);
								thisCall<void>(0x006F2C10, NodeLineConnector);

								if (NewParentNode)
								{
									NiTriShape* ParentIndicator = cdeclCall<NiTriShape*>(0x004AE0F0, 10.f, &ColorIndicator);
									SME_ASSERT(ParentIndicator);
									ParentIndicator->m_localTranslate.x = ParentIndicator->m_worldTranslate.x = xParent->parent->position.x;
									ParentIndicator->m_localTranslate.y = ParentIndicator->m_worldTranslate.y = xParent->parent->position.y;
									ParentIndicator->m_localTranslate.z = ParentIndicator->m_worldTranslate.z = xParent->parent->position.z;
									TESRender::AddProperty(ParentIndicator, Wireframe);
									TESRender::AddProperty(ParentIndicator, VertexColor);
									TESRender::AddToNiNode(Data.ExtraNode, ParentIndicator);
									thisCall<void>(0x006F2C10, ParentIndicator);
								}
							}
						}
					}
				}
			}
		}

		void ReferenceParentChildIndicator::PostRender(RenderData& Data)
		{
			;// nothing to do here as the extra node to which we add the geom gets released elsewhere
		}

		ReferenceVisibilityModifier::ReferenceVisibilityModifier() :
			CulledRefBuffer()
		{
			CulledRefBuffer.reserve(50);
		}

		ReferenceVisibilityModifier::~ReferenceVisibilityModifier()
		{
			SME_ASSERT(CulledRefBuffer.size() == 0);
			CulledRefBuffer.clear();
		}

		void ReferenceVisibilityModifier::PreRender(RenderData& Data)
		{
			for each (auto Itr in Data.LoadedRefs)
			{
				if (Itr->IsTemporary() == false)
				{
					NiNode* Node = Itr->GetNiNode();
					// skip culled nodes as their visibility is controlled by edit dialogs
					if (Node && Node->IsCulled() == false)
					{
						bool ShouldCull = false;
						if (TESRenderWindow::ShowInitiallyDisabledRefs == false && Itr->GetDisabled())
							ShouldCull = true;

						BSExtraData* xData = Itr->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
						if (xData)
						{
							ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
							if (xParent->parent->GetChildrenInvisible() ||
								(xParent->parent->GetDisabled() &&
								 TESRenderWindow::ShowInitiallyDisabledRefChildren == false))
							{
								ShouldCull = true;
							}
						}

						if (Itr->GetInvisible())
							ShouldCull = true;


						if (ShouldCull)
						{
							Node->SetCulled(true);
							CulledRefBuffer.push_back(Node);
						}
					}
				}

			}
		}

		void ReferenceVisibilityModifier::PostRender(RenderData& Data)
		{
			// reset the culled state
			for each (auto Itr in CulledRefBuffer)
				Itr->SetCulled(false);

			CulledRefBuffer.clear();
		}


		RenderWindowSelectionManager::RenderWindowSelectionManager(RenderWindowGroupManager* GroupMan) :
			ReferenceGroupManager(GroupMan)
		{
			SME_ASSERT(ReferenceGroupManager);
		}

		RenderWindowSelectionManager::~RenderWindowSelectionManager()
		{
			ReferenceGroupManager = NULL;
		}

		void RenderWindowSelectionManager::AddToSelection(TESObjectREFR* Ref, bool SelectionBox) const
		{
			Ref->ToggleSelectionBox(false);

			if (GetAsyncKeyState(VK_MENU))
			{
				// if the alt key is held down, fallback to regular handling
				_RENDERSEL->AddToSelection(Ref, SelectionBox);
			}
			else
			{
				if (Ref->GetFrozen() || (Ref->IsActive() == false && TESRenderWindow::FreezeInactiveRefs))
					;// ref's frozen, don't select
				else
				{
					// add the parent group to the selection, if any
					if (ReferenceGroupManager->SelectAffiliatedGroup(Ref, _RENDERSEL, false) == false)
						_RENDERSEL->AddToSelection(Ref, SelectionBox);

					// recheck the selection for frozen refs that may have been a part of the group
					std::vector<TESForm*> FrozenRefs;

					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Selection = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						SME_ASSERT(Selection);

						if (Selection->GetFrozen() || (Selection->IsActive() == false && TESRenderWindow::FreezeInactiveRefs))
							FrozenRefs.push_back(Itr->Data);
					}

					for (std::vector<TESForm*>::const_iterator Itr = FrozenRefs.begin(); Itr != FrozenRefs.end(); Itr++)
						_RENDERSEL->RemoveFromSelection(*Itr, true);
				}
			}
		}

		bool RenderWindowSelectionManager::IsSelectable(TESObjectREFR* Ref) const
		{
			if (GetAsyncKeyState(VK_MENU))		// alt key is down
				return true;
			else if (Ref->GetFrozen())
				return false;
			else if (Ref->IsActive() && TESRenderWindow::FreezeInactiveRefs)
				return false;
			else
				return true;
		}

		RenderWindowFlyCameraOperator::RenderWindowFlyCameraOperator(HWND ParentWindow, bgsee::ResourceTemplateT TemplateID) :
			bgsee::RenderWindowFlyCameraOperator(ParentWindow, TemplateID)
		{
			ZeroMemory(&ViewportFrustumBuffer, sizeof(NiFrustum));
		}

		RenderWindowFlyCameraOperator::~RenderWindowFlyCameraOperator()
		{
			;//
		}

		void RenderWindowFlyCameraOperator::PrologCallback(void)
		{
			_RENDERSEL->ClearSelection(true);
			hooks::_MemHdlr(CellViewSetCurrentCell).WriteUInt8(0xC3);		// write an immediate retn

			float CameraFOV = settings::renderWindowFlyCamera::kCameraFOV.GetData().f;
			if (CameraFOV > 120.0f)
				CameraFOV = 120.0f;
			else if (CameraFOV < 50.0f)
				CameraFOV = 50.0f;

			memcpy(&ViewportFrustumBuffer, &_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, sizeof(NiFrustum));
			TESRender::SetCameraFOV(_PRIMARYRENDERER->primaryCamera, CameraFOV);

			RefreshRenderWindow();
		}

		void RenderWindowFlyCameraOperator::EpilogCallback(void)
		{
			hooks::_MemHdlr(CellViewSetCurrentCell).WriteBuffer();			// write original instruction

			memcpy(&_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, &ViewportFrustumBuffer, sizeof(NiFrustum));
			TESRender::UpdateAVObject(_PRIMARYRENDERER->primaryCamera);

			RefreshRenderWindow();
		}

		void RenderWindowFlyCameraOperator::Rotate(int XOffset, int YOffset)
		{
			static Vector3 RotationPivot((float)3.4028235e38, (float)3.4028235e38, (float)3.4028235e38);

			if (XOffset || YOffset)
			{
				float RotationSpeed = settings::renderWindowFlyCamera::kRotationSpeed.GetData().f;

				TESRender::RotateNode(_PRIMARYRENDERER->primaryCameraParentNode,
									  &RotationPivot,
									  XOffset,
									  YOffset,
									  RotationSpeed);

				RefreshRenderWindow();
			}
		}

		void RenderWindowFlyCameraOperator::Move(UInt8 Direction, bool Sprinting, bool Crawling)
		{
			float Velocity = settings::renderWindowFlyCamera::kMovementSpeed.GetData().f;

			if (Sprinting)
				Velocity *= settings::renderWindowFlyCamera::kSprintMultiplier.GetData().f;

			if (Crawling)
				Velocity *= settings::renderWindowFlyCamera::kCrawlMultiplier.GetData().f;

			switch (Direction)
			{
			case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Forward:
			case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Backward:
				{
					if (Direction == bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Backward)
						Velocity *= -1;

					TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk04, Velocity);
				}

				break;
			case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Left:
			case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Right:
				{
					if (Direction == bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Right)
						Velocity *= -1;

					TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk02, Velocity);
				}

				break;
			case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Up:
			case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Down:
				{
					if (Direction == bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Down)
						Velocity *= -1;

					TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk00, Velocity);
				}

				break;
			}

			RefreshRenderWindow();
		}

		void RenderWindowFlyCameraOperator::RefreshRenderWindow(void)
		{
			TESRenderWindow::Redraw();
		}




		RenderWindowManager				RenderWindowManager::Instance;


		RenderWindowManager::DialogExtraData::DialogExtraData() :
			bgsee::WindowExtraData(kTypeID)
		{
			TunnelingKeyMessage = false;
		}

		RenderWindowManager::DialogExtraData::~DialogExtraData()
		{
			;//
		}

		RenderWindowManager::GlobalEventSink::GlobalEventSink(RenderWindowManager* Parent) :
			Parent(Parent)
		{
			SME_ASSERT(Parent);
		}

		void RenderWindowManager::GlobalEventSink::Handle(SME::MiscGunk::IEventData* Data)
		{
			const events::TypedEventSource* Source = dynamic_cast<const events::TypedEventSource*>(Data->Source);
			SME_ASSERT(Source);

			switch (Source->GetTypeID())
			{
			case events::TypedEventSource::kType_Renderer_Release:
				Parent->HandleD3DRelease();
				break;
			case events::TypedEventSource::kType_Renderer_Renew:
				Parent->HandleD3DRenew();
				break;
			case events::TypedEventSource::kType_Renderer_PreMainSceneGraphRender:
				{
					events::renderer::PreSceneGraphRenderData* Args = dynamic_cast<events::renderer::PreSceneGraphRenderData*>(Data);
					SME_ASSERT(Args);

					Parent->HandleSceneGraphRender(Args->Camera, Args->SceneGraph, Args->CullingProc, Args->RenderTarget);
					break;
				}
			case events::TypedEventSource::kType_Renderer_PostMainSceneGraphRender:
				Parent->HandlePostSceneGraphRender();
				break;
			}
		}

		LRESULT CALLBACK RenderWindowManager::RenderWindowMenuInitSelectSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																bool& Return, bgsee::WindowExtraDataCollection* ExtraData)
		{
			LRESULT DlgProcResult = TRUE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITMENUPOPUP:
				{
					if (HIWORD(lParam) == FALSE)
					{
						HMENU Popup = (HMENU)wParam;

						for (int i = 0, j = GetMenuItemCount(Popup); i < j; i++)
						{
							MENUITEMINFO CurrentItem = { 0 };
							CurrentItem.cbSize = sizeof(MENUITEMINFO);
							CurrentItem.fMask = MIIM_ID | MIIM_STATE;

							if (GetMenuItemInfo(Popup, i, TRUE, &CurrentItem) == TRUE)
							{
								bool UpdateItem = true;
								bool CheckItem = false;
								bool DisableItem = false;
								char NewItemText[0x200] = { 0 };

								switch (CurrentItem.wID)
								{
								case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
									if (TESRenderWindow::FreezeInactiveRefs)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_COPLANARDROP:
									if (settings::renderer::kCoplanarRefDrops.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_SWITCHCNY:
									if (settings::renderer::kSwitchCAndY.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_STATICCAMERAPIVOT:
									if (settings::renderer::kFixedCameraPivot.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_MOUSEREFENABLED:
									if (settings::renderWindowPainter::kShowMouseRef.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_MOUSEREFCTRLMODIFIED:
									if (settings::renderWindowPainter::kMouseRefCtrlModified.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_USEALTERNATEMOVEMENTSETTINGS:
									if (TESRenderWindow::UseAlternateMovementSettings)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_SAVEEXTERIORSNAPSHOT:
									FORMAT_STR(NewItemText, "Save Current Exterior Cell Snapshot");

									if (*TESRenderWindow::ActiveCell == NULL || _TES->currentInteriorCell)
										DisableItem = true;
									else
									{
										FORMAT_STR(NewItemText, "Save Exterior Cell %i,%i Snapshot",
											(*TESRenderWindow::ActiveCell)->cellData.coords->x,
												   (*TESRenderWindow::ActiveCell)->cellData.coords->y);
									}

									break;
								case IDC_RENDERWINDOWCONTEXT_OFFSETDUPLICATEDREFSINTHEZAXIS:
									if (settings::renderer::kZOffsetDuplicatedRefs().i)
										CheckItem = true;

									break;
								default:
									UpdateItem = false;
									break;
								}

								if (UpdateItem)
								{
									if (CheckItem)
									{
										CurrentItem.fState &= ~MFS_UNCHECKED;
										CurrentItem.fState |= MFS_CHECKED;
									}
									else
									{
										CurrentItem.fState &= ~MFS_CHECKED;
										CurrentItem.fState |= MFS_UNCHECKED;
									}

									if (DisableItem)
									{
										CurrentItem.fState &= ~MFS_ENABLED;
										CurrentItem.fState |= MFS_DISABLED;
									}
									else
									{
										CurrentItem.fState &= ~MFS_DISABLED;
										CurrentItem.fState |= MFS_ENABLED;
									}

									CurrentItem.fMask = MIIM_STATE;

									if (NewItemText[0] != 0)
									{
										CurrentItem.fMask |= MIIM_STRING;
										CurrentItem.fType |= MFT_STRING;
										CurrentItem.dwTypeData = NewItemText;
										CurrentItem.cch = strlen(NewItemText);
									}

									SetMenuItemInfo(Popup, i, TRUE, &CurrentItem);
								}
							}
						}
					}

					Return = true;
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_RENDERWINDOWCONTEXT_USEALTERNATEMOVEMENTSETTINGS:
					{
						TESRenderWindow::UseAlternateMovementSettings = (TESRenderWindow::UseAlternateMovementSettings == false);

						if (TESRenderWindow::UseAlternateMovementSettings == false)
							NotificationOSDLayer::ShowNotification("Using vanilla movement settings");
						else
							NotificationOSDLayer::ShowNotification("Using alternate movement settings");

						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_AlternateRenderWindowMovement);
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_STATICCAMERAPIVOT:
					{
						settings::renderer::kFixedCameraPivot.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_UNLOADCURRENTCELLS:
					TESRenderWindow::Reset();

					break;
				case IDC_RENDERWINDOWCONTEXT_SWITCHCNY:
					{
						settings::renderer::kSwitchCAndY.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OFFSETDUPLICATEDREFSINTHEZAXIS:
					{
						settings::renderer::kZOffsetDuplicatedRefs.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
					TESRenderWindow::FreezeInactiveRefs = (TESRenderWindow::FreezeInactiveRefs == false);

					if (TESRenderWindow::FreezeInactiveRefs)
						NotificationOSDLayer::ShowNotification("Inactive references frozen");
					else
						NotificationOSDLayer::ShowNotification("Inactive references thawed");

					break;
				case IDC_RENDERWINDOWCONTEXT_INVERTSELECTION:
					if (*TESRenderWindow::PathGridEditFlag == 0)
					{
						CellObjectListT Refs;
						if (TESRenderWindow::GetActiveCellObjects(Refs))
						{
							TESRenderSelection* Buffer = TESRenderSelection::CreateInstance(_RENDERSEL);
							_RENDERSEL->ClearSelection(true);

							for (CellObjectListT::iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
							{
								TESObjectREFR* Ref = *Itr;

								if (Buffer->HasObject(Ref) == false)
									_RENDERSEL->AddToSelection(Ref, true);
							}

							Buffer->DeleteInstance();
							TESRenderWindow::Redraw();
						}
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_BATCHREFERENCEEDITOR:
					{
						CellObjectListT Refs;
						UInt32 RefCount = TESRenderWindow::GetActiveCellObjects(Refs);

						if (RefCount > 1)
						{
							int i = 0;
							componentDLLInterface::CellObjectData* RefData = new componentDLLInterface::CellObjectData[RefCount];
							componentDLLInterface::BatchRefData* BatchData = new componentDLLInterface::BatchRefData();

							for (CellObjectListT::iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr, ++i)
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

										if (BatchData->Flags.UsePersistent() &&
											ThisRef->baseForm->formType != TESForm::kFormType_NPC &&
											ThisRef->baseForm->formType != TESForm::kFormType_Creature)
										{
											ThisRef->SetQuestItem(BatchData->Flags.Persistent);
											Modified = true;
										}

										if (BatchData->Flags.UseDisabled())
											SME::MiscGunk::ToggleFlag(&ThisRef->formFlags,
																	  TESForm::kFormFlags_Disabled,
																	  BatchData->Flags.Disabled), Modified = true;

										if (BatchData->Flags.UseVWD())
											SME::MiscGunk::ToggleFlag(&ThisRef->formFlags,
																	  TESForm::kFormFlags_VisibleWhenDistant,
																	  BatchData->Flags.VWD), Modified = true;

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
											ThisRef->extraData.ModExtraGlobal(NULL);
											ThisRef->extraData.ModExtraRank(-1);
											ThisRef->extraData.ModExtraOwnership(NULL);

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

						break;
					}
				case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
				case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
					{
						CellObjectListT Refs;
						TESRenderWindow::GetActiveCellObjects(Refs);

						for (CellObjectListT::iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
						{
							TESObjectREFR* Ref = *Itr;

							switch (LOWORD(wParam))
							{
							case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
								if (Ref->GetInvisible())
									Ref->ToggleInvisiblity();

								if (Ref->GetChildrenInvisible())
									Ref->ToggleChildrenInvisibility();

								break;
							case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
								Ref->SetFrozenState(false);

								break;
							}
						}

						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
							NotificationOSDLayer::ShowNotification("Reset visibility flags on the active cell's references");

							break;
						case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
							NotificationOSDLayer::ShowNotification("Thawed all of the active cell's references");

							break;
						}

						TESRenderWindow::Redraw();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
				case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
				case IDC_RENDERWINDOWCONTEXT_FREEZE:
				case IDC_RENDERWINDOWCONTEXT_THAW:
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);

						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
							Ref->ToggleInvisiblity();
							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefVisibility);

							break;
						case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
							Ref->ToggleChildrenInvisibility();
							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefVisibility);

							break;
						case IDC_RENDERWINDOWCONTEXT_FREEZE:
							Ref->SetFrozenState(true);
							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefFreezing);

							break;
						case IDC_RENDERWINDOWCONTEXT_THAW:
							Ref->SetFrozenState(false);
							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefFreezing);

							break;
						}
					}

					TESRenderWindow::Redraw();
					Return = true;

					break;
				case IDC_RENDERWINDOWCONTEXT_GROUP:
				case IDC_RENDERWINDOWCONTEXT_UNGROUP:
					if (_RENDERSEL->selectionCount > 1)
					{
						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_GROUP:
							if (RenderWindowManager::Instance.GetReferenceGroupManager()->AddGroup(_RENDERSEL) == false)
							{
								BGSEEUI->MsgBoxW(hWnd, 0,
												 "Couldn't add current selection to a new group.\n\nCheck the console for more details.");
							}
							else
								NotificationOSDLayer::ShowNotification("Created new selection group");

							break;
						case IDC_RENDERWINDOWCONTEXT_UNGROUP:
							if (RenderWindowManager::Instance.GetReferenceGroupManager()->RemoveGroup(_RENDERSEL) == false)
							{
								BGSEEUI->MsgBoxW(hWnd, 0,
												 "Couldn't dissolve the current selection's group.\n\nCheck the console for more details.");
							}
							else
								NotificationOSDLayer::ShowNotification("Removed selection group");

							break;
						}

						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefGrouping);
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_ALIGNTOX:
				case IDC_RENDERWINDOWCONTEXT_ALIGNTOY:
				case IDC_RENDERWINDOWCONTEXT_ALIGNTOZ:
					if (_RENDERSEL->selectionCount > 1)
					{
						// record the op twice, otherwise the thingy will crash on undo for some reason
						_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_Unk03, _RENDERSEL->selectionList);
						_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_Unk03, _RENDERSEL->selectionList);

						TESObjectREFR* AlignRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList->Next; Itr && Itr->Data; Itr = Itr->Next)
						{
							TESObjectREFR* ThisRef = CS_CAST(Itr->Data, TESForm, TESObjectREFR);

							switch (LOWORD(wParam))
							{
							case IDC_RENDERWINDOWCONTEXT_ALIGNTOX:
								ThisRef->position.x = AlignRef->position.x;
								break;
							case IDC_RENDERWINDOWCONTEXT_ALIGNTOY:
								ThisRef->position.y = AlignRef->position.y;
								break;
							case IDC_RENDERWINDOWCONTEXT_ALIGNTOZ:
								ThisRef->position.z = AlignRef->position.z;
								break;
							}

							ThisRef->SetFromActiveFile(true);
							ThisRef->UpdateNiNode();
						}

						NotificationOSDLayer::ShowNotification("Selection aligned to %08X", AlignRef->formID);
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefAlignment);
						TESRenderWindow::Redraw();

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_COPLANARDROP:
					{
						settings::renderer::kCoplanarRefDrops.ToggleData();

						if (settings::renderer::kCoplanarRefDrops.GetData().i)
							NotificationOSDLayer::ShowNotification("Enabled co-planar dropping");
						else
							NotificationOSDLayer::ShowNotification("Disabled co-planar dropping");

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_COPYTOGLOBALCLIPBOARD:
					{
						globalClipboard::GlobalCopyBuffer Buffer;

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
							Buffer.Add(Itr->Data);

						Buffer.Copy();
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_GlobalClipboard);
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_PASTEFROMGLOBALCLIPBOARD:
					{
						BGSEECLIPBOARD->Paste();
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_GlobalClipboard);
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_MOUSEREFENABLED:
					{
						settings::renderWindowPainter::kShowMouseRef.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_MOUSEREFCTRLMODIFIED:
					{
						settings::renderWindowPainter::kMouseRefCtrlModified.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_SAVEEXTERIORSNAPSHOT:
					{
						SME_ASSERT(*TESRenderWindow::ActiveCell && _TES->currentInteriorCell == NULL);

						TESLODTextureGenerator::SaveExteriorSnapshot(*TESRenderWindow::ActiveCell,
																	 settings::renderer::kExteriorSnapshotResolution.GetData().i,
																	 NULL);
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}


		LRESULT CALLBACK RenderWindowManager::RenderWindowMasterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													  bool& Return, bgsee::WindowExtraDataCollection* ExtraData)
		{
			LRESULT DlgProcResult = TRUE;
			Return = false;

			UInt8* YKeyState = (UInt8*)0x00A0BC1E;
			float* UnkRotFactor = (float*)0x00A0BAC4;

			static Vector3 kCameraStaticPivot;

			if (bgsee::RenderWindowFlyCamera::IsActive() && uMsg != WM_DESTROY)
			{
				// do nothing if the fly camera is active
				return DlgProcResult;
			}
			else if (_RENDERWIN_MGR.OSD->NeedsInput() && uMsg != WM_DESTROY)
			{
				// defer to the OSD window proc
				return DlgProcResult;
			}

			switch (uMsg)
			{
			case WM_RENDERWINDOW_UPDATEFOV:
				{
					Return = true;

					float CameraFOV = settings::renderer::kCameraFOV.GetData().f;
					if (CameraFOV > 120.0f)
						CameraFOV = 120.0f;
					else if (CameraFOV < 50.0f)
						CameraFOV = 50.0f;

					TESRender::SetCameraFOV(_PRIMARYRENDERER->primaryCamera, CameraFOV);
					memcpy(&TESRenderWindow::CameraFrustumBuffer, &_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, sizeof(NiFrustum));
				}

				break;
			case WM_INITDIALOG:
				{
					DialogExtraData* xData = BGSEE_GETWINDOWXDATA(DialogExtraData, ExtraData);
					if (xData == NULL)
					{
						xData = new DialogExtraData();
						ExtraData->Add(xData);
					}

				}

				break;
			case WM_DESTROY:
				{
					DialogExtraData* xData = BGSEE_GETWINDOWXDATA(DialogExtraData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(DialogExtraData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_RENDERWINDOW_GETCAMERASTATICPIVOT:
				{
					SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (LONG_PTR)&kCameraStaticPivot);
					DlgProcResult = TRUE;
					Return = true;
				}

				break;
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_RenderWindow, NULL);
				Return = true;

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case TESRenderWindow::kTimer_ViewportUpdate:			// update timer
					static bool SetTimerPeriod = true;
					if (SetTimerPeriod)
					{
						SetTimerPeriod = false;
						UInt32 Period = settings::renderer::kUpdatePeriod.GetData().i;
						if (Period == 0 || Period >= 100)
							Period = 10;

						SetTimer(hWnd, TESRenderWindow::kTimer_ViewportUpdate, Period, NULL);
					}

					if (TESLODTextureGenerator::GeneratorState != TESLODTextureGenerator::kLODDiffuseMapGeneratorState_NotInUse)
					{
						// prevent the OS from triggering the screen-saver/switching to standby mode
						SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
					}


					// the primary camera's view frustum gets updated ever so often, resetting its horizontal FOV
					// we update it here in case it has changed
					if (_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.l != TESRenderWindow::CameraFrustumBuffer.l ||
						_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.r != TESRenderWindow::CameraFrustumBuffer.r ||
						_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.b != TESRenderWindow::CameraFrustumBuffer.b ||
						_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.t != TESRenderWindow::CameraFrustumBuffer.t)
					{
						SendMessage(hWnd, WM_RENDERWINDOW_UPDATEFOV, NULL, NULL);
					}

					break;
				}

				break;
			case WM_SIZING:
				{
					if (TESPreviewControl::ActivePreviewControls->Count())
					{
						BGSEEUI->MsgBoxW(hWnd, 0, "Please close any dialogs with preview controls before attempting to resize the render window.");
						Return = true;
					}
				}

				break;
			case WM_LBUTTONDBLCLK:
				{
					if (*TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0)
					{
						TESObjectREFR* Ref = TESRender::PickRefAtCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
						if (Ref)
						{
							if (_RENDERWIN_MGR.GetSelectionManager()->IsSelectable(Ref) == false)
							{
								// preempt the vanilla handler
								Return = true;
							}
						}
					}
				}

				break;
			case WM_MOUSEMOVE:
				{
					TESRenderWindow::CurrentMouseCoord.x = GET_X_LPARAM(lParam);
					TESRenderWindow::CurrentMouseCoord.y = GET_Y_LPARAM(lParam);

					TESObjectREFR* LastMouseRef = TESRenderWindow::CurrentMouseRef;
					TESRenderWindow::CurrentMouseRef = NULL;

					TESPathGridPoint* LastPathGridPoint = TESRenderWindow::CurrentMousePathGridPoint;
					TESRenderWindow::CurrentMousePathGridPoint = NULL;

					if (GetActiveWindow() == hWnd && GetCapture() != hWnd && *TESRenderWindow::LandscapeEditFlag == 0)
					{
						int Enabled = settings::renderWindowPainter::kShowMouseRef.GetData().i;
						int ControlModified = settings::renderWindowPainter::kMouseRefCtrlModified.GetData().i;

						if (Enabled && (ControlModified == false || GetAsyncKeyState(VK_CONTROL)))
						{
							if (*TESRenderWindow::PathGridEditFlag == 0)
							{
								TESRenderWindow::CurrentMouseRef = TESRender::PickRefAtCoords(TESRenderWindow::CurrentMouseCoord.x,
																							  TESRenderWindow::CurrentMouseCoord.y);

								if (_RENDERSEL->selectionCount == 1 && _RENDERSEL->selectionList->Data == TESRenderWindow::CurrentMouseRef)
									TESRenderWindow::CurrentMouseRef = NULL;

								if (TESRenderWindow::CurrentMouseRef ||
									(LastMouseRef && TESRenderWindow::CurrentMouseRef == NULL))
								{
									TESRenderWindow::Redraw();
								}
							}
							else
							{
								TESRenderWindow::CurrentMousePathGridPoint = TESRender::PickPathGridPointAtCoords(TESRenderWindow::CurrentMouseCoord.x,
																												  TESRenderWindow::CurrentMouseCoord.y);

								if (TESRenderWindow::CurrentMousePathGridPoint ||
									(LastPathGridPoint && TESRenderWindow::CurrentMousePathGridPoint == NULL))
								{
									TESRenderWindow::Redraw();
								}
							}
						}
					}

					if (GetCapture() != hWnd)
					{
						HCURSOR Icon = *TESRenderWindow::CursorArrow;

						if (*TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0)
						{
							TESObjectREFR* MouseRef = TESRender::PickRefAtCoords(TESRenderWindow::CurrentMouseCoord.x, TESRenderWindow::CurrentMouseCoord.y);
							if (MouseRef)
							{
								if (MouseRef->GetFrozen() || (MouseRef->IsActive() == false && TESRenderWindow::FreezeInactiveRefs))
									Icon = LoadCursor(NULL, IDC_NO);
								else if (_RENDERSEL->HasObject(MouseRef))
									Icon = *TESRenderWindow::CursorMove;
								else
									Icon = *TESRenderWindow::CursorSelect;
							}
						}

						HCURSOR CurrentCursor = GetCursor();
						if (Icon && CurrentCursor != Icon)
							SetCursor(Icon);
					}
				}

				break;
			case WM_MOUSELEAVE:
			case WM_NCMOUSELEAVE:
				TESRenderWindow::CurrentMouseRef = NULL;
				TESRenderWindow::Redraw();

				break;
			case WM_LBUTTONDOWN:
				TESRenderWindow::CurrentMouseLBDragCoordDelta.x = GET_X_LPARAM(lParam);
				TESRenderWindow::CurrentMouseLBDragCoordDelta.y = GET_Y_LPARAM(lParam);

				if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL))
				{
					// handle it for the button up event
					Return = true;
				}

				break;
			case WM_LBUTTONUP:
				TESRenderWindow::CurrentMouseLBDragCoordDelta.x -= GET_X_LPARAM(lParam);
				TESRenderWindow::CurrentMouseLBDragCoordDelta.y -= GET_Y_LPARAM(lParam);

				TESRenderWindow::CurrentMouseLBDragCoordDelta.x = abs(TESRenderWindow::CurrentMouseLBDragCoordDelta.x);
				TESRenderWindow::CurrentMouseLBDragCoordDelta.y = abs(TESRenderWindow::CurrentMouseLBDragCoordDelta.y);

				if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL))
				{
					// place palette object, if any
					Return = true;
					objectPalette::ObjectPaletteManager::Instance.PlaceObject(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				}

				break;
			case WM_KEYUP:
				switch (wParam)
				{
				case 0x43:		// C
					{
						int SwitchEnabled = settings::renderer::kSwitchCAndY.GetData().i;
						if (SwitchEnabled)
						{
							if (*YKeyState)
								*UnkRotFactor = 0.0;

							*YKeyState = 0;

							Return = true;
						}
					}

					break;
				case 0x59:		// Y
					{
						int SwitchEnabled = settings::renderer::kSwitchCAndY.GetData().i;
						if (SwitchEnabled)
							Return = true;
					}

					break;
				}

				break;
			case WM_KEYDOWN:
				switch (wParam)
				{
				case VK_SHIFT:
					_PRIMARYRENDERER->GetCameraPivot(&kCameraStaticPivot, 0.18);

					break;
				case VK_F1:		// F1
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						TESDialog::ShowUseReportDialog(Ref);
					}

					Return = true;

					break;
				case 0x5A:		// Z
					if (*TESRenderWindow::PathGridEditFlag && GetAsyncKeyState(VK_CONTROL))
					{
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_PathGridAdditions);
						RenderWindowManager::Instance.GetPathGridUndoManager()->PerformUndo();
						Return = true;
					}

					break;
				case 0x59:		// Y
					if (*TESRenderWindow::PathGridEditFlag && GetAsyncKeyState(VK_CONTROL))
					{
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_PathGridAdditions);
						RenderWindowManager::Instance.GetPathGridUndoManager()->PerformRedo();
						Return = true;
					}
					else
					{
						int SwitchEnabled = settings::renderer::kSwitchCAndY.GetData().i;
						DialogExtraData* xData = BGSEE_GETWINDOWXDATA(DialogExtraData, ExtraData);
						SME_ASSERT(xData);

						if (SwitchEnabled)
						{
							xData->TunnelingKeyMessage = true;
							SendMessage(hWnd, WM_KEYDOWN, 0x43, lParam);
							xData->TunnelingKeyMessage = false;

							Return = true;
						}
					}

					break;
				case 0x43:		// C
					{
						int SwitchEnabled = settings::renderer::kSwitchCAndY.GetData().i;
						DialogExtraData* xData = BGSEE_GETWINDOWXDATA(DialogExtraData, ExtraData);
						SME_ASSERT(xData);

						if (SwitchEnabled && xData->TunnelingKeyMessage == false)
						{
							if (*YKeyState == 0)
								*UnkRotFactor = 0.0;

							*YKeyState = 1;

							Return = true;
						}
					}

					break;
				case 0x52:		// R
					if (*TESRenderWindow::PathGridEditFlag)
					{
						if (GetAsyncKeyState(VK_CONTROL))
						{
							RenderWindowManager::Instance.GetPathGridUndoManager()->ResetRedoStack();

							if (TESRenderWindow::SelectedPathGridPoints->Count())
								RenderWindowManager::Instance.GetPathGridUndoManager()->RecordOperation(PathGridUndoManager::kOperation_DataChange, TESRenderWindow::SelectedPathGridPoints);

							for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
							{
								TESPathGridPoint* Point = Itr.Get();
								Point->UnlinkFromReference();
								Point->HideSelectionRing();
								achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_PathGridAdditions);
							}

							TESRenderWindow::Redraw(true);

							Return = true;
						}
						else
						{
							std::list<TESPathGridPoint*> Delinquents;

							for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
							{
								if (Itr.Get()->linkedRef)
									Delinquents.push_back(Itr.Get());

								Itr.Get()->HideSelectionRing();
							}

							// remove from selection
							for (std::list<TESPathGridPoint*>::iterator Itr = Delinquents.begin(); Itr != Delinquents.end(); Itr++)
								thisCall<void>(0x00452AE0, TESRenderWindow::SelectedPathGridPoints, *Itr);
						}
					}

					break;
				case 0x51:		// Q
					if (GetAsyncKeyState(VK_CONTROL))
					{
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_USEALTERNATEMOVEMENTSETTINGS, NULL);

						Return = true;
					}
					else if (GetAsyncKeyState(VK_SHIFT) && AUXVIEWPORT->GetVisible())
					{
						if (AUXVIEWPORT->ToggleFrozenState())
							NotificationOSDLayer::ShowNotification("Froze auxiliary viewport camera");
						else
							NotificationOSDLayer::ShowNotification("Released auxiliary viewport camera");

						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_AuxViewPort);
						Return = true;
					}

					break;
				case 0x45:		// E
					if (GetAsyncKeyState(VK_SHIFT))
					{
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_COPLANARDROP, NULL);

						Return = true;
					}

					break;
				case 0x56:		// V
					if (GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_MENU))
					{
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_INVERTSELECTION, NULL);

						Return = true;
					}

					break;
				case 0x32:		// 2
					SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY, NULL);
					TESRenderWindow::Redraw();

					Return = true;
					break;
				case 0x33:		// 3
				case 0x34:		// 4
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						if (wParam == 0x33)
						{
							float Alpha = settings::renderer::kRefToggleOpacityAlpha().f;
							if (Alpha < 0.1)
								Alpha = 0.1;

							Ref->SetAlpha(Alpha);
						}
						else
							Ref->SetAlpha();
					}

					TESRenderWindow::Redraw();

					Return = true;
					break;
				case 0x47:		// G
					SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, TESCSMain::kToolbar_PathGridEdit, NULL);

					Return = true;
					break;
				case VK_F5:
					{
						SendMessage(hWnd, WM_RENDERWINDOW_UPDATEFOV, NULL, NULL);
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL, NULL);
					}

					break;
				case VK_OEM_3:	// ~
					{
						if (TESLODTextureGenerator::GeneratorState != TESLODTextureGenerator::kLODDiffuseMapGeneratorState_NotInUse)
							break;
						else if (GetCapture())
							break;

						TESRenderWindow::CurrentMouseRef = NULL;

						bgsee::RenderWindowFlyCamera* xFreeCamData = BGSEE_GETWINDOWXDATA(bgsee::RenderWindowFlyCamera, ExtraData);
						SME_ASSERT(xFreeCamData == NULL);

						xFreeCamData = new bgsee::RenderWindowFlyCamera(new RenderWindowFlyCameraOperator(hWnd,
																										  TESDialog::kDialogTemplate_RenderWindow));
						ExtraData->Add(xFreeCamData);

						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_FlyCamera);
						Return = true;
					}

					break;
				case 0x41:		// A
					if (GetAsyncKeyState(VK_CONTROL))
					{
						Return = true;

						CellObjectListT Refs;
						TESRenderWindow::GetActiveCellObjects(Refs);

						for (CellObjectListT::iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
						{
							TESObjectREFR* Ref = *Itr;

							if (_RENDERSEL->HasObject(Ref) == false &&
								(Ref->formFlags & TESObjectREFR::kSpecialFlags_3DInvisible) == false &&
								(Ref->formFlags & TESObjectREFR::kSpecialFlags_Frozen) == false)
							{
								_RENDERSEL->AddToSelection(Ref, true);
							}
						}
					}

					break;
				case 0x4B:		// K
					Return = true;

					if (*TESRenderWindow::PathGridEditFlag == 0)
						settings::renderer::kParentChildVisualIndicator.ToggleData();
					else
						settings::renderer::kPathGridLinkedRefIndicator.ToggleData();

					TESRenderWindow::Redraw(*TESRenderWindow::PathGridEditFlag);

					break;
				case 0x46:		// F
					if (*TESRenderWindow::PathGridEditFlag)
					{
						// prevent the linked ref indicator trishape from interfering with the fall operation
						Return = true;
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
						BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, wParam, lParam);

						// reset culled state
						for each (auto Itr in Delinquents)
							Itr->SetCulled(false);
					}

					break;
				case VK_DELETE:
					// clear the picked objects just in case they are about to be deleted
					TESRenderWindow::CurrentMouseRef = NULL;
					TESRenderWindow::CurrentMousePathGridPoint = NULL;

					break;
				}

				break;
			}

			return DlgProcResult;
		}



		RenderWindowManager::RenderWindowManager()
		{
			SceneGraphManager = NULL;
			SelectionManager = NULL;
			PGUndoManager = NULL;
			GroupManager = NULL;
			OSD = NULL;
			EventSink = new GlobalEventSink(this);

			Initialized = false;
		}


		RenderWindowManager::~RenderWindowManager()
		{
			SAFEDELETE(SceneGraphManager);
			SAFEDELETE(SelectionManager);
			SAFEDELETE(PGUndoManager);
			SAFEDELETE(GroupManager);
			SAFEDELETE(OSD);
			SAFEDELETE(EventSink);

			Initialized = false;
		}

		bool RenderWindowManager::Initialize()
		{
			SME_ASSERT(Initialized == false);

			SceneGraphManager = new RenderWindowSceneGraphManager();
			PGUndoManager = new PathGridUndoManager();
			GroupManager = new RenderWindowGroupManager();
			SelectionManager = new RenderWindowSelectionManager(GroupManager);
			OSD = new RenderWindowOSD();

			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RenderWindow, RenderWindowMenuInitSelectSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RenderWindow, RenderWindowMasterSubclassProc);
			BGSEEUI->GetMenuHotSwapper()->RegisterTemplateReplacer(IDR_RENDERWINDOWCONTEXT, BGSEEMAIN->GetExtenderHandle());

			events::renderer::kRelease.AddSink(EventSink);
			events::renderer::kRenew.AddSink(EventSink);
			events::renderer::kPreSceneGraphRender.AddSink(EventSink);
			events::renderer::kPostSceneGraphRender.AddSink(EventSink);

			Initialized = true;

			return Initialized;
		}

		bool RenderWindowManager::InitializeOSD()
		{
			SME_ASSERT(Initialized);
			bool OSDReady = OSD->Initialize();
			return OSDReady;
		}

		RenderWindowGroupManager* RenderWindowManager::GetReferenceGroupManager() const
		{
			SME_ASSERT(Initialized);
			return GroupManager;
		}

		RenderWindowSceneGraphManager* RenderWindowManager::GetSceneGraphManager() const
		{
			SME_ASSERT(Initialized);
			return SceneGraphManager;
		}

		PathGridUndoManager* RenderWindowManager::GetPathGridUndoManager() const
		{
			SME_ASSERT(Initialized);
			return PGUndoManager;
		}

		RenderWindowSelectionManager* RenderWindowManager::GetSelectionManager() const
		{
			SME_ASSERT(Initialized);
			return SelectionManager;
		}

		RenderWindowOSD* RenderWindowManager::GetOSD() const
		{
			SME_ASSERT(Initialized);
			return OSD;
		}

		void RenderWindowManager::HandleD3DRelease()
		{
			SME_ASSERT(Initialized);
			BGSEERWPAINTER->HandleReset(true, false);
			OSD->HandleD3DRelease();
		}

		void RenderWindowManager::HandleD3DRenew()
		{
			SME_ASSERT(Initialized);
			BGSEERWPAINTER->HandleReset(false, true);
			OSD->HandleD3DRenew();
		}

		void RenderWindowManager::HandleSceneGraphRender(NiCamera* Camera, NiNode* SceneGraph, NiCullingProcess* CullingProc, BSRenderedTexture* RenderTarget)
		{
			SME_ASSERT(Initialized);
			SceneGraphManager->HandleRender(Camera, SceneGraph, CullingProc, RenderTarget);
		}

		void RenderWindowManager::HandlePostSceneGraphRender()
		{
			SME_ASSERT(Initialized);
			OSD->Render();
			BGSEERWPAINTER->Render();
		}


		void Initialize()
		{
			bool ComponentInitialized = _RENDERWIN_MGR.InitializeOSD();
			SME_ASSERT(ComponentInitialized);

			_RENDERWIN_MGR.GetSceneGraphManager()->AddModifier(new ReferenceParentChildIndicator());
			_RENDERWIN_MGR.GetSceneGraphManager()->AddModifier(new ReferenceVisibilityModifier());

			SendMessage(*TESRenderWindow::WindowHandle, WM_RENDERWINDOW_UPDATEFOV, NULL, NULL);
		}

		void Deinitialize(void)
		{
			;//
		}



	}
}