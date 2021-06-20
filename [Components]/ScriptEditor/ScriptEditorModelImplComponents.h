#pragma once
#include "Macros.h"
#include "IScriptEditorModel.h"


namespace cse
{


namespace scriptEditor
{


namespace modelImpl
{


namespace components
{


ref class BackgroundSemanticAnalyzer : public model::components::IBackgroundSemanticAnalyzer
{
	static const UInt32 kTimerPollInterval = 100;	// in ms

	static enum class ExecutionMode
	{
		IdlePoll,
		IdleQueue,
	};

	using AnalysisTaskT = System::Threading::Tasks::Task<obScriptParsing::AnalysisData^>;

	model::IScriptDocument^ ParentScriptDocument;
	Timer^ QueuePollTimer;
	AnalysisTaskT^ ActiveAnalysisTask;
	obScriptParsing::AnalysisData^ LastAnalysisData;
	DateTime LastAnalysisTimestamp;
	bool ModificationsSinceLastAnalysis;
	ExecutionMode Mode;
	bool Paused;

	EventHandler^ PreferencesChangedHandler;
	model::IScriptDocument::StateChangeEventHandler^ ParentScriptDocumentStateChangedHandler;
	EventHandler^ QueuePollTimerTickHandler;

	static obScriptParsing::AnalysisData^ PerformBackgroundTask(Object^ Input);

	void Preferences_Saved(Object^ Sender, EventArgs^ E);
	void ParentScriptDocument_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E);
	void QueuePollTimer_Tick(Object^ Sender, EventArgs^ E);

	obScriptParsing::AnalysisData::Params^ GenerateAnalysisParameters();
	bool QueueBackgroundTask();
	void SetExecutionMode(ExecutionMode Mode);

	bool HandleAnalysisTaskQueueing();
	bool HandleActiveAnalysisTaskPolling();
public:
	BackgroundSemanticAnalyzer(model::IScriptDocument^ ParentScriptDocument);
	virtual ~BackgroundSemanticAnalyzer();

	ImplPropertyGetOnly(obScriptParsing::AnalysisData^, LastAnalysisResult, LastAnalysisData);

	virtual event IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler^ SemanticAnalysisComplete;

	void Pause();
	void Resume();
	void WaitForBackgroundTask();
	virtual obScriptParsing::AnalysisData^ DoSynchronousAnalysis(bool RaiseCompletionEvent);
};


ref class NavigationHelper : public model::components::INavigationHelper
{
	model::IScriptDocument^ ParentScriptDocument;
	obScriptParsing::Structurizer^ StructureData;

	EventHandler^ ParentScriptDocumentLineChangedHandler;
	EventHandler^ ParentScriptDocumentTextUpdatedHandler;
	model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler^ ParentScriptDocumentBgAnalysisCompleteHandler;

	void ParentScriptDocument_LineChanged(Object^ Sender, EventArgs^ E);
	void ParentScriptDocument_BgAnalysisComplete(Object^ Sender, model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs^ E);
	void ParentScriptDocument_TextUpdated(Object^ Sender, EventArgs^ E);

	void UpdateStructureData(obScriptParsing::AnalysisData^ AnalysisData);
public:
	NavigationHelper(model::IScriptDocument^ ParentScriptDocument);
	virtual ~NavigationHelper();

	virtual event INavigationHelper::NavigationChangedEventHandler^ NavigationChanged;

	virtual obScriptParsing::Structurizer^ GenerateStructureData(obScriptParsing::AnalysisData^ AnalysisData);
};


} // namespace components


} // namespace modelImpl


} // namespace scriptEditor


} // namespace cse