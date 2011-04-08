#pragma once 
#include "Common.h"

// hooks that do stuff to the renderer and the render window
void PatchRendererHooks(void);

_DeclareMemHdlr(DoorMarkerProperties, "allows the displaying of reference properties for door markers");
_DeclareMemHdlr(TESObjectREFRGet3DData, "selectively culls reference nodes depending on the presence of various visibiity flags");
_DeclareMemHdlr(NiWindowRender, "allows various obscenities to be written to the render window");
_DeclareMemHdlr(NiDX9RendererRecreate, "used to release any D3D resources");
_DeclareMemHdlr(RenderWindowStats, "displays the stats of selected refs/other info in the render window");
_DeclareMemHdlr(UpdateViewport, "updates the render window viewport regardless of user activity");
_DeclareMemHdlr(RenderWindowSelection, "allows groups of references to be selected at a time");
_DeclareMemHdlr(TESRenderControlPerformMove, "allows the freezing of references in the render window");
_DeclareMemHdlr(TESRenderControlPerformRotate, "");
_DeclareMemHdlr(TESRenderControlPerformScale, "");
_DeclareMemHdlr(TESRenderControlPerformFall, "");
