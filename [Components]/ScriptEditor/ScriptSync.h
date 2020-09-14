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
			private:
				String^			ScriptEditorID;
				String^			DiskFilePath;
				String^			LogFilePath;
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

				property DateTime FileLastWrite
				{
					DateTime get()
					{
						if (File::Exists(DiskFilePath) == false)
							return DateTime();

						return File::GetLastWriteTime(DiskFilePath);
					}
				}

				property DateTime FileLastSyncAttempt;

				SyncedScriptData(String^ ScriptEditorID, String^ WorkingDir);

				String^		ReadFileContents(bool UpdateSyncTimestamp);
				bool		WriteFileContents(String^ Text, bool Overwrite);
				String^		ReadLog();
				bool		WriteLog(List<String^>^ Messages);
				void		RemoveLog();
			};

			ref class SyncStartEventArgs : public EventArgs
			{
			public:
				static enum class ExistingFileHandlingOperation
				{
					Overwrite,
					Keep
				};

				property List<String^>^		SyncedScriptEditorIDs;
				property Dictionary<String^, ExistingFileHandlingOperation>^
											ExistingFilesOnDisk;	// key = editorID

				SyncStartEventArgs(IEnumerable<SyncedScriptData^>^ SyncedScripts,
								IEnumerable<String^>^ ExistingScriptEditorIDs) : EventArgs()
				{
					for each (SyncedScriptData^ Data in SyncedScripts)
						this->SyncedScriptEditorIDs->Add(Data->EditorID);

					for each (String^ ExistingScriptEditor in ExistingScriptEditorIDs)
						this->ExistingFilesOnDisk->Add(ExistingScriptEditor,
													ExistingFileHandlingOperation::Overwrite);
				}
			};

			ref class SyncPreCompileEventArgs : public EventArgs
			{
			public:
				property String^	ScriptEditorID;
				property String^	DiskFileContents;

				SyncPreCompileEventArgs(String^ ScriptEditorID, String^ DiskFileContents) : EventArgs()
				{
					this->ScriptEditorID = ScriptEditorID;
					this->DiskFileContents = DiskFileContents;
				}
			};

			ref class SyncPostCompileEventArgs : public EventArgs
			{
			public:
				property String^			ScriptEditorID;
				property bool				Success;
				property List<String^>^		OutputMessages;

				SyncPostCompileEventArgs(String^ ScriptEditorID) : EventArgs()
				{
					this->ScriptEditorID = ScriptEditorID;
					this->Success = false;
					this->OutputMessages = gcnew List<String^>();
				}
			};

			ref class SyncStopEventArgs : public EventArgs
			{
			public:
				property UInt32		NumSyncedScripts;
				property UInt32		NumFailedCompilations;
				property bool		RemoveLogFiles;

				SyncStopEventArgs(UInt32 NumSyncedScripts, UInt32 NumFailedCompilations) : EventArgs()
				{
					this->NumSyncedScripts = NumSyncedScripts;
					this->NumFailedCompilations = NumFailedCompilations;
					this->RemoveLogFiles = false;
				}
			};



			delegate void SyncStartEventHandler(Object^ Sender, SyncStartEventArgs^ E);
			delegate void SyncStopEventHandler(Object^ Sender, SyncStopEventArgs^ E);
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

				void								DoSync(bool Force, UInt32% OutFailedCompilations);
				bool								ExportScriptToFile(SyncedScriptData^ SyncedScript, bool Overwrite);
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
				event SyncPreCompileEventHandler^	ScriptPreCompile;
				event SyncPostCompileEventHandler^	ScriptPostCompile;


				void		Start(String^ WorkingDir, List<String^>^ SyncedScriptEditorIDs);
				void		Stop();
				bool		IsScriptBeingSynced(String^ ScriptEditorID);
				String^		GetSyncLogContents(String^ ScriptEditorID);

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

			ref class DiskSyncDialog : public System::Windows::Forms::Form
			{
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
				TextBox^					TextBoxSelectedScriptLog;
				Label^						LabelSelectedScriptLog;

				void						InitializeComponent();

				/*void						ButtonSelectWorkingDir_Click(Object^ Sender, EventArgs^ E);
				void						ButtonSelectScripts_Click(Object^ Sender, EventArgs^ E);
				void						ButtonStartStopSync_Click(Object^ Sender, EventArgs^ E);
				void						SyncToDiskToolStripMenuItem_Click(Object^ Sender, EventArgs^ E);
				void						SyncFromDiskToolStripMenuItem_Click(Object^ Sender, EventArgs^ E);
				void						CheckboxAutoSync_Click(Object^ Sender, EventArgs^ E);
				void						NumericAutoSyncSeconds_ValueChanged(Object^ Sender, EventArgs^ E);

				void						Dialog_Cancel(Object^ Sender, CancelEventArgs^ E);

				void						SyncStartEventHandler(Object^ Sender, SyncStartEventArgs^ E);
				void						SyncStopEventHandler(Object^ Sender, SyncStopEventArgs^ E);
				void						SyncPreCompileEventHandler(Object^ Sender, SyncPreCompileEventArgs^ E);
				void						SyncPostCompileEventHandler(Object^ Sender, SyncPostCompileEventArgs^ E);

				void						StartSync();
				void						StopSync();

				static Object^				ListViewAspectScriptNameGetter(Object^ RowObject);
				static Object^				ListViewImageScriptNameGetter(Object^ RowObject);
				static Object^				ListViewAspectLastSyncTimeGetter(Object^ RowObject);
				static Object^				ListViewAspectToStringLastSyncTime(Object^ RowObject);*/
			public:
				DiskSyncDialog();
				~DiskSyncDialog();
			};
		}
	};
}
