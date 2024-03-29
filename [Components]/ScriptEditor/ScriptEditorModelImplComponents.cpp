#include "ScriptEditorModelImplComponents.h"
#include "Preferences.h"
#include "IntelliSenseBackend.h"


namespace cse
{


namespace scriptEditor
{


namespace modelImpl
{


namespace components
{


obScriptParsing::AnalysisData^ BackgroundSemanticAnalyzer::PerformBackgroundTask(Object^ Input)
{
	auto AnalysisParams = safe_cast<obScriptParsing::AnalysisData::Params^>(Input);
	Debug::Assert(AnalysisParams != nullptr);

	auto AnalysisResult = gcnew obScriptParsing::AnalysisData;
	AnalysisResult->PerformAnalysis(AnalysisParams);

	return AnalysisResult;
}

void BackgroundSemanticAnalyzer::Preferences_Saved(Object^ Sender, EventArgs^ E)
{
	if (Mode == eExecutionMode::IdleQueue)
		SetExecutionMode(eExecutionMode::IdleQueue);		// re-init timer
}

void BackgroundSemanticAnalyzer::ParentScriptDocument_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E)
{
	if (E->EventType == model::IScriptDocument::StateChangeEventArgs::eEventType::Dirty)
		ModificationsSinceLastAnalysis = true;
}

void BackgroundSemanticAnalyzer::QueuePollTimer_Tick(Object^ Sender, EventArgs^ E)
{
	switch (Mode)
	{
	case eExecutionMode::IdlePoll:
		HandleActiveAnalysisTaskPolling();
		break;
	case eExecutionMode::IdleQueue:
		if (!Paused)
			HandleAnalysisTaskQueueing(false);
		break;
	}
}

obScriptParsing::AnalysisData::Params^ BackgroundSemanticAnalyzer::GenerateAnalysisParameters()
{
	auto AnalysisParams = gcnew obScriptParsing::AnalysisData::Params;
	if (ParentScriptDocument->ScriptText->Length == 0)
		return AnalysisParams;

	AnalysisParams->ScriptText = ParentScriptDocument->ScriptText;

	switch (ParentScriptDocument->ScriptType)
	{
	case model::IScriptDocument::eScriptType::Object:
		AnalysisParams->Type = obScriptParsing::eScriptType::Object;
		break;
	case model::IScriptDocument::eScriptType::Quest:
		AnalysisParams->Type = obScriptParsing::eScriptType::Quest;
		break;
	case model::IScriptDocument::eScriptType::MagicEffect:
		AnalysisParams->Type = obScriptParsing::eScriptType::MagicEffect;
		break;
	}

	AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::FillVariables;
	AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::FillControlBlocks;
	AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::PerformBasicValidation;

	if (preferences::SettingsHolder::Get()->Validator->CheckVarCommandNameCollisions)
		AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::CheckVariableNameCommandCollisions;
	if (preferences::SettingsHolder::Get()->Validator->CheckVarFormNameCollisions)
		AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::CheckVariableNameFormCollisions;
	if (preferences::SettingsHolder::Get()->Validator->CountVariableRefs)
		AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::CountVariableReferences;
	if (preferences::SettingsHolder::Get()->Validator->NoQuestVariableRefCounting)
		AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::eOperation::SuppressQuestVariableRefCount;

	AnalysisParams->ScriptCommandIdentifiers = intellisense::IntelliSenseBackend::Get()->CreateIndentifierSnapshot(intellisense::eDatabaseLookupFilter::Command);

	auto Filter = intellisense::eDatabaseLookupFilter::All & ~intellisense::eDatabaseLookupFilter::Snippet;
	Filter = Filter & ~intellisense::eDatabaseLookupFilter::Command;
	AnalysisParams->FormIdentifiers = intellisense::IntelliSenseBackend::Get()->CreateIndentifierSnapshot(Filter);

	return AnalysisParams;
}

bool BackgroundSemanticAnalyzer::QueueBackgroundTask(bool Force)
{
	if (!Force && Paused)
		return false;
	else if (!Force && !ModificationsSinceLastAnalysis)
		return false;

	auto AnalysisParams = GenerateAnalysisParameters();
	if (AnalysisParams->ScriptText->Length == 0)
		return false;

	auto TaskDelegate = gcnew System::Func<Object^, obScriptParsing::AnalysisData^>(&BackgroundSemanticAnalyzer::PerformBackgroundTask);
	ActiveAnalysisTask = AnalysisTaskT::Factory->StartNew(TaskDelegate, AnalysisParams);

	return true;
}

void BackgroundSemanticAnalyzer::SetExecutionMode(eExecutionMode Mode)
{
	this->Mode = Mode;

	switch (Mode)
	{
	case eExecutionMode::IdlePoll:
		QueuePollTimer->Interval = kTimerPollInterval;
		QueuePollTimer->Stop();
		QueuePollTimer->Start();

		break;
	case eExecutionMode::IdleQueue:
		QueuePollTimer->Interval = preferences::SettingsHolder::Get()->IntelliSense->BackgroundAnalysisInterval * 1000;
		QueuePollTimer->Stop();
		QueuePollTimer->Start();

		break;
	default:
		break;

	}
}

bool BackgroundSemanticAnalyzer::HandleAnalysisTaskQueueing(bool Force)
{
	Debug::Assert(ActiveAnalysisTask == nullptr);

	if (!Force && (DateTime::Now - LastAnalysisTimestamp).TotalMilliseconds < QueuePollTimer->Interval)
		return false;

	if (!QueueBackgroundTask(Force))
		return false;

	SetExecutionMode(eExecutionMode::IdlePoll);
	return true;
}

bool BackgroundSemanticAnalyzer::HandleActiveAnalysisTaskPolling()
{
	Debug::Assert(ActiveAnalysisTask != nullptr);

	bool Error = false;
	switch (ActiveAnalysisTask->Status)
	{
	case System::Threading::Tasks::TaskStatus::Faulted:
		DebugPrint("Background analysis task failed! Exception: " + ActiveAnalysisTask->Exception->ToString(), true);
		Error = true;
		break;
	case System::Threading::Tasks::TaskStatus::RanToCompletion:
		LastAnalysisData = ActiveAnalysisTask->Result;
		break;
	default:
		return false;
	}

	ActiveAnalysisTask = nullptr;
	LastAnalysisTimestamp = DateTime::Now;
	ModificationsSinceLastAnalysis = false;

	SetExecutionMode(eExecutionMode::IdleQueue);

	if (!Error)
		SemanticAnalysisComplete(this, gcnew IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs(LastAnalysisData, true));

	return true;
}

BackgroundSemanticAnalyzer::BackgroundSemanticAnalyzer(model::IScriptDocument^ ParentScriptDocument)
{
	this->ParentScriptDocument = ParentScriptDocument;

	QueuePollTimer = gcnew Timer;
	QueuePollTimer->Interval = preferences::SettingsHolder::Get()->IntelliSense->BackgroundAnalysisInterval * 1000;
	QueuePollTimer->Stop();

	ActiveAnalysisTask = nullptr;
	LastAnalysisTimestamp = DateTime::Now;
	LastAnalysisData = nullptr;
	ModificationsSinceLastAnalysis = false;
	Mode = eExecutionMode::IdleQueue;
	Paused = true;

	PreferencesChangedHandler = gcnew EventHandler(this, &BackgroundSemanticAnalyzer::Preferences_Saved);
	ParentScriptDocumentStateChangedHandler = gcnew model::IScriptDocument::StateChangeEventHandler(this, &BackgroundSemanticAnalyzer::ParentScriptDocument_StateChanged);
	QueuePollTimerTickHandler = gcnew EventHandler(this, &BackgroundSemanticAnalyzer::QueuePollTimer_Tick);

	preferences::SettingsHolder::Get()->PreferencesChanged += PreferencesChangedHandler;
	QueuePollTimer->Tick += QueuePollTimerTickHandler;
	ParentScriptDocument->StateChanged += ParentScriptDocumentStateChangedHandler;
}

BackgroundSemanticAnalyzer::~BackgroundSemanticAnalyzer()
{
	CleanupActiveBgTask();
	QueuePollTimer->Stop();

	preferences::SettingsHolder::Get()->PreferencesChanged -= PreferencesChangedHandler;
	QueuePollTimer->Tick -= QueuePollTimerTickHandler;
	ParentScriptDocument->StateChanged -= ParentScriptDocumentStateChangedHandler;

	SAFEDELETE_CLR(PreferencesChangedHandler);
	SAFEDELETE_CLR(ParentScriptDocumentStateChangedHandler);
	SAFEDELETE_CLR(QueuePollTimerTickHandler);

	ParentScriptDocument = nullptr;
}

void BackgroundSemanticAnalyzer::Pause()
{
	Debug::Assert(Paused == false);

	CleanupActiveBgTask();
	QueuePollTimer->Stop();
	Paused = true;
}

void BackgroundSemanticAnalyzer::Resume()
{
	Debug::Assert(Paused == true);

	Paused = false;
	QueuePollTimer->Start();
}

void BackgroundSemanticAnalyzer::CleanupActiveBgTask()
{
	if (ActiveAnalysisTask == nullptr)
		return;

	ActiveAnalysisTask = nullptr;
	LastAnalysisTimestamp = DateTime::Now;
	ModificationsSinceLastAnalysis = false;
	SetExecutionMode(eExecutionMode::IdleQueue);
}

obScriptParsing::AnalysisData^ BackgroundSemanticAnalyzer::DoSynchronousAnalysis(bool RaiseCompletionEvent)
{
	auto AnalysisParams = GenerateAnalysisParameters();
	auto AnalysisResult = gcnew obScriptParsing::AnalysisData;

	if (AnalysisParams->ScriptText->Length == 0)
		return AnalysisResult;

	AnalysisResult->PerformAnalysis(AnalysisParams);
	if (RaiseCompletionEvent)
		SemanticAnalysisComplete(this, gcnew IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs(AnalysisResult, false));

	return AnalysisResult;
}

void BackgroundSemanticAnalyzer::QueueImmediateBgAnalysis()
{
	if (ActiveAnalysisTask)
		return;

	HandleAnalysisTaskQueueing(true);
}

void NavigationHelper::ParentScriptDocument_LineChanged(Object^ Sender, EventArgs^ E)
{
	auto EventArgs = gcnew INavigationHelper::NavigationChangedEventArgs(nullptr, ParentScriptDocument->TextEditor->CurrentLine);
	EventArgs->LineChanged = true;

	NavigationChanged(ParentScriptDocument, EventArgs);
}

void NavigationHelper::ParentScriptDocument_BgAnalysisComplete(Object^ Sender, model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs^ E)
{
	UpdateStructureData(E->Result);

	auto EventArgs = gcnew INavigationHelper::NavigationChangedEventArgs(StructureData, ParentScriptDocument->TextEditor->CurrentLine);
	EventArgs->StructureChanged = true;

	NavigationChanged(ParentScriptDocument, EventArgs);
}

void NavigationHelper::ParentScriptDocument_TextUpdated(Object^ Sender, EventArgs^ E)
{
	StructureData = nullptr;

	auto EventArgs = gcnew INavigationHelper::NavigationChangedEventArgs(nullptr, ParentScriptDocument->TextEditor->CurrentLine);
	EventArgs->StructureChanged = true;
	EventArgs->LineChanged = true;

	NavigationChanged(ParentScriptDocument, EventArgs);
}

void NavigationHelper::UpdateStructureData(obScriptParsing::AnalysisData^ AnalysisData)
{
	StructureData = GenerateStructureData(AnalysisData);
	if (!StructureData->Valid)
		StructureData = nullptr;
}

NavigationHelper::NavigationHelper(model::IScriptDocument^ ParentScriptDocument)
{
	this->ParentScriptDocument = ParentScriptDocument;
	StructureData = nullptr;

	ParentScriptDocumentLineChangedHandler = gcnew EventHandler(this, &NavigationHelper::ParentScriptDocument_LineChanged);
	ParentScriptDocumentTextUpdatedHandler = gcnew EventHandler(this, &NavigationHelper::ParentScriptDocument_TextUpdated);
	ParentScriptDocumentBgAnalysisCompleteHandler = gcnew model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler(this, &NavigationHelper::ParentScriptDocument_BgAnalysisComplete);

	ParentScriptDocument->TextEditor->LineChanged += ParentScriptDocumentLineChangedHandler;
	ParentScriptDocument->TextEditor->TextUpdated += ParentScriptDocumentTextUpdatedHandler;
	ParentScriptDocument->BackgroundAnalyzer->SemanticAnalysisComplete += ParentScriptDocumentBgAnalysisCompleteHandler;
}

NavigationHelper::~NavigationHelper()
{
	ParentScriptDocument->TextEditor->LineChanged -= ParentScriptDocumentLineChangedHandler;
	ParentScriptDocument->TextEditor->TextUpdated -= ParentScriptDocumentTextUpdatedHandler;
	ParentScriptDocument->BackgroundAnalyzer->SemanticAnalysisComplete -= ParentScriptDocumentBgAnalysisCompleteHandler;

	SAFEDELETE_CLR(ParentScriptDocumentLineChangedHandler);
	SAFEDELETE_CLR(ParentScriptDocumentTextUpdatedHandler);
	SAFEDELETE_CLR(ParentScriptDocumentBgAnalysisCompleteHandler);

	ParentScriptDocument = nullptr;
	StructureData = nullptr;
}

obScriptParsing::Structurizer^ NavigationHelper::GenerateStructureData(obScriptParsing::AnalysisData^ AnalysisData)
{
	auto GetLineTextDelegate = gcnew obScriptParsing::Structurizer::GetLineText(ParentScriptDocument->TextEditor, &textEditor::ITextEditor::GetText);
	return gcnew obScriptParsing::Structurizer(AnalysisData, GetLineTextDelegate, ParentScriptDocument->TextEditor->CurrentLine);

}


} // namespace components


} // namespace modelImpl


} // namespace scriptEditor


} // namespace cse