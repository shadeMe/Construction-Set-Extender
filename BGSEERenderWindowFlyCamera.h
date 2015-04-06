#pragma once
#include "BGSEERenderWindowPainter.h"
#include "BGSEEUIManager.h"

// BGSEERenderWindowFlyCamera - Render window free camera

namespace BGSEditorExtender
{
	class BGSEERenderWindowFlyCameraOperator
	{
	public:
		HWND									RenderWindow;
		ResourceTemplateT						RenderWindowTemplateID;

		BGSEERenderWindowFlyCameraOperator(HWND Parent, ResourceTemplateT TemplateID);
		virtual ~BGSEERenderWindowFlyCameraOperator() = 0;

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

	class BGSEERenderWindowFlyCamera : public BGSEEWindowExtraData
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

		BGSEERenderWindowFlyCameraOperator*		Operator;

		bool									GetMouseDelta(POINT* OutDelta, bool InvertXAxis = true, bool InvertYAxis = false);
		bool									GetKeyPressed(BYTE Key);

		static LRESULT CALLBACK					RenderWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																		bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData);
	public:
		BGSEERenderWindowFlyCamera(BGSEERenderWindowFlyCameraOperator* Operator);	// takes ownership of the pointer
		virtual ~BGSEERenderWindowFlyCamera();

		enum { kTypeID = 'XRFC' };

		static bool								FlyCamModeActive;
	};
}