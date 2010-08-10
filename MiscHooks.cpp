#include "MiscHooks.h"
#include "SEHooks.h"
#include "Common/CLIWrapper.h"
#include "Exports.h"
#include "ExtenderInternals.h"
#include "Common/HandshakeStructs.h"
#include "WindowManager.h"

extern FARPROC						g_WindowHandleCallAddr;

FormData*							UIL_FormData = new FormData();
UseListCellItemData*				UIL_CellData = new UseListCellItemData();

static const char*					g_AssetSelectorReturnPath = NULL;
static bool							g_DataHandlerPopulateModListInit_PerformCleanup = false;
static bool							g_DataHandlerPopulateModList_ProcessESPs = true;
static bool							g_DataHandlerPopulateModList_DefaultWorkspace = true;
static bool							g_DataHandlerPopulateModList_QuitReturn = true;
static char*						g_CustomWorkspacePath = new char[MAX_PATH];
static const char*					g_DefaultWorkspacePath = "Data\\";

static const char*					g_DefaultWaterTextureStr = "Water\\dungeonwater01.dds";

static bool							g_QuickLoadToggle = false;
static HFONT						g_CSDefaultFont = NULL;

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

	COMMON_DIALOG_SELECTOR_PATCH(Model);
	COMMON_DIALOG_SELECTOR_PATCH(Animation);
	COMMON_DIALOG_SELECTOR_PATCH(Sound);
	COMMON_DIALOG_SELECTOR_PATCH(Texture);
	COMMON_DIALOG_SELECTOR_PATCH(SPT);

	COMMON_DIALOG_POST_PATCH(Model);
	COMMON_DIALOG_POST_PATCH(Animation);
	COMMON_DIALOG_POST_PATCH(Sound);
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
	PLACE_HOOK(QuickLoadPluginLoadHandlerPrologue);
	PLACE_HOOK(QuickLoadPluginLoadHandler);
	PLACE_HOOK(QuickLoadPluginSaveHandler);
	PLACE_HOOK(DataDlgInit);

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


void __stdcall DoExitCS(HWND MainWindow)
{
	WritePositionToINI(MainWindow, NULL);
	WritePositionToINI(*g_HWND_CellView, "Cell View");
	WritePositionToINI(*g_HWND_ObjectWindow, "Object Window");
	WritePositionToINI(*g_HWND_RenderWindow, "Render Window");;
	ExitProcess(0);
}

void _declspec(naked) ExitCSHook(void)
{
	__asm
	{
		push    ebx
		call    DoExitCS
	}
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

UInt32 __stdcall InitBSAViewer(UInt32 Filter)
{
	switch (Filter)
	{
	case 0:
		g_AssetSelectorReturnPath = CLIWrapper::BSAV_InitializeViewer(g_AppPath.c_str(), "nif");
		break;
	case 1:
		g_AssetSelectorReturnPath = CLIWrapper::BSAV_InitializeViewer(g_AppPath.c_str(), "kf");
		break;
	case 2:
		g_AssetSelectorReturnPath = CLIWrapper::BSAV_InitializeViewer(g_AppPath.c_str(), "wav");
		break;
	case 3:
		g_AssetSelectorReturnPath = CLIWrapper::BSAV_InitializeViewer(g_AppPath.c_str(), "dds");
		break;
	case 4:
		g_AssetSelectorReturnPath = CLIWrapper::BSAV_InitializeViewer(g_AppPath.c_str(), "spt");
		break;
	}

	if (!g_AssetSelectorReturnPath)
		return 0;
	else
		return e_FetchPath;
}

UInt32 __stdcall InitPathEditor(int ID, HWND Dialog)
{
	GetDlgItemText(Dialog, ID, g_Buffer, sizeof(g_Buffer));
	g_AssetSelectorReturnPath = (const char*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TEXTEDIT), Dialog, (DLGPROC)TextEditDlgProc, (LPARAM)g_Buffer);

	if (!g_AssetSelectorReturnPath)
		return 0;
	else
		return e_FetchPath;
}

UInt32 __stdcall InitAssetSelectorDlg(HWND Dialog)
{
	return DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_ASSETSEL), Dialog, (DLGPROC)AssetSelectorDlgProc);
}

COMMON_DIALOG_SELECTOR_HOOK(Model)
COMMON_DIALOG_SELECTOR_HOOK(Animation)
COMMON_DIALOG_SELECTOR_HOOK(Sound)
COMMON_DIALOG_SELECTOR_HOOK(Texture)
COMMON_DIALOG_SELECTOR_HOOK(SPT)

void __declspec(naked) ModelPostCommonDialogHook(void)     
{
    __asm
    {
		cmp		eax, e_FetchPath	
		jz		SELECT

		lea		eax, [esp + 0x14]
        jmp     [kModelPostCommonDialogRetnAddr]
	SELECT:
		mov		eax, g_AssetSelectorReturnPath
        jmp     [kModelPostCommonDialogRetnAddr]
    }
}
void __declspec(naked) AnimationPostCommonDialogHook(void)     
{
    __asm
    {
		cmp		eax, e_FetchPath
		jz		SELECT

		lea		edx, [ebp]
        jmp		POST
	SELECT:
		mov		edx, g_AssetSelectorReturnPath
	POST:
		push	edx
		lea		ecx, [esp + 0x24]
        jmp     [kAnimationPostCommonDialogRetnAddr]
    }
}
void __declspec(naked) SoundPostCommonDialogHook(void)     
{
    __asm
    {
		cmp		eax, e_FetchPath	
		jz		SELECT

		lea		ecx, [esp + 8]
		push	ecx
        jmp     [kSoundPostCommonDialogRetnAddr]
	SELECT:
		mov		ecx, g_AssetSelectorReturnPath
		push	ecx
        jmp     [kSoundPostCommonDialogRetnAddr]
    }
}
void __declspec(naked) TexturePostCommonDialogHook(void)     
{
    __asm
    {
		cmp		eax, e_FetchPath
		jz		SELECT

		lea		eax, [ebp]
        jmp		POST
	SELECT:
		mov		edx, g_AssetSelectorReturnPath
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
		cmp		eax, e_FetchPath
		jz		SELECT

		lea		ecx, [esp + 0x14]
        jmp     [kSPTPostCommonDialogRetnAddr]
	SELECT:
		mov		ecx, g_AssetSelectorReturnPath
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

void __stdcall DoQuickLoadPluginLoadHandlerPrologueHook(HWND DataDlg)
{
	if (IsDlgButtonChecked(DataDlg, 9900) == BST_CHECKED)
		g_QuickLoadToggle = true;
	else
		g_QuickLoadToggle = false;
}

void __declspec(naked) QuickLoadPluginLoadHandlerPrologueHook(void)
{
	__asm
	{
		pushad
		push	edi
		call	DoQuickLoadPluginLoadHandlerPrologueHook
		popad
		call	[kQuickLoadPluginLoadHandlerPrologueCallAddr]
		jmp		[kQuickLoadPluginLoadHandlerPrologueRetnAddr]
	}
}

bool __stdcall DoQuickLoadPluginLoadHandlerHook(ModEntry::Data* CurrentFile)
{
	return _stricmp(CurrentFile->name, (*g_TESActivePlugin)->name);
}

void __declspec(naked) QuickLoadPluginLoadHandlerHook(void)
{
	__asm
	{
		pushad
		mov		al, g_QuickLoadToggle
		test	al, al
		jz		CONTINUE
		push	edx
		call	DoQuickLoadPluginLoadHandlerHook
		test	eax, eax
		jnz		SKIP
	CONTINUE:
		popad

		push	ecx
		push	edx
		mov		ecx, edi
		call	[kQuickLoadPluginLoadHandlerCallAddr]
		jmp		[kQuickLoadPluginLoadHandlerRetnAddr]
	SKIP:
		popad
		jmp		[kQuickLoadPluginLoadHandlerSkipAddr]
	}
}

bool __stdcall DoQuickLoadPluginSaveHandlerHook()
{
	if (MessageBox(*g_HWND_CSParent, 
					"Are you sure you want to save the quick-loaded active plugin? There will be a loss of data if it contains overridden records", 
					"Save Warning", 
					MB_ICONWARNING|MB_YESNO) == IDYES)
		return false;
	else
		return true;
}

void __declspec(naked) QuickLoadPluginSaveHandlerHook(void)
{
	__asm
	{
		pushad
		mov		al, g_QuickLoadToggle
		test	al, al
		jz		CONTINUE

		call	DoQuickLoadPluginSaveHandlerHook
		test	eax, eax
		jnz		SKIP
	CONTINUE:
		popad
		call	[kQuickLoadPluginSaveHandlerCallAddr]
		jmp		[kQuickLoadPluginSaveHandlerRetnAddr]
	SKIP:
		popad
		jmp		[kQuickLoadPluginSaveHandlerSkipAddr]
	}
}

void __stdcall DoDataDlgInitHook(HWND DataDialog)
{
	// create new controls
	HWND QuickLoadCheckBox = CreateWindowEx(0, 
											"BUTTON", 
											"Quick-Load Plugin", 
											BS_AUTOCHECKBOX|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
											474, 198, 142, 15, 
											DataDialog, 
											(HMENU)9900, 
											GetModuleHandle(NULL), 
											NULL);
	CheckDlgButton(DataDialog, 9900, (!g_QuickLoadToggle ? BST_UNCHECKED : BST_CHECKED));
	g_CSDefaultFont = (HFONT)SendMessage(GetDlgItem(DataDialog, 1), WM_GETFONT, NULL, NULL);
	SendMessage(QuickLoadCheckBox, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);

	g_DataDlgOrgWindowProc = (WNDPROC)SetWindowLong(DataDialog, GWL_WNDPROC, (LONG)DataDlgSubClassProc);
}

void __declspec(naked) DataDlgInitHook(void)
{
	__asm
	{
		call	[kDataDlgInitCallAddr]

		pushad
		push	esi
		call	DoDataDlgInitHook
		popad

		jmp		[kDataDlgInitRetnAddr]
	}
}