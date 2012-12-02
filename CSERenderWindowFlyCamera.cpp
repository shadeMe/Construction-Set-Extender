#include "CSERenderWindowFlyCamera.h"
#include "Hooks\Hooks-Renderer.h"

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kRenderWindowFlyCameraINISettings[kRenderWindowFlyCamera__MAX] =
		{
			{ "MovementSpeed",			"10.0",			"Fly camera movement speed" },
			{ "SprintMultiplier",		"7.5",			"Fly camera sprint speed multiplier" },
			{ "CrawlMultiplier",		"0.05",			"Fly camera crawl speed multiplier" },
			{ "RotationSpeed",			"1.0",			"Fly camera rotation" },
			{ "CameraFOV",				"90",			"Fly camera field of vision" }
		};

		BGSEditorExtender::BGSEEINIManagerSettingFactory* GetRenderWindowFlyCamera( void )
		{
			static BGSEditorExtender::BGSEEINIManagerSettingFactory	kFactory("RenderWindowFlyCamera");
			if (kFactory.Settings.size() == 0)
			{
				for (int i = 0; i < kRenderWindowFlyCamera__MAX; i++)
					kFactory.Settings.push_back(&kRenderWindowFlyCameraINISettings[i]);
			}

			return &kFactory;
		}
	}

	CSERenderWindowFlyCameraOperator::CSERenderWindowFlyCameraOperator( HWND Parent, BGSEditorExtender::ResourceTemplateT TemplateID ) :
		BGSEditorExtender::BGSEERenderWindowFlyCameraOperator(Parent, TemplateID)
	{
		ZeroMemory(&ViewportFrustumBuffer, sizeof(NiFrustum));
	}

	CSERenderWindowFlyCameraOperator::~CSERenderWindowFlyCameraOperator()
	{
		;//
	}

	void CSERenderWindowFlyCameraOperator::PrologCallback( void )
	{
		_RENDERSEL->ClearSelection(true);
		Hooks::_MemHdlr(CellViewSetCurrentCell).WriteUInt8(0xC3);		// write an immediate retn

		float CameraFOV = atof(INISettings::GetRenderWindowFlyCamera()->Get(INISettings::kRenderWindowFlyCamera_CameraFOV, BGSEEMAIN->INIGetter()));
		if (CameraFOV > 120.0f)
			CameraFOV = 120.0f;
		else if (CameraFOV < 50.0f)
			CameraFOV = 50.0f;

		memcpy(&ViewportFrustumBuffer, &_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, sizeof(NiFrustum));
		TESRender::SetCameraFOV(_PRIMARYRENDERER->primaryCamera, CameraFOV);

		BGSEEUI->GetSubclasser()->UnregisterDialogSubclass(RenderWindowTemplateID, UIManager::RenderWindowMiscSubclassProc);
		RefreshRenderWindow();
	}

	void CSERenderWindowFlyCameraOperator::EpilogCallback( void )
	{
		Hooks::_MemHdlr(CellViewSetCurrentCell).WriteBuffer();			// write original instruction

		memcpy(&_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, &ViewportFrustumBuffer, sizeof(NiFrustum));
		TESRender::UpdateAVObject(_PRIMARYRENDERER->primaryCamera);

		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(RenderWindowTemplateID, UIManager::RenderWindowMiscSubclassProc);
		RefreshRenderWindow();
	}

	void CSERenderWindowFlyCameraOperator::Rotate( int XOffset, int YOffset )
	{
		static Vector3 RotationPivot((float)3.4028235e38, (float)3.4028235e38, (float)3.4028235e38);

		if (XOffset || YOffset)
		{
			float RotationSpeed = atof(INISettings::GetRenderWindowFlyCamera()->Get(INISettings::kRenderWindowFlyCamera_RotationSpeed, BGSEEMAIN->INIGetter()));

			TESRender::RotateNode(_PRIMARYRENDERER->primaryCameraParentNode,
									&RotationPivot,
									XOffset,
									YOffset,
									RotationSpeed);

			RefreshRenderWindow();
		}
	}

	void CSERenderWindowFlyCameraOperator::Move( UInt8 Direction, bool Sprinting, bool Crawling )
	{
		float Velocity = atof(INISettings::GetRenderWindowFlyCamera()->Get(INISettings::kRenderWindowFlyCamera_MovementSpeed, BGSEEMAIN->INIGetter()));

		if (Sprinting)
			Velocity *= atof(INISettings::GetRenderWindowFlyCamera()->Get(INISettings::kRenderWindowFlyCamera_SprintMultiplier, BGSEEMAIN->INIGetter()));

		if (Crawling)
			Velocity *= atof(INISettings::GetRenderWindowFlyCamera()->Get(INISettings::kRenderWindowFlyCamera_CrawlMultiplier, BGSEEMAIN->INIGetter()));

		switch (Direction)
		{
		case BGSEditorExtender::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Forward:
		case BGSEditorExtender::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Backward:
			{
				if (Direction == BGSEditorExtender::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Backward)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk04, Velocity);
			}

			break;
		case BGSEditorExtender::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Left:
		case BGSEditorExtender::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Right:
			{
				if (Direction == BGSEditorExtender::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Right)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk02, Velocity);
			}

			break;
		case BGSEditorExtender::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Up:
		case BGSEditorExtender::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Down:
			{
				if (Direction == BGSEditorExtender::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Down)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk00, Velocity);
			}

			break;
		}

		RefreshRenderWindow();
	}

	void CSERenderWindowFlyCameraOperator::RefreshRenderWindow( void )
	{
		TESRenderWindow::Redraw();
	}
}