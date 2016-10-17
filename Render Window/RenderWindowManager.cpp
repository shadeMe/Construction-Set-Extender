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
#include "IconFontCppHeaders\IconsMaterialDesign.h"
#include "ToolbarOSDLayer.h"

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
				if (Itr->IsTemporary() == false && ReferenceVisibilityManager::ShouldBeInvisible(Itr, Reason))
				{
					if (ReferenceVisibilityManager::IsCulled(Itr) == false)
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

			SelectionMask = TESRender::CreateTexturingProperty("Textures\\Effects\\TerrainNoise.dds");
			thisCall<void>(0x00410B50, SelectionMask, 0);
		}

		DebugSceneGraphModifier::~DebugSceneGraphModifier()
		{
			TESRender::DeleteNiRefObject(MatProp);
			TESRender::DeleteNiRefObject(Stencil);
			TESRender::DeleteNiRefObject(SelectionMask);
		}

		void DebugSceneGraphModifier::PreRender(RenderData& Data)
		{
			return;

			for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
				NiNode* Node = Ref->GetNiNode();
				bool HasProp = false;

				for (NiTListBase<NiProperty>::Node* i = Node->m_propertyList.start; i && i->data; i = i->next)
				{
					if (i->data->GetPropertyType() == NiTexturingProperty::kType)
					{
						HasProp = true;
						break;
					}
				}

				if (HasProp == false)
				{
					TESRender::AddProperty(Node, SelectionMask);
					TESRender::UpdateDynamicEffectState(Node);
					TESRender::UpdateAVObject(Node);
					BGSEECONSOLE_MESSAGE("added sel mask to %08X", Ref->formID);
				}
			}
		}

		void DebugSceneGraphModifier::PostRender(RenderData& Data)
		{
			return;

			for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
			}
		}

		bool ReferenceVisibilityManager::ShouldBeInvisible(TESObjectREFR* Ref)
		{
			UInt32 Throwaway = 0;
			return ShouldBeInvisible(Ref, Throwaway);
		}

		bool ReferenceVisibilityManager::ShouldBeInvisible(TESObjectREFR* Ref, UInt32& OutReasonFlags)
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

			if (_RENDERWIN_MGR.GetLayerManager()->IsParentLayerVisible(Ref) == false)
				OutReasonFlags |= kReason_ParentLayerInvisible;

			if (OutReasonFlags)
				return true;
			else
				return false;
		}

		bool ReferenceVisibilityManager::IsCulled(TESObjectREFR* Ref)
		{
			NiNode* Node = Ref->GetNiNode();
			if (Node == nullptr || Node->IsCulled())
				return true;
			else
				return false;
		}

		void ReferenceSelectionManager::AddToSelection(TESObjectREFR* Ref, bool AddSelectionBox, bool PaintingSelection)
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
					if (_RENDERWIN_MGR.GetGroupManager()->SelectAffiliatedGroup(Ref, _RENDERSEL, false) == false)
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

		void ReferenceSelectionManager::RemoveFromSelection(TESObjectREFR* Ref, bool RemoveSelectionBox)
		{
			// nothing to do here
			_RENDERSEL->RemoveFromSelection(Ref, RemoveSelectionBox);
		}

		bool ReferenceSelectionManager::IsSelectable(TESObjectREFR* Ref, bool PaintingSelection, bool& OutRegularHandling, UInt32& OutReasonFlags)
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

			if (ReferenceVisibilityManager::ShouldBeInvisible(Ref) || ReferenceVisibilityManager::IsCulled(Ref))
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

			if (_RENDERWIN_MGR.GetLayerManager()->IsParentLayerFrozen(Ref))
			{
				OutReasonFlags |= kReason_ParentLayerFrozen;
				Result = false;
			}

			return Result;
		}

		bool ReferenceSelectionManager::IsSelectable(TESObjectREFR* Ref, bool PaintingSelection)
		{
			bool Throwaway = false;
			UInt32 ThrowawayEx = 0;
			return IsSelectable(Ref, PaintingSelection, Throwaway, ThrowawayEx);
		}

		bool ReferenceSelectionManager::IsSelectable(TESObjectREFR* Ref, UInt32& OutReasonFlags, bool PaintingSelection)
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

		void RenderWindowDeferredExecutor::HandlePostRenderWindowUpdate()
		{
			for (auto& Itr : Handlers)
				Itr();

			Handlers.clear();
		}

		void RenderWindowDeferredExecutor::QueueTask(DelegateT& Delegate)
		{
			Handlers.push_back(Delegate);
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
			case events::TypedEventSource::kType_Renderer_PostRenderWindowUpdate:
				Parent->HandlePostRenderWindowUpdate();
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
								case IDC_RENDERWINDOWCONTEXT_OSD_SELECTIONCONTROLS:
									if (settings::renderWindowOSD::kShowSelectionControls().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_OSD_TOOLBARS:
									if (settings::renderWindowOSD::kShowToolbar().i)
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
			else if (Instance.OSD->HandleMessage(hWnd, uMsg, wParam, lParam))
			{
				// OSD handled the message, consume
				Return = true;
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

					if (Instance.IsRenderingScene())
					{
						// consume the message as the previous render call isn't done yet
						Return = true;
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
			OSD = new RenderWindowOSD();
			CellLists = new RenderWindowCellLists();
			LayerManager = new RenderWindowLayerManager();
			EventSink = new GlobalEventSink(this);
			KeyboardInputManager = new input::RenderWindowKeyboardManager();
			MouseInputManager = new input::RenderWindowMouseManager();
			DeferredExecutor = new RenderWindowDeferredExecutor();
			ActiveRefCache.reserve(500);
			RenderingScene = false;

			Initialized = false;
		}


		RenderWindowManager::~RenderWindowManager()
		{
			SAFEDELETE(ExtendedState);
			SAFEDELETE(SceneGraphManager);
			SAFEDELETE(PGUndoManager);
			SAFEDELETE(GroupManager);
			SAFEDELETE(OSD);
			SAFEDELETE(EventSink);
			SAFEDELETE(CellLists);
			SAFEDELETE(LayerManager);
			SAFEDELETE(KeyboardInputManager);
			SAFEDELETE(MouseInputManager);
			SAFEDELETE(DeferredExecutor);

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
			events::renderer::kPostRenderWindowUpdate.AddSink(EventSink);

			ExtendedState->Initialize();
			SceneGraphManager->AddModifier(&ReferenceParentChildIndicator::Instance);
			SceneGraphManager->AddModifier(&ReferenceVisibilityModifier::Instance);
#ifndef NDEBUG
			SceneGraphManager->AddModifier(&DebugSceneGraphModifier::Instance);
#endif
			CellLists->Initialize();
			GroupManager->Initialize();
			LayerManager->Initialize();
			KeyboardInputManager->Initialize();
			MouseInputManager->Initialize(KeyboardInputManager->GetSharedBindings());

			// register active ref collection popups
			ToolbarOSDLayer::Instance.RegisterTopToolbarButton("activerefcol_invisible_interface",
															   std::bind(&RenderWindowManager::RenderActiveRefCollectionButton,
																		 this,
																		 kActiveRefCollection_Invisible),
															   std::bind(&RenderWindowManager::RenderActiveRefCollectionPopup,
																		 this,
																		 kActiveRefCollection_Invisible));
			ToolbarOSDLayer::Instance.RegisterTopToolbarButton("activerefcol_frozen_interface",
															   std::bind(&RenderWindowManager::RenderActiveRefCollectionButton,
																		 this,
																		 kActiveRefCollection_Frozen),
															   std::bind(&RenderWindowManager::RenderActiveRefCollectionPopup,
																		 this,
																		 kActiveRefCollection_Frozen));
			ToolbarOSDLayer::Instance.RegisterTopToolbarButton("activerefcol_groups_interface",
															   std::bind(&RenderWindowManager::RenderActiveRefCollectionButton,
																		 this,
																		 kActiveRefCollection_Groups),
															   std::bind(&RenderWindowManager::RenderActiveRefCollectionPopup,
																		 this,
																		 kActiveRefCollection_Groups));

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
			events::renderer::kPostRenderWindowUpdate.RemoveSink(EventSink);

			MouseInputManager->Deinitialize();
			KeyboardInputManager->Deinitialize();
			LayerManager->Deinitialize();
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

		RenderWindowLayerManager* RenderWindowManager::GetLayerManager() const
		{
			SME_ASSERT(Initialized);
			return LayerManager;
		}

		RenderWindowDeferredExecutor* RenderWindowManager::GetDeferredExecutor() const
		{
			SME_ASSERT(Initialized);
			return DeferredExecutor;
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

		bool RenderWindowManager::IsRenderingScene() const
		{
			return RenderingScene;
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

		void RenderWindowManager::RenderActiveRefCollectionRefTable(UInt8 Type, ImGuiTextFilter& FilterHelper)
		{
			const TESObjectREFRArrayT& ActiveRefs = ActiveRefCache;
			std::vector<std::string> ActiveGroups;
			ActiveGroups.reserve(0x20);

			char FilterBuffer[0x200] = { 0 };
			char Label[0x100] = { 0 };
			bool HasContents = false;

			for (auto Itr : ActiveRefs)
			{
				std::string EditorID(IRenderWindowOSDLayer::Helpers::GetRefEditorID(Itr));
				UInt32 FormID = Itr->formID;
				const char* FormType = TESForm::GetFormTypeIDLongName(Itr->baseForm->formType);

				switch (Type)
				{
				case kActiveRefCollection_Invisible:
				case kActiveRefCollection_Frozen:
					{
						bool InvisibleRefs = Type == kActiveRefCollection_Invisible;
						FORMAT_STR(FilterBuffer, "%s %08X %s", EditorID.c_str(), FormID, FormType);
						if (FilterHelper.PassFilter(FilterBuffer) == false)
							return;

						bool TruthCond = false;
						UInt32 InvisibleReasons = 0;
						if (InvisibleRefs)
							TruthCond = ReferenceVisibilityManager::ShouldBeInvisible(Itr, InvisibleReasons) && InvisibleReasons != ReferenceVisibilityManager::kReason_ParentLayerInvisible;
						else
							TruthCond = Itr->IsFrozen();

						if (TruthCond)
						{
							HasContents = true;
							FORMAT_STR(Label, "%08X-%d", FormID, Type);
							ImGui::PushID(Label);
							{
								if (ImGui::Selectable(EditorID.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick) &&
									ImGui::IsMouseDoubleClicked(0))
								{
									_TES->LoadCellIntoViewPort(nullptr, Itr);
								}

								if (ImGui::IsItemHovered())
									ImGui::SetTooltip(EditorID.c_str());

								if (ImGui::BeginPopupContextItem(InvisibleRefs ? "Invisible_Popup" : "Frozen_Popup"))
								{
									if (InvisibleRefs)
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
							ImGui::Selectable(FormType);
							ImGui::NextColumn();

							if (InvisibleRefs)
							{
								std::string FlagsBuffer;
								if ((InvisibleReasons & ReferenceVisibilityManager::kReason_InvisibleSelf))
									FlagsBuffer.append("IS");

								if ((InvisibleReasons & ReferenceVisibilityManager::kReason_InvisibleChild))
									FlagsBuffer.append(" IC");

								if ((InvisibleReasons & ReferenceVisibilityManager::kReason_InitiallyDisabledSelf))
									FlagsBuffer.append(" DS");

								if ((InvisibleReasons & ReferenceVisibilityManager::kReason_InitiallyDisabledChild))
									FlagsBuffer.append(" DC");

								boost::trim(FlagsBuffer);
								ImGui::Selectable(FlagsBuffer.c_str());
								ImGui::NextColumn();
							}
						}

					}

					break;
				case kActiveRefCollection_Groups:
					{
						const char* ParentGroup = GroupManager->GetParentGroupID(Itr);
						if (ParentGroup)
						{
							// cache unique hits
							std::string GroupID(ParentGroup);
							if (std::find(ActiveGroups.begin(), ActiveGroups.end(), GroupID) == ActiveGroups.end())
								ActiveGroups.push_back(GroupID);
						}
					}

					break;
				}
			}

			switch (Type)
			{
			case kActiveRefCollection_Groups:
				{
					if (ActiveGroups.size())
					{
						TESObjectREFRArrayT GroupMembers;
						GroupMembers.reserve(100);

						bool Dissolved = false;
						for (auto Itr : ActiveGroups)
						{
							HasContents = true;
							if (GroupManager->GetGroupData(Itr.c_str(), GroupMembers))
							{
								SME_ASSERT(GroupMembers.size());

								// filter with members first
								int NonMatchCount = 0;
								for (auto Member : GroupMembers)
								{
									std::string EditorID(IRenderWindowOSDLayer::Helpers::GetRefEditorID(Member));
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

								FORMAT_STR(Label, "%s-group-%d", Itr.c_str(), Type);
								ImGui::PushID(Label);
								{
									if (ImGui::Selectable(Itr.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick) &&
										ImGui::IsMouseDoubleClicked(0))
									{
										_RENDERWIN_MGR.GetGroupManager()->SelectAffiliatedGroup(GroupMembers[0], _RENDERSEL, true);
									}

									if (ImGui::IsItemHovered())
										ImGui::SetTooltip(Itr.c_str());

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
																							std::bind(&RenderModalEditGroupMembers,
																									  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
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
									std::string EditorID(IRenderWindowOSDLayer::Helpers::GetRefEditorID(Member));
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
			}

			if (HasContents == false)
			{
				// add a dummy item
				int Cols = 0;
				switch (Type)
				{
				case kActiveRefCollection_Groups:
					Cols = 2;
					break;
				case kActiveRefCollection_Invisible:
					Cols = 4;
					break;
				case kActiveRefCollection_Frozen:
					Cols = 3;
					break;
				}

				for (int i = 0; i < Cols; i++)
				{
					ImGui::TextDisabled("-");
					ImGui::NextColumn();
				}
			}
		}

		void RenderWindowManager::RenderActiveRefCollectionButton(UInt8 Type)
		{
			const ImVec4 MainColor(0, 0, 0, 0);

			ImGui::PushStyleColor(ImGuiCol_Button, MainColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, MainColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, MainColor);
			const char* Hover = nullptr;
			switch (Type)
			{
			case kActiveRefCollection_Invisible:
				ImGui::Button(ICON_MD_VISIBILITY_OFF "##popupbtn_invisible_refs", ImVec2(0, 0));
				Hover = "Invisible References";
				break;
			case kActiveRefCollection_Frozen:
				ImGui::Button(ICON_MD_LOCK "##popupbtn_frozen_refs", ImVec2(0, 0));
				Hover = "Frozen References";
				break;
			case kActiveRefCollection_Groups:
				ImGui::Button(ICON_MD_SELECT_ALL "##popupbtn_ref_groups", ImVec2(0, 0));
				Hover = "Reference Groups";
				break;
			}

			if (Hover && ImGui::IsItemHoveredRect())
				ImGui::SetTooltip(Hover);

			ImGui::PopStyleColor(3);
		}

		void RenderWindowManager::RenderActiveRefCollectionPopup(UInt8 Type)
		{
			static ImGuiTextFilter TextFilters[kActiveRefCollection__MAX];

			ImGuiTextFilter& CurrentFilter = TextFilters[Type];
			CurrentFilter.Draw();

			ImGui::BeginChild("contents_child_frame", ImVec2(0, 250));
			{
				switch (Type)
				{
				case kActiveRefCollection_Invisible:
				case kActiveRefCollection_Frozen:
					{
						bool InvisibleRefs = Type == kActiveRefCollection_Invisible;
						ImGui::Columns(2, "ref_table_header", false);
						{
							if (InvisibleRefs)
							{
								ImGui::TextWrapped("Invisible References: (?)");
								if (ImGui::IsItemHovered())
									ImGui::SetTooltip("Excluding references whose parent layer is hidden.\nEditorIDs with an asterisk correspond to the reference's base form.\n\nRight click on an item to display the context menu.");
							}
							else
							{
								ImGui::TextWrapped("Frozen References: (?)");
								if (ImGui::IsItemHovered())
									ImGui::SetTooltip("Excluding references whose parent layer is frozen and those that are frozen with the \"Freeze Inactive Refs\" tool.\nEditorIDs with an asterisk correspond to the reference's base form.\n\nRight click on an item to display the context menu.");
							}

							ImGui::NextColumn();

							if (InvisibleRefs)
							{
								if (ImGui::Button(ICON_MD_VISIBILITY_OFF "##hide_selection_btn"))
									actions::ToggleSelectionVisibility();
								if (ImGui::IsItemHovered())
									ImGui::SetTooltip("Toggle Selection Visibility");

								ImGui::SameLine(0, 10);
								if (ImGui::Button(ICON_MD_FLIP_TO_FRONT "##reveal_all_btn"))
									actions::RevealAll();
								if (ImGui::IsItemHovered())
									ImGui::SetTooltip("Reveal All");
							}
							else
							{
								const char* FreezeInactiveCaption = nullptr;
								const char* FreezeInactiveToolTip = nullptr;
								if (_RENDERWIN_XSTATE.FreezeInactiveRefs)
								{
									FreezeInactiveCaption = ICON_MD_LOCK " " ICON_MD_STAR "##freeze_inactive_refs";
									FreezeInactiveToolTip = "Inactive References Frozen";

									ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0, 0.6f, 0.6f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0, 0.7f, 0.7f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0, 0.8f, 0.8f));
								}
								else
								{
									FreezeInactiveCaption = ICON_MD_LOCK_OPEN " " ICON_MD_STAR "##freeze_inactive_refs";
									FreezeInactiveToolTip = "Inactive References not Frozen";

									ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));
								}

								if (ImGui::Button(FreezeInactiveCaption, ImVec2(45, 0)))
									_RENDERWIN_XSTATE.FreezeInactiveRefs = _RENDERWIN_XSTATE.FreezeInactiveRefs == false;

								if (ImGui::IsItemHovered())
									ImGui::SetTooltip(FreezeInactiveToolTip);

								ImGui::PopStyleColor(3);
								ImGui::SameLine(0, 20);

								if (ImGui::Button(ICON_MD_LOCK "##freeze_selection_btn"))
									actions::FreezeSelection();
								if (ImGui::IsItemHovered())
									ImGui::SetTooltip("Freeze Selection");

								ImGui::SameLine(0, 10);
								if (ImGui::Button(ICON_MD_LOCK_OPEN "##thaw_all_btn"))
									actions::ThawAll();
								if (ImGui::IsItemHovered())
									ImGui::SetTooltip("Thaw All");
							}

							ImGui::NextColumn();
						}
						ImGui::Columns();

						ImGui::Columns(InvisibleRefs ? 4 : 3, "ref_table_invisible/frozen", false);
						{
							ImGui::Separator();
							ImGui::Text("EditorID"); ImGui::NextColumn();
							ImGui::Text("FormID"); ImGui::NextColumn();
							ImGui::Text("Type"); ImGui::NextColumn();

							if (InvisibleRefs)
							{
								ImGui::Text("Reason (?)");
								if (ImGui::IsItemHovered())
									ImGui::SetTooltip("DS - Initially Disabled\nDC - Child of Initially Disabled Parent\nIS - Invisible\nIC - Child of Parent with \"Invisible Children\" Flag");
								ImGui::NextColumn();
							}

							ImGui::Separator();
						}
						ImGui::Columns();

						ImGui::BeginChild("ref_list_child_frame", ImVec2(0, 165));
						{
							ImGui::Columns(InvisibleRefs ? 4 : 3, "ref_table_invisible/frozen", false);
							{
								RenderActiveRefCollectionRefTable(Type, CurrentFilter);
							}
							ImGui::Columns();
						}
						ImGui::EndChild();
					}

					break;
				case kActiveRefCollection_Groups:
					{
						ImGui::Columns(2, "ref_table_header", false);
						{
							ImGui::TextWrapped("Reference Groups: (?)");
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("EditorIDs with an asterisk correspond to the reference's base form.\n\nDouble click on an item to select the group.\nRight click on an item to display the context menu.\nHover the cursor over the \"Count\" column to view the first 10 members of the group.");

							ImGui::NextColumn();

							if (ImGui::Button(ICON_MD_GROUP_WORK "##group_selection"))
								actions::GroupSelection();
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("Group Current Selection");

							ImGui::NextColumn();
						}
						ImGui::Columns();

						ImGui::Columns(2, "ref_table_groups", false);
						{
							ImGui::Separator();
							ImGui::Text("Name"); ImGui::NextColumn();
							ImGui::Text("Count"); ImGui::NextColumn();

							ImGui::Separator();
						}
						ImGui::Columns();

						ImGui::BeginChild("ref_list_child_frame", ImVec2(0, 165));
						{
							ImGui::Columns(2, "ref_table_groups", false);
							{
								RenderActiveRefCollectionRefTable(Type, CurrentFilter);
							}
							ImGui::Columns();
						}
						ImGui::EndChild();
					}

					break;
				}
			}
			ImGui::EndChild();
		}

		bool RenderWindowManager::RenderModalEditGroupMembers(RenderWindowOSD* OSD, ImGuiDX9* GUI, void* UserData)
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
					ImGui::Text("EditorID"); ImGui::NextColumn();
					ImGui::Text("FormID"); ImGui::NextColumn();
					ImGui::Text("Type"); ImGui::NextColumn();
					ImGui::Separator();

					int Count = 0;
					TESObjectREFRArrayT::iterator ToRemove = Data->MemberList.end();
					bool Removed = false;
					for (TESObjectREFRArrayT::iterator Member = Data->MemberList.begin(); Member != Data->MemberList.end(); ++Member)
					{
						std::string EditorID(IRenderWindowOSDLayer::Helpers::GetRefEditorID(*Member));
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
			SME_ASSERT(RenderingScene == false);

			CacheActiveRefs();
			RenderingScene = true;
			SceneGraphManager->HandleRender(Camera, SceneGraph, CullingProc, RenderTarget);
		}

		void RenderWindowManager::HandlePostSceneGraphRender()
		{
			SME_ASSERT(Initialized);
			SME_ASSERT(RenderingScene);

			// no OSD for the aux viewport
			if (AUXVIEWPORT->IsRenderingPerspective() == false)
			{
				OSD->Draw();
				OSD->Render();
			}

			RenderingScene = false;
		}

		void RenderWindowManager::HandlePostRenderWindowUpdate()
		{
			SME_ASSERT(RenderingScene == false);

			DeferredExecutor->HandlePostRenderWindowUpdate();
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