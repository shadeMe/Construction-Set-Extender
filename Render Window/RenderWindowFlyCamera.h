#pragma once

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowFlyCamera
		{
			bool					Active;

			LPDIRECTINPUT8			DInput;
			LPDIRECTINPUTDEVICE8	DIMouse;
			LPDIRECTINPUTDEVICE8	DIKeyboard;
			HKL						KeyboardLayout;
			bool					InputValid;

			DIMOUSESTATE			DIMouseState;
			BYTE					DIKeyboardState[256];
			NiFrustum				ViewportFrustumBuffer;

			bgsee::util::ThunkStdCall<RenderWindowFlyCamera, LRESULT, HWND, UINT, WPARAM, LPARAM, bool&, bgsee::WindowExtraDataCollection*, bgsee::WindowSubclasser*>
									ThunkRenderWindowSubclassProc;

			enum
			{
				kMoveDirection_Forward	= 0,
				kMoveDirection_Backward,
				kMoveDirection_Left,
				kMoveDirection_Right,
				kMoveDirection_Up,
				kMoveDirection_Down,
			};

			bool	CreateDInput();
			void	ReleaseDInput();
			bool	UpdateDInput();

			void	EnterFlyCamMode();
			void	ExitFlyCamMode();

			void	Rotate(int XOffset, int YOffset);
			void	Move(UInt8 Direction, bool Sprinting, bool Crawling);

			void	RefreshRenderWindow();

			bool	GetMouseDelta(POINT* OutDelta, bool InvertXAxis = true, bool InvertYAxis = false);
			bool	GetKeyPressed(BYTE Key);
			bool	GetToggleHotkeyPressed();

			LRESULT RenderWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);

			RenderWindowFlyCamera();
		public:
			void		Activate();
			void		Deactivate();
			bool		IsActive();

			static RenderWindowFlyCamera	Instance;
		};
	}
}