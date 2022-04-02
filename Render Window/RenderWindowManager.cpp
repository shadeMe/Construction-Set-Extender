#include "Render Window\RenderWindowManager.h"
#include "Construction Set Extender_Resource.h"
#include "AuxiliaryViewport.h"
#include "ObjectPaletteManager.h"
#include "ObjectPrefabManager.h"
#include "Achievements.h"
#include "GlobalClipboard.h"
#include "[Common]\CLIWrapper.h"
#include "RenderWindowActions.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"
#include "ToolbarOSDLayer.h"
#include "RenderWindowFlyCamera.h"

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

		void ReferenceParentChildIndicator::LazyInitialize()
		{
			// these need lazy initialization as the dynamic initializer for the static instance will
			// get called even when the plugin is loaded in an non-editor context
			if (Initialized)
				return;

			VertexColor = (NiVertexColorProperty*)TESRender::CreateProperty(NiVertexColorProperty::kType);
			VertexColor->flags |= NiVertexColorProperty::kSrcMode_Emissive;

			Wireframe = (NiWireframeProperty*)TESRender::CreateProperty(NiWireframeProperty::kType);
			Wireframe->m_bWireframe = 1;

			Initialized = true;
		}

		ReferenceParentChildIndicator::ReferenceParentChildIndicator()
		{
			Initialized = false;

			VertexColor = nullptr;
			Wireframe = nullptr;
		}

		ReferenceParentChildIndicator::~ReferenceParentChildIndicator()
		{
			if (!Initialized)
				return;

			SME_ASSERT(Wireframe->m_uiRefCount == 1);
			TESRender::DeleteNiRefObject(Wireframe);

			SME_ASSERT(VertexColor->m_uiRefCount == 1);
			TESRender::DeleteNiRefObject(VertexColor);
		}

		void ReferenceParentChildIndicator::PreRender(RenderData& Data)
		{
			LazyInitialize();

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
				if (ReferenceVisibilityManager::ShouldBeInvisible(Itr, Reason))
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

		bool ReferenceVisibilityManager::ShouldBeInvisible(TESObjectREFR* Ref)
		{
			UInt32 Throwaway = 0;
			return ShouldBeInvisible(Ref, Throwaway);
		}

		bool ReferenceVisibilityManager::ShouldBeInvisible(TESObjectREFR* Ref, UInt32& OutReasonFlags)
		{
			OutReasonFlags = 0;

			if (_RENDERWIN_XSTATE.ShowInitiallyDisabledRefs == false && Ref->IsInitiallyDisabled())
				OutReasonFlags |= kReason_InitiallyDisabledSelf;

			if (Ref->IsInvisible())
				OutReasonFlags |= kReason_InvisibleSelf;

			BSExtraData* xData = Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
			if (xData)
			{
				ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
				if (xParent->parent->IsChildrenInvisible())
					OutReasonFlags |= kReason_InvisibleChild;

				if (xParent->parent->IsInitiallyDisabled() && _RENDERWIN_XSTATE.ShowInitiallyDisabledRefChildren == false)
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

		ReferenceColorMaskManager::ReferenceColorMaskManager()
		{
			Masks[kMask_Selection].ToggleSetting = &settings::renderer::kShowSelectionMask;
			Masks[kMask_Selection].ColorSetting = &settings::renderer::kSelectionMaskColor;

			Masks[kMask_MouseOver].ToggleSetting = &settings::renderer::kShowMouseOverMask;
			Masks[kMask_MouseOver].ColorSetting = &settings::renderer::kMouseOverMaskColor;
		}


		void ReferenceColorMaskManager::Initialize()
		{
			for (auto& Mask : Masks)
			{
				Mask.Enabled = Mask.ToggleSetting->GetData().i;

				int MaskColorR = 0, MaskColorG = 0, MaskColorB = 0;
				SME::StringHelpers::GetRGB(Mask.ColorSetting->GetData().s, MaskColorR, MaskColorG, MaskColorB);
				Mask.Color.r = MaskColorR / 255.f;
				Mask.Color.g = MaskColorG / 255.f;
				Mask.Color.b = MaskColorB / 255.f;
			}
		}

		void ReferenceColorMaskManager::Deinitialize()
		{
			for (auto& Mask : Masks)
			{
				Mask.ToggleSetting->SetInt(Mask.Enabled);

				Mask.ColorSetting->SetString("%d,%d,%d",
					static_cast<int>(Mask.Color.r * 255),
					static_cast<int>(Mask.Color.g * 255),
					static_cast<int>(Mask.Color.b * 255));
			}
		}

		bool ReferenceColorMaskManager::IsAnyMaskEnabled() const
		{
			for (const auto& Mask : Masks)
			{
				if (Mask.Enabled)
					return true;
			}

			return false;
		}

		bool ReferenceColorMaskManager::GetMaskEnabled(UInt8 Mask) const
		{
			SME_ASSERT(Mask < kMask__MAX);

			for (auto i = kMask__BEGIN + 1; i < kMask__MAX; ++i)
			{
				const auto& ThisMask = Masks[i];
				if (Mask == i)
					return ThisMask.Enabled;
			}

			return false;

		}

		void ReferenceColorMaskManager::SetMaskEnabled(UInt8 Mask, bool Enabled)
		{
			SME_ASSERT(Mask < kMask__MAX);

			Masks[Mask].Enabled = Enabled;
		}

		const NiColor& ReferenceColorMaskManager::GetMaskColor(UInt8 Mask) const
		{
			SME_ASSERT(Mask < kMask__MAX);

			return Masks[Mask].Color;
		}

		void ReferenceColorMaskManager::SetMaskColor(UInt8 Mask, const NiColor& Color)
		{
			SME_ASSERT(Mask < kMask__MAX);

			Masks[Mask].Color.r = Color.r;
			Masks[Mask].Color.g = Color.g;
			Masks[Mask].Color.b = Color.b;
		}

		bool ReferenceColorMaskManager::GetActiveMaskForRef(TESObjectREFR* Ref, NiColor* OutColor) const
		{
			SME_ASSERT(Ref);

			const auto& MouseOverMask = Masks[kMask_MouseOver];
			if (MouseOverMask.Enabled && _RENDERWIN_XSTATE.CurrentMouseRef == Ref)
			{
				OutColor->r = MouseOverMask.Color.r;
				OutColor->g = MouseOverMask.Color.g;
				OutColor->b = MouseOverMask.Color.b;

				return true;
			}

			const auto& SelectionMask = Masks[kMask_Selection];
			if (SelectionMask.Enabled && _RENDERSEL->HasObject(Ref))
			{
				OutColor->r = SelectionMask.Color.r;
				OutColor->g = SelectionMask.Color.g;
				OutColor->b = SelectionMask.Color.b;

				return true;
			}

			return false;
		}

		const float RenderWindowExtendedState::kMaxLandscapeEditBrushRadius = 25.f;

		RenderWindowExtendedState::RenderWindowExtendedState() :
			Initialized(false)
		{
			UseAlternateMovementSettings = false;
			FreezeInactiveRefs = false;
			CameraFrustumBuffer = { 0 };
			CurrentMouseRef = nullptr;
			CurrentMousePathGridPoint = nullptr;
			CurrentMouseExteriorCell = nullptr;
			ShowInitiallyDisabledRefs = true;
			ShowInitiallyDisabledRefChildren = true;
			UseGrassTextureOverlay = false;
			GrassOverlayTexture = nullptr;
			StaticCameraPivot.Scale(0);
			DraggingPathGridPoints = false;
			MeasureBaseRuler = MeasureBaseCircle = nullptr;
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
			if (Executing)
				return;

			Executing = true;

			for (auto& Itr : Handlers)
				Itr();

			Handlers.clear();

			Executing = false;
		}

		void RenderWindowDeferredExecutor::QueueTask(DelegateT Delegate)
		{
			SME_ASSERT(Executing == false);

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
			case events::TypedEventSource::kType_CellView_SelectCell:
				Parent->GetState().CurrentMouseRef = nullptr;
				Parent->GetState().CurrentMousePathGridPoint = nullptr;
				Parent->GetState().CurrentMouseExteriorCell = nullptr;
				break;
			case events::TypedEventSource::kType_Plugin_ClearData:
				Parent->HandleClearData();
				break;
			case events::TypedEventSource::kType_Plugin_ConstructSpecialForms:
				Parent->HandleConstructSpecialForms();
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
																bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT DlgProcResult = TRUE;

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
								bool CheckItem = false;
								bool DisableItem = false;
								char NewItemText[0x200] = { 0 };

								switch (CurrentItem.wID)
								{
								case IDC_RENDERWINDOWCONTEXT_GROUP:
								case IDC_RENDERWINDOWCONTEXT_UNGROUP:
								case IDC_RENDERWINDOWCONTEXT_FREEZE:
								case IDC_RENDERWINDOWCONTEXT_THAW:
								case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL :
								case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
								case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
								case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
								case IDC_RENDERWINDOWCONTEXT_INVERTSELECTION:
								case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
								case IDC_RENDERWINDOWCONTEXT_UNLOADCURRENTCELLS:
								case IDC_RENDERWINDOWCONTEXT_COPYTOGLOBALCLIPBOARD:
								case IDC_RENDERWINDOWCONTEXT_PASTEFROMGLOBALCLIPBOARD:
								case IDC_RENDERWINDOWCONTEXT_OSD_REFERENCEEDITOR:
								case IDC_RENDERWINDOWCONTEXT_RENDERWINDOWHOTKEYS:
								case IDC_RENDERWINDOWCONTEXT_OSD_EDITCOLORTHEME:
								case IDC_RENDERWINDOWCONTEXT_SPAWNRULERMEASURE:
								case IDC_RENDERWINDOWCONTEXT_SPAWNCIRCLEMEASURE:
									DisableItem = TESRenderWindow::IsAnyCellLoaded() == false;
									break;
								case IDC_RENDERWINDOWCONTEXT_SAVEEXTERIORSNAPSHOT:
									DisableItem = *TESRenderWindow::ActiveCell == nullptr || _TES->currentInteriorCell;
									break;
								}

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

									if (*TESRenderWindow::ActiveCell && _TES->currentInteriorCell == nullptr)
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
								case IDC_RENDERWINDOWCONTEXT_OSD_REFERENCEEDITOR:
									if (settings::renderWindowOSD::kShowRefBatchEditor().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_OSD_TOOLBARS:
									if (settings::renderWindowOSD::kShowToolbar().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_UNRESTRICTEDMOUSEMOVEMENT:
									if (settings::renderer::kUnrestrictedMouseMovement().i)
										CheckItem = true;

									break;
								}

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

					SubclassParams->Out.MarkMessageAsHandled = true;
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
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
					{
						actions::ToggleFreezeInactive();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_INVERTSELECTION:
					{
						actions::InvertSelection();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
					{
						actions::ThawAll();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
					{
						actions::RevealAll();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
					{
						actions::ToggleSelectionVisibility();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
					{
						actions::ToggleSelectionChildrenVisibility();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_FREEZE:
					{
						actions::FreezeSelection();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_THAW:
					{
						actions::ThawSelection();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_GROUP:
					{
						actions::GroupSelection();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_UNGROUP:
					{
						actions::UngroupSelection();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_COPLANARDROP:
					{
						settings::renderer::kCoplanarRefDrops.ToggleData();

						if (settings::renderer::kCoplanarRefDrops.GetData().i)
							NotificationOSDLayer::Instance.ShowNotification("Enabled co-planar dropping");
						else
							NotificationOSDLayer::Instance.ShowNotification("Disabled co-planar dropping");

						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_COPYTOGLOBALCLIPBOARD:
					{
						globalClipboard::GlobalCopyBuffer Buffer;

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
							Buffer.Add(Itr->Data);

						Buffer.Copy();
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_GlobalClipboard);
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_PASTEFROMGLOBALCLIPBOARD:
					{
						BGSEECLIPBOARD->Paste();
						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_GlobalClipboard);
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_MOUSEREF:
					{
						settings::renderWindowOSD::kShowMouseRef.ToggleData();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_MOUSEREF_CTRLMODIFIED:
					{
						settings::renderWindowOSD::kMouseRefCtrlModified.ToggleData();
						SubclassParams->Out.MarkMessageAsHandled = true;
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
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_REFERENCEEDITOR:
					{
						actions::ShowBatchEditor();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_TOOLBARS:
					{
						settings::renderWindowOSD::kShowToolbar.ToggleData();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_RENDERWINDOWHOTKEYS:
					SubclassParams->Out.MarkMessageAsHandled = true;
					Instance.KeyboardInputManager->ShowHotKeyEditor();

					break;
				case IDC_RENDERWINDOWCONTEXT_UNRESTRICTEDMOUSEMOVEMENT:
					{
						settings::renderer::kUnrestrictedMouseMovement.ToggleData();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_OSD_EDITCOLORTHEME:
					{
						RenderWindowOSD::ShowColorThemeEditor();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_SPAWNRULERMEASURE:
					{
						actions::CreateMeasureRuler();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_SPAWNCIRCLEMEASURE:
					{
						actions::CreateMeasureCircle();
						SubclassParams->Out.MarkMessageAsHandled = true;
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}


		LRESULT CALLBACK RenderWindowManager::RenderWindowMasterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													  bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT DlgProcResult = TRUE;

			if (Instance.Initialized == false || BGSEEDAEMON->IsCrashing())
			{
				// bugger off
				return DlgProcResult;
			}
			if (RenderWindowFlyCamera::Instance.IsActive() && uMsg != WM_DESTROY)
			{
				// do nothing if the fly camera is active
				return DlgProcResult;
			}
			else if (Instance.OSD->HandleMessage(hWnd, uMsg, wParam, lParam, &Instance))
			{
				// OSD handled the message, consume
				SubclassParams->Out.MarkMessageAsHandled = true;
				return DlgProcResult;
			}
			else if (Instance.KeyboardInputManager->HandleInput(hWnd, uMsg, wParam, lParam, &Instance))
			{
				// key input was handled, consume message
				SubclassParams->Out.MarkMessageAsHandled = true;
				return DlgProcResult;
			}
			else if (Instance.MouseInputManager->HandleInput(hWnd, uMsg, wParam, lParam, &Instance))
			{
				// mouse input was handled, consume message
				SubclassParams->Out.MarkMessageAsHandled = true;
				return DlgProcResult;
			}

			switch (uMsg)
			{
			case WM_QUIT:
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_RenderWindow, NULL);
				SubclassParams->Out.MarkMessageAsHandled = true;

				break;
			case WM_MOUSEMOVE:
			case WM_NCMOUSEMOVE:
				if (GetActiveWindow() == hWnd)
					Instance.MouseInClientArea = true;

				break;
			case WM_MOUSELEAVE:
			case WM_NCMOUSELEAVE:
				Instance.MouseInClientArea = false;

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
						TESLODTextureGenerator::GeneratorState == TESLODTextureGenerator::kState_NotInUse &&
						Instance.IsRenderingScene() == false)
					{
						AUXVIEWPORT->Draw(nullptr, nullptr);
					}

					if (Instance.IsRenderingScene())
					{
						// consume the message as the previous render call isn't done yet
						SubclassParams->Out.MarkMessageAsHandled = true;
					}
					else if (Instance.MouseInClientArea || Instance.MouseInputManager->IsFreeMouseMovementActive())
					{
						// always render when the mouse is in the client area/in free-movement
						TESRenderWindow::Redraw();
					}

					break;
				}

				break;
			case WM_WINDOWPOSCHANGING:
				{
					if (TESPreviewControl::ActivePreviewControls->Count() > 0)
					{
						// Prevent the user from resizing the render window when an preview control is active.
						// Otherwise, this inevitably leads to memory corruption in the render (use-after-frees galore w.r.t off-screen buffers when recreating the renderer context).
						auto WindowPosData = reinterpret_cast<WINDOWPOS*>(lParam);
						if (WindowPosData->cx != 0 && WindowPosData->cy != 0)
						{
							RECT CurrentWindowRect;
							GetWindowRect(hWnd, &CurrentWindowRect);

							auto CurrentWidth = CurrentWindowRect.right - CurrentWindowRect.left;
							auto CurrentHeight = CurrentWindowRect.bottom - CurrentWindowRect.top;

							if (WindowPosData->cx != CurrentWidth || WindowPosData->cy != CurrentHeight)
							{
								WindowPosData->flags |= SWP_NOSIZE;
								DlgProcResult = FALSE;

								BGSEEUI->MsgBoxW(hWnd, 0, "Please close any dialogs with preview controls before attempting to resize the render window.");
								SubclassParams->Out.MarkMessageAsHandled = true;
							}
						}
					}

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
			ColorMaskManager = new ReferenceColorMaskManager();
			GizmoManager = new RenderWindowGizmoManager();
			ActiveRefCache.reserve(500);
			RenderingScene = false;
			MouseInClientArea = false;

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
			SAFEDELETE(ColorMaskManager);
			SAFEDELETE(GizmoManager);

			Initialized = false;
		}

		bool RenderWindowManager::Initialize()
		{
			SME_ASSERT(Initialized == false);

			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_RenderWindow,
				RenderWindowMenuInitSelectSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_RenderWindow,
				RenderWindowMasterSubclassProc);
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
			events::dialog::cellView::kSelectCell.AddSink(EventSink);
			events::plugin::kClearData.AddSink(EventSink);
			events::plugin::kConstructSpecialForms.AddSink(EventSink);

			SceneGraphManager->AddModifier(&ReferenceParentChildIndicator::Instance);
			SceneGraphManager->AddModifier(&ReferenceVisibilityModifier::Instance);

			GizmoManager->Initialize(OSD);
			CellLists->Initialize();
			GroupManager->Initialize();
			LayerManager->Initialize();
			KeyboardInputManager->Initialize();
			MouseInputManager->Initialize(KeyboardInputManager->GetSharedBindings());
			ColorMaskManager->Initialize();

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
			events::dialog::cellView::kSelectCell.RemoveSink(EventSink);
			events::plugin::kClearData.RemoveSink(EventSink);
			events::plugin::kConstructSpecialForms.RemoveSink(EventSink);

			ColorMaskManager->Deinitialize();
			MouseInputManager->Deinitialize();
			KeyboardInputManager->Deinitialize();
			LayerManager->Deinitialize();
			GroupManager->Deinitialize();
			CellLists->Deinitialize();
			GizmoManager->Deinitialize(OSD);
			OSD->Deinitialize();
			SceneGraphManager->RemoveModifier(&ReferenceParentChildIndicator::Instance);
			SceneGraphManager->RemoveModifier(&ReferenceVisibilityModifier::Instance);

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

		ReferenceColorMaskManager* RenderWindowManager::GetColorMaskManager() const
		{
			SME_ASSERT(Initialized);
			return ColorMaskManager;
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
			const TESObjectREFRArrayT& ActiveRefs = GetActiveRefs();
			std::unordered_set<std::string> ActiveGroups;

			char FilterBuffer[0x200] = { 0 };
			char Label[0x100] = { 0 };

			for (const auto& Itr : ActiveRefs)
			{
				std::string EditorID(IRenderWindowOSDLayer::Helpers::GetRefEditorID(Itr));
				UInt32 FormID = Itr->formID;
				const char* FormType = TESForm::GetFormTypeIDLongName(Itr->baseForm->formType);

				switch (Type)
				{
				case kActiveRefCollection_Invisible:
				case kActiveRefCollection_Frozen:
					{
						bool EnumeratingInvisibleRefs = Type == kActiveRefCollection_Invisible;
						FORMAT_STR(FilterBuffer, "%s %08X %s", EditorID.c_str(), FormID, FormType);
						if (FilterHelper.PassFilter(FilterBuffer) == false)
							return;

						bool CanEnumerate = false;
						UInt32 InvisibleReasons = 0;
						if (EnumeratingInvisibleRefs)
							CanEnumerate = ReferenceVisibilityManager::ShouldBeInvisible(Itr, InvisibleReasons) && InvisibleReasons != ReferenceVisibilityManager::kReason_ParentLayerInvisible;
						else
							CanEnumerate = Itr->IsFrozen();

						if (!CanEnumerate)
							break;

						FORMAT_STR(Label, "%08X-%d", FormID, Type);
						ImGui::PushID(Label);
						{
							ImGui::TableNextRow();
							{
								ImGui::TableNextColumn();
								{
									if (ImGui::Selectable(EditorID.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick) &&
										ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
									{
										_TES->LoadCellIntoViewPort(nullptr, Itr);
									}

									if (ImGui::BeginPopupContextItem(EnumeratingInvisibleRefs ? "Invisible_Popup" : "Frozen_Popup"))
									{
										if (EnumeratingInvisibleRefs)
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
								ImGui::TableNextColumn();
								{
									char FormIDBuffer[0x10] = { 0 };
									FORMAT_STR(FormIDBuffer, "%08X", FormID);
									ImGui::Selectable(FormIDBuffer);
								}
								ImGui::TableNextColumn();
								{
									ImGui::Selectable(FormType);
								}
								if (EnumeratingInvisibleRefs)
								{
									ImGui::TableNextColumn();
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

										ImGui::Selectable(FlagsBuffer.c_str());
									}
								}
							}
						}
						ImGui::PopID();
					}

					break;
				case kActiveRefCollection_Groups:
					{
						const char* ParentGroup = GroupManager->GetParentGroupID(Itr);
						if (ParentGroup)
							ActiveGroups.emplace(ParentGroup);
					}

					break;
				}
			}

			if (Type != kActiveRefCollection_Groups)
				return;
			else if (ActiveGroups.empty())
				return;

			TESObjectREFRArrayT GroupMembers;

			bool Dissolved = false;
			for (const auto& Itr : ActiveGroups)
			{
				if (!GroupManager->GetGroupData(Itr.c_str(), GroupMembers))
					continue;

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
					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
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
										std::bind(&RenderModalEditGroupMembers,
											std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
										UserData, NULL, ImVec2(500, 500), ImGuiCond_Appearing);
								}

								ImGui::EndPopup();
							}
						}
						if (!Dissolved)
						{
							ImGui::TableNextColumn();

							char CountBuffer[0x100] = { 0 };
							std::string MemberList;
							int Count = 0;
							static constexpr int kMaxDiplayCount = 10;

							for (const auto& Member : GroupMembers)
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
							ImGui::Selectable(CountBuffer);
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(MemberList.c_str());
						}
					}
				}
				ImGui::PopID();
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
				// buttons
				switch (Type)
				{
				case kActiveRefCollection_Invisible:
				{
					if (ImGui::Button(ICON_MD_VISIBILITY_OFF "##hide_selection_btn"))
						actions::ToggleSelectionVisibility();
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Toggle Selection Visibility");

					ImGui::SameLine();
					if (ImGui::Button(ICON_MD_FLIP_TO_FRONT "##reveal_all_btn"))
						actions::RevealAll();
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Reveal All");

					ImGui::SameLine(0, 20);
					ImGui::TextWrapped("Invisible References");
					ImGui::SameLine(0, 3);
					ImGui::ShowHelpPopup("Excluding references whose parent layer is hidden.\n\nRight click on an item to display the context menu."
										"\n\nReason Flags:\n\tDS - Initially Disabled\n\tDC - Child of Initially Disabled Parent\n\tIS - Invisible\n\tIC - Child of Parent with \"Invisible Children\" Flag");

					break;
				}
				case kActiveRefCollection_Frozen:
				{
					const char* FreezeInactiveCaption = nullptr;
					const char* FreezeInactiveToolTip = nullptr;
					if (_RENDERWIN_XSTATE.FreezeInactiveRefs)
					{
						FreezeInactiveCaption = ICON_MD_LOCK " " ICON_MD_STAR " ##freeze_inactive_refs";
						FreezeInactiveToolTip = "Inactive References Frozen";

						ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
					}
					else
					{
						FreezeInactiveCaption = ICON_MD_LOCK_OPEN " " ICON_MD_STAR "##freeze_inactive_refs";
						FreezeInactiveToolTip = "Inactive References not Frozen";

						ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));
					}

					if (ImGui::Button(FreezeInactiveCaption, ImVec2(45, 0)))
						_RENDERWIN_XSTATE.FreezeInactiveRefs = _RENDERWIN_XSTATE.FreezeInactiveRefs == false;

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip(FreezeInactiveToolTip);

					ImGui::PopStyleColor(3);
					ImGui::SameLine();

					if (ImGui::Button(ICON_MD_LOCK "##freeze_selection_btn"))
						actions::FreezeSelection();
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Freeze Selection");

					ImGui::SameLine();
					if (ImGui::Button(ICON_MD_LOCK_OPEN "##thaw_all_btn"))
						actions::ThawAll();
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Thaw All");

					ImGui::SameLine(0, 20);
					ImGui::TextWrapped("Frozen References");
					ImGui::SameLine(0, 3);
					ImGui::ShowHelpPopup("Excluding references whose parent layer is frozen and those that are frozen with the \"Freeze Inactive Refs\" tool.\n\nRight click on an item to display the context menu.");

					break;
				}
				case kActiveRefCollection_Groups:
				{
					if (ImGui::Button(ICON_MD_GROUP_WORK "##group_selection"))
						actions::GroupSelection();
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Group Current Selection");

					ImGui::SameLine(0, 20);
					ImGui::TextWrapped("Reference Groups");
					ImGui::SameLine(0, 3);
					ImGui::ShowHelpPopup("Double click on an item to select the group.\nRight click on an item to display the context menu.\nHover the cursor over the \"Count\" column to view the first 10 members of the group.");

					break;
				}
				}

				const char* TableName = nullptr;
				int NumColumns = 0;
				switch (Type)
				{
				case kActiveRefCollection_Invisible:
					TableName = "##invisble_refs_table";
					NumColumns = 4;
					break;
				case kActiveRefCollection_Frozen:
					TableName = "##frozen_refs_table";
					NumColumns = 3;
					break;
				case kActiveRefCollection_Groups:
					TableName = "##grouped_refs_table";
					NumColumns = 2;
					break;
				}

				if (ImGui::BeginTable(TableName, NumColumns,
					ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
					ImVec2(0, 0)))
				{
					switch (Type)
					{
					case kActiveRefCollection_Invisible:
						ImGui::TableSetupColumn("Editor ID", ImGuiTableColumnFlags_WidthFixed, 125);
						ImGui::TableSetupColumn("Form ID", ImGuiTableColumnFlags_WidthFixed, 65);
						ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 75);
						ImGui::TableSetupColumn("Reason", ImGuiTableColumnFlags_WidthFixed, 50);

						break;
					case kActiveRefCollection_Frozen:
						ImGui::TableSetupColumn("Editor ID", ImGuiTableColumnFlags_WidthFixed, 150);
						ImGui::TableSetupColumn("Form ID", ImGuiTableColumnFlags_WidthFixed, 65);
						ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100);

						break;
					case kActiveRefCollection_Groups:
						ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 250);
						ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 50);

						break;
					}

					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 2));
					ImGui::TableHeadersRow();
					ImGui::PopStyleVar();

					RenderActiveRefCollectionRefTable(Type, CurrentFilter);

					ImGui::EndTable();
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

			int Count = Data->MemberList.size();
			if (Count <= 1)
				Count = 0;

			ImGui::TextWrapped("Group: %s\tMember Count: %d", Data->GroupName.c_str(), Count);
			FilterHelper.Draw();

			ImGui::BeginChild("##subwindow", ImVec2(0, 0));
			{
				if (Data->MemberList.size() <= 1)
					ImGui::TextDisabled("No Members - This group has been dissolved.");
				else
				{
					if (ImGui::BeginTable("##group_member_table", 4,
						ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders,
						ImVec2(0, -35)))
					{
						ImGui::TableSetupColumn("Editor ID", ImGuiTableColumnFlags_WidthFixed, 200);
						ImGui::TableSetupColumn("Form ID", ImGuiTableColumnFlags_WidthFixed, 75);
						ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 85);
						ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 85);

						ImGui::TableSetupScrollFreeze(0, 1);
						ImGui::TableHeadersRow();

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
								ImGui::TableNextRow();
								{
									ImGui::TableNextColumn();
									{
										ImGui::Text(EditorID.c_str());
									}
									ImGui::TableNextColumn();
									{
										char FormIDBuffer[0x10] = { 0 };
										FORMAT_STR(FormIDBuffer, "%08X", FormID);
										ImGui::Text(FormIDBuffer);
									}
									ImGui::TableNextColumn();
									{
										ImGui::Text(Type);
									}
									ImGui::TableNextColumn();
									{
										if (ImGui::Button("Remove##remove_button", ImVec2(-FLT_MIN, 0)))
										{
											if (_RENDERWIN_MGR.GetGroupManager()->Orphanize(*Member))
											{
												ToRemove = Member;
												Removed = true;
											}
										}
									}
								}
							}
							ImGui::PopID();

							if (Removed)
								break;

							Count++;
						}

						// remove from the modal's cache
						if (Removed)
							Data->MemberList.erase(ToRemove);

						ImGui::EndTable();
					}
				}

				if (ImGui::Button("Close", ImVec2(75, 0)))
					Close = true;
			}
			ImGui::EndChild();

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

			ExtendedState->MeasureBaseRuler = nullptr;
			ExtendedState->MeasureBaseCircle = nullptr;
		}

		void RenderWindowManager::HandleConstructSpecialForms()
		{
			SME_ASSERT(ExtendedState->MeasureBaseRuler == nullptr);
			SME_ASSERT(ExtendedState->MeasureBaseCircle == nullptr);

			ExtendedState->MeasureBaseRuler = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Static), TESForm, TESObjectSTAT);
			ExtendedState->MeasureBaseRuler->SetFormID(0x200);
			ExtendedState->MeasureBaseRuler->SetEditorID("zzCSEMeasureRuler");
			ExtendedState->MeasureBaseRuler->modelPath.Set("Meshes\\CSE\\MeasureRuler.nif");
			_DATAHANDLER->AddTESObject(ExtendedState->MeasureBaseRuler);
			ExtendedState->MeasureBaseRuler->SetFromActiveFile(false);
			ExtendedState->MeasureBaseRuler->MarkAsTemporary();


			ExtendedState->MeasureBaseCircle = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Static), TESForm, TESObjectSTAT);
			ExtendedState->MeasureBaseCircle->SetFormID(0x201);
			ExtendedState->MeasureBaseCircle->SetEditorID("zzCSEMeasureCircle");
			ExtendedState->MeasureBaseCircle->modelPath.Set("Meshes\\CSE\\MeasureCircle.nif");
			_DATAHANDLER->AddTESObject(ExtendedState->MeasureBaseCircle);
			ExtendedState->MeasureBaseCircle->SetFromActiveFile(false);
			ExtendedState->MeasureBaseCircle->MarkAsTemporary();
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

			_RENDERWIN_XSTATE.Initialize();
			_RENDERWIN_MGR.RefreshFOV();
		}

		void Deinitialize(void)
		{
			_RENDERWIN_MGR.Deinitialize();
		}

	}
}