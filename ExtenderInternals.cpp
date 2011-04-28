#include "ExtenderInternals.h"
#include "Exports.h"
#include "[Common]\CLIWrapper.h"

std::string							g_INIPath;
std::string							g_AppPath;
bool								g_PluginPostLoad = false;
SME::INI::INIManager*				g_INIManager = new CSEINIManager();
SME::INI::INIEditGUI*				g_INIEditGUI = new SME::INI::INIEditGUI();

char								g_Buffer[0x200] = {0};
HINSTANCE							g_DLLInstance = NULL;

const HINSTANCE*					g_TESCS_Instance = (HINSTANCE*)0x00A0AF1C;
const DLGPROC						g_ScriptEditor_DlgProc = (DLGPROC)0x004FE760;
const DLGPROC						g_UseReport_DlgProc = (DLGPROC)0x00433FE0;
const DLGPROC						g_TESDialog_DlgProc = (DLGPROC)0x00447580;
const DLGPROC						g_TESDialogListView_DlgProc = (DLGPROC)0x00448820;
const DLGPROC						g_ChooseReference_DlgProc = (DLGPROC)0x0044D470;

HWND*								g_HWND_RenderWindow = (HWND*)0x00A0AF28;
HWND*								g_HWND_ObjectWindow = (HWND*)0x00A0AF44;
HWND*								g_HWND_CellView = (HWND*)0x00A0AF4C;
HWND*								g_HWND_CSParent = (HWND*)0x00A0AF20;
HWND*								g_HWND_AIPackagesDlg = (HWND*)0x00A0AFD8;
HWND*								g_HWND_ObjectWindow_FormList = (HWND*)0x00A0BAA0;
HWND*								g_HWND_ObjectWindow_Tree = (HWND*)0x00A0BAA4;
HWND*								g_HWND_MainToolbar = (HWND*)0x00A0AFD0;
HWND*								g_HWND_QuestWindow = (HWND*)0x00A0B034;

TBBUTTON*							g_MainToolbarButtonArray = (TBBUTTON*)0x009EAD50;
INISetting*							g_LocalMasterPath = (INISetting*)0x009ED710;
char**								g_TESActivePluginName = (char**)0x00A0AF00;
UInt8*								g_WorkingFileFlag = (UInt8*)0x00A0B628;
UInt8*								g_ActiveChangesFlag = (UInt8*)0x00A0B13C;
TESRenderSelection**				g_TESRenderSelectionPrimary = (TESRenderSelection**)0x00A0AF60;
HMENU*								g_RenderWindowPopup = (HMENU*)0x00A0BC40;
void*								g_ScriptCompilerUnkObj = (void*)0x00A0B128;
TESWaterForm**						g_DefaultWater = (TESWaterForm**)0x00A137CC;
TESObjectREFR**						g_PlayerRef = (TESObjectREFR**)0x00A0E088;
GenericNode<Archive>**				g_LoadedArchives = (GenericNode<Archive>**)0x00A0DD8C;
UInt8*								g_Flag_ObjectWindow_MenuState = (UInt8*)0x00A0AF40;
UInt8*								g_Flag_CellView_MenuState = (UInt8*)0x00A0AF48;
CRITICAL_SECTION*					g_ExtraListCS = (CRITICAL_SECTION*)0x00A0DA80;
TESSound**							g_FSTSnowSneak = (TESSound**)0x00A110F0;
BSTextureManager**					g_TextureManager = (BSTextureManager**)0x00A8E760;
NiDX9Renderer**						g_CSRenderer = (NiDX9Renderer**)0x00A0F87C;
UInt8*								g_Flag_RenderWindowUpdateViewPort = (UInt8*)0x00A0BC4D;
UInt32*								g_RenderWindowStateFlags = (UInt32*)0x00A0B058;
FileFinder**						g_FileFinder = (FileFinder**)0x00A0DE8C;
ResponseEditorData**				g_ResponseEditorData = (ResponseEditorData**)0x00A10E2C;
GameSettingCollection*				g_GMSTCollection = (GameSettingCollection*)0x00A10198;
void*								g_GMSTMap = (void*)0x00A102A4;

TESForm**							g_DoorMarker = (TESForm**)0x00A13470;
TESForm**							g_NorthMarker = (TESForm**)0x00A13484;
TESForm**							g_TravelMarker = (TESForm**)0x00A13480;
TESForm**							g_MapMarker = (TESForm**)0x00A13474;
TESForm**							g_HorseMarker = (TESForm**)0x00A134A0;

LPDIRECT3DTEXTURE9*					g_LODD3DTexture32x = (LPDIRECT3DTEXTURE9*)0x00A0AAC4;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture64x = (LPDIRECT3DTEXTURE9*)0x00A0AAC0;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture128x = (LPDIRECT3DTEXTURE9*)0x00A0AABC;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture512x = (LPDIRECT3DTEXTURE9*)0x00A0AAC8;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture1024x = (LPDIRECT3DTEXTURE9*)0x00A0AAD0;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture2048x = (LPDIRECT3DTEXTURE9*)0x00A0AACC;

BSRenderedTexture**					g_LODBSTexture32x = (BSRenderedTexture**)0x00A0AADC;
BSRenderedTexture**					g_LODBSTexture64x = (BSRenderedTexture**)0x00A0AAD8;
BSRenderedTexture**					g_LODBSTexture128x = (BSRenderedTexture**)0x00A0AAD4;
BSRenderedTexture**					g_LODBSTexture512x = (BSRenderedTexture**)0x00A0AAE0;
BSRenderedTexture**					g_LODBSTexture1024x = (BSRenderedTexture**)0x00A0AAE8;
BSRenderedTexture**					g_LODBSTexture2048x = (BSRenderedTexture**)0x00A0AAE4;

LPDIRECT3DTEXTURE9					g_LODD3DTexture256x = NULL;
BSRenderedTexture*					g_LODBSTexture256x = NULL;
LPDIRECT3DTEXTURE9					g_LODD3DTexture4096x = NULL;
BSRenderedTexture*					g_LODBSTexture4096x = NULL;
LPDIRECT3DTEXTURE9					g_LODD3DTexture8192x = NULL;
BSRenderedTexture*					g_LODBSTexture8192x = NULL;

const _WriteToStatusBar				WriteToStatusBar = (_WriteToStatusBar)0x00431310;
const _WritePositionToINI			WritePositionToINI = (_WritePositionToINI)0x00417510;
const _GetPositionFromINI			GetPositionFromINI = (_GetPositionFromINI)0x004176D0;
const _GetTESDialogTemplateForType	GetTESDialogTemplateForType = (_GetTESDialogTemplateForType)0x00442050;
const _GetComboBoxItemData			GetComboBoxItemData = (_GetComboBoxItemData)0x00403690;
const _SelectTESFileCommonDialog	SelectTESFileCommonDialog = (_SelectTESFileCommonDialog)0x00446D40;
const _TESDialog_SetCSWindowTitleModifiedFlag
									TESDialog_SetCSWindowTitleModifiedFlag = (_TESDialog_SetCSWindowTitleModifiedFlag)0x004306F0;
const _ChooseRefWrapper				ChooseRefWrapper = (_ChooseRefWrapper)0x0044D660;	// pass TESObjectREFR__PickComparator as arg3 and 0 as args 2 and 4
const _InitializeCSWindows			InitializeCSWindows = (_InitializeCSWindows)0x00430980;
const _DeInitializeCSWindows		DeInitializeCSWindows = (_DeInitializeCSWindows)0x00431220;
const _AddFormToObjectWindow		AddFormToObjectWindow = (_AddFormToObjectWindow)0x00422470;
const _InitializeDefaultPlayerSpell	InitializeDefaultPlayerSpell = (_InitializeDefaultPlayerSpell)0x0056FD90;
const _ConstructEffectSetting		ConstructEffectSetting = (_ConstructEffectSetting)0x0056AC40;
const _TESDialog_AddComboBoxItem	TESDialog_AddComboBoxItem = (_TESDialog_AddComboBoxItem)0x00403540;
const _BSPrintF						BSPrintF = (_BSPrintF)0x004053F0;
const _ShowCompilerError			ShowCompilerErrorEx = (_ShowCompilerError)0x004FFF40;
const _AutoSavePlugin				AutoSavePlugin = (_AutoSavePlugin)0x004307C0;
const _CreateArchive				CreateArchive = (_CreateArchive)0x004665C0;
const _TESDialog_GetListViewSelectedItemLParam
									TESDialog_GetListViewSelectedItemLParam = (_TESDialog_GetListViewSelectedItemLParam)0x00403C40;
const _TESForm_LookupByFormID		TESForm_LookupByFormID = (_TESForm_LookupByFormID)0x00495EF0;
const _TESDialog_GetDialogExtraParam
									TESDialog_GetDialogExtraParam = (_TESDialog_GetDialogExtraParam)0x004429D0;
const _TESDialog_ComboBoxPopulateWithRaces
									TESDialog_ComboBoxPopulateWithRaces = (_TESDialog_ComboBoxPopulateWithRaces)0x00445240;
const _TESDialog_ComboBoxPopulateWithForms
									TESDialog_ComboBoxPopulateWithForms = (_TESDialog_ComboBoxPopulateWithForms)0x004456F0;
const _TESDialog_GetSelectedItemData
									TESDialog_GetSelectedItemData = (_TESDialog_GetSelectedItemData)0x00403690;
const _TESDialog_GetDialogExtraLocalCopy
									TESDialog_GetDialogExtraLocalCopy = (_TESDialog_GetDialogExtraLocalCopy)0x004429B0;
const _DataHandler_PlaceTESBoundObjectReference
									DataHandler_PlaceTESBoundObjectReference = (_DataHandler_PlaceTESBoundObjectReference)0x0047C610;

const void*							RTTI_TESCellUseList = (void*)0x009EB2E4;

const UInt32						kVTBL_TESObjectREFR = 0x00958824;
const UInt32						kVTBL_TESForm = 0x0094688C;
const UInt32						kVTBL_TESTopicInfo = 0x0094820C;
const UInt32						kVTBL_TESQuest = 0x00945D7C;
const UInt32						kVTBL_TESNPC = 0x0094561C;
const UInt32						kVTBL_TESCreature = 0x00944334;
const UInt32						kVTBL_TESFurniture = 0x00950E94;
const UInt32						kVTBL_TESObjectACTI = 0x009537DC;
const UInt32						kVTBL_TESObjectMISC = 0x00955224;
const UInt32						kVTBL_TESObjectWEAP = 0x00955C8C;
const UInt32						kVTBL_TESObjectCONT = 0x00954B44;
const UInt32						kVTBL_TESObjectCLOT = 0x0095482C;
const UInt32						kVTBL_SpellItem = 0x0095E504;
const UInt32						kVTBL_Script = 0x0094944C;
const UInt32						kVTBL_MessageHandler = 0x00940760;
const UInt32						kVTBL_FileFinder = 0x009389BC;

const UInt32						kTESNPC_Ctor = 0x004D8FF0;
const UInt32						kTESCreature_Ctor = 0x004CE820;
const UInt32						kTESFurniture_Ctor = 0x0050C830;
const UInt32						kTESObjectACTI_Ctor = 0x00515530;
const UInt32						kTESObjectMISC_Ctor = 0x0051ABA0;
const UInt32						kTESObjectWEAP_Ctor = 0x0051DAB0;
const UInt32						kTESObjectCONT_Ctor = 0x00518F60;
const UInt32						kTESObjectREFR_Ctor = 0x00541870;
const UInt32						kTESObjectCLOT_Ctor = 0x00518350;
const UInt32						kTESQuest_Ctor = 0x004E0500;
const UInt32						kScript_Ctor = 0x004FCA50;
const UInt32						kTESRenderSelection_Ctor = 0x00511A20;
const UInt32						kGameSetting_Ctor = 0x004FA040;

const UInt32						kTESChildCell_LoadCell = 0x00430F40;
const UInt32						kTESCellUseList_GetUseListRefHead = 0x006E5850;
const UInt32						kTESObjectCELL_GetParentWorldSpace = 0x00532E50;
const UInt32						kScript_SaveResultScript = 0x005034E0;
const UInt32						kScript_SaveScript = 0x00503450;
const UInt32						kLinkedListNode_NewNode = 0x004E3900;
const UInt32						kDataHandler_AddBoundObject = 0x005135F0;
const UInt32						kTESForm_SetFormID = 0x00497E50;
const UInt32						kTESForm_SetEditorID = 0x00497670;
const UInt32						kTESObjectREFR_SetBaseForm = 0x005415A0;
const UInt32						kTESObjectREFR_SetFlagPersistent = 0x0053F0D0;
const UInt32						kExtraDataList_InitItem = 0x0045D740;
const UInt32						kScript_SetText = 0x004FC6C0;
const UInt32						kDataHandler_SortScripts = 0x0047BA30;
const UInt32						kTESScriptableForm_SetScript = 0x004A1830;
const UInt32						kBSString_Set = 0x004051E0;
const UInt32						kExtraDataList_CopyListForReference = 0x004603D0;
const UInt32						kExtraDataList_CopyList = 0x00460380;
const UInt32						kGMSTMap_Add = 0x0044F680;
const UInt32						kBSTextureManager_CreateBSRenderedTexture = 0x00773080;
const UInt32						kTESForm_GetOverrideFile = 0x00495FE0;
const UInt32						kTESForm_AddReference = 0x00496430;
const UInt32						kTESForm_GetFormReferenceList = 0x00496380;
const UInt32						kTESForm_CleanupFormReferenceList = 0x00496400;
const UInt32						kTESQuest_SetStartEnabled = 0x004DD7E0;
const UInt32						kTESQuest_SetAllowedRepeatedStages = 0x004DD7C0;
const UInt32						kTESObjectCELL_GetIsInterior = 0x00532240;
const UInt32						kTESBipedModelForm_GetIsPlayable = 0x00490290;
const UInt32						kTESRenderSelection_ClearSelection = 0x00511C20;
const UInt32						kTESRenderSelection_AddFormToSelection = 0x00512730;
const UInt32						kTESRenderSelection_Free = 0x00511A50;
const UInt32						kTESForm_SaveFormRecord = 0x00494950;
const UInt32						kTESFile_GetIsESM = 0x00485B00;
const UInt32						kTESFile_Dtor = 0x00487E60;
const UInt32						kDataHandler_PopulateModList = 0x0047E4C0;
const UInt32						kTESRenderSelection_RemoveFormFromSelection = 0x00512830;
const UInt32						kLinkedListNode_RemoveNode = 0x00452AE0;
const UInt32						kLinkedListNode_GetIsDangling = 0x0048E0E0;
const UInt32						kLinkedListNode_Cleanup = 0x00405DC0;
const UInt32						kLinkedListNode_GetData = 0x004FC950;
const UInt32						kTESForm_SetTemporary = 0x004972A0;

const UInt32						kBaseExtraList_GetExtraDataByType = 0x0045B1B0;
const UInt32						kBaseExtraList_ModExtraEnableStateParent = 0x0045CAA0;
const UInt32						kBaseExtraList_ModExtraOwnership = 0x0045E060;
const UInt32						kBaseExtraList_ModExtraGlobal = 0x0045E120;
const UInt32						kBaseExtraList_ModExtraRank = 0x0045E1E0;
const UInt32						kBaseExtraList_ModExtraCount = 0x0045E2A0;

const UInt32						kTESObjectREFR_ModExtraHealth = 0x0053F4E0;
const UInt32						kTESObjectREFR_ModExtraCharge = 0x0053F3C0;
const UInt32						kTESObjectREFR_ModExtraTimeLeft = 0x0053F620;
const UInt32						kTESObjectREFR_ModExtraSoul = 0x0053F710;
const UInt32						kTESObjectREFR_SetExtraEnableStateParent_OppositeState = 0x0053FA80;
const UInt32						kTESObjectREFR_GetExtraRef3DData = 0x00542950;
const UInt32						kTESObjectREFR_RemoveExtraTeleport = 0x0053F7A0;

TES* TES::GetSingleton()
{
	return *g_TES;
}

void CSEINIManager::Initialize()
{
	DebugPrint("INI Path: %s", INIFile.c_str());
	std::fstream INIStream(INIFile.c_str(), std::fstream::in);
	bool CreateINI = false;

	if (INIStream.fail())
	{
		DebugPrint("INI File not found; Creating one...");
		CreateINI = true;
	}

	INIStream.close();
	INIStream.clear();		// only initializing non script editor keys as those are taken care of by its code

	RegisterSetting(new SME::INI::INISetting(this, "Top", "Console::General", "150", "Client Rect Top"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "Left", "Console::General", "150", "Client Rect Left"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "Right", "Console::General", "500", "Client Rect Right"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "Bottom", "Console::General", "350", "Client Rect Bottom"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "LogCSWarnings", "Console::General", "1", "Log CS Warnings to the Console"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "LogAssertions", "Console::General", "1", "Log CS Assertions to the Console"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "HideOnStartup", "Console::General", "0", "Hide the console on CS startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "ConsoleUpdatePeriod", "Console::General", "2000", "Duration, in milliseconds, between console window updates"), (CreateINI == false));

	RegisterSetting(new SME::INI::INISetting(this, "LoadPluginOnStartup", "Extender::General", "0", "Loads a plugin on CS startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "StartupPluginName", "Extender::General", "", "Name of the plugin, with extension, that is to be loaded on startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "OpenScriptWindowOnStartup", "Extender::General", "0", "Open an empty script editor window on startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "StartupScriptEditorID", "Extender::General", "", "EditorID of the script to be loaded on startup, should a script editor also be opened. An empty string results in a blank workspace"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "ShowNumericEditorIDWarning", "Extender::General", "1", "Displays a warning when editorIDs start with an integer"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "SetWorkspaceOnStartup", "Extender::General", "0", "Sets the working directory to a custom path"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "DefaultWorkspacePath", "Extender::General", "", "Path of the custom workspace directory"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "SaveLoadedESPsAsMasters", "Extender::General", "1", "Save loaded plugin files as the active plugin's master"), (CreateINI == false));

	RegisterSetting(new SME::INI::INISetting(this, "UpdatePeriod", "Extender::Renderer", "8", "Duration, in milliseconds, between render window updates"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "DisplaySelectionStats", "Extender::Renderer", "1", "Display info on the render window selection"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "UpdateViewPortAsync", "Extender::Renderer", "0", "Allow the render window to be updated in the background"), (CreateINI == false));

	if (CreateINI)		SaveSettingsToINI();
	else				ReadSettingsFromINI();
}