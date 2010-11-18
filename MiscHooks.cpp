#include "MiscHooks.h"
#include "SEHooks.h"
#include "Common/CLIWrapper.h"
#include "Exports.h"
#include "ExtenderInternals.h"
#include "Common/HandshakeStructs.h"
#include "WindowManager.h"
#include "resource.h"

FormData*							UIL_FormData = new FormData();
UseListCellItemData*				UIL_CellData = new UseListCellItemData();

static const char*					g_AssetSelectorReturnPath = NULL;
static bool							g_DataHandlerPopulateModListInit_PerformCleanup = false;
static bool							g_DataHandlerPopulateModList_ProcessESPs = true;
static bool							g_DataHandlerPopulateModList_DefaultWorkspace = true;
static bool							g_DataHandlerPopulateModList_QuitReturn = true;
static char*						g_CustomWorkspacePath = new char[MAX_PATH];
static const char*					g_DefaultWorkspacePath = "Data\\";

const char*							g_DefaultWaterTextureStr = "Water\\dungeonwater01.dds";

bool								g_QuickLoadToggle = false;
static HFONT						g_CSDefaultFont = NULL;
bool								g_SaveAsRoutine = false;
ModEntry::Data*						g_SaveAsBuffer = NULL;
static bool							g_BitSwapBuffer = false;

void __stdcall SetWindowTextAddress(void);
void DispatchInteropMessage(void);


void __stdcall DoTestHook(void* Ref3DData)
{
	DumpClass(Ref3DData, 6);
}

void _declspec(naked) TestHook(void)
{
	static const UInt32 kReturn = 0x00497460, kCall = 0x004F4670;
	__asm
	{
		jmp		kReturn
	}
}



bool PatchMiscHooks()
{
	DoNop(&kCheckIsActivePluginAnESMPatch);
	PLACE_HOOK(LoadPluginsProlog);
	PLACE_HOOK(LoadPluginsEpilog);
	DoNop(&kResponseEditorMicPatch);

	SafeWrite8(kDataDialogPluginDescriptionPatchAddr, 0xEB);
	SafeWrite8(kDataDialogPluginAuthorPatchAddr, 0xEB);

	PLACE_HOOK(SavePluginCommonDialog);
	PLACE_HOOK(SavePluginMasterEnum);

	SafeWrite8(kDataHandlerPostErrorPatchAddr, 0xEB);	
	SafeWrite8(kEditorWarningPatchAddr, 0xEB);

	PLACE_HOOK(ExitCS);	
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
	DoNop(&kTESFormGetUnUsedFormIDPatch);

	PLACE_HOOK(NPCFaceGen);
	
	sprintf_s(g_CustomWorkspacePath, MAX_PATH, "Data");

	SafeWrite32(kDefaultWaterTextureFixPatchAddr, (UInt32)g_DefaultWaterTextureStr);
	PLACE_HOOK(QuickLoadPluginLoadHandlerPrologue);
	PLACE_HOOK(QuickLoadPluginLoadHandler);
	PLACE_HOOK(DataDlgInit);

	WriteRelJump(kMissingMasterOverridePatchAddr, kMissingMasterOverrideJumpAddr);

	if (g_INIManager->FetchSetting("LogCSWarnings")->GetValueAsInteger()) {
		PLACE_HOOK(CSWarningsDetour);
	}
	if (g_INIManager->FetchSetting("LogAssertions")->GetValueAsInteger()) {
		PLACE_HOOK(AssertOverride);
	}
	SafeWrite8(kTextureMipMapCheckPatchAddr, 0xEB);
	DoNop(&kAnimGroupNotePatch);

	SafeWrite8(kUnnecessaryCellEditsPatchAddr, 0xEB);
	SafeWrite8(kUnnecessaryDialogEditsPatchAddr, 0xEB);
//	PLACE_HOOK(RenderWindowPopupPatch);
	PLACE_HOOK(CustomCSWindowPatch);
	SafeWrite8(kRaceDescriptionDirtyEditPatchAddr, 0xEB);

	PLACE_HOOK(PluginSave);
	PLACE_HOOK(PluginLoad);
	PLACE_HOOK(AddListViewItem);
	PLACE_HOOK(ObjectListPopulateListViewItems);
	PLACE_HOOK(CellViewPopulateObjectList);

//	PLACE_HOOK(DoorMarkerProperties);		### TODO screws up dialog instantiation for no reason. messed up stack ?
	SafeWrite16(kDoorMarkerPropertiesHookAddr, 0x9090);


	if (CreateDirectory(std::string(g_AppPath + "Data\\Backup").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		DebugPrint("Couldn't create the Backup folder in Data directory");
	}

	OSVERSIONINFO OSInfo;
	GetVersionEx(&OSInfo);
	if (OSInfo.dwMajorVersion >= 6)		// if running on Windows Vista/7, fix the listview selection sound
		RegDeleteKey(HKEY_CURRENT_USER , "AppEvents\\Schemes\\Apps\\.Default\\CCSelect\\.Current");	
	
	return true;
}


SHORT __stdcall IsControlKeyDown(void)
{
	return GetAsyncKeyState(VK_CONTROL);
}

bool __stdcall InitTESFileSaveDlg()
{
	return DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_TESFILE), *g_HWND_CSParent, (DLGPROC)TESFileDlgProc);
}

void _declspec(naked) SavePluginCommonDialogHook(void)
{
	_asm
	{
		pushad
		call	InitTESFileSaveDlg
		test	eax, eax
		jnz		ESM

		popad
		jmp		[kSavePluginCommonDialogESPRetnAddr]
	ESM:
		popad
		jmp		[kSavePluginCommonDialogESMRetnAddr]
	}
}


void __stdcall DoLoadPluginsPrologHook(void)
{
	g_BitSwapBuffer = false;
	ModEntry::Data* ActiveFile = (*g_dataHandler)->unk8B8.activeFile;

	if (!ActiveFile ||
		(ActiveFile->flags & ModEntry::Data::kFlag_IsMaster) == 0)
		return;
	else {
		g_BitSwapBuffer = true;
		ToggleFlag(&ActiveFile->flags, ModEntry::Data::kFlag_IsMaster, 0);
	}
}

void _declspec(naked) LoadPluginsPrologHook(void)
{
	__asm
	{
		pushad
		call	DoLoadPluginsPrologHook
		popad

		call	[kLoadPluginsPrologCallAddr]
		jmp		[kLoadPluginsPrologRetnAddr]
	}
}

void __stdcall DoLoadPluginsEpilogHook(void)
{
	if (g_BitSwapBuffer) {
		ModEntry::Data* ActiveFile = (*g_dataHandler)->unk8B8.activeFile;

		if (!ActiveFile ||
			ActiveFile->flags & ModEntry::Data::kFlag_IsMaster)
			DebugPrint("Assertion Error - LoadPluginEpilog encountered a swapped master"), MessageBeep(MB_ICONHAND);
		else {
			ToggleFlag(&ActiveFile->flags, ModEntry::Data::kFlag_IsMaster, 1);
		}
	}
}

void _declspec(naked) LoadPluginsEpilogHook(void)
{
	__asm
	{
		pushad
		call	DoLoadPluginsEpilogHook
		popad

		call	[kLoadPluginsEpilogCallAddr]
		jmp		[kLoadPluginsEpilogRetnAddr]		
	}
}


bool __stdcall DoSavePluginMasterEnumHook(ModEntry::Data* CurrentFile)
{
	if (g_SaveAsRoutine && !_stricmp(g_SaveAsBuffer->name, CurrentFile->name))
		return false;
	else
		return true;
}

void _declspec(naked) SavePluginMasterEnumHook(void)
{
	__asm
	{
		push	ecx
		call	DoSavePluginMasterEnumHook
		test	eax, eax
		jz		SKIP
		jmp		[kSavePluginMasterEnumRetnPassAddr]
	SKIP:
		jmp		[kSavePluginMasterEnumRetnFailAddr]
	}
}


void __stdcall DoExitCSHook(HWND MainWindow)
{
	WritePositionToINI(MainWindow, NULL);
	WritePositionToINI(*g_HWND_CellView, "Cell View");
	WritePositionToINI(*g_HWND_ObjectWindow, "Object Window");
	WritePositionToINI(*g_HWND_RenderWindow, "Render Window");
	CONSOLE->SaveINISettings();
	g_INIManager->SaveSettingsToINI();
	ExitProcess(0);
}

void _declspec(naked) ExitCSHook(void)
{
	__asm
	{
		push    ebx
		call    DoExitCSHook
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

void __declspec(naked) UseInfoListInitHook(void)
{
	__asm
	{
		call	CLIWrapper::UseInfoList::OpenUseInfoBox
		jmp		[kUseInfoListInitRetnAddr]
	}
}

void PatchMenus()
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent),
		  GameplayMenu = GetSubMenu(MainMenu, 5),
		  ViewMenu = GetSubMenu(MainMenu, 2),
		  FileMenu = GetSubMenu(MainMenu, 0),
		  WorldMenu = GetSubMenu(MainMenu, 3);

	MENUITEMINFO ItemGameplayUseInfo, 
				ItemViewRenderWindow, 
				ItemDataSaveAs, 
				ItemWorldBatchEdit, 
				ItemViewConsole, 
				ItemViewModifiedRecords,
				ItemFileCSEPreferences;
	ItemGameplayUseInfo.cbSize = sizeof(MENUITEMINFO);
	ItemGameplayUseInfo.fMask = MIIM_STRING;
	ItemGameplayUseInfo.dwTypeData = "Use Info Listings";
	ItemGameplayUseInfo.cch = 15;
	SetMenuItemInfo(GameplayMenu, 245, FALSE, &ItemGameplayUseInfo);

	ItemViewRenderWindow.cbSize = sizeof(MENUITEMINFO);		// the tool coder seems to have mixed up the controlID for the button
	ItemViewRenderWindow.fMask = MIIM_ID|MIIM_STATE;		// as the code to handle hiding/showing is present in the wndproc
	ItemViewRenderWindow.wID = 40423;						// therefore we simply change it to the one that's expected by the proc
	ItemViewRenderWindow.fState = MFS_CHECKED;
	SetMenuItemInfo(ViewMenu, 40198, FALSE, &ItemViewRenderWindow);	

	ItemDataSaveAs.cbSize = sizeof(MENUITEMINFO);		
	ItemDataSaveAs.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;	
	ItemDataSaveAs.wID = 9901;
	ItemDataSaveAs.fState = MFS_ENABLED;
	ItemDataSaveAs.dwTypeData = "Save As";
	ItemDataSaveAs.cch = 7;
	InsertMenuItem(FileMenu, 40127, FALSE, &ItemDataSaveAs);

	ItemWorldBatchEdit.cbSize = sizeof(MENUITEMINFO);		
	ItemWorldBatchEdit.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;	
	ItemWorldBatchEdit.wID = 9902;
	ItemWorldBatchEdit.fState = MFS_ENABLED;
	ItemWorldBatchEdit.dwTypeData = "Batch Edit References";
	ItemWorldBatchEdit.cch = 0;
	InsertMenuItem(WorldMenu, 40194, FALSE, &ItemWorldBatchEdit);
	InsertMenuItem(*g_RenderWindowPopup, 293, FALSE, &ItemWorldBatchEdit);
	
	ItemViewConsole.cbSize = sizeof(MENUITEMINFO);		
	ItemViewConsole.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;	
	ItemViewConsole.wID = 9903;
	ItemViewConsole.fState = MFS_ENABLED|MFS_CHECKED;
	ItemViewConsole.dwTypeData = "Console Window";
	ItemViewConsole.cch = 0;
	InsertMenuItem(ViewMenu, 40455, FALSE, &ItemViewConsole);

	ItemViewModifiedRecords.cbSize = sizeof(MENUITEMINFO);		
	ItemViewModifiedRecords.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;	
	ItemViewModifiedRecords.wID = 9904;
	ItemViewModifiedRecords.fState = MFS_ENABLED|MFS_UNCHECKED;
	ItemViewModifiedRecords.dwTypeData = "Hide Unmodified Forms";
	ItemViewModifiedRecords.cch = 0;
	InsertMenuItem(ViewMenu, 40030, FALSE, &ItemViewModifiedRecords);

	ItemFileCSEPreferences.cbSize = sizeof(MENUITEMINFO);		
	ItemFileCSEPreferences.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;	
	ItemFileCSEPreferences.wID = 9905;
	ItemFileCSEPreferences.fState = MFS_ENABLED;
	ItemFileCSEPreferences.dwTypeData = "CSE Preferences";
	ItemFileCSEPreferences.cch = 0;
	InsertMenuItem(FileMenu, 40003, FALSE, &ItemFileCSEPreferences);
}

void __stdcall DoCSInitHook()
{
	if (!g_PluginPostLoad) return;			// prevents the hook from being called before the full init
											// perform deferred patching
											// remove hook rightaway to keep it from hindering the subclassing that follows
	for (UInt32 i = 0; i < sizeof(kCSInitCodeBuffer); i++) {
		SafeWrite8(kCSInitHookAddr + i, kCSInitCodeBuffer[i]);
	}

	PatchMenus();
	CLIWrapper::ScriptEditor::InitializeDatabaseUpdateTimer();

	CONSOLE->InitializeConsole();
	CONSOLE->LoadINISettings();

	(*g_DefaultWater)->texture.ddsPath.Set(g_DefaultWaterTextureStr);

	g_RenderWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_RenderWindow, GWL_WNDPROC, (LONG)RenderWndSubClassProc);
	g_CSMainWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_CSParent, GWL_WNDPROC, (LONG)CSMainWndSubClassProc);
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
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "nif");
		break;
	case 1:
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "kf");
		break;
	case 2:
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "wav");
		break;
	case 3:
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "dds");
		break;
	case 4:
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "spt");
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
		mov		eax, g_AssetSelectorReturnPath
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
	return _stricmp(CurrentFile->name, (*g_dataHandler)->unk8B8.activeFile->name);
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

void __stdcall DoAssertOverrideHook(UInt32 EIP)
{
	DebugPrint("{{ Assert call handled at 0x%08X }}", EIP);
	MessageBeep(MB_ICONHAND);
}

void __declspec(naked) AssertOverrideHook(void)
{
	__asm
	{
		mov		eax, [esp]
		sub		eax, 5
		pushad
		push	eax
		call	DoAssertOverrideHook
		popad

		jmp		[kAssertOverrideRetnAddr]
	}
}

void __stdcall DoCSWarningsDetourHook(LPCSTR DebugMessage)
{
	DebugPrint(Console::e_CS, "%s", DebugMessage);
}


void __declspec(naked) CSWarningsDetourHook(void)
{
	__asm
	{
		mov		esi, [esp + 0x4]
		pushad
		push	esi
		call	DoCSWarningsDetourHook
		popad
		xor		esi, esi

		sub		esp, 0x520
		jmp		[kCSWarningsDetourRetnAddr]
	}
}

void __stdcall DoRenderWindowPopupPatchHook()
{
	MENUITEMINFO ItemRenderBatchEdit;

	ItemRenderBatchEdit.cbSize = sizeof(MENUITEMINFO);		
	ItemRenderBatchEdit.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;	
	ItemRenderBatchEdit.wID = 9903;
	ItemRenderBatchEdit.fState = MFS_ENABLED;
	ItemRenderBatchEdit.dwTypeData = "Batch Edit References";
	ItemRenderBatchEdit.cch = 0;
	InsertMenuItem(*g_RenderWindowPopup, 293, FALSE, &ItemRenderBatchEdit);	
}

void __declspec(naked) RenderWindowPopupPatchHook(void)
{
	__asm
	{
		pushad
		call	DoRenderWindowPopupPatchHook
		popad
		call	GetPositionFromINI
		jmp		[kRenderWindowPopupPatchRetnAddr]
	}
}

bool __stdcall DoCustomCSWindowPatchHook(HWND Window)
{
									// enumerate custom windows here
	if (Window == CONSOLE->GetWindowHandle())
		return false;
	else
		return true;
}

void __declspec(naked) CustomCSWindowPatchHook(void)
{
	__asm
	{
		mov		edi, [g_HWND_CSParent]
		cmp		eax, edi
		jnz		FAIL
		xor		edi, edi
		pushad
		push	esi
		call	DoCustomCSWindowPatchHook
		test	eax, eax
		jz		FAIL
		popad
		mov		edi, esi
	FAIL:
		jmp		[kCustomCSWindowPatchRetnAddr]
	}

}

void __declspec(naked) PluginSaveHook(void)
{
    __asm
    {
		call	SetWindowTextAddress
		call	[g_WindowHandleCallAddr]				// SetWindowTextA
		pushad
		push	10
		call	SendPingBack
		popad
		jmp		[kPluginSaveRetnAddr]
    }
}

void __stdcall FixDefaultWater(void)
{
	(*g_DefaultWater)->texture.ddsPath.Set(g_DefaultWaterTextureStr);
}

void __stdcall DoHiddenFormsCheck(void)
{
	if (AreUnModifiedFormsHidden())
		ToggleHideUnModifiedForms(true);
}

void __declspec(naked) PluginLoadHook(void)
{
    __asm
    {
		pushad
		call	FixDefaultWater
		popad

		call	InitializeCSWindows

		pushad
		push	9
		call	SendPingBack
		call	DoHiddenFormsCheck
		popad

		jmp		[kPluginLoadRetnAddr]
    }
}

bool __stdcall PerformControlPopulationPrologCheck(TESForm* Form)
{
	if (AreUnModifiedFormsHidden() && ((Form->flags & TESForm::kFormFlags_FromActiveFile) == 0))
		return false;		// skip addition
	else
		return true;
}

UInt8 __stdcall CheckCallLocations(UInt32 CallAddress)
{
	switch (CallAddress)
	{
	case 0x00445C88:
	case 0x00445DC8:
	case 0x00445E6E:
	case 0x00452FA8:
	case 0x00440FBD:
	case 0x0040A4BF:
	case 0x00412F7A:
	case 0x0043FDFF:
	case 0x00442576:
	case 0x00452409:
	case 0x00560DC2:
		return 1;
	default:
		return 0;
	}
}

void __declspec(naked) AddListViewItemHook(void)
{
    __asm
    {
		mov		eax, [esp]
		sub		eax, 5
		pushad
		push	eax
		call	CheckCallLocations
		test	al, al
		jz		SKIP
		popad

		mov		eax, [esp + 8]
		pushad
		push	eax
		call	PerformControlPopulationPrologCheck
		test	al, al
		jz		EXIT
	SKIP:
		popad

		mov		ecx, [esp + 0x10]
		or		edx, 0x0FFFFFFFF
		jmp		[kAddListViewItemRetnAddr]
	EXIT:
		popad
		jmp		[kAddListViewItemExitAddr]
    }
}

void __declspec(naked) AddComboBoxItemHook(void)
{
    __asm
    {
		pushad
		push	[esp + 0xC]
		call	PerformControlPopulationPrologCheck
		test	al, al
		jz		EXIT
		popad

		sub		esp, 8
		push	esi
		mov		esi, [esp + 0x10]
		jmp		[kAddComboBoxItemRetnAddr]
	EXIT:
		popad
		jmp		[kAddComboBoxItemExitAddr]
    }
}

void __declspec(naked) ObjectListPopulateListViewItemsHook(void)
{
	__asm
	{
		mov		eax, [esp + 8]
		pushad
		push	eax
		call	PerformControlPopulationPrologCheck
		test	al, al
		jz		EXIT2
		popad

		sub		esp, 0x28
		mov		eax, [0x00A0AF40]		// object window menu item checked state
		cmp		eax, 0

		jmp		[kObjectListPopulateListViewItemsRetnAddr]
	EXIT2:
		popad
		jmp		[kObjectListPopulateListViewItemsExitAddr]
	}
}

void __declspec(naked) CellViewPopulateObjectListHook(void)
{
	__asm
	{
		mov		eax, [esp + 8]

		sub		esp, 0x28
		push	esi
		mov		esi, eax
		push	edi
		xor		edi, edi
		cmp		esi, edi

		jz		EXIT2

		pushad
		push	eax
		call	PerformControlPopulationPrologCheck
		test	al, al
		jz		EXIT1
		popad

		jmp		[kCellViewPopulateObjectListRetnAddr]
	EXIT1:
		popad
	EXIT2:
		jmp		[kCellViewPopulateObjectListExitAddr]
	}
}

void __declspec(naked) DoorMarkerPropertiesHook(void)
{
	__asm
	{
		pushad
		call	IsControlKeyDown
		test	al, al
		jnz		TELEPORT
		popad

		mov		edx, [g_HWND_CSParent]
		jmp		[kDoorMarkerPropertiesPropertiesAddr]
	TELEPORT:
		popad
		jmp		[kDoorMarkerPropertiesTeleportAddr]
	}
}