#pragma once

#include "ScriptTextEditorInterface.h"
#include "SemanticAnalysis.h"
#include "WorkspaceModelInterface.h"

namespace cse
{
	namespace scriptEditor
	{
		ref struct SemanticAnalysisCompleteEventArgs
		{
			property obScriptParsing::AnalysisData^		Result;
			property bool								BackgroundAnalysis;

			SemanticAnalysisCompleteEventArgs(obScriptParsing::AnalysisData^ Result, bool BackgroundAnalysis)
			{
				this->Result = Result->Clone();
				this->BackgroundAnalysis = BackgroundAnalysis;
			}
		};

		delegate void SemanticAnalysisCompleteEventHandler(Object^ Sender, SemanticAnalysisCompleteEventArgs^ E);


		interface class IBackgroundSemanticAnalyzer
		{
			event SemanticAnalysisCompleteEventHandler^ SemanticAnalysisComplete;

			property obScriptParsing::AnalysisData^ LastAnalysisResult;

			obScriptParsing::AnalysisData^ DoSynchronousAnalysis(bool RaiseCompletionEvent);
		};


		ref class ScriptBackgroundAnalysis : public IBackgroundSemanticAnalyzer
		{
			static const UInt32 kTimerPollInterval = 100;	// in ms

			static enum class ExecutionMode
			{
				IdlePoll,
				IdleQueue,
			};

			using AnalysisTaskT = System::Threading::Tasks::Task<obScriptParsing::AnalysisData^>;

			IWorkspaceModel^		ParentModel;
			Timer^					QueuePollTimer;
			AnalysisTaskT^			ActiveAnalysisTask;
			obScriptParsing::AnalysisData^
									LastAnalysisData;
			DateTime				LastAnalysisTimestamp;
			bool					ModificationsSinceLastAnalysis;
			ExecutionMode			Mode;
			bool					Paused;

			EventHandler^			PreferencesChangedHandler;
			IWorkspaceModel::StateChangeEventHandler^
									ParentModelOnStateChangedDirtyHandler;
			EventHandler^			QueuePollTimerTickHandler;

			static obScriptParsing::AnalysisData^
						PerformBackgroundTask(Object^ Input);

			void		Preferences_Saved(Object^ Sender, EventArgs^ E);
			void		ParentModel_OnStateChangedDirty(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E);
			void		QueuePollTimer_Tick(Object^ Sender, EventArgs^ E);

			obScriptParsing::AnalysisData::Params^
						GenerateAnalysisParameters();
			bool		QueueBackgroundTask();
			void		SetExecutionMode(ExecutionMode Mode);

			bool		HandleAnalysisTaskQueueing();
			bool		HandleActiveAnalysisTaskPolling();
		public:
			ScriptBackgroundAnalysis(IWorkspaceModel^ ParentModel);
			~ScriptBackgroundAnalysis();

			virtual event SemanticAnalysisCompleteEventHandler^
														SemanticAnalysisComplete;

			virtual property obScriptParsing::AnalysisData^ LastAnalysisResult
			{
				obScriptParsing::AnalysisData^ get() { return LastAnalysisData; }
				void set(obScriptParsing::AnalysisData^) {}
			}

			void	Pause();
			void	Resume();
			void	WaitForBackgroundTask();

			virtual obScriptParsing::AnalysisData^		DoSynchronousAnalysis(bool RaiseCompletionEvent);

		};
	}
}
