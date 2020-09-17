#include "ScriptSync.h"
#include "ScriptTextEditorInterface.h"
#include "SemanticAnalysis.h"
#include "[Common]\NativeWrapper.h"
#include "Preferences.h"
#include "IntelliSenseDatabase.h"
#include "WorkspaceModelComponents.h"

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
			}

			System::String^ SyncedScriptData::ReadFileContents(bool UpdateSyncTimestamp)
			{
				String^ Out = nullptr;
				if (File::Exists(DiskFilePath))
				{
					try { Out = File::ReadAllText(DiskFilePath); }
					catch (Exception^ E) {
						DebugPrint("Couldn't read from script sync file @ " + DiskFilePath + "! Exception: " + E->ToString(), true);
					}
				}

				if (UpdateSyncTimestamp)
					FileLastSyncAttempt = DateTime::Now;

				return Out;
			}

			bool SyncedScriptData::WriteFileContents(String^ Text, bool Overwrite)
			{
				if (File::Exists(DiskFilePath) && Overwrite == false)
					return true;

				try
				{
					File::WriteAllText(DiskFilePath, Text);
					FileLastSyncAttempt = DateTime::Now;
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
				if (File::Exists(DiskFilePath))
				{
					try { Out = File::ReadAllText(LogFilePath); }
					catch (Exception^ E) {
						DebugPrint("Couldn't read from script sync log @ " + LogFilePath + "! Exception: " + E->ToString(), true);
					}
				}

				return Out;
			}

			bool SyncedScriptData::WriteLog(List<String^>^ Messages)
			{
				try { File::WriteAllLines(DiskFilePath, Messages); }
				catch (Exception^ E) {
					DebugPrint("Couldn't write to script sync log @ " + LogFilePath + "! Exception: " + E->ToString(), true);
					return false;
				}

				return true;
			}

			void SyncedScriptData::RemoveLog()
			{
				try { File::Delete(LogFilePath); }
				catch (Exception^ E) {
					DebugPrint("Couldn't delete script sync log @ " + LogFilePath + "! Exception: " + E->ToString(), true);
				}
			}

			void DiskSync::DoSync(bool Force, UInt32% OutFailedCompilations)
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
						WriteToConsoleContext("Compiling script '" + Data->EditorID + " @ " + Data->FilePath);
						ExportScriptToFile(Data, false);

						if (Force || Data->FileLastWrite > Data->FileLastSyncAttempt)
						{
							String^ DiskFileContents = Data->ReadFileContents(true);
							if (DiskFileContents == nullptr)
							{
								WriteToConsoleContext("\tError reading from disk file");
								continue;
							}

							SyncPreCompileEventArgs^ PrecompileArgs = gcnew SyncPreCompileEventArgs(Data->EditorID, DiskFileContents);
							ScriptPreCompile(this, PrecompileArgs);

							SyncPostCompileEventArgs^ PostcompileArgs = gcnew SyncPostCompileEventArgs(Data->EditorID);
							bool CompileSuccess = false;
							List<String^>^ CompileMessages = gcnew List<String^>();

							CompileTimer->Restart();
							CompileScript(Data, DiskFileContents, CompileSuccess, CompileMessages);
							CompileTimer->Stop();

							if (CompileSuccess)
								WriteToConsoleContext("\tCompilation succeeded! Messages:");
							else
							{
								WriteToConsoleContext("\tCompilation failed! Messages:");
								OutFailedCompilations += 1;
							}

							for each (String ^ Message in CompileMessages)
								WriteToConsoleContext("\t\t" + Message);

							if (CompileSuccess)
							{
								List<String^>^ LogMessages = gcnew List<String ^>;
								LogMessages->Add(FormatLogMessage(1, "Compiled successfully at " + DateTime::Now.ToString(), false));
								LogMessages->Add(FormatLogMessage(1, "Elapsed Time: " + CompileTimer->Elapsed.Seconds + " seconds", false));
								LogMessages->AddRange(CompileMessages);

								CompileMessages = LogMessages;
							}

							if (Data->WriteLog(CompileMessages) == false)
								WriteToConsoleContext("\tError writing to log file");


							PostcompileArgs->Success = CompileSuccess;
							PostcompileArgs->OutputMessages = CompileMessages;
							ScriptPostCompile(this, PostcompileArgs);
						}
					}

					WriteToConsoleContext(gcnew String('-', 50));
				}
				ExecutingSyncLoop = false;
			}


			bool DiskSync::ExportScriptToFile(SyncedScriptData^ SyncedScript, bool Overwrite)
			{
				if (File::Exists(SyncedScript->FilePath) && Overwrite == false)
					return true;

				CString EID(SyncedScript->EditorID);
				DisposibleDataAutoPtr<componentDLLInterface::ScriptData> NativeScript
				(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str()));
				Debug::Assert(NativeScript);

				ScriptTextMetadata^ EmbeddedMetadata = gcnew ScriptTextMetadata();
				String^ ScriptText = "";

				ScriptTextMetadataHelper::DeserializeRawScriptText(gcnew String(NativeScript->Text),
					ScriptText, EmbeddedMetadata);

				return SyncedScript->WriteFileContents(ScriptText, false);
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
					DoSync(false, Failed);
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

				CString ContextName(gcnew String("Script Sync"));
				ConsoleMessageLogContext = nativeWrapper::g_CSEInterfaceTable->EditorAPI.RegisterConsoleContext(ContextName.c_str());
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

				List<String^>^ Existing = gcnew List<String^>;
				for each (String ^ EID in SyncedScriptEditorIDs)
				{

					SyncedScriptData^ Data = gcnew SyncedScriptData(EID, WorkingDir);
					this->SyncedScripts->Add(Data);
					this->EditorIDsToSyncedData->Add(EID, Data);

					if (File::Exists(Data->FilePath))
						Existing->Add(EID);
				}


				SyncStartEventArgs^ StartEventArgs = gcnew SyncStartEventArgs(SyncedScripts, Existing);
				SyncStart(this, StartEventArgs);

				SyncInProgress = true;

				for each (SyncedScriptData ^ Data in SyncedScripts)
				{
					ExportScriptToFile(Data,
									StartEventArgs->ExistingFilesOnDisk[Data->EditorID]
									== SyncStartEventArgs::ExistingFileHandlingOperation::Overwrite);
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
				DoSync(false, Failed);


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
				this->TextBoxSelectedScriptLog = (gcnew System::Windows::Forms::TextBox());
				this->LabelSelectedScriptLog = (gcnew System::Windows::Forms::Label());
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
				this->TextBoxWorkingDir->Size = System::Drawing::Size(229, 20);
				this->TextBoxWorkingDir->TabIndex = 1;
				//
				// ButtonSelectWorkingDir
				//
				this->ButtonSelectWorkingDir->Location = System::Drawing::Point(752, 6);
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
				this->CheckboxAutoDeleteLogs->Size = System::Drawing::Size(255, 17);
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
				this->ListViewSyncedScripts->ContextMenuStrip = this->LVSyncedStripsContextMenu;
				this->ListViewSyncedScripts->Cursor = System::Windows::Forms::Cursors::Default;
				this->ListViewSyncedScripts->HideSelection = false;
				this->ListViewSyncedScripts->Location = System::Drawing::Point(15, 36);
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
				this->ColLastSyncTime->Width = 169;
				//
				// LVSyncedStripsContextMenu
				//
				this->LVSyncedStripsContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
					this->SyncToDiskToolStripMenuItem,
						this->SyncFromDiskToolStripMenuItem
				});
				this->LVSyncedStripsContextMenu->Name = L"LVSyncedStripsContextMenu";
				this->LVSyncedStripsContextMenu->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
				this->LVSyncedStripsContextMenu->ShowImageMargin = false;
				this->LVSyncedStripsContextMenu->Size = System::Drawing::Size(131, 76);
				//
				// SyncToDiskToolStripMenuItem
				//
				this->SyncToDiskToolStripMenuItem->Name = L"SyncToDiskToolStripMenuItem";
				this->SyncToDiskToolStripMenuItem->Size = System::Drawing::Size(130, 22);
				this->SyncToDiskToolStripMenuItem->Text = L"Sync To Disk";
				//
				// SyncFromDiskToolStripMenuItem
				//
				this->SyncFromDiskToolStripMenuItem->Name = L"SyncFromDiskToolStripMenuItem";
				this->SyncFromDiskToolStripMenuItem->Size = System::Drawing::Size(130, 22);
				this->SyncFromDiskToolStripMenuItem->Text = L"Sync From Disk";
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
				this->LabelSelectedScriptLog->Location = System::Drawing::Point(418, 36);
				this->LabelSelectedScriptLog->Name = L"LabelSelectedScriptLog";
				this->LabelSelectedScriptLog->Size = System::Drawing::Size(25, 13);
				this->LabelSelectedScriptLog->TabIndex = 12;
				this->LabelSelectedScriptLog->Text = L"Log";
				//
				// SESyncUI
				//
				this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
				this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
				this->ClientSize = System::Drawing::Size(830, 413);
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
				this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
				this->Name = L"SESyncUI";
				this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
				this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
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

			}


			//System::Object^ DiskSyncDialog::ListViewAspectScriptNameGetter(Object^ RowObject)
			//{
			//	return nullptr;
			//}

			//System::Object^ DiskSyncDialog::ListViewImageScriptNameGetter(Object^ RowObject)
			//{
			//	return nullptr;
			//}

			//System::Object^ DiskSyncDialog::ListViewAspectLastSyncTimeGetter(Object^ RowObject)
			//{
			//	return nullptr;
			//}

			//System::Object^ DiskSyncDialog::ListViewAspectToStringLastSyncTime(Object^ RowObject)
			//{
			//	return nullptr;
			//}

			DiskSyncDialog::DiskSyncDialog()
			{

			}

			DiskSyncDialog::~DiskSyncDialog()
			{
				if (components)
				{
					delete components;
				}
			}

		}

	}
}

