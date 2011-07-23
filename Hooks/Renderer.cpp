#include "Renderer.h"
#include "..\RenderSelectionGroupManager.h"
#include "..\ElapsedTimeCounter.h"
#include "..\RenderWindowTextPainter.h"
#include "..\CSDialogs.h"

#pragma warning(disable: 4410)		// illegal operand size; fild instruction

namespace Hooks
{
	#define PI					3.151592653589793

	TESForm*					g_TESObjectREFRUpdate3DBuffer = NULL;
	bool						g_RenderWindowAltMovementSettings = false;

	_DefineHookHdlr(DoorMarkerProperties, 0x00429EA1);
	_DefineHookHdlr(TESObjectREFRGet3DData, 0x00542950);
	_DefineHookHdlr(NiWindowRender, 0x00406442);
	_DefineHookHdlr(NiDX9RendererRecreate, 0x006D7260);
	_DefineHookHdlr(RenderWindowStats, 0x0042D3F4);
	_DefineHookHdlr(UpdateViewport, 0x0042CE70);
	_DefineHookHdlr(RenderWindowSelection, 0x0042AE71);
	_DefineHookHdlr(TESRenderControlPerformMove, 0x00425670);
	_DefineHookHdlr(TESRenderControlPerformRotate, 0x00425D6E);
	_DefineHookHdlr(TESRenderControlPerformScale, 0x00424650);
	_DefineHookHdlr(TESRenderControlPerformFall, 0x0042886A);
	_DefineHookHdlr(TESObjectREFRSetupDialog, 0x005499FB);
	_DefineHookHdlr(TESObjectREFRCleanDialog, 0x00549B52);
	_DefineHookHdlr(TESRenderControlPerformFallVoid, 0x004270C2);
	_DefineHookHdlrWithBuffer(TESObjectREFRUpdate3D, 0x00549AC5, 5, 0x56, 0x8B, 0x74, 0x24, 0x34);
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

	void PatchRendererHooks(void)
	{
		_MemHdlr(DoorMarkerProperties).WriteJump();
		_MemHdlr(TESObjectREFRGet3DData).WriteJump();
		_MemHdlr(NiWindowRender).WriteJump();
		_MemHdlr(NiDX9RendererRecreate).WriteJump();
		_MemHdlr(RenderWindowStats).WriteJump();
		_MemHdlr(UpdateViewport).WriteJump();
		_MemHdlr(RenderWindowSelection).WriteJump();
		_MemHdlr(TESRenderControlPerformMove).WriteJump();
		_MemHdlr(TESRenderControlPerformRotate).WriteJump();
		_MemHdlr(TESRenderControlPerformScale).WriteJump();
		_MemHdlr(TESRenderControlPerformFall).WriteJump();
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

			jmp		[_hhGetVar(Properties)]
		TELEPORT:
			popad
			jmp		[_hhGetVar(Teleport)]
		DOORMARKER:
			pushad
			call	IsControlKeyDown
			test	eax, eax
			jz		TELEPORT
			popad

			jmp		[_hhGetVar(Properties)]
		}
	}

	void __stdcall DoTESObjectREFRGet3DDataHook(TESObjectREFR* Object, NiNode* Node)
	{
		if ((Node->m_flags & kNiNodeSpecialFlags_DontUncull))
			return;

		ToggleFlag(&Node->m_flags, NiNode::kFlag_AppCulled, false);

		BSExtraData* xData = Object->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
		if (xData)
		{
			ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
			if ((xParent->parent->formFlags & kTESObjectREFRSpecialFlags_Children3DInvisible))
				ToggleFlag(&Node->m_flags, NiNode::kFlag_AppCulled, true);
		}

		if ((Object->formFlags & kTESObjectREFRSpecialFlags_3DInvisible))
			ToggleFlag(&Node->m_flags, NiNode::kFlag_AppCulled, true);
	}

	#define _hhName		TESObjectREFRGet3DData
	_hhBegin()
	{
		_hhSetVar(Call, 0x0045B1B0);
		__asm
		{
			push	esi
			push	ecx		// store
			push	0x56
			add		ecx, 0x4C
			xor		esi, esi
			call	[_hhGetVar(Call)]
			test	eax, eax
			jz		NO3DDATA

			mov		eax, [eax + 0xC]
			pop		ecx		// restore
			push	ecx		// store again for epilog

			pushad
			push	eax
			push	ecx
			call	DoTESObjectREFRGet3DDataHook
			popad
			jmp		EXIT
		NO3DDATA:
			mov		eax, esi
		EXIT:
			pop		ecx
			pop		esi
			retn
		}
	}

	void __stdcall NiWindowRenderDrawHook(void)
	{
		g_RenderWindowTimeManager.Update();
		RENDERTEXT->Render();
	}

	#define _hhName		NiWindowRender
	_hhBegin()
	{
		_hhSetVar(Call, 0x0076A3B0);
		_hhSetVar(Retn, 0x00406447);
		__asm
		{
			call	[_hhGetVar(Call)]

			pushad
			call	NiWindowRenderDrawHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoNiDX9RendererRecreateHook(void)
	{
		RENDERTEXT->Recreate();
	}

	#define _hhName		NiDX9RendererRecreate
	_hhBegin()
	{
		_hhSetVar(Retn, 0x006D7266);
		__asm
		{
			pushad
			call	DoNiDX9RendererRecreateHook
			popad

			sub     esp, 0x10
			push    ebx
			push    ebp
			push    esi

			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoRenderWindowStatsHook(void)
	{
		if (g_INIManager->GetINIInt("DisplaySelectionStats"))
		{
			if ((*g_TESRenderSelectionPrimary)->selectionCount > 1)
			{
				PrintToBuffer("%d Objects Selected\nPosition Vector Sum: %.04f, %.04f, %.04f",
							(*g_TESRenderSelectionPrimary)->selectionCount,
							(*g_TESRenderSelectionPrimary)->selectionPositionVectorSum.x,
							(*g_TESRenderSelectionPrimary)->selectionPositionVectorSum.y,
							(*g_TESRenderSelectionPrimary)->selectionPositionVectorSum.z);
				RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_1, g_TextBuffer, 0);
			}
			else if ((*g_TESRenderSelectionPrimary)->selectionCount)
			{
				TESObjectREFR* Selection = CS_CAST((*g_TESRenderSelectionPrimary)->selectionList->Data, TESForm, TESObjectREFR);
				char Buffer[0x50] = {0};
				sprintf_s(Buffer, 0x50, "");

				BSExtraData* xData = Selection->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
				if (xData)
				{
					ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
					sprintf_s(Buffer, 0x50, "Parent: %s [%08X]  Opposite State: %d",
																	((xParent->parent->editorID.Size())?(xParent->parent->editorID.c_str()):("")),
																	xParent->parent->formID, (UInt8)xParent->oppositeState);
				}
				PrintToBuffer("%s (%08X) BASE[%s (%08X)]\nP[%.04f, %.04f, %.04f]\nR[%.04f, %.04f, %.04f]\nS[%.04f]\nFlags: %s %s %s %s %s %s\n%s",
								((Selection->editorID.Size())?(Selection->editorID.c_str()):("")), Selection->formID,
								((Selection->baseForm->editorID.Size())?(Selection->baseForm->editorID.c_str()):("")), Selection->baseForm->formID,
								Selection->position.x, Selection->position.y, Selection->position.z,
								Selection->rotation.x * 180 / PI,
								Selection->rotation.x * 180 / PI,
								Selection->rotation.x * 180 / PI,
								Selection->scale,
								((Selection->formFlags & TESForm::kFormFlags_QuestItem)?("P"):("-")),
								((Selection->formFlags & TESForm::kFormFlags_Disabled)?("D"):("-")),
								((Selection->formFlags & TESForm::kFormFlags_VisibleWhenDistant)?("V"):("-")),
								((Selection->formFlags & kTESObjectREFRSpecialFlags_3DInvisible)?("I"):("-")),
								((Selection->formFlags & kTESObjectREFRSpecialFlags_Children3DInvisible)?("CI"):("-")),
								((Selection->formFlags & kTESObjectREFRSpecialFlags_Frozen)?("F"):("-")),
								Buffer);

				RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_1, g_TextBuffer, 0);
			}
			else
				RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_1, NULL, 0);
		}
		else
			RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_1, NULL, 0);
	}

	#define _hhName		RenderWindowStats
	_hhBegin()
	{
		_hhSetVar(Call, 0x006F25E0);
		_hhSetVar(Retn, 0x0042D3F9);
		__asm
		{
			call	[_hhGetVar(Call)]

			pushad
			call	DoRenderWindowStatsHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	bool __stdcall DoUpdateViewportHook(void)
	{
		if (RENDERTEXT->GetRenderChannelQueueSize(RenderWindowTextPainter::kRenderChannel_2) || g_INIManager->GetINIInt("UpdateViewPortAsync"))
			return true;
		else
			return false;
	}

	#define _hhName		UpdateViewport
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042EF86);
		_hhSetVar(Jump, 0x0042CE7D);
		__asm
		{
			mov		eax, [g_RenderWindowUpdateViewPortFlag]
			mov		eax, [eax]
			cmp		al, 0
			jz		DONTUPDATE

			jmp		[_hhGetVar(Jump)]
		DONTUPDATE:
			pushad
			xor		eax, eax
			call	DoUpdateViewportHook
			test	al, al
			jz		EXIT

			popad
			jmp		[_hhGetVar(Jump)]
		EXIT:
			popad
			jmp		[_hhGetVar(Retn)]
		}
	}

	bool __stdcall DoRenderWindowSelectionHook(TESObjectREFR* Ref)
	{
		bool Result = false;

		TESObjectCELL* CurrentCell = (*g_TES)->currentInteriorCell;
		if (CurrentCell == NULL)
			CurrentCell = (*g_TES)->currentExteriorCell;

		if (CurrentCell)
		{
			TESRenderSelection* Selection = g_RenderSelectionGroupManager.GetRefSelectionGroup(Ref, CurrentCell);
			if (Selection)
			{
				for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					(*g_TESRenderSelectionPrimary)->AddToSelection(Itr->Data, true);

				RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_2, "Selected object selection group", 3);
				Result = true;
			}
		}

		return Result;
	}

	#define _hhName		RenderWindowSelection
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042AE76);
		_hhSetVar(Jump, 0x0042AE84);
		_hhSetVar(Call, 0x00511C20);
		__asm
		{
			call	[_hhGetVar(Call)]
			xor		eax, eax

			pushad
			push	esi
			call	DoRenderWindowSelectionHook
			test	al, al
			jnz		GROUPED
			popad

			jmp		[_hhGetVar(Retn)]
		GROUPED:
			popad
			jmp		[_hhGetVar(Jump)]
		}
	}

	void __stdcall TESRenderControlProcessFrozenRefs(void)
	{
		std::vector<TESForm*> FrozenRefs;
		for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->selectionList; Itr && Itr->Data; Itr = Itr->Next)
		{
			if ((Itr->Data->formFlags & kTESObjectREFRSpecialFlags_Frozen))
				FrozenRefs.push_back(Itr->Data);
		}

		for (std::vector<TESForm*>::const_iterator Itr = FrozenRefs.begin(); Itr != FrozenRefs.end(); Itr++)
			(*g_TESRenderSelectionPrimary)->AddToSelection(*Itr, true);
	}

	#define _hhName		TESRenderControlPerformMove
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00425676);
		__asm
		{
			sub		esp, 0x114
			pushad
			call	TESRenderControlProcessFrozenRefs
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlPerformRotate
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00425D74);
		__asm
		{
			sub		esp, 0xC0
			pushad
			call	TESRenderControlProcessFrozenRefs
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlPerformScale
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00424659);
		__asm
		{
			sub		esp, 0x40
			mov		ecx, [g_TESRenderSelectionPrimary]
			mov		ecx, [ecx]

			pushad
			call	TESRenderControlProcessFrozenRefs
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlPerformFall
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042886F);
		_hhSetVar(Call, 0x00512990);
		__asm
		{
			pushad
			call	TESRenderControlProcessFrozenRefs
			popad

			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoTESObjectREFREditDialogHook(NiNode* Node, bool State)
	{
		ToggleFlag(&Node->m_flags, kNiNodeSpecialFlags_DontUncull, State);
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

			jmp		[_hhGetVar(Retn)]
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

			jmp		[_hhGetVar(Retn)]
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

			jmp		[_hhGetVar(Retn)]
		FIX:
			jmp		[_hhGetVar(Jump)]
		}
	}

	#define _hhName		TESObjectREFRUpdate3D
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00549B2E);
		__asm
		{
			push	esi
			mov		ebp, ecx
			mov		ebx, g_TESObjectREFRUpdate3DBuffer

			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoForceShowTESObjectREFRDialogHook(HWND PropertiesDialog)
	{
		SendMessage(*g_HWND_RenderWindow, WM_TIMER, (WPARAM)1, NULL);	// update viewport
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

			jmp		[_hhGetVar(Retn)]
		}
	}

	static float s_MovementSettingBuffer = 0.0;

	void __stdcall InitializeCurrentRenderWindowMovementSetting(const char* SettingName)
	{
		if (g_RenderWindowAltMovementSettings)
			s_MovementSettingBuffer = g_INIManager->GetINIFlt((std::string("Alt" + std::string(SettingName)).c_str()));
		else
		{
			if (!_stricmp(SettingName, "RefMovementSpeed"))
				s_MovementSettingBuffer = *g_RenderWindowRefMovementSpeed;
			else if (!_stricmp(SettingName, "RefSnapGrid"))
				s_MovementSettingBuffer = *g_RenderWindowSnapGridDistance;
			else if (!_stricmp(SettingName, "RefRotationSpeed"))
				s_MovementSettingBuffer = *g_RenderWindowRefRotationSpeed;
			else if (!_stricmp(SettingName, "RefSnapAngle"))
				s_MovementSettingBuffer = *g_RenderWindowSnapAngle;
			else if (!_stricmp(SettingName, "CamRotationSpeed"))
				s_MovementSettingBuffer = *g_RenderWindowCameraRotationSpeed;
			else if (!_stricmp(SettingName, "CamZoomSpeed"))
				s_MovementSettingBuffer = *g_RenderWindowCameraZoomSpeed;
			else if (!_stricmp(SettingName, "CamPanSpeed"))
				s_MovementSettingBuffer = *g_RenderWindowCameraPanSpeed;
			else
				s_MovementSettingBuffer = 0.0;
		}
	}

	#define _hhName		TESRenderControlAltSnapGrid
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00425A3E);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("RefSnapGrid");
		__asm	popad
		__asm
		{
			fild	s_MovementSettingBuffer
			fstp	dword ptr [esp + 0x20]

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltRefMovementSpeedA
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00425741);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("RefMovementSpeed");
		__asm	popad
		__asm
		{
			fmul	s_MovementSettingBuffer
			lea		ecx, [esp + 0x28]

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltRefMovementSpeedB
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042BE85);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("RefMovementSpeed");
		__asm	popad
		__asm
		{
			lea		ecx, s_MovementSettingBuffer

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltRefMovementSpeedC
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042D0B2);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("RefMovementSpeed");
		__asm	popad
		__asm
		{
			lea		ecx, s_MovementSettingBuffer

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltRefRotationSpeed
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00425DC5);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("RefRotationSpeed");
		__asm	popad
		__asm
		{
			fmul	s_MovementSettingBuffer
			fstp	dword ptr [esp + 0x14]

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltRefSnapAngle
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00425DCD);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("RefSnapAngle");
		__asm	popad
		__asm
		{
			fild	s_MovementSettingBuffer

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltCamRotationSpeed
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042CCB0);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("CamRotationSpeed");
		__asm	popad
		__asm
		{
			lea		ecx, s_MovementSettingBuffer

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltCamZoomSpeedA
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042CCE5);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("CamZoomSpeed");
		__asm	popad
		__asm
		{
			lea		ecx, s_MovementSettingBuffer

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltCamZoomSpeedB
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042CDB4);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("CamZoomSpeed");
		__asm	popad
		__asm
		{
			lea		ecx, s_MovementSettingBuffer

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltCamPanSpeedA
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042CD2B);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("CamPanSpeed");
		__asm	popad
		__asm
		{
			lea		ecx, s_MovementSettingBuffer

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlAltCamPanSpeedB
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042CD76);
		__asm	pushad
		InitializeCurrentRenderWindowMovementSetting("CamPanSpeed");
		__asm	popad
		__asm
		{
			lea		ecx, s_MovementSettingBuffer

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESRenderControlRedrawGrid
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0042EF88);

		SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);
		SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);

		SetActiveWindow(*g_HWND_CSParent);
		SetActiveWindow(*g_HWND_RenderWindow);
		__asm
		{
			mov		eax, 1
			jmp		[_hhGetVar(Retn)]
		}
	}
}