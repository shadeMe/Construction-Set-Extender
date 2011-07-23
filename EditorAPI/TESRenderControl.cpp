#include "TESRenderControl.h"
#include "Core.h"
#include "ElapsedTimeCounter.h"

ElapsedTimeCounter					g_RenderWindowTimeManager;
UInt32*								g_RenderWindowStateFlags = (UInt32*)0x00A0B058;
TESRenderSelection**				g_TESRenderSelectionPrimary = (TESRenderSelection**)0x00A0AF60;
TESRenderUndoStack**				g_TESRenderUndoStack = (TESRenderUndoStack**)0x00A0B124;
UInt8*								g_RenderWindowUpdateViewPortFlag = (UInt8*)0x00A0BC4D;
TESLandTexture**					g_ActiveLandscapeEditTexture = (TESLandTexture**)0x00A0B0C0;
float*								g_RenderWindowRefMovementSpeed = (float*)0x00A0B078;
float*								g_RenderWindowSnapGridDistance = (float*)0x00A0B060;
float*								g_RenderWindowRefRotationSpeed = (float*)0x00A0B070;
float*								g_RenderWindowSnapAngle = (float*)0x00A0B068;
float*								g_RenderWindowCameraRotationSpeed = (float*)0x00A0B080;
float*								g_RenderWindowCameraZoomSpeed = (float*)0x00A0B088;
float*								g_RenderWindowCameraPanSpeed = (float*)0x00A0B090;
UInt8*								g_RenderWindowPathGridEditModeFlag = (UInt8*)0x00A0BC5C;
tList<TESPathGridPoint>*			g_RenderWindowSelectedPathGridPoints = (tList<TESPathGridPoint>*)0x00A0AF68;

void TESRenderSelection::AddToSelection( TESForm* Form, bool AddSelectionBox )
{
	thisCall<UInt32>(0x00512730, this, Form, AddSelectionBox);
}

void TESRenderSelection::RemoveFromSelection( TESForm* Form, bool RemoveSelectionBox )
{
	thisCall<UInt32>(0x00512830, this, Form, RemoveSelectionBox);
}

void TESRenderSelection::ClearSelection( bool RemoveSelectionBox )
{
	thisCall<UInt32>(0x00511C20, this, RemoveSelectionBox);
}

TESRenderSelection* TESRenderSelection::CreateInstance()
{
	TESRenderSelection* NewInstance = (TESRenderSelection*)FormHeap_Allocate(sizeof(TESRenderSelection));
	thisCall<UInt32>(0x00511A20, NewInstance);
	return NewInstance;
}

void TESRenderSelection::DeleteInstance()
{
	this->ClearSelection(false);
	FormHeap_Free(this);
}

void TESRenderUndoStack::RecordReference( UInt32 Operation, TESRenderSelection* Selection )
{
	thisCall<UInt32>(0x00432D40, this, Operation, Selection);
}