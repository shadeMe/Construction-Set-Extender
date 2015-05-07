#include "TESRender.h"
#include "Core.h"
#include "CSEPathGridUndoManager.h"

using namespace ConstructionSetExtender;

NiDX9Renderer**						TESRender::NiRendererSingleton = (NiDX9Renderer**)0x00A0F87C;
TESRender::PrimaryRenderer**		TESRender::PrimaryRenderer::Singleton = (TESRender::PrimaryRenderer**)0x00A0BACC;
TESRender::Scenegraph**				TESRender::Scenegraph::Singleton = (TESRender::Scenegraph**)0x00A0B634;
NiNode**							TESRender::PathGridSceneRoot = (NiNode**)0x00A1358C;

HWND*								TESRenderWindow::WindowHandle = (HWND*)0x00A0AF28;
TESRenderSelection**				TESRenderWindow::ClipboardSelection = (TESRenderSelection**)0x00A0AF64;
TESRenderWindow::UndoStack**		TESRenderWindow::UndoBuffer = (TESRenderWindow::UndoStack**)0x00A0B124;
TESRenderWindow::RubberBandSelection**
									TESRenderWindow::RubberBandSelector = (TESRenderWindow::RubberBandSelection**)0x00A0BC48;
TESRender::PickData*				TESRenderWindow::PickBuffer = (TESRender::PickData*)0x00A0BC64;

PathGridPointListT*					TESRenderWindow::SelectedPathGridPoints = (PathGridPointListT*)0x00A0AF68;

float*								TESRenderWindow::CameraPanSpeed = (float*)0x00A0B090;
float*								TESRenderWindow::CameraZoomSpeed = (float*)0x00A0B088;
float*								TESRenderWindow::CameraRotationSpeed = (float*)0x00A0B080;
float*								TESRenderWindow::SnapGridDistance = (float*)0x00A0B060;
float*								TESRenderWindow::SnapAngle = (float*)0x00A0B068;
float*								TESRenderWindow::RefRotationSpeed = (float*)0x00A0B070;
float*								TESRenderWindow::RefMovementSpeed = (float*)0x00A0B078;

TESLandTexture**					TESRenderWindow::ActiveLandscapeTexture = (TESLandTexture**)0x00A0B0C0;
TESObjectCELL**						TESRenderWindow::ActiveCell = (TESObjectCELL**)0x00A0BC3C;

UInt8*								TESRenderWindow::LandscapeEditFlag = (UInt8*)0x00A0BC35;
UInt8*								TESRenderWindow::PathGridEditFlag = (UInt8*)0x00A0BC5C;
UInt8*								TESRenderWindow::RefreshFlag = (UInt8*)0x00A0BC4D;
UInt32*								TESRenderWindow::StateFlags = (UInt32*)0x00A0B058;

HCURSOR*							TESRenderWindow::CursorMove = (HCURSOR*)0x00A0BAB4;
HCURSOR*							TESRenderWindow::CursorSelect = (HCURSOR*)0x00A0BAB0;
HCURSOR*							TESRenderWindow::CursorArrow = (HCURSOR*)0x00A0BABC;

POINT								TESRenderWindow::CurrentMouseLBDragCoordDelta = { 0, 0 };
bool								TESRenderWindow::UseAlternateMovementSettings = false;
bool								TESRenderWindow::FreezeInactiveRefs = false;
NiFrustum							TESRenderWindow::CameraFrustumBuffer = {0};
POINT								TESRenderWindow::CurrentMouseCoord = {0};
TESObjectREFR*						TESRenderWindow::CurrentMouseRef = NULL;
TESPathGridPoint*					TESRenderWindow::CurrentMousePathGridPoint = NULL;
bool								TESRenderWindow::ShowInitiallyDisabledRefs = true;
bool								TESRenderWindow::ShowInitiallyDisabledRefChildren = true;

const float							TESRenderWindow::MaxLandscapeEditBrushRadius = 25.0f;

TESPreviewControl::PreviewControlListT*		TESPreviewControl::ActivePreviewControls = (TESPreviewControl::PreviewControlListT*)0x00A0BE90;

void TESRenderWindow::Reset()
{
	UInt8 ObjWndState = *TESObjectWindow::Initialized, CellWndState = *TESCellViewWindow::MainMenuState;

	*TESObjectWindow::Initialized = 0;
	*TESCellViewWindow::MainMenuState = 0;

	SendMessage(*TESRenderWindow::WindowHandle, 0x419, 6, 1);
	SendMessage(*TESRenderWindow::WindowHandle, 0x419, 5, 0);
	InvalidateRect(*TESRenderWindow::WindowHandle, 0, 1);

	TESCSMain::DeinitializeCSWindows();
	TESCSMain::InitializeCSWindows();

	*TESObjectWindow::Initialized = ObjWndState;
	*TESCellViewWindow::MainMenuState = CellWndState;
}

void TESRenderWindow::Redraw( bool RefreshPathGrid )
{
	if (RefreshPathGrid && *PathGridEditFlag)
	{
		CSEPathGridUndoManager::Instance.SetCanReset(false);

		SendMessage(*TESRenderWindow::WindowHandle, 0x419, NULL, NULL);
		TogglePathGridEditMode();
		SendMessage(*TESRenderWindow::WindowHandle, 0x419, 2, NULL);
		TogglePathGridEditMode();

		CSEPathGridUndoManager::Instance.SetCanReset(true);
	}
	else
		*RefreshFlag = 1;
}

void TESRenderWindow::TogglePathGridEditMode()
{
	cdeclCall<void>(0x00550660);
}

void TESRenderWindow::Refresh3D()
{
	SendMessage(*WindowHandle, WM_KEYDOWN, VK_F5, NULL);
}

UInt32 TESRenderWindow::GetActiveCellObjects(CellObjectListT& OutList, CellObjectListVisitor Visitor)
{
	OutList.clear();

	if (_TES->currentInteriorCell)
	{
		for (TESObjectCELL::ObjectREFRList::Iterator Itr = _TES->currentInteriorCell->objectList.Begin(); !Itr.End(); ++Itr)
		{
			TESObjectREFR* Ref = Itr.Get();
			if (Ref)
			{
				if (Visitor == NULL || Visitor(Ref) == true)
					OutList.push_back(Ref);
			}
		}
	}
	else
	{
		GridCellArray* CellGrid = _TES->gridCellArray;

		for (int i = 0; i < CellGrid->size; i++)
		{
			for (int j = 0; j < CellGrid->size; j++)
			{
				GridCellArray::GridEntry* Data = CellGrid->GetCellEntry(i, j);
				if (Data && Data->cell)
				{
					for (TESObjectCELL::ObjectREFRList::Iterator Itr = Data->cell->objectList.Begin(); !Itr.End(); ++Itr)
					{
						TESObjectREFR* Ref = Itr.Get();
						if (Ref)
						{
							if (Visitor == NULL || Visitor(Ref) == true)
								OutList.push_back(Ref);
						}
					}
				}
			}
		}
	}

	return OutList.size();
}

bool TESRenderWindow::GetCellInActiveGrid(TESObjectCELL* Cell)
{
	if (_TES->currentInteriorCell)
		return Cell == _TES->currentInteriorCell;
	else
	{
		GridCellArray* CellGrid = _TES->gridCellArray;

		for (int i = 0; i < CellGrid->size; i++)
		{
			for (int j = 0; j < CellGrid->size; j++)
			{
				GridCellArray::GridEntry* Data = CellGrid->GetCellEntry(i, j);
				if (Data && Data->cell)
				{
					if (Data->cell == Cell)
						return true;
				}
			}
		}

		return false;
	}
}

void TESRenderWindow::UndoStack::RecordReference( UInt32 Operation, TESRenderSelection::SelectedObjectsEntry* Selection )
{
	thisCall<UInt32>(0x00432D40, this, Operation, Selection);
}

void TESRenderWindow::UndoStack::Clear( void )
{
	thisCall<void>(0x00433440, this);
}

void TESRender::PrimaryRenderer::RenderNode( NiCamera* Camera /*= NULL*/, NiNode* NodeToRender /*= NULL*/, BSRenderedTexture* RenderToTexture /*= NULL*/ )
{
	if (Camera == NULL)
		Camera = primaryCamera;

	niWindow->Render(Camera, NodeToRender, RenderToTexture);
}

void TESRender::PrimaryRenderer::GetCameraPivot( Vector3* OutPivot, float ScaleFactor )
{
	NiVector3* CameraPos = &primaryCameraParentNode->m_localTranslate;

	OutPivot->x = CameraPos->x;
	OutPivot->y = CameraPos->y;
	OutPivot->z = CameraPos->z;

	Vector3 Buffer(*OutPivot);
	Buffer.Scale(ScaleFactor);

	*OutPivot += Buffer;
}

bool TESRender::UpdateNode( NiNode* Node, UInt32 UpdateType, float Multiplier )
{
	bool Result = cdeclCall<bool>(0x00430080, Node, UpdateType, Multiplier);

	if (Result)
		UpdateAVObject(Node);

	return Result;
}

void TESRender::UpdateAVObject( NiAVObject* Object )
{
	thisCall<void>(0x006F25E0, Object, 0.0, true);		// NiAVObject::Update
}

void TESRender::RotateNode( NiNode* Node, Vector3* Pivot, int XOffset, int YOffset, float SpeedMultiplier )
{
	cdeclCall<void>(0x00430420, Node, Pivot, XOffset, YOffset, SpeedMultiplier);
}

void TESRender::SetCameraFOV( NiCamera* Camera, float FOV, float Width /*= -1*/, float Height /*= -1*/ )
{
	NiFrustum ViewFrustum = {0};

	ViewFrustum.f = Camera->m_kViewFrustum.f;
	ViewFrustum.n = Camera->m_kViewFrustum.n;

	RECT Bounds = {0};
	GetWindowRect(*TESRenderWindow::WindowHandle, &Bounds);

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

TESObjectREFR* TESRender::PickRefAtCoords( int X, int Y )
{
	return cdeclCall<TESObjectREFR*>(0x00426BB0, X, Y);
}

NiNode* TESRender::CreateNiNode()
{
	NiNode* New = (NiNode*)FormHeap_Allocate(sizeof(NiNode));
	thisCall<void>(0x006F5830, New, 0);
	return New;
}

void TESRender::AddToNiNode(NiNode* To, NiAVObject* Child)
{
	thisVirtualCall<void>(0x84, To, Child, 0);
}

void TESRender::DeleteNiRefObject(NiRefObject* Object)
{
	thisVirtualCall<void>(0x0, Object, true);
}

bool TESRender::RemoveFromNiNode(NiNode* From, NiAVObject* Child)
{
	NiAVObject* Out = NULL;
	thisVirtualCall<void>(0x88, From, &Out, Child);
	if (Out)
	{
		SME_ASSERT(Out == Child);
		SME_ASSERT(Out->m_uiRefCount > 0);
		if (InterlockedDecrement(&Out->m_uiRefCount) == 0)
		{
			DeleteNiRefObject(Out);
			return true;
		}
	}

	return false;
}

ShadowSceneNode* TESRender::GetSceneGraphRoot()
{
	return cdeclCall<ShadowSceneNode*>(0x007662E0, 0);
}

void TESRender::AddProperty(NiAVObject* To, NiProperty* Property, bool InitializeState /*= true*/)
{
	thisCall<void>(0x00411190, To, Property);
	if (InitializeState)
		thisCall<void>(0x006F28A0, To);
}

NiProperty* TESRender::GetProperty(NiAVObject* In, UInt16 ID)
{
	return thisCall<NiProperty*>(0x006F27C0, In, ID);
}

TESPathGridPoint* TESRender::PickPathGridPointAtCoords(int X, int Y)
{
	if (*TESRenderWindow::PathGridEditFlag == 0 || *PathGridSceneRoot == NULL)
		return NULL;

	TESPathGridPoint* Result = NULL;
	Vector3 Arg1(0, 0, 0), Arg2(Arg1);

	TESRenderWindow::PickBuffer->SetRoot(*PathGridSceneRoot);
	thisCall<void>(0x006FF1A0, _PRIMARYRENDERER->primaryCamera, X, Y, &Arg1, &Arg2);
	if (TESRenderWindow::PickBuffer->PerformPick(&Arg1, &Arg2))
	{
		if (TESRenderWindow::PickBuffer->pickRecords.numObjs > 0)
		{
			NiAVObject* Pick = TESRenderWindow::PickBuffer->pickRecords.data[0]->picked;
			if (Pick && Pick->m_parent)
			{
				if (Pick->m_parent->m_extraDataListLen)
				{
					NiIntegerExtraData* xData = NI_CAST(Pick->m_parent->m_extraDataList[0], NiIntegerExtraData);
					if (xData)
						Result = (TESPathGridPoint*)xData->m_iValue;
				}
			}
		}
	}

	return Result;
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

void TESPreviewControl::UpdatePreviewWindows( bool RefreshRenderWindow /*= true*/ )
{
	cdeclCall<void>(0x00446F30, RefreshRenderWindow);
}

TESPreviewControl* TESPreviewControl::CreatePreviewControl( HWND Dialog, const TESRenderControl::Parameters* Params )
{
	return cdeclCall<TESPreviewControl*>(0x00446440, Dialog, Params);
}

void TESPreviewControl::RemovePreviewNode(NiNode* Node)
{
	thisCall<void>(0x0044D960, this, Node);
}

void TESPreviewControl::RemoveGroundPlane()
{
	thisCall<void>(0x0044BCF0, this);
}

void TESRender::PickData::SetRoot(NiNode* To)
{
	thisCall<void>(0x00417C40, this, To);
}

bool TESRender::PickData::PerformPick(Vector3* Arg1, Vector3* Arg2, bool KeepExisting /*= false*/)
{
	return thisCall<bool>(0x005E6030, this, Arg1, Arg2, KeepExisting);
}