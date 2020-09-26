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
				static String^	ScriptFileExtension = ".obscript";
				static String^	LogFileExtension = ".log";

				static enum class SyncDirection
				{
					None,
					FromDisk,
					ToDisk,
				};
			private:
				String^			ScriptEditorID;
				String^			DiskFilePath;
				String^			LogFilePath;
				DateTime		LastSyncAttemptTimestamp;
				SyncDirection	LastSyncAttemptDirection;
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
				property SyncDirection LastSyncAttemptDir
				{
					SyncDirection get() { return LastSyncAttemptDirection; }
				}

				SyncedScriptData(String^ ScriptEditorID, String^ WorkingDir);

				String^		ReadFileContents();
				bool		WriteFileContents(String^ Text, bool Overwrite);
				String^		ReadLog();
				bool		WriteLog(List<String^>^ Messages);
				void		RemoveLog();
			};


			ref class SyncStartEventArgs
			{
			public:
				static enum class ExistingFileHandlingOperation
				{
					Prompt = 0,
					Overwrite,
					Keep
				};

				property List<String^>^		SyncedScriptEditorIDs;
				property Dictionary<String^, ExistingFileHandlingOperation>^
											ExistingFilesOnDisk;	// key = editorID
				property Dictionary<String^, DateTime>^
											ExistingFileLastWriteTimestamps;	// key = editorID

				SyncStartEventArgs(IEnumerable<SyncedScriptData^>^ SyncedScripts,
								IEnumerable<Tuple<String^, DateTime>^>^ ExistingScripts)
				{
					SyncedScriptEditorIDs = gcnew List<String ^>;
					ExistingFilesOnDisk = gcnew Dictionary<String ^, ExistingFileHandlingOperation>;
					ExistingFileLastWriteTimestamps = gcnew Dictionary<String ^, DateTime>;

					for each (SyncedScriptData^ Data in SyncedScripts)
						this->SyncedScriptEditorIDs->Add(Data->EditorID);

					for each (auto Itr in ExistingScripts)
					{
						this->ExistingFilesOnDisk->Add(Itr->Item1,
													ExistingFileHandlingOperation::Overwrite);
						this->ExistingFileLastWriteTimestamps->Add(Itr->Item1, Itr->Item2);
					}
				}
			};

			ref class SyncWriteToDiskEventArgs
			{
			public:
				property String^	ScriptEditorID;
				property DateTime	AccessTimestamp;
				property bool		Success;

				SyncWriteToDiskEventArgs(String^ ScriptEditorID, DateTime AccessTimestamp)
				{
					this->ScriptEditorID = ScriptEditorID;
					this->AccessTimestamp = AccessTimestamp;
					this->Success = false;
				}
			};

			ref class SyncPreCompileEventArgs
			{
			public:
				property String^	ScriptEditorID;
				property String^	DiskFileContents;
				property DateTime	AccessTimestamp;

				SyncPreCompileEventArgs(String^ ScriptEditorID, String^ DiskFileContents, DateTime AccessTimestamp)
				{
					this->ScriptEditorID = ScriptEditorID;
					this->DiskFileContents = DiskFileContents;
					this->AccessTimestamp = AccessTimestamp;
				}
			};

			ref class SyncPostCompileEventArgs
			{
			public:
				property String^			ScriptEditorID;
				property bool				Success;
				property List<String^>^		OutputMessages;

				SyncPostCompileEventArgs(String^ ScriptEditorID)
				{
					this->ScriptEditorID = ScriptEditorID;
					this->Success = false;
					this->OutputMessages = gcnew List<String^>();
				}
			};

			ref class SyncStopEventArgs
			{
			public:
				property UInt32		NumSyncedScripts;
				property UInt32		NumFailedCompilations;
				property bool		RemoveLogFiles;

				SyncStopEventArgs(UInt32 NumSyncedScripts, UInt32 NumFailedCompilations)
				{
					this->NumSyncedScripts = NumSyncedScripts;
					this->NumFailedCompilations = NumFailedCompilations;
					this->RemoveLogFiles = false;
				}
			};



			delegate void SyncStartEventHandler(Object^ Sender, SyncStartEventArgs^ E);
			delegate void SyncStopEventHandler(Object^ Sender, SyncStopEventArgs^ E);
			delegate void SyncWriteToDiskEventHandler(Object^ Sender, SyncWriteToDiskEventArgs^ E);
			delegate void SyncPreCompileEventHandler(Object^ Sender, SyncPreCompileEventArgs^ E);
			delegate void SyncPostCompileEventHandler(Object^ Sender, SyncPostCompileEventArgs^ E);


			ref class DiskSync
			{
				bool								SyncInProgress;
				bool								ExecutingSyncLoop;
				String^								WorkingDir;
				List<SyncedScriptData^>^			SyncedScripts;
				Dictionary<String^, SyncedScriptData^>^
													EditorIDsToSyncedData;

				Timer^								SyncTimer;
				componentDLLInterface::CSEInterfaceTable::IEditorAPI::ConsoleContextObjectPtr
													ConsoleMessageLogContext;

				void								DoSyncLoop(bool Force, UInt32% OutFailedCompilations);
				bool								SyncToDisk(SyncedScriptData^ SyncedScript, bool Overwrite);
				bool								SyncFromDisk(SyncedScriptData^ SyncedScript);
				bool								DoPreprocessingAndAnalysis(componentDLLInterface::ScriptData* Script,
																			String^ ImportedScriptText,
																			String^% OutPreprocessedText,
																			bool% OutHasDirectives,
																			List<String^>^% OutMessages);
				void								CompileScript(SyncedScriptData^ SyncedScript,
																String^ ImportedScriptText,
																bool% OutSuccess,
																List<String^>^% OutMessages);
				void								WriteToConsoleContext(String^ Message);
				String^								FormatLogMessage(int Line, String^ Message, bool Critical);


				void								SyncTimer_Tick(Object^ Sender, EventArgs^ E);

				DiskSync();
				~DiskSync();

				static DiskSync^					Singleton = nullptr;
			public:
				event SyncStartEventHandler^		SyncStart;
				event SyncStopEventHandler^			SyncStop;
				event SyncWriteToDiskEventHandler^	ScriptWriteToDisk;
				event SyncPreCompileEventHandler^	ScriptPreCompile;
				event SyncPostCompileEventHandler^	ScriptPostCompile;


				void		Start(String^ WorkingDir, List<String^>^ SyncedScriptEditorIDs);
				void		Stop();
				bool		IsScriptBeingSynced(String^ ScriptEditorID);
				String^		GetSyncLogContents(String^ ScriptEditorID);

				void		ForceSyncToDisk(String^ ScriptEditorID);
				void		ForceSyncFromDisk(String^ ScriptEditorID);

				void		OpenLogFile(String^ ScriptEditorID);
				void		OpenScriptFile(String^ ScriptEditorID);


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


				static DiskSync^					Get();
			};

			ref struct SyncedScriptListViewWrapper
			{
				static enum class SuccessState
				{
					None = 0,
					Indeterminate,
					Success,
					Failure
				};

				property String^		EditorID;
				property DateTime		LastSyncTime;
				property SuccessState	LastSyncSuccess;

				SyncedScriptListViewWrapper(String^ EditorID)
				{
					this->EditorID = EditorID;
					Reset();
				}

				void Reset()
				{
					LastSyncTime = DateTime();
					LastSyncSuccess = SuccessState::None;
				}
			};

			ref class DiskSyncDialog : public System::Windows::Forms::Form
			{
				static DiskSyncDialog^		Singleton = nullptr;


				System::ComponentModel::Container^ components;

				Label^						LabelWorkingDir;
				FolderBrowserDialog^		FolderDlgWorkingDir;
				TextBox^					TextBoxWorkingDir;
				Button^						ButtonSelectWorkingDir;

				GroupBox^					GroupSyncSettings;
				CheckBox^					CheckboxAutoSync;
				Label^						LabelSeconds;
				NumericUpDown^				NumericAutoSyncSeconds;
				GroupBox^					GroupStartupFileHandling;
				RadioButton^				RadioPromptForFileHandling;
				RadioButton^				RadioUseExistingFiles;
				RadioButton^				RadioOverwriteExistingFiles;
				Label^						LabelScriptsToSync;
				Button^						ButtonSelectScripts;
				Button^						ButtonStartStopSync;
				CheckBox^					CheckboxAutoDeleteLogs;
				BrightIdeasSoftware::FastObjectListView^
											ListViewSyncedScripts;
				BrightIdeasSoftware::OLVColumn^
											ColScriptName;
				BrightIdeasSoftware::OLVColumn^
											ColLastSyncTime;
				System::Windows::Forms::ContextMenuStrip^
											LVSyncedStripsContextMenu;
				ToolStripMenuItem^			SyncToDiskToolStripMenuItem;
				ToolStripMenuItem^			SyncFromDiskToolStripMenuItem;
				ToolStripSeparator^			ToolStripSeparator1;
				ToolStripMenuItem^			OpenLogToolStripMenuItem;
				ToolStripMenuItem^			OpenSyncedFileToolStripMenuItem;
				Button^						ButtonOpenWorkingDir;
				TextBox^					TextBoxSelectedScriptLog;
				Label^						LabelSelectedScriptLog;

				SyncStartEventHandler^			DiskSyncSyncStartHandler;
				SyncStopEventHandler^			DiskSyncSyncStopHandler;
				SyncWriteToDiskEventHandler^	DiskSyncSyncWriteToDiskHandler;
				SyncPreCompileEventHandler^		DiskSyncSyncPreCompileHandler;
				SyncPostCompileEventHandler^	DiskSyncSyncPostCompileHandler;

				Dictionary<String^, SyncedScriptListViewWrapper^>^
											SyncedScripts;

				void						InitializeComponent();

				void						ButtonSelectWorkingDir_Click(Object^ Sender, EventArgs^ E);
				void						ButtonOpenWorkingDir_Click(Object^ Sender, EventArgs^ E);
				void						ButtonSelectScripts_Click(Object^ Sender, EventArgs^ E);
				void						ButtonStartStopSync_Click(Object^ Sender, EventArgs^ E);
				void						SyncToDiskToolStripMenuItem_Click(Object^ Sender, EventArgs^ E);
				void						SyncFromDiskToolStripMenuItem_Click(Object^ Sender, EventArgs^ E);
				void						OpenLogToolStripMenuItem_Click(Object^ Sender, EventArgs^ E);
				void						OpenSyncedFileToolStripMenuItem_Click(Object^ Sender, EventArgs^ E);
				void						CheckboxAutoSync_Click(Object^ Sender, EventArgs^ E);
				void						NumericAutoSyncSeconds_ValueChanged(Object^ Sender, EventArgs^ E);
				void						ListViewSyncedScripts_CellRightClick(Object^ Sender, BrightIdeasSoftware::CellRightClickEventArgs^ E);
				void						ListViewSyncedScripts_SelectedIndexChanged(Object^ Sender, EventArgs^ E);

				void						Dialog_Cancel(Object^ Sender, CancelEventArgs^ E);

				void						DiskSync_SyncStart(Object^ Sender, SyncStartEventArgs^ E);
				void						DiskSync_SyncStop(Object^ Sender, SyncStopEventArgs^ E);
				void						DiskSync_SyncWriteToDisk(Object^ Sender, SyncWriteToDiskEventArgs^ E);
				void						DiskSync_SyncPreCompile(Object^ Sender, SyncPreCompileEventArgs^ E);
				void						DiskSync_SyncPostCompile(Object^ Sender, SyncPostCompileEventArgs^ E);

				bool						IsSyncInProgress();

				static Object^				ListViewAspectScriptNameGetter(Object^ RowObject);
				static Object^				ListViewImageScriptNameGetter(Object^ RowObject);
				static Object^				ListViewAspectLastSyncTimeGetter(Object^ RowObject);
				static String^				ListViewAspectToStringLastSyncTime(Object^ RowObject);

				DiskSyncDialog();
				~DiskSyncDialog();
			public:
				static void					Show();
				static void					Close();
			};
		}
	};
}
