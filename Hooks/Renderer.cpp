#include "Renderer.h"
#include "..\RenderSelectionGroupManager.h"
#include "..\RenderTimeManager.h"
#include "..\RenderWindowTextPainter.h"
#include "..\CSDialogs.h"

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

void PatchRendererHooks(void)
{
	_MemoryHandler(DoorMarkerProperties).WriteJump();
	_MemoryHandler(TESObjectREFRGet3DData).WriteJump();
	_MemoryHandler(NiWindowRender).WriteJump();
	_MemoryHandler(NiDX9RendererRecreate).WriteJump();
	_MemoryHandler(RenderWindowStats).WriteJump();
	_MemoryHandler(UpdateViewport).WriteJump();
	_MemoryHandler(RenderWindowSelection).WriteJump();
	_MemoryHandler(TESRenderControlPerformMove).WriteJump();
	_MemoryHandler(TESRenderControlPerformRotate).WriteJump();
	_MemoryHandler(TESRenderControlPerformScale).WriteJump();
	_MemoryHandler(TESRenderControlPerformFall).WriteJump();
	_MemoryHandler(TESObjectREFRSetupDialog).WriteJump();
	_MemoryHandler(TESObjectREFRCleanDialog).WriteJump();
}

_BeginHookHdlrFn(DoorMarkerProperties)
{
	_DeclareHookHdlrFnVariable(DoorMarkerProperties, Properties, 0x00429EAB);
	_DeclareHookHdlrFnVariable(DoorMarkerProperties, Teleport, 0x00429EE8);
	__asm
	{
		mov		eax, [esi + 0x8]
		shr		eax, 0x0E
		test	al, 1
		jnz		DOORMARKER

		jmp		[_HookHdlrFnVariable(DoorMarkerProperties, Properties)]
	TELEPORT:
		popad
		jmp		[_HookHdlrFnVariable(DoorMarkerProperties, Teleport)]
	DOORMARKER:
		pushad
		call	IsControlKeyDown
		test	eax, eax
		jz		TELEPORT
		popad

		jmp		[_HookHdlrFnVariable(DoorMarkerProperties, Properties)]
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

_BeginHookHdlrFn(TESObjectREFRGet3DData)
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

_BeginHookHdlrFn(NiWindowRender)
{
	_DeclareHookHdlrFnVariable(NiWindowRender, Call, 0x0076A3B0);
	_DeclareHookHdlrFnVariable(NiWindowRender, Retn, 0x00406447);
	__asm
	{
		call	[_HookHdlrFnVariable(NiWindowRender, Call)]

		pushad
		call	NiWindowRenderDrawHook
		popad

		jmp		[_HookHdlrFnVariable(NiWindowRender, Retn)]
	}
}

void __stdcall DoNiDX9RendererRecreateHook(void)
{
	RENDERTEXT->Recreate();
}

_BeginHookHdlrFn(NiDX9RendererRecreate)
{
	_DeclareHookHdlrFnVariable(NiDX9RendererRecreate, Retn, 0x006D7266);
	__asm
	{
		pushad
		call	DoNiDX9RendererRecreateHook
		popad

		sub     esp, 0x10
		push    ebx
		push    ebp
		push    esi

		jmp		[_HookHdlrFnVariable(NiDX9RendererRecreate, Retn)]
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
			RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_1, g_Buffer, 0);
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
							Selection->posX, Selection->posY, Selection->posZ, Selection->rotX, Selection->rotY, Selection->rotZ, Selection->scale,
							((Selection->flags & TESForm::kFormFlags_Essential)?("P"):("-")),
							((Selection->flags & TESForm::kFormFlags_InitiallyDisabled)?("D"):("-")),
							((Selection->flags & TESForm::kFormFlags_VisibleWhenDistant)?("V"):("-")),
							((Selection->flags & kTESObjectREFRSpecialFlags_3DInvisible)?("I"):("-")),
							((Selection->flags & kTESObjectREFRSpecialFlags_Children3DInvisible)?("CI"):("-")),
							((Selection->flags & kTESObjectREFRSpecialFlags_Frozen)?("F"):("-")),
							Buffer);

			RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_1, g_Buffer, 0);
		}
		else
			RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_1, NULL, 0);
	}
	else
		RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_1, NULL, 0);
}

_BeginHookHdlrFn(RenderWindowStats)
{
	_DeclareHookHdlrFnVariable(RenderWindowStats, Call, 0x006F25E0);
	_DeclareHookHdlrFnVariable(RenderWindowStats, Retn, 0x0042D3F9);
	__asm
	{
		call	[_HookHdlrFnVariable(RenderWindowStats, Call)]

		pushad
		call	DoRenderWindowStatsHook
		popad

		jmp		[_HookHdlrFnVariable(RenderWindowStats, Retn)]
	}
}

bool __stdcall DoUpdateViewportHook(void)
{
	if (RENDERTEXT->GetRenderChannelQueueSize(RenderWindowTextPainter::kRenderChannel_2) || g_INIManager->GetINIInt("UpdateViewPortAsync"))
		return true;
	else
		return false;
}

_BeginHookHdlrFn(UpdateViewport)
{
	_DeclareHookHdlrFnVariable(UpdateViewport, Retn, 0x0042EF86);
	_DeclareHookHdlrFnVariable(UpdateViewport, Jump, 0x0042CE7D);
	__asm
	{
		mov		eax, [g_Flag_RenderWindowUpdateViewPort]
		mov		eax, [eax]
		cmp		al, 0
		jz		DONTUPDATE

		jmp		[_HookHdlrFnVariable(UpdateViewport, Jump)]
	DONTUPDATE:
		pushad
		xor		eax, eax
		call	DoUpdateViewportHook
		test	al, al
		jz		EXIT

		popad
		jmp		[_HookHdlrFnVariable(UpdateViewport, Jump)]
	EXIT:
		popad
		jmp		[_HookHdlrFnVariable(UpdateViewport, Retn)]
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
			for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->RenderSelection; Itr && Itr->Data; Itr = Itr->Next)
				thisCall(kTESRenderSelection_AddFormToSelection, *g_TESRenderSelectionPrimary, Itr->Data, 1);

			RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_2, "Selected object selection group", 3);
			Result = true;
		}
	}

	return Result;
}

_BeginHookHdlrFn(RenderWindowSelection)
{
	_DeclareHookHdlrFnVariable(RenderWindowSelection, Retn, 0x0042AE76);
	_DeclareHookHdlrFnVariable(RenderWindowSelection, Jump, 0x0042AE84);
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

		jmp		[_HookHdlrFnVariable(RenderWindowSelection, Retn)]
	GROUPED:
		popad
		jmp		[_HookHdlrFnVariable(RenderWindowSelection, Jump)]
	}
}

void __stdcall TESRenderControlProcessFrozenRefs(void)
{
	std::vector<TESForm*> FrozenRefs;
	for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->RenderSelection; Itr && Itr->Data; Itr = Itr->Next)
	{
		if ((Itr->Data->flags & kTESObjectREFRSpecialFlags_Frozen))
			FrozenRefs.push_back(Itr->Data);
	}

	for (std::vector<TESForm*>::const_iterator Itr = FrozenRefs.begin(); Itr != FrozenRefs.end(); Itr++)
		thisCall(kTESRenderSelection_RemoveFormFromSelection, *g_TESRenderSelectionPrimary, *Itr, 1);
}

_BeginHookHdlrFn(TESRenderControlPerformMove)
{
	_DeclareHookHdlrFnVariable(TESRenderControlPerformMove, Retn, 0x00425676);
	__asm
	{
		sub		esp, 0x114
		pushad
		call	TESRenderControlProcessFrozenRefs
		popad

		jmp		[_HookHdlrFnVariable(TESRenderControlPerformMove, Retn)]
	}
}

_BeginHookHdlrFn(TESRenderControlPerformRotate)
{
	_DeclareHookHdlrFnVariable(TESRenderControlPerformRotate, Retn, 0x00425D74);
	__asm
	{
		sub		esp, 0xC0
		pushad
		call	TESRenderControlProcessFrozenRefs
		popad

		jmp		[_HookHdlrFnVariable(TESRenderControlPerformRotate, Retn)]
	}
}

_BeginHookHdlrFn(TESRenderControlPerformScale)
{
	_DeclareHookHdlrFnVariable(TESRenderControlPerformScale, Retn, 0x00424659);
	__asm
	{
		sub		esp, 0x40
		mov		ecx, [g_TESRenderSelectionPrimary]
		mov		ecx, [ecx]

		pushad
		call	TESRenderControlProcessFrozenRefs
		popad

		jmp		[_HookHdlrFnVariable(TESRenderControlPerformScale, Retn)]
	}
}

_BeginHookHdlrFn(TESRenderControlPerformFall)
{
	_DeclareHookHdlrFnVariable(TESRenderControlPerformFall, Retn, 0x0042886F);
	_DeclareHookHdlrFnVariable(TESRenderControlPerformFall, Call, 0x00512990);
	__asm
	{
		pushad
		call	TESRenderControlProcessFrozenRefs
		popad

		call	[_HookHdlrFnVariable(TESRenderControlPerformFall, Call)]
		jmp		[_HookHdlrFnVariable(TESRenderControlPerformFall, Retn)]
	}
}

void __stdcall DoTESObjectREFREditDialogHook(NiNode* Node, bool State)
{
	ToggleFlag(&Node->m_flags, kNiNodeSpecialFlags_DontUncull, State);
}

_BeginHookHdlrFn(TESObjectREFRSetupDialog)
{
	_DeclareHookHdlrFnVariable(TESObjectREFRSetupDialog, Retn, 0x00549A05);
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

		jmp		[_HookHdlrFnVariable(TESObjectREFRSetupDialog, Retn)]
	}
}

_BeginHookHdlrFn(TESObjectREFRCleanDialog)
{
	_DeclareHookHdlrFnVariable(TESObjectREFRCleanDialog, Retn, 0x00549B57);
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

		jmp		[_HookHdlrFnVariable(TESObjectREFRCleanDialog, Retn)]
	}
}