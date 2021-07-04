#pragma once

namespace cse
{


namespace scriptEditor
{


namespace scriptSync
{


ref class SyncedScriptData
{
public:
	static String^ DefaultScriptFileExtension = ".obscript";
	static String^ LogFileExtension = ".log";

	static enum class eSyncDirection
	{
		None,
		FromDisk,
		ToDisk,
	};
private:
	String^ ScriptEditorID;
	String^ DiskFilePath;
	String^ LogFilePath;
	DateTime LastSyncAttemptTimestamp;
	eSyncDirection LastSyncAttemptDirection;
public:
	property String^ EditorID
	{
		String^ get() { return ScriptEditorID; }
	}

	property String^ FilePath
	{
		String^ get() { return DiskFilePath; }
	}

	property String^ LogPath
	{
		String^ get() { return LogFilePath; }
	}

	property DateTime LastFileWriteTime
	{
		DateTime get()
		{
			if (File::Exists(DiskFilePath) == false)
				return DateTime();

			return File::GetLastWriteTime(DiskFilePath);
		}
	}

	property DateTime LastSyncAttemptTime
	{
		DateTime get() { return LastSyncAttemptTimestamp; }
	}
	property eSyncDirection LastSyncAttemptDir
	{
		eSyncDirection get() { return LastSyncAttemptDirection; }
	}

	SyncedScriptData(String^ ScriptEditorID, String^ WorkingDir);

	String^ ReadFileContents();
	bool WriteFileContents(String^ Text, bool Overwrite);
	String^ ReadLog();
	bool WriteLog(List<String^>^ Messages);
	void RemoveLog();
};


ref class SyncStartEventArgs
{
public:
	static enum class eExistingFileHandlingOperation
	{
		Prompt = 0,
		Overwrite,
		Keep
	};

	property List<String^>^ SyncedScriptEditorIDs;
	property Dictionary<String^, eExistingFileHandlingOperation>^ ExistingFilesOnDisk;	// key = editorID
	property Dictionary<String^, DateTime>^ ExistingFileLastWriteTimestamps;	// key = editorID

	SyncStartEventArgs(IEnumerable<SyncedScriptData^>^ SyncedScripts, IEnumerable<Tuple<String^, DateTime>^>^ ExistingScripts);
};

ref class SyncWriteToDiskEventArgs
{
public:
	property String^ ScriptEditorID;
	property DateTime AccessTimestamp;
	property bool Success;

	SyncWriteToDiskEventArgs(String^ ScriptEditorID, DateTime AccessTimestamp);
};

ref class SyncPreCompileEventArgs
{
public:
	property String^ ScriptEditorID;
	property String^ DiskFileContents;
	property DateTime AccessTimestamp;

	SyncPreCompileEventArgs(String^ ScriptEditorID, String^ DiskFileContents, DateTime AccessTimestamp);
};

ref class SyncPostCompileEventArgs
{
public:
	property String^ ScriptEditorID;
	property bool Success;
	property List<String^>^ OutputMessages;

	SyncPostCompileEventArgs(String^ ScriptEditorID);
};
;
ref class SyncStopEventArgs
{
public:
	property UInt32		NumSyncedScripts;
	property UInt32		NumFailedCompilations;
	property bool		RemoveLogFiles;

	SyncStopEventArgs(UInt32 NumSyncedScripts, UInt32 NumFailedCompilations);
};


delegate void SyncStartEventHandler(Object^ Sender, SyncStartEventArgs^ E);
delegate void SyncStopEventHandler(Object^ Sender, SyncStopEventArgs^ E);
delegate void SyncWriteToDiskEventHandler(Object^ Sender, SyncWriteToDiskEventArgs^ E);
delegate void SyncPreCompileEventHandler(Object^ Sender, SyncPreCompileEventArgs^ E);
delegate void SyncPostCompileEventHandler(Object^ Sender, SyncPostCompileEventArgs^ E);


ref class DiskSync
{
	bool SyncInProgress;
	bool ExecutingSyncLoop;
	String^ WorkingDir;
	List<SyncedScriptData^>^ SyncedScripts;
	Dictionary<String^, SyncedScriptData^>^ EditorIDsToSyncedData;

	Timer^ SyncTimer;
	componentDLLInterface::CSEInterfaceTable::IEditorAPI::ConsoleContextObjectPtr ConsoleMessageLogContext;

	void DoSyncFromDiskLoop(bool Force, UInt32% OutFailedCompilations);
	bool SyncToDisk(SyncedScriptData^ SyncedScript, bool Overwrite);
	bool SyncFromDisk(SyncedScriptData^ SyncedScript);
	bool DoPreprocessingAndAnalysis(componentDLLInterface::ScriptData* Script,
									String^ ImportedScriptText,
									String^% OutPreprocessedText,
									bool% OutHasDirectives,
									List<String^>^% OutMessages);
	void CompileScript(SyncedScriptData^ SyncedScript,
					   String^ ImportedScriptText,
					   bool% OutSuccess,
					   List<String^>^% OutMessages);
	void WriteToConsoleContext(String^ Message);
	String^ FormatLogMessage(int Line, String^ Message, bool Critical);


	void SyncTimer_Tick(Object^ Sender, EventArgs^ E);

	DiskSync();
	~DiskSync();

	static DiskSync^ Singleton = nullptr;
public:
	event SyncStartEventHandler^ SyncStart;
	event SyncStopEventHandler^ SyncStop;
	event SyncWriteToDiskEventHandler^ ScriptWriteToDisk;
	event SyncPreCompileEventHandler^ ScriptPreCompile;
	event SyncPostCompileEventHandler^ ScriptPostCompile;

	bool Start(String^ WorkingDir, List<String^>^ SyncedScriptEditorIDs);
	void Stop();
	bool IsScriptBeingSynced(String^ ScriptEditorID);
	String^ GetSyncLogContents(String^ ScriptEditorID);

	void ForceSyncToDisk(String^ ScriptEditorID);
	void ForceSyncFromDisk(String^ ScriptEditorID);

	void OpenLogFile(String^ ScriptEditorID);
	void OpenScriptFile(String^ ScriptEditorID);

	property bool AutomaticSync
	{
		void set(bool val) { SyncTimer->Enabled = val; }
	}
	property UInt32 AutomaticSyncIntervalSeconds
	{
		void set(UInt32 val)
		{
			if (val < 1)
				val = 1;
			else if (val > 120)
				val = 120;

			SyncTimer->Interval = val * 1000;
		}
	}
	property bool InProgress
	{
		bool get() { return SyncInProgress; }
	}
	property String^ WorkingDirectory
	{
		String^ get() { return WorkingDir; }
	}


	static DiskSync^ Get();
};

ref struct SyncedScriptListViewWrapper
{
	static enum class eSuccessState
	{
		None = 0,
		Indeterminate,
		Success,
		Failure
	};

	property String^		EditorID;
	property DateTime		LastSyncTime;
	property eSuccessState	LastSyncSuccess;
	property List<String^>^	LastOutputMessages;

	SyncedScriptListViewWrapper(String^ EditorID);

	void Reset();
};

ref class DiskSyncDialog : public DevComponents::DotNetBar::Metro::MetroForm
{
	static DiskSyncDialog^ Singleton = nullptr;

	DevComponents::DotNetBar::ButtonX^ ButtonOpenWorkingDir;
	BrightIdeasSoftware::OLVColumn^ ColLastSyncTime;
	BrightIdeasSoftware::OLVColumn^ ColScriptName;
	DevComponents::DotNetBar::LabelX^ LabelSelectedScriptLog;
	BrightIdeasSoftware::FastObjectListView^ ListViewSyncedScripts;
	DevComponents::DotNetBar::ButtonX^ ButtonStartStopSync;
	DevComponents::DotNetBar::ButtonX^ ButtonSelectWorkingDir;
	System::Windows::Forms::FolderBrowserDialog^ FolderDlgWorkingDir;
	DevComponents::DotNetBar::LabelX^ LabelWorkingDir;
	DevComponents::DotNetBar::Controls::GroupPanel^ GroupSyncSettings;
	DevComponents::DotNetBar::Controls::GroupPanel^ GroupStartupFileHandling;
	System::Windows::Forms::RadioButton^ RadioPromptForFileHandling;
	System::Windows::Forms::RadioButton^ RadioUseExistingFiles;
	System::Windows::Forms::RadioButton^ RadioOverwriteExistingFiles;
	System::Windows::Forms::CheckBox^ CheckboxAutoDeleteLogs;
	DevComponents::DotNetBar::LabelX^ LabelSeconds;
	System::Windows::Forms::CheckBox^ CheckboxAutoSync;
	DevComponents::Editors::IntegerInput^ NumericAutoSyncSeconds;
	DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxSelectedScriptLog;
	DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxWorkingDir;
	DevComponents::DotNetBar::PanelEx^ LeftPanel;
	DevComponents::DotNetBar::Bar^ Toolbar;
	DevComponents::DotNetBar::ButtonItem^ ToolbarSyncToDisk;
	DevComponents::DotNetBar::LabelItem^ ToolbarLabelSyncedScripts;
	DevComponents::DotNetBar::ButtonItem^ ToolbarSelectScripts;
	DevComponents::DotNetBar::ButtonItem^ ToolbarSyncFromDisk;
	DevComponents::DotNetBar::ButtonItem^ ToolbarOpenLog;
	DevComponents::DotNetBar::ButtonItem^ ToolbarOpenSyncedFile;
	System::Windows::Forms::Timer^ DeferredSelectionUpdateTimer;
	DevComponents::DotNetBar::StyleManagerAmbient^ ColorManager;
	System::ComponentModel::IContainer^ components;

	SyncStartEventHandler^ DiskSyncSyncStartHandler;
	SyncStopEventHandler^ DiskSyncSyncStopHandler;
	SyncWriteToDiskEventHandler^ DiskSyncSyncWriteToDiskHandler;
	SyncPreCompileEventHandler^ DiskSyncSyncPreCompileHandler;
	SyncPostCompileEventHandler^ DiskSyncSyncPostCompileHandler;

	Dictionary<String^, SyncedScriptListViewWrapper^>^ SyncedScripts;

	void InitializeComponent();
	void FinalizeComponents();

	void ButtonSelectWorkingDir_Click(Object^ Sender, EventArgs^ E);
	void ButtonOpenWorkingDir_Click(Object^ Sender, EventArgs^ E);
	void ButtonStartStopSync_Click(Object^ Sender, EventArgs^ E);

	void ToolbarSelectScripts_Click(Object^ Sender, EventArgs^ E);
	void ToolbarSyncToDisk_Click(Object^ Sender, EventArgs^ E);
	void ToolbarSyncFromDisk_Click(Object^ Sender, EventArgs^ E);
	void ToolbarOpenLog_Click(Object^ Sender, EventArgs^ E);
	void ToolbarOpenSyncedFile_Click(Object^ Sender, EventArgs^ E);

	void CheckboxAutoSync_Click(Object^ Sender, EventArgs^ E);
	void NumericAutoSyncSeconds_ValueChanged(Object^ Sender, EventArgs^ E);
	void ListViewSyncedScripts_SelectionChanged(Object^ Sender, EventArgs^ E);
	void DeferredSelectionUpdateTimer_Tick(Object^ Sender, EventArgs^ E);

	void Dialog_Cancel(Object^ Sender, CancelEventArgs^ E);

	void DiskSync_SyncStart(Object^ Sender, SyncStartEventArgs^ E);
	void DiskSync_SyncStop(Object^ Sender, SyncStopEventArgs^ E);
	void DiskSync_SyncWriteToDisk(Object^ Sender, SyncWriteToDiskEventArgs^ E);
	void DiskSync_SyncPreCompile(Object^ Sender, SyncPreCompileEventArgs^ E);
	void DiskSync_SyncPostCompile(Object^ Sender, SyncPostCompileEventArgs^ E);

	bool IsSyncInProgress();
	String^ GetOutputMessagesForScript(String^ EditorID);
	void UpdateToolbarEnabledState();

	static Object^ ListViewAspectScriptNameGetter(Object^ RowObject);
	static Object^ ListViewImageScriptNameGetter(Object^ RowObject);
	static Object^ ListViewAspectLastSyncTimeGetter(Object^ RowObject);
	static String^ ListViewAspectToStringLastSyncTime(Object^ RowObject);

	DiskSyncDialog();
	~DiskSyncDialog();
public:
	static void Show();
	static void Close();
};


} // namespace scriptSync


} // namespace scriptEditor


} // namespace cse
