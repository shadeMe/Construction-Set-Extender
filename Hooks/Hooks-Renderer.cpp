#include "Hooks-Renderer.h"
#include "Render Window\RenderWindowManager.h"
#include "Render Window\AuxiliaryViewport.h"
#include "Render Window\PathGridUndoManager.h"
#include "Hooks-LOD.h"

#pragma warning(push)
#pragma warning(disable: 4005 4748 4410)


namespace cse
{
	using namespace renderWindow;

	namespace hooks
	{
		_DefineHookHdlr(DoorMarkerProperties, 0x00429EA1);
		_DefineHookHdlr(TESObjectREFRSetupDialog, 0x005499FB);
		_DefineHookHdlr(TESObjectREFRCleanDialog, 0x00549B52);
		_DefineHookHdlr(TESRenderControlPerformFallVoid, 0x004270C2);
		_DefineHookHdlr(ForceShowTESObjectREFRDialog, 0x00429EE3);
		_DefineHookHdlr(TESRenderControlAltSnapGrid, 0x00425A34);
		_DefineHookHdlr(TESRenderControlAltRefMovementSpeedA, 0x00425737);
		_DefineHookHdlr(TESRenderControlAltRefMovementSpeedB, 0x0042BE80);
		_DefineHookHdlr(TESRenderControlAltRefMovementSpeedC, 0x0042D0AD);
		_DefineHookHdlr(TESRenderControlAltRefRotationSpeed, 0x00425DBB);
		_DefineHookHdlr(TESRenderControlAltRefSnapAngle, 0x00425DC7);
		_DefineHookHdlr(TESRenderControlAltCamRotationSpeed, 0x0042CCAB);
		_DefineHookHdlr(TESRenderControlAltCamZoomSpeedA, 0x0042CCE0);
		_DefineHookHdlr(TESRenderControlAltCamZoomSpeedB, 0x0042CDAF);
		_DefineHookHdlr(TESRenderControlAltCamPanSpeedA, 0x0042CD26);
		_DefineHookHdlr(TESRenderControlAltCamPanSpeedB, 0x0042CD71);
		_DefineHookHdlr(TESRenderControlRedrawGrid, 0x004283F7);
		_DefineHookHdlr(TESPreviewControlCallWndProc, 0x0044D700);
		_DefineHookHdlr(ActivateRenderWindowPostLandTextureChange, 0x0042B4E5);
		_DefineHookHdlr(TESPathGridRecordOperationMoveA, 0x0042A62D);
		_DefineHookHdlr(TESPathGridRecordOperationMoveB, 0x0042BE6D);
		_DefineHookHdlr(TESPathGridRecordOperationLink, 0x0042A829);
		_DefineHookHdlr(TESPathGridRecordOperationFlag, 0x0042A714);
		_DefineHookHdlr(TESPathGridRecordOperationRef, 0x00428367);
		_DefineHookHdlr(TESPathGridDeletePoint, 0x004291C6);
		_DefineHookHdlr(TESPathGridPointDtor, 0x00556190);
		_DefineHookHdlr(TESPathGridToggleEditMode, 0x00550660);
		_DefineHookHdlr(TESPathGridCreateNewLinkedPoint, 0x0042B37B);
		_DefineHookHdlr(TESPathGridPerformFall, 0x00428605);
		_DefineHookHdlr(TESPathGridShowMultipleSelectionRing, 0x0042FC7C);
		_DefinePatchHdlr(TESPathGridDtor, 0x00550B81);
		_DefineHookHdlr(InitialCellLoadCameraPosition, 0x0040A8AE);
		_DefinePatchHdlr(LandscapeEditBrushRadius, 0x0041F2EE + 2);
		_DefineHookHdlrWithBuffer(ConvertNiRenderedTexToD3DBaseTex, 0x00411616, 5, 0x85, 0xC0, 0x75, 0x2E, 0x8B);
		_DefineHookHdlr(DuplicateReferences, 0x0042EC2E);
		_DefinePatchHdlrWithBuffer(NiDX9RendererPresent, 0x006D5C9D, 2, 0xFF, 0xD0);
		_DefineHookHdlr(TESRenderControlPerformRelativeScale, 0x00424700);
		_DefinePatchHdlr(DataHandlerClosePlugins, 0x0047B2FA);
		_DefineHookHdlr(TESPathGridRubberBandSelection, 0x0042FBE0);
		_DefineHookHdlr(CoplanarRefDrop, 0x0042DE2A);
		_DefineHookHdlr(InitPathGridNodeSelectionRing, 0x00419AFA);
		_DefineHookHdlr(TESLoadIntoViewPort, 0x00430F5A);
		_DefineHookHdlr(RenderWindowAxisHotkeysMoveReferences, 0x0042CB79);
		_DefineHookHdlr(RenderWindowAxisHotkeysMovePathGridPoints, 0x0042BF17);
		_DefinePatchHdlr(RenderWindowAxisHotkeysRotateReferences, 0x0042CBBD + 2);
		_DefineHookHdlr(BSFadeNodeDrawTransparency, 0x004BC527);
		_DefinePatchHdlrWithBuffer(CellViewSetCurrentCell, 0x00409170, 1, 0x53);
		_DefineJumpHdlr(RenderWindowCursorSwap, 0x0042CA34, 0x0042CAA3);
		_DefinePatchHdlr(RenderWindowCopySelection, 0x0042E8A6 + 1);
		_DefineHookHdlr(TESPathGridPointGenerateNiNodeA, 0x00556779);
		_DefineHookHdlr(TESPathGridPointGenerateNiNodeB, 0x005567DB);
		_DefineHookHdlr(TESPathGridGenerateNiNode, 0x0054EE0D);
		_DefineHookHdlr(CenterCameraOnRefSelection, 0x00428E35);
		_DefineHookHdlr(TopCameraOnRefSelection, 0x00428FB8);
		_DefineHookHdlr(TESRenderRotateSelectionWorldA, 0x00425F16);
		_DefineHookHdlr(TESRenderRotateSelectionWorldB, 0x00426043);
		_DefineHookHdlr(RotateCameraDrag, 0x0042CBFD);
		_DefineHookHdlr(LandscapeTextureLoad, 0x005232E0);
		_DefineHookHdlr(PatchGetAsyncKeyStateA, 0x0042753B);
		_DefineJumpHdlr(PatchGetAsyncKeyStateB, 0x00427541, 0x00427593);
		_DefineHookHdlr(UndoStackUndoOp3, 0x00431D98);
		_DefineHookHdlr(UndoStackRedoOp3, 0x004328FA);
		_DefineHookHdlr(MoveSelectionClampMul, 0x0042572C);

#ifndef NDEBUG
		void __stdcall DoTestHook1()
		{
			BGSEECONSOLE_MESSAGE("MOVE CALL!");
		}

		#define _hhName		TestHook1
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00425676);
			__asm
			{
				pushad
				call	DoTestHook1
				popad

				sub     esp, 114h
				jmp		_hhGetVar(Retn)
			}
		}
	//	_DefineJumpHdlr(TestHook1, 0x004262BF, 0x004262D3);
		_DefineHookHdlr(TestHook1, 0x00425670);
#endif

		void PatchRendererHooks(void)
		{
#ifndef NDEBUG
	//		_MemHdlr(TestHook1).WriteJump();
#endif
			_MemHdlr(DoorMarkerProperties).WriteJump();
			_MemHdlr(TESObjectREFRSetupDialog).WriteJump();
			_MemHdlr(TESObjectREFRCleanDialog).WriteJump();
			_MemHdlr(TESRenderControlPerformFallVoid).WriteJump();
			_MemHdlr(ForceShowTESObjectREFRDialog).WriteJump();
			_MemHdlr(TESRenderControlAltSnapGrid).WriteJump();
			_MemHdlr(TESRenderControlAltRefMovementSpeedA).WriteJump();
			_MemHdlr(TESRenderControlAltRefMovementSpeedB).WriteJump();
			_MemHdlr(TESRenderControlAltRefMovementSpeedC).WriteJump();
			_MemHdlr(TESRenderControlAltRefRotationSpeed).WriteJump();
			_MemHdlr(TESRenderControlAltRefSnapAngle).WriteJump();
			_MemHdlr(TESRenderControlAltCamRotationSpeed).WriteJump();
			_MemHdlr(TESRenderControlAltCamZoomSpeedA).WriteJump();
			_MemHdlr(TESRenderControlAltCamZoomSpeedB).WriteJump();
			_MemHdlr(TESRenderControlAltCamPanSpeedA).WriteJump();
			_MemHdlr(TESRenderControlAltCamPanSpeedB).WriteJump();
			_MemHdlr(TESRenderControlRedrawGrid).WriteJump();
			_MemHdlr(TESPreviewControlCallWndProc).WriteJump();
			_MemHdlr(ActivateRenderWindowPostLandTextureChange).WriteJump();
			_MemHdlr(TESPathGridRecordOperationMoveA).WriteJump();
			_MemHdlr(TESPathGridRecordOperationMoveB).WriteJump();
			_MemHdlr(TESPathGridRecordOperationLink).WriteJump();
			_MemHdlr(TESPathGridRecordOperationFlag).WriteJump();
			_MemHdlr(TESPathGridRecordOperationRef).WriteJump();
			_MemHdlr(TESPathGridDeletePoint).WriteJump();
			_MemHdlr(TESPathGridPointDtor).WriteJump();
			_MemHdlr(TESPathGridToggleEditMode).WriteJump();
			_MemHdlr(TESPathGridCreateNewLinkedPoint).WriteJump();
			_MemHdlr(TESPathGridPerformFall).WriteJump();
			_MemHdlr(TESPathGridShowMultipleSelectionRing).WriteJump();
			_MemHdlr(TESPathGridDtor).WriteUInt8(0xEB);
			_MemHdlr(InitialCellLoadCameraPosition).WriteJump();
			_MemHdlr(LandscapeEditBrushRadius).WriteUInt32((UInt32)&RenderWindowExtendedState::MaxLandscapeEditBrushRadius);
			_MemHdlr(DuplicateReferences).WriteJump();
			_MemHdlr(TESRenderControlPerformRelativeScale).WriteJump();
			_MemHdlr(DataHandlerClosePlugins).WriteUInt8(0xEB);
			_MemHdlr(TESPathGridRubberBandSelection).WriteJump();
			_MemHdlr(CoplanarRefDrop).WriteJump();
			_MemHdlr(InitPathGridNodeSelectionRing).WriteJump();
			_MemHdlr(TESLoadIntoViewPort).WriteJump();
			_MemHdlr(RenderWindowAxisHotkeysMoveReferences).WriteJump();
			_MemHdlr(RenderWindowAxisHotkeysMovePathGridPoints).WriteJump();
			_MemHdlr(RenderWindowAxisHotkeysRotateReferences).WriteUInt32(0x00A0BC1E);
			_MemHdlr(BSFadeNodeDrawTransparency).WriteJump();
			_MemHdlr(RenderWindowCursorSwap).WriteJump();
			_MemHdlr(RenderWindowCopySelection).WriteUInt8(0x0);
			_MemHdlr(TESPathGridPointGenerateNiNodeA).WriteJump();
			_MemHdlr(TESPathGridPointGenerateNiNodeB).WriteJump();
			_MemHdlr(TESPathGridGenerateNiNode).WriteJump();
			_MemHdlr(CenterCameraOnRefSelection).WriteJump();
			_MemHdlr(TopCameraOnRefSelection).WriteJump();
			_MemHdlr(TESRenderRotateSelectionWorldA).WriteJump();
			_MemHdlr(TESRenderRotateSelectionWorldB).WriteJump();
			_MemHdlr(RotateCameraDrag).WriteJump();
			_MemHdlr(LandscapeTextureLoad).WriteJump();
			_MemHdlr(PatchGetAsyncKeyStateA).WriteJump();
			_MemHdlr(PatchGetAsyncKeyStateB).WriteJump();
			_MemHdlr(UndoStackUndoOp3).WriteJump();
			_MemHdlr(UndoStackRedoOp3).WriteJump();
			_MemHdlr(MoveSelectionClampMul).WriteJump();

			for (int i = 0; i < 4; i++)
			{
				static const UInt32 kRenderWindowRefSelectionCallSites[4] =
				{
					0x0042AE3A, 0x0042AE7F,
					0x0042AEDB, 0x0042AF8B				// rubber band selection
				};

				_DefineCallHdlr(RerouteRenderWindowRefSelection, kRenderWindowRefSelectionCallSites[i], RenderWindowReferenceSelectionDetour);
				_MemHdlr(RerouteRenderWindowRefSelection).WriteCall();
			}
		}

		void __stdcall RenderWindowReferenceSelectionDetour( TESObjectREFR* Ref, bool ShowSelectionBox )
		{
			ReferenceSelectionManager::AddToSelection(Ref, ShowSelectionBox);
		}

		#define _hhName		DoorMarkerProperties
		_hhBegin()
		{
			_hhSetVar(Properties, 0x00429EAB);
			_hhSetVar(Teleport, 0x00429EE8);
			__asm
			{
				mov		eax, [esi + 0x8]
				shr		eax, 0x0E
				test	al, 1
				jnz		DOORMARKER

				jmp		_hhGetVar(Properties)
			TELEPORT:
				popad
				jmp		_hhGetVar(Teleport)
			DOORMARKER:
				pushad
				call	IsControlKeyDown
				test	eax, eax
				jz		TELEPORT
				popad

				jmp		_hhGetVar(Properties)
			}
		}

		void __stdcall DoTESObjectREFREditDialogHook(NiNode* Node, bool State)
		{
			// reset the current mouse-over ref, just in case it happens to be the temp ref the edit dlg created
			_RENDERWIN_XSTATE.CurrentMouseRef = nullptr;
		}

		#define _hhName		TESObjectREFRSetupDialog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00549A05);
			__asm
			{
				mov     eax, [edx + 0x180]
				mov     ecx, esi
				call    eax

				pushad
				push	1
				push	eax
				call	DoTESObjectREFREditDialogHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESObjectREFRCleanDialog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00549B57);
			__asm
			{
				push    edi
				mov     ecx, ebx
				call    edx

				pushad
				push	0
				push	eax
				call	DoTESObjectREFREditDialogHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlPerformFallVoid
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004270C9);
			_hhSetVar(Jump, 0x00427193);
			__asm
			{
				test	eax, eax
				jz		FIX

				mov		edx, [eax + 0x8]
				mov		[esp + 0x3C], edx

				jmp		_hhGetVar(Retn)
			FIX:
				jmp		_hhGetVar(Jump)
			}
		}

		void __stdcall DoForceShowTESObjectREFRDialogHook(HWND PropertiesDialog)
		{
			TESRenderWindow::Redraw();
			SetWindowPos(PropertiesDialog, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		}

		#define _hhName		ForceShowTESObjectREFRDialog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042EF86);
			__asm
			{
				pushad
				push	eax
				call	DoForceShowTESObjectREFRDialogHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		static float s_MovementSettingBuffer = 0.0;

		enum
		{
			kMovementSetting_RefMovementSpeed = 0,
			kMovementSetting_RefSnapGrid,
			kMovementSetting_RefRotationSpeed,
			kMovementSetting_RefSnapAngle,
			kMovementSetting_CamRotationSpeed,
			kMovementSetting_CamZoomSpeed,
			kMovementSetting_CamPanSpeed,
		};

		void __stdcall InitializeCurrentRenderWindowMovementSetting(UInt8 Setting)
		{
			switch (Setting)
			{
			case kMovementSetting_RefMovementSpeed:
				if (_RENDERWIN_XSTATE.UseAlternateMovementSettings)
					s_MovementSettingBuffer = settings::renderer::kAltRefMovementSpeed.GetData().f;
				else
					s_MovementSettingBuffer = *TESRenderWindow::RefMovementSpeed;

				break;
			case kMovementSetting_RefSnapGrid:
				if (_RENDERWIN_XSTATE.UseAlternateMovementSettings)
					s_MovementSettingBuffer = settings::renderer::kAltRefSnapGrid.GetData().f;
				else
					s_MovementSettingBuffer = *TESRenderWindow::SnapGridDistance;

				break;
			case kMovementSetting_RefRotationSpeed:
				if (_RENDERWIN_XSTATE.UseAlternateMovementSettings)
					s_MovementSettingBuffer = settings::renderer::kAltRefRotationSpeed.GetData().f;
				else
					s_MovementSettingBuffer = *TESRenderWindow::RefRotationSpeed;

				break;
			case kMovementSetting_RefSnapAngle:
				if (_RENDERWIN_XSTATE.UseAlternateMovementSettings)
					s_MovementSettingBuffer = settings::renderer::kAltRefSnapAngle.GetData().f;
				else
					s_MovementSettingBuffer = *TESRenderWindow::SnapAngle;

				break;
			case kMovementSetting_CamRotationSpeed:
				if (_RENDERWIN_XSTATE.UseAlternateMovementSettings)
					s_MovementSettingBuffer = settings::renderer::kAltCamRotationSpeed.GetData().f;
				else
					s_MovementSettingBuffer = *TESRenderWindow::CameraRotationSpeed;

				break;
			case kMovementSetting_CamZoomSpeed:
				if (_RENDERWIN_XSTATE.UseAlternateMovementSettings)
					s_MovementSettingBuffer = settings::renderer::kAltCamZoomSpeed.GetData().f;
				else
					s_MovementSettingBuffer = *TESRenderWindow::CameraZoomSpeed;

				break;
			case kMovementSetting_CamPanSpeed:
				if (_RENDERWIN_XSTATE.UseAlternateMovementSettings)
					s_MovementSettingBuffer = settings::renderer::kAltCamPanSpeed.GetData().f;
				else
					s_MovementSettingBuffer = *TESRenderWindow::CameraPanSpeed;

				break;
			default:
				s_MovementSettingBuffer = 0.0;

				break;
			}
		}

		#define _hhName		TESRenderControlAltSnapGrid
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00425A3E);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_RefSnapGrid);
			__asm	popad
			__asm
			{
				fild	s_MovementSettingBuffer
				fstp	dword ptr [esp + 0x20]

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltRefMovementSpeedA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00425741);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_RefMovementSpeed);
			__asm	popad
			__asm
			{
				fmul	s_MovementSettingBuffer
				lea		ecx, [esp + 0x28]

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltRefMovementSpeedB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042BE85);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_RefMovementSpeed);
			__asm	popad
			__asm
			{
				lea		ecx, s_MovementSettingBuffer

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltRefMovementSpeedC
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042D0B2);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_RefMovementSpeed);
			__asm	popad
			__asm
			{
				lea		ecx, s_MovementSettingBuffer

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltRefRotationSpeed
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00425DC1);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_RefRotationSpeed);
			__asm	popad
			__asm
			{
				fmul	s_MovementSettingBuffer

				mov		eax, [TESRenderWindow::StateFlags]
				mov		eax, [eax]
				test	eax, 0x2

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltRefSnapAngle
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00425DCD);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_RefSnapAngle);
			__asm	popad
			__asm
			{
				fild	s_MovementSettingBuffer

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltCamRotationSpeed
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042CCB0);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_CamRotationSpeed);
			__asm	popad
			__asm
			{
				lea		ecx, s_MovementSettingBuffer

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltCamZoomSpeedA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042CCE5);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_CamZoomSpeed);
			__asm	popad
			__asm
			{
				lea		ecx, s_MovementSettingBuffer

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltCamZoomSpeedB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042CDB4);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_CamZoomSpeed);
			__asm	popad
			__asm
			{
				lea		ecx, s_MovementSettingBuffer

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltCamPanSpeedA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042CD2B);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_CamPanSpeed);
			__asm	popad
			__asm
			{
				lea		ecx, s_MovementSettingBuffer

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlAltCamPanSpeedB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042CD76);
			__asm	pushad
			InitializeCurrentRenderWindowMovementSetting(kMovementSetting_CamPanSpeed);
			__asm	popad
			__asm
			{
				lea		ecx, s_MovementSettingBuffer

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderControlRedrawGrid
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042EF88);
			_asm	pushad
			TESRenderWindow::Redraw(true);
			SetActiveWindow(*TESCSMain::WindowHandle);
			SetActiveWindow(*TESRenderWindow::WindowHandle);
			SetForegroundWindow(*TESRenderWindow::WindowHandle);
			__asm
			{
				popad
				mov		eax, 1
				jmp		_hhGetVar(Retn)
			}
		}

		UInt32 __stdcall DoTESPreviewControlCallWndProcHook(void* OrgWindowProc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (OrgWindowProc)
				return CallWindowProc((WNDPROC)OrgWindowProc, hWnd, uMsg, wParam, lParam);
			else
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		#define _hhName		TESPreviewControlCallWndProc
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0044D70E);
			__asm
			{
				push    ebx
				push    ebp
				push    edi
				push    esi
				push	eax
				call	DoTESPreviewControlCallWndProcHook

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall ActivateRenderWindow(void)
		{
			if (TESLODTextureGenerator::GeneratorState == TESLODTextureGenerator::kState_NotInUse)
				SetForegroundWindow(*TESRenderWindow::WindowHandle);
		}

		#define _hhName		ActivateRenderWindowPostLandTextureChange
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042B4EB);
			__asm
			{
				pushad
				call	IATCacheSendMessageAddress
				popad

				call	IATProcBuffer
				pushad
				call	ActivateRenderWindow
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		static bool s_PathGridMoveStart = false;

		#define _hhName		TESPathGridRecordOperationMoveA
		_hhBegin()
		{
			_hhSetVar(Call, 0x0054D600);
			_hhSetVar(Retn, 0x0042A632);
			__asm
			{
				mov		s_PathGridMoveStart, 1
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESPathGridRecordOperation(void)
		{
			_RENDERWIN_MGR.GetPathGridUndoManager()->ResetRedoStack();

			if (TESRenderWindow::SelectedPathGridPoints->Count())
				_RENDERWIN_MGR.GetPathGridUndoManager()->RecordOperation(PathGridUndoManager::kOperation_DataChange, TESRenderWindow::SelectedPathGridPoints);
		}

		void __stdcall DoTESPathGridRecordOperationMoveBHook(void)
		{
			if (s_PathGridMoveStart)
			{
				s_PathGridMoveStart = false;
				DoTESPathGridRecordOperation();
			}
		}

		#define _hhName		TESPathGridRecordOperationMoveB
		_hhBegin()
		{
			_hhSetVar(Call, 0x004FC950);
			_hhSetVar(Retn, 0x0042BE72);
			__asm
			{
				pushad
				call	DoTESPathGridRecordOperationMoveBHook
				popad
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESPathGridRecordOperationLink
		_hhBegin()
		{
			_hhSetVar(Call, 0x00405DA0);
			_hhSetVar(Retn, 0x0042A82E);
			__asm
			{
				pushad
				call	DoTESPathGridRecordOperation
				popad
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESPathGridRecordOperationFlag
		_hhBegin()
		{
			_hhSetVar(Call, 0x005557A0);
			_hhSetVar(Retn, 0x0042A719);
			__asm
			{
				pushad
				call	DoTESPathGridRecordOperation
				popad
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESPathGridRecordOperationRef
		_hhBegin()
		{
			_hhSetVar(Call, 0x00405DA0);
			_hhSetVar(Retn, 0x0042836C);
			__asm
			{
				pushad
				call	DoTESPathGridRecordOperation
				popad
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESPathGridDeletePointHook(void)
		{
			_RENDERWIN_MGR.GetPathGridUndoManager()->ResetRedoStack();
			_RENDERWIN_MGR.GetPathGridUndoManager()->HandlePathGridPointDeletion(TESRenderWindow::SelectedPathGridPoints);

			if (TESRenderWindow::SelectedPathGridPoints->Count())
				_RENDERWIN_MGR.GetPathGridUndoManager()->RecordOperation(PathGridUndoManager::kOperation_PointDeletion, TESRenderWindow::SelectedPathGridPoints);
		}

		#define _hhName		TESPathGridDeletePoint
		_hhBegin()
		{
			_hhSetVar(Call, 0x0048E0E0);
			_hhSetVar(Retn, 0x004291CB);
			__asm
			{
				pushad
				call	DoTESPathGridDeletePointHook
				popad
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESPathGridPointDtorHook(TESPathGridPoint* Point)
		{
			PathGridPointListT* DeletionList = (PathGridPointListT*)PathGridPointListT::Create(&FormHeap_Allocate);
			DeletionList->AddAt(Point, eListEnd);
			_RENDERWIN_MGR.GetPathGridUndoManager()->HandlePathGridPointDeletion(DeletionList);
			DeletionList->RemoveAll();
			FormHeap_Free(DeletionList);

			_RENDERWIN_XSTATE.CurrentMousePathGridPoint = nullptr;
		}

		#define _hhName		TESPathGridPointDtor
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00556197);
			__asm
			{
				mov		eax, [esp]
				sub		eax, 5
				cmp		eax, 0x0054E5A3
				jnz		CULL

				mov		eax, [esp + 0x18]
				sub		eax, 5
				cmp		eax, 0x00429200		// don't handle deletion if called from the render window wnd proc, as we already do that in the previous hook
				jz		SKIP
			CULL:
				pushad
				push	ecx
				call	DoTESPathGridPointDtorHook
				popad
			SKIP:
				push    ebx
				push    esi
				mov     esi, ecx
				lea     ecx, [esi + 0x10]
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESPathGridToggleEditModeHook(void)
		{
			_RENDERWIN_MGR.GetPathGridUndoManager()->ResetRedoStack();
			_RENDERWIN_MGR.GetPathGridUndoManager()->ResetUndoStack();
		}

		#define _hhName		TESPathGridToggleEditMode
		_hhBegin()
		{
			_hhSetVar(Call, 0x0054C560);
			_hhSetVar(Retn, 0x00550665);
			__asm
			{
				pushad
				call	DoTESPathGridToggleEditModeHook
				popad
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESPathGridCreateNewLinkedPointHook(void)
		{
			_RENDERWIN_MGR.GetPathGridUndoManager()->ResetRedoStack();

			if (TESRenderWindow::SelectedPathGridPoints->Count())
				_RENDERWIN_MGR.GetPathGridUndoManager()->RecordOperation(PathGridUndoManager::kOperation_PointCreation, TESRenderWindow::SelectedPathGridPoints);
		}

		#define _hhName		TESPathGridCreateNewLinkedPoint
		_hhBegin()
		{
			_hhSetVar(Call, 0x004E3900);
			_hhSetVar(Retn, 0x0042B380);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				call	DoTESPathGridCreateNewLinkedPointHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESPathGridShowMultipleSelectionRingHook(TESPathGridPoint* Point)
		{
			Point->ShowSelectionRing();
		}

		#define _hhName		TESPathGridShowMultipleSelectionRing
		_hhBegin()
		{
			_hhSetVar(Call, 0x004E3900);
			_hhSetVar(Retn, 0x0042FC81);
			__asm
			{
				pushad
				push	esi
				call	DoTESPathGridShowMultipleSelectionRingHook
				popad
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESPathGridPerformFall
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00428610);
			__asm
			{
				mov		ebx, TESRenderWindow::SelectedPathGridPoints
				lea		ebx, [ebx]

				pushad
				call	DoTESPathGridRecordOperation
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoInitialCellLoadCameraPositionHook(TESObjectCELL* Cell)
		{
			static const Vector3 kDepletedMan(0.0, 0.0, 0.0), kNotSoDepletedMan(0.0, 0.0, -8192.0);

			if (Cell->objectList.Count())
			{
				TESObjectREFR* FirstRef = nullptr;

				for (TESObjectCELL::ObjectREFRList::Iterator Itr = Cell->objectList.Begin(); Itr.End() == false && Itr.Get(); ++Itr)
				{
					if (FirstRef == nullptr)
						FirstRef = Itr.Get();
					else if ((FirstRef->formID & 0xFFFFFF) > (Itr->formID & 0xFFFFFF))
						FirstRef = Itr.Get();
				}

				if (FirstRef)
				{
					_TES->LoadCellIntoViewPort(&kDepletedMan, FirstRef);
					_RENDERSEL->ClearSelection(true);
					return;
				}
			}

			SendMessage(*TESRenderWindow::WindowHandle, 0x40D, NULL, (LPARAM)&kNotSoDepletedMan);
		}

		#define _hhName		InitialCellLoadCameraPosition
		_hhBegin()
		{
			_hhSetVar(Call, 0x00532240);
			_hhSetVar(Retn, 0x0040A8B7);
			_hhSetVar(Jump, 0x0040A8D8);
			__asm
			{
				call	_hhGetVar(Call)
				test	al, al
				jnz		FIX

				jmp		_hhGetVar(Retn)
			FIX:
				push	ecx
				call	DoInitialCellLoadCameraPositionHook
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		ConvertNiRenderedTexToD3DBaseTex
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004116A5);
			__asm
			{
				push	esi		// store IDirect3DBaseTexture9*

				mov     esi, [esp + 0x30]
				lea     eax, [esi + 4]
				push    eax
				pushad
				call	IATCacheInterlockedDecrementAddress
				popad
				call	IATProcBuffer
				test    eax, eax
				jnz     EXIT
				mov     edx, [esi]
				mov     eax, [edx]
				push    1
				mov     ecx, esi
				call    eax
			EXIT:
				pop		esi		// restore
				mov		eax, esi
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoDuplicateReferencesHook(void)
		{
			if (settings::renderer::kZOffsetDuplicatedRefs().i)
			{
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Reference = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					Reference->SetPosition(Reference->position.x, Reference->position.y, Reference->position.z + 10.0f);
				}
			}
		}

		#define _hhName		DuplicateReferences
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042EC6C);
			__asm
			{
				pushad
				call	DoDuplicateReferencesHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESRenderControlPerformRelativeScaleHook(TESObjectREFR* Ref, float Scale, bool Relative)
		{
			if (Relative == false)
				Ref->SetScale(Scale);
			else
			{
				if (Scale > 10.0f)
					Scale = 10.0;
				else if (Scale < 0.01)
					Scale = 0.01;

				Vector3 PositionOffset(_RENDERSEL->selectionPositionVectorSum.x - Ref->position.x,
									_RENDERSEL->selectionPositionVectorSum.y - Ref->position.y,
									_RENDERSEL->selectionPositionVectorSum.z - Ref->position.z);

				float ScaleFactor = (Ref->scale - Scale)/Ref->scale;

				Ref->scale = Scale;
				Ref->position.x += PositionOffset.x * ScaleFactor;
				Ref->position.y += PositionOffset.y * ScaleFactor;
				Ref->position.z += PositionOffset.z * ScaleFactor;
				Ref->UpdateNiNode();
			}
		}

		#define _hhName		TESRenderControlPerformRelativeScale
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00424807);
			__asm
			{
				pop		ecx
				pushad
				movzx	eax, bl
				push	eax
				push	ecx
				push	esi
				call	DoTESRenderControlPerformRelativeScaleHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		bool __stdcall DoTESPathGridRubberBandSelectionHook(TESRenderWindow::RubberBandSelection* Selector)
		{
			return Selector->hasSelection;
		}

		#define _hhName		TESPathGridRubberBandSelection
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042FBE8);
			__asm
			{
				pushad
				push	ecx
				call	DoTESPathGridRubberBandSelectionHook
				test	al, al
				jz		SKIP
				popad

				push	ecx
				mov		[esp], ecx
				mov		ecx, [esp + 0x8]

				jmp		_hhGetVar(Retn)
			SKIP:
				popad
				retn	8
			}
		}

		bool __stdcall DoCoplanarRefDropHook(NiCamera* Camera, int XCoord, int YCoord, Vector3* OutPosition, Vector3* OutRotation)
		{
			bool Result = true;

			thisCall<bool>(0x006FF1A0, Camera, XCoord, YCoord, OutPosition, OutRotation);
			Vector3 PosBuf(*OutPosition), RotBuf(*OutRotation);

			if (settings::renderer::kCoplanarRefDrops.GetData().i)
			{
				// perform the necessary (nose)picking nonsense
				thisCall<void>(0x00417C40, 0x00A0BC64, _TES->sceneGraphObjectRoot);
				if (thisCall<bool>(0x005E6030, 0x00A0BC64, OutPosition, OutRotation, 0))
				{
					// sacrilege! SACRILEGE!!
					float*** NewPosition = (float***)0x00A0BC80;
					OutPosition->x = *(float*)((UInt32)**NewPosition + 0x8);
					OutPosition->y = *(float*)((UInt32)**NewPosition + 0xC);
					OutPosition->z = *(float*)((UInt32)**NewPosition + 0x10);
					OutRotation->Scale(0.0f);

					Result = false;
				}

				if (_TES->currentInteriorCell == nullptr)
				{
					TESWorldSpace* CurrentWorldspace = _TES->currentWorldSpace;
					if (CurrentWorldspace == nullptr || _DATAHANDLER->GetExteriorCell(OutPosition->x, OutPosition->y, CurrentWorldspace) == nullptr)
					{
						*OutPosition = PosBuf;
						*OutRotation = RotBuf;
					}
				}
			}

			return Result;
		}

		#define _hhName		CoplanarRefDrop
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042DE37);
			__asm
			{
				push	ecx
				xor		eax, eax
				call	DoCoplanarRefDropHook
				mov		ecx, 0x00A0E064
				mov		ecx, [ecx]
				push	eax
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoInitPathGridNodeSelectionRingHook(void)
		{
			for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
				Itr.Get()->selected = 0;
		}

		#define _hhName		InitPathGridNodeSelectionRing
		_hhBegin()
		{
			_hhSetVar(Call, 0x00405DC0);
			_hhSetVar(Retn, 0x00419AFF);
			__asm
			{
				pushad
				call	DoInitPathGridNodeSelectionRingHook
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESLoadIntoViewPort
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00430F61);
			__asm
			{
				push	ebx
				mov		ebx, [esp + 0xC]
				test	ebx, ebx
				jnz		TESTCELL
			ENDTEST:
				jmp		_hhGetVar(Retn)
			TESTCELL:
				mov		eax, [ebx + 0x48]
				test	eax, eax
				jnz		ENDTEST
				pop		ebx
				retn
			}
		}

		#define _hhName		RenderWindowAxisHotkeysMoveReferences
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042CB81);
			__asm
			{
				push	edx
				push	eax			// swap x/y key states, stored in al and cl respectively
				push	ecx
				mov		eax, 0x00A0BACC
				mov		eax, [eax]
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		RenderWindowAxisHotkeysMovePathGridPoints
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042BF1E);
			__asm
			{
				push	eax			// swap x/y key states, stored in al and cl respectively
				push	ecx
				push	ebx
				push	edi
				push	ecx
				mov		ecx, esi
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoBSFadeNodeDrawTransparencyHook(BSFadeNode* FadeNode, float* OutAlpha)
		{
			if ((FadeNode->m_flags & TESObjectREFR::kNiNodeSpecialFlags_SpecialFade))
				*OutAlpha = FadeNode->fCurrentAlpha;
		}

		#define _hhName		BSFadeNodeDrawTransparency
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004BC52E);
			__asm
			{
				mov		eax, esp
				add		eax, 0x2C

				pushad
				push	eax
				push	esi
				call	DoBSFadeNodeDrawTransparencyHook
				popad

				cmp		byte ptr [esi + 0xDC], 0
				jmp		_hhGetVar(Retn)
			}
		}



		void __stdcall DoTESPathGridPointGenerateNiNodeA(NiLines* Connector)
		{
			if (settings::renderer::kPathGridLinkedRefIndicator().i == 0)
			{
				if ((settings::renderer::kPathGridLinkedRefIndicatorFlags().u & settings::renderer::kPathGridLinkedRefIndicatorFlag_HideLineConnector))
					Connector->SetCulled(true);
			}
		}

		#define _hhName		TESPathGridPointGenerateNiNodeA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0055677E);
			_hhSetVar(Call, 0x004AD0C0);
			__asm
			{
				call	_hhGetVar(Call)

				pushad
				push	eax
				call	DoTESPathGridPointGenerateNiNodeA
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESPathGridPointGenerateNiNodeB(NiTriShape* BoundingBox)
		{
			if (settings::renderer::kPathGridLinkedRefIndicator().i == 0)
			{
				if ((settings::renderer::kPathGridLinkedRefIndicatorFlags().u & settings::renderer::kPathGridLinkedRefIndicatorFlag_HidePointBoundingBox))
					BoundingBox->SetCulled(true);
			}
		}

		#define _hhName		TESPathGridPointGenerateNiNodeB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x005567E0);
			_hhSetVar(Call, 0x004ACC30);
			__asm
			{
				call	_hhGetVar(Call)

				pushad
				push	eax
				call	DoTESPathGridPointGenerateNiNodeB
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESPathGridGenerateNiNode(NiTriShape* RefNode)
		{
			if (settings::renderer::kPathGridLinkedRefIndicator().i == 0)
			{
				if ((settings::renderer::kPathGridLinkedRefIndicatorFlags().u & settings::renderer::kPathGridLinkedRefIndicatorFlag_HideLinkedRefNode))
				{
					RefNode->SetCulled(true);
				}
			}
		}

		#define _hhName		TESPathGridGenerateNiNode
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0054EE12);
			_hhSetVar(Call, 0x004AE0F0);
			__asm
			{
				call	_hhGetVar(Call)

				pushad
				push	eax
				call	DoTESPathGridGenerateNiNode
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		CenterCameraOnRefSelection
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00428E3A);
			_hhSetVar(Call, 0x0088DC0C);
			_hhSetVar(Jump, 0x0042EF86);
			__asm
			{
				call	_hhGetVar(Call)

				test	eax, eax
				jz		SKIP

				jmp		_hhGetVar(Retn)
			SKIP:
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		TopCameraOnRefSelection
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00428FBD);
			_hhSetVar(Call, 0x0088DC0C);
			_hhSetVar(Jump, 0x0042EF86);
			__asm
			{
				call	_hhGetVar(Call)

				test	eax, eax
				jz		SKIP

				jmp		_hhGetVar(Retn)
			SKIP:
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		TESRenderRotateSelectionWorldA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00425F32);
			_hhSetVar(Jump, 0x0042604F);
			__asm
			{
				cmp		eax, 1
				jz		LOCAL

				test	bl, bl
				jz		LOCAL

				jmp		_hhGetVar(Jump)
			LOCAL:
				// the vanilla local rotation code only rotates the first ref in the selection, so that needs fixing as well
				mov		ecx, 0x00A0AF60
				mov		eax, [ecx]
				mov		eax, [eax]
				mov		[esp + 0x30], eax			// save the first entry to an unused location on the stack

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESRenderRotateSelectionWorldB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00426048);
			_hhSetVar(Call, 0x0053FC70);
			_hhSetVar(Jump, 0x00425F32);
			__asm
			{
				call	_hhGetVar(Call)

				mov		eax, [esp + 0x30]			// restore the selection entry we stored earlier and check if we have more refs to process
				mov		eax, [eax + 0x8]

				test	eax, eax
				jz		END

				mov		[esp + 0x30], eax			// update our variable on the stack and jump back
				xor		esi, esi					// ensure ESI is zero as it's supposed to be non-volatile
				jmp		_hhGetVar(Jump)
			END:
				jmp		_hhGetVar(Retn)
			}
		}

		bool __stdcall UseCustomPivot(Vector3* OutPivot, UInt8* AlternatePivot)
		{
			if (settings::renderer::kFixedCameraPivot.GetData().i)
			{
				*OutPivot = _RENDERWIN_XSTATE.UpdateStaticCameraPivot();
				return true;
			}

			return false;
		}

		#define _hhName		RotateCameraDrag
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0042CC04);
			_hhSetVar(Jump, 0x0042C919);
			_hhSetVar(CustomPivot, 0x0042CCA3);
			__asm
			{
				mov		ecx, 0x00A0BBDC
				mov		cl, byte ptr [ecx]
				test	cl, cl
				jz		NODRAG

				jmp		_hhGetVar(Jump)
			NODRAG:
				// skip rotation if the Control key is held down
				pushad
				call	IsControlKeyDown
				test	eax, eax
				jnz		EXIT
				popad

				// switch to the static pivot if need be
				lea		eax, [esp + 0x1C]
				push	0x00A0BC21
				push	eax
				call	UseCustomPivot
				test	al, al
				jnz		USECUSTOMPIVOT

				mov		ecx, 0x00A0BC21
				mov		cl, byte ptr[ecx]
				cmp		cl, 0

				jmp		_hhGetVar(Retn)
			USECUSTOMPIVOT:
				jmp		_hhGetVar(CustomPivot)
			EXIT:
				popad
				jmp		_hhGetVar(Jump)
			}
		}

		NiSourceTexture* __stdcall DoLandscapeTextureLoad(TESLandTexture* Texture)
		{
			if (_RENDERWIN_XSTATE.UseGrassTextureOverlay && Texture->potentialGrassList.Count())
			{
				if (_RENDERWIN_XSTATE.GrassOverlayTexture)
					return _RENDERWIN_XSTATE.GrassOverlayTexture;
				else
					return Texture->sourceTexture;
			}
			else
				return Texture->sourceTexture;
		}

		#define _hhName		LandscapeTextureLoad
		_hhBegin()
		{
			_hhSetVar(Retn, 0x005232E6);
			__asm
			{
				push	ecx
				push	ecx
				call	DoLandscapeTextureLoad
				test	eax, eax

				jmp		_hhGetVar(Retn)
			}
		}

		SHORT WINAPI GetAsyncKeyStateOverride(int vKey)
		{
			// we only need to handle Alt here as the rest are handled by the input manager
			SME_ASSERT(vKey == VK_MENU);

			UInt8 Modifier = renderWindow::input::BuiltIn::kModifier_Alt;
			if (renderWindow::input::BuiltIn::ModifierOverride::Instance.IsActive(Modifier))
			{
				SHORT Result = 0;
				if (renderWindow::input::BuiltIn::ModifierOverride::Instance.GetOverrideState(Modifier))
					Result |= 1 << 15;

				return Result;
			}

			// key not overridden, call the original function
			return GetAsyncKeyState(vKey);
		}

		#define _hhName		PatchGetAsyncKeyStateA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00427541);
			__asm
			{
				mov		esi, GetAsyncKeyStateOverride
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		UndoStackUndoOp3
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00431D9F);
			_hhSetVar(Jump, 0x004320DD);
			__asm
			{
				mov		eax, TESRenderWindow::UndoBuffer
				mov		eax, [eax]
				cmp		[eax], esi
				jz		BOOKEND

				mov		eax, [esi + 0x8]
				cmp		byte ptr [eax + 0x4], 0x31
				jmp		_hhGetVar(Retn)
			BOOKEND:
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		UndoStackRedoOp3
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00432901);
			_hhSetVar(Jump, 0x00432C19);
			__asm
			{
				cmp		esi, 0
				jz		BOOKEND

				mov		eax, TESRenderWindow::UndoBuffer
				mov		eax, [eax]
				cmp		[eax], esi
				jz		BOOKEND

				mov		eax, [esi + 0x8]
				cmp		byte ptr [eax + 0x4], 0x31
				jmp		_hhGetVar(Retn)
			BOOKEND:
				jmp		_hhGetVar(Jump)
			}
		}

		void __stdcall ClampMovementMulti(float* CameraDist)
		{
			if (*CameraDist > 5000.f)
				*CameraDist = 5000.f;
		}

		#define _hhName		MoveSelectionClampMul
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00425734);
			__asm
			{
				fstp    dword ptr[esp + 0x24]

				lea		eax, [esp + 0x24]
				push	eax
				call	ClampMovementMulti

				fld		dword ptr[esp + 0x24]
				jmp		_hhGetVar(Retn)
			}
		}
	}
}

#pragma warning(pop)