#include "ExtenderInternals.h"
#include "Exports.h"
#include "MiscHooks.h"
#include "[Common]\CLIWrapper.h"

std::string							g_INIPath;
std::string							g_AppPath;
bool								g_PluginPostLoad = false;
SME::INI::INIManager*				g_INIManager = new CSEINIManager();
SME::INI::INIEditGUI*				g_INIEditGUI = new SME::INI::INIEditGUI();

EditorAllocator*					EditorAllocator::Singleton = NULL;
char								g_Buffer[0x200] = {0};
_DefaultGMSTMap						g_DefaultGMSTMap;
HINSTANCE							g_DLLInstance = NULL;
RenderTimeManager					g_RenderTimeManager;
RenderWindowTextPainter*			RenderWindowTextPainter::Singleton = NULL;
RenderSelectionGroupManager			g_RenderSelectionGroupManager;


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
GameSettingCollection*				g_GMSTCollection = (GameSettingCollection*)0x00A10198;
void*								g_GMSTMap = (void*)0x00A102A4;
GenericNode<Archive>**				g_LoadedArchives = (GenericNode<Archive>**)0x00A0DD8C;
ResponseEditorData**				g_ResponseEditorData = (ResponseEditorData**)0x00A10E2C;
UInt8*								g_Flag_ObjectWindow_MenuState = (UInt8*)0x00A0AF40;
UInt8*								g_Flag_CellView_MenuState = (UInt8*)0x00A0AF48;
CRITICAL_SECTION*					g_ExtraListCS = (CRITICAL_SECTION*)0x00A0DA80;
TESSound**							g_FSTSnowSneak = (TESSound**)0x00A110F0;
BSTextureManager**					g_TextureManager = (BSTextureManager**)0x00A8E760;
NiDX9Renderer**						g_CSRenderer = (NiDX9Renderer**)0x00A0F87C;
UInt8*								g_Flag_RenderWindowUpdateViewPort = (UInt8*)0x00A0BC4D;

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

const _WriteToStatusBar				WriteToStatusBar = (_WriteToStatusBar)0x00431310;
const _WritePositionToINI			WritePositionToINI = (_WritePositionToINI)0x00417510;
const _GetPositionFromINI			GetPositionFromINI = (_GetPositionFromINI)0x004176D0;
const _GetTESDialogTemplateForType	GetTESDialogTemplateForType = (_GetTESDialogTemplateForType)0x00442050;
const _GetComboBoxItemData			GetComboBoxItemData = (_GetComboBoxItemData)0x00403690;
const _SelectTESFileCommonDialog	SelectTESFileCommonDialog = (_SelectTESFileCommonDialog)0x00446D40;
const _sub_4306F0					sub_4306F0 = (_sub_4306F0)0x004306F0;
const _ChooseRefWrapper				ChooseRefWrapper = (_ChooseRefWrapper)0x0044D660;	// pass 0x00545B10 as arg3 and 0 as args 2 and 4
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

const UInt32						kTESChildCell_LoadCell = 0x00430F40; 
const UInt32						kTESForm_GetObjectUseList = 0x00496380;		// Node<TESForm> GetObjectUseRefHead(UInt32 unk01 = 0);
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
const UInt32						kTESQuest_SetStartEnabled = 0x004DD7E0;
const UInt32						kTESQuest_SetAllowedRepeatedStages = 0x004DD7C0;
const UInt32						kTESObjectCELL_GetIsInterior = 0x00532240;
const UInt32						kTESBipedModelForm_GetIsPlayable = 0x00490290;
const UInt32						kTESRenderSelection_ClearSelection = 0x00511C20;
const UInt32						kTESRenderSelection_AddFormToSelection = 0x00512730;
const UInt32						kTESRenderSelection_Free = 0x00511A50;

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

const char*							g_FormTypeIdentifier[] =			// uses TESForm::typeID as its index
									{
											"None",
											"TES4",
											"Group",
											"GMST",
											"Global",
											"Class",
											"Faction",
											"Hair",
											"Eyes",
											"Race",
											"Sound",
											"Skill",
											"Effect",
											"Script",
											"LandTexture",
											"Enchantment",
											"Spell",
											"BirthSign",
											"Activator",
											"Apparatus",
											"Armor",
											"Book",
											"Clothing",
											"Container",
											"Door",
											"Ingredient",
											"Light",
											"MiscItem",
											"Static",
											"Grass",
											"Tree",
											"Flora",
											"Furniture",
											"Weapon",
											"Ammo",
											"NPC",
											"Creature",
											"LeveledCreature",
											"SoulGem",
											"Key",
											"AlchemyItem",
											"SubSpace",
											"SigilStone",
											"LeveledItem",
											"SNDG",
											"Weather",
											"Climate",
											"Region",
											"Cell",
											"Reference",
											"Reference",			// ACHR
											"Reference",			// ACRE
											"PathGrid",
											"World Space",
											"Land",
											"TLOD",
											"Road",
											"Dialog",
											"Dialog Info",
											"Quest",
											"Idle",
											"AI Package",
											"CombatStyle",
											"LoadScreen",
											"LeveledSpell",
											"AnimObject",
											"WaterType",
											"EffectShader",
											"TOFT"
										};


TES* TES::GetSingleton()
{
	return *g_TES;
}

EditorAllocator* EditorAllocator::GetSingleton(void)
{
	if (!Singleton)	{
		EditorAllocator::Singleton = new EditorAllocator;
		Singleton->NextIndex = 1;
	}
	return Singleton;
}

UInt32 EditorAllocator::TrackNewEditor(HWND EditorDialog)
{
	UInt32 Result = NextIndex++;
	AllocationMap.insert(std::make_pair<HWND, SEAlloc*>(EditorDialog, new SEAlloc(GetDlgItem(EditorDialog, 1166), GetDlgItem(EditorDialog, 2259), Result)));
	if (NextIndex == 2147483648)
		MessageBox(*g_HWND_CSParent, "Holy crap, mate! I have no idea how you managed to create 2147483648 editor workspaces xO I'd suggest that you pack up some essentials and head to the Andes as the next allocation is certain to warp the space-time continuum in unimaginable ways.\n\nDamn you...", "The Developer Speaks", MB_HELP|MB_ICONSTOP);

	return Result;
}

void EditorAllocator::DeleteTrackedEditor(UInt32 TrackedEditorIndex)
{					
	for (AlMap::iterator Itr = AllocationMap.begin(); Itr != AllocationMap.end(); Itr++) {		
		if (Itr->second->Index == TrackedEditorIndex) {
			delete Itr->second;
			AllocationMap.erase(Itr);
			break;
		}
	}
}

void EditorAllocator::DeleteAllTrackedEditors(void)
{
	for (AlMap::iterator Itr = AllocationMap.begin(); Itr != AllocationMap.end(); Itr++) {	
		delete Itr->second;
	}
	AllocationMap.clear();
}

void EditorAllocator::DestroyVanillaDialogs(void)
{
	for (AlMap::iterator Itr = AllocationMap.begin(); Itr != AllocationMap.end(); Itr++) 
		DestroyWindow(Itr->first);
}

HWND EditorAllocator::GetTrackedREC(HWND TrackedEditorDialog)
{
	AlMap::const_iterator Itr = AllocationMap.find(TrackedEditorDialog);
	if (Itr == AllocationMap.end())
		return NULL;
	else 	
		return Itr->second->RichEditControl;
}

HWND EditorAllocator::GetTrackedLBC(HWND TrackedEditorDialog)
{
	AlMap::const_iterator Itr = AllocationMap.find(TrackedEditorDialog);
	if (Itr == AllocationMap.end())
		return NULL;
	else 	
		return Itr->second->ListBoxControl;
}

UInt32 EditorAllocator::GetTrackedIndex(HWND TrackedEditorDialog)
{
	UInt32 Result = 0;
	AlMap::const_iterator Itr = AllocationMap.find(TrackedEditorDialog);
	if (Itr == AllocationMap.end())
		return 0;
	else 	
		return Itr->second->Index;
}

HWND EditorAllocator::GetTrackedDialog(UInt32 TrackedEditorIndex)
{
	HWND Result= NULL;
	for (AlMap::const_iterator Itr = AllocationMap.begin(); Itr != AllocationMap.end(); Itr++) {
		Result = Itr->first;
		if (Itr->second->Index == TrackedEditorIndex)
			break;
	}
	return Result;
}

void CSEINIManager::Initialize()
{
	DebugPrint("INI Path: %s", INIFile.c_str());
	std::fstream INIStream(INIFile.c_str(), std::fstream::in);
	bool CreateINI = false;

	if (INIStream.fail()) {
		_MESSAGE("INI File not found; Creating one...");
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

	RegisterSetting(new SME::INI::INISetting(this, "LoadPluginOnStartup", "Extender::General", "1", "Loads a plugin on CS startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "StartupPluginName", "Extender::General", "Plugin.esp", "Name of the plugin, with extension, that is to be loaded on startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "OpenScriptWindowOnStartup", "Extender::General", "0", "Open an empty script editor window on startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "StartupScriptEditorID", "Extender::General", "", "EditorID of the script to be loaded on startup, should a script editor also be opened. An empty string results in a blank workspace"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "ShowNumericEditorIDWarning", "Extender::General", "1", "Displays a warning when editorIDs start with an integer"), (CreateINI == false));
	
	RegisterSetting(new SME::INI::INISetting(this, "UpdatePeriod", "Extender::Renderer", "8", "Duration, in milliseconds, between render window updates"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "DisplaySelectionStats", "Extender::Renderer", "1", "Display info on the render window selection"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "UpdateViewPortAsync", "Extender::Renderer", "0", "Allow the render window to be updated in the background"), (CreateINI == false));

	if (CreateINI)		SaveSettingsToINI();
	else				ReadSettingsFromINI();
}

TESDialogInitParam::TESDialogInitParam(const char* EditorID)
{
	Form = GetFormByID(EditorID);
	TypeID = Form->typeID;
}

UInt32 GetDialogTemplate(const char* FormType)
{
	if (!_stricmp(FormType, "Activator") ||
		!_stricmp(FormType, "Apparatus") ||
		!_stricmp(FormType, "Armor") ||
		!_stricmp(FormType, "Book") ||
		!_stricmp(FormType, "Clothing") ||
		!_stricmp(FormType, "Container") ||
		!_stricmp(FormType, "Door") ||
		!_stricmp(FormType, "Ingredient") ||
		!_stricmp(FormType, "Light") ||
		!_stricmp(FormType, "MiscItem") ||
		!_stricmp(FormType, "SoulGem") ||
		!_stricmp(FormType, "Static") ||
		!_stricmp(FormType, "Grass") ||
		!_stricmp(FormType, "Tree") ||
		!_stricmp(FormType, "Flora") ||
		!_stricmp(FormType, "Furniture") ||
		!_stricmp(FormType, "Ammo") ||
		!_stricmp(FormType, "Weapon") ||
		!_stricmp(FormType, "NPC") ||
		!_stricmp(FormType, "Creature") ||
		!_stricmp(FormType, "LeveledCreature") ||
		!_stricmp(FormType, "Spell") ||
		!_stricmp(FormType, "Enchantment") ||
		!_stricmp(FormType, "Potion") ||
		!_stricmp(FormType, "Leveled Item") ||
		!_stricmp(FormType, "Sound") ||
		!_stricmp(FormType, "LandTexture") ||
		!_stricmp(FormType, "CombatStyle") ||
		!_stricmp(FormType, "LoadScreen") ||
		!_stricmp(FormType, "WaterType") ||
		!_stricmp(FormType, "LeveledSpell") ||
		!_stricmp(FormType, "AnimObject") ||
		!_stricmp(FormType, "Subspace") ||
		!_stricmp(FormType, "EffectShader") ||
		!_stricmp(FormType, "SigilStone"))
			return 1;									// TESDialog
	else if (!_stricmp(FormType, "Script"))
			return 9;
	else if (!_stricmp(FormType, "Reference"))				
			return 10;									// Special Handlers
	else if (!_stricmp(FormType, "Hair") ||				
		!_stricmp(FormType, "Eyes") ||					
		!_stricmp(FormType, "Race") ||
		!_stricmp(FormType, "Class") ||
		!_stricmp(FormType, "Birthsign") ||				
		!_stricmp(FormType, "Climate") ||
		!_stricmp(FormType, "World Space"))
			return 2;									// TESDialog ListView
	else
			return 0;
}

UInt32 GetDialogTemplate(UInt8 FormTypeID)
{
	const char* FormType = g_FormTypeIdentifier[FormTypeID];

	return GetDialogTemplate(FormType);
}

void SpawnCustomScriptEditor(const char* ScriptEditorID)
{
	g_EditorAuxScript =  CS_CAST(GetFormByID(ScriptEditorID), TESForm, Script);;
	tagRECT ScriptEditorLoc;
	GetPositionFromINI("Script Edit", &ScriptEditorLoc);
	CLIWrapper::ScriptEditor::AllocateNewEditor(ScriptEditorLoc.left, ScriptEditorLoc.top, ScriptEditorLoc.right, ScriptEditorLoc.bottom);
	g_EditorAuxScript = NULL;
}

void LoadFormIntoView(const char* EditorID, const char* FormType)
{
	UInt32 Type = GetDialogTemplate(FormType);
	TESDialogInitParam InitData(EditorID);

	switch (Type)
	{
	case 9:					
		if (GetFormByID(EditorID))
			SpawnCustomScriptEditor(EditorID);
		break;
	case 10:
		RemoteLoadRef(EditorID);
		break;
	case 1:
	case 2:
		CreateDialogParamA(*g_TESCS_Instance, 
							(LPCSTR)GetTESDialogTemplateForType(InitData.TypeID), 
							*g_HWND_CSParent, 
							((Type == 1) ? g_TESDialog_DlgProc : g_TESDialogListView_DlgProc), 
							(LPARAM)&InitData);
		break;
	}
}

void LoadFormIntoView(const char* EditorID, UInt8 FormType)
{
	LoadFormIntoView(EditorID, g_FormTypeIdentifier[FormType]);
}

void RemoteLoadRef(const char* EditorID)
{
	TESObjectREFR* Reference = CS_CAST(GetFormByID(EditorID), TESForm, TESObjectREFR);
	TESChildCell* Cell = (TESChildCell*)thisVirtualCall(kVTBL_TESObjectREFR, 0x1A0, Reference);
	thisCall(kTESChildCell_LoadCell, Cell, Cell, Reference);
}

TESObjectREFR* ChooseReferenceDlg(HWND Parent)
{
	return ChooseRefWrapper(Parent, 0, 0x00545B10, 0);
}

void LoadStartupPlugin()
{
	kAutoLoadActivePluginOnStartup.WriteJump();

	const char* PluginName = g_INIManager->GET_INI_STR("StartupPluginName");
	const ModEntry* TESFile = (*g_dataHandler)->LookupModByName(PluginName);
	if (TESFile)
	{
		DebugPrint("Loading plugin '%s' on startup...", PluginName);

		if (_stricmp(PluginName, "Oblivion.esm"))
			ToggleFlag(&TESFile->data->flags, ModEntry::Data::kFlag_Active, true);
		ToggleFlag(&TESFile->data->flags, ModEntry::Data::kFlag_Loaded, true);
		SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CD1, 0);
	} 
	else if (strlen(PluginName) >= 1)
	{
		DebugPrint("Couldn't load plugin '%s' on startup - It doesn't exist!", PluginName);
	}

	kAutoLoadActivePluginOnStartup.WriteBuffer();
}

void InitializeDefaultGMSTMap()
{
	void* Unk01 = (void*)thisCall(0x0051F920, (void*)g_GMSTMap);
	while (Unk01)
	{
		const char*	 Name = NULL;
		GMSTData*	 Data;

		thisCall(0x005E0F90, (void*)g_GMSTMap, &Unk01, &Name, &Data);
		if (Name)
		{
			g_DefaultGMSTMap.insert(std::make_pair<const char*, GMSTData*>(Name, Data));
		}
	}
}

void LoadedMasterArchives()
{
	if (*g_LoadedArchives == 0)		return;

	for (IDirectoryIterator Itr((std::string(g_AppPath + "Data\\")).c_str(), "*.bsa"); !Itr.Done(); Itr.Next())
	{
		std::string FileName(Itr.Get()->cFileName);
		FileName = FileName.substr(FileName.find_last_of("\\") + 1);
		
		bool IsLoaded = false;
		for (GenericNode<Archive>* Itr = (*g_LoadedArchives); Itr; Itr = Itr->next)
		{
			if (Itr->data)
			{
				std::string LoadedFileName(Itr->data->bsfile.m_path);
				LoadedFileName = LoadedFileName.substr(LoadedFileName.find_last_of("\\") + 1);

				if (!_stricmp(LoadedFileName.c_str(), FileName.c_str()))
				{	
					IsLoaded = true;
					break;
				}	
			}
		}

		if (IsLoaded == false)
		{
			CreateArchive(FileName.c_str(), 0, 0);
			DebugPrint("BSA Archive %s loaded", FileName.c_str());
		}
	}
}

void UnloadLoadedCell()
{
	UInt8 ObjWndState = *g_Flag_ObjectWindow_MenuState, CellWndState = *g_Flag_CellView_MenuState;

	*g_Flag_ObjectWindow_MenuState = 0;
	*g_Flag_CellView_MenuState = 0;

	SendMessage(*g_HWND_RenderWindow, 0x419, 6, 1);
	SendMessage(*g_HWND_RenderWindow, 0x419, 5, 0);
	InvalidateRect(*g_HWND_RenderWindow, 0, 1);

	DeInitializeCSWindows();
	InitializeCSWindows();

	*g_Flag_ObjectWindow_MenuState = ObjWndState;
	*g_Flag_CellView_MenuState = CellWndState;
}


void __stdcall FormEnumerationWrapper::ReinitializeFormLists()
{
	DeInitializeCSWindows();	

	SendMessage(*g_HWND_CellView, 0x40E, 1, 1);			// for worldspaces
	SendMessage(*g_HWND_AIPackagesDlg, 0x41A, 0, 0);	// for AI packages

	InitializeCSWindows();
	InvalidateRect(*g_HWND_ObjectWindow_FormList, NULL, TRUE);
	SendMessage(*g_HWND_ObjectWindow_FormList, 0x41A, 0, 0);
}

bool FormEnumerationWrapper::GetUnmodifiedFormHiddenState()	// returns true when hidden
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
	UInt32 State = GetMenuState(ViewMenu, MAIN_VIEW_MODIFIEDRECORDS, MF_BYCOMMAND);

	return (State & MF_CHECKED);
}

bool FormEnumerationWrapper::GetDeletedFormHiddenState()
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
	UInt32 State = GetMenuState(ViewMenu, MAIN_VIEW_DELETEDRECORDS, MF_BYCOMMAND);

	return (State & MF_CHECKED);
}

bool __stdcall FormEnumerationWrapper::GetShouldEnumerateForm(TESForm* Form)
{
	if (GetUnmodifiedFormHiddenState() && (Form->flags & TESForm::kFormFlags_FromActiveFile) == 0)
		return false;		// skip addition
	else if (GetDeletedFormHiddenState() && (Form->flags & TESForm::kFormFlags_Deleted))
		return false;
	else
		return true;
}

bool __stdcall FormEnumerationWrapper::PerformListViewPrologCheck(UInt32 CallAddress)
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
	case 0x00445E12:	
	case 0x00445D81:
	case 0x004F00C3:
		return 1;
	default:
		return 0;
	}
}


void FormEnumerationWrapper::ToggleUnmodifiedFormVisibility()
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
	if (GetUnmodifiedFormHiddenState())
		CheckMenuItem(ViewMenu, MAIN_VIEW_MODIFIEDRECORDS, MF_UNCHECKED);
	else
		CheckMenuItem(ViewMenu, MAIN_VIEW_MODIFIEDRECORDS, MF_CHECKED);		

	ReinitializeFormLists();
}
void FormEnumerationWrapper::ToggleDeletedFormVisibility()
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
	if (GetDeletedFormHiddenState())
		CheckMenuItem(ViewMenu, MAIN_VIEW_DELETEDRECORDS, MF_UNCHECKED);
	else
		CheckMenuItem(ViewMenu, MAIN_VIEW_DELETEDRECORDS, MF_CHECKED);		

	ReinitializeFormLists();
}

void RenderTimeManager::Update(void)
{
	QueryPerformanceCounter(&FrameBuffer);
	TimePassed = ((LONGLONG)((FrameBuffer.QuadPart - ReferenceFrame.QuadPart ) * 1000 / TimerFrequency.QuadPart)) / 1000.0;
	ReferenceFrame = FrameBuffer;
}


void __stdcall FormEnumerationWrapper::ResetFormVisibility(void)
{
	if (GetUnmodifiedFormHiddenState())
		ToggleUnmodifiedFormVisibility();
	if (GetDeletedFormHiddenState())
		ToggleDeletedFormVisibility();
}


void RenderWindowTextPainter::StaticRenderChannel::Render()
{
	if (Valid == false)
		return;
	else if (TextToRender.length() < 1)
		return;

	Font->DrawTextA(NULL, TextToRender.c_str(), -1, &DrawArea, 0, Color);
}

void RenderWindowTextPainter::StaticRenderChannel::Queue(const char* Text)
{
	if (Valid == false)
		return;

	if (Text)
		TextToRender = Text;
	else
		TextToRender.clear();
}

void RenderWindowTextPainter::DynamicRenderChannel::Render()
{
	if (Valid == false)
		return;
	else if (DrawQueue.size() < 1)
		return;

	QueueTask* CurrentTask = DrawQueue.front();

	if (CurrentTask->RemainingTime > 0)
	{
		Font->DrawTextA(NULL, CurrentTask->Text.c_str(), -1, &DrawArea, 0, Color);
		CurrentTask->RemainingTime -= g_RenderTimeManager.GetTimePassedSinceLastFrame();
	}
	else
	{
		delete CurrentTask;
		DrawQueue.pop();
	}
}

void RenderWindowTextPainter::DynamicRenderChannel::Queue(const char* Text, long double SecondsToDisplay)
{
	if (Valid == false)
		return;

	if (GetQueueSize() == 0)
		g_RenderTimeManager.Update();

	if (Text && SecondsToDisplay > 0)
		DrawQueue.push(new QueueTask(Text, SecondsToDisplay));
}

void RenderWindowTextPainter::DynamicRenderChannel::Release()
{
	if (Valid == false)
		return;

	while (DrawQueue.size())
	{
		QueueTask* CurrentTask = DrawQueue.front();
		delete CurrentTask;
		DrawQueue.pop();
	}

	RenderChannelBase::Release();
}

RenderWindowTextPainter::RenderWindowTextPainter()
{
	RenderChannel1 = NULL;
	RenderChannel2 = NULL;
	Valid = false;
}

RenderWindowTextPainter* RenderWindowTextPainter::GetSingleton(void)
{
	if (Singleton == NULL)
		Singleton = new RenderWindowTextPainter();
	return Singleton;
}

bool RenderWindowTextPainter::Initialize()
{
	if (Valid)
		return true;

	CONSOLE->Indent();

	RECT DrawRect;
	DrawRect.left = 3;
	DrawRect.top = 3;
	DrawRect.right = 1280;
	DrawRect.bottom = 600;
	RenderChannel1 = new StaticRenderChannel(12, 7, FW_THIN, "Lucida Console", D3DCOLOR_ARGB(220, 189, 237, 99), &DrawRect);

	DrawRect.top += 350;
	RenderChannel2 = new DynamicRenderChannel(12, 7, FW_THIN, "Lucida Console", D3DCOLOR_ARGB(255, 190, 35, 47), &DrawRect);

	if (RenderChannel1->GetIsValid() == false || RenderChannel2->GetIsValid() == false)
		Valid = false;
	else
		Valid = true;

	CONSOLE->Exdent();
	return Valid;
}

void RenderWindowTextPainter::Release()
{
	RenderChannel1->Release();
	RenderChannel2->Release();

	delete RenderChannel1;
	delete RenderChannel2;

	Valid = false;

//	DebugPrint("RenderWindowTextPainter released");
}

void RenderWindowTextPainter::Render()
{
	if (Valid == false)
		return;

	RenderChannel1->Render();
	RenderChannel2->Render();
}

void RenderWindowTextPainter::QueueDrawTask(UInt8 Channel, const char* Text, long double SecondsToDisplay)
{
	if (Valid == false)
		return;

	switch (Channel)
	{
	case kRenderChannel_1:
		RenderChannel1->Queue(Text);
		break;
	case kRenderChannel_2:
		RenderChannel2->Queue(Text, SecondsToDisplay);
		break;
	}
}

UInt32 RenderWindowTextPainter::GetRenderChannelQueueSize(UInt8 Channel)
{
	if (Valid == false)
		return 0;

	switch (Channel)
	{
	case kRenderChannel_1:
		return RenderChannel1->GetQueueSize();
	case kRenderChannel_2:
		return RenderChannel2->GetQueueSize();
	}

	return 0;
}


std::vector<TESRenderSelection*>* RenderSelectionGroupManager::GetCellExists(TESObjectCELL* Cell)
{
	_RenderSelectionGroupMap::iterator Match = SelectionGroupMap.find(Cell);
	if (Match != SelectionGroupMap.end())
		return &Match->second;
	else
		return NULL;
}

TESRenderSelection* RenderSelectionGroupManager::GetRefSelectionGroup(TESObjectREFR* Ref, TESObjectCELL* Cell)
{
	TESRenderSelection* Result = NULL;

	std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
	if (SelectionList)
	{
		for (std::vector<TESRenderSelection*>::iterator Itr = SelectionList->begin(); Itr != SelectionList->end(); Itr++)
		{
			for (TESRenderSelection::SelectedObjectsEntry* ItrEx = (*Itr)->RenderSelection; ItrEx && ItrEx->Data; ItrEx = ItrEx->Next)
			{
				if (ItrEx->Data == Ref)
				{
					Result = *Itr;
					break;
				}
			}
		}
	}

	return Result;
}

void RenderSelectionGroupManager::Clear()
{
	for (_RenderSelectionGroupMap::iterator Itr = SelectionGroupMap.begin(); Itr != SelectionGroupMap.end(); Itr++)
	{
		for (std::vector<TESRenderSelection*>::iterator ItrEx = Itr->second.begin(); ItrEx != Itr->second.end(); ItrEx++)
		{
			thisCall(kTESRenderSelection_ClearSelection, *ItrEx, 0);
			thisCall(kTESRenderSelection_Free, *ItrEx);
			FormHeap_Free(*ItrEx);
		}

		Itr->second.clear();
	}
	SelectionGroupMap.clear();
}

TESRenderSelection* RenderSelectionGroupManager::AllocateNewSelection(TESRenderSelection* Selection)
{
	TESRenderSelection* Group = (TESRenderSelection*)FormHeap_Allocate(0x18);
	thisCall(kTESRenderSelection_Ctor, Group);

	for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->RenderSelection; Itr && Itr->Data; Itr = Itr->Next)
		thisCall(kTESRenderSelection_AddFormToSelection, Group, Itr->Data, 0);

//	DebugPrint("Allocated Selection %08X", (UInt32)Group);

	return Group;
}

TESObjectREFR* RenderSelectionGroupManager::GetRefAtSelectionIndex(TESRenderSelection* Selection, UInt32 Index)
{
	UInt32 Count = 0;
	for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->RenderSelection; Itr && Itr->Data; Itr = Itr->Next, Count++)
	{
		if (Count == Index)
			return Itr->Data;
	}
	return NULL;
}

TESRenderSelection* RenderSelectionGroupManager::GetTrackedSelection(TESObjectCELL* Cell, TESRenderSelection* Selection)
{
	TESRenderSelection* Result = NULL;

	std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
	if (SelectionList)
	{
		for (std::vector<TESRenderSelection*>::iterator Itr = SelectionList->begin(); Itr != SelectionList->end(); Itr++)
		{
			TESRenderSelection* Base = *Itr;
			if (Base->SelectionCount == Selection->SelectionCount)
			{
				bool Mismatch = false;
				for (int i = 0; i < Selection->SelectionCount; i++)
				{
					if (GetRefAtSelectionIndex(Selection, i) != GetRefAtSelectionIndex(Base, i))
					{
						Mismatch = true;
						break;
					}
				}
				if (Mismatch)
					break;

				Result = Base;
			}
		}		
	}

	return Result;
}

void RenderSelectionGroupManager::UntrackSelection(TESObjectCELL* Cell, TESRenderSelection* Selection)
{
	std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
	if (SelectionList)
	{	
		std::vector<TESRenderSelection*>::const_iterator EraseItr = SelectionList->end();

		for (std::vector<TESRenderSelection*>::iterator Itr = SelectionList->begin(); Itr != SelectionList->end(); Itr++)
		{
			if (*Itr == Selection)
			{
				thisCall(kTESRenderSelection_ClearSelection, *Itr, 0);
				thisCall(kTESRenderSelection_Free, *Itr);
				FormHeap_Free(*Itr);

//				DebugPrint("Freed Selection %08X", (UInt32)*Itr);

				EraseItr = Itr;
				break;
			}
		}

		if (EraseItr != SelectionList->end())
			SelectionList->erase(EraseItr);
	}
}

bool RenderSelectionGroupManager::AddGroup(TESObjectCELL *Cell, TESRenderSelection *Selection)
{
	bool Result = false;

	std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
	if (SelectionList)
	{
		bool ExistingGroup = false;
		for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->RenderSelection; Itr && Itr->Data; Itr = Itr->Next)
		{
			if (GetRefSelectionGroup(Itr->Data, Cell))
			{
				ExistingGroup = true;
				break;
			}
		}

		if (!ExistingGroup)
		{
			TESRenderSelection* Group = AllocateNewSelection(Selection);
			SelectionList->push_back(Group);
			Result = true;	
		}
	}
	else
	{
		SelectionGroupMap[Cell] = std::vector<TESRenderSelection*>();

		TESRenderSelection* Group = AllocateNewSelection(Selection);
		SelectionGroupMap[Cell].push_back(Group);
		Result = true;
	}

	return Result;
}

bool RenderSelectionGroupManager::RemoveGroup(TESObjectCELL *Cell, TESRenderSelection *Selection)
{
	bool Result = false;

	std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
	if (SelectionList)
	{
		TESRenderSelection* TrackedSelection = GetTrackedSelection(Cell, Selection);
		if (TrackedSelection)
		{
			UntrackSelection(Cell, TrackedSelection);
			Result = true;
		}
	}

	return Result;
}