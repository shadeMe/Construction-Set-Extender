#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "BGSEERenderWindowFlyCamera.h"

namespace BGSEditorExtender
{
	BGSEERenderWindowFlyCameraOperator::BGSEERenderWindowFlyCameraOperator( HWND Parent, ResourceTemplateT TemplateID )
	{
		RenderWindow = Parent;
		RenderWindowTemplateID = TemplateID;
	}

	BGSEERenderWindowFlyCameraOperator::~BGSEERenderWindowFlyCameraOperator()
	{
		;//
	}

	bool BGSEERenderWindowFlyCamera::FlyCamModeActive = false;

	BGSEERenderWindowFlyCamera::BGSEERenderWindowFlyCamera( BGSEERenderWindowFlyCameraOperator* Operator ) :
		BGSEEWindowExtraData()
	{
		this->Operator = Operator;

		DInput = NULL;
		DIMouse = NULL;
		DIKeyboard = NULL;
		ZeroMemory(&DIMouseState, sizeof(DIMouseState));
		ZeroMemory(&DIKeyboardState, sizeof(DIKeyboardState));
		InputValid = false;

		EnterFlyCamMode();
	}

	BGSEERenderWindowFlyCamera::~BGSEERenderWindowFlyCamera()
	{
		ExitFlyCamMode();

		delete Operator;
	}

	void BGSEERenderWindowFlyCamera::EnterFlyCamMode()
	{
		SME_ASSERT(FlyCamModeActive == false);
		FlyCamModeActive = true;

		SME_ASSERT(Operator && Operator->RenderWindow && Operator->RenderWindowTemplateID);

		CreateDInput();
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(Operator->RenderWindowTemplateID, RenderWindowSubclassProc);

		Operator->PrologCallback();
	}

	void BGSEERenderWindowFlyCamera::ExitFlyCamMode()
	{
		SME_ASSERT(FlyCamModeActive);
		FlyCamModeActive = false;

		ReleaseDInput();
		BGSEEUI->GetSubclasser()->UnregisterDialogSubclass(Operator->RenderWindowTemplateID, RenderWindowSubclassProc);

		Operator->EpilogCallback();
	}

	bool BGSEERenderWindowFlyCamera::CreateDInput()
	{
		HRESULT Result = S_OK;
		InputValid = false;

		if (FAILED((Result = DirectInput8Create(BGSEEMAIN->GetExtenderHandle(),
			DIRECTINPUT_VERSION,
			IID_IDirectInput8,
			(void**)&DInput,
			NULL))))
		{
			BGSEECONSOLE_MESSAGE("Couldn't initialize DirectInput for fly camera mode! Result = %d (%08X)", Result, Result);
			return InputValid;
		}

		if (FAILED((Result = DInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL))))
		{
			BGSEECONSOLE_MESSAGE("Couldn't create DirectInput Keyboard object for fly camera mode! Result = %d (%08X)", Result, Result);
			return InputValid;
		}

		if (FAILED((Result = DInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL))))
		{
			BGSEECONSOLE_MESSAGE("Couldn't create DirectInput Mouse object for fly camera mode! Result = %d (%08X)", Result, Result);
			return InputValid;
		}

		DIMouse->SetDataFormat(&c_dfDIMouse);
		DIMouse->SetCooperativeLevel(Operator->RenderWindow, DISCL_EXCLUSIVE|DISCL_FOREGROUND);

		DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
		DIKeyboard->SetCooperativeLevel(Operator->RenderWindow, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);

		InputValid = true;
		return InputValid;
	}

	void BGSEERenderWindowFlyCamera::ReleaseDInput()
	{
		DIKeyboard->Unacquire();
		DIMouse->Unacquire();
		DInput->Release();
	}

	bool BGSEERenderWindowFlyCamera::UpdateDInput()
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

	bool BGSEERenderWindowFlyCamera::GetMouseDelta( POINT* OutDelta, bool InvertXAxis /*= true*/, bool InvertYAxis /*= false*/ )
	{
		if (InputValid && OutDelta)
		{
			OutDelta->x = (InvertXAxis ? -DIMouseState.lX : DIMouseState.lX);
			OutDelta->y = (InvertYAxis ? -DIMouseState.lY : DIMouseState.lY);

			return true;
		}

		return false;
	}

	bool BGSEERenderWindowFlyCamera::GetKeyPressed( BYTE Key )
	{
		if (InputValid == false || Key >= 256)
			return false;

		if (DIKeyboardState[Key] & 0x80)
			return true;
		else
			return false;
	}

	LRESULT CALLBACK BGSEERenderWindowFlyCamera::RenderWindowSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																		bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
	{
		LRESULT DlgProcResult = TRUE;
		Return = true;						// never let a message pass through to the original dlgproc, unless it's absolutely necessary

		BGSEERenderWindowFlyCamera* CamData = BGSEE_GETWINDOWXDATA_QUICK(BGSEERenderWindowFlyCamera, ExtraData);
		SME_ASSERT(CamData);

		switch (uMsg)
		{
		case WM_DESTROY:
			ExtraData->Remove(BGSEERenderWindowFlyCamera::kTypeID);
			delete CamData;

			break;
		case WM_TIMER:
			{
				Return = false;				// the org dlgproc needs to process the timer message to update the viewport

				POINT MouseDelta = {0};
				bool Sprinting = false;
				bool Crawling = false;

				if (CamData->UpdateDInput())
				{
					if (CamData->GetKeyPressed(DIK_LSHIFT) || CamData->GetKeyPressed(DIK_RSHIFT))
						Sprinting = true;

					if (CamData->GetKeyPressed(DIK_LALT) || CamData->GetKeyPressed(DIK_RALT))
						Crawling = true;

					if (CamData->GetMouseDelta(&MouseDelta))
						CamData->Operator->Rotate(MouseDelta.x, MouseDelta.y);

					if (CamData->GetKeyPressed(DIK_W) || CamData->GetKeyPressed(DIK_UPARROW))
						CamData->Operator->Move(BGSEERenderWindowFlyCameraOperator::kMoveDirection_Forward, Sprinting, Crawling);
					else if (CamData->GetKeyPressed(DIK_S) || CamData->GetKeyPressed(DIK_DOWNARROW))
						CamData->Operator->Move(BGSEERenderWindowFlyCameraOperator::kMoveDirection_Backward, Sprinting, Crawling);

					if (CamData->GetKeyPressed(DIK_A) || CamData->GetKeyPressed(DIK_LEFTARROW))
						CamData->Operator->Move(BGSEERenderWindowFlyCameraOperator::kMoveDirection_Left, Sprinting, Crawling);
					else if (CamData->GetKeyPressed(DIK_D) || CamData->GetKeyPressed(DIK_RIGHTARROW))
						CamData->Operator->Move(BGSEERenderWindowFlyCameraOperator::kMoveDirection_Right, Sprinting, Crawling);

					if (CamData->GetKeyPressed(DIK_SPACE))
						CamData->Operator->Move(BGSEERenderWindowFlyCameraOperator::kMoveDirection_Up, Sprinting, Crawling);
					else if (CamData->GetKeyPressed(DIK_LCONTROL) || CamData->GetKeyPressed(DIK_RCONTROL))
						CamData->Operator->Move(BGSEERenderWindowFlyCameraOperator::kMoveDirection_Down, Sprinting, Crawling);

					if (CamData->GetKeyPressed(DIK_ESCAPE) || CamData->GetKeyPressed(DIK_GRAVE))
					{
						ExtraData->Remove(BGSEERenderWindowFlyCamera::kTypeID);
						delete CamData;

						break;
					}
				}
			}

			break;
		}

		return DlgProcResult;
	}
}