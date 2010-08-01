#include "MiscHooks.h"
#include "SEHooks.h"
#include "Common/CLIWrapper.h"
#include "commctrl.h"
#include "Exports.h"
#include "ExtenderInternals.h"
#include "Common/HandshakeStructs.h"
#include "shlobj.h"

static WNDPROC						g_FindTextOrgWindowProc = NULL;
extern FARPROC						g_WindowHandleCallAddr;

FormData*							UIL_FormData = new FormData();
UseListCellItemData*				UIL_CellData = new UseListCellItemData();

static const char*					g_BSAReturnPath = NULL;
static bool							g_DataHandlerPopulateModListInit_PerformCleanup = false;
static bool							g_DataHandlerPopulateModList_ProcessESPs = true;
static bool							g_DataHandlerPopulateModList_DefaultWorkspace = true;
static bool							g_DataHandlerPopulateModList_QuitReturn = true;
static char*						g_CustomWorkspacePath = new char[MAX_PATH];
static const char*					g_DefaultWorkspacePath = "Data\\";

static const char*					g_DefaultWaterTextureStr = "Water\\dungeonwater01.dds";

void __stdcall DoT()		
{
	MessageBox(NULL, "Hooked", NULL, 1);
}

void _declspec(naked) T(void)
{
	static const UInt32 kT = 0x0047BCCF, kC = 0x004038F0;
	__asm
	{
	//	call	DoT
		jmp		[kT]
	}
}



bool PatchMiscHooks()
{
	DoNop(&kCheckLoadedPluginTypePatch);
	DoNop(&kCheckIsActivePluginAnESMPatch);
	DoNop(&kDoValidateBitArrayPatch[0]);
	DoNop(&kDoValidateBitArrayPatch[1]);
	DoNop(&kResponseEditorMicPatch);

	SafeWrite8(kDataDialogPluginDescriptionPatchAddr, 0xEB);
	SafeWrite8(kDataDialogPluginAuthorPatchAddr, 0xEB);

	PLACE_HOOK(SavePluginCommonDialog);

	WriteRelJump(kConstructObjectUnknownRecordPatchJmpAddr, kConstructObjectUnknownRecordPatchDestAddr);
	SafeWrite8(kDataHandlerPostErrorPatchAddr, 0xEB);

	PLACE_HOOK(ExitCS);	
	
	SafeWrite8(kEditorWarningPatchAddr, 0xEB);

	PLACE_HOOK(FindTextInit);
	PLACE_HOOK(UseInfoListInit);
	PLACE_HOOK(CSInit);
	
	COMMON_DIALOG_CANCEL_PATCH(Model)
	COMMON_DIALOG_CANCEL_PATCH(Animation)
	COMMON_DIALOG_CANCEL_PATCH(Sound)
	COMMON_DIALOG_CANCEL_PATCH(Texture)
	COMMON_DIALOG_CANCEL_PATCH(SPT)

	COMMON_DIALOG_BSA_PATCH(Model);
	COMMON_DIALOG_BSA_PATCH(Animation);
	COMMON_DIALOG_BSA_PATCH(Texture);
	COMMON_DIALOG_BSA_PATCH(SPT);

	COMMON_DIALOG_POST_PATCH(Model);
	COMMON_DIALOG_POST_PATCH(Animation);
	COMMON_DIALOG_POST_PATCH(Texture);
	COMMON_DIALOG_POST_PATCH(SPT);

	DoNop(&kMissingTextureWarningPatch);
	DoNop(&kTopicResultScriptResetPatch);

	PLACE_HOOK(NPCFaceGen);
//	SafeWrite8(kSavePluginWarnMissingMastersPatchAddr, 0);		// TODO: Check ** issues with the render window
	
//	PLACE_HOOK(DataDlgCallPopulateModList);
//	PLACE_HOOK(DataHandlerPopulateModListFindLoop);
//	PLACE_HOOK(DataHandlerPopulateModListInitCleanup);
//	PLACE_HOOK(DataHandlerPopulateModListQuickExit);
//	PLACE_HOOK(TESCtorSkipModListPopulation);
	sprintf_s(g_CustomWorkspacePath, MAX_PATH, "Data");

	SafeWrite32(kDefaultWaterTextureFixPatchAddr, (UInt32)g_DefaultWaterTextureStr);

	WriteRelJump(0x0047BCBC, (UInt32)T);
	if (CreateDirectory(std::string(g_AppPath + "Data\\Backup").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		_D_PRINT("Couldn't create the Backup folder in Data directory");
	}
	return true;
}


SHORT __stdcall IsControlKeyDown(void)
{
	return GetAsyncKeyState(VK_CONTROL);
}

void _declspec(naked) SavePluginCommonDialogHook(void)
{
	_asm
	{
		pushad
		call	IsControlKeyDown
		test	eax, eax
		jnz		ESM

		popad
		jmp		[kSavePluginCommonDialogESPRetnAddr]
	ESM:
		popad
		jmp		[kSavePluginCommonDialogESMRetnAddr]
	}
}

void DoNop(const NopData* Data)
{
	for (int Offset = 0; Offset < Data->Size; Offset++) {
		SafeWrite8(Data->Address + Offset, 0x90);
	}	
}

void _declspec(naked) ExitCSHook(void)
{
	__asm
	{
		push    ebx
		call    DoExitCS
	}
}

void __stdcall DoExitCS(HWND MainWindow)
{
	WritePositionToINI(MainWindow, NULL);
	WritePositionToINI(*g_HWND_CellView, "Cell View");
	WritePositionToINI(*g_HWND_ObjectWindow, "Object Window");
	WritePositionToINI(*g_HWND_RenderWindow, "Render Window");;
	ExitProcess(0);
}


LRESULT CALLBACK FindTextDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
	switch (uMsg)
	{
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case LVN_ITEMACTIVATE:				// ID = 1018
			NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;
			ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 0, g_Buffer, sizeof(g_Buffer));
			std::string EditorID, FormTypeStr(g_Buffer);

			ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 1, g_Buffer, sizeof(g_Buffer));
			EditorID = g_Buffer;
			UInt32 PadStart = EditorID.find("'") + 1, PadEnd  = EditorID.find("'", PadStart + 1);
			if (PadStart != std::string::npos && PadEnd != std::string::npos) {
				EditorID = EditorID.substr(PadStart, PadEnd - PadStart);
				LoadFormIntoView(EditorID.c_str(), FormTypeStr.c_str());
			}
			break;
		}
		break;
	case WM_DESTROY: 
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_FindTextOrgWindowProc);
		break; 
	}
 
	return CallWindowProc(g_FindTextOrgWindowProc, hWnd, uMsg, wParam, lParam); 
} 

void __stdcall DoFindTextInitHook(HWND FindTextDialog)
{
	g_FindTextOrgWindowProc = (WNDPROC)SetWindowLong(FindTextDialog, GWL_WNDPROC, (LONG)FindTextDlgSubClassProc);
}

void __declspec(naked) FindTextInitHook(void)
{
	__asm
	{
		call	CreateDialogParamAddress
		call	[g_WindowHandleCallAddr]			// CreateDialogParamA

		pushad
		push	eax	
		call	DoFindTextInitHook
		popad

		jmp		[kFindTextInitRetnAddr]
	}
}

void RemoteLoadRef(const char* EditorID)
{
	TESObjectREFR* Reference = CS_CAST(GetFormByID(EditorID), TESForm, TESObjectREFR);
	TESChildCell* Cell = (TESChildCell*)thisVirtualCall(kTESObjectREFR_VTBL, 0x1A0, Reference);
	thisCall(kTESChildCell_LoadCellFnAddr, Cell, Cell, Reference);
}

void __declspec(naked) UseInfoListInitHook(void)
{
	__asm
	{
		call	CLIWrapper::UIL_OpenUseInfoBox
		jmp		[kUseInfoListInitRetnAddr]
	}
}

void __stdcall DoCSInitHook()
{
	static bool DoOnce = false;
	if (DoOnce)		return;

								// perform deferred patching
								// do NOT do anything that would pump messages to the main WndProc
	PatchUseInfoListMenu();
	DoOnce = true;
}


void __declspec(naked) CSInitHook(void)
{
	__asm
	{
		call	[kCSInitCallAddr]
		call	DoCSInitHook
		jmp		[kCSInitRetnAddr]
	}
}

void PatchUseInfoListMenu()
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent),
		  GameplayMenu = GetSubMenu(MainMenu, 5),
		  ViewMenu = GetSubMenu(MainMenu, 2);

	MENUITEMINFO ItemDataUseInfo, ItemDataRenderWindow;
	ItemDataUseInfo.cbSize = sizeof(MENUITEMINFO);
	ItemDataUseInfo.fMask = MIIM_STRING;
	ItemDataUseInfo.dwTypeData = "Use Info Listings";
	ItemDataUseInfo.cch = 15;
	SetMenuItemInfo(GameplayMenu, 245, FALSE, &ItemDataUseInfo);

	ItemDataRenderWindow.cbSize = sizeof(MENUITEMINFO);		// the tool coder seems to have mixed up the menucode for the button
	ItemDataRenderWindow.fMask = MIIM_ID|MIIM_STATE;		// as the code to handle hiding/showing is present in the wndproc
	ItemDataRenderWindow.wID = 40423;						// we simply change the code to the one that's expected by the proc
	ItemDataRenderWindow.fState = MFS_CHECKED;
	SetMenuItemInfo(ViewMenu, 40198, FALSE, &ItemDataRenderWindow);	
}

COMMON_DIALOG_CANCEL_HOOK(Model)
COMMON_DIALOG_CANCEL_HOOK(Animation)
COMMON_DIALOG_CANCEL_HOOK(Sound)
COMMON_DIALOG_CANCEL_HOOK(Texture)
COMMON_DIALOG_CANCEL_HOOK(SPT)

void __stdcall ShowBSAViewer(UInt32 Filter)
{
	switch (Filter)
	{
	case 0:
		g_BSAReturnPath = CLIWrapper::BSAV_InitializeViewer(g_AppPath.c_str(), "nif");
		break;
	case 1:
		g_BSAReturnPath = CLIWrapper::BSAV_InitializeViewer(g_AppPath.c_str(), "kf");
		break;
	case 3:
		g_BSAReturnPath = CLIWrapper::BSAV_InitializeViewer(g_AppPath.c_str(), "dds");
		break;
	case 4:
		g_BSAReturnPath = CLIWrapper::BSAV_InitializeViewer(g_AppPath.c_str(), "spt");
		break;
	}
}

COMMON_DIALOG_BSA_HOOK(Model)
COMMON_DIALOG_BSA_HOOK(Animation)
COMMON_DIALOG_BSA_HOOK(Texture)
COMMON_DIALOG_BSA_HOOK(SPT)

void __declspec(naked) ModelPostCommonDialogHook(void)     
{
    __asm
    {
		cmp		[g_BSAReturnPath], 0
		jnz		BSA

		lea		eax, [esp + 0x14]
        jmp     [kModelPostCommonDialogRetnAddr]
	BSA:
		mov		eax, g_BSAReturnPath
        jmp     [kModelPostCommonDialogRetnAddr]
    }
}
void __declspec(naked) AnimationPostCommonDialogHook(void)     
{
    __asm
    {
		cmp		[g_BSAReturnPath], 0
		jnz		BSA

		lea		edx, [ebp]
        jmp		POST
	BSA:
		mov		edx, g_BSAReturnPath
	POST:
		push	edx
		lea		ecx, [esp + 0x24]
        jmp     [kAnimationPostCommonDialogRetnAddr]
    }
}
void __declspec(naked) TexturePostCommonDialogHook(void)     
{
    __asm
    {
		cmp		[g_BSAReturnPath], 0
		jnz		BSA

		lea		eax, [ebp]
        jmp		POST
	BSA:
		mov		edx, g_BSAReturnPath
	POST:
		push	eax
		lea		ecx, [ebp - 0x14]
        jmp     [kTexturePostCommonDialogRetnAddr]
    }
}
void __declspec(naked) SPTPostCommonDialogHook(void)     
{
    __asm
    {
		cmp		[g_BSAReturnPath], 0
		jnz		BSA

		lea		ecx, [esp + 0x14]
        jmp     [kSPTPostCommonDialogRetnAddr]
	BSA:
		mov		ecx, g_BSAReturnPath
        jmp     [kSPTPostCommonDialogRetnAddr]
    }
}

void __stdcall DoNPCFaceGenHook(HWND Dialog)
{
	SendMessageA(Dialog, WM_COMMAND, 1014, 0);
}

void __declspec(naked) NPCFaceGenHook(void)     
{
    __asm
    {
		call	[kNPCFaceGenCallAddr]
		push	esi
		call	DoNPCFaceGenHook
		jmp		[kNPCFaceGenRetnAddr]
	}
}

void __declspec(naked) DataHandlerPopulateModListInitCleanupHook(void)
{
	__asm
	{
		test	eax, eax
		jz		FAIL
		
		mov		al, [g_DataHandlerPopulateModListInit_PerformCleanup]		// our test
		test	al, al
		jz		FAIL

		jmp		[kDataHandlerPopulateModListInitCleanupPassRetnAddr]		// perform cleanup
	FAIL:
		jmp		[kDataHandlerPopulateModListInitCleanupFailRetnAddr]		// don't cleanup
	}
}

void __declspec(naked) DataHandlerPopulateModListFindLoopHook(void)
{
	__asm
	{
		jb		PASS
		jmp		[kDataHandlerPopulateModListFindLoopFailRetnAddr]
	PASS:
		mov		al, [g_DataHandlerPopulateModList_ProcessESPs]
		test	al, al
		jz		SKIP

		jmp		[kDataHandlerPopulateModListFindLoopPassRetnAddr]
	SKIP:
		jmp		[kDataHandlerPopulateModListFindLoopFailRetnAddr]
	}
}

void __stdcall DoDataDlgCallPopulateModListHook(INISetting* Path)
{
	if (IsControlKeyDown()) {
		BROWSEINFO WorkspaceInfo;
		WorkspaceInfo.hwndOwner = *g_HWND_CSParent;
		WorkspaceInfo.iImage = NULL;
		WorkspaceInfo.pszDisplayName = g_CustomWorkspacePath;
		WorkspaceInfo.lpszTitle = "Select a folder to use as this session's workspace";
		WorkspaceInfo.ulFlags = BIF_NEWDIALOGSTYLE|BIF_RETURNONLYFSDIRS;
		WorkspaceInfo.pidlRoot = NULL;
		WorkspaceInfo.lpfn = NULL;
		WorkspaceInfo.lParam = NULL;

		PIDLIST_ABSOLUTE ReturnPath = SHBrowseForFolder(&WorkspaceInfo);
		if (ReturnPath) {
			if (!SHGetPathFromIDList(ReturnPath, g_CustomWorkspacePath)) {
				_D_PRINT("Had trouble extracting workspace path!");
			}
		}
		else
			sprintf_s(g_CustomWorkspacePath, MAX_PATH, "Data");	
	}
	sprintf_s(g_CustomWorkspacePath, MAX_PATH, "%s\\", g_CustomWorkspacePath);
	g_INI_LocalMasterPath->Data = g_CustomWorkspacePath;

	if (!_stricmp(g_CustomWorkspacePath, (std::string(g_AppPath + "Data\\")).c_str()) ||
		!_stricmp(g_CustomWorkspacePath, g_DefaultWorkspacePath))
	{
			g_DataHandlerPopulateModList_DefaultWorkspace = true;
	}
	else	g_DataHandlerPopulateModList_DefaultWorkspace = false;
}

void __stdcall CleanUpModList(void)
{
//	FormHeap_Free
}

void __declspec(naked) DataDlgCallPopulateModListHook(void)
{
	__asm
	{
		pushad
		push	eax
		call	DoDataDlgCallPopulateModListHook
		popad
		
		mov		al, g_DataHandlerPopulateModList_DefaultWorkspace
		test	al, al
		jnz		DEFAULT
		
		mov		[g_DataHandlerPopulateModList_ProcessESPs], 0				// custom workspace
		mov		[g_DataHandlerPopulateModListInit_PerformCleanup], 1
		push	g_DefaultWorkspacePath
		call	[kDataDlgCallPopulateModListCallAddr]						// parse masters in the default workspace before proceeding to the custom

	//	push	g_CustomWorkspacePath
	//	mov		ecx, [g_dataHandler]
	//	mov		[g_DataHandlerPopulateModList_ProcessESPs], 1
	//	mov		[g_DataHandlerPopulateModListInit_PerformCleanup], 1
	//	call	[kDataDlgCallPopulateModListCallAddr]

		jmp		EXIT
	DEFAULT:																// workspace is set to default
		push	g_DefaultWorkspacePath
		call	[kDataDlgCallPopulateModListCallAddr]
	EXIT:																	// reset flags
		mov		[g_DataHandlerPopulateModList_ProcessESPs], 1
		mov		[g_DataHandlerPopulateModListInit_PerformCleanup], 0
		jmp		[kDataDlgCallPopulateModListRetnAddr]
	}
}

void __declspec(naked) DataHandlerPopulateModListQuickExitHook(void)
{
	__asm
	{
		mov		al, [g_DataHandlerPopulateModList_QuitReturn]
		test	al, al
		jz		FAIL

	//	mov		large fs:0, eax
		jmp		[kDataHandlerPopulateModListQuickExitPassRetnAddr]		// continue
	FAIL:
		jmp		[kDataHandlerPopulateModListQuickExitFailRetnAddr]		// exit
	}
}

void __declspec(naked) TESCtorSkipModListPopulationHook(void)
{
	__asm
	{
		mov		[g_DataHandlerPopulateModList_QuitReturn], 1
		call	[kTESCtorSkipModListPopulationCallAddr]
		mov		[g_DataHandlerPopulateModList_QuitReturn], 0
		call	DoT
		jmp		[kTESCtorSkipModListPopulationRetnAddr]
	}
}