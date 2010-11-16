#include "ExtenderInternals.h"
#include "Exports.h"

EditorAllocator*					EditorAllocator::Singleton = NULL;
EditorAllocator::SEAlloc*			EditorAllocator::NullRef = new EditorAllocator::SEAlloc(NULL, NULL, NULL);
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

INISetting*							g_LocalMasterPath = (INISetting*)0x009ED710;
ModEntry::Data**					g_TESActivePlugin = (ModEntry::Data**)0x00A0AA7C;
char**								g_TESActivePluginName = (char**)0x00A0AF00;
UInt8*								g_WorkingFileFlag = (UInt8*)0x00A0B628;
UInt8*								g_ActiveChangesFlag = (UInt8*)0x00A0B13C;
TESRenderWindowBuffer**				g_TESRenderWindowBuffer = (TESRenderWindowBuffer**)0x00A0AF60;
HMENU*								g_RenderWindowPopup = (HMENU*)0x00A0BC40;
void*								g_ScriptCompilerUnkObj = (void*)0x00A0B128;
TESWaterForm**						g_DefaultWater = (TESWaterForm**)0x00A137CC;

const _WriteToStatusBar				WriteToStatusBar	=	(_WriteToStatusBar)0x00431310;
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

const void*							RTTI_TESCellUseList = (void*)0x009EB2E4;

const UInt32						kVTBL_TESObjectREFR = 0x00958824;
const UInt32						kVTBL_TESForm = 0x0094688C;
const UInt32						kVTBL_TESTopicInfo = 0x0094820C;
const UInt32						kVTBL_TESQuest = 0x00945D7C;

const UInt32						kTESChildCell_LoadCell = 0x00430F40; 
const UInt32						kTESForm_GetObjectUseList = 0x00496380;		// Node<TESForm> GetObjectUseRefHead(UInt32 unk01 = 0);
const UInt32						kTESCellUseList_GetUseListRefHead = 0x006E5850;
const UInt32						kTESObjectCELL_GetParentWorldSpace = 0x00532E50;
const UInt32						kScript_SaveResultScript = 0x005034E0;
const UInt32						kScript_SaveScript = 0x00503450;
const UInt32						kLinkedListNode_NewNode = 0x004E3900;

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
	UInt32 Result = NextIndex;
	AllocationMap.insert(std::make_pair<HWND, SEAlloc*>(EditorDialog, new SEAlloc(GetDlgItem(EditorDialog, 1166), GetDlgItem(EditorDialog, 2259), Result)));
	++NextIndex;
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
		DeleteTrackedEditor(Itr->second->Index);
	}	
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

void LoadFormIntoView(const char* EditorID, const char* FormType)
{
	UInt32 Type = GetDialogTemplate(FormType);
	TESDialogInitParam InitData(EditorID);

	switch (Type)
	{
	case 9:					
		ScriptEditor_InstantiateCustomEditor(EditorID);
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

bool __stdcall AreUnModifiedFormsHidden()
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
	UInt32 State = GetMenuState(ViewMenu, 9904, MF_BYCOMMAND);

	return (State & MF_CHECKED);
}

void ToggleHideUnModifiedForms(bool State)
{
	DeInitializeCSWindows();	

	SendMessage(*g_HWND_CellView, 0x40E, 1, 1);			// for worldspaces
	SendMessage(*g_HWND_AIPackagesDlg, 0x41A, 0, 0);	// for AI packages

	InitializeCSWindows();
	InvalidateRect(*g_HWND_ObjectWindow_FormList, NULL, TRUE);
}