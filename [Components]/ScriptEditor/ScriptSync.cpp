#include "ScriptSync.h"
#include "SemanticAnalysis.h"
#include "[Common]\NativeWrapper.h"
#include "Preferences.h"
#include "IntelliSenseBackend.h"
#include "ScriptSelectionDialog.h"
#include "Preferences.h"
#include "IScriptEditorModel.h"
#include "ScriptPreprocessor.h"
#include "ScriptEditorViewImplComponents.h"

namespace cse
{


namespace scriptEditor
{


namespace scriptSync
{


SyncedScriptData::SyncedScriptData(String^ ScriptEditorID, String^ WorkingDir)
{
	auto ScriptFileExtension = preferences::SettingsHolder::Get()->General->ExportedScriptFileExtension;
	Debug::Assert(ScriptFileExtension != LogFileExtension);

	this->ScriptEditorID = ScriptEditorID;
	this->DiskFilePath = Path::Combine(WorkingDir, ScriptEditorID + ScriptFileExtension);
	this->LogFilePath = Path::Combine(WorkingDir, ScriptEditorID + LogFileExtension);
	this->LastSyncAttemptTimestamp = DateTime::Now;
	this->LastSyncAttemptDirection = eSyncDirection::None;
}

System::String^ SyncedScriptData::ReadFileContents()
{
	LastSyncAttemptTimestamp = DateTime::Now;
	LastSyncAttemptDirection = eSyncDirection::FromDisk;

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
	LastSyncAttemptDirection = eSyncDirection::ToDisk;

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

SyncStartEventArgs::SyncStartEventArgs(IEnumerable<SyncedScriptData^>^ SyncedScripts, IEnumerable<Tuple<String^, DateTime>^>^ ExistingScripts)
{
	SyncedScriptEditorIDs = gcnew List<String^>;
	ExistingFilesOnDisk = gcnew Dictionary<String^, eExistingFileHandlingOperation>;
	ExistingFileLastWriteTimestamps = gcnew Dictionary<String^, DateTime>;

	for each (SyncedScriptData ^ Data in SyncedScripts)
		this->SyncedScriptEditorIDs->Add(Data->EditorID);

	for each (auto Itr in ExistingScripts)
	{
		this->ExistingFilesOnDisk->Add(Itr->Item1,
			eExistingFileHandlingOperation::Overwrite);
		this->ExistingFileLastWriteTimestamps->Add(Itr->Item1, Itr->Item2);
	}
}

SyncWriteToDiskEventArgs::SyncWriteToDiskEventArgs(String^ ScriptEditorID, DateTime AccessTimestamp)
{
	this->ScriptEditorID = ScriptEditorID;
	this->AccessTimestamp = AccessTimestamp;
	this->Success = false;
}

SyncPreCompileEventArgs::SyncPreCompileEventArgs(String^ ScriptEditorID, String^ DiskFileContents, DateTime AccessTimestamp)
{
	this->ScriptEditorID = ScriptEditorID;
	this->DiskFileContents = DiskFileContents;
	this->AccessTimestamp = AccessTimestamp;
}

SyncPostCompileEventArgs::SyncPostCompileEventArgs(String^ ScriptEditorID)
{
	this->ScriptEditorID = ScriptEditorID;
	this->Success = false;
	this->OutputMessages = gcnew List<String^>();
}

SyncStopEventArgs::SyncStopEventArgs(UInt32 NumSyncedScripts, UInt32 NumFailedCompilations)
{
	this->NumSyncedScripts = NumSyncedScripts;
	this->NumFailedCompilations = NumFailedCompilations;
	this->RemoveLogFiles = false;
}

void DiskSync::DoSyncFromDiskLoop(bool Force, UInt32% OutFailedCompilations)
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
	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> NativeScript
	(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str()));
	Debug::Assert(NativeScript);

	auto EmbeddedMetadata = gcnew model::components::ScriptTextMetadata;
	String^ ScriptText = "";

	model::components::ScriptTextMetadataHelper::DeserializeRawScriptText(gcnew String(NativeScript->Text), ScriptText, EmbeddedMetadata);

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
		LogMessages->Add(FormatLogMessage(1, "Compiled successfully at " + DateTime::Now.ToString(), eLogMessageType::Info));
		LogMessages->Add(FormatLogMessage(1, "Elapsed Time: " + CompileTimer->Elapsed.Milliseconds + " milliseconds", eLogMessageType::Info));
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

	preprocessor::StandardOutputError^ Delegate()
	{
		return gcnew preprocessor::StandardOutputError(this, &PreprocessorErrorCapture::OnError);
	}
};

bool DiskSync::DoPreprocessingAndAnalysis(componentDLLInterface::ScriptData* Script,
										String^ ImportedScriptText,
										String^% OutPreprocessedText,
										bool% OutHasDirectives,
										List<String^>^% OutMessages)
{
	PreprocessorErrorCapture^ PreprocessorErrors = gcnew PreprocessorErrorCapture();

	auto PreprocessorData = gcnew preprocessor::PreprocessorParams(
		gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
		gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
		preferences::SettingsHolder::Get()->Preprocessor->AllowMacroRedefs,
		preferences::SettingsHolder::Get()->Preprocessor->NumPasses);


	bool PreprocessingSuccessful = preprocessor::Preprocessor::Get()->PreprocessScript(
								ImportedScriptText, OutPreprocessedText, PreprocessorErrors->Delegate(), PreprocessorData);

	if (PreprocessingSuccessful == false)
	{
		for each (Tuple<int, String^>^ Itr in PreprocessorErrors->Errors)
			OutMessages->Add(FormatLogMessage(Itr->Item1, Itr->Item2, eLogMessageType::Error));

		return false;
	}

	OutHasDirectives = PreprocessorData->ContainsDirectives;

	auto AnalysisParams = gcnew obScriptParsing::AnalysisData::Params;
	AnalysisParams->Ops = obScriptParsing::AnalysisData::eOperation::FillVariables
						| obScriptParsing::AnalysisData::eOperation::FillControlBlocks
						| obScriptParsing::AnalysisData::eOperation::PerformBasicValidation;

	if (preferences::SettingsHolder::Get()->Validator->CheckVarCommandNameCollisions)
		AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::CheckVariableNameCommandCollisions;
	if (preferences::SettingsHolder::Get()->Validator->CheckVarFormNameCollisions)
		AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::CheckVariableNameFormCollisions;
	if (preferences::SettingsHolder::Get()->Validator->CountVariableRefs)
		AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::CountVariableReferences;
	if (preferences::SettingsHolder::Get()->Validator->NoQuestVariableRefCounting)
		AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::SuppressQuestVariableRefCount;

	AnalysisParams->Type = obScriptParsing::eScriptType::Object;
	if (Script->Type == componentDLLInterface::ScriptData::kScriptType_Magic)
		AnalysisParams->Type = obScriptParsing::eScriptType::MagicEffect;
	else if (Script->Type == componentDLLInterface::ScriptData::kScriptType_Quest)
		AnalysisParams->Type = obScriptParsing::eScriptType::Quest;

	AnalysisParams->ScriptText = OutPreprocessedText;
	AnalysisParams->ScriptCommandIdentifiers = intellisense::IntelliSenseBackend::Get()->CreateIndentifierSnapshot(
		intellisense::eDatabaseLookupFilter::Command);

	auto Filter = intellisense::eDatabaseLookupFilter::All & ~intellisense::eDatabaseLookupFilter::Snippet;
	Filter = Filter & ~intellisense::eDatabaseLookupFilter::Command;
	AnalysisParams->FormIdentifiers = intellisense::IntelliSenseBackend::Get()->CreateIndentifierSnapshot(Filter);

	obScriptParsing::AnalysisData^ AnalysisResults = gcnew obScriptParsing::AnalysisData();
	AnalysisResults->PerformAnalysis(AnalysisParams);

	for each (auto Msg in AnalysisResults->AnalysisErrors)
		OutMessages->Add(FormatLogMessage(Msg->Line, Msg->Message, eLogMessageType::Error));

	for each (auto Msg in AnalysisResults->AnalysisWarnings)
		OutMessages->Add(FormatLogMessage(Msg->Line, Msg->Message, eLogMessageType::Warning));

	if (AnalysisResults->HasErrors || AnalysisResults->MalformedStructure)
		return false;

	String^ ScriptOrgEID = gcnew String(Script->EditorID);
	if (ScriptOrgEID->Equals(AnalysisResults->Name, StringComparison::CurrentCultureIgnoreCase) == false)
	{
		OutMessages->Add(FormatLogMessage(1,
										  "Script names can only be changed inside the script editor. Original name: " + ScriptOrgEID,
										  eLogMessageType::Error));
		return false;
	}

	return true;

}

void DiskSync::CompileScript(SyncedScriptData^ SyncedScript, String^ ImportedScriptText, bool% OutSuccess, List<String^>^% OutMessages)
{
	CString EID(SyncedScript->EditorID);
	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> NativeScript
						(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str()));
	Debug::Assert(NativeScript);

	String^ PreprocessedScriptText = "";
	bool ContainsPreprocessorDirectives = false;
	if (!DoPreprocessingAndAnalysis(NativeScript.get(),
									ImportedScriptText,
									PreprocessedScriptText,
									ContainsPreprocessorDirectives,
									OutMessages))
		return;

	auto CompilationData = gcnew model::components::ScriptCompilationData;
	CompilationData->PreprocessedScriptText = PreprocessedScriptText;
	CompilationData->UnpreprocessedScriptText = ImportedScriptText;
	CompilationData->HasPreprocessorDirectives = ContainsPreprocessorDirectives;
	CompilationData->CanCompile = true;

	auto Metadata = gcnew model::components::ScriptTextMetadata;
	Metadata->HasPreprocessorDirectives = ContainsPreprocessorDirectives;
	CompilationData->SerializedMetadata = model::components::ScriptTextMetadataHelper::SerializeMetadata(Metadata);

	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptCompileData> CompilationResult(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateCompileData());

	CString ScriptTextToCompile(CompilationData->PreprocessedScriptText->Replace("\n", "\r\n"));
	CompilationResult->Script.Text = ScriptTextToCompile.c_str();
	CompilationResult->Script.Type = NativeScript->Type;
	CompilationResult->Script.ParentForm = NativeScript->ParentForm;
	CompilationResult->PrintErrorsToConsole = false;

	OutSuccess = nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileScript(CompilationResult.get());
	if (OutSuccess)
	{
		String^ OriginalText = CompilationData->UnpreprocessedScriptText + CompilationData->SerializedMetadata;
		CString OrgScriptText(OriginalText);
		nativeWrapper::g_CSEInterfaceTable->ScriptEditor.SetScriptText(NativeScript->ParentForm, OrgScriptText.c_str());
	}

	for (int i = 0; i < CompilationResult->CompilerMessages.Count; i++)
	{
		auto& MessageData = CompilationResult->CompilerMessages.MessageListHead[i];
		String^ MessageStr = gcnew String(MessageData.Message);
		int Line = MessageData.Line;
		if (Line < 1)
			Line = 1;

		auto MessageType = eLogMessageType::Error;
		if (MessageData.IsWarning())
			MessageType = eLogMessageType::Warning;
		else if (MessageData.IsInfo())
			MessageType = eLogMessageType::Info;

		OutMessages->Add(FormatLogMessage(Line, MessageStr, MessageType));
	}
}


void DiskSync::WriteToConsoleContext(String^ Message)
{
	CString Msg(Message);
	nativeWrapper::g_CSEInterfaceTable->EditorAPI.PrintToConsoleContext(ConsoleMessageLogContext, Msg.c_str(), true);
}

System::String^ DiskSync::FormatLogMessage(int Line, String^ Message, eLogMessageType Type)
{
	if (Line < 1)
		Line = 1;

	String^ MessageTag;
	switch (Type)
	{
	case eLogMessageType::Error:
		MessageTag = "E";
		break;
	case eLogMessageType::Warning:
		MessageTag = "W";
		break;
	case eLogMessageType::Info:
		MessageTag = "I";
		break;
	default:
		MessageTag = "<unknown>";
		break;
	}

	return String::Format("[{0}]\tLine {1}\t{2}", MessageTag, Line, Message);
}

void DiskSync::SyncTimer_Tick(Object^ Sender, EventArgs^ E)
{
	if (SyncInProgress)
	{
		UInt32 Failed = 0;
		DoSyncFromDiskLoop(false, Failed);
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

bool DiskSync::Start(String^ WorkingDir, List<String^>^ SyncedScriptEditorIDs)
{
	if (SyncInProgress)
	{
		DebugPrint("Script sync already in progress!", true);
		return false;
	}

	this->WorkingDir = WorkingDir;
	if (!System::IO::Directory::Exists(WorkingDir))
	{
		DebugPrint("Invalid working directory!", true);
		return false;
	}

	auto Existing = gcnew List<Tuple<String ^, DateTime> ^>;
	for each (String ^ EID in SyncedScriptEditorIDs)
	{

		SyncedScriptData^ Data = gcnew SyncedScriptData(EID, WorkingDir);
		this->SyncedScripts->Add(Data);
		this->EditorIDsToSyncedData->Add(EID, Data);

		if (File::Exists(Data->FilePath))
			Existing->Add(gcnew Tuple<String ^, DateTime>(EID, System::IO::File::GetLastWriteTime(Data->FilePath)));
	}

	SyncInProgress = true;

	SyncStartEventArgs^ StartEventArgs = gcnew SyncStartEventArgs(SyncedScripts, Existing);
	SyncStart(this, StartEventArgs);

	WriteToConsoleContext("Syncing === STARTED!");
	for each (SyncedScriptData ^ Data in SyncedScripts)
	{
		WriteToConsoleContext("Writing script '" + Data->EditorID + " to disk @ " + Data->FilePath);
		bool Overwrite = StartEventArgs->ExistingFilesOnDisk->ContainsKey(Data->EditorID) &&
						StartEventArgs->ExistingFilesOnDisk[Data->EditorID] == SyncStartEventArgs::eExistingFileHandlingOperation::Overwrite;

		if (!SyncToDisk(Data, Overwrite))
			WriteToConsoleContext("\tError writing to disk file for the first time");

		auto InitLogMessages = gcnew List<String^>;
		InitLogMessages->Add("");
		Data->WriteLog(InitLogMessages);
	}

	return true;
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
	DoSyncFromDiskLoop(false, Failed);

	SyncInProgress = false;

	SyncStopEventArgs^ StopEventArgs = gcnew SyncStopEventArgs(SyncedScripts->Count, Failed);
	SyncStop(this, StopEventArgs);

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

SyncedScriptListViewWrapper::SyncedScriptListViewWrapper(String^ EditorID)
{
	this->EditorID = EditorID;
	this->LastOutputMessages = gcnew List<String^>;
	Reset();
}

void SyncedScriptListViewWrapper::Reset()
{
	LastSyncTime = DateTime();
	LastSyncSuccess = eSuccessState::None;
	LastOutputMessages->Clear();
}

void DiskSyncDialog::InitializeComponent(void)
{
	this->components = (gcnew System::ComponentModel::Container());
	auto resources = (gcnew System::Resources::ResourceManager("ScriptEditor.ScriptSyncDialog", Assembly::GetExecutingAssembly()));
	this->ButtonOpenWorkingDir = (gcnew DevComponents::DotNetBar::ButtonX());
	this->ColLastSyncTime = (gcnew BrightIdeasSoftware::OLVColumn());
	this->ColScriptName = (gcnew BrightIdeasSoftware::OLVColumn());
	this->LabelSelectedScriptLog = (gcnew DevComponents::DotNetBar::LabelX());
	this->ListViewSyncedScripts = (gcnew BrightIdeasSoftware::FastObjectListView());
	this->ButtonStartStopSync = (gcnew DevComponents::DotNetBar::ButtonX());
	this->ButtonSelectWorkingDir = (gcnew DevComponents::DotNetBar::ButtonX());
	this->FolderDlgWorkingDir = (gcnew System::Windows::Forms::FolderBrowserDialog());
	this->LabelWorkingDir = (gcnew DevComponents::DotNetBar::LabelX());
	this->GroupSyncSettings = (gcnew DevComponents::DotNetBar::Controls::GroupPanel());
	this->NumericAutoSyncSeconds = (gcnew DevComponents::Editors::IntegerInput());
	this->GroupStartupFileHandling = (gcnew DevComponents::DotNetBar::Controls::GroupPanel());
	this->RadioPromptForFileHandling = (gcnew System::Windows::Forms::RadioButton());
	this->RadioUseExistingFiles = (gcnew System::Windows::Forms::RadioButton());
	this->RadioOverwriteExistingFiles = (gcnew System::Windows::Forms::RadioButton());
	this->CheckboxAutoDeleteLogs = (gcnew System::Windows::Forms::CheckBox());
	this->LabelSeconds = (gcnew DevComponents::DotNetBar::LabelX());
	this->CheckboxAutoSync = (gcnew System::Windows::Forms::CheckBox());
	this->TextBoxSelectedScriptLog = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
	this->TextBoxWorkingDir = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
	this->LeftPanel = (gcnew DevComponents::DotNetBar::PanelEx());
	this->Toolbar = (gcnew DevComponents::DotNetBar::Bar());
	this->ToolbarLabelSyncedScripts = (gcnew DevComponents::DotNetBar::LabelItem());
	this->ToolbarSelectScripts = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->ToolbarSyncToDisk = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->ToolbarSyncFromDisk = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->ToolbarOpenLog = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->ToolbarOpenSyncedFile = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->DeferredSelectionUpdateTimer = (gcnew System::Windows::Forms::Timer(this->components));
	this->ColorManager = (gcnew DevComponents::DotNetBar::StyleManagerAmbient(this->components));
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->BeginInit();
	this->GroupSyncSettings->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->BeginInit();
	this->GroupStartupFileHandling->SuspendLayout();
	this->LeftPanel->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Toolbar))->BeginInit();
	this->SuspendLayout();
	//
	// ButtonOpenWorkingDir
	//
	this->ButtonOpenWorkingDir->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
	this->ButtonOpenWorkingDir->Location = System::Drawing::Point(791, 8);
	this->ButtonOpenWorkingDir->Name = L"ButtonOpenWorkingDir";
	this->ButtonOpenWorkingDir->Size = System::Drawing::Size(45, 20);
	this->ButtonOpenWorkingDir->TabIndex = 24;
	this->ButtonOpenWorkingDir->Text = L"Open";
	//
	// ColLastSyncTime
	//
	this->ColLastSyncTime->Sortable = false;
	this->ColLastSyncTime->Text = L"Last Sync Attempt Time";
	this->ColLastSyncTime->Width = 152;
	//
	// ColScriptName
	//
	this->ColScriptName->Sortable = false;
	this->ColScriptName->Text = L"EditorID";
	this->ColScriptName->Width = 195;
	//
	// LabelSelectedScriptLog
	//
	this->LabelSelectedScriptLog->AutoSize = true;
	this->LabelSelectedScriptLog->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	//
	//
	//
	this->LabelSelectedScriptLog->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->LabelSelectedScriptLog->ForeColor = System::Drawing::Color::Black;
	this->LabelSelectedScriptLog->Location = System::Drawing::Point(435, 36);
	this->LabelSelectedScriptLog->Name = L"LabelSelectedScriptLog";
	this->LabelSelectedScriptLog->Size = System::Drawing::Size(21, 17);
	this->LabelSelectedScriptLog->TabIndex = 23;
	this->LabelSelectedScriptLog->Text = L"Log";
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
	this->ListViewSyncedScripts->Dock = System::Windows::Forms::DockStyle::Fill;
	this->ListViewSyncedScripts->EmptyListMsg = L"Doesn\'t look like anything to me...";
	this->ListViewSyncedScripts->EmptyListMsgFont = (gcnew System::Drawing::Font(L"Segoe UI caps", 9.75F, System::Drawing::FontStyle::Regular,
		System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
	this->ColorManager->SetEnableAmbientSettings(this->ListViewSyncedScripts, DevComponents::DotNetBar::eAmbientSettings::All);
	this->ListViewSyncedScripts->FullRowSelect = true;
	this->ListViewSyncedScripts->HideSelection = false;
	this->ListViewSyncedScripts->Location = System::Drawing::Point(0, 37);
	this->ListViewSyncedScripts->Name = L"ListViewSyncedScripts";
	this->ListViewSyncedScripts->ShowGroups = false;
	this->ListViewSyncedScripts->Size = System::Drawing::Size(400, 323);
	this->ListViewSyncedScripts->TabIndex = 21;
	this->ListViewSyncedScripts->UseCompatibleStateImageBehavior = false;
	this->ListViewSyncedScripts->View = System::Windows::Forms::View::Details;
	this->ListViewSyncedScripts->VirtualMode = true;
	//
	// ButtonStartStopSync
	//
	this->ButtonStartStopSync->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
	this->ButtonStartStopSync->Location = System::Drawing::Point(12, 374);
	this->ButtonStartStopSync->Name = L"ButtonStartStopSync";
	this->ButtonStartStopSync->Size = System::Drawing::Size(400, 32);
	this->ButtonStartStopSync->TabIndex = 18;
	this->ButtonStartStopSync->Text = L"Start Syncing";
	//
	// ButtonSelectWorkingDir
	//
	this->ButtonSelectWorkingDir->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
	this->ButtonSelectWorkingDir->Location = System::Drawing::Point(718, 8);
	this->ButtonSelectWorkingDir->Name = L"ButtonSelectWorkingDir";
	this->ButtonSelectWorkingDir->Size = System::Drawing::Size(67, 20);
	this->ButtonSelectWorkingDir->TabIndex = 16;
	this->ButtonSelectWorkingDir->Text = L"Browse...";
	//
	// LabelWorkingDir
	//
	this->LabelWorkingDir->AutoSize = true;
	this->LabelWorkingDir->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(242)));
	//
	//
	//
	this->LabelWorkingDir->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->LabelWorkingDir->ForeColor = System::Drawing::Color::Black;
	this->LabelWorkingDir->Location = System::Drawing::Point(435, 8);
	this->LabelWorkingDir->Name = L"LabelWorkingDir";
	this->LabelWorkingDir->Size = System::Drawing::Size(91, 17);
	this->LabelWorkingDir->TabIndex = 14;
	this->LabelWorkingDir->Text = L"Working Directory";
	//
	// GroupSyncSettings
	//
	this->GroupSyncSettings->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	this->GroupSyncSettings->CanvasColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	this->GroupSyncSettings->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->GroupSyncSettings->Controls->Add(this->NumericAutoSyncSeconds);
	this->GroupSyncSettings->Controls->Add(this->GroupStartupFileHandling);
	this->GroupSyncSettings->Controls->Add(this->CheckboxAutoDeleteLogs);
	this->GroupSyncSettings->Controls->Add(this->LabelSeconds);
	this->GroupSyncSettings->Controls->Add(this->CheckboxAutoSync);
	this->GroupSyncSettings->DisabledBackColor = System::Drawing::Color::Empty;
	this->GroupSyncSettings->Location = System::Drawing::Point(435, 243);
	this->GroupSyncSettings->Name = L"GroupSyncSettings";
	this->GroupSyncSettings->Size = System::Drawing::Size(401, 163);
	//
	//
	//
	this->GroupSyncSettings->Style->BackColor2SchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground2;
	this->GroupSyncSettings->Style->BackColorGradientAngle = 90;
	this->GroupSyncSettings->Style->BackColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
	this->GroupSyncSettings->Style->BorderBottom = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupSyncSettings->Style->BorderBottomWidth = 1;
	this->GroupSyncSettings->Style->BorderColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
	this->GroupSyncSettings->Style->BorderLeft = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupSyncSettings->Style->BorderLeftWidth = 1;
	this->GroupSyncSettings->Style->BorderRight = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupSyncSettings->Style->BorderRightWidth = 1;
	this->GroupSyncSettings->Style->BorderTop = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupSyncSettings->Style->BorderTopWidth = 1;
	this->GroupSyncSettings->Style->CornerDiameter = 4;
	this->GroupSyncSettings->Style->CornerType = DevComponents::DotNetBar::eCornerType::Rounded;
	this->GroupSyncSettings->Style->TextAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Center;
	this->GroupSyncSettings->Style->TextColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
	this->GroupSyncSettings->Style->TextLineAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Near;
	//
	//
	//
	this->GroupSyncSettings->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	//
	//
	//
	this->GroupSyncSettings->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->GroupSyncSettings->TabIndex = 26;
	this->GroupSyncSettings->Text = L"Sync Settings";
	//
	// NumericAutoSyncSeconds
	//
	this->NumericAutoSyncSeconds->BackColor = System::Drawing::Color::White;
	//
	//
	//
	this->NumericAutoSyncSeconds->BackgroundStyle->Class = L"DateTimeInputBackground";
	this->NumericAutoSyncSeconds->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->NumericAutoSyncSeconds->ForeColor = System::Drawing::Color::Black;
	this->NumericAutoSyncSeconds->Location = System::Drawing::Point(214, 3);
	this->NumericAutoSyncSeconds->MaxValue = 60;
	this->NumericAutoSyncSeconds->MinValue = 1;
	this->NumericAutoSyncSeconds->Name = L"NumericAutoSyncSeconds";
	this->NumericAutoSyncSeconds->ShowUpDown = true;
	this->NumericAutoSyncSeconds->Size = System::Drawing::Size(63, 22);
	this->NumericAutoSyncSeconds->TabIndex = 27;
	this->NumericAutoSyncSeconds->Value = 1;
	//
	// GroupStartupFileHandling
	//
	this->GroupStartupFileHandling->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	this->GroupStartupFileHandling->CanvasColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	this->GroupStartupFileHandling->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->GroupStartupFileHandling->Controls->Add(this->RadioPromptForFileHandling);
	this->GroupStartupFileHandling->Controls->Add(this->RadioUseExistingFiles);
	this->GroupStartupFileHandling->Controls->Add(this->RadioOverwriteExistingFiles);
	this->GroupStartupFileHandling->DisabledBackColor = System::Drawing::Color::Empty;
	this->GroupStartupFileHandling->Location = System::Drawing::Point(3, 32);
	this->GroupStartupFileHandling->Name = L"GroupStartupFileHandling";
	this->GroupStartupFileHandling->Size = System::Drawing::Size(389, 71);
	//
	//
	//
	this->GroupStartupFileHandling->Style->BackColor2SchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground2;
	this->GroupStartupFileHandling->Style->BackColorGradientAngle = 90;
	this->GroupStartupFileHandling->Style->BackColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
	this->GroupStartupFileHandling->Style->BorderBottom = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupStartupFileHandling->Style->BorderBottomWidth = 1;
	this->GroupStartupFileHandling->Style->BorderColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
	this->GroupStartupFileHandling->Style->BorderLeft = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupStartupFileHandling->Style->BorderLeftWidth = 1;
	this->GroupStartupFileHandling->Style->BorderRight = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupStartupFileHandling->Style->BorderRightWidth = 1;
	this->GroupStartupFileHandling->Style->BorderTop = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupStartupFileHandling->Style->BorderTopWidth = 1;
	this->GroupStartupFileHandling->Style->CornerDiameter = 4;
	this->GroupStartupFileHandling->Style->CornerType = DevComponents::DotNetBar::eCornerType::Rounded;
	this->GroupStartupFileHandling->Style->TextAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Center;
	this->GroupStartupFileHandling->Style->TextColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
	this->GroupStartupFileHandling->Style->TextLineAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Near;
	//
	//
	//
	this->GroupStartupFileHandling->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	//
	//
	//
	this->GroupStartupFileHandling->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->GroupStartupFileHandling->TabIndex = 27;
	this->GroupStartupFileHandling->Text = L"Existing File Handling During Startup";
	//
	// RadioPromptForFileHandling
	//
	this->RadioPromptForFileHandling->AutoSize = true;
	this->RadioPromptForFileHandling->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	this->RadioPromptForFileHandling->ForeColor = System::Drawing::Color::Black;
	this->RadioPromptForFileHandling->Location = System::Drawing::Point(14, 26);
	this->RadioPromptForFileHandling->Name = L"RadioPromptForFileHandling";
	this->RadioPromptForFileHandling->Size = System::Drawing::Size(126, 17);
	this->RadioPromptForFileHandling->TabIndex = 5;
	this->RadioPromptForFileHandling->TabStop = true;
	this->RadioPromptForFileHandling->Text = L"Prompt for each file";
	this->RadioPromptForFileHandling->UseVisualStyleBackColor = false;
	//
	// RadioUseExistingFiles
	//
	this->RadioUseExistingFiles->AutoSize = true;
	this->RadioUseExistingFiles->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	this->RadioUseExistingFiles->ForeColor = System::Drawing::Color::Black;
	this->RadioUseExistingFiles->Location = System::Drawing::Point(219, 3);
	this->RadioUseExistingFiles->Name = L"RadioUseExistingFiles";
	this->RadioUseExistingFiles->Size = System::Drawing::Size(161, 17);
	this->RadioUseExistingFiles->TabIndex = 4;
	this->RadioUseExistingFiles->TabStop = true;
	this->RadioUseExistingFiles->Text = L"Use without modifications";
	this->RadioUseExistingFiles->UseVisualStyleBackColor = false;
	//
	// RadioOverwriteExistingFiles
	//
	this->RadioOverwriteExistingFiles->AutoSize = true;
	this->RadioOverwriteExistingFiles->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	this->RadioOverwriteExistingFiles->ForeColor = System::Drawing::Color::Black;
	this->RadioOverwriteExistingFiles->Location = System::Drawing::Point(14, 3);
	this->RadioOverwriteExistingFiles->Name = L"RadioOverwriteExistingFiles";
	this->RadioOverwriteExistingFiles->Size = System::Drawing::Size(191, 17);
	this->RadioOverwriteExistingFiles->TabIndex = 3;
	this->RadioOverwriteExistingFiles->TabStop = true;
	this->RadioOverwriteExistingFiles->Text = L"Overwrite with plugin script text";
	this->RadioOverwriteExistingFiles->UseVisualStyleBackColor = false;
	//
	// CheckboxAutoDeleteLogs
	//
	this->CheckboxAutoDeleteLogs->AutoSize = true;
	this->CheckboxAutoDeleteLogs->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	this->CheckboxAutoDeleteLogs->ForeColor = System::Drawing::Color::Black;
	this->CheckboxAutoDeleteLogs->Location = System::Drawing::Point(16, 114);
	this->CheckboxAutoDeleteLogs->Name = L"CheckboxAutoDeleteLogs";
	this->CheckboxAutoDeleteLogs->Size = System::Drawing::Size(276, 17);
	this->CheckboxAutoDeleteLogs->TabIndex = 9;
	this->CheckboxAutoDeleteLogs->Text = L"Automatically delete log files when syncing ends";
	this->CheckboxAutoDeleteLogs->UseVisualStyleBackColor = false;
	//
	// LabelSeconds
	//
	this->LabelSeconds->AutoSize = true;
	this->LabelSeconds->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(242)));
	//
	//
	//
	this->LabelSeconds->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->LabelSeconds->ForeColor = System::Drawing::Color::Black;
	this->LabelSeconds->Location = System::Drawing::Point(283, 5);
	this->LabelSeconds->Name = L"LabelSeconds";
	this->LabelSeconds->Size = System::Drawing::Size(42, 17);
	this->LabelSeconds->TabIndex = 7;
	this->LabelSeconds->Text = L"seconds";
	//
	// CheckboxAutoSync
	//
	this->CheckboxAutoSync->AutoSize = true;
	this->CheckboxAutoSync->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
		static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
	this->CheckboxAutoSync->ForeColor = System::Drawing::Color::Black;
	this->CheckboxAutoSync->Location = System::Drawing::Point(16, 5);
	this->CheckboxAutoSync->Name = L"CheckboxAutoSync";
	this->CheckboxAutoSync->Size = System::Drawing::Size(195, 17);
	this->CheckboxAutoSync->TabIndex = 5;
	this->CheckboxAutoSync->Text = L"Automatically sync changes every";
	this->CheckboxAutoSync->UseVisualStyleBackColor = false;
	//
	// TextBoxSelectedScriptLog
	//
	this->TextBoxSelectedScriptLog->BackColor = System::Drawing::Color::White;
	//
	//
	//
	this->TextBoxSelectedScriptLog->Border->Class = L"TextBoxBorder";
	this->TextBoxSelectedScriptLog->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->TextBoxSelectedScriptLog->DisabledBackColor = System::Drawing::Color::Black;
	this->TextBoxSelectedScriptLog->ForeColor = System::Drawing::Color::Black;
	this->TextBoxSelectedScriptLog->Location = System::Drawing::Point(435, 59);
	this->TextBoxSelectedScriptLog->Multiline = true;
	this->TextBoxSelectedScriptLog->Name = L"TextBoxSelectedScriptLog";
	this->TextBoxSelectedScriptLog->PreventEnterBeep = true;
	this->TextBoxSelectedScriptLog->ReadOnly = true;
	this->TextBoxSelectedScriptLog->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
	this->TextBoxSelectedScriptLog->Size = System::Drawing::Size(401, 178);
	this->TextBoxSelectedScriptLog->TabIndex = 27;
	//
	// TextBoxWorkingDir
	//
	this->TextBoxWorkingDir->BackColor = System::Drawing::Color::White;
	//
	//
	//
	this->TextBoxWorkingDir->Border->Class = L"TextBoxBorder";
	this->TextBoxWorkingDir->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->TextBoxWorkingDir->DisabledBackColor = System::Drawing::Color::White;
	this->TextBoxWorkingDir->ForeColor = System::Drawing::Color::Black;
	this->TextBoxWorkingDir->Location = System::Drawing::Point(532, 8);
	this->TextBoxWorkingDir->Name = L"TextBoxWorkingDir";
	this->TextBoxWorkingDir->PreventEnterBeep = true;
	this->TextBoxWorkingDir->Size = System::Drawing::Size(180, 22);
	this->TextBoxWorkingDir->TabIndex = 28;
	//
	// LeftPanel
	//
	this->LeftPanel->CanvasColor = System::Drawing::SystemColors::Control;
	this->LeftPanel->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->LeftPanel->Controls->Add(this->ListViewSyncedScripts);
	this->LeftPanel->Controls->Add(this->Toolbar);
	this->LeftPanel->DisabledBackColor = System::Drawing::Color::Empty;
	this->LeftPanel->Location = System::Drawing::Point(12, 6);
	this->LeftPanel->Name = L"LeftPanel";
	this->LeftPanel->Size = System::Drawing::Size(400, 360);
	this->LeftPanel->Style->Alignment = System::Drawing::StringAlignment::Center;
	this->LeftPanel->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
	this->LeftPanel->Style->Border = DevComponents::DotNetBar::eBorderType::SingleLine;
	this->LeftPanel->Style->BorderColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
	this->LeftPanel->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
	this->LeftPanel->Style->GradientAngle = 90;
	this->LeftPanel->TabIndex = 29;
	//
	// Toolbar
	//
	this->Toolbar->AntiAlias = true;
	this->Toolbar->Dock = System::Windows::Forms::DockStyle::Top;
	this->Toolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
	this->Toolbar->IsMaximized = false;
	this->Toolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(6) {
		this->ToolbarLabelSyncedScripts,
			this->ToolbarSelectScripts, this->ToolbarSyncToDisk, this->ToolbarSyncFromDisk, this->ToolbarOpenLog, this->ToolbarOpenSyncedFile
	});
	this->Toolbar->ItemSpacing = 5;
	this->Toolbar->Location = System::Drawing::Point(0, 0);
	this->Toolbar->Name = L"Toolbar";
	this->Toolbar->PaddingBottom = 5;
	this->Toolbar->PaddingLeft = 5;
	this->Toolbar->PaddingRight = 5;
	this->Toolbar->PaddingTop = 5;
	this->Toolbar->Size = System::Drawing::Size(400, 37);
	this->Toolbar->Stretch = true;
	this->Toolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->Toolbar->TabIndex = 22;
	this->Toolbar->TabStop = false;
	this->Toolbar->Text = L"bar1";
	//
	// ToolbarLabelSyncedScripts
	//
	this->ToolbarLabelSyncedScripts->Name = L"ToolbarLabelSyncedScripts";
	this->ToolbarLabelSyncedScripts->Text = L"Synced Scripts";
	//
	// ToolbarSelectScripts
	//
	this->ToolbarSelectScripts->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarSelectScripts.Image")));
	this->ToolbarSelectScripts->ImagePaddingHorizontal = 10;
	this->ToolbarSelectScripts->ImagePaddingVertical = 10;
	this->ToolbarSelectScripts->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
	this->ToolbarSelectScripts->Name = L"ToolbarSelectScripts";
	this->ToolbarSelectScripts->Text = L"Select Scripts...";
	this->ToolbarSelectScripts->Tooltip = L"Select Scripts...";
	//
	// ToolbarSyncToDisk
	//
	this->ToolbarSyncToDisk->BeginGroup = true;
	this->ToolbarSyncToDisk->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarSyncToDisk.Image")));
	this->ToolbarSyncToDisk->ImagePaddingHorizontal = 10;
	this->ToolbarSyncToDisk->ImagePaddingVertical = 10;
	this->ToolbarSyncToDisk->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
	this->ToolbarSyncToDisk->Name = L"ToolbarSyncToDisk";
	this->ToolbarSyncToDisk->Text = L"Sync to Disk";
	this->ToolbarSyncToDisk->Tooltip = L"Sync to Disk";
	//
	// ToolbarSyncFromDisk
	//
	this->ToolbarSyncFromDisk->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarSyncFromDisk.Image")));
	this->ToolbarSyncFromDisk->ImagePaddingHorizontal = 10;
	this->ToolbarSyncFromDisk->ImagePaddingVertical = 10;
	this->ToolbarSyncFromDisk->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
	this->ToolbarSyncFromDisk->Name = L"ToolbarSyncFromDisk";
	this->ToolbarSyncFromDisk->Text = L"Sync from Disk";
	this->ToolbarSyncFromDisk->Tooltip = L"Sync from Disk";
	//
	// ToolbarOpenLog
	//
	this->ToolbarOpenLog->BeginGroup = true;
	this->ToolbarOpenLog->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarOpenLog.Image")));
	this->ToolbarOpenLog->ImagePaddingHorizontal = 10;
	this->ToolbarOpenLog->ImagePaddingVertical = 10;
	this->ToolbarOpenLog->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
	this->ToolbarOpenLog->Name = L"ToolbarOpenLog";
	this->ToolbarOpenLog->Text = L"Open Log";
	this->ToolbarOpenLog->Tooltip = L"Open Log";
	//
	// ToolbarOpenSyncedFile
	//
	this->ToolbarOpenSyncedFile->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarOpenSyncedFile.Image")));
	this->ToolbarOpenSyncedFile->ImagePaddingHorizontal = 10;
	this->ToolbarOpenSyncedFile->ImagePaddingVertical = 10;
	this->ToolbarOpenSyncedFile->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
	this->ToolbarOpenSyncedFile->Name = L"ToolbarOpenSyncedFile";
	this->ToolbarOpenSyncedFile->Text = L"Open Synced File";
	this->ToolbarOpenSyncedFile->Tooltip = L"Open Synced File";
	//
	// ScriptSyncDialog
	//
	this->ClientSize = System::Drawing::Size(852, 413);
	this->Controls->Add(this->LeftPanel);
	this->Controls->Add(this->TextBoxWorkingDir);
	this->Controls->Add(this->TextBoxSelectedScriptLog);
	this->Controls->Add(this->GroupSyncSettings);
	this->Controls->Add(this->ButtonOpenWorkingDir);
	this->Controls->Add(this->LabelSelectedScriptLog);
	this->Controls->Add(this->ButtonStartStopSync);
	this->Controls->Add(this->ButtonSelectWorkingDir);
	this->Controls->Add(this->LabelWorkingDir);
	this->StartPosition = FormStartPosition::CenterScreen;
	this->DoubleBuffered = true;
	this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
	this->MaximizeBox = false;
	this->Name = L"ScriptSyncDialog";
	this->Text = L"Sync Scripts To Disk";
	this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->EndInit();
	this->GroupSyncSettings->ResumeLayout(false);
	this->GroupSyncSettings->PerformLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->EndInit();
	this->GroupStartupFileHandling->ResumeLayout(false);
	this->GroupStartupFileHandling->PerformLayout();
	this->LeftPanel->ResumeLayout(false);
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Toolbar))->EndInit();
	this->ResumeLayout(false);
}

void DiskSyncDialog::FinalizeComponents()
{
	utilities::DisableFormAutoScale(this);
	SetDefaultFont(preferences::SettingsHolder::Get()->Appearance->UIFont);

	this->ColScriptName->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&DiskSyncDialog::ListViewAspectScriptNameGetter);
	this->ColScriptName->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&DiskSyncDialog::ListViewImageScriptNameGetter);
	this->ColLastSyncTime->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&DiskSyncDialog::ListViewAspectLastSyncTimeGetter);
	this->ColLastSyncTime->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&DiskSyncDialog::ListViewAspectToStringLastSyncTime);

	SyncedScripts = gcnew Dictionary<String ^, SyncedScriptListViewWrapper ^>;

	ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &DiskSyncDialog::ScriptEditorPreferences_Saved);
	DiskSyncSyncStartHandler = gcnew SyncStartEventHandler(this, &DiskSyncDialog::DiskSync_SyncStart);
	DiskSyncSyncStopHandler = gcnew SyncStopEventHandler(this, &DiskSyncDialog::DiskSync_SyncStop);
	DiskSyncSyncWriteToDiskHandler = gcnew SyncWriteToDiskEventHandler(this, &DiskSyncDialog::DiskSync_SyncWriteToDisk);
	DiskSyncSyncPreCompileHandler = gcnew SyncPreCompileEventHandler(this, &DiskSyncDialog::DiskSync_SyncPreCompile);
	DiskSyncSyncPostCompileHandler = gcnew SyncPostCompileEventHandler(this, &DiskSyncDialog::DiskSync_SyncPostCompile);

	ButtonSelectWorkingDir->Click += gcnew EventHandler(this, &DiskSyncDialog::ButtonSelectWorkingDir_Click);
	ButtonOpenWorkingDir->Click += gcnew EventHandler(this, &DiskSyncDialog::ButtonOpenWorkingDir_Click);
	ButtonStartStopSync->Click += gcnew EventHandler(this, &DiskSyncDialog::ButtonStartStopSync_Click);

	ToolbarSelectScripts->Click += gcnew EventHandler(this, &DiskSyncDialog::ToolbarSelectScripts_Click);
	ToolbarSyncToDisk->Click += gcnew EventHandler(this, &DiskSyncDialog::ToolbarSyncToDisk_Click);
	ToolbarSyncFromDisk->Click += gcnew EventHandler(this, &DiskSyncDialog::ToolbarSyncFromDisk_Click);
	ToolbarOpenLog->Click += gcnew EventHandler(this, &DiskSyncDialog::ToolbarOpenLog_Click);
	ToolbarOpenSyncedFile->Click += gcnew EventHandler(this, &DiskSyncDialog::ToolbarOpenSyncedFile_Click);
	CheckboxAutoSync->Click += gcnew EventHandler(this, &DiskSyncDialog::CheckboxAutoSync_Click);

	NumericAutoSyncSeconds->ValueChanged += gcnew EventHandler(this, &DiskSyncDialog::NumericAutoSyncSeconds_ValueChanged);
	ListViewSyncedScripts->SelectedIndexChanged += gcnew EventHandler(this, &DiskSyncDialog::ListViewSyncedScripts_SelectionChanged);
	// ### The SelectedIndexChanged event does not update the SelectedObjects collection when the Shift-key selection triggers the event
	// ### We need to workaround this by deferring the original handler by using a timer
	DeferredSelectionUpdateTimer->Tick += gcnew EventHandler(this, &DiskSyncDialog::DeferredSelectionUpdateTimer_Tick);

	this->Closing += gcnew CancelEventHandler(this, &DiskSyncDialog::Dialog_Cancel);
	preferences::SettingsHolder::Get()->PreferencesChanged += ScriptEditorPreferencesSavedHandler;

	this->ListViewSyncedScripts->SmallImageList = gcnew ImageList();
	this->ListViewSyncedScripts->SmallImageList->ImageSize = Drawing::Size(14, 14);
	this->ListViewSyncedScripts->SmallImageList->Images->Add(view::components::CommonIcons::Get()->Transparent);
	this->ListViewSyncedScripts->SmallImageList->Images->Add(view::components::CommonIcons::Get()->InProgress);
	this->ListViewSyncedScripts->SmallImageList->Images->Add(view::components::CommonIcons::Get()->Success);
	this->ListViewSyncedScripts->SmallImageList->Images->Add(view::components::CommonIcons::Get()->Error);

	ListViewThemeWrapper = gcnew viewImpl::components::ObjectListView(ListViewSyncedScripts, view::eViewRole::None, nullptr);

	UpdateToolbarEnabledState();
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

void DiskSyncDialog::ToolbarSelectScripts_Click(Object^ Sender, EventArgs^ E)
{
	Debug::Assert(IsSyncInProgress() == false);

	auto Params = gcnew selectScript::ScriptSelectionDialog::Params;
	Params->ShowDeletedScripts = false;
	Params->PreventSyncedScriptSelection = true;
	Params->ParentWindowHandle = this->Handle;

	selectScript::ScriptSelectionDialog ScriptSelection(Params);
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
		MessageBox::Show("At least one script must to be selected to begin syncing.", view::IScriptEditorView::MainWindowDefaultTitle, MessageBoxButtons::OK, MessageBoxIcon::Information);
		return;
	}

	String^ WorkingDir = TextBoxWorkingDir->Text->TrimEnd();
	if (WorkingDir == "")
	{
		MessageBox::Show("A working directory must be selected to begin syncing.", view::IScriptEditorView::MainWindowDefaultTitle, MessageBoxButtons::OK, MessageBoxIcon::Information);
		return;
	}
	else if (!System::IO::Directory::Exists(WorkingDir))
	{
		MessageBox::Show("The selected working directory does not exist.", view::IScriptEditorView::MainWindowDefaultTitle, MessageBoxButtons::OK, MessageBoxIcon::Error);
		return;
	}

	DiskSync::Get()->Start(WorkingDir, SyncedEditorIDs);
}

void DiskSyncDialog::ToolbarSyncToDisk_Click(Object^ Sender, EventArgs^ E)
{
	Debug::Assert(IsSyncInProgress());

	for each (auto Itr in ListViewSyncedScripts->SelectedObjects)
	{
		auto Selection = safe_cast<SyncedScriptListViewWrapper^>(Itr);
		DiskSync::Get()->ForceSyncToDisk(Selection->EditorID);
	}
}

void DiskSyncDialog::ToolbarSyncFromDisk_Click(Object^ Sender, EventArgs^ E)
{
	Debug::Assert(IsSyncInProgress());

	for each (auto Itr in ListViewSyncedScripts->SelectedObjects)
	{
		auto Selection = safe_cast<SyncedScriptListViewWrapper^>(Itr);
		DiskSync::Get()->ForceSyncFromDisk(Selection->EditorID);
	}
}

void DiskSyncDialog::ToolbarOpenLog_Click(Object^ Sender, EventArgs^ E)
{
	for each (auto Itr in ListViewSyncedScripts->SelectedObjects)
	{
		auto Selection = safe_cast<SyncedScriptListViewWrapper^>(Itr);
		DiskSync::Get()->OpenLogFile(Selection->EditorID);
	}
}

void DiskSyncDialog::ToolbarOpenSyncedFile_Click(Object^ Sender, EventArgs^ E)
{
	for each (auto Itr in ListViewSyncedScripts->SelectedObjects)
	{
		auto Selection = safe_cast<SyncedScriptListViewWrapper^>(Itr);
		DiskSync::Get()->OpenScriptFile(Selection->EditorID);
	}
}

void DiskSyncDialog::CheckboxAutoSync_Click(Object^ Sender, EventArgs^ E)
{
	DiskSync::Get()->AutomaticSync = CheckboxAutoSync->Checked;
}

void DiskSyncDialog::NumericAutoSyncSeconds_ValueChanged(Object^ Sender, EventArgs^ E)
{
	DiskSync::Get()->AutomaticSyncIntervalSeconds = Decimal::ToUInt32(NumericAutoSyncSeconds->Value);
}

void DiskSyncDialog::ListViewSyncedScripts_SelectionChanged(Object^ Sender, EventArgs^ E)
{
	DeferredSelectionUpdateTimer->Start();
}

void DiskSyncDialog::DeferredSelectionUpdateTimer_Tick(Object^ Sender, EventArgs^ E)
{
	auto Selection = safe_cast<SyncedScriptListViewWrapper^>(ListViewSyncedScripts->SelectedObject);
	if (Selection == nullptr)
	{
		LabelSelectedScriptLog->Text = "Log";
		TextBoxSelectedScriptLog->Text = "";
	}
	else
	{
		LabelSelectedScriptLog->Text = "Log [" + Selection->EditorID + "]";
		TextBoxSelectedScriptLog->Text = GetOutputMessagesForScript(Selection->EditorID);
	}

	UpdateToolbarEnabledState();
	DeferredSelectionUpdateTimer->Stop();
}

void DiskSyncDialog::Dialog_Cancel(Object^ Sender, CancelEventArgs^ E)
{
	if (IsSyncInProgress())
		DiskSync::Get()->Stop();
}

void DiskSyncDialog::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
{
	SetDefaultFont(preferences::SettingsHolder::Get()->Appearance->UIFont);
}


void DiskSyncDialog::DiskSync_SyncStart(Object^ Sender, SyncStartEventArgs^ E)
{
	ButtonSelectWorkingDir->Enabled = false;
	UpdateToolbarEnabledState();

	ButtonStartStopSync->Text = "Stop syncing";
	LabelSelectedScriptLog->Text = "Log";
	TextBoxSelectedScriptLog->Text = "";

	bool PromptUser = RadioPromptForFileHandling->Checked;
	auto DefaultOp = SyncStartEventArgs::eExistingFileHandlingOperation::Overwrite;
	if (RadioUseExistingFiles->Checked)
	{
		DefaultOp = SyncStartEventArgs::eExistingFileHandlingOperation::Keep;
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

		auto Selection = MessageBox::Show(Message, view::IScriptEditorView::MainWindowDefaultTitle, MessageBoxButtons::YesNo, MessageBoxIcon::Warning);
		DefaultOp = Selection == Windows::Forms::DialogResult::Yes ? SyncStartEventArgs::eExistingFileHandlingOperation::Overwrite : SyncStartEventArgs::eExistingFileHandlingOperation::Keep;
		if (Control::ModifierKeys == Keys::Control)
			ApplyToAll = true;

		E->ExistingFilesOnDisk[Itr] = DefaultOp;
	}
}

void DiskSyncDialog::DiskSync_SyncStop(Object^ Sender, SyncStopEventArgs^ E)
{
	E->RemoveLogFiles = CheckboxAutoDeleteLogs->Checked;

	ButtonSelectWorkingDir->Enabled = true;
	UpdateToolbarEnabledState();

	ButtonStartStopSync->Text = "Start syncing";

	LabelSelectedScriptLog->Text = "Log";
	TextBoxSelectedScriptLog->Text = "";
}


void DiskSyncDialog::DiskSync_SyncWriteToDisk(Object ^ Sender, SyncWriteToDiskEventArgs ^ E)
{
	auto ListViewObject = SyncedScripts[E->ScriptEditorID];
	ListViewObject->LastSyncTime = E->AccessTimestamp;
	if (ListViewObject->LastSyncSuccess == SyncedScriptListViewWrapper::eSuccessState::None)
		ListViewObject->LastSyncSuccess = E->Success ? SyncedScriptListViewWrapper::eSuccessState::Success : SyncedScriptListViewWrapper::eSuccessState::Failure;

	ListViewSyncedScripts->RefreshObject(ListViewObject);
}

void DiskSyncDialog::DiskSync_SyncPreCompile(Object^ Sender, SyncPreCompileEventArgs^ E)
{
	auto ListViewObject = SyncedScripts[E->ScriptEditorID];
	ListViewObject->LastSyncTime = E->AccessTimestamp;
	ListViewObject->LastSyncSuccess = SyncedScriptListViewWrapper::eSuccessState::Indeterminate;

	ListViewSyncedScripts->RefreshObject(ListViewObject);
}

void DiskSyncDialog::DiskSync_SyncPostCompile(Object^ Sender, SyncPostCompileEventArgs^ E)
{
	auto ListViewObject = SyncedScripts[E->ScriptEditorID];
	ListViewObject->LastSyncSuccess = E->Success ? SyncedScriptListViewWrapper::eSuccessState::Success : SyncedScriptListViewWrapper::eSuccessState::Failure;
	ListViewObject->LastOutputMessages->Clear();
	ListViewObject->LastOutputMessages->AddRange(E->OutputMessages);

	ListViewSyncedScripts->RefreshObject(ListViewObject);

	if (ListViewSyncedScripts->SelectedObject == ListViewObject)
		TextBoxSelectedScriptLog->Text = GetOutputMessagesForScript(E->ScriptEditorID);
}

bool DiskSyncDialog::IsSyncInProgress()
{
	return DiskSync::Get()->InProgress;
}

System::String^ DiskSyncDialog::GetOutputMessagesForScript(String ^ EditorID)
{
	if (!SyncedScripts->ContainsKey(EditorID))
		return "";

	auto Selection = SyncedScripts[EditorID];
	String^ Out = "";

	for each (auto Message in Selection->LastOutputMessages)
		Out += Message + "\r\n";

	return Out;
}

void DiskSyncDialog::UpdateToolbarEnabledState()
{
	ToolbarSelectScripts->Enabled = true;
	ToolbarSyncFromDisk->Enabled = true;
	ToolbarSyncToDisk->Enabled = true;
	ToolbarOpenLog->Enabled = true;
	ToolbarOpenSyncedFile->Enabled = true;

	if (IsSyncInProgress())
		ToolbarSelectScripts->Enabled = false;
	else
	{
		ToolbarSelectScripts->Enabled = true;
		ToolbarSyncFromDisk->Enabled = false;
		ToolbarSyncToDisk->Enabled = false;
		ToolbarOpenLog->Enabled = false;
		ToolbarOpenSyncedFile->Enabled = false;
	}

	if (ListViewSyncedScripts->SelectedObjects->Count == 0 || ListViewSyncedScripts->Objects == nullptr || ListViewSyncedScripts->GetItemCount() == 0)
	{
		ToolbarSyncFromDisk->Enabled = false;
		ToolbarSyncToDisk->Enabled = false;
		ToolbarOpenLog->Enabled = false;
		ToolbarOpenSyncedFile->Enabled = false;
	}
}

void DiskSyncDialog::SetDefaultFont(System::Drawing::Font^ DefaultFont)
{
	this->Font = DefaultFont;
	ButtonOpenWorkingDir->Font = DefaultFont;
	LabelSelectedScriptLog->Font = DefaultFont;
	ListViewSyncedScripts->Font = DefaultFont;
	ButtonStartStopSync->Font = DefaultFont;
	ButtonSelectWorkingDir->Font = DefaultFont;
	LabelWorkingDir->Font = DefaultFont;
	GroupSyncSettings->Font = DefaultFont;
	GroupStartupFileHandling->Font = DefaultFont;
	RadioPromptForFileHandling->Font = DefaultFont;
	RadioUseExistingFiles->Font = DefaultFont;
	RadioOverwriteExistingFiles->Font = DefaultFont;
	CheckboxAutoDeleteLogs->Font = DefaultFont;
	LabelSeconds->Font = DefaultFont;
	CheckboxAutoSync->Font = DefaultFont;
	NumericAutoSyncSeconds->Font = DefaultFont;
	TextBoxSelectedScriptLog->Font = DefaultFont;
	TextBoxWorkingDir->Font = DefaultFont;
	LeftPanel->Font = DefaultFont;
	Toolbar->Font = DefaultFont;
	ToolbarLabelSyncedScripts->Font = DefaultFont;
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
	FinalizeComponents();

	DiskSync::Get()->SyncStart += DiskSyncSyncStartHandler;
	DiskSync::Get()->SyncStop += DiskSyncSyncStopHandler;
	DiskSync::Get()->ScriptWriteToDisk += DiskSyncSyncWriteToDiskHandler;
	DiskSync::Get()->ScriptPreCompile += DiskSyncSyncPreCompileHandler;
	DiskSync::Get()->ScriptPostCompile += DiskSyncSyncPostCompileHandler;

	switch (preferences::SettingsHolder::Get()->ScriptSync->ExistingFileHandlingOp)
	{
	case SyncStartEventArgs::eExistingFileHandlingOperation::Prompt:
		RadioPromptForFileHandling->Checked = true;
		break;
	case SyncStartEventArgs::eExistingFileHandlingOperation::Overwrite:
		RadioOverwriteExistingFiles->Checked = true;
		break;
	case SyncStartEventArgs::eExistingFileHandlingOperation::Keep:
		RadioUseExistingFiles->Checked = true;
		break;
	}

	CheckboxAutoSync->Checked = preferences::SettingsHolder::Get()->ScriptSync->AutoSyncChanges;
	CheckboxAutoDeleteLogs->Checked = preferences::SettingsHolder::Get()->ScriptSync->AutoDeleteLogs;
	NumericAutoSyncSeconds->Value = preferences::SettingsHolder::Get()->ScriptSync->AutoSyncInterval;
	TextBoxWorkingDir->Text = DiskSync::Get()->WorkingDirectory;

	auto ParentWindowHandle = safe_cast<IntPtr>(nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetMainWindowHandle());
	this->Show(gcnew WindowHandleWrapper(ParentWindowHandle));
}

DiskSyncDialog::~DiskSyncDialog()
{
	Debug::Assert(Singleton != nullptr);

	preferences::SettingsHolder::Get()->PreferencesChanged -= ScriptEditorPreferencesSavedHandler;

	DiskSync::Get()->SyncStart -= DiskSyncSyncStartHandler;
	DiskSync::Get()->SyncStop -= DiskSyncSyncStopHandler;
	DiskSync::Get()->ScriptWriteToDisk -= DiskSyncSyncWriteToDiskHandler;
	DiskSync::Get()->ScriptPreCompile -= DiskSyncSyncPreCompileHandler;
	DiskSync::Get()->ScriptPostCompile -= DiskSyncSyncPostCompileHandler;

	if (RadioPromptForFileHandling->Checked)
		preferences::SettingsHolder::Get()->ScriptSync->ExistingFileHandlingOp = SyncStartEventArgs::eExistingFileHandlingOperation::Prompt;
	else if (RadioOverwriteExistingFiles->Checked)
		preferences::SettingsHolder::Get()->ScriptSync->ExistingFileHandlingOp = SyncStartEventArgs::eExistingFileHandlingOperation::Overwrite;
	else
		preferences::SettingsHolder::Get()->ScriptSync->ExistingFileHandlingOp = SyncStartEventArgs::eExistingFileHandlingOperation::Keep;

	preferences::SettingsHolder::Get()->ScriptSync->AutoSyncChanges = CheckboxAutoSync->Checked;
	preferences::SettingsHolder::Get()->ScriptSync->AutoDeleteLogs = CheckboxAutoDeleteLogs->Checked;
	preferences::SettingsHolder::Get()->ScriptSync->AutoSyncInterval = Decimal::ToUInt32(NumericAutoSyncSeconds->Value);

	SAFEDELETE_CLR(ScriptEditorPreferencesSavedHandler);
	SAFEDELETE_CLR(ListViewThemeWrapper);

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


} // namespace scriptSync


} // namespace scriptEditor


} // namespace cse

