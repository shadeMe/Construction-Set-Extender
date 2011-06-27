#pragma once
#include "Common.h"

namespace Hooks
{
	extern TESForm*				g_TESObjectREFRUpdate3DBuffer;
	extern bool					g_RenderWindowAltMovementSettings;

	// hooks that do stuff to the renderer and the render window
	void PatchRendererHooks(void);

	_DeclareMemHdlr(DoorMarkerProperties, "allows the displaying of reference properties for door markers");
	_DeclareMemHdlr(TESObjectREFRGet3DData, "selectively culls reference nodes depending on the presence of various visibility flags");
	_DeclareMemHdlr(NiWindowRender, "allows various obscenities to be written to the render window");
	_DeclareMemHdlr(NiDX9RendererRecreate, "used to release any D3D resources");
	_DeclareMemHdlr(RenderWindowStats, "displays the stats of selected refs/other info in the render window");
	_DeclareMemHdlr(UpdateViewport, "updates the render window viewport regardless of user activity");
	_DeclareMemHdlr(RenderWindowSelection, "allows groups of references to be selected at a time");
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
}
