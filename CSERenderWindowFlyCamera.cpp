#include "CSERenderWindowFlyCamera.h"
#include "Hooks\Hooks-Renderer.h"

namespace ConstructionSetExtender
{
	CSERenderWindowFlyCameraOperator::CSERenderWindowFlyCameraOperator( HWND Parent, bgsee::ResourceTemplateT TemplateID ) :
		bgsee::BGSEERenderWindowFlyCameraOperator(Parent, TemplateID)
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

		float CameraFOV = Settings::RenderWindowFlyCamera::kCameraFOV.GetData().f;
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
			float RotationSpeed = Settings::RenderWindowFlyCamera::kRotationSpeed.GetData().f;

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
		float Velocity = Settings::RenderWindowFlyCamera::kMovementSpeed.GetData().f;

		if (Sprinting)
			Velocity *= Settings::RenderWindowFlyCamera::kSprintMultiplier.GetData().f;

		if (Crawling)
			Velocity *= Settings::RenderWindowFlyCamera::kCrawlMultiplier.GetData().f;

		switch (Direction)
		{
		case bgsee::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Forward:
		case bgsee::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Backward:
			{
				if (Direction == bgsee::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Backward)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk04, Velocity);
			}

			break;
		case bgsee::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Left:
		case bgsee::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Right:
			{
				if (Direction == bgsee::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Right)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk02, Velocity);
			}

			break;
		case bgsee::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Up:
		case bgsee::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Down:
			{
				if (Direction == bgsee::BGSEERenderWindowFlyCameraOperator::kMoveDirection_Down)
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