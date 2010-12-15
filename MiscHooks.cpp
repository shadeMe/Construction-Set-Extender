#include "MiscHooks.h"
#include "SEHooks.h"
#include "Common/CLIWrapper.h"
#include "Exports.h"
#include "ExtenderInternals.h"
#include "Common/HandshakeStructs.h"
#include "WindowManager.h"
#include "resource.h"
#include "HallofFame.h"

FormData*						UIL_FormData = new FormData();
UseListCellItemData*			UIL_CellData = new UseListCellItemData();
const char*						g_AssetSelectorReturnPath = NULL;
char*							g_CustomWorkspacePath = new char[MAX_PATH];
const char*						g_DefaultWorkspacePath = "Data\\";
const char*						g_DefaultWaterTextureStr = "Water\\dungeonwater01.dds";
bool							g_QuickLoadToggle = false;
static HFONT					g_CSDefaultFont = NULL;
bool							g_SaveAsRoutine = false;
ModEntry::Data*					g_SaveAsBuffer = NULL;
static bool						g_BitSwapBuffer = false;


MemHdlr							kSavePluginMasterEnum				(0x0047ECC6, SavePluginMasterEnumHook, 0, 0);
NopHdlr							kCheckIsActivePluginAnESM			(0x0040B65E, 2);
NopHdlr							kTESFormGetUnUsedFormID				(0x00486C08, 2);
MemHdlr							kLoadPluginsProlog					(0x00485252, LoadPluginsPrologHook, 0, 0);
MemHdlr							kLoadPluginsEpilog					(0x004856B2, LoadPluginsEpilogHook, 0, 0);
MemHdlr							kDataDialogPluginDescription		(0x0040CAB6, (UInt32)0, 0, 0);
MemHdlr							kDataDialogPluginAuthor				(0x0040CAFE, (UInt32)0, 0, 0);
MemHdlr							kSavePluginCommonDialog				(0x00446D51, SavePluginCommonDialogHook, 0, 0);
NopHdlr							kResponseEditorMic					(0x00407F3D, 5);
MemHdlr							kDataHandlerPostError				(0x004852F0, (UInt32)0, 0, 0);
MemHdlr							kExitCS								(0x00419354, ExitCSHook, 0, 0);
MemHdlr							kFindTextInit						(0x00419A42, FindTextInitHook, 0, 0);
MemHdlr							kCSInit								(0x00419260, CSInitHook, MakeUInt8Array(5, 0xE8, 0xEB, 0xC5, 0x2C, 0), 5);
MemHdlr							kUseInfoListInit					(0x00419833, UseInfoListInitHook, 0, 0);
NopHdlr							kMissingTextureWarning				(0x0044F3AF, 14);
NopHdlr							kTopicResultScriptResetNop			(0x004F49A0, 90);
MemHdlr							kTopicResultScriptReset				(0x004F49A0, 0x004F49FA, 0, 0);
MemHdlr							kNPCFaceGen							(0x004D76AC, NPCFaceGenHook, 0, 0);
MemHdlr							kDefaultWaterTextureFix				(0x0047F792, (UInt32)0, 0, 0);
MemHdlr							kDataDlgInit						(0x0040C6D7, DataDlgInitHook, 0, 0);
MemHdlr							kQuickLoadPluginLoadHandlerPrologue	(0x0040D073, QuickLoadPluginLoadHandlerPrologueHook, 0, 0);
MemHdlr							kQuickLoadPluginLoadHandler			(0x004852E5, QuickLoadPluginLoadHandlerHook, 0, 0);
MemHdlr							kMissingMasterOverride				(0x00484FC9, 0x00484E8E, 0, 0);
MemHdlr							kAssertOverride						(0x004B5670, AssertOverrideHook, 0, 0);
MemHdlr							kCSWarningsDetour					(0x004B5140, CSWarningsDetourHook, 0, 0);
MemHdlr							kTextureMipMapCheck					(0x0044F49B, (UInt32)0, 0, 0);
NopHdlr							kAnimGroupNote						(0x004CA21D, 5);
MemHdlr							kUnnecessaryDialogEdits				(0x004EDFF7, (UInt32)0, 0, 0);
MemHdlr							kRenderWindowPopup					(0x004297CE, RenderWindowPopupPatchHook, 0, 0);
MemHdlr							kUnnecessaryCellEdits				(0x005349A5, (UInt32)0, 0, 0);
MemHdlr							kCustomCSWindow						(0x004311E5, CustomCSWindowPatchHook, 0, 0);
MemHdlr							kRaceDescriptionDirtyEdit			(0x0049405C, (UInt32)0, 0, 0);
MemHdlr							kPluginSave							(0x0041BBCD, PluginSaveHook, 0, 0);
MemHdlr							kPluginLoad							(0x0041BEFA, PluginLoadHook, 0, 0);
MemHdlr							kAddListViewItem					(0x004038F0, AddListViewItemHook, 0, 0);
MemHdlr							kAddComboBoxItem					(0x00403540, AddComboBoxItemHook, 0, 0);
MemHdlr							kObjectListPopulateListViewItems	(0x00413980, ObjectListPopulateListViewItemsHook, 0, 0);
MemHdlr							kCellViewPopulateObjectList			(0x004087C0, CellViewPopulateObjectListHook, 0, 0);
MemHdlr							kDoorMarkerProperties				(0x00429EA9, DoorMarkerPropertiesHook, 0, 0);
MemHdlr							kAutoLoadActivePluginOnStartup		(0x0041A26A, AutoLoadActivePluginOnStartupHook, MakeUInt8Array(6, 0x8B, 0x0D, 0x44, 0xB6, 0xA0, 0x0), 6);
MemHdlr							kDataHandlerClearDataShadeMeRefDtor	(0x0047AE76, DataHandlerClearDataShadeMeRefDtorHook, 0, 0);
MemHdlr							kCellObjectListShadeMeRefAppend		(0x00445128, CellObjectListShadeMeRefAppendHook, 0, 0);
MemHdlr							kDeathToTheCloseOpenDialogsMessage	(0x0041BAA7, (UInt32)0, 0, 0);


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

	kLoadPluginsProlog.WriteJump();
	kLoadPluginsEpilog.WriteJump();
	kSavePluginCommonDialog.WriteJump();
	kSavePluginMasterEnum.WriteJump();
	kExitCS.WriteJump();
	kFindTextInit.WriteJump();
	kUseInfoListInit.WriteJump();
	kCSInit.WriteJump();
	kQuickLoadPluginLoadHandlerPrologue.WriteJump();
	kQuickLoadPluginLoadHandler.WriteJump();
	kDataDlgInit.WriteJump();
	kNPCFaceGen.WriteJump();
	kMissingMasterOverride.WriteJump();
	if (g_INIManager->FetchSetting("LogCSWarnings")->GetValueAsInteger())
	kCSWarningsDetour.WriteJump();
	if (g_INIManager->FetchSetting("LogAssertions")->GetValueAsInteger())
	kAssertOverride.WriteJump();
//	kRenderWindowPopupPatch.WriteJump();
	kCustomCSWindow.WriteJump();
	kPluginSave.WriteJump();
	kPluginLoad.WriteJump();
	kAddListViewItem.WriteJump();
	kObjectListPopulateListViewItems.WriteJump();
	kCellViewPopulateObjectList.WriteJump();
	kTopicResultScriptReset.WriteJump();

//	kDoorMarkerProperties.WriteJump();		### TODO screws up dialog instantiation for no reason.
	kDoorMarkerProperties.WriteUInt16(0x9090);

	kDataHandlerPostError.WriteUInt8(0xEB);	
	kDataDialogPluginDescription.WriteUInt8(0xEB);
	kDataDialogPluginAuthor.WriteUInt8(0xEB);
	kDefaultWaterTextureFix.WriteUInt32((UInt32)g_DefaultWaterTextureStr);
	kTextureMipMapCheck.WriteUInt8(0xEB);
	kUnnecessaryCellEdits.WriteUInt8(0xEB);
	kUnnecessaryDialogEdits.WriteUInt8(0xEB);
	kRaceDescriptionDirtyEdit.WriteUInt8(0xEB);
	kCheckIsActivePluginAnESM.WriteNop();
	kMissingTextureWarning.WriteNop();
	kResponseEditorMic.WriteNop(); 
	kTESFormGetUnUsedFormID.WriteNop();
	kAnimGroupNote.WriteNop();
	kDataHandlerClearDataShadeMeRefDtor.WriteJump();
	kCellObjectListShadeMeRefAppend.WriteJump();
	kDeathToTheCloseOpenDialogsMessage.WriteUInt8(0xEB);

	sprintf_s(g_CustomWorkspacePath, MAX_PATH, "Data");
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
	static const UInt32			kSavePluginCommonDialogESMRetnAddr = 0x00446D58;
	static const UInt32			kSavePluginCommonDialogESPRetnAddr = 0x00446D69;
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
	static const UInt32			kLoadPluginsPrologRetnAddr = 0x00485257;
	static const UInt32			kLoadPluginsPrologCallAddr = 0x00431310;
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
	static const UInt32			kLoadPluginsEpilogRetnAddr = 0x004856B7;
	static const UInt32			kLoadPluginsEpilogCallAddr = 0x0047DA60;
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
	static const UInt32			kSavePluginMasterEnumRetnPassAddr = 0x0047ECCF;
	static const UInt32			kSavePluginMasterEnumRetnFailAddr = 0x0047ECEB;
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
	static const UInt32			kExitCSJumpAddr = 0x004B52C1;
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
	static const UInt32			kFindTextInitRetnAddr = 0x00419A48;
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
	static const UInt32			kUseInfoListInitRetnAddr = 0x00419848;
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
	kCSInit.WriteBuffer();

	PatchMenus();
	CLIWrapper::ScriptEditor::InitializeDatabaseUpdateTimer();
	HallOfFame::Initialize(true);
	CONSOLE->InitializeConsole();
	CONSOLE->LoadINISettings();

	(*g_DefaultWater)->texture.ddsPath.Set(g_DefaultWaterTextureStr);

	g_RenderWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_RenderWindow, GWL_WNDPROC, (LONG)RenderWndSubClassProc);
	g_CSMainWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_CSParent, GWL_WNDPROC, (LONG)CSMainWndSubClassProc);

	if (g_INIManager->GET_INI_INT("LoadPluginOnStartup"))
		LoadStartupPlugin();

	if (g_INIManager->GET_INI_INT("OpenScriptWindowOnStartup"))
	{
		const char* ScriptID = g_INIManager->GET_INI_STR("StartupScriptEditorID");
		if (strcmp(ScriptID, "") && GetFormByID(ScriptID)) {
			g_EditorAuxScript = CS_CAST(GetFormByID(ScriptID), TESForm, Script);

			tagRECT ScriptEditorLoc;
			GetPositionFromINI("Script Edit", &ScriptEditorLoc);
			CLIWrapper::ScriptEditor::AllocateNewEditor(ScriptEditorLoc.left, ScriptEditorLoc.top, ScriptEditorLoc.right, ScriptEditorLoc.bottom);
			g_EditorAuxScript = NULL;
		}
		else
			SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CE1, 0);
	}

	
}


void __declspec(naked) CSInitHook(void)
{
	static const UInt32			kCSInitRetnAddr = 0x00419265;
	static const UInt32			kCSInitCallAddr = 0x006E5850;
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
	static const UInt32			kNPCFaceGenRetnAddr = 0x004D76B1;
	static const UInt32			kNPCFaceGenCallAddr = 0x0049C230;
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
	static const UInt32			kQuickLoadPluginLoadHandlerPrologueCallAddr = 0x0040CA30;
	static const UInt32			kQuickLoadPluginLoadHandlerPrologueRetnAddr = 0x0040D078;
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
	static const UInt32			kQuickLoadPluginLoadHandlerCallAddr = 0x00484A60;		// f_DataHandler::LoadTESFile
	static const UInt32			kQuickLoadPluginLoadHandlerRetnAddr = 0x004852EE;
	static const UInt32			kQuickLoadPluginLoadHandlerSkipAddr = 0x004852F0;
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
											NULL),
		 StartupPluginBtn = CreateWindowEx(0, 
											"BUTTON", 
											"Set As Startup Plugin", 
											WS_CHILD|WS_VISIBLE|WS_TABSTOP,
											474, 215, 130, 20, 
											DataDialog, 
											(HMENU)9906, 
											GetModuleHandle(NULL), 
											NULL);
	CheckDlgButton(DataDialog, 9900, (!g_QuickLoadToggle ? BST_UNCHECKED : BST_CHECKED));
	g_CSDefaultFont = (HFONT)SendMessage(GetDlgItem(DataDialog, 1), WM_GETFONT, NULL, NULL);
	SendMessage(QuickLoadCheckBox, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
	SendMessage(StartupPluginBtn, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);

	g_DataDlgOrgWindowProc = (WNDPROC)SetWindowLong(DataDialog, GWL_WNDPROC, (LONG)DataDlgSubClassProc);
}

void __declspec(naked) DataDlgInitHook(void)
{
	static const UInt32			kDataDlgInitRetnAddr = 0x0040C6DC;
	static const UInt32			kDataDlgInitCallAddr = 0x00404A90;
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
	DebugPrint("\t\tAssert call handled at 0x%08X !", EIP);
	MessageBeep(MB_ICONHAND);
}

void __declspec(naked) AssertOverrideHook(void)
{
	static const UInt32			kAssertOverrideRetnAddr = 0x004B575E;
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
	static const UInt32			kCSWarningsDetourRetnAddr = 0x004B52F2;
	__asm
	{
		mov		esi, [esp + 0x4]
		pushad
		push	esi
		call	DoCSWarningsDetourHook
		popad
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
	static const UInt32			kRenderWindowPopupPatchRetnAddr = 0x004297D3;
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
	static const UInt32			kCustomCSWindowPatchRetnAddr = 0x004311EF;
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
	static const UInt32			kPluginSaveRetnAddr	=	0x0041BBD3;
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
	static const UInt32			kPluginLoadRetnAddr	=	0x0041BEFF;
    __asm
    {
		pushad
		call	FixDefaultWater
		push	0
		call	HallOfFame::Initialize
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
	case 0x00445E12:	// ?
	case 0x00445D81:
	case 0x004F00C3:
		return 1;
	default:
		return 0;
	}
}

void __declspec(naked) AddListViewItemHook(void)
{
	static const UInt32			kAddListViewItemRetnAddr = 0x004038F7;
	static const UInt32			kAddListViewItemExitAddr = 0x0040396E;
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
	static const UInt32			kAddComboBoxItemRetnAddr = 0x00403548;
	static const UInt32			kAddComboBoxItemExitAddr = 0x004035F4;
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
	static const UInt32			kObjectListPopulateListViewItemsRetnAddr = 0x0041398A;
	static const UInt32			kObjectListPopulateListViewItemsExitAddr = 0x00413A50;
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
	static const UInt32			kCellViewPopulateObjectListRetnAddr = 0x004087D3;
	static const UInt32			kCellViewPopulateObjectListExitAddr = 0x004088AF;
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
	static const UInt32			kDoorMarkerPropertiesPropertiesAddr = 0x00429EB1;
	static const UInt32			kDoorMarkerPropertiesTeleportAddr = 0x00429EE8;
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

void __declspec(naked) AutoLoadActivePluginOnStartupHook(void)
{
	static const UInt32			kAutoLoadActivePluginOnStartupRetnAddr = 0x0041A284;
	__asm
	{
		mov		eax, 1
		jmp		[kAutoLoadActivePluginOnStartupRetnAddr]
	}
}

void __stdcall DestroyShadeMeRef(void)
{
	TESForm* Ref = GetFormByID("TheShadeMeRef");
	if (Ref)
		thisVirtualCall(kVTBL_TESObjectREFR, 0x34, Ref);
}

void __declspec(naked) DataHandlerClearDataShadeMeRefDtorHook(void)
{
	static const UInt32			kDataHandlerClearDataShadeMeRefDtorRetnAddr = 0x0047AE7B;
	__asm
	{
		lea     edi, [ebx+44h]
		mov     ecx, edi
		pushad
		call	DestroyShadeMeRef
		popad

		jmp		[kDataHandlerClearDataShadeMeRefDtorRetnAddr]
	}
}

void __stdcall AppendShadeMeRefToComboBox(HWND hWnd)
{
	TESForm* Ref = GetFormByID("TheShadeMeRef");
	sprintf_s(g_Buffer, sizeof(g_Buffer), "'shadeMe' 'TheShadeMeRef'");
	TESDialog_AddComboBoxItem(hWnd, g_Buffer, (LPARAM)Ref, 1);
}

void __declspec(naked) CellObjectListShadeMeRefAppendHook(void)
{
	static const UInt32			kCellObjectListShadeMeRefAppendRetnAddr = 0x0044512D;
	__asm
	{
		pushad
		push	edx
		call	AppendShadeMeRefToComboBox
		popad

		call	TESDialog_AddComboBoxItem
		jmp		[kCellObjectListShadeMeRefAppendRetnAddr]
	}
}