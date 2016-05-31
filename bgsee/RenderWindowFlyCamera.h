#pragma once
#include "RenderWindowPainter.h"
#include "UIManager.h"

// RenderWindowFlyCamera - Render window free camera

namespace bgsee
{
	class RenderWindowFlyCameraOperator
	{
	public:
		HWND									RenderWindow;
		ResourceTemplateT						RenderWindowTemplateID;

		RenderWindowFlyCameraOperator(HWND Parent, ResourceTemplateT TemplateID);
		virtual ~RenderWindowFlyCameraOperator() = 0;

		enum
		{
			kMoveDirection_Forward	= 0,
			kMoveDirection_Backward,
			kMoveDirection_Left,
			kMoveDirection_Right,
			kMoveDirection_Up,
			kMoveDirection_Down,
		};

		virtual void							PrologCallback(void) = 0;		// called right before entering fly cam mode
		virtual void							EpilogCallback(void) = 0;		// called right after exiting fly cam mode

		virtual void							Rotate(int XOffset, int YOffset) = 0;
		virtual void							Move(UInt8 Direction, bool Sprinting, bool Crawling) = 0;

		virtual void							RefreshRenderWindow(void) = 0;
	};

	class RenderWindowFlyCamera : public WindowExtraData
	{
		bool									CreateDInput();
		void									ReleaseDInput();
		bool									UpdateDInput();

		void									EnterFlyCamMode();
		void									ExitFlyCamMode();

		LPDIRECTINPUT8							DInput;
		LPDIRECTINPUTDEVICE8					DIMouse;
		LPDIRECTINPUTDEVICE8					DIKeyboard;
		bool									InputValid;

		DIMOUSESTATE							DIMouseState;
		BYTE									DIKeyboardState[256];

		RenderWindowFlyCameraOperator*			Operator;

		bool									GetMouseDelta(POINT* OutDelta, bool InvertXAxis = true, bool InvertYAxis = false);
		bool									GetKeyPressed(BYTE Key);

		static LRESULT CALLBACK					RenderWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																		bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
	public:
		RenderWindowFlyCamera(RenderWindowFlyCameraOperator* Operator);	// takes ownership of the pointer
		virtual ~RenderWindowFlyCamera();

		enum { kTypeID = 'XRFC' };

		static bool								FlyCamModeActive;
	};
}