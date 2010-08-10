#include "ExtenderInternals.h"
#include "Exports.h"

EditorAllocator*					EditorAllocator::Singleton = NULL;
EditorAllocator::SEAlloc*			EditorAllocator::NullRef = new EditorAllocator::SEAlloc(NULL, NULL, NULL);
char								g_Buffer[0x200] = {0};
HINSTANCE							g_DLLInstance = NULL;


HINSTANCE*							g_TESCS_Instance = (HINSTANCE*)0x00A0AF1C;

const DLGPROC						g_ScriptEditor_DlgProc = (DLGPROC)0x004FE760;
const DLGPROC						g_UseReport_DlgProc = (DLGPROC)0x00433FE0;
const DLGPROC						g_TESDialog_DlgProc = (DLGPROC)0x00447580;
const DLGPROC						g_TESDialogListView_DlgProc = (DLGPROC)0x00448820;

const _WriteToStatusBar				WriteToStatusBar	=	(_WriteToStatusBar)0x00431310;

HWND*								g_HWND_RenderWindow = (HWND*)0x00A0AF28;
HWND*								g_HWND_ObjectWindow = (HWND*)0x00A0AF44;
HWND*								g_HWND_CellView = (HWND*)0x00A0AF4C;
HWND*								g_HWND_CSParent = (HWND*)0x00A0AF20;

const _WritePositionToINI			WritePositionToINI = (_WritePositionToINI)0x00417510;
const _GetPositionFromINI			GetPositionFromINI = (_GetPositionFromINI)0x004176D0;
const _GetTESDialogTemplateForType	GetTESDialogTemplateForType = (_GetTESDialogTemplateForType)0x00442050;

const UInt32						kTESChildCell_LoadCellFnAddr = 0x00430F40; 
const void*							RTTI_TESObjectCELL = (void*)0x009EA750;
const void*							RTTI_TESCellUseList = (void*)0x009EB2E4;

const UInt32						kTESObjectREFR_VTBL = 0x00958824;
const UInt32						kTESForm_GetObjectUseRefHeadFnAddr = 0x00496380;		// Node<TESForm*>* GetObjectUseRefHead(UInt32 unk01);

TES**								g_TES = (TES**)0x00A0ABB0;

const UInt32						kTESCellUseList_GetUseListRefHeadFnAddr = 0x006E5850;
const UInt32						kTESObjectCELL_GetParentWorldSpaceFnAddr = 0x00532E50;
INISetting*							g_INI_LocalMasterPath = (INISetting*)0x009ED710;

const _GetComboBoxItemData			GetComboBoxItemData = (_GetComboBoxItemData)0x00403690;
ModEntry::Data**					g_TESActivePlugin = (ModEntry::Data**)0x00A0AA7C;


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

void LogWinAPIErrorMessage(DWORD ErrorID)
{
	LPVOID ErrorMsg;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		ErrorID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &ErrorMsg,
		0, NULL );

	_D_PRINT(("\tError Message: " + std::string((LPSTR)ErrorMsg)).c_str()); 
	LocalFree(ErrorMsg);
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