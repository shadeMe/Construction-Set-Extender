#pragma once
#pragma warning(disable : 4800; disable : 4005)

#include "UtilityBox.h"
#include "obse_editor/EditorAPI.h"
#include "obse/GameData.h"
#include "obse/Script.h"
#include "obse/PluginAPI.h"
#include "obse/GameObjects.h"
#include "Console.h"

#include "[ Libraries ]\INI Manager\INIManager.h"
#include "[ Libraries ]\INI Manager\INIEditGUI.h"

extern std::fstream					g_DEBUG_LOG;
extern std::string					g_AppPath;
extern std::string					g_INIPath;
extern char							g_Buffer[0x200];

extern OBSEMessagingInterface*		g_msgIntfc;
extern PluginHandle					g_pluginHandle;
extern HINSTANCE					g_DLLInstance;
extern SME::INI::INIManager*		g_INIManager;
extern SME::INI::INIEditGUI*		g_INIEditGUI;

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
	UInt32																	LastContactedEditor;
public:
	static EditorAllocator*													GetSingleton(void);
	UInt32																	TrackNewEditor(HWND EditorDialog);
	void																	DeleteTrackedEditor(UInt32 TrackedEditorIndex);
	void																	DeleteAllTrackedEditors(void);
	void																	DestroyVanillaDialogs(void);

	HWND																	GetTrackedREC(HWND TrackedEditorDialog);
	HWND																	GetTrackedLBC(HWND TrackedEditorDialog);
	UInt32																	GetTrackedIndex(HWND TrackedEditorDialog);
	HWND																	GetTrackedDialog(UInt32 TrackedEditorIndex);
	UInt32																	GetTrackedEditorCount() { return AllocationMap.size(); }
	UInt32																	GetLastContactedEditor() { return LastContactedEditor; }
	void																	SetLastContactedEditor(UInt32 TrackedEditorIndex) { LastContactedEditor = TrackedEditorIndex; }
};

#define EDAL																EditorAllocator::GetSingleton()

class CSEINIManager : public SME::INI::INIManager
{
public:
	void									Initialize();
};


				
// 0C+?
class TESDialogInitParam
{
public:
	UInt32		TypeID;					// 00
	TESForm*	Form;					// 04
	UInt32		unk08;					// 08

	TESDialogInitParam(const char* EditorID);
};

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

	union							// 00
	{
		int			iData;
		UInt32		uData;
		float		fData;
		char*		sData;
	};

	const char*		Name;			// 04
};

// 24
class TESFormIDListView
{
public:
	TESFormIDListView();
	~TESFormIDListView();

	// bases
	TESForm					Form;

	// no members
};

// 2C
class GameSetting
{
public:
	GameSetting();
	~GameSetting();

	// bases
	TESFormIDListView		listView;

	//members
	union								// 24
	{
		int					iData;
		float				fData;
		const char*			sData;
	};

	const char*				settingID;	// 28
};

class GameSettingCollection;

template<typename Type> struct GenericNode
{
	Type				* data;
	GenericNode<Type>	* next;
};

// 18 
struct TESRenderWindowBuffer 
{
	struct SelectedObjectsEntry
	{
		TESObjectREFR*			Data;
		SelectedObjectsEntry*	Prev;
		SelectedObjectsEntry*	Next;
	};

	SelectedObjectsEntry*	RenderSelection;	// 00
	UInt32					SelectionCount;		// 04
	float					x, y, z;			// 08 sum of position vectors of selected refr's
	float					unk14;				// 14 init to 0.0
};

// A0
class TESTopicInfo : public TESForm
{
public:
	TESTopicInfo();
	~TESTopicInfo();

	struct TopicListEntry
	{
		TESTopic*			data;
		TopicListEntry*		next;
	};

	struct LinkedTopics
	{
		TopicListEntry		topicsLinkedFrom;
		TopicListEntry		topicsLinkedTo;		// doubles as choices for appropriate infotype
	};

	struct ResponseEntry
	{
		// 24
		struct Data 
		{
			enum 
			{
				kEmotionType_Neutral = 0,
				kEmotionType_Anger,
				kEmotionType_Disgust,
				kEmotionType_Fear,
				kEmotionType_Sad,
				kEmotionType_Happy,
				kEmotionType_Surprise,
			};

			UInt32			emotionType;				// 00
			UInt32			emotionValue;				// 04
			UInt32			unk08;						// 08
			UInt32			unk0C;						// 0C
			String			responseText;				// 10
			String			actorNotes;					// 18	
			UInt32			unk20;						// 20
		};

		Data*				data;
		ResponseEntry*		next;		
	};

	 enum
	 {
		kInfoType_Topic = 0,
		kInfoType_Conversation,
		kInfoType_Combat,
		kInfoType_Persuasion,
		kInfoType_Detection,
		kInfoType_Service,
		kInfoType_Miscellaneous
	 };

	 enum
	 {
		kFlags_Goodbye = 0x0001,
		kFlags_Random = 0x0002,
		kFlags_SayOnce = 0x0004,
		kFlags_Unk008 = 0x0008,
		kFlags_InfoRefusal = 0x0010,
		kFlags_RandomEnd = 0x0020,
		kFlags_RunforRumors = 0x0040
	 };

	TESTopic*			topic024;		// 24 - always NULL ?
	ConditionEntry		conditions;		// 28
	UInt16				unk30;			// 30 - init to -1. used to determine previous info	
	UInt16				infotype;		// 32 
	UInt8				flags;			// 34
	UInt8				flagsPad[3];	// 35
	TopicListEntry		addedTopics;	// 38
	LinkedTopics*		linkedTopics;	// 40
	ResponseEntry		responses;		// 44
	Script				resultScript;	// 4C
};

union GMSTData
{
	int				i;
	float			f;
	const char*		s;	
};
struct GMSTMap_Key_Comparer
{
	bool operator()(const char* Key1, const char* Key2) const {
		return _stricmp(Key1, Key2) < 0;
	}
};

typedef std::map<const char*, GMSTData*, GMSTMap_Key_Comparer>		_DefaultGMSTMap;
extern _DefaultGMSTMap			g_DefaultGMSTMap;


extern const HINSTANCE*			g_TESCS_Instance;

extern const DLGPROC			g_ScriptEditor_DlgProc;
extern const DLGPROC			g_UseReport_DlgProc;
extern const DLGPROC			g_TESDialog_DlgProc;
extern const DLGPROC			g_TESDialogListView_DlgProc;

extern HWND*					g_HWND_RenderWindow;
extern HWND*					g_HWND_ObjectWindow;
extern HWND*					g_HWND_CellView;
extern HWND*					g_HWND_CSParent;
extern HWND*					g_HWND_AIPackagesDlg;
extern HWND*					g_HWND_ObjectWindow_FormList;
extern HWND*					g_HWND_ObjectWindow_Tree;

extern INISetting*				g_LocalMasterPath;
extern char**					g_TESActivePluginName;
extern UInt8*					g_WorkingFileFlag;
extern UInt8*					g_ActiveChangesFlag;

extern TESWaterForm**			g_DefaultWater;
extern TESRenderWindowBuffer**	g_TESRenderWindowBuffer;
extern HMENU*					g_RenderWindowPopup;
extern void*					g_ScriptCompilerUnkObj;
extern TESObjectREFR**			g_PlayerRef;
extern GameSettingCollection*	g_GMSTCollection;
extern void*					g_GMSTMap;			// BSTCaseInsensitiveMap<GMSTData*>* , should be similar to OBSE's SettingInfo


typedef LRESULT (__cdecl *_WriteToStatusBar)(WPARAM wParam, LPARAM lParam);
extern const _WriteToStatusBar WriteToStatusBar;

typedef UInt32			(__cdecl *_WritePositionToINI)(HWND Handle, CHAR* ClassName);
extern const _WritePositionToINI WritePositionToINI;

typedef bool			(__cdecl *_GetPositionFromINI)(const char* WindowName, tagRECT* Rect);
extern const _GetPositionFromINI GetPositionFromINI;

typedef UInt32			(__cdecl *_GetTESDialogTemplateForType)(UInt8 FormTypeID);
extern const _GetTESDialogTemplateForType GetTESDialogTemplateForType;

typedef void*			(__cdecl *_GetComboBoxItemData)(HWND ComboBox);
extern const _GetComboBoxItemData GetComboBoxItemData;

typedef bool			(__cdecl *_SelectTESFileCommonDialog)(HWND Parent, const char* SaveDir, bool SaveAsESM, char* Buffer, size_t Size);
extern const _SelectTESFileCommonDialog SelectTESFileCommonDialog;

typedef void			(__cdecl *_sub_4306F0)(bool unk01);
extern const _sub_4306F0 sub_4306F0;

typedef TESObjectREFR* (__cdecl *_ChooseRefWrapper)(HWND Parent, UInt32 unk01, UInt32 unk02, UInt32 unk03);
extern const _ChooseRefWrapper	ChooseRefWrapper;

typedef void*			(__cdecl *_InitializeCSWindows)();
extern const _InitializeCSWindows		InitializeCSWindows;

typedef void			(__cdecl *_DeInitializeCSWindows)();
extern const _DeInitializeCSWindows		DeInitializeCSWindows;

typedef void			(__cdecl *_AddFormToObjectWindow)(TESForm* Form);
extern const _AddFormToObjectWindow		AddFormToObjectWindow;

typedef SpellItem*		(__stdcall *_InitializeDefaultPlayerSpell)(void* Throwaway);
extern const _InitializeDefaultPlayerSpell		InitializeDefaultPlayerSpell;

typedef void (__cdecl *_ConstructEffectSetting)(int EffectID, const char *EffectName, int School, float BaseCost, int MGEFParamA, int Flags, int ResistAV, int NoOfCounterEffects, ...);
extern const _ConstructEffectSetting	ConstructEffectSetting;

typedef void (__cdecl *_TESDialog_AddComboBoxItem)(HWND hWnd, const char* Text, LPARAM unk3, UInt8 unk4);
extern const _TESDialog_AddComboBoxItem TESDialog_AddComboBoxItem;

typedef void			(__cdecl *_BSPrintF)(const char* format, ...);
extern const _BSPrintF		BSPrintF;

typedef void			(__cdecl *_ShowCompilerError)(ScriptBuffer* Buffer, const char* format, ...);
extern const _ShowCompilerError		ShowCompilerErrorEx;

typedef void			(__cdecl *_AutoSavePlugin)(void);
extern const _AutoSavePlugin		AutoSavePlugin;

extern const void *			RTTI_TESCellUseList;

extern const UInt32			kTESChildCell_LoadCell;
extern const UInt32			kTESForm_GetObjectUseList;
extern const UInt32			kTESCellUseList_GetUseListRefHead;
extern const UInt32			kTESObjectCELL_GetParentWorldSpace;
extern const UInt32			kScript_SaveResultScript;
extern const UInt32			kScript_SaveScript;
extern const UInt32			kLinkedListNode_NewNode;
extern const UInt32			kDataHandler_AddBoundObject;
extern const UInt32			kTESForm_SetFormID;
extern const UInt32			kTESForm_SetEditorID;
extern const UInt32			kTESObjectREFR_SetBaseForm;
extern const UInt32			kTESObjectREFR_SetFlagPersistent;

extern const UInt32			kBaseExtraList_ModExtraEnableStateParent;
extern const UInt32			kBaseExtraList_ModExtraOwnership;
extern const UInt32			kBaseExtraList_ModExtraGlobal;
extern const UInt32			kBaseExtraList_ModExtraRank;
extern const UInt32			kBaseExtraList_ModExtraCount;
extern const UInt32			kTESObjectREFR_ModExtraHealth;
extern const UInt32			kTESObjectREFR_ModExtraCharge;
extern const UInt32			kTESObjectREFR_ModExtraTimeLeft;
extern const UInt32			kTESObjectREFR_ModExtraSoul;
extern const UInt32			kTESObjectREFR_SetExtraEnableStateParent_OppositeState;

extern const UInt32			kVTBL_TESObjectREFR;
extern const UInt32			kVTBL_TESForm;
extern const UInt32			kVTBL_TESTopicInfo;
extern const UInt32			kVTBL_TESQuest;
extern const UInt32			kVTBL_TESNPC;
extern const UInt32			kVTBL_TESCreature;
extern const UInt32			kVTBL_TESFurniture;
extern const UInt32			kVTBL_TESObjectMISC;
extern const UInt32			kVTBL_TESObjectWEAP;
extern const UInt32			kVTBL_TESObjectCONT;
extern const UInt32			kVTBL_SpellItem;

extern const UInt32			kTESNPC_Ctor;
extern const UInt32			kTESCreature_Ctor;
extern const UInt32			kTESFurniture_Ctor;
extern const UInt32			kTESObjectMISC_Ctor;
extern const UInt32			kTESObjectWEAP_Ctor;
extern const UInt32			kTESObjectCONT_Ctor;
extern const UInt32			kTESObjectREFR_Ctor;


TESObjectREFR*				ChooseReferenceDlg(HWND Parent);
UInt32						GetDialogTemplate(const char* FormType);
UInt32						GetDialogTemplate(UInt8 FormTypeID);
void						RemoteLoadRef(const char* EditorID);
void						LoadFormIntoView(const char* EditorID, const char* FormType);
void						LoadFormIntoView(const char* EditorID, UInt8 FormType);
bool __stdcall				AreUnModifiedFormsHidden();
void						ToggleHideUnModifiedForms(bool State);
void						LoadStartupPlugin();
void						InitializeDefaultGMSTMap();