#pragma once
#pragma warning(disable : 4800; disable : 4005)

#include "UtilityBox.h"
#include "obse_editor/EditorAPI.h"
#include "obse/GameData.h"
#include "obse/Script.h"
#include "obse/PluginAPI.h"
#include "obse/obse_common/SafeWrite.h"
#include "obse/GameObjects.h"
#include "Editor_RTTI.h"

extern std::fstream					g_DEBUG_LOG;
extern std::string					g_AppPath;
extern std::string					g_INIPath;
extern char							g_Buffer[0x200];

extern OBSEMessagingInterface*		g_msgIntfc;
extern PluginHandle					g_pluginHandle;
extern HINSTANCE					g_DLLInstance;
extern TESObjectREFR*				g_Update3DBuffer;

// TODO: ++++++++++++++++++



class EditorAllocator
{
	static EditorAllocator*													Singleton;

	struct SEAlloc {
		HWND																RichEditControl;
		HWND																ListBoxControl;
		UInt32																Index;

																			SEAlloc(HWND REC, HWND LBC, UInt32 IDX): 
																			RichEditControl(REC), ListBoxControl(LBC), Index(IDX) {};
	};
	static SEAlloc*															NullRef;

	typedef std::map<HWND, SEAlloc*>										AlMap;
	AlMap																	AllocationMap;
	UInt32																	NextIndex;
public:
	static EditorAllocator*													GetSingleton(void);
	UInt32																	TrackNewEditor(HWND EditorDialog);
	void																	DeleteTrackedEditor(UInt32 TrackedEditorIndex);
	void																	DeleteAllTrackedEditors(void);

	HWND																	GetTrackedREC(HWND TrackedEditorDialog);
	HWND																	GetTrackedLBC(HWND TrackedEditorDialog);
	UInt32																	GetTrackedIndex(HWND TrackedEditorDialog);
	HWND																	GetTrackedDialog(UInt32 TrackedEditorIndex);
};

void																		LogWinAPIErrorMessage(DWORD ErrorID);

#define EDAL																EditorAllocator::GetSingleton()


				
// EDITOR API

// 0C+?
class TESDialogInitParam
{
public:
	UInt32		TypeID;					// 00
	TESForm*	Form;					// 04
	UInt32		unk08;					// 08

	TESDialogInitParam(const char* EditorID);
};


UInt32 GetDialogTemplate(const char* FormType);

void RemoteLoadRef(const char* EditorID);
void LoadFormIntoView(const char* EditorID, const char* FormType);



// 08
class TESCellUseData
{
public:
	TESCellUseData();
	~TESCellUseData();

	TESObjectCELL*		Cell;		// 00
	UInt32				Count;		// 04	
};

class TESCellUseList;

// 08
class INISetting
{
public:
	INISetting();
	~INISetting();

	char*			Data;			// 00
	const char*		Name;			// 04

	INISetting(char* Data, char* Name) : Data(Data), Name(Name) {}
};

template<typename Type> struct GenericNode
{
	Type				* data;
	GenericNode<Type>	* next;
};

struct RendSel // 0x18 ; ctor = 0x511A20
{
	GenericNode<TESObjectREFR>*		unk00;			// 00 - sel head?
	UInt32		unk04;			// 04 - sel count
	UInt32		unk08;			// 08 init to 0x00A8AF64
	UInt32		unk0C;			// 0C
	UInt32		unk10;			// 10 init to 0x00A8AF6C
	double		unk14;			// 14 init to 0.0
}; // = 0x00A0AF60;


#define CS_CAST(obj, from, to) (to *)Oblivion_DynamicCast((void*)(obj), 0, RTTI_ ## from, RTTI_ ## to, 0)

extern const HINSTANCE*	g_TESCS_Instance;
extern const DLGPROC	g_ScriptEditor_DlgProc;
extern const DLGPROC	g_UseReport_DlgProc;
extern const DLGPROC	g_TESDialog_DlgProc;
extern const DLGPROC	g_TESDialogListView_DlgProc;

typedef LRESULT (__cdecl *_WriteToStatusBar)(WPARAM wParam, LPARAM lParam);
extern const _WriteToStatusBar WriteToStatusBar;

extern HWND*			g_HWND_RenderWindow;
extern HWND*			g_HWND_ObjectWindow;
extern HWND*			g_HWND_CellView;
extern HWND*			g_HWND_CSParent;

typedef UInt32			(__cdecl *_WritePositionToINI)(HWND Handle, CHAR* ClassName);
extern const _WritePositionToINI WritePositionToINI;

typedef bool			(__cdecl *_GetPositionFromINI)(const char* WindowName, tagRECT* Rect);
extern const _GetPositionFromINI GetPositionFromINI;

typedef UInt32			(__cdecl *_GetTESDialogTemplateForType)(UInt8 FormTypeID);
extern const _GetTESDialogTemplateForType GetTESDialogTemplateForType;

extern const UInt32			kTESChildCell_LoadCellFnAddr;
extern const void *			RTTI_TESCellUseList;

extern TES** g_TES;

extern const UInt32			g_VTBL_TESObjectREFR;
extern const UInt32			g_VTBL_TESForm;
extern const UInt32			kTESForm_GetObjectUseRefHeadFnAddr;
extern const UInt32			kTESCellUseList_GetUseListRefHeadFnAddr;
extern const UInt32			kTESObjectCELL_GetParentWorldSpaceFnAddr;

typedef void*			(__cdecl *_GetComboBoxItemData)(HWND ComboBox);
extern const _GetComboBoxItemData GetComboBoxItemData;

typedef bool			(__cdecl *_SelectTESFileCommonDialog)(HWND Parent, const char* SaveDir, bool SaveAsESM, char* Buffer, size_t Size);
extern const _SelectTESFileCommonDialog SelectTESFileCommonDialog;

typedef void			(__cdecl *_sub_4306F0)(bool unk01);
extern const _sub_4306F0 sub_4306F0;

extern INISetting*			g_INI_LocalMasterPath;
extern ModEntry::Data**		g_TESActivePlugin;
extern char**				g_TESActivePluginName;
extern UInt8*				g_WorkingFileFlag;
extern UInt8*				g_ActiveChangesFlag;

typedef TESObjectREFR* (__cdecl *_ChooseRefWrapper)(HWND Parent, UInt32 unk01, UInt32 unk02, UInt32 unk03);
extern const _ChooseRefWrapper	ChooseRefWrapper;

extern const UInt32				kBaseExtraList_ModExtraEnableStateParent;
extern const UInt32				kBaseExtraList_ModExtraOwnership;
extern const UInt32				kBaseExtraList_ModExtraGlobal;
extern const UInt32				kBaseExtraList_ModExtraRank;
extern const UInt32				kTESObjectREFR_ModExtraHealth;
extern const UInt32				kBaseExtraList_ModExtraCount;
extern const UInt32				kTESObjectREFR_ModExtraCharge;
extern const UInt32				kTESObjectREFR_ModExtraTimeLeft;
extern const UInt32				kTESObjectREFR_ModExtraSoul;
extern const UInt32				kTESObjectREFR_SetExtraEnableStateParent_OppositeState;

extern TESWaterForm**			g_SpecialForm_DefaultWater;

extern RendSel**					g_RenderWindow_UnkLL;

TESObjectREFR* ChooseReferenceDlg(HWND Parent);



void UpdateTESObjectREFR3D(TESObjectREFR* Object);	// in the render window

