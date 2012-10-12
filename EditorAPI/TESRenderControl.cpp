#include "TESRenderControl.h"
#include "Core.h"

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
TESRenderComponents**				g_TESRenderComponents = (TESRenderComponents**)0x00A0BACC;
TESObjectCELL**						g_RenderWindowCurrentlyLoadedCell = (TESObjectCELL**)0x00A0BC3C;
tList<TESRenderControl>*			g_ActivePreviewControls = (tList<TESRenderControl>*)0x00A0BE90;

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

TESRenderSelection* TESRenderSelection::CreateInstance( TESRenderSelection* Source )
{
	TESRenderSelection* NewInstance = (TESRenderSelection*)FormHeap_Allocate(sizeof(TESRenderSelection));
	thisCall<UInt32>(0x00511A20, NewInstance);

	if (Source && Source->selectionCount)
	{
		for (SelectedObjectsEntry* Itr = Source->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			NewInstance->AddToSelection(Itr->Data);
	}

	return NewInstance;
}

void TESRenderSelection::DeleteInstance()
{
	this->ClearSelection(false);
	FormHeap_Free(this);
}

void TESRenderSelection::CalculatePositionVectorSum( void )
{
	thisCall<void>(0x00511A70, this);
}

bool TESRenderSelection::HasObject( TESForm* Form )
{
	return thisCall<bool>(0x00511CC0, this, Form);
}

void TESRenderUndoStack::RecordReference( UInt32 Operation, TESRenderSelection::SelectedObjectsEntry* Selection )
{
	thisCall<UInt32>(0x00432D40, this, Operation, Selection);
}

void TESRenderUndoStack::Clear( void )
{
	thisCall<void>(0x00433440, this);
}

void TESRenderComponents::RenderNode( NiCamera* Camera /*= NULL*/, NiNode* NodeToRender /*= NULL*/, BSRenderedTexture* RenderToTexture /*= NULL*/ )
{
	if (Camera == NULL)
		Camera = primaryCamera;

	thisVirtualCall<void>(0x0, niWindow, Camera, NodeToRender, RenderToTexture);
}

void TESRenderComponents::GetCameraPivot( Vector3* OutPivot, float ScaleFactor )
{
	NiVector3* CameraPos = &primaryCameraParentNode->m_localTranslate;

	OutPivot->x = CameraPos->x;
	OutPivot->y = CameraPos->y;
	OutPivot->z = CameraPos->z;

	Vector3 Buffer(*OutPivot);
	Buffer.Scale(ScaleFactor);

	*OutPivot += Buffer;
}

bool TESRenderComponents::UpdateNode( NiNode* Node, UInt32 UpdateType, float Multiplier )
{
	bool Result = cdeclCall<bool>(0x00430080, Node, UpdateType, Multiplier);

	if (Result)
		UpdateAVObject(Node);

	return Result;
}

void TESRenderComponents::UpdateAVObject( NiAVObject* Object )
{
	thisCall<void>(0x006F25E0, Object, 0.0, true);		// NiAVObject::Update
}

void TESRenderComponents::RotateNode( NiNode* Node, Vector3* Pivot, int XOffset, int YOffset, float SpeedMultiplier )
{
	cdeclCall<void>(0x00430420, Node, Pivot, XOffset, YOffset, SpeedMultiplier);
}

void TESRenderComponents::SetCameraFOV( NiCamera* Camera, float FOV, float Width /*= -1*/, float Height /*= -1*/ )
{
	NiFrustum ViewFrustum = {0};

	ViewFrustum.f = Camera->m_kViewFrustum.f;
	ViewFrustum.n = Camera->m_kViewFrustum.n;

	RECT Bounds = {0};
	GetWindowRect(*g_HWND_RenderWindow, &Bounds);

	if (Width == -1)
		Width = (Bounds.right - Bounds.left) * 1.0f;

	if (Height == -1)
		Height = (Bounds.bottom - Bounds.top) * 1.0f;

	float SizeRatio = Height / Width * 1.0f;
	float FOVMultiplier = FOV * 0.01745329238474369 * 0.5;

	ViewFrustum.l = -tan(FOVMultiplier);
	ViewFrustum.r = tan(FOVMultiplier);
	ViewFrustum.b = ViewFrustum.l * SizeRatio;
	ViewFrustum.t = SizeRatio * ViewFrustum.r;

	thisCall<void>(0x006FE030, Camera, &ViewFrustum);		// NiCamera::SetViewFrustum
	UpdateAVObject(Camera);
}

TESSceneNodeDebugData* TESSceneNodeDebugData::Initialize( HINSTANCE Instance,
																	HWND Parent,
																	NiNode* Node, 
																	const char* WindowTitle, 
																	int X, int Y,
																	int Width, int Height )
{
	return thisCall<TESSceneNodeDebugData*>(0x004B7640, this, Instance, Parent, Node, WindowTitle, X, Y, Width, Height);
}
