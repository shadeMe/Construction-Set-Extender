#pragma once
#include "HandShakeStructs.h"

// Internal interfaces exported by the DLLs for interop

#ifndef CSE
typedef void* HWND;
#endif

namespace ComponentDLLInterface
{
	// main function exported by all the dlls
	typedef void*									(* QueryInterface)(void);

	// all returned pointers are expected to be released by the caller
	class CSEInterfaceTable
	{
	public:
		class IEditorAPI
		{
		public:
			void									(* DebugPrint)(UInt8 Source, const char* Message);
			void									(* WriteToStatusBar)(int PanelIndex, const char* Message);
			const char*								(* GetAppPath)(void);
			HWND									(* GetMainWindowHandle)(void);
			HWND									(* GetRenderWindowHandle)(void);

			FormData*								(* LookupFormByEditorID)(const char* EditorID);
			ScriptData*								(* LookupScriptableFormByEditorID)(const char* EditorID);
			bool									(* GetIsFormReference)(const char* EditorID);

			void									(* LoadFormForEdit)(const char* EditorID);
			FormData*								(* ShowPickReferenceDialog)(HWND Parent);
			void									(* ShowUseReportDialog)(const char* EditorID);

			void									(* SaveActivePlugin)(void);

			void									(* ReadFromINI)(const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size);
			void									(* WriteToINI)(const char* Setting, const char* Section, const char* Value);
		};

		class IScriptEditor
		{
		public:
			ScriptData*								(* CreateNewScript)(void);
			void									(* DestroyScriptInstance)(void* CurrentScript);
			bool									(* CompileScript)(ScriptCompileData* Data);
			void									(* RecompileScripts)(void);
			void									(* ToggleScriptCompilation)(bool State);
			void									(* DeleteScript)(const char* EditorID);
			ScriptData*								(* GetPreviousScriptInList)(void* CurrentScript);
			ScriptData*								(* GetNextScriptInList)(void* CurrentScript);

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
		};

		class IUseInfoList
		{
		public:
			UseInfoListFormData*					(* GetLoadedForms)(void);
			UseInfoListCrossRefData*				(* GetCrossRefDataForForm)(const char* EditorID);
			UseInfoListCellItemListData*			(* GetCellRefDataForForm)(const char* EditorID);
		};

		class IBatchRefEditor
		{
		public:
			BatchRefOwnerFormData*					(* GetOwnershipData)(void);		// typeID check performed by the caller
		};

		class ITagBrowser
		{
		public:
			void									(* InstantiateObjects)(TagBrowserInstantiationData* Data);
		};

		void										(* DeleteNativeHeapPointer)(void* Pointer, bool IsArray);

		IEditorAPI									EditorAPI;
		IScriptEditor								ScriptEditor;
		IUseInfoList								UseInfoList;
		IBatchRefEditor								BatchRefEditor;
		ITagBrowser									TagBrowser;
	};

	class ScriptEditorInterface
	{
	public:
		void										(* DeleteManagedHeapPointer)(void* Pointer, bool IsArray);

		void										(* InitializeComponents)(CommandTableData* Data, IntelliSenseUpdateData* GMSTData);

		void										(* InstantiateEditor)(ScriptData* InitializerScript, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height);
		void										(* AddScriptCommandDeveloperURL)(const char* ScriptCommandName, const char* URL);

		void										(* CloseAllOpenEditors)(void);
		void										(* UpdateIntelliSenseDatabase)(void);
		UInt32										(* GetOpenEditorCount)(void);
		void										(* CleanupAutoRecoveryCache)(void);
	};

	class UseInfoListInterface
	{
	public:
		void										(* DeleteManagedHeapPointer)(void* Pointer, bool IsArray);

		void										(* ShowUseInfoListDialog)(const char* FilterString);
	};

	class BSAViewerInterface
	{
	public:
		void										(* DeleteManagedHeapPointer)(void* Pointer, bool IsArray);

		void										(* ShowBSAViewerDialog)(const char* WorkingDir, const char* ExtensionFilter, char* ReturnPathOut, UInt32 BufferSize);
	};

	class BatchEditorInterface
	{
	public:
		void										(* DeleteManagedHeapPointer)(void* Pointer, bool IsArray);

		bool										(* ShowBatchRefEditorDialog)(BatchRefData* Data);
	};

	class TagBrowserInterface
	{
	public:
		void										(* DeleteManagedHeapPointer)(void* Pointer, bool IsArray);

		void										(* ShowTagBrowserDialog)(HWND Parent);
		void										(* HideTagBrowserDialog)(void);
		bool										(* AddFormToActiveTag)(FormData* Data);
		HWND										(* GetFormDropWindowHandle)(void);
		HWND										(* GetFormDropParentHandle)(void);
	};
}