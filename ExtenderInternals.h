#pragma once
#pragma warning(disable : 4800; disable : 4005)

#include "obse_editor/EditorAPI.h"
#include "obse/GameData.h"
#include "obse/Script.h"
#include "obse/PluginAPI.h"
#include "obse/obse_common/SafeWrite.h"


#include <string>
#include <fstream>
#include <map>
#include "windows.h"
#include "atltypes.h"
#include "commctrl.h"
#include "richedit.h"

extern std::fstream					g_DEBUG_LOG;
extern std::string					g_AppPath;
extern std::string					g_INIPath;
extern char							g_Buffer[0x200];

extern OBSEMessagingInterface*		g_msgIntfc;
extern PluginHandle					g_pluginHandle;

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

#define CS_CAST(obj, from, to) (to *)Oblivion_DynamicCast((void*)(obj), 0, RTTI_ ## from, RTTI_ ## to, 0)

extern HINSTANCE*		g_TESCS_Instance;
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
extern const void *			RTTI_TESObjectCELL;
extern const void *			RTTI_TESCellUseList;

extern TES** g_TES;

extern const UInt32			kTESObjectREFR_VTBL;
extern const UInt32			kTESForm_GetObjectUseRefHeadFnAddr;
extern const UInt32			kTESCellUseList_GetUseListRefHeadFnAddr;
extern const UInt32			kTESObjectCELL_GetParentWorldSpaceFnAddr;

typedef void*			(__cdecl *_GetComboBoxItemData)(HWND ComboBox);
extern const _GetComboBoxItemData GetComboBoxItemData;


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

extern INISetting*			g_INI_LocalMasterPath;



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

