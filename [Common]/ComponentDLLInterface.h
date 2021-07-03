#pragma once
#include "HandShakeStructs.h"

// Internal interfaces exported by the DLLs for interop
// Main function exported by all the DLLs
#define QUERYINTERFACE_EXPORT						__declspec(dllexport) void* QueryInterface(void)
#define QUERYINTERFACE_FWD							typedef void* (* QueryInterface)(void)

namespace componentDLLInterface
{
	QUERYINTERFACE_FWD;

	// pointers returned by an interface are expected to be released by the caller
	// pointers passed to an interface are expected to be released by the caller
	class CSEInterfaceTable
	{
	public:
		class IEditorAPI
		{
		public:
			typedef void* ConsoleContextObjectPtr;

			void									(* DebugPrint)(UInt8 Source, const char* Message);
			void									(* WriteToStatusBar)(int PanelIndex, const char* Message);
			const char*								(* GetAppPath)(void);
			HWND									(* GetMainWindowHandle)(void);
			HWND									(* GetRenderWindowHandle)(void);

			FormData*								(* LookupFormByEditorID)(const char* EditorID);
			ScriptData*								(* LookupScriptableFormByEditorID)(const char* EditorID);		// returns the form's script data
			const char*								(* GetFormTypeIDLongName)(UInt8 TypeID);

			void									(* LoadFormForEditByEditorID)(const char* EditorID);
			void									(* LoadFormForEditByFormID)(UInt32 FormID);
			FormData*								(* ShowPickReferenceDialog)(HWND Parent);
			void									(* ShowUseReportDialog)(const char* EditorID);

			void									(* SaveActivePlugin)(void);

			void									(* ReadFromINI)(const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size);
			void									(* WriteToINI)(const char* Setting, const char* Section, const char* Value);

			UInt32									(* GetFormListActiveItemForegroundColor)(void);
			UInt32									(* GetFormListActiveItemBackgroundColor)(void);
			bool									(* GetShouldColorizeActiveForms)(void);
			bool									(* GetShouldSortActiveFormsFirst)(void);

			ConsoleContextObjectPtr					(* RegisterConsoleContext)(const char* Name);
			void									(* DeregisterConsoleContext)(ConsoleContextObjectPtr ContextObject);
			void									(* PrintToConsoleContext)(ConsoleContextObjectPtr ContextObject, const char* Message, bool PrintTimestamp);
		};

		class IScriptEditor
		{
		public:
			ScriptData*								(* CreateNewScript)(void);
			void									(* DestroyScriptInstance)(void* CurrentScript);
			bool									(* IsUnsavedNewScript)(void* CurrentScript);
			bool									(* CompileScript)(ScriptCompileData* Data);
			void									(* RecompileAllScriptsInActiveFile)(void);
			void									(* ToggleScriptCompilation)(bool State);
			bool									(* DeleteScript)(const char* EditorID);
			ScriptData*								(* GetPreviousScriptInList)(void* CurrentScript);
			ScriptData*								(* GetNextScriptInList)(void* CurrentScript);
			void									(* RemoveScriptBytecode)(void* Script);

			void									(* SaveEditorBoundsToINI)(UInt32 Left, UInt32 Top, UInt32 Width, UInt32 Height);
			ScriptListData*							(* GetScriptList)(void);
			ScriptVarListData*						(* GetScriptVarList)(const char* EditorID);
			bool									(* UpdateScriptVarIndices)(const char* EditorID, ScriptVarListData* Data);
			void									(* CompileDependencies)(const char* EditorID);
			IntelliSenseUpdateData*					(* GetIntelliSenseUpdateData)(void);
			void									(* BindScript)(const char* EditorID, HWND Parent);
			void									(* SetScriptText)(void* CurrentScript, const char* ScriptText);
			void									(* UpdateScriptVarNames)(const char* EditorID, ScriptVarRenameData* Data);
			bool									(* CanUpdateIntelliSenseDatabase)(void);

			const char*								(* GetDefaultCachePath)(void);
			const char*								(* GetAutoRecoveryCachePath)(void);
			const char*								(* GetPreprocessorBasePath)(void);
			const char*								(* GetPreprocessorStandardPath)(void);
			const char*								(* GetSnippetCachePath)(void);

			ScriptVarRenameData*					(* AllocateVarRenameData)(UInt32 VarCount);
			ScriptCompileData*						(* AllocateCompileData)(void);
		};

		class IUseInfoList
		{
		public:
			UseInfoListFormData*					(* GetLoadedForms)(void);
			UseInfoListCrossRefData*				(* GetCrossRefDataForForm)(const char* EditorID);
			UseInfoListCellItemListData*			(* GetCellRefDataForForm)(const char* EditorID);
		};

		class ITagBrowser
		{
		public:
			void									(* InstantiateObjects)(TagBrowserInstantiationData* Data);
			void									(* InitiateDragonDrop)(void);

			TagBrowserInstantiationData*			(* AllocateInstantionData)(UInt32 FormCount);
		};

		void										(* DeleteInterOpData)(IDisposableData* Pointer);

		IEditorAPI									EditorAPI;
		IScriptEditor								ScriptEditor;
		IUseInfoList								UseInfoList;
		ITagBrowser									TagBrowser;
	};

	class ScriptEditorInterface
	{
	public:
		void										(* InitializeComponents)(CommandTableData* ScriptCommandData, IntelliSenseUpdateData* GMSTData);

		void										(* InstantiateEditor)(ScriptData* InitializerScript,
																		  UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height);
		void										(* InstantiateEditorAndHighlight)(ScriptData* InitializerScript, const char* SearchQuery,
																		  UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height);
		void										(* InstantiateEditors)(ScriptListData* InitializerScripts,
																		   UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height);
		void										(* InstantiateEditorsAndHighlight)(ScriptListData* InitializerScripts, const char* SearchQuery,
																					   UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height);

		bool										(* IsDiskSyncInProgress)(void);
		void										(* ShowDiskSyncDialog)(void);
		void										(* CloseAllOpenEditors)(void);
		void										(* UpdateIntelliSenseDatabase)(void);
		UInt32										(* GetOpenEditorCount)(void);
		bool										(* PreprocessScript)(const char* ScriptText, char* OutPreprocessed, UInt32 BufferSize);
		void										(* Deinitalize)(void);
	};

	class UseInfoListInterface
	{
	public:
		void										(* InitializeComponents)(void);

		void										(* ShowUseInfoListDialog)(const char* FilterString);
		void										(* HideUseInfoListDialog)(void);
	};

	class BSAViewerInterface
	{
	public:
		void										(* InitializeComponents)(void);

		void										(* ShowBSAViewerDialog)(const char* WorkingDir, const char* ExtensionFilter, char* ReturnPathOut, UInt32 BufferSize);
	};

	class TagBrowserInterface
	{
	public:
		void										(* InitializeComponents)(void);

		void										(* ShowTagBrowserDialog)(HWND Parent);
		void										(* HideTagBrowserDialog)(void);
		bool										(* AddFormToActiveTag)(FormData* Data);
		HWND										(* GetFormDropWindowHandle)(void);
		HWND										(* GetFormDropParentHandle)(void);
		bool										(* GetDragOpInProgress)(void);
	};
}