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

#include "[ Libraries ]\CSE Handshake\CSEL.h"
#include "[ Libraries ]\INI Manager\INIManager.h"

class CSEINIManager;

extern std::fstream					g_DEBUG_LOG;
extern std::string					g_AppPath;
extern std::string					g_INIPath;
extern char							g_Buffer[0x200];

extern OBSEMessagingInterface*		g_msgIntfc;
extern PluginHandle					g_pluginHandle;
extern HINSTANCE					g_DLLInstance;
extern CSEINIManager*				g_INIManager;

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

class CSEINIManager : public SME::INI::INIManager
{
public:
	void									Initialize();
};


				
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

	char*			Data;			// 00 - use a union ?
	const char*		Name;			// 04

	INISetting(char* Data, char* Name) : Data(Data), Name(Name) {}
};

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

// 38 / A0
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
		// ? / 24
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

#ifndef OBLIVION
STATIC_ASSERT(sizeof(TESTopicInfo) == 0xA0);
#endif



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

extern const UInt32			kVTBL_TESObjectREFR;
extern const UInt32			kVTBL_TESForm;
extern const UInt32			kTESForm_GetObjectUseListAddr;
extern const UInt32			kTESCellUseList_GetUseListRefHeadFnAddr;
extern const UInt32			kTESObjectCELL_GetParentWorldSpaceFnAddr;
extern const UInt32			kScript_SaveResultScript;
extern const UInt32			kScript_SaveScript;

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

extern TESRenderWindowBuffer**	g_TESRenderWindowBuffer;

TESObjectREFR* ChooseReferenceDlg(HWND Parent);

extern HMENU*					g_RenderWindowPopup;
extern void*					g_ScriptCompilerUnkObj;
extern const UInt32				kVTBL_TESTopicInfo;
extern const UInt32				kVTBL_TESQuest;
