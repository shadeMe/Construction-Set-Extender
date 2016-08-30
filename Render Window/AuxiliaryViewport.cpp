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
	SME::INI::INISetting					AuxiliaryViewport::kINI_Top("Top", AUXVIEWPORT_INISECTION,
																		"Dialog Rect Top",
																		(SInt32)150);
	SME::INI::INISetting					AuxiliaryViewport::kINI_Left("Left", AUXVIEWPORT_INISECTION,
																		"Dialog Rect Left",
																		(SInt32)150);
	SME::INI::INISetting					AuxiliaryViewport::kINI_Right("Right", AUXVIEWPORT_INISECTION,
																		"Dialog Rect Right",
																		(SInt32)150);
	SME::INI::INISetting					AuxiliaryViewport::kINI_Bottom("Bottom", AUXVIEWPORT_INISECTION,
																		"Dialog Rect Bottom",
																		(SInt32)150);
	SME::INI::INISetting					AuxiliaryViewport::kINI_Visible("Visible", AUXVIEWPORT_INISECTION,
																		"Dialog Visibility State",
																		(SInt32)1);

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
		Return = false;

		switch (uMsg)
		{
		case WM_SIZE:
			{
				AuxiliaryViewport* Instance = dynamic_cast<AuxiliaryViewport*>(UserData->Instance);
				if (Instance->IsFrozen() == false)
				{
					RECT CurrentSize = { 0 };
					GetClientRect(hWnd, &CurrentSize);

					HWND Placeholder = GetDlgItem(hWnd, IDC_AUXVIEWPORT_PLACEHOLDER);
					SetWindowPos(Placeholder, HWND_TOP, 0, 0, CurrentSize.right, CurrentSize.bottom, 0);
					BGSEEUI->GetInvalidationManager()->Redraw(hWnd);
				}
			}

			break;
		}

		return DlgProcResult;
	}

	AuxiliaryViewport::AuxiliaryViewport() :
		bgsee::GenericModelessDialog(BGSEEUI->GetMainWindow(),
													BGSEEMAIN->GetExtenderHandle(),
													IDD_AUXVIEWPORT,
													IDR_BGSEE_GENERIC_CONTEXTMENU,
													BaseDlgProc),
		ViewportCamera(nullptr),
		Frozen(false),
		Rendering(false)
	{
		ViewportCamera = (NiCamera*)FormHeap_Allocate(0x124);
		thisCall<void>(0x006FF430, ViewportCamera);		// ctor
		ViewportCamera->m_uiRefCount++;
	}

	AuxiliaryViewport::~AuxiliaryViewport()
	{
		ViewportCamera->m_uiRefCount--;
		thisVirtualCall<void>(0x0, ViewportCamera, true);
		INISaveUIState(&kINI_Top, &kINI_Left, &kINI_Right, &kINI_Bottom, &kINI_Visible);

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
	}

	void AuxiliaryViewport::ClearScreen()
	{
		InvalidateRect(DialogHandle, nullptr, TRUE);
	}

	void AuxiliaryViewport::Redraw()
	{
		TESRenderWindow::Redraw();
	}

	void AuxiliaryViewport::SyncViewportCamera( NiCamera* Camera )
	{
		UInt32 RefCountBuffer = ViewportCamera->m_uiRefCount;

		memcpy(ViewportCamera, Camera, sizeof(NiCamera));
		ViewportCamera->m_uiRefCount = RefCountBuffer;
		ViewportCamera->m_parent = nullptr;
	}

	bool AuxiliaryViewport::ToggleFrozenState()
	{
		if (IsFrozen())
			Frozen = false;
		else
			Frozen = true;

		return Frozen;
	}

	void AuxiliaryViewport::Draw( NiNode* NodeToRender, NiCamera* Camera )
	{
		SME::MiscGunk::ScopedSetter<bool> Sentry(Rendering, true);

		hooks::_MemHdlr(NiDX9RendererPresent).WriteUInt16(0x9090);
		if (Camera == nullptr)
			Camera = ViewportCamera;

		_PRIMARYRENDERER->RenderNode(Camera, NodeToRender);
		hooks::_MemHdlr(NiDX9RendererPresent).WriteBuffer();

		DrawBackBuffer();
	}

	void AuxiliaryViewport::DrawBackBuffer( void )
	{
		_NIRENDERER->device->Present(nullptr, nullptr, DialogHandle, nullptr);
	}

	bool AuxiliaryViewport::IsFrozen() const
	{
		return Frozen;
	}

	void AuxiliaryViewport::RegisterINISettings( bgsee::INISettingDepotT& Depot )
	{
		Depot.push_back(&kINI_Top);
		Depot.push_back(&kINI_Left);
		Depot.push_back(&kINI_Right);
		Depot.push_back(&kINI_Bottom);
		Depot.push_back(&kINI_Visible);
	}

	bool AuxiliaryViewport::IsRenderingPerspective() const
	{
		return Rendering;
	}

}