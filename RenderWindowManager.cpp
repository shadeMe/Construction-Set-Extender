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
			GridCellArray* CellGrid = _TES->gridCellArray;
			for (int i = 0; i < CellGrid->size; i++)
			{
				for (int j = 0; j < CellGrid->size; j++)
				{
					GridCellArray::GridEntry* Data = CellGrid->GetCellEntry(i, j);
					if (Data && Data->cell)
					{
						TESObjectLAND* Land = Data->cell->land;
						if (Land)
						{
							for (int k = 0; k < 3; k++)
							{
								NiNode* Node = Land->GetQuadLandNode(k);
								NiTriStrips* LandTriStrips = NI_CAST(Node->m_children.data[0], NiTriStrips);
								if (LandTriStrips)
								{
									BSShaderProperty* ShaderProp = (BSShaderProperty*)TESRender::GetProperty(LandTriStrips, BSShaderPPLightingProperty::kType);
									BSShaderPPLightingProperty* PPLighting = NI_CAST(ShaderProp, BSShaderPPLightingProperty);
									if (PPLighting)
									{
										int TexNo = 0;
										while (TexNo < 9)
										{
											NiSourceTexture* currentTex = NI_CAST(PPLighting->diffuse[TexNo], NiSourceTexture);
											const char* SourcePath = settings::renderer::kGrassOverlayTexturePath().s;
											TexNo++;
										}
									}
								}
							}
						}
					}
				}
			}
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

		void RenderWindowSelectionManager::AddToSelection(TESObjectREFR* Ref, bool AddSelectionBox) const
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
			if (IsSelectable(Ref, RegularHandling, Throwaway))
			{
				if (RegularHandling || _RENDERWIN_XSTATE.PaintingSelection)
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

		bool RenderWindowSelectionManager::IsSelectable(TESObjectREFR* Ref, bool& OutRegularHandling, UInt32& OutReasonFlags) const
		{
			OutReasonFlags = 0;
			OutRegularHandling = false;

			bool Result = true;
			if (GetAsyncKeyState(VK_MENU) && _RENDERWIN_XSTATE.PaintingSelection == false)
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

		bool RenderWindowSelectionManager::IsSelectable(TESObjectREFR* Ref) const
		{
			bool Throwaway = false;
			UInt32 ThrowawayEx = 0;
			return IsSelectable(Ref, Throwaway, ThrowawayEx);
		}

		bool RenderWindowSelectionManager::IsSelectable(TESObjectREFR* Ref, UInt32& OutReasonFlags) const
		{
			bool Throwaway = false;
			return IsSelectable(Ref, Throwaway, OutReasonFlags);
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


		RenderWindowExtendedState		RenderWindowExtendedState::Instance;

		RenderWindowExtendedState::RenderWindowExtendedState() :
			Initialized(false),
			MaxLandscapeEditBrushRadius(25.f)
		{
			CurrentMouseLBDragCoordDelta = { 0, 0 };
			UseAlternateMovementSettings = false;
			FreezeInactiveRefs = false;
			CameraFrustumBuffer = { 0 };
			CurrentMouseCoord = { 0 };
			CurrentMouseRef = nullptr;
			CurrentMousePathGridPoint = nullptr;
			ShowInitiallyDisabledRefs = true;
			ShowInitiallyDisabledRefChildren = true;
			UseGrassTextureOverlay = false;
			GrassOverlayTexture = nullptr;
			StaticCameraPivot.Scale(0);
			PaintingSelection = false;
			SelectionPaintingMode = kSelectionPaintingMode_NotSet;
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

		RenderWindowManager				_RENDERWIN_MGR;


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
								case IDC_RENDERWINDOWCONTEXT_SWITCHCNY:
									if (settings::renderer::kSwitchCAndY.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_STATICCAMERAPIVOT:
									if (settings::renderer::kFixedCameraPivot.GetData().i)
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
								case IDC_RENDERWINDOWCONTEXT_USEALTERNATEMOVEMENTSETTINGS:
									if (_RENDERWIN_XSTATE.UseAlternateMovementSettings)
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
								case IDC_RENDERWINDOWCONTEXT_GRASSOVERLAY:
									CheckItem = _RENDERWIN_XSTATE.UseGrassTextureOverlay;

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
						_RENDERWIN_XSTATE.UseAlternateMovementSettings = (_RENDERWIN_XSTATE.UseAlternateMovementSettings == false);

						if (_RENDERWIN_XSTATE.UseAlternateMovementSettings == false)
							NotificationOSDLayer::Instance.ShowNotification("Using vanilla movement settings");
						else
							NotificationOSDLayer::Instance.ShowNotification("Using alternate movement settings");

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
					_RENDERWIN_XSTATE.FreezeInactiveRefs = (_RENDERWIN_XSTATE.FreezeInactiveRefs == false);

					if (_RENDERWIN_XSTATE.FreezeInactiveRefs)
						NotificationOSDLayer::Instance.ShowNotification("Inactive references frozen");
					else
						NotificationOSDLayer::Instance.ShowNotification("Inactive references thawed");

					break;
				case IDC_RENDERWINDOWCONTEXT_INVERTSELECTION:
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
							TESRenderWindow::Redraw();
						}
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_BATCHREFERENCEEDITOR:
					{
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

						break;
					}
				case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
				case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
					{
						const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
						for (TESObjectREFRArrayT::const_iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
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
								Ref->SetFrozen(false);

								break;
							}
						}

						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
							NotificationOSDLayer::Instance.ShowNotification("Reset visibility flags on the active cell/grid's references");

							break;
						case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
							NotificationOSDLayer::Instance.ShowNotification("Thawed all of the active cell/grid's references");

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
							Ref->SetFrozen(true);
							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefFreezing);

							break;
						case IDC_RENDERWINDOWCONTEXT_THAW:
							Ref->SetFrozen(false);
							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefFreezing);

							break;
						}
					}

					TESRenderWindow::Redraw();
					Return = true;

					break;
				case IDC_RENDERWINDOWCONTEXT_GROUP:
					{
						if (_RENDERWIN_MGR.GetReferenceGroupManager()->IsSelectionGroupable(_RENDERSEL))
							_RENDERWIN_MGR.CreateRefGroup();

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_UNGROUP:
					{
						Return = true;
						_RENDERWIN_MGR.DeleteRefGroup();
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
				case IDC_RENDERWINDOWCONTEXT_GRASSOVERLAY:
					_RENDERWIN_XSTATE.UseGrassTextureOverlay = _RENDERWIN_XSTATE.UseGrassTextureOverlay == false;
					_TES->ReloadLandscapeTextures();
					Return = true;

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
			case WM_INITDIALOG:
				break;
			case WM_DESTROY:
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
						_RENDERWIN_MGR.RefreshFOV();
					}


					// update and render the aux viewport's perspective
					if (AUXVIEWPORT->IsFrozen() == false)
						AUXVIEWPORT->SyncViewportCamera(_PRIMARYRENDERER->primaryCamera);
					else if (AUXVIEWPORT->IsVisible())
						AUXVIEWPORT->Draw(nullptr, nullptr);

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
							if (_RENDERWIN_MGR.SelectionManager->IsSelectable(Ref) == false)
							{
								// preempt the vanilla handler
								Return = true;
							}
						}
					}
				}

				break;
			case WM_SETCURSOR:
				{
					if (GetCapture() != hWnd)
					{
						HCURSOR Icon = *TESRenderWindow::CursorArrow;

						if (*TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0)
						{
							TESObjectREFR* MouseRef = TESRender::PickRefAtCoords(_RENDERWIN_XSTATE.CurrentMouseCoord.x, _RENDERWIN_XSTATE.CurrentMouseCoord.y);
							if (MouseRef)
							{
								UInt32 SelectionReason = 0;
								if (_RENDERWIN_MGR.SelectionManager->IsSelectable(MouseRef, SelectionReason))
								{
									if (_RENDERSEL->HasObject(MouseRef))
										Icon = *TESRenderWindow::CursorMove;
									else
										Icon = *TESRenderWindow::CursorSelect;
								}
								else if ((SelectionReason & RenderWindowSelectionManager::kReason_FrozenInactive) ||
									(SelectionReason & RenderWindowSelectionManager::kReason_FrozenSelf))
								{
									Icon = LoadCursor(nullptr, IDC_NO);
								}
							}
						}

						if (Icon != *TESRenderWindow::CursorArrow)
						{
							HCURSOR CurrentCursor = GetCursor();
							if (Icon && CurrentCursor != Icon)
								SetCursor(Icon);

							DlgProcResult = TRUE;
							Return = true;
						}
					}
				}

				break;
			case WM_MOUSEMOVE:
				{
					_RENDERWIN_XSTATE.CurrentMouseCoord.x = GET_X_LPARAM(lParam);
					_RENDERWIN_XSTATE.CurrentMouseCoord.y = GET_Y_LPARAM(lParam);

					if (_RENDERWIN_XSTATE.PaintingSelection)
					{
						Return = true;
						// paint only when the control key is held down
						if (GetAsyncKeyState(VK_CONTROL))
						{
							TESObjectREFR* MouseRef = TESRender::PickRefAtCoords(_RENDERWIN_XSTATE.CurrentMouseCoord.x, _RENDERWIN_XSTATE.CurrentMouseCoord.y);
							if (MouseRef)
							{
								if (_RENDERWIN_XSTATE.SelectionPaintingMode == RenderWindowExtendedState::kSelectionPaintingMode_NotSet)
								{
									if (_RENDERSEL->HasObject(MouseRef))
										_RENDERWIN_XSTATE.SelectionPaintingMode = RenderWindowExtendedState::kSelectionPaintingMode_Deselect;
									else
										_RENDERWIN_XSTATE.SelectionPaintingMode = RenderWindowExtendedState::kSelectionPaintingMode_Select;
								}

								if (_RENDERWIN_XSTATE.SelectionPaintingMode == RenderWindowExtendedState::kSelectionPaintingMode_Select)
									_RENDERWIN_MGR.SelectionManager->AddToSelection(MouseRef, true);
								else
									_RENDERWIN_MGR.SelectionManager->RemoveFromSelection(MouseRef, true);
							}
						}
					}

					_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
					_RENDERWIN_XSTATE.CurrentMousePathGridPoint = nullptr;

					if (GetActiveWindow() == hWnd && *TESRenderWindow::LandscapeEditFlag == 0 && (_RENDERWIN_XSTATE.PaintingSelection || GetCapture() != hWnd))
					{
						int Enabled = settings::renderWindowOSD::kShowMouseRef.GetData().i;
						int ControlModified = settings::renderWindowOSD::kMouseRefCtrlModified.GetData().i;

						if (Enabled && (_RENDERWIN_XSTATE.PaintingSelection || ControlModified == false || GetAsyncKeyState(VK_CONTROL)))
						{
							if (*TESRenderWindow::PathGridEditFlag == 0)
							{
								_RENDERWIN_XSTATE.CurrentMouseRef = TESRender::PickRefAtCoords(_RENDERWIN_XSTATE.CurrentMouseCoord.x,
																							  _RENDERWIN_XSTATE.CurrentMouseCoord.y);
								if (_RENDERWIN_XSTATE.CurrentMouseRef)
								{
									if (_RENDERWIN_XSTATE.PaintingSelection == false &&
										_RENDERSEL->selectionCount == 1 &&
										_RENDERSEL->selectionList->Data == _RENDERWIN_XSTATE.CurrentMouseRef)
									{
										_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
									}
									else if (ReferenceVisibilityValidator::IsCulled(_RENDERWIN_XSTATE.CurrentMouseRef) ||
											 ReferenceVisibilityValidator::ShouldBeInvisible(_RENDERWIN_XSTATE.CurrentMouseRef))
									{
										_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
									}
								}
							}
							else
							{
								_RENDERWIN_XSTATE.CurrentMousePathGridPoint = TESRender::PickPathGridPointAtCoords(_RENDERWIN_XSTATE.CurrentMouseCoord.x,
																												_RENDERWIN_XSTATE.CurrentMouseCoord.y);
							}
						}
					}
				}

				break;
			case WM_MOUSELEAVE:
			case WM_NCMOUSELEAVE:
				_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;

				break;
			case WM_LBUTTONDOWN:
				if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL) &&
					*TESRenderWindow::LandscapeEditFlag == 0 && *TESRenderWindow::PathGridEditFlag == 0)
				{
					SME_ASSERT(_RENDERWIN_XSTATE.PaintingSelection == false &&
							   _RENDERWIN_XSTATE.SelectionPaintingMode == RenderWindowExtendedState::kSelectionPaintingMode_NotSet);

					_RENDERWIN_XSTATE.PaintingSelection = true;
					NotificationOSDLayer::Instance.ShowNotification("Painting reference selection...");

					SetCapture(hWnd);
					Return = true;
				}

				_RENDERWIN_XSTATE.CurrentMouseLBDragCoordDelta.x = GET_X_LPARAM(lParam);
				_RENDERWIN_XSTATE.CurrentMouseLBDragCoordDelta.y = GET_Y_LPARAM(lParam);

				break;
			case WM_LBUTTONUP:
				if (_RENDERWIN_XSTATE.PaintingSelection)
				{
					_RENDERWIN_XSTATE.PaintingSelection = false;
					_RENDERWIN_XSTATE.SelectionPaintingMode = RenderWindowExtendedState::kSelectionPaintingMode_NotSet;

					ReleaseCapture();
					Return = true;
				}

				_RENDERWIN_XSTATE.CurrentMouseLBDragCoordDelta.x -= GET_X_LPARAM(lParam);
				_RENDERWIN_XSTATE.CurrentMouseLBDragCoordDelta.y -= GET_Y_LPARAM(lParam);

				_RENDERWIN_XSTATE.CurrentMouseLBDragCoordDelta.x = abs(_RENDERWIN_XSTATE.CurrentMouseLBDragCoordDelta.x);
				_RENDERWIN_XSTATE.CurrentMouseLBDragCoordDelta.y = abs(_RENDERWIN_XSTATE.CurrentMouseLBDragCoordDelta.y);

				break;
			case WM_RBUTTONDOWN:
				if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL))
				{
					// handle it for the button up event
					Return = true;
				}

				break;
			case WM_RBUTTONUP:
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
						if (SwitchEnabled && GetAsyncKeyState(VK_CONTROL) == FALSE)
						{
							BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, 0x59, lParam);
							Return = true;
						}
					}

					break;
				case 0x59:		// Y
					{
						int SwitchEnabled = settings::renderer::kSwitchCAndY.GetData().i;
						if (SwitchEnabled && GetAsyncKeyState(VK_CONTROL) == FALSE)
						{
							BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, 0x43, lParam);
							Return = true;
						}
					}

					break;
				}

				break;
			case WM_KEYDOWN:
				switch (wParam)
				{
				case VK_SHIFT:
					_PRIMARYRENDERER->GetCameraPivot(&_RENDERWIN_XSTATE.StaticCameraPivot, 0.18);

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
						_RENDERWIN_MGR.PGUndoManager->PerformUndo();
						Return = true;
					}

					break;
				case 0x59:		// Y
					if (*TESRenderWindow::PathGridEditFlag && GetAsyncKeyState(VK_CONTROL))
					{
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_PathGridAdditions);
						_RENDERWIN_MGR.PGUndoManager->PerformRedo();
						Return = true;
					}
					else
					{
						int SwitchEnabled = settings::renderer::kSwitchCAndY.GetData().i;
						if (SwitchEnabled && GetAsyncKeyState(VK_CONTROL) == FALSE)
						{
							BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, 0x43, lParam);
							Return = true;
						}
					}

					break;
				case 0x43:		// C
					{
						int SwitchEnabled = settings::renderer::kSwitchCAndY.GetData().i;
						if (SwitchEnabled && GetAsyncKeyState(VK_CONTROL) == FALSE)
						{
							BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, 0x59, lParam);
							Return = true;
						}
					}

					break;
				case 0x52:		// R
					if (*TESRenderWindow::PathGridEditFlag)
					{
						if (GetAsyncKeyState(VK_CONTROL))
						{
							_RENDERWIN_MGR.PGUndoManager->ResetRedoStack();

							if (TESRenderWindow::SelectedPathGridPoints->Count())
								_RENDERWIN_MGR.PGUndoManager->RecordOperation(PathGridUndoManager::kOperation_DataChange, TESRenderWindow::SelectedPathGridPoints);

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
					else if (GetAsyncKeyState(VK_SHIFT) && AUXVIEWPORT->IsVisible())
					{
						if (AUXVIEWPORT->ToggleFrozenState())
							NotificationOSDLayer::Instance.ShowNotification("Froze auxiliary viewport camera");
						else
							NotificationOSDLayer::Instance.ShowNotification("Released auxiliary viewport camera");

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
						_RENDERWIN_MGR.RefreshFOV();
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL, NULL);
					}

					break;
				case VK_OEM_3:	// ~
					{
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
				case 0x41:		// A
					if (GetAsyncKeyState(VK_CONTROL))
					{
						Return = true;

						const TESObjectREFRArrayT& Refs = _RENDERWIN_MGR.GetActiveRefs();
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
						for (auto Itr : Delinquents)
							Itr->SetCulled(false);
					}

					break;
				case VK_DELETE:
					// clear the picked objects just in case they are about to be deleted
					_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
					_RENDERWIN_XSTATE.CurrentMousePathGridPoint = nullptr;

					break;
				}

				break;
			}

			return DlgProcResult;
		}



		RenderWindowManager::RenderWindowManager() :
			ActiveRefCache()
		{
			SceneGraphManager = new RenderWindowSceneGraphManager();
			PGUndoManager = new PathGridUndoManager();
			GroupManager = new RenderWindowGroupManager();
			SelectionManager = new RenderWindowSelectionManager(GroupManager);
			OSD = new RenderWindowOSD();
			CellLists = new RenderWindowCellLists();
			EventSink = new GlobalEventSink(this);
			ActiveRefCache.reserve(200);

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
			SAFEDELETE(CellLists);

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

			SceneGraphManager->AddModifier(&ReferenceParentChildIndicator::Instance);
			SceneGraphManager->AddModifier(&ReferenceVisibilityModifier::Instance);
#ifndef NDEBUG
			SceneGraphManager->AddModifier(&DebugSceneGraphModifier::Instance);
#endif
			CellLists->Initialize(OSD);
			GroupManager->Initialize();

			_RENDERWIN_XSTATE.Initialize();
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

			GroupManager->Deinitialize();
			CellLists->Deinitialize(OSD);
			OSD->Deinitialize();
			SceneGraphManager->RemoveModifier(&ReferenceParentChildIndicator::Instance);
			SceneGraphManager->RemoveModifier(&ReferenceVisibilityModifier::Instance);
#ifndef NDEBUG
			SceneGraphManager->RemoveModifier(&DebugSceneGraphModifier::Instance);
#endif
			_RENDERWIN_XSTATE.Deinitialize();
			Initialized = false;
		}

		RenderWindowGroupManager* RenderWindowManager::GetReferenceGroupManager() const
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

		void RenderWindowManager::InvokeContextMenuTool(int Identifier)
		{
			SME_ASSERT(Initialized);
			SendMessage(*TESRenderWindow::WindowHandle, WM_COMMAND, Identifier, NULL);
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

		bool RenderWindowManager::RenderModalNewRefGroup(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			static char NewGroupNameBuffer[0x100] = { 0 };

			ImGui::InputText("Name", NewGroupNameBuffer, sizeof(NewGroupNameBuffer));
			bool InvalidName = GroupManager->GetGroupExists(NewGroupNameBuffer);
			if (InvalidName)
				ImGui::Text("Group already exists!");
			else if (strlen(NewGroupNameBuffer) == 0)
				InvalidName = true;

			ImGui::Separator();
			bool Close = false;
			if (ImGui::Button("OK", ImVec2(120, 0)) && InvalidName == false)
			{
				if (GroupManager->AddGroup(NewGroupNameBuffer, _RENDERSEL) == false)
					NotificationOSDLayer::Instance.ShowNotification("Couldn't add current selection to a new group.");
				else
					NotificationOSDLayer::Instance.ShowNotification("Created new selection group");

				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefGrouping);
				Close = true;
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
		}

		void RenderWindowManager::CreateRefGroup()
		{
			ModalWindowProviderOSDLayer::Instance.ShowModal("New Reference Group",
															std::bind(&RenderWindowManager::RenderModalNewRefGroup, this, std::placeholders::_1, std::placeholders::_2),
															ImGuiWindowFlags_AlwaysAutoResize);
		}

		void RenderWindowManager::DeleteRefGroup()
		{
			if (GroupManager->RemoveGroup(_RENDERSEL) == false)
				NotificationOSDLayer::Instance.ShowNotification("Couldn't dissolve the current selection's group.");
			else
				NotificationOSDLayer::Instance.ShowNotification("Removed selection group");
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