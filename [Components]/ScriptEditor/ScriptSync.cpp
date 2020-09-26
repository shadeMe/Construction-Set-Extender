#include "ScriptSync.h"
#include "ScriptTextEditorInterface.h"
#include "SemanticAnalysis.h"
#include "[Common]\NativeWrapper.h"
#include "Preferences.h"
#include "IntelliSenseDatabase.h"
#include "WorkspaceModelComponents.h"
#include "SelectScriptDialog.h"
#include "Globals.h"
#include "Preferences.h"

namespace cse
{
	namespace scriptEditor
	{
		namespace scriptSync
		{
			SyncedScriptData::SyncedScriptData(String^ ScriptEditorID, String^ WorkingDir)
			{
				this->ScriptEditorID = ScriptEditorID;
				this->DiskFilePath = Path::Combine(WorkingDir, ScriptEditorID + ScriptFileExtension);
				this->LogFilePath = Path::Combine(WorkingDir, ScriptEditorID + LogFileExtension);
				this->LastSyncAttemptTimestamp = DateTime::Now;
				this->LastSyncAttemptDirection = SyncDirection::None;
			}

			System::String^ SyncedScriptData::ReadFileContents()
			{
				LastSyncAttemptTimestamp = DateTime::Now;
				LastSyncAttemptDirection = SyncDirection::FromDisk;

				String^ Out = nullptr;
				if (File::Exists(DiskFilePath))
				{
					try { Out = File::ReadAllText(DiskFilePath); }
					catch (Exception^ E) {
						DebugPrint("Couldn't read from script sync file @ " + DiskFilePath + "! Exception: " + E->ToString(), true);
					}
				}

				return Out;
			}

			bool SyncedScriptData::WriteFileContents(String^ Text, bool Overwrite)
			{
				LastSyncAttemptTimestamp = DateTime::Now;
				LastSyncAttemptDirection = SyncDirection::ToDisk;

				if (File::Exists(DiskFilePath) && Overwrite == false)
					return true;

				try
				{
					File::WriteAllText(DiskFilePath, Text);
				}
				catch (Exception^ E) {
					DebugPrint("Couldn't write to script sync file @ " + DiskFilePath + "! Exception: " + E->ToString(), true);
					return false;
				}

				return true;
			}

			System::String^ SyncedScriptData::ReadLog()
			{
				String^ Out = nullptr;
				if (File::Exists(LogFilePath))
				{
					try { Out = File::ReadAllText(LogFilePath); }
					catch (Exception^ E) {
						DebugPrint("Couldn't read from script sync log @ " + LogFilePath + "! Exception: " + E->ToString(), true);
					}
				}
				else
					Out = "";

				return Out;
			}

			bool SyncedScriptData::WriteLog(List<String^>^ Messages)
			{
				try { File::WriteAllLines(LogFilePath, Messages); }
				catch (Exception^ E) {
					DebugPrint("Couldn't write to script sync log @ " + LogFilePath + "! Exception: " + E->ToString(), true);
					return false;
				}

				return true;
			}

			void SyncedScriptData::RemoveLog()
			{
				if (!File::Exists(LogFilePath))
					return;

				try { File::Delete(LogFilePath); }
				catch (Exception^ E) {
					DebugPrint("Couldn't delete script sync log @ " + LogFilePath + "! Exception: " + E->ToString(), true);
				}
			}

			void DiskSync::DoSyncLoop(bool Force, UInt32% OutFailedCompilations)
			{
				Debug::Assert(SyncInProgress == true);

				if (ExecutingSyncLoop)
					return;

				ExecutingSyncLoop = true;
				{
					WriteToConsoleContext("Syncing changes from " + WorkingDir + "...");
					Stopwatch^ CompileTimer = gcnew Stopwatch;
					OutFailedCompilations = 0;

					for each (SyncedScriptData ^ Data in SyncedScripts)
					{
						SyncToDisk(Data, false);
						if (Force || Data->LastFileWriteTime > Data->LastSyncAttemptTime)
						{
							WriteToConsoleContext("Compiling script '" + Data->EditorID + " @ " + Data->FilePath);
							if (!SyncFromDisk(Data))
								OutFailedCompilations += 1;
						}
					}

					WriteToConsoleContext(gcnew String('-', 50));
				}
				ExecutingSyncLoop = false;
			}


			bool DiskSync::SyncToDisk(SyncedScriptData^ SyncedScript, bool Overwrite)
			{
				auto EventArgs = gcnew SyncWriteToDiskEventArgs(SyncedScript->EditorID, DateTime::Now);
				if (File::Exists(SyncedScript->FilePath) && Overwrite == false)
				{
					EventArgs->Success = true;
					ScriptWriteToDisk(this, EventArgs);
					return true;
				}

				CString EID(SyncedScript->EditorID);
				DisposibleDataAutoPtr<componentDLLInterface::ScriptData> NativeScript
				(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str()));
				Debug::Assert(NativeScript);

				ScriptTextMetadata^ EmbeddedMetadata = gcnew ScriptTextMetadata();
				String^ ScriptText = "";

				ScriptTextMetadataHelper::DeserializeRawScriptText(gcnew String(NativeScript->Text),
					ScriptText, EmbeddedMetadata);

				EventArgs->Success = SyncedScript->WriteFileContents(ScriptText, Overwrite);
				EventArgs->AccessTimestamp = SyncedScript->LastSyncAttemptTime;
				ScriptWriteToDisk(this, EventArgs);

				return EventArgs->Success;
			}

			bool DiskSync::SyncFromDisk(SyncedScriptData^ SyncedScript)
			{
				bool Failed = false;

				String^ DiskFileContents = SyncedScript->ReadFileContents();
				if (DiskFileContents == nullptr)
				{
					WriteToConsoleContext("\tError reading from disk file");
					Failed = true;
					return Failed;
				}

				SyncPreCompileEventArgs^ PrecompileArgs = gcnew SyncPreCompileEventArgs(SyncedScript->EditorID, DiskFileContents, SyncedScript->LastSyncAttemptTime);
				ScriptPreCompile(this, PrecompileArgs);

				SyncPostCompileEventArgs^ PostcompileArgs = gcnew SyncPostCompileEventArgs(SyncedScript->EditorID);
				bool CompileSuccess = false;
				List<String^>^ CompileMessages = gcnew List<String^>();

				Stopwatch^ CompileTimer = gcnew Stopwatch;
				CompileTimer->Restart();
				CompileScript(SyncedScript, DiskFileContents, CompileSuccess, CompileMessages);
				CompileTimer->Stop();

				if (CompileSuccess)
					WriteToConsoleContext("\tCompilation succeeded! Messages:");
				else
				{
					WriteToConsoleContext("\tCompilation failed! Messages:");
					Failed = true;
				}

				for each (String ^ Message in CompileMessages)
					WriteToConsoleContext("\t\t" + Message);

				if (CompileSuccess)
				{
					List<String^>^ LogMessages = gcnew List<String^>;
					LogMessages->Add(FormatLogMessage(1, "Compiled successfully at " + DateTime::Now.ToString(), false));
					LogMessages->Add(FormatLogMessage(1, "Elapsed Time: " + CompileTimer->Elapsed.Milliseconds + " milliseconds", false));
					LogMessages->AddRange(CompileMessages);

					CompileMessages = LogMessages;
				}

				if (SyncedScript->WriteLog(CompileMessages) == false)
					WriteToConsoleContext("\tError writing to log file");

				PostcompileArgs->Success = CompileSuccess;
				PostcompileArgs->OutputMessages = CompileMessages;
				ScriptPostCompile(this, PostcompileArgs);

				return Failed;
			}

			ref struct PreprocessorErrorCapture
			{
				List<Tuple<int, String^>^>^ Errors;

				PreprocessorErrorCapture() : Errors(gcnew List<Tuple<int, String^>^>()) {}

				void OnError(int Line, String^ Message)
				{
					Errors->Add(Tuple::Create(Line, Message));
				}

				scriptPreprocessor::StandardOutputError^ Delegate()
				{
					return gcnew scriptPreprocessor::StandardOutputError(this, &PreprocessorErrorCapture::OnError);
				}
			};

			bool DiskSync::DoPreprocessingAndAnalysis(componentDLLInterface::ScriptData* Script,
													String^ ImportedScriptText,
													String^% OutPreprocessedText,
													bool% OutHasDirectives,
													List<String^>^% OutMessages)
			{
				PreprocessorErrorCapture^ PreprocessorErrors = gcnew PreprocessorErrorCapture();

				ScriptEditorPreprocessorData^ PreprocessorData = gcnew ScriptEditorPreprocessorData(
					gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
					gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
					preferences::SettingsHolder::Get()->Preprocessor->AllowMacroRedefs,
					preferences::SettingsHolder::Get()->Preprocessor->NumPasses);


				bool PreprocessingSuccessful = Preprocessor::GetSingleton()->PreprocessScript(
											ImportedScriptText, OutPreprocessedText, PreprocessorErrors->Delegate(), PreprocessorData);

				if (PreprocessingSuccessful == false)
				{
					for each (Tuple<int, String^>^ Itr in PreprocessorErrors->Errors)
						OutMessages->Add(FormatLogMessage(Itr->Item1, Itr->Item2, true));

					return false;
				}

				OutHasDirectives = PreprocessorData->ContainsDirectives;

				auto AnalysisParams = gcnew obScriptParsing::AnalysisData::Params;
				AnalysisParams->Ops = obScriptParsing::AnalysisData::Operation::FillVariables
									| obScriptParsing::AnalysisData::Operation::FillControlBlocks
									| obScriptParsing::AnalysisData::Operation::PerformBasicValidation;

				if (preferences::SettingsHolder::Get()->Validator->CheckVarCommandNameCollisions)
					AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::CheckVariableNameCommandCollisions;
				if (preferences::SettingsHolder::Get()->Validator->CheckVarFormNameCollisions)
					AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::CheckVariableNameFormCollisions;
				if (preferences::SettingsHolder::Get()->Validator->CountVariableRefs)
					AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::CountVariableReferences;
				if (preferences::SettingsHolder::Get()->Validator->NoQuestVariableRefCounting)
					AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::SuppressQuestVariableRefCount;

				AnalysisParams->Type = obScriptParsing::ScriptType::Object;
				if (Script->Type == componentDLLInterface::ScriptData::kScriptType_Magic)
					AnalysisParams->Type = obScriptParsing::ScriptType::MagicEffect;
				else if (Script->Type == componentDLLInterface::ScriptData::kScriptType_Quest)
					AnalysisParams->Type = obScriptParsing::ScriptType::Quest;

				AnalysisParams->ScriptText = OutPreprocessedText;
				AnalysisParams->ScriptCommandIdentifiers = intellisense::IntelliSenseBackend::Get()->CreateIndentifierSnapshot(
					intellisense::DatabaseLookupFilter::Command);

				auto Filter = intellisense::DatabaseLookupFilter::All & ~intellisense::DatabaseLookupFilter::Snippet;
				Filter = Filter & ~intellisense::DatabaseLookupFilter::Command;
				AnalysisParams->FormIdentifiers = intellisense::IntelliSenseBackend::Get()->CreateIndentifierSnapshot(Filter);

				obScriptParsing::AnalysisData^ AnalysisResults = gcnew obScriptParsing::AnalysisData();
				AnalysisResults->PerformAnalysis(AnalysisParams);

				for each (obScriptParsing::AnalysisData::UserMessage^ Msg in AnalysisResults->AnalysisMessages)
					OutMessages->Add(FormatLogMessage(Msg->Line, Msg->Message, Msg->Critical));

				if (AnalysisResults->HasCriticalMessages || AnalysisResults->MalformedStructure)
					return false;

				String^ ScriptOrgEID = gcnew String(Script->EditorID);
				if (ScriptOrgEID->Equals(AnalysisResults->Name, StringComparison::CurrentCultureIgnoreCase) == false)
				{
					OutMessages->Add(FormatLogMessage(1,
													"Script names can only be changed inside the script editor. Original name: " + ScriptOrgEID,
													true));
					return false;
				}

				return true;

			}

			void DiskSync::CompileScript(SyncedScriptData^ SyncedScript, String^ ImportedScriptText, bool% OutSuccess, List<String^>^% OutMessages)
			{
				CString EID(SyncedScript->EditorID);
				DisposibleDataAutoPtr<componentDLLInterface::ScriptData> NativeScript
									(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str()));
				Debug::Assert(NativeScript);

				String^ PreprocessedScriptText = "";
				bool ContainsPreprocessorDirectives = false;
				if (DoPreprocessingAndAnalysis(NativeScript.get(),
												ImportedScriptText,
												PreprocessedScriptText,
												ContainsPreprocessorDirectives,
												OutMessages) == false)
					return;

				textEditors::CompilationData^ CompilationData = gcnew textEditors::CompilationData;
				CompilationData->PreprocessedScriptText = PreprocessedScriptText;
				CompilationData->UnpreprocessedScriptText = ImportedScriptText;
				CompilationData->HasDirectives = ContainsPreprocessorDirectives;
				CompilationData->CanCompile = true;

				auto Metadata = gcnew ScriptTextMetadata;
				Metadata->HasPreprocessorDirectives = ContainsPreprocessorDirectives;
				CompilationData->SerializedMetadata = ScriptTextMetadataHelper::SerializeMetadata(Metadata);


				DisposibleDataAutoPtr<componentDLLInterface::ScriptCompileData> CompilationResult(
					nativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateCompileData());

				CString ScriptTextToCompile(CompilationData->PreprocessedScriptText->Replace("\n", "\r\n"));
				CompilationResult->Script.Text = ScriptTextToCompile.c_str();
				CompilationResult->Script.Type = NativeScript->Type;
				CompilationResult->Script.ParentForm = NativeScript->ParentForm;

				OutSuccess = nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileScript(CompilationResult.get());
				if (OutSuccess)
				{
					String^ OriginalText = CompilationData->UnpreprocessedScriptText + CompilationData->SerializedMetadata;
					CString OrgScriptText(OriginalText);
					nativeWrapper::g_CSEInterfaceTable->ScriptEditor.SetScriptText(NativeScript->ParentForm, OrgScriptText.c_str());
				}
				else
				{
					for (int i = 0; i < CompilationResult->CompileErrorData.Count; i++)
					{
						String^ Message = gcnew String(CompilationResult->CompileErrorData.ErrorListHead[i].Message);
						int Line = CompilationResult->CompileErrorData.ErrorListHead[i].Line;
						if (Line < 1)
							Line = 1;

						OutMessages->Add(FormatLogMessage(Line, Message, true));
					}
				}
			}


			void DiskSync::WriteToConsoleContext(String^ Message)
			{
				CString Msg(Message);
				nativeWrapper::g_CSEInterfaceTable->EditorAPI.PrintToConsoleContext(ConsoleMessageLogContext, Msg.c_str(), true);
			}

			System::String^ DiskSync::FormatLogMessage(int Line, String^ Message, bool Critical)
			{
				if (Line < 1)
					Line = 1;

				return String::Format("[{0}]\tLine {1}\t{2}",
									(Critical ? "E" : "I"), Line, Message);
			}

			void DiskSync::SyncTimer_Tick(Object^ Sender, EventArgs^ E)
			{
				if (SyncInProgress)
				{
					UInt32 Failed = 0;
					DoSyncLoop(false, Failed);
				}
			}

			DiskSync::DiskSync()
			{
				SyncInProgress = false;
				ExecutingSyncLoop = false;
				WorkingDir = "";
				SyncedScripts = gcnew List<SyncedScriptData^>;
				EditorIDsToSyncedData = gcnew Dictionary<String ^, SyncedScriptData ^>;

				SyncTimer = gcnew Timer;
				SyncTimer->Interval = 3000;
				SyncTimer->Enabled = false;
				SyncTimer->Tick += gcnew System::EventHandler(this, &DiskSync::SyncTimer_Tick);

				CString ContextName(gcnew String("Script Sync"));
				ConsoleMessageLogContext = nativeWrapper::g_CSEInterfaceTable->EditorAPI.RegisterConsoleContext(ContextName.c_str());

				AutomaticSync = preferences::SettingsHolder::Get()->ScriptSync->AutoSyncChanges;
				AutomaticSyncIntervalSeconds = preferences::SettingsHolder::Get()->ScriptSync->AutoSyncInterval;
			}

			DiskSync::~DiskSync()
			{
				if (SyncInProgress)
					Stop();

				nativeWrapper::g_CSEInterfaceTable->EditorAPI.DeregisterConsoleContext(ConsoleMessageLogContext);
			}

			void DiskSync::Start(String^ WorkingDir, List<String^>^ SyncedScriptEditorIDs)
			{
				if (SyncInProgress)
				{
					DebugPrint("Script sync already in progress!", true);
					return;
				}

				this->WorkingDir = WorkingDir;

				List<Tuple<String^, DateTime>^>^ Existing = gcnew List<Tuple<String ^, DateTime> ^>;
				for each (String ^ EID in SyncedScriptEditorIDs)
				{

					SyncedScriptData^ Data = gcnew SyncedScriptData(EID, WorkingDir);
					this->SyncedScripts->Add(Data);
					this->EditorIDsToSyncedData->Add(EID, Data);

					if (File::Exists(Data->FilePath))
						Existing->Add(gcnew Tuple<String ^, DateTime>(EID, System::IO::File::GetLastWriteTime(Data->FilePath)));
				}


				SyncStartEventArgs^ StartEventArgs = gcnew SyncStartEventArgs(SyncedScripts, Existing);
				SyncStart(this, StartEventArgs);

				SyncInProgress = true;

				WriteToConsoleContext("Syncing === STARTED!");
				for each (SyncedScriptData ^ Data in SyncedScripts)
				{
					WriteToConsoleContext("Writing script '" + Data->EditorID + " to disk @ " + Data->FilePath);
					bool Overwrite = StartEventArgs->ExistingFilesOnDisk->ContainsKey(Data->EditorID) &&
									StartEventArgs->ExistingFilesOnDisk[Data->EditorID] == SyncStartEventArgs::ExistingFileHandlingOperation::Overwrite;

					if (!SyncToDisk(Data, Overwrite))
						WriteToConsoleContext("\tError writing to disk file for the first time");

					auto InitLogMessages = gcnew List<String^>;
					InitLogMessages->Add("");
					Data->WriteLog(InitLogMessages);
				}
			}

			void DiskSync::Stop()
			{
				if (SyncInProgress == false)
				{
					DebugPrint("Script sync has not started yet!", true);
					return;
				}

				Debug::Assert(ExecutingSyncLoop == false);

				UInt32 Failed = 0;
				DoSyncLoop(false, Failed);


				SyncStopEventArgs^ StopEventArgs = gcnew SyncStopEventArgs(SyncedScripts->Count, Failed);
				SyncStop(this, StopEventArgs);

				SyncInProgress = false;

				if (StopEventArgs->RemoveLogFiles)
				{
					for each (SyncedScriptData^ Data in SyncedScripts)
						Data->RemoveLog();
				}


				this->EditorIDsToSyncedData->Clear();
				this->SyncedScripts->Clear();

				WriteToConsoleContext("Syncing === ENDED!");
			}

			bool DiskSync::IsScriptBeingSynced(String^ ScriptEditorID)
			{
				return SyncInProgress && EditorIDsToSyncedData->ContainsKey(ScriptEditorID);
			}

			System::String^ DiskSync::GetSyncLogContents(String^ ScriptEditorID)
			{
				if (IsScriptBeingSynced(ScriptEditorID) == false)
					return String::Empty;

				String^ Out = EditorIDsToSyncedData[ScriptEditorID]->ReadLog();
				if (Out == nullptr)
					Out = "Error reading log file. Check the console for more information.";

				return Out;
			}

			void DiskSync::ForceSyncToDisk(String^ ScriptEditorID)
			{
				if (IsScriptBeingSynced(ScriptEditorID) == false)
					return;

				auto Data = EditorIDsToSyncedData[ScriptEditorID];
				WriteToConsoleContext("Forcefully syncing script '" + Data->EditorID + " to disk @ " + Data->FilePath);

				SyncToDisk(Data, true);
			}

			void DiskSync::ForceSyncFromDisk(String^ ScriptEditorID)
			{
				if (IsScriptBeingSynced(ScriptEditorID) == false)
					return;

				auto Data = EditorIDsToSyncedData[ScriptEditorID];
				WriteToConsoleContext("Forcefully syncing script '" + Data->EditorID + " from disk @ " + Data->FilePath);

				SyncFromDisk(Data);
			}

			void DiskSync::OpenLogFile(String^ ScriptEditorID)
			{
				if (IsScriptBeingSynced(ScriptEditorID) == false)
					return;

				auto Path = EditorIDsToSyncedData[ScriptEditorID]->LogPath;
				try { Process::Start(Path); }
				catch (Exception^ E) {
					DebugPrint("Couldn't open script sync log file @" + Path + "! Exception: " + E->ToString(), true);
				}
			}

			void DiskSync::OpenScriptFile(String^ ScriptEditorID)
			{
				if (IsScriptBeingSynced(ScriptEditorID) == false)
					return;

				auto Path = EditorIDsToSyncedData[ScriptEditorID]->FilePath;
				try { Process::Start(Path); }
				catch (Exception^ E) {
					DebugPrint("Couldn't open script sync disk file @" + Path + "! Exception: " + E->ToString(), true);
				}
			}

			DiskSync^ DiskSync::Get()
			{
				if (Singleton == nullptr)
					Singleton = gcnew DiskSync();

				return Singleton;
			}

			void DiskSyncDialog::InitializeComponent(void)
			{
				this->components = (gcnew System::ComponentModel::Container());
				this->LabelWorkingDir = (gcnew System::Windows::Forms::Label());
				this->FolderDlgWorkingDir = (gcnew System::Windows::Forms::FolderBrowserDialog());
				this->TextBoxWorkingDir = (gcnew System::Windows::Forms::TextBox());
				this->ButtonSelectWorkingDir = (gcnew System::Windows::Forms::Button());
				this->GroupSyncSettings = (gcnew System::Windows::Forms::GroupBox());
				this->CheckboxAutoDeleteLogs = (gcnew System::Windows::Forms::CheckBox());
				this->GroupStartupFileHandling = (gcnew System::Windows::Forms::GroupBox());
				this->RadioPromptForFileHandling = (gcnew System::Windows::Forms::RadioButton());
				this->RadioUseExistingFiles = (gcnew System::Windows::Forms::RadioButton());
				this->RadioOverwriteExistingFiles = (gcnew System::Windows::Forms::RadioButton());
				this->LabelSeconds = (gcnew System::Windows::Forms::Label());
				this->NumericAutoSyncSeconds = (gcnew System::Windows::Forms::NumericUpDown());
				this->CheckboxAutoSync = (gcnew System::Windows::Forms::CheckBox());
				this->ButtonStartStopSync = (gcnew System::Windows::Forms::Button());
				this->LabelScriptsToSync = (gcnew System::Windows::Forms::Label());
				this->ButtonSelectScripts = (gcnew System::Windows::Forms::Button());
				this->ListViewSyncedScripts = (gcnew BrightIdeasSoftware::FastObjectListView());
				this->ColScriptName = (gcnew BrightIdeasSoftware::OLVColumn());
				this->ColLastSyncTime = (gcnew BrightIdeasSoftware::OLVColumn());
				this->LVSyncedStripsContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
				this->SyncToDiskToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
				this->SyncFromDiskToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
				this->ToolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
				this->OpenLogToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
				this->OpenSyncedFileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
				this->TextBoxSelectedScriptLog = (gcnew System::Windows::Forms::TextBox());
				this->LabelSelectedScriptLog = (gcnew System::Windows::Forms::Label());
				this->ButtonOpenWorkingDir = (gcnew System::Windows::Forms::Button());
				this->GroupSyncSettings->SuspendLayout();
				this->GroupStartupFileHandling->SuspendLayout();
				(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->BeginInit();
				(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->BeginInit();
				this->LVSyncedStripsContextMenu->SuspendLayout();
				this->SuspendLayout();


				//
				// LabelWorkingDir
				//
				this->LabelWorkingDir->AutoSize = true;
				this->LabelWorkingDir->Location = System::Drawing::Point(418, 9);
				this->LabelWorkingDir->Name = L"LabelWorkingDir";
				this->LabelWorkingDir->Size = System::Drawing::Size(92, 13);
				this->LabelWorkingDir->TabIndex = 0;
				this->LabelWorkingDir->Text = L"Working Directory";
				//
				// TextBoxWorkingDir
				//
				this->TextBoxWorkingDir->Location = System::Drawing::Point(516, 6);
				this->TextBoxWorkingDir->Name = L"TextBoxWorkingDir";
				this->TextBoxWorkingDir->ReadOnly = true;
				this->TextBoxWorkingDir->Size = System::Drawing::Size(179, 20);
				this->TextBoxWorkingDir->TabIndex = 1;
				//
				// ButtonSelectWorkingDir
				//
				this->ButtonSelectWorkingDir->Location = System::Drawing::Point(701, 6);
				this->ButtonSelectWorkingDir->Name = L"ButtonSelectWorkingDir";
				this->ButtonSelectWorkingDir->Size = System::Drawing::Size(67, 20);
				this->ButtonSelectWorkingDir->TabIndex = 2;
				this->ButtonSelectWorkingDir->Text = L"Browse...";
				this->ButtonSelectWorkingDir->UseVisualStyleBackColor = true;
				//
				// GroupSyncSettings
				//
				this->GroupSyncSettings->Controls->Add(this->CheckboxAutoDeleteLogs);
				this->GroupSyncSettings->Controls->Add(this->GroupStartupFileHandling);
				this->GroupSyncSettings->Controls->Add(this->LabelSeconds);
				this->GroupSyncSettings->Controls->Add(this->NumericAutoSyncSeconds);
				this->GroupSyncSettings->Controls->Add(this->CheckboxAutoSync);
				this->GroupSyncSettings->Location = System::Drawing::Point(418, 242);
				this->GroupSyncSettings->Name = L"GroupSyncSettings";
				this->GroupSyncSettings->Size = System::Drawing::Size(401, 163);
				this->GroupSyncSettings->TabIndex = 3;
				this->GroupSyncSettings->TabStop = false;
				this->GroupSyncSettings->Text = L"Sync Settings";
				//
				// CheckboxAutoDeleteLogs
				//
				this->CheckboxAutoDeleteLogs->AutoSize = true;
				this->CheckboxAutoDeleteLogs->Location = System::Drawing::Point(16, 134);
				this->CheckboxAutoDeleteLogs->Name = L"CheckboxAutoDeleteLogs";
				this->CheckboxAutoDeleteLogs->Size = System::Drawing::Size(252, 17);
				this->CheckboxAutoDeleteLogs->TabIndex = 4;
				this->CheckboxAutoDeleteLogs->Text = L"Automatically delete log files when syncing ends";
				this->CheckboxAutoDeleteLogs->UseVisualStyleBackColor = true;
				//
				// GroupStartupFileHandling
				//
				this->GroupStartupFileHandling->Controls->Add(this->RadioPromptForFileHandling);
				this->GroupStartupFileHandling->Controls->Add(this->RadioUseExistingFiles);
				this->GroupStartupFileHandling->Controls->Add(this->RadioOverwriteExistingFiles);
				this->GroupStartupFileHandling->Location = System::Drawing::Point(16, 53);
				this->GroupStartupFileHandling->Name = L"GroupStartupFileHandling";
				this->GroupStartupFileHandling->Size = System::Drawing::Size(362, 66);
				this->GroupStartupFileHandling->TabIndex = 3;
				this->GroupStartupFileHandling->TabStop = false;
				this->GroupStartupFileHandling->Text = L"Existing File Handling During Startup";
				//
				// RadioPromptForFileHandling
				//
				this->RadioPromptForFileHandling->AutoSize = true;
				this->RadioPromptForFileHandling->Location = System::Drawing::Point(6, 42);
				this->RadioPromptForFileHandling->Name = L"RadioPromptForFileHandling";
				this->RadioPromptForFileHandling->Size = System::Drawing::Size(116, 17);
				this->RadioPromptForFileHandling->TabIndex = 2;
				this->RadioPromptForFileHandling->TabStop = true;
				this->RadioPromptForFileHandling->Text = L"Prompt for each file";
				this->RadioPromptForFileHandling->UseVisualStyleBackColor = true;
				//
				// RadioUseExistingFiles
				//
				this->RadioUseExistingFiles->AutoSize = true;
				this->RadioUseExistingFiles->Location = System::Drawing::Point(211, 19);
				this->RadioUseExistingFiles->Name = L"RadioUseExistingFiles";
				this->RadioUseExistingFiles->Size = System::Drawing::Size(145, 17);
				this->RadioUseExistingFiles->TabIndex = 1;
				this->RadioUseExistingFiles->TabStop = true;
				this->RadioUseExistingFiles->Text = L"Use without modifications";
				this->RadioUseExistingFiles->UseVisualStyleBackColor = true;
				//
				// RadioOverwriteExistingFiles
				//
				this->RadioOverwriteExistingFiles->AutoSize = true;
				this->RadioOverwriteExistingFiles->Location = System::Drawing::Point(6, 19);
				this->RadioOverwriteExistingFiles->Name = L"RadioOverwriteExistingFiles";
				this->RadioOverwriteExistingFiles->Size = System::Drawing::Size(171, 17);
				this->RadioOverwriteExistingFiles->TabIndex = 0;
				this->RadioOverwriteExistingFiles->TabStop = true;
				this->RadioOverwriteExistingFiles->Text = L"Overwrite with plugin script text";
				this->RadioOverwriteExistingFiles->UseVisualStyleBackColor = true;
				//
				// LabelSeconds
				//
				this->LabelSeconds->AutoSize = true;
				this->LabelSeconds->Location = System::Drawing::Point(271, 20);
				this->LabelSeconds->Name = L"LabelSeconds";
				this->LabelSeconds->Size = System::Drawing::Size(47, 13);
				this->LabelSeconds->TabIndex = 2;
				this->LabelSeconds->Text = L"seconds";
				//
				// NumericAutoSyncSeconds
				//
				this->NumericAutoSyncSeconds->Location = System::Drawing::Point(208, 18);
				this->NumericAutoSyncSeconds->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 60, 0, 0, 0 });
				this->NumericAutoSyncSeconds->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
				this->NumericAutoSyncSeconds->Name = L"NumericAutoSyncSeconds";
				this->NumericAutoSyncSeconds->Size = System::Drawing::Size(57, 20);
				this->NumericAutoSyncSeconds->TabIndex = 1;
				this->NumericAutoSyncSeconds->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
				//
				// CheckboxAutoSync
				//
				this->CheckboxAutoSync->AutoSize = true;
				this->CheckboxAutoSync->Location = System::Drawing::Point(16, 19);
				this->CheckboxAutoSync->Name = L"CheckboxAutoSync";
				this->CheckboxAutoSync->Size = System::Drawing::Size(186, 17);
				this->CheckboxAutoSync->TabIndex = 0;
				this->CheckboxAutoSync->Text = L"Automatically sync changes every";
				this->CheckboxAutoSync->UseVisualStyleBackColor = true;
				//
				// ButtonStartStopSync
				//
				this->ButtonStartStopSync->Location = System::Drawing::Point(15, 373);
				this->ButtonStartStopSync->Name = L"ButtonStartStopSync";
				this->ButtonStartStopSync->Size = System::Drawing::Size(396, 32);
				this->ButtonStartStopSync->TabIndex = 4;
				this->ButtonStartStopSync->Text = L"Start Syncing";
				this->ButtonStartStopSync->UseVisualStyleBackColor = true;
				//
				// LabelScriptsToSync
				//
				this->LabelScriptsToSync->AutoSize = true;
				this->LabelScriptsToSync->Location = System::Drawing::Point(12, 9);
				this->LabelScriptsToSync->Name = L"LabelScriptsToSync";
				this->LabelScriptsToSync->Size = System::Drawing::Size(78, 13);
				this->LabelScriptsToSync->TabIndex = 6;
				this->LabelScriptsToSync->Text = L"Synced Scripts";
				//
				// ButtonSelectScripts
				//
				this->ButtonSelectScripts->Location = System::Drawing::Point(345, 5);
				this->ButtonSelectScripts->Name = L"ButtonSelectScripts";
				this->ButtonSelectScripts->Size = System::Drawing::Size(67, 21);
				this->ButtonSelectScripts->TabIndex = 7;
				this->ButtonSelectScripts->Text = L"Select...";
				this->ButtonSelectScripts->UseVisualStyleBackColor = true;
				//
				// ListViewSyncedScripts
				//
				this->ListViewSyncedScripts->AllColumns->Add(this->ColScriptName);
				this->ListViewSyncedScripts->AllColumns->Add(this->ColLastSyncTime);
				this->ListViewSyncedScripts->CellEditUseWholeCell = false;
				this->ListViewSyncedScripts->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {
					this->ColScriptName,
						this->ColLastSyncTime
				});
				this->ListViewSyncedScripts->Cursor = System::Windows::Forms::Cursors::Default;
				this->ListViewSyncedScripts->FullRowSelect = true;
				this->ListViewSyncedScripts->HideSelection = false;
				this->ListViewSyncedScripts->Location = System::Drawing::Point(15, 36);
				this->ListViewSyncedScripts->MultiSelect = false;
				this->ListViewSyncedScripts->Name = L"ListViewSyncedScripts";
				this->ListViewSyncedScripts->ShowGroups = false;
				this->ListViewSyncedScripts->Size = System::Drawing::Size(396, 331);
				this->ListViewSyncedScripts->TabIndex = 8;
				this->ListViewSyncedScripts->UseCompatibleStateImageBehavior = false;
				this->ListViewSyncedScripts->View = System::Windows::Forms::View::Details;
				this->ListViewSyncedScripts->VirtualMode = true;
				//
				// ColScriptName
				//
				this->ColScriptName->Sortable = false;
				this->ColScriptName->Text = L"EditorID";
				this->ColScriptName->Width = 211;
				//
				// ColLastSyncTime
				//
				this->ColLastSyncTime->Sortable = false;
				this->ColLastSyncTime->Text = L"Last Sync Attempt Time";
				this->ColLastSyncTime->Width = 160;
				//
				// LVSyncedStripsContextMenu
				//
				this->LVSyncedStripsContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {
					this->SyncToDiskToolStripMenuItem,
						this->SyncFromDiskToolStripMenuItem, this->ToolStripSeparator1, this->OpenLogToolStripMenuItem, this->OpenSyncedFileToolStripMenuItem
				});
				this->LVSyncedStripsContextMenu->Name = L"LVSyncedStripsContextMenu";
				this->LVSyncedStripsContextMenu->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
				this->LVSyncedStripsContextMenu->ShowImageMargin = false;
				this->LVSyncedStripsContextMenu->Size = System::Drawing::Size(141, 98);
				//
				// SyncToDiskToolStripMenuItem
				//
				this->SyncToDiskToolStripMenuItem->Name = L"SyncToDiskToolStripMenuItem";
				this->SyncToDiskToolStripMenuItem->Size = System::Drawing::Size(140, 22);
				this->SyncToDiskToolStripMenuItem->Text = L"Sync To Disk";
				//
				// SyncFromDiskToolStripMenuItem
				//
				this->SyncFromDiskToolStripMenuItem->Name = L"SyncFromDiskToolStripMenuItem";
				this->SyncFromDiskToolStripMenuItem->Size = System::Drawing::Size(140, 22);
				this->SyncFromDiskToolStripMenuItem->Text = L"Sync From Disk";
				//
				// ToolStripSeparator1
				//
				this->ToolStripSeparator1->Name = L"ToolStripSeparator1";
				this->ToolStripSeparator1->Size = System::Drawing::Size(137, 6);
				//
				// OpenLogToolStripMenuItem
				//
				this->OpenLogToolStripMenuItem->Name = L"OpenLogToolStripMenuItem";
				this->OpenLogToolStripMenuItem->Size = System::Drawing::Size(140, 22);
				this->OpenLogToolStripMenuItem->Text = L"Open Log";
				//
				// OpenSyncedFileToolStripMenuItem
				//
				this->OpenSyncedFileToolStripMenuItem->Name = L"OpenSyncedFileToolStripMenuItem";
				this->OpenSyncedFileToolStripMenuItem->Size = System::Drawing::Size(140, 22);
				this->OpenSyncedFileToolStripMenuItem->Text = L"Open Synced File";
				//
				// TextBoxSelectedScriptLog
				//
				this->TextBoxSelectedScriptLog->Location = System::Drawing::Point(418, 52);
				this->TextBoxSelectedScriptLog->Multiline = true;
				this->TextBoxSelectedScriptLog->Name = L"TextBoxSelectedScriptLog";
				this->TextBoxSelectedScriptLog->ReadOnly = true;
				this->TextBoxSelectedScriptLog->Size = System::Drawing::Size(401, 184);
				this->TextBoxSelectedScriptLog->TabIndex = 11;
				//
				// LabelSelectedScriptLog
				//
				this->LabelSelectedScriptLog->AutoSize = true;
				this->LabelSelectedScriptLog->Location = System::Drawing::Point(418, 35);
				this->LabelSelectedScriptLog->Name = L"LabelSelectedScriptLog";
				this->LabelSelectedScriptLog->Size = System::Drawing::Size(25, 13);
				this->LabelSelectedScriptLog->TabIndex = 12;
				this->LabelSelectedScriptLog->Text = L"Log";
				//
				// ButtonOpenWorkingDir
				//
				this->ButtonOpenWorkingDir->Location = System::Drawing::Point(774, 5);
				this->ButtonOpenWorkingDir->Name = L"ButtonOpenWorkingDir";
				this->ButtonOpenWorkingDir->Size = System::Drawing::Size(45, 21);
				this->ButtonOpenWorkingDir->TabIndex = 13;
				this->ButtonOpenWorkingDir->Text = L"Open";
				this->ButtonOpenWorkingDir->UseVisualStyleBackColor = true;
				//
				// SESyncUI
				//
				this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
				this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
				this->ClientSize = System::Drawing::Size(830, 413);
				this->Controls->Add(this->ButtonOpenWorkingDir);
				this->Controls->Add(this->LabelSelectedScriptLog);
				this->Controls->Add(this->TextBoxSelectedScriptLog);
				this->Controls->Add(this->ListViewSyncedScripts);
				this->Controls->Add(this->ButtonSelectScripts);
				this->Controls->Add(this->LabelScriptsToSync);
				this->Controls->Add(this->ButtonStartStopSync);
				this->Controls->Add(this->GroupSyncSettings);
				this->Controls->Add(this->ButtonSelectWorkingDir);
				this->Controls->Add(this->TextBoxWorkingDir);
				this->Controls->Add(this->LabelWorkingDir);
				this->DoubleBuffered = true;
				this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
				this->MaximizeBox = false;
				this->Name = L"SESyncUI";
				this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
				this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
				this->Text = L"Sync Scripts to Disk";
				this->GroupSyncSettings->ResumeLayout(false);
				this->GroupSyncSettings->PerformLayout();
				this->GroupStartupFileHandling->ResumeLayout(false);
				this->GroupStartupFileHandling->PerformLayout();
				(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->EndInit();
				(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->EndInit();
				this->LVSyncedStripsContextMenu->ResumeLayout(false);
				this->ResumeLayout(false);
				this->PerformLayout();


				this->ListViewSyncedScripts->SmallImageList = gcnew ImageList();
				this->ListViewSyncedScripts->SmallImageList->ImageSize = Drawing::Size(14, 14);
				this->ListViewSyncedScripts->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("ScriptSyncStatusEmpty"));
				this->ListViewSyncedScripts->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("ScriptSyncStatusIndeterminate"));
				this->ListViewSyncedScripts->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("ScriptSyncStatusSuccess"));
				this->ListViewSyncedScripts->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("ScriptSyncStatusFailure"));

				this->ColScriptName->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&DiskSyncDialog::ListViewAspectScriptNameGetter);
				this->ColScriptName->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&DiskSyncDialog::ListViewImageScriptNameGetter);
				this->ColLastSyncTime->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&DiskSyncDialog::ListViewAspectLastSyncTimeGetter);
				this->ColLastSyncTime->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&DiskSyncDialog::ListViewAspectToStringLastSyncTime);


				SyncedScripts = gcnew Dictionary<String ^, SyncedScriptListViewWrapper ^>;

				DiskSyncSyncStartHandler = gcnew SyncStartEventHandler(this, &DiskSyncDialog::DiskSync_SyncStart);
				DiskSyncSyncStopHandler = gcnew SyncStopEventHandler(this, &DiskSyncDialog::DiskSync_SyncStop);
				DiskSyncSyncWriteToDiskHandler = gcnew SyncWriteToDiskEventHandler(this, &DiskSyncDialog::DiskSync_SyncWriteToDisk);
				DiskSyncSyncPreCompileHandler = gcnew SyncPreCompileEventHandler(this, &DiskSyncDialog::DiskSync_SyncPreCompile);
				DiskSyncSyncPostCompileHandler = gcnew SyncPostCompileEventHandler(this, &DiskSyncDialog::DiskSync_SyncPostCompile);

				ButtonSelectWorkingDir->Click += gcnew EventHandler(this, &DiskSyncDialog::ButtonSelectWorkingDir_Click);
				ButtonOpenWorkingDir->Click += gcnew EventHandler(this, &DiskSyncDialog::ButtonOpenWorkingDir_Click);
				ButtonSelectScripts->Click += gcnew EventHandler(this, &DiskSyncDialog::ButtonSelectScripts_Click);
				ButtonStartStopSync->Click += gcnew EventHandler(this, &DiskSyncDialog::ButtonStartStopSync_Click);
				SyncToDiskToolStripMenuItem->Click += gcnew EventHandler(this, &DiskSyncDialog::SyncToDiskToolStripMenuItem_Click);
				SyncFromDiskToolStripMenuItem->Click += gcnew EventHandler(this, &DiskSyncDialog::SyncFromDiskToolStripMenuItem_Click);
				OpenLogToolStripMenuItem->Click += gcnew EventHandler(this, &DiskSyncDialog::OpenLogToolStripMenuItem_Click);
				OpenSyncedFileToolStripMenuItem->Click += gcnew EventHandler(this, &DiskSyncDialog::OpenSyncedFileToolStripMenuItem_Click);
				CheckboxAutoSync->Click += gcnew EventHandler(this, &DiskSyncDialog::CheckboxAutoSync_Click);

				NumericAutoSyncSeconds->ValueChanged += gcnew EventHandler(this, &DiskSyncDialog::NumericAutoSyncSeconds_ValueChanged);
				ListViewSyncedScripts->CellRightClick += gcnew EventHandler<BrightIdeasSoftware::CellRightClickEventArgs^>(this, &DiskSyncDialog::ListViewSyncedScripts_CellRightClick);
				ListViewSyncedScripts->SelectedIndexChanged += gcnew EventHandler(this, &DiskSyncDialog::ListViewSyncedScripts_SelectedIndexChanged);

				this->Closing += gcnew CancelEventHandler(this, &DiskSyncDialog::Dialog_Cancel);
			}


			void DiskSyncDialog::ButtonSelectWorkingDir_Click(Object^ Sender, EventArgs^ E)
			{
				Debug::Assert(IsSyncInProgress() == false);

				auto Result = FolderDlgWorkingDir->ShowDialog();
				if (Result == Windows::Forms::DialogResult::OK)
					TextBoxWorkingDir->Text = FolderDlgWorkingDir->SelectedPath;
			}


			void DiskSyncDialog::ButtonOpenWorkingDir_Click(Object ^ Sender, EventArgs ^ E)
			{
				if (TextBoxWorkingDir->Text == "")
					return;

				auto StartInfo = gcnew System::Diagnostics::ProcessStartInfo();
				StartInfo->FileName = TextBoxWorkingDir->Text;
				StartInfo->UseShellExecute = true;
				StartInfo->Verb = "open";

				try { Process::Start(StartInfo); }
				catch (Exception^ E) {
					DebugPrint("Couldn't open script sync working directory @" + TextBoxWorkingDir->Text + "! Exception: " + E->ToString(), true);
				}
			}

			void DiskSyncDialog::ButtonSelectScripts_Click(Object^ Sender, EventArgs^ E)
			{
				Debug::Assert(IsSyncInProgress() == false);

				SelectScriptDialogParams^ Params = gcnew SelectScriptDialogParams;
				Params->ShowDeletedScripts = false;
				Params->PreventSyncedScriptSelection = true;

				SelectScriptDialog ScriptSelection(Params);
				if (ScriptSelection.HasResult == false || ScriptSelection.ResultData->SelectionCount == 0)
					return;

				SyncedScripts->Clear();
				for each (auto Itr in ScriptSelection.ResultData->SelectedScriptEditorIDs)
					SyncedScripts[Itr] = gcnew SyncedScriptListViewWrapper(Itr);

				ListViewSyncedScripts->SetObjects(SyncedScripts->Values, false);
			}

			void DiskSyncDialog::ButtonStartStopSync_Click(Object^ Sender, EventArgs^ E)
			{
				if (IsSyncInProgress())
				{
					DiskSync::Get()->Stop();
					return;
				}

				if (SyncedScripts->Count)
				{
					for each (auto% Itr in SyncedScripts->Keys)
						SyncedScripts[Itr]->Reset();

					ListViewSyncedScripts->SetObjects(SyncedScripts->Values, true);
				}

				List<String^>^ SyncedEditorIDs = gcnew List<String^>;
				for each (auto% Itr in SyncedScripts->Keys)
					SyncedEditorIDs->Add(Itr);

				if (SyncedEditorIDs->Count == 0)
				{
					MessageBox::Show("At least one script must to be selected to begin syncing.", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Information);
					return;
				}

				String^ WorkingDir = TextBoxWorkingDir->Text->TrimEnd();
				if (WorkingDir == "")
				{
					MessageBox::Show("A working directory must be selected to begin syncing.", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Information);
					return;
				}

				DiskSync::Get()->Start(WorkingDir, SyncedEditorIDs);
			}

			void DiskSyncDialog::SyncToDiskToolStripMenuItem_Click(Object^ Sender, EventArgs^ E)
			{
				Debug::Assert(IsSyncInProgress());

				auto Selection = safe_cast<SyncedScriptListViewWrapper^>(LVSyncedStripsContextMenu->Tag);
				DiskSync::Get()->ForceSyncToDisk(Selection->EditorID);
			}

			void DiskSyncDialog::SyncFromDiskToolStripMenuItem_Click(Object^ Sender, EventArgs^ E)
			{
				Debug::Assert(IsSyncInProgress());

				auto Selection = safe_cast<SyncedScriptListViewWrapper^>(LVSyncedStripsContextMenu->Tag);
				DiskSync::Get()->ForceSyncFromDisk(Selection->EditorID);
			}

			void DiskSyncDialog::OpenLogToolStripMenuItem_Click(Object^ Sender, EventArgs^ E)
			{
				auto Selection = safe_cast<SyncedScriptListViewWrapper^>(LVSyncedStripsContextMenu->Tag);
				DiskSync::Get()->OpenLogFile(Selection->EditorID);
			}

			void DiskSyncDialog::OpenSyncedFileToolStripMenuItem_Click(Object^ Sender, EventArgs^ E)
			{
				auto Selection = safe_cast<SyncedScriptListViewWrapper^>(LVSyncedStripsContextMenu->Tag);
				DiskSync::Get()->OpenScriptFile(Selection->EditorID);
			}

			void DiskSyncDialog::CheckboxAutoSync_Click(Object^ Sender, EventArgs^ E)
			{
				DiskSync::Get()->AutomaticSync = CheckboxAutoSync->Checked;
			}

			void DiskSyncDialog::NumericAutoSyncSeconds_ValueChanged(Object^ Sender, EventArgs^ E)
			{
				DiskSync::Get()->AutomaticSyncIntervalSeconds = Decimal::ToUInt32(NumericAutoSyncSeconds->Value);
			}

			void DiskSyncDialog::ListViewSyncedScripts_CellRightClick(Object^ Sender, BrightIdeasSoftware::CellRightClickEventArgs^ E)
			{
				if (E->Model == nullptr)
					return;

				SyncFromDiskToolStripMenuItem->Enabled = true;
				SyncToDiskToolStripMenuItem->Enabled = true;
				OpenLogToolStripMenuItem->Enabled = true;
				OpenSyncedFileToolStripMenuItem->Enabled = true;

				if (!IsSyncInProgress())
				{
					SyncFromDiskToolStripMenuItem->Enabled = false;
					SyncToDiskToolStripMenuItem->Enabled = false;
					OpenLogToolStripMenuItem->Enabled = false;
					OpenSyncedFileToolStripMenuItem->Enabled = false;
				}

				E->MenuStrip = LVSyncedStripsContextMenu;
				LVSyncedStripsContextMenu->Tag = E->Model;
			}


			void DiskSyncDialog::ListViewSyncedScripts_SelectedIndexChanged(Object^ Sender, EventArgs^ E)
			{
				auto Selection = safe_cast<SyncedScriptListViewWrapper^>(ListViewSyncedScripts->SelectedObject);
				if (Selection == nullptr)
				{
					LabelSelectedScriptLog->Text = "Log";
					TextBoxSelectedScriptLog->Text = "";

					return;
				}

				LabelSelectedScriptLog->Text = "Log [" + Selection->EditorID + "]";

				if (!IsSyncInProgress())
					TextBoxSelectedScriptLog->Text = "Log contents are only available during the syncing operation.";
				else
					TextBoxSelectedScriptLog->Text = DiskSync::Get()->GetSyncLogContents(Selection->EditorID);
			}

			void DiskSyncDialog::Dialog_Cancel(Object^ Sender, CancelEventArgs^ E)
			{
				if (IsSyncInProgress())
					DiskSync::Get()->Stop();
			}


			void DiskSyncDialog::DiskSync_SyncStart(Object^ Sender, SyncStartEventArgs^ E)
			{
				ButtonSelectScripts->Enabled = false;
				ButtonSelectWorkingDir->Enabled = false;

				ButtonStartStopSync->Text = "Stop syncing";
				bool PromptUser = RadioPromptForFileHandling->Checked;
				auto DefaultOp = SyncStartEventArgs::ExistingFileHandlingOperation::Overwrite;
				if (RadioUseExistingFiles->Checked)
				{
					DefaultOp = SyncStartEventArgs::ExistingFileHandlingOperation::Keep;
					Debug::Assert(PromptUser == false);
				}

				int i = 0, Total = E->ExistingFilesOnDisk->Count;
				bool ApplyToAll = false;
				auto KeyCopy = gcnew List<String^>(E->ExistingFilesOnDisk->Keys);
				for each (auto Itr in KeyCopy)
				{
					++i;
					if (!PromptUser || ApplyToAll)
					{
						E->ExistingFilesOnDisk[Itr] = DefaultOp;
						continue;
					}

					auto LastWriteTime = E->ExistingFileLastWriteTimestamps[Itr];
					String^ Message = "[" + i + "/" + Total + "] Script '" + Itr + "' has an existing file in the working directory, dated " + LastWriteTime.ToShortDateString() + " " + LastWriteTime.ToLongTimeString() + ".";
					Message += "\n\nWould you like to overwrite this file?\n\nHolding down the Control key while clicking on the buttons below will apply the selection to all following conflicts.";

					auto Selection = MessageBox::Show(Message, SCRIPTEDITOR_TITLE, MessageBoxButtons::YesNo, MessageBoxIcon::Warning);
					DefaultOp = Selection == Windows::Forms::DialogResult::Yes ? SyncStartEventArgs::ExistingFileHandlingOperation::Overwrite : SyncStartEventArgs::ExistingFileHandlingOperation::Keep;
					if (Control::ModifierKeys == Keys::Control)
						ApplyToAll = true;

					E->ExistingFilesOnDisk[Itr] = DefaultOp;
				}
			}

			void DiskSyncDialog::DiskSync_SyncStop(Object^ Sender, SyncStopEventArgs^ E)
			{
				E->RemoveLogFiles = CheckboxAutoDeleteLogs->Checked;

				ButtonSelectScripts->Enabled = true;
				ButtonSelectWorkingDir->Enabled = true;

				ButtonStartStopSync->Text = "Start syncing";

				LabelSelectedScriptLog->Text = "Log";
				TextBoxSelectedScriptLog->Text = "";
			}


			void DiskSyncDialog::DiskSync_SyncWriteToDisk(Object ^ Sender, SyncWriteToDiskEventArgs ^ E)
			{
				auto ListViewObject = SyncedScripts[E->ScriptEditorID];
				ListViewObject->LastSyncTime = E->AccessTimestamp;
				if (ListViewObject->LastSyncSuccess == SyncedScriptListViewWrapper::SuccessState::None)
					ListViewObject->LastSyncSuccess = E->Success ? SyncedScriptListViewWrapper::SuccessState::Success : SyncedScriptListViewWrapper::SuccessState::Failure;

				ListViewSyncedScripts->RefreshObject(ListViewObject);
			}

			void DiskSyncDialog::DiskSync_SyncPreCompile(Object^ Sender, SyncPreCompileEventArgs^ E)
			{
				auto ListViewObject = SyncedScripts[E->ScriptEditorID];
				ListViewObject->LastSyncTime = E->AccessTimestamp;
				ListViewObject->LastSyncSuccess = SyncedScriptListViewWrapper::SuccessState::Indeterminate;

				ListViewSyncedScripts->RefreshObject(ListViewObject);
			}

			void DiskSyncDialog::DiskSync_SyncPostCompile(Object^ Sender, SyncPostCompileEventArgs^ E)
			{
				auto ListViewObject = SyncedScripts[E->ScriptEditorID];
				ListViewObject->LastSyncSuccess = E->Success ? SyncedScriptListViewWrapper::SuccessState::Success : SyncedScriptListViewWrapper::SuccessState::Failure;

				ListViewSyncedScripts->RefreshObject(ListViewObject);

				if (ListViewSyncedScripts->SelectedObject == ListViewObject)
					TextBoxSelectedScriptLog->Text = DiskSync::Get()->GetSyncLogContents(E->ScriptEditorID);
			}

			bool DiskSyncDialog::IsSyncInProgress()
			{
				return DiskSync::Get()->InProgress;
			}

			System::Object^ DiskSyncDialog::ListViewAspectScriptNameGetter(Object^ RowObject)
			{
				auto Model = safe_cast<SyncedScriptListViewWrapper^>(RowObject);
				if (Model == nullptr)
					return nullptr;

				return Model->EditorID;
			}

			System::Object^ DiskSyncDialog::ListViewImageScriptNameGetter(Object^ RowObject)
			{
				auto Model = safe_cast<SyncedScriptListViewWrapper^>(RowObject);
				if (Model == nullptr)
					return nullptr;

				return (int)Model->LastSyncSuccess;
			}

			System::Object^ DiskSyncDialog::ListViewAspectLastSyncTimeGetter(Object^ RowObject)
			{
				auto Model = safe_cast<SyncedScriptListViewWrapper^>(RowObject);
				if (Model == nullptr)
					return nullptr;

				return Model->LastSyncTime;
			}

			System::String^ DiskSyncDialog::ListViewAspectToStringLastSyncTime(Object^ RowObject)
			{
				auto Model = safe_cast<DateTime^>(RowObject);
				if (Model == nullptr || Model->Equals(DateTime()))
					return String::Empty;

				return Model->ToLongTimeString() + "   " + Model->ToShortDateString();
			}

			DiskSyncDialog::DiskSyncDialog()
			{

				InitializeComponent();

				DiskSync::Get()->SyncStart += DiskSyncSyncStartHandler;
				DiskSync::Get()->SyncStop += DiskSyncSyncStopHandler;
				DiskSync::Get()->ScriptWriteToDisk += DiskSyncSyncWriteToDiskHandler;
				DiskSync::Get()->ScriptPreCompile += DiskSyncSyncPreCompileHandler;
				DiskSync::Get()->ScriptPostCompile += DiskSyncSyncPostCompileHandler;

				switch (preferences::SettingsHolder::Get()->ScriptSync->ExistingFileHandlingOp)
				{
				case SyncStartEventArgs::ExistingFileHandlingOperation::Prompt:
					RadioPromptForFileHandling->Checked = true;
					break;
				case SyncStartEventArgs::ExistingFileHandlingOperation::Overwrite:
					RadioOverwriteExistingFiles->Checked = true;
					break;
				case SyncStartEventArgs::ExistingFileHandlingOperation::Keep:
					RadioUseExistingFiles->Checked = true;
					break;
				}

				CheckboxAutoSync->Checked = preferences::SettingsHolder::Get()->ScriptSync->AutoSyncChanges;
				CheckboxAutoDeleteLogs->Checked = preferences::SettingsHolder::Get()->ScriptSync->AutoDeleteLogs;
				NumericAutoSyncSeconds->Value = preferences::SettingsHolder::Get()->ScriptSync->AutoSyncInterval;
				TextBoxWorkingDir->Text = DiskSync::Get()->WorkingDirectory;

				this->Form::Show();
			}

			DiskSyncDialog::~DiskSyncDialog()
			{
				Debug::Assert(Singleton != nullptr);

				DiskSync::Get()->SyncStart -= DiskSyncSyncStartHandler;
				DiskSync::Get()->SyncStop -= DiskSyncSyncStopHandler;
				DiskSync::Get()->ScriptWriteToDisk -= DiskSyncSyncWriteToDiskHandler;
				DiskSync::Get()->ScriptPreCompile -= DiskSyncSyncPreCompileHandler;
				DiskSync::Get()->ScriptPostCompile -= DiskSyncSyncPostCompileHandler;

				if (RadioPromptForFileHandling->Checked)
					preferences::SettingsHolder::Get()->ScriptSync->ExistingFileHandlingOp = SyncStartEventArgs::ExistingFileHandlingOperation::Prompt;
				else if (RadioOverwriteExistingFiles->Checked)
					preferences::SettingsHolder::Get()->ScriptSync->ExistingFileHandlingOp = SyncStartEventArgs::ExistingFileHandlingOperation::Overwrite;
				else
					preferences::SettingsHolder::Get()->ScriptSync->ExistingFileHandlingOp = SyncStartEventArgs::ExistingFileHandlingOperation::Keep;

				preferences::SettingsHolder::Get()->ScriptSync->AutoSyncChanges = CheckboxAutoSync->Checked;
				preferences::SettingsHolder::Get()->ScriptSync->AutoDeleteLogs = CheckboxAutoDeleteLogs->Checked;
				preferences::SettingsHolder::Get()->ScriptSync->AutoSyncInterval = Decimal::ToUInt32(NumericAutoSyncSeconds->Value);

				if (components)
				{
					delete components;
				}

				Singleton = nullptr;
			}


			void DiskSyncDialog::Show()
			{
				if (Singleton == nullptr)
					Singleton = gcnew DiskSyncDialog();

				Singleton->BringToFront();
				Singleton->Focus();
			}

			void DiskSyncDialog::Close()
			{
				if (Singleton)
					Singleton->Form::Close();
			}
		}

	}
}

