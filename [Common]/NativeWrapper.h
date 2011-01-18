#pragma once
#include "Includes.h"
#include "HandShakeStructs.h"

class NativeWrapper
{
	[DllImport("Construction Set Extender.dll")] 
	static void											_D_PRINT(UInt8 Source, const char* Message);
	[DllImport("Construction Set Extender.dll")] 
	static void											WriteStatusBarText(int PanelIndex, const char* Message);
public:
	[DllImport("Construction Set Extender.dll")] 
	static const char*									GetAppPath(void);
	[DllImport("Construction Set Extender.dll")] 
	static IntPtr										GetCSMainWindowHandle(void);

	[DllImport("Construction Set Extender.dll")] 
	static ScriptData*									FetchScriptFromForm(const char* EditorID);
	[DllImport("Construction Set Extender.dll")] 
	static bool											IsFormAnObjRefr(const char* EditorID);
	

	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_MessagingInterface(UInt32 TrackedEditorIndex, UInt16 Message);
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_SetScriptData(UInt32 TrackedEditorIndex, ScriptData* Data);
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_SetWindowParameters(UInt32 TrackedEditorIndex, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height);
	[DllImport("Construction Set Extender.dll")] 
	static UInt32										ScriptEditor_InstantiateCustomEditor(const char* ScriptID);
	[DllImport("Construction Set Extender.dll")] 
	static ScriptData*									ScriptEditor_GetScriptData();
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_GetScriptListData(UInt32 TrackedEditorIndex);
	[DllImport("Construction Set Extender.dll")] 
	static const char*									ScriptEditor_GetScriptListItemText(const char* EditorID);
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_SetScriptListResult(const char* EditorID);
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_GetUseReportForForm(const char* EditorID);
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_GetScriptVariableIndices(UInt32 TrackedEditorIndex, const char* EditorID);
	[DllImport("Construction Set Extender.dll")] 
	static bool											ScriptEditor_SetScriptVariableIndex(const char* EditorID, ScriptVarIndexData::ScriptVarInfo* Data);
	[DllImport("Construction Set Extender.dll")] 
	static const char*									ScriptEditor_GetAuxScriptName();
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_PostProcessEditorInit(UInt32 AllocatedIndex);
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_CompileDependencies(const char* EditorID);
	[DllImport("Construction Set Extender.dll")] 
	static IntelliSenseUpdateData*						ScriptEditor_BeginIntelliSenseDatabaseUpdate();
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_EndIntelliSenseDatabaseUpdate(IntelliSenseUpdateData* Data);
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_ToggleScriptCompiling(bool Enable);
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_SaveActivePlugin();
	[DllImport("Construction Set Extender.dll")] 
	static void											ScriptEditor_SetScriptText(const char* EditorID, const char* ScriptText);

	[DllImport("Construction Set Extender.dll")] 
	static void											UseInfoList_SetFormListItemText();
	[DllImport("Construction Set Extender.dll")] 
	static void											UseInfoList_SetObjectListItemText(const char* EditorID);
	[DllImport("Construction Set Extender.dll")] 
	static void											UseInfoList_SetCellListItemText(const char* EditorID);

	[DllImport("Construction Set Extender.dll")] 
	static void											TESForm_LoadIntoView(const char* EditorID, const char* FormType);

	[DllImport("Construction Set Extender.dll")] 
	static void											BatchRefEditor_SetFormListItem(UInt8 ListID);
	[DllImport("Construction Set Extender.dll")] 
	static const char*									BatchRefEditor_ChooseParentReference(BatchRefData* Data, IntPtr Parent);

	[DllImport("user32.dll")]
	static bool											LockWindowUpdate(IntPtr hWndLock);
	[DllImport("user32.dll")]
	static IntPtr										WindowFromPoint(Point Point);


	static void											PrintToCSStatusBar(int PanelIndex, String^ Message);
	friend void											DebugDump(UInt8 Source, String^% Message);
};

class INIWrapper
{
	[DllImport("Construction Set Extender.dll")]
	static const char* GetINIString(const char* Section, const char* Key, const char* Default);
	[DllImport("KERNEL32.DLL", EntryPoint="WritePrivateProfileStringW",
		SetLastError=true,
		CharSet=CharSet::Unicode, ExactSpelling=true,
		CallingConvention=CallingConvention::StdCall)]
	static int											WritePrivateProfileString(String^ Section, String^ Key, String^ Value, String^ INIPath);
public:
	static String^										GetINIValue(String^ Section, String^ Key, String^ Default);
	static void											SetINIValue(String^ Section, String^ Key, String^ Value);
};

public ref class WindowHandleWrapper : public IWin32Window
{
	IntPtr					_hwnd;
public:
	WindowHandleWrapper(IntPtr Handle) : _hwnd(Handle) {}

    property IntPtr Handle
    {
        virtual IntPtr get()
        {
            return _hwnd;
        }
    };
};


