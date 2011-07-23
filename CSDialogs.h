#pragma once

extern const char*					g_FormTypeIdentifier[];

class FormEnumerationWrapper
{
public:
	static void __stdcall ReinitializeFormLists();
	static bool GetUnmodifiedFormHiddenState();	// returns true when hidden
	static bool GetDeletedFormHiddenState();
	static bool __stdcall GetShouldEnumerateForm(TESForm* Form);
	static bool __stdcall PerformListViewPrologCheck(UInt32 CallAddress);
	static void ToggleUnmodifiedFormVisibility();
	static void	ToggleDeletedFormVisibility();

	static void __stdcall ResetFormVisibility(void);
};

class TESDialogWindowHandleCollection
{
	typedef std::vector<HWND>			_HandleCollection;
	_HandleCollection					WindowHandles;

	_HandleCollection::iterator			FindHandle(HWND Handle);
public:
	void								AddHandle(HWND Handle) { WindowHandles.push_back(Handle); }
	bool								RemoveHandle(HWND Handle);
	bool								GetHandleExists(HWND Handle) { return FindHandle(Handle) != WindowHandles.end(); }
	void								ClearHandles(void) { WindowHandles.clear(); }
};

extern TESDialogWindowHandleCollection	g_CustomMainWindowChildrenDialogs,		// used to keep them from being closed during a plugin load event
										g_DragDropSupportDialogs;				// keeps track of custom dialogs/controls that allow form (drag-)dropping

TESObjectREFR*				ShowReferencePickDialog(HWND Parent);
UInt32						GetFormDialogTemplate(const char* FormType);
UInt32						GetFormDialogTemplate(UInt8 FormTypeID);
void						LoadReferenceParentCell(const char* EditorID);
void						LoadReferenceParentCell(UInt32 FormID);
HWND						ShowFormEditDialog(const char* EditorID, const char* FormType);
HWND						ShowFormEditDialog(const char* EditorID, UInt8 FormType);
HWND						ShowFormEditDialog(UInt32 FormID, const char* FormType);
HWND						ShowFormEditDialog(UInt32 FormID, UInt8 FormType);
void						ResetRenderWindow();
void						InstantitateCustomScriptEditor(const char* ScriptEditorID);
void						InstantitateCustomScriptEditor(UInt32 ScriptFormID);

class CSStartupManager
{
public:
	static void				LoadStartupPlugin();
	static void				LoadStartupScript();
	static void				LoadStartupWorkspace();
};