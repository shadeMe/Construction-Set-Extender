#include "Renderer.h"
#include "..\RenderSelectionGroupManager.h"
#include "..\RenderTimeManager.h"
#include "..\RenderWindowTextPainter.h"
#include "..\CSDialogs.h"

namespace Hooks
{
	#define PI					3.151592653589793

	TESForm*					g_TESObjectREFRUpdate3DBuffer = NULL;

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
	_DefineHookHdlrWithBuffer(TESObjectREFRUpdate3D, 0x00549AC5)(5, 0x56, 0x8B, 0x74, 0x24, 0x34), 5);
	_DefineHookHdlr(ForceShowTESObjectREFRDialog, 0x00429EE3);

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

		BSExtraData* xData = (BSExtraData*)thisCall(kBaseExtraList_GetExtraDataByType, &Object->baseExtraList, kExtraData_EnableStateParent);
		if (xData)
		{
			ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
			if ((xParent->parent->flags & kTESObjectREFRSpecialFlags_Children3DInvisible))
				ToggleFlag(&Node->m_flags, NiNode::kFlag_AppCulled, true);
		}

		if ((Object->flags & kTESObjectREFRSpecialFlags_3DInvisible))
			ToggleFlag(&Node->m_flags, NiNode::kFlag_AppCulled, true);
	}

	#define _hhName		TESObjectREFRGet3DData
	_hhBegin()
	{
		__asm
		{
			push	esi
			push	ecx		// store
			push	0x56
			add		ecx, 0x4C
			xor		esi, esi
			call	[kBaseExtraList_GetExtraDataByType]
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
		g_RenderTimeManager.Update();
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
			if ((*g_TESRenderSelectionPrimary)->SelectionCount > 1)
			{
				PrintToBuffer("%d Objects Selected\nPosition Vector Sum: %.04f, %.04f, %.04f",
							(*g_TESRenderSelectionPrimary)->SelectionCount,
							(*g_TESRenderSelectionPrimary)->x,
							(*g_TESRenderSelectionPrimary)->y,
							(*g_TESRenderSelectionPrimary)->z);
				RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_1, g_TextBuffer, 0);
			}
			else if ((*g_TESRenderSelectionPrimary)->SelectionCount)
			{
				TESObjectREFR* Selection = CS_CAST((*g_TESRenderSelectionPrimary)->RenderSelection->Data, TESForm, TESObjectREFR);
				char Buffer[0x50] = {0};
				sprintf_s(Buffer, 0x50, "");

				BSExtraData* xData = (BSExtraData*)thisCall(kBaseExtraList_GetExtraDataByType, &Selection->baseExtraList, kExtraData_EnableStateParent);
				if (xData)
				{
					ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
					sprintf_s(Buffer, 0x50, "Parent: %s [%08X]  Opposite State: %d",
																	((xParent->parent->editorData.editorID.m_dataLen)?(xParent->parent->editorData.editorID.m_data):("")),
																	xParent->parent->refID, (UInt8)xParent->oppositeState);
				}
				PrintToBuffer("%s (%08X) BASE[%s (%08X)]\nP[%.04f, %.04f, %.04f]\nR[%.04f, %.04f, %.04f]\nS[%.04f]\nFlags: %s %s %s %s %s %s\n%s",
								((Selection->editorData.editorID.m_dataLen)?(Selection->editorData.editorID.m_data):("")), Selection->refID,
								((Selection->baseForm->editorData.editorID.m_dataLen)?(Selection->baseForm->editorData.editorID.m_data):("")), Selection->baseForm->refID,
								Selection->posX, Selection->posY, Selection->posZ, 
								Selection->rotX * 180 / PI, 
								Selection->rotY * 180 / PI, 
								Selection->rotZ * 180 / PI, 
								Selection->scale,
								((Selection->flags & ::TESForm::kFormFlags_Essential)?("P"):("-")),
								((Selection->flags & ::TESForm::kFormFlags_InitiallyDisabled)?("D"):("-")),
								((Selection->flags & ::TESForm::kFormFlags_VisibleWhenDistant)?("V"):("-")),
								((Selection->flags & kTESObjectREFRSpecialFlags_3DInvisible)?("I"):("-")),
								((Selection->flags & kTESObjectREFRSpecialFlags_Children3DInvisible)?("CI"):("-")),
								((Selection->flags & kTESObjectREFRSpecialFlags_Frozen)?("F"):("-")),
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
			mov		eax, [g_Flag_RenderWindowUpdateViewPort]
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

		::TESObjectCELL* CurrentCell = (*g_TES)->currentInteriorCell;
		if (CurrentCell == NULL)
			CurrentCell = (*g_TES)->currentExteriorCell;

		if (CurrentCell)
		{
			TESRenderSelection* Selection = g_RenderSelectionGroupManager.GetRefSelectionGroup(Ref, CurrentCell);
			if (Selection)
			{
				for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->RenderSelection; Itr && Itr->Data; Itr = Itr->Next)
					thisCall(kTESRenderSelection_AddFormToSelection, *g_TESRenderSelectionPrimary, Itr->Data, 1);

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
		__asm
		{
			call	[kTESRenderSelection_ClearSelection]
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
		std::vector<::TESForm*> FrozenRefs;
		for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->RenderSelection; Itr && Itr->Data; Itr = Itr->Next)
		{
			if ((Itr->Data->flags & kTESObjectREFRSpecialFlags_Frozen))
				FrozenRefs.push_back(Itr->Data);
		}

		for (std::vector<::TESForm*>::const_iterator Itr = FrozenRefs.begin(); Itr != FrozenRefs.end(); Itr++)
			thisCall(kTESRenderSelection_RemoveFormFromSelection, *g_TESRenderSelectionPrimary, *Itr, 1);
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
}
