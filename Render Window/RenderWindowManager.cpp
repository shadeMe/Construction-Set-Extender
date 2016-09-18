#include "Render Window\RenderWindowManager.h"
#include "Construction Set Extender_Resource.h"
#include "AuxiliaryViewport.h"
#include "ObjectPaletteManager.h"
#include "ObjectPrefabManager.h"
#include "Achievements.h"
#include "GlobalClipboard.h"
#include "[Common]\CLIWrapper.h"
#include "Hooks\Hooks-Renderer.h"
#include "RenderWindowActions.h"

namespace cse
{
	namespace renderWindow
	{
		IRenderWindowSceneGraphModifier::RenderData::RenderData(NiNode* SceneGraph, NiNode* ExtraNode) :
			SceneGraph(SceneGraph),
			ExtraNode(ExtraNode),
			LoadedRefs(_RENDERWIN_MGR.GetActiveRefs())
		{
			;//
		}



		RenderWindowSceneGraphManager::RenderWindowSceneGraphManager() :
			Modifiers()
		{
			Modifiers.reserve(10);
		}

		RenderWindowSceneGraphManager::~RenderWindowSceneGraphManager()
		{
			DEBUG_ASSERT(Modifiers.size() == 0);
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

		void RenderWindowSceneGraphManager::RemoveModifier(IRenderWindowSceneGraphModifier * Mod)
		{
			SME_ASSERT(Mod);

			ModifierArrayT::iterator Match = std::find(Modifiers.begin(), Modifiers.end(), Mod);
			if (Match != Modifiers.end())
				Modifiers.erase(Match);
		}

		void RenderWindowSceneGraphManager::HandleRender(NiCamera* Camera, NiNode* SceneGraph, NiCullingProcess* CullingProc, BSRenderedTexture* RenderTarget)
		{
			NiNode* ExtraFittingsNode = TESRender::CreateNiNode();
			TESRender::AddToNiNode(SceneGraph, ExtraFittingsNode);
			IRenderWindowSceneGraphModifier::RenderData Data(SceneGraph, ExtraFittingsNode);

			for (auto Itr : Modifiers)
				Itr->PreRender(Data);

			TESRender::UpdateAVObject(ExtraFittingsNode);
			cdeclCall<void>(0x00700240, Camera, SceneGraph, CullingProc, RenderTarget);

			for (auto Itr : Modifiers)
				Itr->PostRender(Data);

			bool Freed = TESRender::RemoveFromNiNode(SceneGraph, ExtraFittingsNode);
			SME_ASSERT(Freed == true);
		}


		ReferenceParentChildIndicator		ReferenceParentChildIndicator::Instance;

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
			VertexColor = (NiVertexColorProperty*)TESRender::CreateProperty(NiVertexColorProperty::kType);
			VertexColor->flags |= NiVertexColorProperty::kSrcMode_Emissive;

			Wireframe = (NiWireframeProperty*)TESRender::CreateProperty(NiWireframeProperty::kType);
			Wireframe->m_bWireframe = 1;
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
				TESObjectREFRArrayT CurrentRefs;
				if (TESRenderWindow::GetActiveCellObjects(CurrentRefs, &EnableParentIndicatorVisitor))
				{
					TESObjectREFRArrayT EnumeratedParents;
					for (auto Itr : CurrentRefs)
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
								TESRender::UpdateDynamicEffectState(NodeLineConnector);

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
									TESRender::UpdateDynamicEffectState(ParentIndicator);
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

		ReferenceVisibilityModifier			ReferenceVisibilityModifier::Instance;

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
			for (auto Itr : Data.LoadedRefs)
			{
				UInt32 Reason = 0;
				if (Itr->IsTemporary() == false && ReferenceVisibilityValidator::ShouldBeInvisible(Itr, Reason))
				{
					if (ReferenceVisibilityValidator::IsCulled(Itr) == false)
					{
						NiNode* Node = Itr->GetNiNode();
						Node->SetCulled(true);
						CulledRefBuffer.push_back(Node);
					}
				}
			}
		}

		void ReferenceVisibilityModifier::PostRender(RenderData& Data)
		{
			// reset the culled state
			for (auto Itr : CulledRefBuffer)
				Itr->SetCulled(false);

			CulledRefBuffer.clear();
		}

		DebugSceneGraphModifier			DebugSceneGraphModifier::Instance;

		DebugSceneGraphModifier::DebugSceneGraphModifier()
		{
			MatProp = (NiMaterialProperty*)TESRender::CreateProperty(NiMaterialProperty::kType);
			MatProp->m_diff.r = 0;
			MatProp->m_diff.b = 1;
			MatProp->m_diff.g = 0;
			MatProp->m_fAlpha = 0.3;

			Stencil = (NiStencilProperty*)TESRender::CreateProperty(NiStencilProperty::kType);
			Stencil->flags = 0;
			Stencil->flags |= NiStencilProperty::kTestMode_Always;
		}

		DebugSceneGraphModifier::~DebugSceneGraphModifier()
		{
			TESRender::DeleteNiRefObject(MatProp);
			TESRender::DeleteNiRefObject(Stencil);
		}

		void DebugSceneGraphModifier::PreRender(RenderData& Data)
		{
		}

		void DebugSceneGraphModifier::PostRender(RenderData& Data)
		{
		}

		bool ReferenceVisibilityValidator::ShouldBeInvisible(TESObjectREFR* Ref)
		{
			UInt32 Throwaway = 0;
			return ShouldBeInvisible(Ref, Throwaway);
		}

		bool ReferenceVisibilityValidator::ShouldBeInvisible(TESObjectREFR* Ref, UInt32& OutReasonFlags)
		{
			OutReasonFlags = 0;

			if (_RENDERWIN_XSTATE.ShowInitiallyDisabledRefs == false && Ref->GetDisabled())
				OutReasonFlags |= kReason_InitiallyDisabledSelf;

			if (Ref->GetInvisible())
				OutReasonFlags |= kReason_InvisibleSelf;

			BSExtraData* xData = Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
			if (xData)
			{
				ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
				if (xParent->parent->GetChildrenInvisible())
					OutReasonFlags |= kReason_InvisibleChild;

				if (xParent->parent->GetDisabled() && _RENDERWIN_XSTATE.ShowInitiallyDisabledRefChildren == false)
					OutReasonFlags |= kReason_InitiallyDisabledChild;
			}

			if (OutReasonFlags)
				return true;
			else
				return false;
		}

		bool ReferenceVisibilityValidator::IsCulled(TESObjectREFR* Ref)
		{
			NiNode* Node = Ref->GetNiNode();
			if (Node == nullptr || Node->IsCulled())
				return true;
			else
				return false;
		}

		RenderWindowSelectionManager::RenderWindowSelectionManager(RenderWindowGroupManager* GroupMan) :
			ReferenceGroupManager(GroupMan)
		{
			SME_ASSERT(ReferenceGroupManager);
		}

		RenderWindowSelectionManager::~RenderWindowSelectionManager()
		{
			ReferenceGroupManager = nullptr;
		}

		void RenderWindowSelectionManager::AddToSelection(TESObjectREFR* Ref, bool AddSelectionBox, bool PaintingSelection) const
		{
			Ref->ToggleSelectionBox(false);

			if ((*TESRenderWindow::RubberBandSelector)->dragging && GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_CONTROL))
			{
				// it's a drag deselect op, remove the ref from the current selection
				if (_RENDERSEL->HasObject(Ref))
					_RENDERSEL->RemoveFromSelection(Ref, false);

				return;
			}

			bool RegularHandling = false;
			UInt32 Throwaway = 0;
			if (IsSelectable(Ref, PaintingSelection, RegularHandling, Throwaway) || RegularHandling)
			{
				if (RegularHandling || PaintingSelection)
					_RENDERSEL->AddToSelection(Ref, AddSelectionBox);
				else
				{
					// add the parent group to the selection, if any
					if (ReferenceGroupManager->SelectAffiliatedGroup(Ref, _RENDERSEL, false) == false)
						_RENDERSEL->AddToSelection(Ref, AddSelectionBox);

					// recheck the selection for frozen refs that may have been a part of the group
					TESFormArrayT FrozenRefs;

					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Selection = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						SME_ASSERT(Selection);

						if (Selection->IsFrozen() || (Selection->IsActive() == false && _RENDERWIN_XSTATE.FreezeInactiveRefs))
							FrozenRefs.push_back(Itr->Data);
					}

					for (TESFormArrayT::const_iterator Itr = FrozenRefs.begin(); Itr != FrozenRefs.end(); Itr++)
						_RENDERSEL->RemoveFromSelection(*Itr, true);
				}
			}
		}

		void RenderWindowSelectionManager::RemoveFromSelection(TESObjectREFR* Ref, bool RemoveSelectionBox) const
		{
			// nothing to do here
			_RENDERSEL->RemoveFromSelection(Ref, RemoveSelectionBox);
		}

		bool RenderWindowSelectionManager::IsSelectable(TESObjectREFR* Ref, bool PaintingSelection, bool& OutRegularHandling, UInt32& OutReasonFlags) const
		{
			OutReasonFlags = 0;
			OutRegularHandling = false;

			bool Result = true;
			if (GetAsyncKeyState(VK_MENU) && PaintingSelection == false)
			{
				// alt key is down
				OutRegularHandling = true;
				OutReasonFlags |= kReason_Override;
			}

			if (ReferenceVisibilityValidator::ShouldBeInvisible(Ref) || ReferenceVisibilityValidator::IsCulled(Ref))
			{
				OutReasonFlags |= kReason_InvalidVisibility;
				Result = false;
			}

			if (Ref->IsFrozen())
			{
				OutReasonFlags |= kReason_FrozenSelf;
				Result = false;
			}

			if (Ref->IsActive() == false && _RENDERWIN_XSTATE.FreezeInactiveRefs)
			{
				OutReasonFlags |= kReason_FrozenInactive;
				Result = false;
			}

			return Result;
		}

		bool RenderWindowSelectionManager::IsSelectable(TESObjectREFR* Ref, bool PaintingSelection) const
		{
			bool Throwaway = false;
			UInt32 ThrowawayEx = 0;
			return IsSelectable(Ref, PaintingSelection, Throwaway, ThrowawayEx);
		}

		bool RenderWindowSelectionManager::IsSelectable(TESObjectREFR* Ref, UInt32& OutReasonFlags, bool PaintingSelection) const
		{
			bool Throwaway = false;
			return IsSelectable(Ref, PaintingSelection, Throwaway, OutReasonFlags);
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

		const float RenderWindowExtendedState::MaxLandscapeEditBrushRadius = 25.f;

		RenderWindowExtendedState::RenderWindowExtendedState() :
			Initialized(false)
		{
			UseAlternateMovementSettings = false;
			FreezeInactiveRefs = false;
			CameraFrustumBuffer = { 0 };
			CurrentMouseRef = nullptr;
			CurrentMousePathGridPoint = nullptr;
			ShowInitiallyDisabledRefs = true;
			ShowInitiallyDisabledRefChildren = true;
			UseGrassTextureOverlay = false;
			GrassOverlayTexture = nullptr;
			StaticCameraPivot.Scale(0);
			DraggingPathGridPoints = false;
		}

		RenderWindowExtendedState::~RenderWindowExtendedState()
		{
			DEBUG_ASSERT(Initialized == false);
		}

		void RenderWindowExtendedState::Initialize()
		{
			SME_ASSERT(Initialized == false);

			char Buffer[0x100] = { 0 };
			FORMAT_STR(Buffer, "Textures\\Landscape\\%s", settings::renderer::kGrassOverlayTexturePath().s);
			if (_FILEFINDER->FindFile(Buffer) != FileFinder::kFileStatus_NotFound)
			{
				GrassOverlayTexture = TESRender::CreateSourceTexture(Buffer);
				GrassOverlayTexture->m_uiRefCount = 1;
			}

			Initialized = true;
		}

		void RenderWindowExtendedState::Deinitialize()
		{
			SME_ASSERT(Initialized);

			if (GrassOverlayTexture)
			{
				UseGrassTextureOverlay = false;
				_TES->ReloadLandscapeTextures();
				TESRender::DeleteNiRefObject(GrassOverlayTexture);
				GrassOverlayTexture = nullptr;
			}

			Initialized = false;
		}

		RenderWindowManager				RenderWindowManager::Instance;


		const Vector3& RenderWindowExtendedState::UpdateStaticCameraPivot()
		{
			_PRIMARYRENDERER->GetCameraPivot(&StaticCameraPivot, settings::renderer::kFixedCameraPivotMul().f);
			return StaticCameraPivot;
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
			case events::TypedEventSource::kType_Plugin_ClearData:
				Parent->HandleClearData();
				break;
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

					Parent->HandlePreSceneGraphRender(Args->Camera, Args->SceneGraph, Args->CullingProc, Args->RenderTarget);
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
									if (_RENDERWIN_XSTATE.FreezeInactiveRefs)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_COPLANARDROP:
									if (settings::renderer::kCoplanarRefDrops.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_OSD_MOUSEREF:
									if (settings::renderWindowOSD::kShowMouseRef.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_OSD_MOUSEREF_CTRLMODIFIED:
									if (settings::renderWindowOSD::kMouseRefCtrlModified.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_SAVEEXTERIORSNAPSHOT:
									FORMAT_STR(NewItemText, "Save Current Exterior Cell Snapshot");

									if (*TESRenderWindow::ActiveCell == nullptr || _TES->currentInteriorCell)
										DisableItem = true;
									else
									{
										FORMAT_STR(NewItemText, "Save Exterior Cell %i,%i Snapshot",
											(*TESRenderWindow::ActiveCell)->cellData.coords->x, (*TESRenderWindow::ActiveCell)->cellData.coords->y);
									}

									break;
								case IDC_RENDERWINDOWCONTEXT_OFFSETDUPLICATEDREFSINTHEZAXIS:
									if (settings::renderer::kZOffsetDuplicatedRefs().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_OSD_INFOOVERLAY:
									if (settings::renderWindowOSD::kShowInfoOverlay().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_OSD_CELLLISTS:
									if (settings::renderWindowOSD::kShowCellLists().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_OSD_SELECTIONCONTROLS:
									if (settings::renderWindowOSD::kShowSelectionControls().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_OSD_TOOLBARS:
									if (settings::renderWindowOSD::kShowToolbar().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_OSD_ACTIVEREFERENCECOLLECTIONS:
									if (settings::renderWindowOSD::kShowActiveRefCollections().i)
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
				case IDC_RENDERWINDOWCONTEXT_UNLOADCURRENTCELLS:
					TESRenderWindow::Reset();

					break;
				case IDC_RENDERWINDOWCONTEXT_OFFSETDUPLICATEDREFSINTHEZAXIS:
					{
						settings::renderer::kZOffsetDuplicatedRefs.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
					{
						actions::ToggleFreezeInactive();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_INVERTSELECTION:
					{
						actions::InvertSelection();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_BATCHREFERENCEEDITOR:
					{
						actions::ShowBatchEditor();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
					{
						actions::ThawAll();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
					{
						actions::RevealAll();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
					{
						actions::ToggleSelectionVisibility();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
					{
						actions::ToggleSelectionChildrenVisibility();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_FREEZE:
					{
						actions::FreezeSelection();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_THAW:
					{
						actions::ThawSelection();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_GROUP:
					{
						actions::GroupSelection();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_UNGROUP:
					{
						actions::UngroupSelection();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_COPLANARDROP:
					{
						settings::renderer::kCoplanarRefDrops.ToggleData();

						if (settings::renderer::kCoplanarRefDrops.GetData().i)
							NotificationOSDLayer::Instance.ShowNotification("Enabled co-planar dropping");
						else
							NotificationOSDLayer::Instance.ShowNotification("Disabled co-planar dropping");

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
				case IDC_RENDERWINDOWCONTEXT_OSD_MOUSEREF:
					{
						settings::renderWindowOSD::kShowMouseRef.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_MOUSEREF_CTRLMODIFIED:
					{
						settings::renderWindowOSD::kMouseRefCtrlModified.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_SAVEEXTERIORSNAPSHOT:
					{
						SME_ASSERT(*TESRenderWindow::ActiveCell && _TES->currentInteriorCell == nullptr);

						TESLODTextureGenerator::SaveExteriorSnapshot(*TESRenderWindow::ActiveCell,
																	 settings::renderer::kExteriorSnapshotResolution.GetData().i,
																	 nullptr);
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_CELLLISTS:
					{
						settings::renderWindowOSD::kShowCellLists.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_INFOOVERLAY:
					{
						settings::renderWindowOSD::kShowInfoOverlay.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_SELECTIONCONTROLS:
					{
						settings::renderWindowOSD::kShowSelectionControls.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_TOOLBARS:
					{
						settings::renderWindowOSD::kShowToolbar.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_ACTIVEREFERENCECOLLECTIONS:
					{
						settings::renderWindowOSD::kShowActiveRefCollections.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_RENDERWINDOWHOTKEYS:
					Return = true;

					if (*TESRenderWindow::ActiveCell)
						Instance.KeyboardInputManager->ShowHotKeyEditor();

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
			if (Instance.Initialized == false)
			{
				// bugger off
				return DlgProcResult;
			}
			if (bgsee::RenderWindowFlyCamera::IsActive() && uMsg != WM_DESTROY)
			{
				// do nothing if the fly camera is active
				return DlgProcResult;
			}
			else if (Instance.OSD->NeedsInput(uMsg) && uMsg != WM_DESTROY)
			{
				// defer to the OSD window proc
				return DlgProcResult;
			}
			else if (Instance.KeyboardInputManager->HandleInput(hWnd, uMsg, wParam, lParam, &Instance))
			{
				// key input was handled, consume message
				Return = true;
				return DlgProcResult;
			}
			else if (Instance.MouseInputManager->HandleInput(hWnd, uMsg, wParam, lParam, &Instance))
			{
				// mouse input was handled, consume message
				Return = true;
				return DlgProcResult;
			}

			switch (uMsg)
			{
			case WM_QUIT:
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_RenderWindow, NULL);
				Return = true;

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case TESRenderWindow::kTimer_ViewportUpdate:
					static bool SetTimerPeriod = true;
					if (SetTimerPeriod)
					{
						SetTimerPeriod = false;
						UInt32 Period = 1;

						SetTimer(hWnd, TESRenderWindow::kTimer_ViewportUpdate, Period, nullptr);
					}

					if (TESLODTextureGenerator::GeneratorState != TESLODTextureGenerator::kState_NotInUse)
					{
						// prevent the OS from triggering the screen-saver/switching to standby mode
						SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
					}


					// the primary camera's view frustum gets updated ever so often, resetting its horizontal FOV
					// we update it here in case it has changed
					if (_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.l != _RENDERWIN_XSTATE.CameraFrustumBuffer.l ||
						_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.r != _RENDERWIN_XSTATE.CameraFrustumBuffer.r ||
						_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.b != _RENDERWIN_XSTATE.CameraFrustumBuffer.b ||
						_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.t != _RENDERWIN_XSTATE.CameraFrustumBuffer.t)
					{
						Instance.RefreshFOV();
					}


					// render the aux viewport's perspective
					if (*TESRenderWindow::RefreshFlag && *TESRenderWindow::ActiveCell &&
						TESLODTextureGenerator::GeneratorState == TESLODTextureGenerator::kState_NotInUse)
					{
						AUXVIEWPORT->Draw(nullptr, nullptr);
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
			case WM_KEYDOWN:
				switch (wParam)
				{
				case VK_OEM_3:	// ~
					{
						// the fly camera key binding is hardcoded because I'm too lazy to refactor the relevant code
						if (TESLODTextureGenerator::GeneratorState != TESLODTextureGenerator::kState_NotInUse)
							break;
						else if (GetCapture())
							break;

						_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;

						bgsee::RenderWindowFlyCamera* xFreeCamData = BGSEE_GETWINDOWXDATA(bgsee::RenderWindowFlyCamera, ExtraData);
						SME_ASSERT(xFreeCamData == nullptr);

						xFreeCamData = new bgsee::RenderWindowFlyCamera(new RenderWindowFlyCameraOperator(hWnd,
																										  TESDialog::kDialogTemplate_RenderWindow));
						ExtraData->Add(xFreeCamData);

						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_FlyCamera);
						Return = true;
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}



		RenderWindowManager::RenderWindowManager() :
			ActiveRefCache()
		{
			ExtendedState = new RenderWindowExtendedState();
			SceneGraphManager = new RenderWindowSceneGraphManager();
			PGUndoManager = new PathGridUndoManager();
			GroupManager = new RenderWindowGroupManager();
			SelectionManager = new RenderWindowSelectionManager(GroupManager);
			OSD = new RenderWindowOSD();
			CellLists = new RenderWindowCellLists();
			EventSink = new GlobalEventSink(this);
			KeyboardInputManager = new input::RenderWindowKeyboardManager();
			MouseInputManager = new input::RenderWindowMouseManager();
			ActiveRefCache.reserve(200);

			Initialized = false;
		}


		RenderWindowManager::~RenderWindowManager()
		{
			SAFEDELETE(ExtendedState);
			SAFEDELETE(SceneGraphManager);
			SAFEDELETE(SelectionManager);
			SAFEDELETE(PGUndoManager);
			SAFEDELETE(GroupManager);
			SAFEDELETE(OSD);
			SAFEDELETE(EventSink);
			SAFEDELETE(CellLists);
			SAFEDELETE(KeyboardInputManager);
			SAFEDELETE(MouseInputManager);

			Initialized = false;
		}

		bool RenderWindowManager::Initialize()
		{
			SME_ASSERT(Initialized == false);

			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RenderWindow, RenderWindowMenuInitSelectSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RenderWindow, RenderWindowMasterSubclassProc);
			BGSEEUI->GetMenuHotSwapper()->RegisterTemplateReplacer(IDR_RENDERWINDOWCONTEXT, BGSEEMAIN->GetExtenderHandle());
			if (settings::dialogs::kShowMainWindowsInTaskbar.GetData().i)
			{
				bgsee::WindowStyler::StyleData RegularAppWindow = { 0 };
				RegularAppWindow.Extended = WS_EX_APPWINDOW;
				RegularAppWindow.ExtendedOp = bgsee::WindowStyler::StyleData::kOperation_OR;

				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_RenderWindow, RegularAppWindow);
			}

			events::renderer::kRelease.AddSink(EventSink);
			events::renderer::kRenew.AddSink(EventSink);
			events::renderer::kPreSceneGraphRender.AddSink(EventSink);
			events::renderer::kPostSceneGraphRender.AddSink(EventSink);

			ExtendedState->Initialize();
			SceneGraphManager->AddModifier(&ReferenceParentChildIndicator::Instance);
			SceneGraphManager->AddModifier(&ReferenceVisibilityModifier::Instance);
#ifndef NDEBUG
			SceneGraphManager->AddModifier(&DebugSceneGraphModifier::Instance);
#endif
			CellLists->Initialize();
			GroupManager->Initialize();
			KeyboardInputManager->Initialize();

			Initialized = true;

			return Initialized;
		}

		bool RenderWindowManager::InitializeOSD()
		{
			SME_ASSERT(Initialized);
			bool OSDReady = OSD->Initialize();
			return OSDReady;
		}

		void RenderWindowManager::Deinitialize()
		{
			SME_ASSERT(Initialized);

			events::renderer::kRelease.RemoveSink(EventSink);
			events::renderer::kRenew.RemoveSink(EventSink);
			events::renderer::kPreSceneGraphRender.RemoveSink(EventSink);
			events::renderer::kPostSceneGraphRender.RemoveSink(EventSink);

			KeyboardInputManager->Deinitialize();
			GroupManager->Deinitialize();
			CellLists->Deinitialize();
			OSD->Deinitialize();
			SceneGraphManager->RemoveModifier(&ReferenceParentChildIndicator::Instance);
			SceneGraphManager->RemoveModifier(&ReferenceVisibilityModifier::Instance);
#ifndef NDEBUG
			SceneGraphManager->RemoveModifier(&DebugSceneGraphModifier::Instance);
#endif
			ExtendedState->Deinitialize();
			Initialized = false;
		}

		RenderWindowGroupManager* RenderWindowManager::GetGroupManager() const
		{
			SME_ASSERT(Initialized);
			return GroupManager;
		}

		PathGridUndoManager* RenderWindowManager::GetPathGridUndoManager() const
		{
			SME_ASSERT(Initialized);
			return PGUndoManager;
		}

		RenderWindowSelectionManager * RenderWindowManager::GetSelectionManager() const
		{
			SME_ASSERT(Initialized);
			return SelectionManager;
		}

		RenderWindowExtendedState& RenderWindowManager::GetState() const
		{
			SME_ASSERT(Initialized);
			return *ExtendedState;
		}

		input::RenderWindowKeyboardManager* RenderWindowManager::GetKeyboardInputManager() const
		{
			SME_ASSERT(Initialized);
			return KeyboardInputManager;
		}

		input::RenderWindowMouseManager* RenderWindowManager::GetMouseInputManager() const
		{
			SME_ASSERT(Initialized);
			return MouseInputManager;
		}

		RenderWindowOSD* RenderWindowManager::GetOSD() const
		{
			SME_ASSERT(Initialized);
			return OSD;
		}

		void RenderWindowManager::RefreshFOV()
		{
			float CameraFOV = settings::renderer::kCameraFOV.GetData().f;
			if (CameraFOV > 120.0f)
				CameraFOV = 120.0f;
			else if (CameraFOV < 50.0f)
				CameraFOV = 50.0f;

			TESRender::SetCameraFOV(_PRIMARYRENDERER->primaryCamera, CameraFOV);
			memcpy(&_RENDERWIN_XSTATE.CameraFrustumBuffer, &_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, sizeof(NiFrustum));

			// update the aux viewport cam's frustum as well
			AUXVIEWPORT->SetCameraFOV(CameraFOV);

			TESRenderWindow::Redraw();
		}

		void RenderWindowManager::CacheActiveRefs()
		{
			TESRenderWindow::GetActiveCellObjects(ActiveRefCache, [](TESObjectREFR* Ref)->bool {
				if (Ref->IsDeleted())
					return false;
				else
					return true;
			});
		}

		const TESObjectREFRArrayT& RenderWindowManager::GetActiveRefs() const
		{
			return ActiveRefCache;
		}

		void RenderWindowManager::HandleClearData()
		{
			ActiveRefCache.clear();
		}

		void RenderWindowManager::HandleD3DRelease()
		{
			SME_ASSERT(Initialized);
			OSD->HandleD3DRelease();
		}

		void RenderWindowManager::HandleD3DRenew()
		{
			SME_ASSERT(Initialized);
			OSD->HandleD3DRenew();
		}

		void RenderWindowManager::HandlePreSceneGraphRender(NiCamera* Camera, NiNode* SceneGraph, NiCullingProcess* CullingProc, BSRenderedTexture* RenderTarget)
		{
			SME_ASSERT(Initialized);

			CacheActiveRefs();
			SceneGraphManager->HandleRender(Camera, SceneGraph, CullingProc, RenderTarget);
		}

		void RenderWindowManager::HandlePostSceneGraphRender()
		{
			SME_ASSERT(Initialized);

			// no OSD for the aux viewport
			if (AUXVIEWPORT->IsRenderingPerspective() == false)
			{
				OSD->Draw();
				OSD->Render();
			}
		}


		void Initialize()
		{
			bool ComponentInitialized = _RENDERWIN_MGR.InitializeOSD();
			SME_ASSERT(ComponentInitialized);

			_RENDERWIN_MGR.RefreshFOV();
		}

		void Deinitialize(void)
		{
			_RENDERWIN_MGR.Deinitialize();
		}
	}
}