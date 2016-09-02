#include "AuxiliaryViewport.h"
#include "Construction Set Extender_Resource.h"
#include "Hooks\Hooks-Renderer.h"
#include "Hooks\Hooks-Dialog.h"

#include <bgsee\Main.h>
#include <bgsee\BGSEditorExtenderBase_Resource.h>

namespace cse
{
	AuxiliaryViewport*			AuxiliaryViewport::Singleton = nullptr;
	const char*					AuxiliaryViewport::kWindowTitle = "Auxiliary Viewport Window";

#define AUXVIEWPORT_INISECTION				"AuxViewport"
	INISetting					AuxiliaryViewport::kINI_Top("Top", AUXVIEWPORT_INISECTION, "Dialog Rect Top", (SInt32)150);
	INISetting					AuxiliaryViewport::kINI_Left("Left", AUXVIEWPORT_INISECTION, "Dialog Rect Left", (SInt32)150);
	INISetting					AuxiliaryViewport::kINI_Right("Right", AUXVIEWPORT_INISECTION, "Dialog Rect Right", (SInt32)150);
	INISetting					AuxiliaryViewport::kINI_Bottom("Bottom", AUXVIEWPORT_INISECTION, "Dialog Rect Bottom", (SInt32)150);
	INISetting					AuxiliaryViewport::kINI_Visible("Visible", AUXVIEWPORT_INISECTION, "Dialog Visibility State", (SInt32)1);
	INISetting					AuxiliaryViewport::kINI_PanSpeed("PanSpeed", AUXVIEWPORT_INISECTION, "Auxiliary viewport camera pan speed", (float)5.0);
	INISetting					AuxiliaryViewport::kINI_ZoomSpeed("ZoomSpeed", AUXVIEWPORT_INISECTION, "Auxiliary viewport camera zoom speed", (float)0.5);
	INISetting					AuxiliaryViewport::kINI_RotationSpeed("RotationSpeed", AUXVIEWPORT_INISECTION, "Auxiliary viewport rotation speed", (float)1.0);

	AuxiliaryViewport::GlobalEventSink::GlobalEventSink(AuxiliaryViewport* Parent) :
		SME::MiscGunk::IEventSink(),
		Parent(Parent)
	{
		SME_ASSERT(Parent);
	}

	void AuxiliaryViewport::GlobalEventSink::Handle(SME::MiscGunk::IEventData* Data)
	{
		const events::TypedEventSource* Source = dynamic_cast<const events::TypedEventSource*>(Data->Source);
		SME_ASSERT(Source);

		switch (Source->GetTypeID())
		{
		case events::TypedEventSource::kType_CellView_SelectCell:
			Parent->SyncWithPrimaryCamera();
			break;
		}
	}

	AuxiliaryViewport* AuxiliaryViewport::GetSingleton()
	{
		if (Singleton == nullptr)
			Singleton = new AuxiliaryViewport();

		return Singleton;
	}

	LRESULT CALLBACK AuxiliaryViewport::BaseDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return )
	{
		LRESULT DlgProcResult = FALSE;
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		AuxiliaryViewport* Instance = dynamic_cast<AuxiliaryViewport*>(UserData->Instance);
		Return = false;

		bool ShiftDown = GetAsyncKeyState(VK_SHIFT);
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
			Return = true;
			Instance->BeginMouseCapture(hWnd, Instance->RotatingInPlace);

			break;
		case WM_LBUTTONUP:
			Return = true;
			Instance->EndMouseCapture(Instance->RotatingInPlace);

			break;
		case WM_MBUTTONDOWN:
			Return = true;
			Instance->BeginMouseCapture(hWnd, Instance->Panning);

			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_TAB:
				// sync with render window camera
				Return = true;
				Instance->SyncWithPrimaryCamera();

				break;
			case VK_SHIFT:
				Return = true;
				Instance->BeginMouseCapture(hWnd, Instance->Rotating);

				break;
			case VK_SPACE:
				Return = true;
				Instance->BeginMouseCapture(hWnd, Instance->Panning);

				break;
			case 'V':
				Return = true;
				Instance->BeginMouseCapture(hWnd, Instance->Zooming);

				break;
			case VK_LEFT:
				TESRender::UpdateNode(Instance->CameraRoot,
									  ShiftDown ? TESRender::kNodeUpdate_Unk08 : TESRender::kNodeUpdate_Unk09,
									  ShiftDown ? -0.1 : -2048.f);
				Instance->Redraw();
				break;
			case VK_RIGHT:
				TESRender::UpdateNode(Instance->CameraRoot,
									  ShiftDown ? TESRender::kNodeUpdate_Unk08 : TESRender::kNodeUpdate_Unk09,
									  ShiftDown ? 0.1 : 2048.f);
				Instance->Redraw();
				break;
			case VK_UP:
				TESRender::UpdateNode(Instance->CameraRoot,
									  ShiftDown ? TESRender::kNodeUpdate_Unk07 : TESRender::kNodeUpdate_Unk10,
									  ShiftDown ? 0.1 : 2048.f);
				Instance->Redraw();
				break;
			case VK_DOWN:
				TESRender::UpdateNode(Instance->CameraRoot,
									  ShiftDown ? TESRender::kNodeUpdate_Unk07 : TESRender::kNodeUpdate_Unk10,
									  ShiftDown ? -0.1 : -2048.f);
				Instance->Redraw();
				break;
			}

			break;
		case WM_MBUTTONUP:
			Return = true;
			Instance->EndMouseCapture(Instance->Panning);

			break;
		case WM_KEYUP:
			switch (wParam)
			{
			case VK_SHIFT:
				Return = true;
				Instance->EndMouseCapture(Instance->Rotating);

				break;
			case VK_SPACE:
				Return = true;
				Instance->EndMouseCapture(Instance->Panning);

				break;
			case 'V':
				Return = true;
				Instance->EndMouseCapture(Instance->Zooming);

				break;
			}

			break;
		case WM_MOUSEMOVE:
			{
				POINT CurrentCoords = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				POINT MouseDelta = { Instance->LastMouseCoords.x - CurrentCoords.x, Instance->LastMouseCoords.y - CurrentCoords.y };
				Instance->LastMouseCoords.x = CurrentCoords.x;
				Instance->LastMouseCoords.y = CurrentCoords.y;

				if (Instance->Panning || Instance->Rotating || Instance->RotatingInPlace || Instance->Zooming)
				{
					Return = true;
					NiMatrix33* CameraRootWorldRotate = &Instance->CameraRoot->m_worldRotate;
					Vector3* CameraRootLocalTranslate = (Vector3*)&Instance->CameraRoot->m_localTranslate;

					if (Instance->Rotating || Instance->RotatingInPlace)
					{
						// calculate the pivot as a point in front of the current camera pos when not rotating in place
						Vector3 Pivot;

						if (Instance->RotatingInPlace == false)
						{
							Vector3 Offset(CameraRootWorldRotate->data[1], CameraRootWorldRotate->data[4], CameraRootWorldRotate->data[7]);
							Offset.Scale(-1000.f);

							Pivot = *CameraRootLocalTranslate;
							Pivot += Offset;

						}
						else
							Pivot.x = Pivot.y = Pivot.z = (float)3.4028235e38;

						TESRender::RotateNode(Instance->CameraRoot,
											  &Pivot,
											  MouseDelta.x,
											  MouseDelta.y,
											  kINI_RotationSpeed().f);
					}
					else if (Instance->Zooming)
					{
						float ZoomMul = kINI_ZoomSpeed().f * MouseDelta.y * 10.f;
						TESRender::UpdateNode(Instance->CameraRoot, TESRender::kNodeUpdate_Unk04, ZoomMul);
					}
					else
					{
						float PanMul = kINI_PanSpeed().f;

						TESRender::UpdateNode(Instance->CameraRoot, TESRender::kNodeUpdate_Unk02, PanMul * -MouseDelta.x);
						TESRender::UpdateNode(Instance->CameraRoot, TESRender::kNodeUpdate_Unk00, PanMul * -MouseDelta.y);
					}

					Instance->Redraw();
				}
			}

			break;
		case WM_MOUSEWHEEL:
			{
				Return = true;
				float ZoomMul = kINI_ZoomSpeed().f * GET_WHEEL_DELTA_WPARAM(wParam) * 0.5;
				TESRender::UpdateNode(Instance->CameraRoot, TESRender::kNodeUpdate_Unk04, ZoomMul);
				Instance->Redraw();
			}

			break;
		case WM_SIZE:
			Instance->Redraw();
			break;
		}

		return DlgProcResult;
	}
	void AuxiliaryViewport::BeginMouseCapture(HWND hWnd, bool& StateFlag)
	{
		if (StateFlag == false)
		{
			StateFlag = true;
			if (GetCapture() == NULL)
				SetCapture(hWnd);
		}
	}

	void AuxiliaryViewport::EndMouseCapture(bool& StateFlag)
	{
		if (StateFlag)
		{
			StateFlag = false;
			ReleaseCapture();
		}
	}

	AuxiliaryViewport::AuxiliaryViewport() :
		bgsee::GenericModelessDialog(BGSEEUI->GetMainWindow(),
													BGSEEMAIN->GetExtenderHandle(),
													IDD_AUXVIEWPORT,
													IDR_BGSEE_GENERIC_CONTEXTMENU,
													BaseDlgProc),
		CameraRoot(nullptr),
		ViewportCamera(nullptr),
		Panning(false),
		Zooming(false),
		Rotating(false),
		RotatingInPlace(false),
		LastMouseCoords{ 0 },
		Rendering(false),
		EventSink(nullptr)
	{
		CameraRoot = TESRender::CreateNiNode();
		CameraRoot->m_uiRefCount++;
		ViewportCamera = TESRender::CreateCamera();
		TESRender::AddToNiNode(CameraRoot, ViewportCamera);
		TESRender::UpdateAVObject(CameraRoot);
		EventSink = new GlobalEventSink(this);

		events::dialog::cellView::kSelectCell.AddSink(EventSink);
	}

	AuxiliaryViewport::~AuxiliaryViewport()
	{
		// ### the viewport camera's ref count increments by one for some reason
		// ### could be that the renderer caches the last used camera somewhere
		// ### we'll just play nice and keep from freeing the pointer (this happens at deinit anyway, so it's pretty moot)
		bool Freed = TESRender::RemoveFromNiNode(CameraRoot, ViewportCamera);

		SME_ASSERT(CameraRoot->m_uiRefCount == 1);
		CameraRoot->m_uiRefCount--;
		TESRender::DeleteNiRefObject(CameraRoot);

		INISaveUIState(&kINI_Top, &kINI_Left, &kINI_Right, &kINI_Bottom, &kINI_Visible);

		events::dialog::cellView::kSelectCell.RemoveSink(EventSink);
		SAFEDELETE(EventSink);

		Singleton = nullptr;
	}

	void AuxiliaryViewport::Initialize()
	{
		if (settings::dialogs::kShowMainWindowsInTaskbar.GetData().i)
		{
			bgsee::WindowStyler::StyleData RegularAppWindow = {0};
			RegularAppWindow.Extended = WS_EX_APPWINDOW;
			RegularAppWindow.ExtendedOp = bgsee::WindowStyler::StyleData::kOperation_OR;

			BGSEEUI->GetWindowStyler()->RegisterStyle(IDD_AUXVIEWPORT, RegularAppWindow);
		}

		Create(NULL, true, true);
		INILoadUIState(&kINI_Top, &kINI_Left, &kINI_Right, &kINI_Bottom, &kINI_Visible);

		ClearScreen();
		SyncWithPrimaryCamera();
	}

	void AuxiliaryViewport::ClearScreen()
	{
		InvalidateRect(DialogHandle, nullptr, TRUE);
	}

	void AuxiliaryViewport::Redraw()
	{
		TESRenderWindow::Redraw();
	}

	void AuxiliaryViewport::SetCameraFOV( float FOV )
	{
		TESRender::SetCameraFOV(ViewportCamera, FOV);
	}

	void AuxiliaryViewport::Draw( NiNode* NodeToRender, NiCamera* Camera )
	{
		if (IsVisible() == false)
			return;

		SME::MiscGunk::ScopedSetter<bool> Sentry(Rendering, true);

		hooks::_MemHdlr(NiDX9RendererPresent).WriteUInt16(0x9090);
		if (Camera == nullptr)
			Camera = ViewportCamera;

		_PRIMARYRENDERER->RenderNode(Camera, NodeToRender);
		hooks::_MemHdlr(NiDX9RendererPresent).WriteBuffer();

		DrawBackBuffer();
	}

	void AuxiliaryViewport::DrawBackBuffer(void)
	{
		if (IsVisible() == false)
			return;

		_NIRENDERER->device->Present(nullptr, nullptr, DialogHandle, nullptr);
	}

	void AuxiliaryViewport::RegisterINISettings( bgsee::INISettingDepotT& Depot )
	{
		Depot.push_back(&kINI_Top);
		Depot.push_back(&kINI_Left);
		Depot.push_back(&kINI_Right);
		Depot.push_back(&kINI_Bottom);
		Depot.push_back(&kINI_Visible);
		Depot.push_back(&kINI_PanSpeed);
		Depot.push_back(&kINI_ZoomSpeed);
		Depot.push_back(&kINI_RotationSpeed);
	}

	bool AuxiliaryViewport::IsRenderingPerspective() const
	{
		return Rendering;
	}

	void AuxiliaryViewport::SyncWithPrimaryCamera()
	{
		UInt32 RefCountBuffer = ViewportCamera->m_uiRefCount;
		NiAVObject* ParentBuffer = ViewportCamera->m_parent;

		memcpy(ViewportCamera, _PRIMARYRENDERER->primaryCamera, sizeof(NiCamera));

		ViewportCamera->m_uiRefCount = RefCountBuffer;
		ViewportCamera->m_parent = ParentBuffer;

		memcpy(&CameraRoot->m_localRotate, &_PRIMARYRENDERER->primaryCameraParentNode->m_localRotate, sizeof(NiMatrix33));
		memcpy(&CameraRoot->m_worldRotate, &_PRIMARYRENDERER->primaryCameraParentNode->m_worldRotate, sizeof(NiMatrix33));

		memcpy(&CameraRoot->m_localTranslate, &_PRIMARYRENDERER->primaryCameraParentNode->m_localTranslate, sizeof(NiVector3));
		memcpy(&CameraRoot->m_worldTranslate, &_PRIMARYRENDERER->primaryCameraParentNode->m_worldTranslate, sizeof(NiVector3));

		Redraw();
	}

	bool AuxiliaryViewport::ToggleVisibility()
	{
		bool NewState = GenericModelessDialog::ToggleVisibility();
		// update viewport camera on showing
		if (NewState)
			SyncWithPrimaryCamera();

		return NewState;
	}

}