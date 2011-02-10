#include "MiscHooks.h"
#include "SEHooks.h"
#include "[Common]/CLIWrapper.h"
#include "Exports.h"
#include "ExtenderInternals.h"
#include "[Common]/HandshakeStructs.h"
#include "WindowManager.h"
#include "resource.h"
#include "HallofFame.h"
#include "CSInterop.h"
#include "WindowManager.h"

FormData*						UIL_FormData = new FormData();
UseListCellItemData*			UIL_CellData = new UseListCellItemData();
const char*						g_AssetSelectorReturnPath = NULL;
const char*						g_DefaultWaterTextureStr = "Water\\dungeonwater01.dds";
bool							g_QuickLoadToggle = false;
HFONT							g_CSDefaultFont = NULL;
bool							g_BitSwapBuffer = false;
char							g_NumericIDWarningBuffer[0x10] = {0};

extern FARPROC					g_WindowHandleCallAddr;


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
MemHdlr							kMessagePumpInit					(0x0041CF6F, MessagePumpInitHook, MakeUInt8Array(5, 0x8B, 0x3D, 0x58, 0x43, 0x92), 5);
MemHdlr							kCSInit								(0x00419260, CSInitHook, MakeUInt8Array(5, 0xE8, 0xEB, 0xC5, 0x2C, 0), 5);
MemHdlr							kUseInfoListInit					(0x00419833, UseInfoListInitHook, 0, 0);
NopHdlr							kMissingTextureWarning				(0x0044F3AF, 14);
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
MemHdlr							kUnnecessaryCellEdits				(0x005349A5, (UInt32)0, 0, 0);
MemHdlr							kCustomCSWindow						(0x004311E5, CustomCSWindowPatchHook, 0, 0);
MemHdlr							kRaceDescriptionDirtyEdit			(0x0049405C, (UInt32)0, 0, 0);
MemHdlr							kPluginSave							(0x0041BBCD, PluginSaveHook, 0, 0);
MemHdlr							kPluginLoad							(0x0041BEFA, PluginLoadHook, 0, 0);
MemHdlr							kAddListViewItem					(0x004038F0, AddListViewItemHook, 0, 0);
MemHdlr							kAddComboBoxItem					(0x00403540, AddComboBoxItemHook, 0, 0);
MemHdlr							kObjectListPopulateListViewItems	(0x00413980, ObjectListPopulateListViewItemsHook, 0, 0);
MemHdlr							kCellViewPopulateObjectList			(0x004087C0, CellViewPopulateObjectListHook, 0, 0);
MemHdlr							kDoorMarkerProperties				(0x00429EA1, DoorMarkerPropertiesHook, 0, 0);
MemHdlr							kAutoLoadActivePluginOnStartup		(0x0041A26A, AutoLoadActivePluginOnStartupHook, MakeUInt8Array(6, 0x8B, 0x0D, 0x44, 0xB6, 0xA0, 0x0), 6);
MemHdlr							kDataHandlerClearDataShadeMeRefDtor	(0x0047AE76, DataHandlerClearDataShadeMeRefDtorHook, 0, 0);
MemHdlr							kCellObjectListShadeMeRefAppend		(0x00445128, CellObjectListShadeMeRefAppendHook, 0, 0);
MemHdlr							kDeathToTheCloseOpenDialogsMessage	(0x0041BAA7, (UInt32)0, 0, 0);
MemHdlr							kTopicInfoCopyProlog				(0x004F0738, 0x004F07C4, 0, 0);
MemHdlr							kTopicInfoCopyEpilog				(0x004F1280, TopicInfoCopyEpilogHook, 0, 0);
MemHdlr							kTESDialogPopupMenu					(0x004435A6, TESDialogPopupMenuHook, 0, 0);
MemHdlr							kResponseWindowLipButtonPatch		(0x004EC0E7, 0x004EC0F7, 0, 0);
MemHdlr							kResponseWindowInit					(0x004EBA81, ResponseWindowInitHook, 0, 0);
MemHdlr							kNumericEditorID					(0x00497670, NumericEditorIDHook, 0, 0);
MemHdlr							kDataHandlerConstructSpecialForms	(0x00481049, DataHandlerConstructSpecialFormsHook, 0, 0);
MemHdlr							kResultScriptSaveForm				(0x004FD258, ResultScriptSaveFormHook, 0, 0);
MemHdlr							kDataDlgZOrder						(0x0040C530, 0x0040C552, 0, 0);
MemHdlr							kFormIDListViewInit					(0x00448A8A, FormIDListViewInitHook, 0, 0);
MemHdlr							kFormIDListViewSaveChanges			(0x0044957A, FormIDListViewSaveChangesHook, 0, 0);
MemHdlr							kFormIDListViewItemChange			(0x00448DEC, FormIDListViewItemChangeHook, 0, 0);
MemHdlr							kFormIDListViewSelectItem			(0x00403B3D, FormIDListViewSelectItemHook, 0, 0);
MemHdlr							kFormIDListViewDuplicateSelection	(0x004492AE, FormIDListViewDuplicateSelectionHook, 0, 0);
MemHdlr							kTESRaceCopyHairEyeDataInit			(0x004E9735, TESRaceCopyHairEyeDataInitHook, 0, 0);
MemHdlr							kTESRaceCopyHairEyeDataMessageHandler
																	(0x004E8FE1, TESRaceCopyHairEyeDataMessageHandlerHook, 0, 0);
NopHdlr							kTESDialogSubwindowEnumChildCallback
																	(0x00404E69, 3);
MemHdlr							kTESObjectREFRDoCopyFrom			(0x0054763D, TESObjectREFRDoCopyFromHook, 0, 0);


bool PatchMiscHooks()
{
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
	kCustomCSWindow.WriteJump();
	kPluginSave.WriteJump();
	kPluginLoad.WriteJump();
	kAddListViewItem.WriteJump();
	kObjectListPopulateListViewItems.WriteJump();
	kCellViewPopulateObjectList.WriteJump();
	kTopicResultScriptReset.WriteJump();
	kDoorMarkerProperties.WriteJump();
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
	kTopicInfoCopyProlog.WriteJump();
	kTopicInfoCopyEpilog.WriteJump();
	kTESDialogPopupMenu.WriteJump();
	kResponseWindowLipButtonPatch.WriteJump();
	kResponseWindowInit.WriteJump();
	kNumericEditorID.WriteJump();
	kDataHandlerConstructSpecialForms.WriteJump();
	kResultScriptSaveForm.WriteJump();
	kDataDlgZOrder.WriteJump();
	kFormIDListViewInit.WriteJump();
	kFormIDListViewSaveChanges.WriteJump();
	kFormIDListViewItemChange.WriteJump();
	kFormIDListViewSelectItem.WriteJump();
	kFormIDListViewDuplicateSelection.WriteJump();
	kTESRaceCopyHairEyeDataInit.WriteJump();
	kTESRaceCopyHairEyeDataMessageHandler.WriteJump();
	kTESDialogSubwindowEnumChildCallback.WriteNop();
	kTESObjectREFRDoCopyFrom.WriteJump();

	COMMON_DIALOG_CANCEL_PATCH(Model);
	COMMON_DIALOG_CANCEL_PATCH(Animation);
	COMMON_DIALOG_CANCEL_PATCH(Sound);
	COMMON_DIALOG_CANCEL_PATCH(Texture);
	COMMON_DIALOG_CANCEL_PATCH(SPT);

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


	if (CreateDirectory(std::string(g_AppPath + "Data\\Backup").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
		DebugPrint("Couldn't create the Backup folder in Data directory");

	OSVERSIONINFO OSInfo;
	GetVersionEx(&OSInfo);
	if (OSInfo.dwMajorVersion >= 6)		// if running on Windows Vista/7, fix the listview selection sound
		RegDeleteKey(HKEY_CURRENT_USER , "AppEvents\\Schemes\\Apps\\.Default\\CCSelect\\.Current");	
	
	return true;
}


UInt32 __stdcall IsControlKeyDown(void)
{
	return GetAsyncKeyState(VK_CONTROL);
}

bool __stdcall InitTESFileSaveDlg()
{
	return DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_TESFILE), *g_HWND_CSParent, (DLGPROC)TESFileDlgProc);
}

void __declspec(naked) SavePluginCommonDialogHook(void)
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

	if (ActiveFile && (ActiveFile->flags & ModEntry::Data::kFlag_IsMaster))
	{
		g_BitSwapBuffer = true;
		ToggleFlag(&ActiveFile->flags, ModEntry::Data::kFlag_IsMaster, 0);
	}

	sprintf_s(g_NumericIDWarningBuffer, 0x10, "%s", g_INIManager->GET_INI_STR("ShowNumericEditorIDWarning"));
	static const char* Zero = "0";
	g_INIManager->FetchSetting("ShowNumericEditorIDWarning")->SetValue(Zero);
}

void __declspec(naked) LoadPluginsPrologHook(void)
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
	if (g_BitSwapBuffer)
	{
		ModEntry::Data* ActiveFile = (*g_dataHandler)->unk8B8.activeFile;

		if (!ActiveFile || ActiveFile->flags & ModEntry::Data::kFlag_IsMaster)
			DebugPrint("Assertion Error - LoadPluginEpilog encountered a swapped master"), MessageBeep(MB_ICONHAND);
		else {
			ToggleFlag(&ActiveFile->flags, ModEntry::Data::kFlag_IsMaster, 1);
		}
	}

	g_INIManager->FetchSetting("ShowNumericEditorIDWarning")->SetValue(g_NumericIDWarningBuffer);
}

void __declspec(naked) LoadPluginsEpilogHook(void)
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
	if ((CurrentFile->flags & ModEntry::Data::kFlag_Loaded) == 0)
		return false;
	else
		return true;
}

void __declspec(naked) SavePluginMasterEnumHook(void)
{
	static const UInt32			kSavePluginMasterEnumRetnPassAddr = 0x0047ECCF;
	static const UInt32			kSavePluginMasterEnumRetnFailAddr = 0x0047ECEB;
	__asm
	{
		pushad
		push	ecx
		call	DoSavePluginMasterEnumHook
		test	al, al
		jz		SKIP

		popad
		jmp		[kSavePluginMasterEnumRetnPassAddr]
	SKIP:
		popad
		jmp		[kSavePluginMasterEnumRetnFailAddr]
	}
}


void __stdcall DoExitCSHook(HWND MainWindow)
{
	WritePositionToINI(MainWindow, NULL);
	WritePositionToINI(*g_HWND_CellView, "Cell View");
	WritePositionToINI(*g_HWND_ObjectWindow, "Object Window");
	WritePositionToINI(*g_HWND_RenderWindow, "Render Window");
	g_INIManager->SaveSettingsToINI();

	CONSOLE->Deinitialize();
	CSIOM->Deinitialize();
	
	ExitProcess(0);
}

void __declspec(naked) ExitCSHook(void)
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
		push	0
		call	CLIWrapper::UseInfoList::OpenUseInfoBox
		jmp		[kUseInfoListInitRetnAddr]
	}
}

void __stdcall DoCSInitHook()
{
	if (!g_PluginPostLoad) 
		return;
											// prevents the hook from being called before the full init
											// perform deferred patching
											// remove hook rightaway to keep it from hindering the subclassing that follows
	kCSInit.WriteBuffer();

	InitializeWindowManager();
	CLIWrapper::ScriptEditor::InitializeDatabaseUpdateTimer();
	HallOfFame::Initialize(true);
	CONSOLE->InitializeConsole();
	CONSOLE->LoadINISettings();

	(*g_DefaultWater)->texture.ddsPath.Set(g_DefaultWaterTextureStr);

	if (g_INIManager->GET_INI_INT("LoadPluginOnStartup"))
		LoadStartupPlugin();

	if (g_INIManager->GET_INI_INT("OpenScriptWindowOnStartup"))
	{
		const char* ScriptID = g_INIManager->GET_INI_STR("StartupScriptEditorID");
		if (strcmp(ScriptID, "") && GetFormByID(ScriptID))
			SpawnCustomScriptEditor(ScriptID);
		else
			SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CE1, 0);
	}

	LoadedMasterArchives();
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

void __declspec(naked) MessagePumpInitHook(void)
{
	static const UInt32			kMessagePumpInitRetnAddr = 0x0041CF75;
	__asm
	{
		mov		g_PluginPostLoad, 1

		_emit	0x8B
		_emit	0x3D
		_emit	0x58
		_emit	0x43
		_emit	0x92
		_emit	0

		jmp		[kMessagePumpInitRetnAddr]
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
		mov		ebx, eax

		mov     eax, [esi + 0x24]
		push    ebx
		push    eax
		lea     ecx, [ebp - 0x14]
		mov     byte ptr [ebp - 0x4], 1
		call    kBSString_Set

		cmp		ebx, e_FetchPath
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
	if (IsDlgButtonChecked(DataDlg, DATA_QUICKLOAD) == BST_CHECKED)
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
	RECT DlgRect;
	GetClientRect(DataDialog, &DlgRect);

	HWND QuickLoadCheckBox = CreateWindowEx(0, 
											"BUTTON", 
											"Quick-Load Plugin", 
											BS_AUTOCHECKBOX|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
											DlgRect.right - 141, DlgRect.bottom - 82, 142, 15, 
											DataDialog, 
											(HMENU)DATA_QUICKLOAD, 
											GetModuleHandle(NULL), 
											NULL),
		 StartupPluginBtn = CreateWindowEx(0, 
											"BUTTON", 
											"Set As Startup Plugin", 
											WS_CHILD|WS_VISIBLE|WS_TABSTOP,
											DlgRect.right - 141, DlgRect.bottom - 64, 130, 20, 
											DataDialog, 
											(HMENU)DATA_SETSTARTUPPLUGIN, 
											GetModuleHandle(NULL), 
											NULL);

	CheckDlgButton(DataDialog, DATA_QUICKLOAD, (!g_QuickLoadToggle ? BST_UNCHECKED : BST_CHECKED));

	SendMessage(QuickLoadCheckBox, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
	SendMessage(StartupPluginBtn, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);

	g_DataDlgOrgWindowProc = (WNDPROC)SetWindowLong(DataDialog, GWL_WNDPROC, (LONG)DataDlgSubClassProc);
	SetWindowPos(DataDialog, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
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

void __declspec(naked) PluginLoadHook(void)
{
	static const UInt32			kPluginLoadRetnAddr	=	0x0041BEFF;
    __asm
    {
		call	InitializeCSWindows

		pushad
		push	9
		call	SendPingBack
		call	FormEnumerationWrapper::ResetFormVisibility
		popad

		jmp		[kPluginLoadRetnAddr]
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
		call	FormEnumerationWrapper::PerformListViewPrologCheck
		test	al, al
		jz		SKIP
		popad

		mov		eax, [esp + 8]
		pushad
		push	eax
		call	FormEnumerationWrapper::GetShouldEnumerateForm
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
		call	FormEnumerationWrapper::GetShouldEnumerateForm
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
		call	FormEnumerationWrapper::GetShouldEnumerateForm
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
		call	FormEnumerationWrapper::GetShouldEnumerateForm
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
	static const UInt32			kDoorMarkerPropertiesPropertiesAddr = 0x00429EAB;
	static const UInt32			kDoorMarkerPropertiesTeleportAddr = 0x00429EE8;
	__asm
	{
		mov		eax, [esi + 0x8]
		shr		eax, 0x0E
		test	al, 1
		jnz		DOORMARKER	

		jmp		[kDoorMarkerPropertiesPropertiesAddr]
	TELEPORT:
		popad
		jmp		[kDoorMarkerPropertiesTeleportAddr]
	DOORMARKER:
		pushad
		call	IsControlKeyDown
		test	eax, eax
		jnz		TELEPORT
		popad

		jmp		[kDoorMarkerPropertiesPropertiesAddr]
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

void __declspec(naked) TopicInfoCopyEpilogHook(void)
{
	static const UInt32			kTopicInfoCopyEpilogHookRetnAddr = 0x004F1286;
	__asm
	{
		pushad
		mov		eax, [esi]
		mov		eax, [eax + 0x94]		// SetFromActiveFile
		push	1
		mov		ecx, esi
		call	eax
		popad

		mov     [esi + 0x30], bx
		mov     eax, [edi]
		jmp		[kTopicInfoCopyEpilogHookRetnAddr]
	}
}


void __stdcall InsertFormListPopupMenuItems(HMENU Menu, TESForm* SelectedForm)
{
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_SEPARATOR, NULL, NULL);
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_SETFORMID, "Set FormID");
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_MARKUNMODIFIED, "Mark As Unmodified");
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_UNDELETE, "Undelete");
	if (SelectedForm->IsReference())
	{
		InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_EDITBASEFORM, "Edit Base Form");
	}
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_JUMPTOUSEINFOLIST, "Jump To Central Use Info List");
}
void __stdcall HandleHookedPopup(HWND Parent, int MenuIdentifier, TESForm* SelectedObject)
{
	switch (MenuIdentifier)
	{
	case POPUP_SETFORMID:
	case POPUP_MARKUNMODIFIED:
	case POPUP_JUMPTOUSEINFOLIST:
	case POPUP_UNDELETE:
	case POPUP_EDITBASEFORM:
		EvaluatePopupMenuItems(Parent, MenuIdentifier, SelectedObject);
		break;
	default:
		SendMessage(Parent, WM_COMMAND, (WPARAM)MenuIdentifier, NULL);
		break;
	}
}
void __stdcall RemoveFormListPopupMenuItems(HMENU Menu)
{
	DeleteMenu(Menu, POPUP_SETFORMID, MF_BYCOMMAND);
	DeleteMenu(Menu, POPUP_MARKUNMODIFIED, MF_BYCOMMAND);
	DeleteMenu(Menu, POPUP_JUMPTOUSEINFOLIST, MF_BYCOMMAND);
	DeleteMenu(Menu, POPUP_UNDELETE, MF_BYCOMMAND);
	DeleteMenu(Menu, POPUP_EDITBASEFORM, MF_BYCOMMAND);
	DeleteMenu(Menu, GetMenuItemCount(Menu) - 1, MF_BYPOSITION);
}


void __declspec(naked) TESDialogPopupMenuHook(void)
{
	static const UInt32			kTESDialogPopupMenuHookRetnAddr = 0x004435C3;

	static HWND					ParentHWND = NULL;
	__asm
	{
		pushad
		call	TrackPopupMenuAddress
		popad

		mov		eax, [esp + 0x18]
		mov		ParentHWND, eax
		push	0
		push	eax
		mov		eax, [esp + 0x1C]
		mov		ecx, [eax + 4]
		mov		edx, [eax]
		push	0
		push	ecx
		push	edx

		test	ebx, ebx
		jz		SKIP

		pushad
		push	ebx
		push	esi
		call	InsertFormListPopupMenuItems
		popad

		push	0x102
		push	esi
		call	g_WindowHandleCallAddr

		pushad
		push	esi
		call	RemoveFormListPopupMenuItems
		popad

		push	ebx
		push	eax
		push	ParentHWND
		call	HandleHookedPopup
		jmp		[kTESDialogPopupMenuHookRetnAddr]
	SKIP:
		push	2
		push	esi
		call	g_WindowHandleCallAddr

		jmp		[kTESDialogPopupMenuHookRetnAddr]
	}
}

void __stdcall DoResponseWindowInitHook(HWND hWnd)
{
	g_ResponseWndOrgWindowProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)ResponseWndSubClassProc);
	
	ShowWindow(GetDlgItem(hWnd, 2220), SW_HIDE);
	ShowWindow(GetDlgItem(hWnd, 2221), SW_HIDE);
	ShowWindow(GetDlgItem(hWnd, 2222), SW_HIDE);

	EnableWindow(GetDlgItem(hWnd, 2379), FALSE);
	EnableWindow(GetDlgItem(hWnd, 2380), FALSE);
	EnableWindow(GetDlgItem(hWnd, 1016), TRUE);

	CheckRadioButton(hWnd, 2379, 2380, 2379);
	SetWindowText(GetDlgItem(hWnd, 2379), "From MP3");

	SetWindowText(GetDlgItem(hWnd, 2223), "Copy External File");
	SetWindowPos(GetDlgItem(hWnd, 2223), HWND_TOP, 150, 550, 105, 20, SWP_NOZORDER|SWP_SHOWWINDOW);
}

void __declspec(naked) ResponseWindowInitHook(void)
{
	static UInt32 kResponseWindowInitHookRetnAddr = 0x004EBA92;
	__asm
	{
		push	ebp
		call	DoResponseWindowInitHook
		jmp		[kResponseWindowInitHookRetnAddr]
	}
}

void __stdcall DoNumericEditorIDHook(const char* EditorID)
{
	if (g_INIManager->FetchSetting("ShowNumericEditorIDWarning")->GetValueAsInteger() && 
		g_PluginPostLoad && 
		strlen(EditorID) > 0 && 
		isdigit((int)*EditorID))
	{
		sprintf_s(g_Buffer, sizeof(g_Buffer), "The editorID '%s' begins with an integer.\n\nWhile this is generally acceptable by the engine, scripts referring this form might fail to run or compile as the script compiler can attempt to parse it as an integer.\n\nConsider starting the editorID with an alphabet.", EditorID);
		MessageBox(*g_HWND_CSParent, g_Buffer, "CSE", MB_OK|MB_ICONWARNING);
	}
}

void __declspec(naked) NumericEditorIDHook(void)
{
	static UInt32 kNumericEditorIDHookRetnAddr = 0x00497676;
	__asm
	{
		mov		eax, [esp + 0x4]
		pushad
		push	eax
		call	DoNumericEditorIDHook
		popad

		xor		eax, eax
		push	ebp
		mov		ebp, esp
		sub		esp, 0x10
		jmp		[kNumericEditorIDHookRetnAddr]
	}
}

void __declspec(naked) DataHandlerConstructSpecialFormsHook(void)
{
	static UInt32 kDataHandlerConstructSpecialFormsHookRetnAddr = 0x0048104E;
	static UInt32 kDataHandlerConstructSpecialFormsHookCallAddr = 0x00505070;
	__asm
	{
		pushad
		call	FixDefaultWater
		push	0
		call	HallOfFame::Initialize
		popad

		call	[kDataHandlerConstructSpecialFormsHookCallAddr]
		jmp		[kDataHandlerConstructSpecialFormsHookRetnAddr]
	}
}



void __declspec(naked) ResultScriptSaveFormHook(void)
{
	static UInt32 kResultScriptSaveFormHookRetnAddr = 0x004FD260;
	__asm
	{
		mov		eax, [ecx]
		mov		edx, [eax + 0x8]
		test	edx, edx
		jz		FAIL

		jmp		[kResultScriptSaveFormHookRetnAddr]
	FAIL:
		mov		eax, 0x004FD271
		jmp		eax
	}
}

void __stdcall DoFormIDListViewInitHook(HWND hWnd)
{
	if (hWnd != *g_HWND_QuestWindow)
	{
		SetWindowText(GetDlgItem(hWnd, 1), "Apply");
		SetWindowText(GetDlgItem(hWnd, 2), "Close");
	}
}

void __declspec(naked) FormIDListViewInitHook(void)
{
	static UInt32 kFormIDListViewInitHookRetnAddr = 0x00448A94;
	static UInt32 kByteAddr = 0x00A0BE45;
	__asm
	{
		pushad
		push	esi
		call	DoFormIDListViewInitHook
		popad

		push	5
		push	esi
		mov		[kByteAddr], 0

		jmp		[kFormIDListViewInitHookRetnAddr]
	}
}

UInt32 __stdcall DoFormIDListViewSaveChangesHookProlog(HWND Parent)
{
	return Parent != *g_HWND_QuestWindow;
}

void __stdcall DoFormIDListViewSaveChangesHookEpilog(HWND Parent)
{
	if (IsWindowEnabled(GetDlgItem(Parent, 1)))
	{
		TESForm* LocalCopy = TESDialog_GetDialogExtraLocalCopy(Parent);
		TESForm* WorkingCopy = TESDialog_GetDialogExtraParam(Parent);

		if (WorkingCopy)
		{
			thisVirtualCall(*((UInt32*)LocalCopy), 0x118, LocalCopy, Parent);						// GetDataFromDialog
			if (thisVirtualCall(*((UInt32*)WorkingCopy), 0xBC, WorkingCopy, LocalCopy))				// CompareTo
			{
				if (thisVirtualCall(*((UInt32*)WorkingCopy), 0x104, WorkingCopy, LocalCopy))		// UpdateUsageInfo
				{
					thisVirtualCall(*((UInt32*)WorkingCopy), 0x94, LocalCopy, 1);					// SetFromActiveFile
					thisVirtualCall(*((UInt32*)WorkingCopy), 0xB8, WorkingCopy, LocalCopy);			// CopyFrom
				}
			}
		}
	}
}

void __declspec(naked) FormIDListViewSaveChangesHook(void)
{
	static UInt32 kFormIDListViewSaveChangesHookQuestRetnAddr = 0x00449580;
	static UInt32 kFormIDListViewSaveChangesHookExitRetnAddr = 0x00448BF0;	
	__asm
	{
		push	esi
		call	TESDialog_GetDialogExtraParam

		pushad
		push	esi
		call	DoFormIDListViewSaveChangesHookProlog
		test	eax, eax
		jnz		NOTQUEST
		popad

		jmp		[kFormIDListViewSaveChangesHookQuestRetnAddr]
	NOTQUEST:
		popad

		pushad
		push	esi
		call	DoFormIDListViewSaveChangesHookEpilog
		popad

		jmp		[kFormIDListViewSaveChangesHookExitRetnAddr]
	}
}

int __stdcall DoFormIDListViewItemChangeHook(HWND Parent)
{
	return MessageBox(Parent, "Save Changes made to the active form?", "CSE", MB_YESNO);
}

void __declspec(naked) FormIDListViewItemChangeHook(void)
{
	static UInt32 kFormIDListViewItemChangeHookRetnAddr = 0x00448DF4;
	__asm
	{
		pushad
		push	esi
		call	DoFormIDListViewItemChangeHook
		cmp		eax, IDYES
		jnz		REVERT
		popad

		mov		eax, [edi]
		mov		edx, [eax + 0x104]

		jmp		[kFormIDListViewItemChangeHookRetnAddr]
	REVERT:
		popad

		mov		eax, 0x004494C9
		jmp		eax
	}
}

void __stdcall DoFormIDListViewSelectItemHook(HWND ListView, int ItemIndex)
{
	SetFocus(ListView);
	ListView_SetItemState(ListView, ItemIndex, LVIS_FOCUSED | LVIS_SELECTED, 0x000F); 
}

void __declspec(naked) FormIDListViewSelectItemHook(void)
{
	static UInt32 kFormIDListViewSelectItemHookRetnAddr = 0x00403B8A;
	__asm
	{
		pushad
		push	ebx
		push	esi
		call	DoFormIDListViewSelectItemHook
		popad

		jmp		[kFormIDListViewSelectItemHookRetnAddr]
	}
}

void __declspec(naked) FormIDListViewDuplicateSelectionHook(void)
{
	static UInt32 kFormIDListViewDuplicateSelectionHookRetnAddr = 0x004492B7;
	__asm
	{
		add		eax, 1
		push	eax
		push	0x810
		push	esi
		call	edi

		jmp		[kFormIDListViewDuplicateSelectionHookRetnAddr]
	}
}

void __stdcall DoTESRaceCopyHairEyeDataInitHook(Subwindow* RaceDialogSubWindow, UInt8 Template)
{
	if (Template == 2)
	{
		HWND Parent = RaceDialogSubWindow->hDialog;

		HWND CopyHairButton = CreateWindowEx(0, 
												"BUTTON", 
												"Copy Hair From Race", 
												WS_CHILD|WS_VISIBLE|WS_TABSTOP,
												RaceDialogSubWindow->position.x + 175 - 42, RaceDialogSubWindow->position.y + 285, 165, 25, 
												Parent, 
												(HMENU)RACE_COPYHAIR, 
												GetModuleHandle(NULL), 
												NULL);

		HWND CopyEyesButton = CreateWindowEx(0, 
												"BUTTON", 
												"Copy Eyes From Race", 
												WS_CHILD|WS_VISIBLE|WS_TABSTOP,
												RaceDialogSubWindow->position.x + 175 + 130, RaceDialogSubWindow->position.y + 285, 165, 25,
												Parent, 
												(HMENU)RACE_COPYEYES, 
												GetModuleHandle(NULL), 
												NULL);

		SendMessage(CopyHairButton, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
		SendMessage(CopyEyesButton, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);	

		thisCall(kLinkedListNode_NewNode, &RaceDialogSubWindow->controls, CopyHairButton);
		thisCall(kLinkedListNode_NewNode, &RaceDialogSubWindow->controls, CopyEyesButton);
	}
}

void __declspec(naked) TESRaceCopyHairEyeDataInitHook(void)
{
	static UInt32 kTESRaceCopyHairEyeDataInitHookRetnAddr = 0x004E973A;
	static UInt32 kTESRaceCopyHairEyeDataInitHookCallAddr = 0x004E83B0;
	__asm
	{
		call	[kTESRaceCopyHairEyeDataInitHookCallAddr]

		pushad
		push	ebx
		push	edi
		call	DoTESRaceCopyHairEyeDataInitHook
		popad
		jmp		[kTESRaceCopyHairEyeDataInitHookRetnAddr]
	}
}

UInt32 __stdcall DoTESRaceCopyHairEyeDataMessageHandlerHook(HWND Dialog, INT Identifier, TESRace* WorkingRace)
{
	switch (Identifier)
	{
	case RACE_COPYEYES:
	case RACE_COPYHAIR:
		TESForm* Selection = (TESForm*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TESCOMBOBOX), Dialog, (DLGPROC)TESComboBoxDlgProc, (LPARAM)kFormType_Race);
		if (Selection)
		{
			TESRace* SelectedRace = CS_CAST(Selection, TESForm, TESRace);

			if (WorkingRace)
			{
				int Count = 0;
				if (Identifier == RACE_COPYEYES)
				{
					GenericNode<TESEyes>* Source = (GenericNode<TESEyes>*)&SelectedRace->eyes;
					tList<TESEyes>* Destination = &WorkingRace->eyes;

					Destination->RemoveAll();

					for (;Source && Source->data; Source = Source->next, Count++)
						thisCall(kLinkedListNode_NewNode, Destination, Source->data);

					sprintf_s(g_Buffer, sizeof(g_Buffer), "Copied %d eye forms from race '%s'.", Count, SelectedRace->editorData.editorID.m_data);
					MessageBox(Dialog, g_Buffer, "CSE", MB_OK);
				}
				else if (Identifier == RACE_COPYHAIR)
				{
					GenericNode<TESHair>* Source = (GenericNode<TESHair>*)&SelectedRace->hairs;
					tList<TESHair>* Destination = &WorkingRace->hairs;

					Destination->RemoveAll();

					for (;Source && Source->data; Source = Source->next, Count++)
						thisCall(kLinkedListNode_NewNode, Destination, Source->data);

					sprintf_s(g_Buffer, sizeof(g_Buffer), "Copied %d hair forms from race '%s'.", Count, SelectedRace->editorData.editorID.m_data);
					MessageBox(Dialog, g_Buffer, "CSE", MB_OK);
				}
			}
		}
		return 1;
	}

	return 0;
}

void __declspec(naked) TESRaceCopyHairEyeDataMessageHandlerHook(void)
{
	static UInt32 kTESRaceCopyHairEyeDataMessageHandlerHookRetnAddr = 0x004E8FF2;
	static UInt32 kTESRaceCopyHairEyeDataMessageHandlerHookJumpAddr = 0x004E8E6D;
	__asm
	{
		movzx	eax, di
		add		eax, 0FFFFF78Ch
		cmp		eax, 0x24 
		ja		DEFAULT

		jmp		[kTESRaceCopyHairEyeDataMessageHandlerHookRetnAddr]
	DEFAULT:
		pushad
		push	ebp
		push	edi
		push	esi
		call	DoTESRaceCopyHairEyeDataMessageHandlerHook
		test	eax, eax
		jnz		HANDLED
		popad

		jmp		[kTESRaceCopyHairEyeDataMessageHandlerHookJumpAddr]
	HANDLED:
		popad

		xor		al, al
		mov		ecx, 0x004E9D5B
		jmp		ecx
	}
}

void __declspec(naked) TESObjectREFRDoCopyFromHook(void)
{
	static UInt32 kTESObjectREFRDoCopyFromHookRetnAddr = 0x00547668;
	__asm
	{
		pushad
		push	ebx
		mov		ecx, ebp
		call	kExtraDataList_CopyList
		popad

		jmp		[kTESObjectREFRDoCopyFromHookRetnAddr]
	}
}




