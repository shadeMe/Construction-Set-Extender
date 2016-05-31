#include "RenderWindowFlyCamera.h"
#include "Hooks\Hooks-Renderer.h"

namespace cse
{
	RenderWindowFlyCameraOperator::RenderWindowFlyCameraOperator( HWND Parent, bgsee::ResourceTemplateT TemplateID ) :
		bgsee::RenderWindowFlyCameraOperator(Parent, TemplateID)
	{
		ZeroMemory(&ViewportFrustumBuffer, sizeof(NiFrustum));
	}

	RenderWindowFlyCameraOperator::~RenderWindowFlyCameraOperator()
	{
		;//
	}

	void RenderWindowFlyCameraOperator::PrologCallback( void )
	{
		_RENDERSEL->ClearSelection(true);
		hooks::_MemHdlr(CellViewSetCurrentCell).WriteUInt8(0xC3);		// write an immediate retn

		float CameraFOV = settings::renderWindowFlyCamera::kCameraFOV.GetData().f;
		if (CameraFOV > 120.0f)
			CameraFOV = 120.0f;
		else if (CameraFOV < 50.0f)
			CameraFOV = 50.0f;

		memcpy(&ViewportFrustumBuffer, &_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, sizeof(NiFrustum));
		TESRender::SetCameraFOV(_PRIMARYRENDERER->primaryCamera, CameraFOV);

		BGSEEUI->GetSubclasser()->UnregisterDialogSubclass(RenderWindowTemplateID, uiManager::RenderWindowMiscSubclassProc);
		RefreshRenderWindow();
	}

	void RenderWindowFlyCameraOperator::EpilogCallback( void )
	{
		hooks::_MemHdlr(CellViewSetCurrentCell).WriteBuffer();			// write original instruction

		memcpy(&_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, &ViewportFrustumBuffer, sizeof(NiFrustum));
		TESRender::UpdateAVObject(_PRIMARYRENDERER->primaryCamera);

		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(RenderWindowTemplateID, uiManager::RenderWindowMiscSubclassProc);
		RefreshRenderWindow();
	}

	void RenderWindowFlyCameraOperator::Rotate( int XOffset, int YOffset )
	{
		static Vector3 RotationPivot((float)3.4028235e38, (float)3.4028235e38, (float)3.4028235e38);

		if (XOffset || YOffset)
		{
			float RotationSpeed = settings::renderWindowFlyCamera::kRotationSpeed.GetData().f;

			TESRender::RotateNode(_PRIMARYRENDERER->primaryCameraParentNode,
									&RotationPivot,
									XOffset,
									YOffset,
									RotationSpeed);

			RefreshRenderWindow();
		}
	}

	void RenderWindowFlyCameraOperator::Move( UInt8 Direction, bool Sprinting, bool Crawling )
	{
		float Velocity = settings::renderWindowFlyCamera::kMovementSpeed.GetData().f;

		if (Sprinting)
			Velocity *= settings::renderWindowFlyCamera::kSprintMultiplier.GetData().f;

		if (Crawling)
			Velocity *= settings::renderWindowFlyCamera::kCrawlMultiplier.GetData().f;

		switch (Direction)
		{
		case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Forward:
		case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Backward:
			{
				if (Direction == bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Backward)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk04, Velocity);
			}

			break;
		case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Left:
		case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Right:
			{
				if (Direction == bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Right)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk02, Velocity);
			}

			break;
		case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Up:
		case bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Down:
			{
				if (Direction == bgsee::RenderWindowFlyCameraOperator::kMoveDirection_Down)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk00, Velocity);
			}

			break;
		}

		RefreshRenderWindow();
	}

	void RenderWindowFlyCameraOperator::RefreshRenderWindow( void )
	{
		TESRenderWindow::Redraw();
	}
}