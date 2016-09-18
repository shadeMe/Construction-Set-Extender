#include "TESRender.h"
#include "Core.h"
#include "Render Window\RenderWindowManager.h"

using namespace cse;
using namespace cse::renderWindow;

NiDX9Renderer**						TESRender::NiRendererSingleton = (NiDX9Renderer**)0x00A0F87C;
TESRender::PrimaryRenderer**		TESRender::PrimaryRenderer::Singleton = (TESRender::PrimaryRenderer**)0x00A0BACC;
TESRender::Scenegraph**				TESRender::Scenegraph::Singleton = (TESRender::Scenegraph**)0x00A0B634;
NiNode**							TESRender::PathGridSceneRoot = (NiNode**)0x00A1358C;

HWND*								TESRenderWindow::WindowHandle = (HWND*)0x00A0AF28;
int*								TESRenderWindow::ScreeWidth = (int*)0x00A0F870;
int*								TESRenderWindow::ScreeHeight = (int*)0x00A0F874;
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
TESObjectREFR**						TESRenderWindow::SnapReference = (TESObjectREFR**)0x00A0B050;

UInt8*								TESRenderWindow::LandscapeEditFlag = (UInt8*)0x00A0BC35;
UInt8*								TESRenderWindow::DraggingSelection = (UInt8*)0x00A0BBF6;
UInt8*								TESRenderWindow::RotatingSelection = (UInt8*)0x00A0BBF5;

UInt8*								TESRenderWindow::KeyState_Shift = (UInt8*)0x00A0BC5D;
UInt8*								TESRenderWindow::KeyState_Control = (UInt8*)0x00A0BC5E;
UInt8*								TESRenderWindow::KeyState_SpaceMMB = (UInt8*)0x00A0BC5F;
UInt8*								TESRenderWindow::KeyState_X = (UInt8*)0x00A0BC1F;
UInt8*								TESRenderWindow::KeyState_Y = (UInt8*)0x00A0BC1E;
UInt8*								TESRenderWindow::KeyState_Z = (UInt8*)0x00A0BC1D;
UInt8*								TESRenderWindow::KeyState_V = (UInt8*)0x00A0BC1C;
UInt8*								TESRenderWindow::KeyState_S = (UInt8*)0x00A0BC20;
POINT*								TESRenderWindow::LastMouseCoords = (POINT*)0x00A0BC08;


UInt8*								TESRenderWindow::PathGridEditFlag = (UInt8*)0x00A0BC5C;
UInt8*								TESRenderWindow::RefreshFlag = (UInt8*)0x00A0BC4D;
UInt32*								TESRenderWindow::StateFlags = (UInt32*)0x00A0B058;
UInt8*								TESRenderWindow::NonLandObjectsHiddenFlag = (UInt8*)0x00A0BBF9;
UInt8*								TESRenderWindow::LandscapeHiddenFlag = (UInt8*)0x00A0BBFA;
UInt8*								TESRenderWindow::WaterHiddenFlag = (UInt8*)0x00A0BBFB;
UInt8*								TESRenderWindow::FullBrightLightingFlag = (UInt8*)0x000A8E696;
UInt8*								TESRenderWindow::SkyFlag = (UInt8*)0x00A0B629;
UInt8*								TESRenderWindow::LightRadiusFlag = (UInt8*)0x00A0B12C;

HCURSOR*							TESRenderWindow::CursorMove = (HCURSOR*)0x00A0BAB4;
HCURSOR*							TESRenderWindow::CursorSelect = (HCURSOR*)0x00A0BAB0;
HCURSOR*							TESRenderWindow::CursorArrow = (HCURSOR*)0x00A0BABC;

TESPreviewControl::PreviewControlListT*		TESPreviewControl::ActivePreviewControls = (TESPreviewControl::PreviewControlListT*)0x00A0BE90;
BSTreeManager**						BSTreeManager::Singleton = (BSTreeManager**)0x00A25608;

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
		_RENDERWIN_MGR.GetPathGridUndoManager()->SetCanReset(false);

		SendMessage(*TESRenderWindow::WindowHandle, 0x419, NULL, NULL);
		TogglePathGridEditMode();
		SendMessage(*TESRenderWindow::WindowHandle, 0x419, 2, NULL);
		TogglePathGridEditMode();

		_RENDERWIN_MGR.GetPathGridUndoManager()->SetCanReset(true);
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

UInt32 TESRenderWindow::GetActiveCellObjects(TESObjectREFRArrayT& OutList, CellObjectListVisitorT Visitor)
{
	OutList.clear();

	if (_TES->currentInteriorCell)
	{
		for (TESObjectCELL::ObjectREFRList::Iterator Itr = _TES->currentInteriorCell->objectList.Begin(); !Itr.End(); ++Itr)
		{
			TESObjectREFR* Ref = Itr.Get();
			if (Ref && Ref->IsTemporary() == false)
			{
				if (!Visitor || Visitor(Ref) == true)
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
						if (Ref && Ref->IsTemporary() == false)
						{
							if (!Visitor || Visitor(Ref) == true)
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

std::string TESRenderWindow::GetCellGeomDescription(TESObjectCELL* Cell)
{
	char Buffer[0x200] = {0};
	cdeclCall<void>(0x0053D0F0, Cell, Buffer);
	return Buffer;
}

Vector3* TESRenderWindow::CalculatePathGridPointPositionVectorSum(Vector3& OutPosVecSum)
{
	cdeclCall<void>(0x00426920, &OutPosVecSum, SelectedPathGridPoints);
	return &OutPosVecSum;
}

void TESRenderWindow::UndoStack::RecordReference(UInt32 Operation, TESRenderSelection::SelectedObjectsEntry* Selection)
{
	thisCall<UInt32>(0x00432D40, this, Operation, Selection);
}

void TESRenderWindow::UndoStack::Clear( void )
{
	thisCall<void>(0x00433440, this);
}

void TESRender::PrimaryRenderer::RenderNode( NiCamera* Camera /*= NULL*/, NiNode* NodeToRender /*= NULL*/, BSRenderedTexture* RenderToTexture /*= NULL*/ )
{
	if (Camera == nullptr)
		Camera = primaryCamera;

	niWindow->Render(Camera, NodeToRender, RenderToTexture);
}

void TESRender::PrimaryRenderer::GetCameraPivot( Vector3* OutPivot, float ScaleFactor )
{
	Vector3* CameraRootLocalTranslate = (Vector3*)&primaryCameraParentNode->m_localTranslate;
	NiMatrix33* CameraRootWorldRotate = &primaryCameraParentNode->m_worldRotate;

	Vector3 Offset(CameraRootWorldRotate->data[1], CameraRootWorldRotate->data[4], CameraRootWorldRotate->data[7]);
	Offset.Scale(ScaleFactor);

	*OutPivot = *CameraRootLocalTranslate;
	*OutPivot += Offset;
}

const NiVector3* TESRender::PrimaryRenderer::GetCameraWorldTranslate()
{
	return &primaryCamera->m_worldTranslate;
}

void TESRender::PrimaryRenderer::MoveReferenceSelection(int XOffset, int YOffset, bool AxisX, bool AxisY, bool AxisZ)
{
	cdeclCall<void>(0x00425670, this, XOffset, YOffset, AxisX, AxisY, AxisZ);
}

void TESRender::PrimaryRenderer::RotateReferenceSelection(int Offset, bool AxisX, bool AxisY, bool AxisZ)
{
	int Axis = 0;
	if (AxisX)
		Axis = 1;
	else if (AxisY)
		Axis = 3;		// yeah, 3
	else if (AxisZ)
		Axis = 2;

	SME_ASSERT(Axis);
	cdeclCall<void>(0x00425D60, Offset, Axis);
}

void TESRender::PrimaryRenderer::ScaleReferenceSelection(int Offset, bool Global)
{
	cdeclCall<void>(0x00424650, Offset, Global);
}

bool TESRender::UpdateNode(NiAVObject* Node, UInt32 UpdateType, float Multiplier, bool UpdateOnSuccess)
{
	bool Result = cdeclCall<bool>(0x00430080, Node, UpdateType, Multiplier);

	if (Result && UpdateOnSuccess)
		UpdateAVObject(Node);

	return Result;
}

void TESRender::UpdateAVObject( NiAVObject* Object )
{
	thisCall<void>(0x006F25E0, Object, 0.f, (UInt32)1);		// NiAVObject::Update
}

void TESRender::UpdateDynamicEffectState(NiAVObject* Object)
{
	thisCall<void>(0x006F2C10, Object);
}

void TESRender::RotateNode(NiAVObject* Node, Vector3* Pivot, int XOffset, int YOffset, float SpeedMultiplier)
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
	NiAVObject* Out = nullptr;
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

NiProperty* TESRender::CreateProperty(UInt8 Type)
{
	bool InvalidType = false;
	switch (Type)
	{
	case NiVertexColorProperty::kType:
		{
			NiVertexColorProperty* VertexColor = (NiVertexColorProperty*)FormHeap_Allocate(sizeof(NiVertexColorProperty));
			thisCall<void>(0x00410C50, VertexColor);
			VertexColor->m_uiRefCount++;
			return VertexColor;
		}
	case NiWireframeProperty::kType:
		{
			NiWireframeProperty* Wireframe = (NiWireframeProperty*)FormHeap_Allocate(sizeof(NiWireframeProperty));
			thisCall<void>(0x00417BE0, Wireframe);
			Wireframe->m_uiRefCount++;
			return Wireframe;
		}
	case NiMaterialProperty::kType:
		{
			NiMaterialProperty* Mat = (NiMaterialProperty*)FormHeap_Allocate(sizeof(NiMaterialProperty));
			thisCall<void>(0x00704060, Mat);
			Mat->m_uiRefCount++;
			return Mat;
		}
	case NiStencilProperty::kType:
		{
			NiStencilProperty* Stencil = (NiStencilProperty*)FormHeap_Allocate(sizeof(NiStencilProperty));
			thisCall<void>(0x00410BE0, Stencil);
			Stencil->m_uiRefCount++;
			return Stencil;
		}
	default:
		SME_ASSERT(InvalidType);
		return nullptr;
	}
}

NiSourceTexture* TESRender::CreateSourceTexture(const char* FilePath)
{
	return cdeclCall<NiSourceTexture*>(0x006F8410, FilePath, 0x00A00900, 1);
}

NiCamera* TESRender::CreateCamera()
{
	NiCamera* NewCam = (NiCamera*)FormHeap_Allocate(0x124);
	thisCall<void>(0x006FF430, NewCam);
	return NewCam;
}

TESPathGridPoint* TESRender::PickPathGridPointAtCoords(int X, int Y)
{
	if (*TESRenderWindow::PathGridEditFlag == 0 || *PathGridSceneRoot == nullptr)
		return nullptr;

	TESPathGridPoint* Result = nullptr;
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

TESSceneNodeDebugData* TESSceneNodeDebugData::Initialize(HINSTANCE Instance,
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

