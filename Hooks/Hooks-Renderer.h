#pragma once
#include "Hooks-Common.h"

namespace cse
{
	namespace hooks
	{
		// hooks that do stuff to the renderer and the render window
		void PatchRendererHooks(void);

		_DeclareMemHdlr(DoorMarkerProperties, "allows the displaying of reference properties for door markers");
		_DeclareMemHdlr(TESObjectREFRSetupDialog, "flag temp nodes to prevent them from being unculled unnecessarily");
		_DeclareMemHdlr(TESObjectREFRCleanDialog, "");
		_DeclareMemHdlr(TESRenderControlPerformFallVoid, "fixes a bug that causes a CTD when performing the fall operation under certain conditions");
		_DeclareMemHdlr(ForceShowTESObjectREFRDialog, "prevents reference properties dialogs from being moved behind the render window");
		_DeclareMemHdlr(TESRenderControlAltSnapGrid, "allows two different camera/ref movement settings to be toggled during runtime");
		_DeclareMemHdlr(TESRenderControlAltRefMovementSpeedA, "");
		_DeclareMemHdlr(TESRenderControlAltRefMovementSpeedB, "");
		_DeclareMemHdlr(TESRenderControlAltRefMovementSpeedC, "");
		_DeclareMemHdlr(TESRenderControlAltRefRotationSpeed, "");
		_DeclareMemHdlr(TESRenderControlAltRefSnapAngle, "");
		_DeclareMemHdlr(TESRenderControlAltCamRotationSpeed, "");
		_DeclareMemHdlr(TESRenderControlAltCamZoomSpeedA, "");
		_DeclareMemHdlr(TESRenderControlAltCamZoomSpeedB, "");
		_DeclareMemHdlr(TESRenderControlAltCamPanSpeedA, "");
		_DeclareMemHdlr(TESRenderControlAltCamPanSpeedB, "");
		_DeclareMemHdlr(TESRenderControlRedrawGrid, "fixes a bug that invalidates the viewport after linking a path grid point");
		_DeclareMemHdlr(TESPreviewControlCallWndProc, "fixes a bug that causes a CTD when preview controls are themed");
		_DeclareMemHdlr(ActivateRenderWindowPostLandTextureChange, "resets control focus to the render window after the active landscape texture has been changed");
		_DeclareMemHdlr(TESPathGridRecordOperationMoveA, "adds support for the path grid editor undo manager");
		_DeclareMemHdlr(TESPathGridRecordOperationMoveB, "");
		_DeclareMemHdlr(TESPathGridRecordOperationLink, "");
		_DeclareMemHdlr(TESPathGridRecordOperationFlag, "");
		_DeclareMemHdlr(TESPathGridRecordOperationRef, "");
		_DeclareMemHdlr(TESPathGridDeletePoint, "");
		_DeclareMemHdlr(TESPathGridPointDtor, "");
		_DeclareMemHdlr(TESPathGridToggleEditMode, "");
		_DeclareMemHdlr(TESPathGridCreateNewLinkedPoint, "");
		_DeclareMemHdlr(TESPathGridPerformFall, "");
		_DeclareMemHdlr(TESPathGridShowMultipleSelectionRing, "fixes a bug that basically prevents selection rings from showing on path grid points selected using the rubber band scope");
		_DeclareMemHdlr(TESPathGridDtor, "fixes a bug that caused a CTD when a path grid point was created after every loaded pathgrid was destroyed");
		_DeclareMemHdlr(InitialCellLoadCameraPosition, "moves the camera to the origin on loading an interior cell into the viewport");
		_DeclareMemHdlr(LandscapeEditBrushRadius, "increases the upper limit placed on the landscape edit brush radius");
		_DeclareMemHdlr(ConvertNiRenderedTexToD3DBaseTex, "temporary hook that adds the ability to convert NiAPI rendered textures to D3D equivalents");
		_DeclareMemHdlr(DuplicateReferences, "changes the render window selection to the newly created references after duplication");
		_DeclareMemHdlr(NiDX9RendererPresent, "temporary hook that prevents the renderer from presenting its render targets");
		_DeclareMemHdlr(TESRenderControlPerformRelativeScale, "allows the selection group to be scaled relative to their nominal center");
		_DeclareMemHdlr(DataHandlerClosePlugins, "prevents the view port from flashing while loading exterior cells");
		_DeclareMemHdlr(TESPathGridRubberBandSelection, "fixes a bug that selected all path grid points in the loaded cell(s) when restoring focus to the render window");
		_DeclareMemHdlr(CoplanarRefDrop, "allows new references to be dropped coplanar to the object under the drop location");
		_DeclareMemHdlr(InitPathGridNodeSelectionRing, "fixes a bug that causes selection rings to persist b'ween sessions of the path grid edit mode");
		_DeclareMemHdlr(TESLoadIntoViewPort, "fixes a bug that causes a CTD when a detached reference (with no parent cell) was loaded into the render window");
		_DeclareMemHdlr(RenderWindowAxisHotkeysMoveReferences, "fixes a bug that caused the x/y axis key states to be swapped when moving/rotating references and path grid points");
		_DeclareMemHdlr(RenderWindowAxisHotkeysMovePathGridPoints, "");
		_DeclareMemHdlr(RenderWindowAxisHotkeysRotateReferences, "");
		_DeclareMemHdlr(BSFadeNodeDrawTransparency, "allows objects to be rendered with arbitrary transparency");
		_DeclareMemHdlr(CellViewSetCurrentCell, "temporary hook to prevent the cell window from gaining focus during render window exterior cell switches");
		_DeclareMemHdlr(RenderWindowCursorSwap, "prevents the cursor icon changes from screwing themselves up");
		_DeclareMemHdlr(RenderWindowCopySelection, "prevents the selection's bounding box from being cleared after a copy operation");
		_DeclareMemHdlr(TESPathGridPointGenerateNiNodeA, "allows linked ref indicators to be toggled");
		_DeclareMemHdlr(TESPathGridPointGenerateNiNodeB, "");
		_DeclareMemHdlr(TESPathGridGenerateNiNode, "");
		_DeclareMemHdlr(CenterCameraOnRefSelection, "adds a sanity check for non-reference forms in the selection when centering or 'topping' the camera");
		_DeclareMemHdlr(TopCameraOnRefSelection, "");
		_DeclareMemHdlr(TESRenderRotateSelectionWorldA, "fixes a bug that prevented the rotation of multiple refs in their local axes");
		_DeclareMemHdlr(TESRenderRotateSelectionWorldB, "");
		_DeclareMemHdlr(RotateCameraDrag, "prevent the shift key from rotating the camera when lasso'ing, and add support for a custom camera pivot");
		_DeclareMemHdlr(RubberbandSelectionDragDeselectRefs, "allow ref drag deselection with the Shift key");
		_DeclareMemHdlr(LandscapeTextureLoad, "adds support for landscape texture swaps");
		_DeclareMemHdlr(PatchGetAsyncKeyStateA, "adds support for custom hotkeys");
		_DeclareMemHdlr(PatchGetAsyncKeyStateB, "");
		_DeclareMemHdlr(UndoStackUndoOp3, "fixes a bug that caused a CTD when trying to redo/undo an operation");
		_DeclareMemHdlr(UndoStackRedoOp3, "");
		_DeclareMemHdlr(MoveSelectionClampMul, "prevent refs from being moved so far that it causes a FP overflow");

		void __stdcall RenderWindowReferenceSelectionDetour(TESObjectREFR* Ref, bool ShowSelectionBox);
	}
}