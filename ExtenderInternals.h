#pragma once
#pragma warning(disable : 4800; disable : 4005)

#include "obse_editor/EditorAPI.h"
#include "obse/GameData.h"
#include "obse/Script.h"
#include "obse/PluginAPI.h"
#include "obse/obse_common/SafeWrite.h"
#include "obse/GameObjects.h"
#include "Editor_RTTI.h"
//#include "EditorBSExtraData.h"


#include <string>
#include <fstream>
#include <map>
#include "windows.h"
#include "atltypes.h"
#include "commctrl.h"
#include "richedit.h"
#include "shlobj.h"
#include "resource.h"

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
void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]


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

extern RendSel**					g_RenderWindow_UnkLL;

TESObjectREFR* ChooseReferenceDlg(HWND Parent);

inline void _D_PRINT(const char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsprintf_s(g_Buffer, sizeof(g_Buffer), fmt, args);
	va_end(args);

	g_DEBUG_LOG << g_Buffer << std::endl;
}

void UpdateTESObjectREFR3D(TESObjectREFR* Object);	// in the render window


// __thisCall handlers (non-virtual)
template <typename Tthis>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(); } u = {addr};
    return ((T*)_this->*u.m)();
}
template <typename Tthis, typename T1>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1); } u = {addr};
    return ((T*)_this->*u.m)(arg1);
}
template <typename Tthis, typename T1, typename T2>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1, T2 arg2)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2); } u = {addr};
    return ((T*)_this->*u.m)(arg1, arg2);
}
template <typename Tthis, typename T1, typename T2, typename T3>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1, T2 arg2, T3 arg3)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3); } u = {addr};
    return ((T*)_this->*u.m)(arg1, arg2, arg3);
}
template <typename Tthis, typename T1, typename T2, typename T3, typename T4>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3, T4); } u = {addr};
    return ((T*)_this->*u.m)(arg1, arg2, arg3, arg4);
}
template <typename Tthis, typename T1, typename T2, typename T3, typename T4, typename T5>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3, T4, T5); } u = {addr};
    return ((T*)_this->*u.m)(arg1, arg2, arg3, arg6, arg5);
}

// __thisCall handlers (virtual)
template <typename Tthis>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)();
}
template <typename Tthis, typename T1>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1);
}
template <typename Tthis, typename T1, typename T2>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1, T2 arg2)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1, arg2);
}
template <typename Tthis, typename T1, typename T2, typename T3>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1, T2 arg2, T3 arg3)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1, arg2, arg3);
}
template <typename Tthis, typename T1, typename T2, typename T3, typename T4>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3, T4); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1, arg2, arg3, arg4);
}
template <typename Tthis, typename T1, typename T2, typename T3, typename T4, typename T5>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3, T4, T5); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1, arg2, arg3, arg6, arg5);
}

#define PLACE_HOOK(name)									WriteRelJump(k##name##HookAddr, (UInt32)##name##Hook)

