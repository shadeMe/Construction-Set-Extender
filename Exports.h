#pragma once

extern FormData*				g_FormDataInteropPackage;
extern UseListCellItemData*		g_UseListCellItemDataInteropPackage;

// would be more aesthetically pleasing if this were rearranged into different interfaces but meh
// CSE's gonna be the only user

extern "C"
{
	__declspec(dllexport) void __stdcall _D_PRINT(UInt8 Source, const char* Message);
	__declspec(dllexport) const char* __stdcall GetINIString(const char* Section, const char* Key, const char* Default);
	__declspec(dllexport) const char* __stdcall GetAppPath(void);
	__declspec(dllexport) void __stdcall WriteStatusBarText(int PanelIndex, const char* Message);
	__declspec(dllexport) HWND __stdcall GetCSMainWindowHandle(void);
	__declspec(dllexport) HWND __stdcall GetRenderWindowHandle(void);

	__declspec(dllexport) void __stdcall ScriptEditor_MessagingInterface(UInt32 TrackedEditorIndex, UInt16 Message);
	__declspec(dllexport) void __stdcall ScriptEditor_SetScriptData(UInt32 TrackedEditorIndex, ScriptData* Data);
	__declspec(dllexport) void __stdcall ScriptEditor_SetWindowParameters(UInt32 TrackedEditorIndex, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height);
	__declspec(dllexport) UInt32 __stdcall ScriptEditor_InstantiateCustomEditor(const char* ScriptID);
	__declspec(dllexport) void __stdcall ScriptEditor_PostProcessEditorInit(UInt32 AllocatedIndex);
	__declspec(dllexport) ScriptData* __stdcall ScriptEditor_GetScriptData();
	__declspec(dllexport) const char* __stdcall ScriptEditor_GetAuxScriptName();

	__declspec(dllexport) ScriptData* __stdcall FetchScriptFromForm(const char* EditorID);
	__declspec(dllexport) bool __stdcall IsFormAnObjRefr(const char* EditorID);
	__declspec(dllexport) FormData* __stdcall LookupFormByEditorID(const char* EditorID);

	__declspec(dllexport) void __stdcall ScriptEditor_GetScriptListData(UInt32 TrackedEditorIndex);
	__declspec(dllexport) const char* __stdcall ScriptEditor_GetScriptListItemText(const char* EditorID);
	__declspec(dllexport) void __stdcall ScriptEditor_SetScriptListResult(const char* EditorID);

	__declspec(dllexport) void __stdcall ScriptEditor_GetUseReportForForm(const char* EditorID);

	__declspec(dllexport) void __stdcall ScriptEditor_GetScriptVariableIndices(UInt32 TrackedEditorIndex, const char* EditorID);
	__declspec(dllexport) bool __stdcall ScriptEditor_SetScriptVariableIndex(const char* EditorID, ScriptVarIndexData::ScriptVarInfo* Data);
	__declspec(dllexport) void __stdcall ScriptEditor_CompileDependencies(const char* EditorID);

	__declspec(dllexport) IntelliSenseUpdateData* __stdcall ScriptEditor_BeginIntelliSenseDatabaseUpdate();
	__declspec(dllexport) void __stdcall ScriptEditor_EndIntelliSenseDatabaseUpdate(IntelliSenseUpdateData* Data);
	__declspec(dllexport) void __stdcall ScriptEditor_ToggleScriptCompiling(bool Enable);
	__declspec(dllexport) void __stdcall ScriptEditor_SaveActivePlugin();
	__declspec(dllexport) void __stdcall ScriptEditor_SetScriptText(const char* EditorID, const char* ScriptText);
	__declspec(dllexport) void __stdcall ScriptEditor_BindScript(const char* EditorID, HWND Parent);

	__declspec(dllexport) void __stdcall UseInfoList_SetFormListItemText();
	__declspec(dllexport) void __stdcall UseInfoList_SetObjectListItemText(const char* EditorID);
	__declspec(dllexport) void __stdcall UseInfoList_SetCellListItemText(const char* EditorID);

	__declspec(dllexport) void __stdcall TESForm_LoadIntoView(const char* EditorID, const char* FormType);

	__declspec(dllexport) void __stdcall BatchRefEditor_SetFormListItem(UInt8 ListID);
	__declspec(dllexport) const char* __stdcall BatchRefEditor_ChooseParentReference(BatchRefData* Data, HWND Parent);

	__declspec(dllexport) void __stdcall TagBrowser_InstantiateObjects(TagBrowserInstantiationData* Data);
}