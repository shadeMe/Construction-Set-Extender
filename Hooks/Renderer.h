#pragma once
#include "Common.h"

namespace Hooks
{
	extern TESForm*								g_TESObjectREFRUpdate3DBuffer;
	extern bool									g_RenderWindowAltMovementSettings;	

	// hooks that do stuff to the renderer and the render window
	void PatchRendererHooks(void);

	_DeclareMemHdlr(DoorMarkerProperties, "allows the displaying of reference properties for door markers");
	_DeclareMemHdlr(TESObjectREFRGet3DData, "selectively culls reference nodes depending on the presence of various visibility flags");
	_DeclareMemHdlr(NiWindowRender, "allows various obscenities to be written to the render window");
	_DeclareMemHdlr(NiDX9RendererRecreateA, "used to release any renderer owned D3D resources");
	_DeclareMemHdlr(NiDX9RendererRecreateB, "");
	_DeclareMemHdlr(NiDX9RendererRecreateC, "");
	_DeclareMemHdlr(UpdateViewport, "updates the render window viewport regardless of user activity");
	_DeclareMemHdlr(RenderWindowAddToSelection, "allows groups of references to be selected at a time");
	_DeclareMemHdlr(TESRenderControlPerformMove, "allows the freezing of references in the render window");
	_DeclareMemHdlr(TESRenderControlPerformRotate, "");
	_DeclareMemHdlr(TESRenderControlPerformScale, "");
	_DeclareMemHdlr(TESRenderControlPerformFall, "");
	_DeclareMemHdlr(TESObjectREFRSetupDialog, "flag temp nodes to prevent them from being unculled unnecessarily");
	_DeclareMemHdlr(TESObjectREFRCleanDialog, "");
	_DeclareMemHdlr(TESRenderControlPerformFallVoid, "fixes a bug that causes a CTD when performing the fall operation under certain conditions");
	_DeclareMemHdlr(TESObjectREFRUpdate3D, "temporary hook that allows the updating of ref nodes on demand");
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
	_DeclareMemHdlr(RenderToAuxiliaryViewport, "allows the scenegraph to be rendered to the auxiliary viewport window");
	_DeclareMemHdlr(ActivateRenderWindowPostCellSwitch, "resets control focus to the render window after switching between exterior cells");
	_DeclareMemHdlr(TESRenderControlPerformRelativeScale, "allows the selection group to be scaled relative to their nominal center");

	bool TXTChannelStaticHandler_RenderSelectionInfo(std::string& RenderedText);
}