#include "RenderWindowFlyCamera.h"
#include "Hooks\Hooks-Renderer.h"
#include "RenderWindowManager.h"

namespace cse
{
	namespace renderWindow
	{
		RenderWindowFlyCamera	RenderWindowFlyCamera::Instance;

		RenderWindowFlyCamera::RenderWindowFlyCamera() :
			ThunkRenderWindowSubclassProc(this, &RenderWindowFlyCamera::RenderWindowSubclassProc)
		{
			Active = false;
			DInput = nullptr;
			DIMouse = nullptr;
			DIKeyboard = nullptr;
			ZeroMemory(&DIMouseState, sizeof(DIMouseState));
			ZeroMemory(&DIKeyboardState, sizeof(DIKeyboardState));
			InputValid = false;
			ZeroMemory(&ViewportFrustumBuffer, sizeof(NiFrustum));
		}

		void RenderWindowFlyCamera::EnterFlyCamMode()
		{
			SME_ASSERT(Active == false);
			Active = true;

			CreateDInput();
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_RenderWindow, ThunkRenderWindowSubclassProc());

			_RENDERSEL->ClearSelection(true);
			hooks::_MemHdlr(CellViewSetCurrentCell).WriteUInt8(0xC3);		// write an immediate retn

			float CameraFOV = settings::renderWindowFlyCamera::kCameraFOV.GetData().f;
			if (CameraFOV > 120.0f)
				CameraFOV = 120.0f;
			else if (CameraFOV < 50.0f)
				CameraFOV = 50.0f;

			memcpy(&ViewportFrustumBuffer, &_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, sizeof(NiFrustum));
			TESRender::SetCameraFOV(_PRIMARYRENDERER->primaryCamera, CameraFOV);

			RefreshRenderWindow();
		}

		void RenderWindowFlyCamera::ExitFlyCamMode()
		{
			SME_ASSERT(Active);
			Active = false;

			ReleaseDInput();
			BGSEEUI->GetSubclasser()->DeregisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_RenderWindow, ThunkRenderWindowSubclassProc());

			hooks::_MemHdlr(CellViewSetCurrentCell).WriteBuffer();			// write original instruction

			memcpy(&_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, &ViewportFrustumBuffer, sizeof(NiFrustum));
			TESRender::UpdateAVObject(_PRIMARYRENDERER->primaryCamera);

			RefreshRenderWindow();
		}

		bool RenderWindowFlyCamera::CreateDInput()
		{
			HRESULT Result = S_OK;
			InputValid = false;

			if (FAILED((Result = DirectInput8Create(BGSEEMAIN->GetExtenderHandle(),
				DIRECTINPUT_VERSION,
				IID_IDirectInput8,
				(void**)&DInput,
				nullptr))))
			{
				BGSEECONSOLE_MESSAGE("Couldn't initialize DirectInput for fly camera mode! Result = %d (%08X)", Result, Result);
				return InputValid;
			}

			if (FAILED((Result = DInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, nullptr))))
			{
				BGSEECONSOLE_MESSAGE("Couldn't create DirectInput Keyboard object for fly camera mode! Result = %d (%08X)", Result, Result);
				return InputValid;
			}

			if (FAILED((Result = DInput->CreateDevice(GUID_SysMouse, &DIMouse, nullptr))))
			{
				BGSEECONSOLE_MESSAGE("Couldn't create DirectInput Mouse object for fly camera mode! Result = %d (%08X)", Result, Result);
				return InputValid;
			}

			DIMouse->SetDataFormat(&c_dfDIMouse);
			DIMouse->SetCooperativeLevel(*TESRenderWindow::WindowHandle, DISCL_EXCLUSIVE|DISCL_FOREGROUND);

			DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
			DIKeyboard->SetCooperativeLevel(*TESRenderWindow::WindowHandle, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);

			InputValid = true;
			return InputValid;
		}

		void RenderWindowFlyCamera::ReleaseDInput()
		{
			DIKeyboard->Unacquire();
			DIMouse->Unacquire();
			DInput->Release();
		}

		bool RenderWindowFlyCamera::UpdateDInput()
		{
			HRESULT Result = S_OK;
			if (InputValid == false)
				return false;

			while (true)
			{
				if (FAILED((Result = DIKeyboard->Acquire())))
				{
					BGSEECONSOLE_MESSAGE("Couldn't acquire DirectInput Keyboard object for fly camera mode! Result = %d (%08X)", Result, Result);
					break;
				}
				else
				{
					if (FAILED((Result = DIKeyboard->GetDeviceState(256, (LPVOID)&DIKeyboardState))))
					{
						BGSEECONSOLE_MESSAGE("Couldn't get DirectInput Keyboard state for fly camera mode! Result = %d (%08X)", Result, Result);
						break;
					}
				}

				if (FAILED((Result = DIMouse->Acquire())))
				{
					BGSEECONSOLE_MESSAGE("Couldn't acquire DirectInput Mouse object for fly camera mode! Result = %d (%08X)", Result, Result);
					break;
				}
				else
				{
					if (FAILED((Result = DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&DIMouseState))))
					{
						BGSEECONSOLE_MESSAGE("Couldn't get DirectInput Mouse state for fly camera mode! Result = %d (%08X)", Result, Result);
						break;
					}
				}

				break;
			}

			return !(FAILED(Result));
		}

		bool RenderWindowFlyCamera::GetMouseDelta( POINT* OutDelta, bool InvertXAxis /*= true*/, bool InvertYAxis /*= false*/ )
		{
			if (InputValid && OutDelta)
			{
				OutDelta->x = (InvertXAxis ? -DIMouseState.lX : DIMouseState.lX);
				OutDelta->y = (InvertYAxis ? -DIMouseState.lY : DIMouseState.lY);

				return true;
			}

			return false;
		}

		bool RenderWindowFlyCamera::GetKeyPressed( BYTE Key )
		{
			if (InputValid == false || Key >= 256)
				return false;

			if (DIKeyboardState[Key] & 0x80)
				return true;
			else
				return false;
		}

		bool RenderWindowFlyCamera::GetToggleHotkeyPressed()
		{
			auto ToggleKey = _RENDERWIN_MGR.GetKeyboardInputManager()->GetSharedBindings().ToggleFlyCamera;
			const auto& Binding = ToggleKey->GetActiveBinding();

			return Binding.IsActivated();
		}

		void RenderWindowFlyCamera::Rotate(int XOffset, int YOffset)
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

		void RenderWindowFlyCamera::Move(UInt8 Direction, bool Sprinting, bool Crawling)
		{
			float Velocity = settings::renderWindowFlyCamera::kMovementSpeed.GetData().f;

			if (Sprinting)
				Velocity *= settings::renderWindowFlyCamera::kSprintMultiplier.GetData().f;

			if (Crawling)
				Velocity *= settings::renderWindowFlyCamera::kCrawlMultiplier.GetData().f;

			switch (Direction)
			{
			case RenderWindowFlyCamera::kMoveDirection_Forward:
			case RenderWindowFlyCamera::kMoveDirection_Backward:
			{
				if (Direction == RenderWindowFlyCamera::kMoveDirection_Backward)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk04, Velocity);
			}

			break;
			case RenderWindowFlyCamera::kMoveDirection_Left:
			case RenderWindowFlyCamera::kMoveDirection_Right:
			{
				if (Direction == RenderWindowFlyCamera::kMoveDirection_Right)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk02, Velocity);
			}

			break;
			case RenderWindowFlyCamera::kMoveDirection_Up:
			case RenderWindowFlyCamera::kMoveDirection_Down:
			{
				if (Direction == RenderWindowFlyCamera::kMoveDirection_Down)
					Velocity *= -1;

				TESRender::UpdateNode(_PRIMARYRENDERER->primaryCameraParentNode, TESRender::kNodeUpdate_Unk00, Velocity);
			}

			break;
			}

			RefreshRenderWindow();
		}

		void RenderWindowFlyCamera::RefreshRenderWindow(void)
		{
			TESRenderWindow::Redraw();
		}

		LRESULT RenderWindowFlyCamera::RenderWindowSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																bool& Return, bgsee::WindowExtraDataCollection* ExtraData )
		{
			SME_ASSERT(Active == true);

			LRESULT DlgProcResult = TRUE;
			Return = true;		// never let a message pass through to the original dlgproc, unless it's absolutely necessary

			switch (uMsg)
			{
			case WM_DESTROY:
				Deactivate();

				break;
			case WM_TIMER:
				{
					Return = false;		// the org dlgproc needs to process the timer message to update the viewport

					POINT MouseDelta = {0};
					bool Sprinting = false;
					bool Crawling = false;

					if (!UpdateDInput())
						break;

					if (GetKeyPressed(DIK_LSHIFT) || GetKeyPressed(DIK_RSHIFT))
						Sprinting = true;

					if (GetKeyPressed(DIK_LALT) || GetKeyPressed(DIK_RALT))
						Crawling = true;

					if (GetMouseDelta(&MouseDelta))
						Rotate(MouseDelta.x, MouseDelta.y);

					if (GetKeyPressed(DIK_W) || GetKeyPressed(DIK_UPARROW))
						Move(kMoveDirection_Forward, Sprinting, Crawling);
					else if (GetKeyPressed(DIK_S) || GetKeyPressed(DIK_DOWNARROW))
						Move(kMoveDirection_Backward, Sprinting, Crawling);

					if (GetKeyPressed(DIK_A) || GetKeyPressed(DIK_LEFTARROW))
						Move(kMoveDirection_Left, Sprinting, Crawling);
					else if (GetKeyPressed(DIK_D) || GetKeyPressed(DIK_RIGHTARROW))
						Move(kMoveDirection_Right, Sprinting, Crawling);

					if (GetKeyPressed(DIK_SPACE))
						Move(kMoveDirection_Up, Sprinting, Crawling);
					else if (GetKeyPressed(DIK_LCONTROL) || GetKeyPressed(DIK_RCONTROL))
						Move(kMoveDirection_Down, Sprinting, Crawling);


					if (GetToggleHotkeyPressed())
						Deactivate();

					break;
				}

			}

			return DlgProcResult;
		}

		void RenderWindowFlyCamera::Activate()
		{
			if (Active)
				return;
			else if (TESLODTextureGenerator::GeneratorState != TESLODTextureGenerator::kState_NotInUse)
				return;
			else if (GetCapture())
				return;

			_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
			EnterFlyCamMode();
		}

		void RenderWindowFlyCamera::Deactivate()
		{
			if (!Active)
				return;

			ExitFlyCamMode();
		}

		bool RenderWindowFlyCamera::IsActive()
		{
			return Active;
		}


	}
}