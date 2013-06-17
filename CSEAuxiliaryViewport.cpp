#include "CSEAuxiliaryViewport.h"
#include "Construction Set Extender_Resource.h"
#include "CSERenderWindowPainter.h"
#include "Hooks\Hooks-Renderer.h"
#include "Hooks\Hooks-Dialog.h"

#include <BGSEEMain.h>
#include <BGSEditorExtenderBase_Resource.h>

namespace ConstructionSetExtender
{
	CSEAuxiliaryViewport*		CSEAuxiliaryViewport::Singleton = NULL;
	const char*					CSEAuxiliaryViewport::kWindowTitle = "Auxiliary Viewport Window";

#define AUXVIEWPORT_INISECTION				"AuxViewport"
	SME::INI::INISetting					CSEAuxiliaryViewport::kINI_Top("Top", AUXVIEWPORT_INISECTION,
																		"Dialog Rect Top",
																		(SInt32)150);
	SME::INI::INISetting					CSEAuxiliaryViewport::kINI_Left("Left", AUXVIEWPORT_INISECTION,
																		"Dialog Rect Left",
																		(SInt32)150);
	SME::INI::INISetting					CSEAuxiliaryViewport::kINI_Right("Right", AUXVIEWPORT_INISECTION,
																		"Dialog Rect Right",
																		(SInt32)150);
	SME::INI::INISetting					CSEAuxiliaryViewport::kINI_Bottom("Bottom", AUXVIEWPORT_INISECTION,
																		"Dialog Rect Bottom",
																		(SInt32)150);
	SME::INI::INISetting					CSEAuxiliaryViewport::kINI_Visible("Visible", AUXVIEWPORT_INISECTION,
																		"Dialog Visibility State",
																		(SInt32)1);

	CSEAuxiliaryViewport* CSEAuxiliaryViewport::GetSingleton()
	{
		if (Singleton == NULL)
			Singleton = new CSEAuxiliaryViewport();

		return Singleton;
	}

	LRESULT CALLBACK CSEAuxiliaryViewport::BaseDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return )
	{
		LRESULT DlgProcResult = FALSE;
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		Return = false;

		switch (uMsg)
		{
		case WM_MOVING:
		case WM_SHOWWINDOW:
			{
				CSEAuxiliaryViewport* Instance = dynamic_cast<CSEAuxiliaryViewport*>(UserData->Instance);
				Instance->Redraw();
			}

			break;
		}

		return DlgProcResult;
	}

	CSEAuxiliaryViewport::CSEAuxiliaryViewport() :
		BGSEditorExtender::BGSEEGenericModelessDialog(BGSEEUI->GetMainWindow(),
													BGSEEMAIN->GetExtenderHandle(),
													IDD_AUXVIEWPORT,
													IDR_BGSEE_GENERIC_CONTEXTMENU,
													BaseDlgProc),
		ViewportCamera(NULL),
		Frozen(false)
	{
		ViewportCamera = (NiCamera*)FormHeap_Allocate(0x124);
		thisCall<void>(0x006FF430, ViewportCamera);		// ctor
		ViewportCamera->m_uiRefCount++;
	}

	CSEAuxiliaryViewport::~CSEAuxiliaryViewport()
	{
		ViewportCamera->m_uiRefCount--;
		thisVirtualCall<void>(0x0, ViewportCamera, true);
		INISaveUIState(&kINI_Top, &kINI_Left, &kINI_Right, &kINI_Bottom, &kINI_Visible);

		Singleton = NULL;
	}

	void CSEAuxiliaryViewport::Initialize()
	{
		if (Settings::Dialogs::kShowMainWindowsInTaskbar.GetData().i)
		{
			BGSEditorExtender::BGSEEWindowStyler::StyleData RegularAppWindow = {0};
			RegularAppWindow.Extended = WS_EX_APPWINDOW;
			RegularAppWindow.ExtendedOp = BGSEditorExtender::BGSEEWindowStyler::StyleData::kOperation_OR;

			BGSEEUI->GetWindowStyler()->RegisterStyle(IDD_AUXVIEWPORT, RegularAppWindow);
		}

		Create(NULL, true, true);
		INILoadUIState(&kINI_Top, &kINI_Left, &kINI_Right, &kINI_Bottom, &kINI_Visible);

		ClearScreen();
	}

	void CSEAuxiliaryViewport::ClearScreen()
	{
		InvalidateRect(DialogHandle, NULL, TRUE);
	}

	void CSEAuxiliaryViewport::Redraw()
	{
		TESRenderWindow::Redraw();
	}

	void CSEAuxiliaryViewport::SyncViewportCamera( NiCamera* Camera )
	{
		UInt32 RefCountBuffer = ViewportCamera->m_uiRefCount;

		memcpy(ViewportCamera, Camera, sizeof(NiCamera));
		ViewportCamera->m_uiRefCount = RefCountBuffer;
		ViewportCamera->m_parent = NULL;
	}

	bool CSEAuxiliaryViewport::ToggleFrozenState()
	{
		if (GetFrozen())
			Frozen = false;
		else
			Frozen = true;

		return Frozen;
	}

	void CSEAuxiliaryViewport::Draw( NiNode* NodeToRender, NiCamera* Camera )
	{
		Hooks::_MemHdlr(NiDX9RendererPresent).WriteUInt16(0x9090);
		if (Camera == NULL)
			Camera = ViewportCamera;

		BGSEERWPAINTER->SetEnabled(false);
		_PRIMARYRENDERER->RenderNode(Camera, NodeToRender);
		BGSEERWPAINTER->SetEnabled(true);
		Hooks::_MemHdlr(NiDX9RendererPresent).WriteBuffer();

		DrawBackBuffer();
	}

	void CSEAuxiliaryViewport::DrawBackBuffer( void )
	{
		_NIRENDERER->device->Present(NULL, NULL, DialogHandle, NULL);
	}

	bool CSEAuxiliaryViewport::GetFrozen() const
	{
		return Frozen;
	}

	void CSEAuxiliaryViewport::RegisterINISettings( BGSEditorExtender::INISettingDepotT& Depot )
	{
		Depot.push_back(&kINI_Top);
		Depot.push_back(&kINI_Left);
		Depot.push_back(&kINI_Right);
		Depot.push_back(&kINI_Bottom);
		Depot.push_back(&kINI_Visible);
	}

}