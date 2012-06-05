#include "AuxiliaryViewport.h"
#include <BGSEditorExtenderBase_Resource.h>
#include "Construction Set Extender_Resource.h"
#include "CSERenderWindowPainter.h"
#include "Hooks\Renderer.h"
#include <BGSEEMain.h>

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		BGSEditorExtender::BGSEEINIManagerSettingFactory* GetAuxiliaryViewport(void)
		{
			static BGSEditorExtender::BGSEEINIManagerSettingFactory kFactory(AuxiliaryViewport::kINISection);
			if (kFactory.Settings.size() == 0)
			{
				kFactory.Settings.push_back(&AuxiliaryViewport::kDefaultINISettings[AuxiliaryViewport::kDefaultINISetting_Top]);
				kFactory.Settings.push_back(&AuxiliaryViewport::kDefaultINISettings[AuxiliaryViewport::kDefaultINISetting_Left]);
				kFactory.Settings.push_back(&AuxiliaryViewport::kDefaultINISettings[AuxiliaryViewport::kDefaultINISetting_Right]);
				kFactory.Settings.push_back(&AuxiliaryViewport::kDefaultINISettings[AuxiliaryViewport::kDefaultINISetting_Bottom]);
				kFactory.Settings.push_back(&AuxiliaryViewport::kDefaultINISettings[AuxiliaryViewport::kDefaultINISetting_Visible]);
			}

			return &kFactory;
		}
	}

	AuxiliaryViewport*		AuxiliaryViewport::Singleton = NULL;
	const char*				AuxiliaryViewport::kINISection = "AuxViewport";
	const char*				AuxiliaryViewport::kWindowTitle = "Auxiliary Viewport Window";

	AuxiliaryViewport* AuxiliaryViewport::GetSingleton()
	{
		if (Singleton == NULL)
			Singleton = new AuxiliaryViewport();

		return Singleton;
	}

	LRESULT CALLBACK AuxiliaryViewport::BaseDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return )
	{
		LRESULT DlgProcResult = FALSE;
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		AuxiliaryViewport* Instance = dynamic_cast<AuxiliaryViewport*>(UserData->Instance);
		Return = false;

		switch (uMsg)
		{
		case WM_MOVING:
		case WM_SHOWWINDOW:
			AUXVIEWPORT->Redraw();
			break;
		}

		return DlgProcResult;
	}

	AuxiliaryViewport::AuxiliaryViewport() :
		BGSEditorExtender::BGSEEGenericModelessDialog(BGSEEUI->GetMainWindow(), BGSEEMAIN->GetExtenderHandle(), IDD_AUXVIEWPORT, IDR_BGSEE_GENERIC_CONTEXTMENU, BaseDlgProc),
		ViewportCamera(NULL),
		Frozen(false)
	{
		ViewportCamera = (NiCamera*)FormHeap_Allocate(0x124);
		thisCall<void>(0x006FF430, ViewportCamera);		// ctor
		ViewportCamera->m_uiRefCount++;
	}

	AuxiliaryViewport::~AuxiliaryViewport()
	{
		ViewportCamera->m_uiRefCount--;
		thisVirtualCall<void>(0x0, ViewportCamera, true);
		INISaveUIState(&BGSEEMAIN->INISetter(), kINISection);

		Singleton = NULL;
	}

	void AuxiliaryViewport::Initialize()
	{
		Create(NULL, true);
		INILoadUIState(&BGSEEMAIN->INIGetter(), kINISection);

		ClearScreen();
	}

	void AuxiliaryViewport::ClearScreen()
	{
		InvalidateRect(DialogHandle, NULL, TRUE);
	}

	void AuxiliaryViewport::Redraw()
	{
		TESDialog::RedrawRenderWindow();
	}

	void AuxiliaryViewport::SyncViewportCamera( NiCamera* Camera )
	{
		UInt32 RefCountBuffer = ViewportCamera->m_uiRefCount;

		memcpy(ViewportCamera, Camera, sizeof(NiCamera));
		ViewportCamera->m_uiRefCount = RefCountBuffer;
		ViewportCamera->m_parent = NULL;
	}

	bool AuxiliaryViewport::ToggleFrozenState()
	{
		if (GetFrozen())
			Frozen = false;
		else
			Frozen = true;

		return Frozen;
	}

	void AuxiliaryViewport::Draw( NiNode* NodeToRender, NiCamera* Camera )
	{
		Hooks::_MemHdlr(NiDX9RendererPresent).WriteUInt16(0x9090);
		if (Camera == NULL)
			Camera = ViewportCamera;

		BGSEERWPAINTER->SetEnabled(false);
		_RENDERCMPT->RenderNode(Camera, NodeToRender);
		BGSEERWPAINTER->SetEnabled(true);
		Hooks::_MemHdlr(NiDX9RendererPresent).WriteBuffer();

		DrawBackBuffer();
	}

	void AuxiliaryViewport::DrawBackBuffer( void )
	{
		_RENDERER->device->Present(NULL, NULL, DialogHandle, NULL);
	}

	bool AuxiliaryViewport::GetFrozen() const
	{
		return Frozen;
	}
}