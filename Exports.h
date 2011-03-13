#pragma once

#include "ExtenderInternals.h"
#include "[Common]\HandShakeStructs.h"

class DataHandler;
extern FormData*				g_FormData;
extern UseListCellItemData*		g_UseListCellItemData;

// would be more aesthetically pleasing if this were rearranged into different interfaces but meh
// CSE's gonna be the only user

extern "C"
{

__declspec(dllexport) void _D_PRINT(UInt8 Source, const char* Message);
__declspec(dllexport) const char* GetINIString(const char* Section, const char* Key, const char* Default);
__declspec(dllexport) const char* GetAppPath(void);
__declspec(dllexport) void WriteStatusBarText(int PanelIndex, const char* Message);
__declspec(dllexport) HWND GetCSMainWindowHandle(void);
__declspec(dllexport) HWND GetRenderWindowHandle(void);

__declspec(dllexport) void ScriptEditor_MessagingInterface(UInt32 TrackedEditorIndex, UInt16 Message);
__declspec(dllexport) void ScriptEditor_SetScriptData(UInt32 TrackedEditorIndex, ScriptData* Data);
__declspec(dllexport) void ScriptEditor_SetWindowParameters(UInt32 TrackedEditorIndex, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height);
__declspec(dllexport) UInt32 ScriptEditor_InstantiateCustomEditor(const char* ScriptID);
__declspec(dllexport) void ScriptEditor_PostProcessEditorInit(UInt32 AllocatedIndex);
__declspec(dllexport) ScriptData* ScriptEditor_GetScriptData();
__declspec(dllexport) const char* ScriptEditor_GetAuxScriptName();

__declspec(dllexport) ScriptData* FetchScriptFromForm(const char* EditorID);
__declspec(dllexport) bool IsFormAnObjRefr(const char* EditorID);
__declspec(dllexport) FormData* LookupFormByEditorID(const char* EditorID);	

__declspec(dllexport) void ScriptEditor_GetScriptListData(UInt32 TrackedEditorIndex);
__declspec(dllexport) const char* ScriptEditor_GetScriptListItemText(const char* EditorID);
__declspec(dllexport) void ScriptEditor_SetScriptListResult(const char* EditorID);

__declspec(dllexport) void ScriptEditor_GetUseReportForForm(const char* EditorID);

__declspec(dllexport) void ScriptEditor_GetScriptVariableIndices(UInt32 TrackedEditorIndex, const char* EditorID);
__declspec(dllexport) bool ScriptEditor_SetScriptVariableIndex(const char* EditorID, ScriptVarIndexData::ScriptVarInfo* Data);
__declspec(dllexport) void ScriptEditor_CompileDependencies(const char* EditorID);

__declspec(dllexport) IntelliSenseUpdateData* ScriptEditor_BeginIntelliSenseDatabaseUpdate();
__declspec(dllexport) void ScriptEditor_EndIntelliSenseDatabaseUpdate(IntelliSenseUpdateData* Data);
__declspec(dllexport) void ScriptEditor_ToggleScriptCompiling(bool Enable);
__declspec(dllexport) void ScriptEditor_SaveActivePlugin();
__declspec(dllexport) void ScriptEditor_SetScriptText(const char* EditorID, const char* ScriptText);
__declspec(dllexport) void ScriptEditor_BindScript(const char* EditorID, HWND Parent);

__declspec(dllexport) void UseInfoList_SetFormListItemText();
__declspec(dllexport) void UseInfoList_SetObjectListItemText(const char* EditorID);
__declspec(dllexport) void UseInfoList_SetCellListItemText(const char* EditorID);

__declspec(dllexport) void TESForm_LoadIntoView(const char* EditorID, const char* FormType);

__declspec(dllexport) void BatchRefEditor_SetFormListItem(UInt8 ListID);
__declspec(dllexport) const char* BatchRefEditor_ChooseParentReference(BatchRefData* Data, HWND Parent);

__declspec(dllexport) void TagBrowser_GetObjectWindowSelection(void);
__declspec(dllexport) void TagBrowser_InstantiateObjects(TagBrowserInstantiationData* Data);

}

template <typename tData>
void UseInfoList_SetFormListItemText_ParseFormNode(DataHandler::Node<tData>* ThisNode);

TESObjectREFR* TESForm_LoadIntoView_GetReference(TESObjectCELL* Cell, TESForm* Parent);

template <typename tData>
void BatchRefEditor_ParseFormNode(DataHandler::Node<tData>* ThisNode, UInt8 ListID);

UInt32 ScriptEditor_CompileDependencies_CheckConditions(ConditionEntry* Entry, TESForm* ToCompare);
void ScriptEditor_CompileDependencies_ParseObjectUseList(TESForm* Form);