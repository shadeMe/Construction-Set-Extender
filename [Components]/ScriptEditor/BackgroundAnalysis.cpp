#include "BackgroundAnalysis.h"
#include "Preferences.h"
#include "IntelliSenseDatabase.h"

namespace cse
{
	namespace scriptEditor
	{
		obScriptParsing::AnalysisData^ ScriptBackgroundAnalysis::PerformBackgroundTask(Object^ Input)
		{
			auto AnalysisParams = safe_cast<obScriptParsing::AnalysisData::Params^>(Input);
			Debug::Assert(AnalysisParams != nullptr);

			auto AnalysisResult = gcnew obScriptParsing::AnalysisData;
			AnalysisResult->PerformAnalysis(AnalysisParams);

			return AnalysisResult;
		}

		void ScriptBackgroundAnalysis::Preferences_Saved(Object^ Sender, EventArgs^ E)
		{
			if (Mode == ExecutionMode::IdleQueue)
				SetExecutionMode(ExecutionMode::IdleQueue);		// re-init timer
		}

		void ScriptBackgroundAnalysis::ParentModel_OnStateChangedDirty(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E)
		{
			ModificationsSinceLastAnalysis = true;
		}

		void ScriptBackgroundAnalysis::QueuePollTimer_Tick(Object^ Sender, EventArgs^ E)
		{
			if (Paused)
				return;

			switch (Mode)
			{
			case ExecutionMode::IdlePoll:
				HandleActiveAnalysisTaskPolling();
				break;
			case ExecutionMode::IdleQueue:
				HandleAnalysisTaskQueueing();
				break;
			}
		}

		cse::obScriptParsing::AnalysisData::Params^ ScriptBackgroundAnalysis::GenerateAnalysisParameters()
		{
			auto AnalysisParams = gcnew obScriptParsing::AnalysisData::Params;

			bool Throwaway = false;
			AnalysisParams->ScriptText = ParentModel->Controller->GetText(ParentModel, false, Throwaway);
			if (AnalysisParams->ScriptText->Length == 0)
				return AnalysisParams;

			switch (ParentModel->Type)
			{
			case IWorkspaceModel::ScriptType::Object:
				AnalysisParams->Type = obScriptParsing::ScriptType::Object;
				break;
			case IWorkspaceModel::ScriptType::Quest:
				AnalysisParams->Type = obScriptParsing::ScriptType::Quest;
				break;
			case IWorkspaceModel::ScriptType::MagicEffect:
				AnalysisParams->Type = obScriptParsing::ScriptType::MagicEffect;
				break;
			}

			AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::FillVariables;
			AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::FillControlBlocks;
			AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::PerformBasicValidation;

			if (preferences::SettingsHolder::Get()->Validator->CheckVarCommandNameCollisions)
				AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::CheckVariableNameCommandCollisions;
			if (preferences::SettingsHolder::Get()->Validator->CheckVarFormNameCollisions)
				AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::CheckVariableNameFormCollisions;
			if (preferences::SettingsHolder::Get()->Validator->CountVariableRefs)
				AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::CountVariableReferences;
			if (preferences::SettingsHolder::Get()->Validator->NoQuestVariableRefCounting)
				AnalysisParams->Ops = AnalysisParams->Ops | obScriptParsing::AnalysisData::Operation::SuppressQuestVariableRefCount;

			AnalysisParams->ScriptCommandIdentifiers = intellisense::IntelliSenseBackend::Get()->CreateIndentifierSnapshot(
				intellisense::DatabaseLookupFilter::Command);

			auto Filter = intellisense::DatabaseLookupFilter::All & ~intellisense::DatabaseLookupFilter::Snippet;
			Filter = Filter & ~intellisense::DatabaseLookupFilter::Command;
			AnalysisParams->FormIdentifiers = intellisense::IntelliSenseBackend::Get()->CreateIndentifierSnapshot(Filter);

			return AnalysisParams;
		}

		bool ScriptBackgroundAnalysis::QueueBackgroundTask()
		{
			if (!ParentModel->Bound)
				return false;
			else if (!ModificationsSinceLastAnalysis)
				return false;

			auto AnalysisParams = GenerateAnalysisParameters();
			if (AnalysisParams->ScriptText->Length == 0)
				return false;

			auto TaskDelegate = gcnew System::Func<Object^, obScriptParsing::AnalysisData^>(&ScriptBackgroundAnalysis::PerformBackgroundTask);
			ActiveAnalysisTask = AnalysisTaskT::Factory->StartNew(TaskDelegate, AnalysisParams);

			return true;
		}

		void ScriptBackgroundAnalysis::SetExecutionMode(ExecutionMode Mode)
		{
			switch (Mode)
			{
			case ExecutionMode::IdlePoll:
				QueuePollTimer->Interval = kTimerPollInterval;
				QueuePollTimer->Stop();
				QueuePollTimer->Start();

				break;
			case ExecutionMode::IdleQueue:
				QueuePollTimer->Interval = preferences::SettingsHolder::Get()->IntelliSense->BackgroundAnalysisInterval * 1000;
				QueuePollTimer->Stop();
				QueuePollTimer->Start();

				break;
			default:
				break;

			}

			this->Mode = Mode;
		}

		bool ScriptBackgroundAnalysis::HandleAnalysisTaskQueueing()
		{
			Debug::Assert(ActiveAnalysisTask == nullptr);

			if ((DateTime::Now - LastAnalysisTimestamp).TotalMilliseconds < QueuePollTimer->Interval)
				return false;

			if (!QueueBackgroundTask())
				return false;

			SetExecutionMode(ExecutionMode::IdlePoll);
			return true;
		}

		bool ScriptBackgroundAnalysis::HandleActiveAnalysisTaskPolling()
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

			SetExecutionMode(ExecutionMode::IdleQueue);

			if (!Error)
				SemanticAnalysisComplete(this, gcnew SemanticAnalysisCompleteEventArgs(LastAnalysisData));

			return true;
		}

		ScriptBackgroundAnalysis::ScriptBackgroundAnalysis(IWorkspaceModel^ ParentModel)
		{
			this->ParentModel = ParentModel;

			QueuePollTimer = gcnew Timer;
			QueuePollTimer->Interval = preferences::SettingsHolder::Get()->IntelliSense->BackgroundAnalysisInterval * 1000;
			QueuePollTimer->Stop();

			ActiveAnalysisTask = nullptr;
			LastAnalysisTimestamp = DateTime::Now;
			LastAnalysisData = nullptr;
			ModificationsSinceLastAnalysis = false;
			Mode = ExecutionMode::IdleQueue;
			Paused = true;

			PreferencesChangedHandler = gcnew EventHandler(this, &ScriptBackgroundAnalysis::Preferences_Saved);
			ParentModelOnStateChangedDirtyHandler = gcnew IWorkspaceModel::StateChangeEventHandler(this, &ScriptBackgroundAnalysis::ParentModel_OnStateChangedDirty);
			QueuePollTimerTickHandler = gcnew EventHandler(this, &ScriptBackgroundAnalysis::QueuePollTimer_Tick);

			preferences::SettingsHolder::Get()->SavedToDisk += PreferencesChangedHandler;
			QueuePollTimer->Tick += QueuePollTimerTickHandler;
			ParentModel->StateChangedDirty += ParentModelOnStateChangedDirtyHandler;
		}

		ScriptBackgroundAnalysis::~ScriptBackgroundAnalysis()
		{
			WaitForBackgroundTask();
			QueuePollTimer->Stop();

			preferences::SettingsHolder::Get()->SavedToDisk -= PreferencesChangedHandler;
			QueuePollTimer->Tick -= QueuePollTimerTickHandler;
			ParentModel->StateChangedDirty -= ParentModelOnStateChangedDirtyHandler;

			SAFEDELETE_CLR(PreferencesChangedHandler);
			SAFEDELETE_CLR(ParentModelOnStateChangedDirtyHandler);
			SAFEDELETE_CLR(QueuePollTimerTickHandler);

			ParentModel = nullptr;
		}

		void ScriptBackgroundAnalysis::Pause()
		{
			Debug::Assert(Paused == false);

			WaitForBackgroundTask();
			QueuePollTimer->Stop();
			Paused = true;
		}

		void ScriptBackgroundAnalysis::Resume()
		{
			Debug::Assert(Paused == true);

			Paused = false;
			QueuePollTimer->Start();
		}

		void ScriptBackgroundAnalysis::WaitForBackgroundTask()
		{
			if (ActiveAnalysisTask)
			{
				ActiveAnalysisTask->Wait();
				HandleActiveAnalysisTaskPolling();
			}
		}

		cse::obScriptParsing::AnalysisData^ ScriptBackgroundAnalysis::DoSynchronousAnalysis()
		{
			auto AnalysisParams = GenerateAnalysisParameters();
			auto AnalysisResult = gcnew obScriptParsing::AnalysisData;

			if (AnalysisParams->ScriptText->Length == 0)
				return AnalysisResult;

			AnalysisResult->PerformAnalysis(AnalysisParams);
			return AnalysisResult;
		}

	}
}



