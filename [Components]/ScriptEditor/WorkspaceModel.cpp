#include "WorkspaceModel.h"
#include "[Common]\CustomInputBox.h"
#include "Preferences.h"
#include "IntelliSenseInterfaceModel.h"
#include "IntelliSenseDatabase.h"
#include "RefactorTools.h"
#include "ScriptSync.h"

namespace cse
{
	namespace scriptEditor
	{
		ConcreteWorkspaceModel::ConcreteWorkspaceModel(ConcreteWorkspaceModelController^ Controller, ConcreteWorkspaceModelFactory^ Factory,
													   componentDLLInterface::ScriptData* Data)
		{
			ModelController = Controller;
			ModelFactory = Factory;

			ScriptNativeObject = nullptr;
			ScriptType = IWorkspaceModel::ScriptType::Object;
			EditorID = FIRSTRUNSCRIPTID;
			FormID = 0;
			Bytecode = 0;
			BytecodeLength = 0;
			NewScriptFlag = false;
			CompilationInProgress = false;
			BoundParent = nullptr;
			ActiveBatchUpdateSource = BatchUpdateSource::None;
			ActiveBatchUpdateSourceCounter = 0;
			Messages = gcnew List<ScriptDiagnosticMessage^>;
			Bookmarks = gcnew List<ScriptBookmark^>;
			FindResults = gcnew List<ScriptFindResult^>;

			AutoSaveTimer = gcnew Timer();
			AutoSaveTimer->Interval = preferences::SettingsHolder::Get()->Backup->AutoRecoveryInterval * 1000 * 60;

			BackgroundAnalysis = gcnew ScriptBackgroundAnalysis(this);
			TextEditor = gcnew textEditor::avalonEdit::AvalonEditTextEditor(this);
			IntelliSenseModel = gcnew intellisense::IntelliSenseInterfaceModel(TextEditor);

			TextEditorKeyDownHandler = gcnew KeyEventHandler(this, &ConcreteWorkspaceModel::TextEditor_KeyDown);
			TextEditorScriptModifiedHandler = gcnew textEditor::TextEditorScriptModifiedEventHandler(this, &ConcreteWorkspaceModel::TextEditor_ScriptModified);
			TextEditorMouseClickHandler = gcnew textEditor::TextEditorMouseClickEventHandler(this, &ConcreteWorkspaceModel::TextEditor_MouseClick);
			TextEditorLineAnchorInvalidatedHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::TextEditor_LineAnchorInvalidated);
			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ScriptEditorPreferences_Saved);
			AutoSaveTimerTickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::AutoSaveTimer_Tick);
			BackgroundAnalyzerAnalysisCompleteHandler = gcnew SemanticAnalysisCompleteEventHandler(this, &ConcreteWorkspaceModel::BackgroundAnalysis_AnalysisComplete);

			TextEditor->KeyDown += TextEditorKeyDownHandler;
			TextEditor->ScriptModified += TextEditorScriptModifiedHandler;
			TextEditor->MouseClick += TextEditorMouseClickHandler;
			TextEditor->LineAnchorInvalidated += TextEditorLineAnchorInvalidatedHandler;
			BackgroundAnalysis->SemanticAnalysisComplete += BackgroundAnalyzerAnalysisCompleteHandler;
			preferences::SettingsHolder::Get()->PreferencesChanged += ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick += AutoSaveTimerTickHandler;

			AutoSaveTimer->Start();

			if (Data && Data->ParentForm)
				InitializeState(Data, false);

			nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
		}

		ConcreteWorkspaceModel::~ConcreteWorkspaceModel()
		{
			Unbind();

			AutoSaveTimer->Stop();

			TextEditor->KeyDown -= TextEditorKeyDownHandler;
			TextEditor->ScriptModified -= TextEditorScriptModifiedHandler;
			TextEditor->MouseClick -= TextEditorMouseClickHandler;
			TextEditor->LineAnchorInvalidated -= TextEditorLineAnchorInvalidatedHandler;
			BackgroundAnalysis->SemanticAnalysisComplete -= BackgroundAnalyzerAnalysisCompleteHandler;
			preferences::SettingsHolder::Get()->PreferencesChanged -= ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick -= AutoSaveTimerTickHandler;

			SAFEDELETE_CLR(TextEditorKeyDownHandler);
			SAFEDELETE_CLR(TextEditorScriptModifiedHandler);
			SAFEDELETE_CLR(TextEditorMouseClickHandler);
			SAFEDELETE_CLR(TextEditorLineAnchorInvalidatedHandler);
			SAFEDELETE_CLR(BackgroundAnalyzerAnalysisCompleteHandler);
			SAFEDELETE_CLR(ScriptEditorPreferencesSavedHandler);
			SAFEDELETE_CLR(AutoSaveTimerTickHandler);

			SAFEDELETE_CLR(TextEditor);
			SAFEDELETE_CLR(BackgroundAnalysis);
			SAFEDELETE_CLR(IntelliSenseModel);
			SAFEDELETE_CLR(AutoSaveTimer);

			ModelFactory->Remove(this);

			ModelController = nullptr;
			ModelFactory = nullptr;
		}

		void ConcreteWorkspaceModel::TextEditor_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			Debug::Assert(Bound == true);

			switch (E->KeyCode)
			{
			case Keys::Space:
				if (E->Control && E->Alt)
				{
					obScriptParsing::AnalysisData^ CurrentData = BackgroundAnalysis->LastAnalysisResult->Clone();
					obScriptParsing::Structurizer^ Parser = gcnew obScriptParsing::Structurizer(CurrentData,
						gcnew obScriptParsing::Structurizer::GetLineText(this, &ConcreteWorkspaceModel::GetLineText),
						TextEditor->CurrentLine);

					if (Parser->Valid)
						BoundParent->Controller->ShowOutline(BoundParent, Parser, this);

					E->Handled = true;
				}

				break;
			}

			if (E->Handled == false)
				BoundParent->Controller->BubbleKeyDownEvent(BoundParent, E);
		}

		void ConcreteWorkspaceModel::TextEditor_ScriptModified(Object^ Sender, textEditor::TextEditorScriptModifiedEventArgs^ E)
		{
			OnStateChangedDirty(E->ModifiedStatus);
		}

		void ConcreteWorkspaceModel::TextEditor_MouseClick(Object^ Sender, textEditor::TextEditorMouseClickEventArgs^ E)
		{
			if (Control::ModifierKeys == Keys::Control && E->Button == MouseButtons::Left)
			{
				String^ Token = TextEditor->GetTokenAtCharIndex(E->ScriptTextOffset);
				auto AttachedScript = intellisense::IntelliSenseBackend::Get()->GetAttachedScript(Token);
				if (AttachedScript)
					BoundParent->Controller->Jump(BoundParent, this, AttachedScript->GetIdentifier());
			}
		}

		generic <typename T> where T : textEditor::ILineAnchor
		bool RemoveInvalidatedAnchors(List<T>^ Source)
		{
			auto Invalidated = gcnew List<T>;
			for each (auto Itr in Source)
			{
				if (!Itr->Valid)
					Invalidated->Add(Itr);
			}

			for each (auto Itr in Invalidated)
				Source->Remove(Itr);

			return Invalidated->Count != 0;
		}

		void ConcreteWorkspaceModel::TextEditor_LineAnchorInvalidated(Object^ Sender, EventArgs^ E)
		{
			if (RemoveInvalidatedAnchors<ScriptDiagnosticMessage^>(Messages))
				OnStateChangedMessages();

			if (RemoveInvalidatedAnchors<ScriptBookmark^>(Bookmarks))
				OnStateChangedBookmarks();

			if (RemoveInvalidatedAnchors<ScriptFindResult^>(FindResults))
				OnStateChangedFindResults();
		}

		void ConcreteWorkspaceModel::BackgroundAnalysis_AnalysisComplete(Object^ Sender, scriptEditor::SemanticAnalysisCompleteEventArgs^ E)
		{
			BeginBatchUpdate(BatchUpdateSource::Messages);
			{
				ClearMessages(ScriptDiagnosticMessage::MessageSource::Validator, ScriptDiagnosticMessage::MessageType::All);
				for each (auto Itr in E->Result->AnalysisMessages)
				{
					AddMessage(Itr->Line, Itr->Message,
						Itr->Critical ? ScriptDiagnosticMessage::MessageType::Error : ScriptDiagnosticMessage::MessageType::Warning,
						ScriptDiagnosticMessage::MessageSource::Validator);
				}
			}
			EndBatchUpdate(BatchUpdateSource::Messages);
		}

		void ConcreteWorkspaceModel::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
		{
			AutoSaveTimer->Stop();
			AutoSaveTimer->Interval = preferences::SettingsHolder::Get()->Backup->AutoRecoveryInterval * 1000 * 60;
			AutoSaveTimer->Start();
		}

		void ConcreteWorkspaceModel::AutoSaveTimer_Tick(Object^ Sender, EventArgs^ E)
		{
			if (preferences::SettingsHolder::Get()->Backup->UseAutoRecovery)
			{
				if (Initialized == true && New == false && TextEditor->Modified == true)
				{
					TextEditor->SaveScriptToDisk(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()),
												 false, LongDescription, "txt");
				}
			}
		}

		void ConcreteWorkspaceModel::CheckAutoRecovery()
		{
			String^ CachePath = gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()) + LongDescription + ".txt";
			if (System::IO::File::Exists(CachePath))
			{
				try
				{
					System::DateTime LastWriteTime = System::IO::File::GetLastWriteTime(CachePath);
					if (BoundParent->Controller->MessageBox("An auto-recovery cache for the script '" + LongDescription + "' was found, dated " + LastWriteTime.ToShortDateString() + " " + LastWriteTime.ToLongTimeString() + ".\n\nWould you like to load it instead?",
						MessageBoxButtons::YesNo,
						MessageBoxIcon::Information) == DialogResult::Yes)
					{
						TextEditor->LoadFileFromDisk(CachePath);
					}

					Microsoft::VisualBasic::FileIO::FileSystem::DeleteFile(CachePath,
																		   Microsoft::VisualBasic::FileIO::UIOption::OnlyErrorDialogs,
																		   Microsoft::VisualBasic::FileIO::RecycleOption::SendToRecycleBin);
				}
				catch (Exception^ E)
				{
					DebugPrint("Couldn't access auto-recovery cache '" + LongDescription + "'!\n\tException: " + E->Message, true);
				}
			}
		}

		void ConcreteWorkspaceModel::ClearAutoRecovery()
		{
			try {
				System::IO::File::Delete(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()) + LongDescription + ".txt");
			}
			catch (...) {}
		}

		String^ ConcreteWorkspaceModel::GetLineText(UInt32 Line)
		{
			return TextEditor->GetText(Line);
		}

		void ConcreteWorkspaceModel::ShowSyncedScriptWarning()
		{
			Debug::Assert(Bound);

			if (scriptSync::DiskSync::Get()->IsScriptBeingSynced(EditorID))
			{
				BoundParent->Controller->MessageBox("The current script is actively being synced from/to disk. Modifying it inside the script editor will cause inconsistent and unexpected behaviour.",
													MessageBoxButtons::OK,
													MessageBoxIcon::Exclamation);
			}
		}

		void ConcreteWorkspaceModel::InitializeState(componentDLLInterface::ScriptData* ScriptData, bool NewScript)
		{
			Debug::Assert(ScriptData != nullptr);

			if (!NewScript)
				EditorID = gcnew String(ScriptData->EditorID);
			else
				EditorID = NEWSCRIPTID;

			FormID = ScriptData->FormID;
			Bytecode = ScriptData->ByteCode;
			BytecodeLength = ScriptData->Length;
			ScriptNativeObject = ScriptData->ParentForm;
			NewScriptFlag = NewScript;

			SetType(safe_cast<IWorkspaceModel::ScriptType>(ScriptData->Type));
			OnStateChangedDescription();
			OnStateChangedByteCodeSize(BytecodeLength);
			TextEditor->Modified = false;

			InitializeTextEditor(gcnew String(ScriptData->Text));

			if (Bound)
			{
				BoundParent->Enabled = true;

				if (!NewScript && !ScriptData->Compiled)
				{
					// skip default script forms
					if (FormID >= 0x800)
					{
						BoundParent->Controller->MessageBox("The current script has not been compiled. It will not be executed in-game until it has been compiled at least once.",
															MessageBoxButtons::OK,
															MessageBoxIcon::Exclamation);
					}
				}

				if (preferences::SettingsHolder::Get()->Backup->UseAutoRecovery)
					CheckAutoRecovery();

				TextEditor->FocusTextArea();
				ShowSyncedScriptWarning();
			}
		}

		bool ConcreteWorkspaceModel::PerformHouseKeeping()
		{
			bool Throwaway = false;
			return PerformHouseKeeping(Throwaway);
		}

		bool ConcreteWorkspaceModel::PerformHouseKeeping(bool% OperationCancelled)
		{
			if (TextEditor->Modified)
			{
				Debug::Assert(Bound == true);
				DialogResult Result = BoundParent->Controller->MessageBox("The current script '" + EditorID + "' has unsaved changes.\n\nDo you wish to save them?",
													   MessageBoxButtons::YesNoCancel,
													   MessageBoxIcon::Exclamation);
				bool HasWarnings = false;
				if (Result == DialogResult::Yes)
					return SaveScript(IWorkspaceModel::SaveOperation::Default, HasWarnings);
				else if (Result == DialogResult::No)
				{
					if (NewScriptFlag)
					{
						nativeWrapper::g_CSEInterfaceTable->ScriptEditor.DestroyScriptInstance(ScriptNativeObject);
						ScriptNativeObject = 0;
					}

					ClearAutoRecovery();
					return true;
				}
				else
				{
					OperationCancelled = true;
					return false;
				}
			}

			return true;
		}

		void ConcreteWorkspaceModel::Bind(IWorkspaceView^ To)
		{
			Debug::Assert(Bound == false);

			BoundParent = To;
			BoundParent->Controller->AttachModelInternalView(BoundParent, this);
			BoundParent->Enabled = Initialized;

			// update the view's state
			OnStateChangedType(ScriptType);
			OnStateChangedByteCodeSize(BytecodeLength);
			OnStateChangedDescription();
			OnStateChangedDirty(TextEditor->Modified);
			OnStateChangedMessages();
			OnStateChangedBookmarks();
			OnStateChangedFindResults();

			TextEditor->Bind();

			BoundParent->BreadcrumbManager->Bind(TextEditor, BackgroundAnalysis);
			IntelliSenseModel->Bind(BoundParent->IntelliSenseInterfaceView);
			BackgroundAnalysis->Resume();

			TextEditor->FocusTextArea();
			TextEditor->ScrollToCaret();

			ShowSyncedScriptWarning();
		}

		void ConcreteWorkspaceModel::Unbind()
		{
			if (Bound)
			{
				BackgroundAnalysis->Pause();
				IntelliSenseModel->Unbind();
				BoundParent->BreadcrumbManager->Unbind();
				TextEditor->Unbind();

				BoundParent->Controller->DetachModelInternalView(BoundParent, this);
				BoundParent = nullptr;
			}
		}

		void ConcreteWorkspaceModel::NewScript()
		{
			if (PerformHouseKeeping())
			{
				componentDLLInterface::ScriptData* Data = nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CreateNewScript();
				InitializeState(Data, true);
				nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);

				TextEditor->Modified = true;
			}
		}

		void ConcreteWorkspaceModel::OpenScript(componentDLLInterface::ScriptData* Data)
		{
			Debug::Assert(Data != nullptr);

			if (PerformHouseKeeping())
				InitializeState(Data, false);

			nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
		}

		bool ConcreteWorkspaceModel::SaveScript(IWorkspaceModel::SaveOperation Operation, bool% HasWarnings)
		{
			if (ScriptNativeObject == nullptr)
				return false;

			bool Result = false;
			DisposibleDataAutoPtr<componentDLLInterface::ScriptCompileData> CompileInteropData(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateCompileData());
			auto Data = BeginScriptCompilation();
			{
				if (Data->CanCompile)
				{
					Data->CompileResult = CompileInteropData.get();

					if (Operation == IWorkspaceModel::SaveOperation::NoCompile)
						nativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(false);

					CString ScriptText(Data->PreprocessedScriptText->Replace("\n", "\r\n"));
					CompileInteropData->Script.Text = ScriptText.c_str();
					CompileInteropData->Script.Type = (int)Type;
					CompileInteropData->Script.ParentForm = (TESForm*)ScriptNativeObject;
					CompileInteropData->PrintErrorsToConsole = false;

					nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileScript(CompileInteropData.get());
					Result = CompileInteropData->CompilationSuccessful;

					if (Operation == IWorkspaceModel::SaveOperation::NoCompile)
					{
						nativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(true);
						nativeWrapper::g_CSEInterfaceTable->ScriptEditor.RemoveScriptBytecode(ScriptNativeObject);
					}
					else if (Operation == IWorkspaceModel::SaveOperation::SavePlugin)
						nativeWrapper::g_CSEInterfaceTable->EditorAPI.SaveActivePlugin();
				}
			}
			EndScriptCompilation(Data);
			HasWarnings = Data->HasWarnings;

			if (Result)
				ClearAutoRecovery();

			return Result;
		}

		bool ConcreteWorkspaceModel::CloseScript(bool% OperationCancelled)
		{
			if (PerformHouseKeeping(OperationCancelled))
				return true;

			return false;
		}

		void ConcreteWorkspaceModel::NextScript()
		{
			if (PerformHouseKeeping())
			{
				DisposibleDataAutoPtr<componentDLLInterface::ScriptData> Data(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetNextScriptInList(ScriptNativeObject));
				if (Data)
					InitializeState(Data.get(), false);
			}
		}

		void ConcreteWorkspaceModel::PreviousScript()
		{
			if (PerformHouseKeeping())
			{
				DisposibleDataAutoPtr<componentDLLInterface::ScriptData> Data(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreviousScriptInList(ScriptNativeObject));
				if (Data)
					InitializeState(Data.get(), false);
			}
		}

		void ConcreteWorkspaceModel::SetType(IWorkspaceModel::ScriptType New)
		{
			ScriptType = New;
			OnStateChangedType(New);
		}

		String^ GetSanitizedIdentifier(String^ Identifier)
		{
			return intellisense::IntelliSenseBackend::Get()->SanitizeIdentifier(Identifier);
		}

		bool ConcreteWorkspaceModel::Sanitize()
		{
			obScriptParsing::Sanitizer^ Agent = gcnew obScriptParsing::Sanitizer(TextEditor->GetText());
			obScriptParsing::Sanitizer::eOperation Operation;

			if (preferences::SettingsHolder::Get()->Sanitizer->NormalizeIdentifiers)
				Operation = Operation | obScriptParsing::Sanitizer::eOperation::AnnealCasing;

			if (preferences::SettingsHolder::Get()->Sanitizer->PrefixIfElseIfWithEval)
				Operation = Operation | obScriptParsing::Sanitizer::eOperation::EvalifyIfs;

			if (preferences::SettingsHolder::Get()->Sanitizer->ApplyCompilerOverride)
				Operation = Operation | obScriptParsing::Sanitizer::eOperation::CompilerOverrideBlocks;

			if (preferences::SettingsHolder::Get()->Sanitizer->IndentLines)
				Operation = Operation | obScriptParsing::Sanitizer::eOperation::IndentLines;

			bool Result = Agent->SanitizeScriptText(Operation, gcnew obScriptParsing::Sanitizer::GetSanitizedIdentifier(GetSanitizedIdentifier));
			if (Result)
				TextEditor->SetText(Agent->Output, false);

			return Result;
		}

		void ConcreteWorkspaceModel::AddMessage(UInt32 Line, String^ Text, ScriptDiagnosticMessage::MessageType Type, ScriptDiagnosticMessage::MessageSource Source)
		{
			if (Line > TextEditor->LineCount)
				Line = TextEditor->LineCount;

			auto LineAnchor = TextEditor->CreateLineAnchor(Line);
			auto NewMessage = gcnew ScriptDiagnosticMessage(LineAnchor,
															Text->Replace("\t", "")->Replace("\r", "")->Replace("\n", ""),
															Type, Source);
			Messages->Add(NewMessage);
			OnStateChangedMessages();
		}

		void ConcreteWorkspaceModel::ClearMessages(ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter)
		{
			auto Buffer = gcnew List<ScriptDiagnosticMessage^> ;

			for each (auto Itr in Messages)
			{
				bool MatchedSource = SourceFilter == ScriptDiagnosticMessage::MessageSource::All || Itr->Source == SourceFilter;
				bool MatchedType = TypeFilter == ScriptDiagnosticMessage::MessageType::All || Itr->Type == TypeFilter;

				if (MatchedSource && MatchedType)
					Buffer->Add(Itr);
			}

			if (Buffer->Count)
			{
				for each (auto Itr in Buffer)
					Messages->Remove(Itr);

				OnStateChangedBookmarks();
			}
		}

		bool ConcreteWorkspaceModel::GetMessages(UInt32 Line, ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter, List<ScriptDiagnosticMessage^>^% OutMessages)
		{
			bool Result = false;
			for each (auto Itr in Messages)
			{
				if (Itr->Line == Line)
				{
					bool MatchedSource = SourceFilter == ScriptDiagnosticMessage::MessageSource::All || Itr->Source == SourceFilter;
					bool MatchedType = TypeFilter == ScriptDiagnosticMessage::MessageType::All || Itr->Type == TypeFilter;

					if (MatchedSource && MatchedType)
					{
						OutMessages->Add(Itr);
						Result = true;
					}
				}
			}

			return Result;
		}

		UInt32 ConcreteWorkspaceModel::GetErrorCount(UInt32 Line)
		{
			UInt32 Count = 0;
			for each (auto Itr in Messages)
			{
				if ((Itr->Line == 0 || Itr->Line == Line) && Itr->Type == ScriptDiagnosticMessage::MessageType::Error)
					++Count;
			}

			return Count;
		}

		UInt32 ConcreteWorkspaceModel::GetWarningCount(UInt32 Line)
		{
			UInt32 Count = 0;
			for each (auto Itr in Messages)
			{
				if ((Itr->Line == 0 || Itr->Line == Line) && Itr->Type == ScriptDiagnosticMessage::MessageType::Warning)
					++Count;
			}

			return Count;
		}

		void ConcreteWorkspaceModel::AddBookmark(UInt32 Line, String^ BookmarkText)
		{
			if (Line > TextEditor->LineCount)
				Line = TextEditor->LineCount;

			if (LookupBookmark(Line, BookmarkText) != nullptr)
				return;

			auto LineAnchor = TextEditor->CreateLineAnchor(Line);
			auto NewBookmark= gcnew ScriptBookmark(LineAnchor, BookmarkText->Replace("\t", ""));
			Bookmarks->Add(NewBookmark);
			OnStateChangedBookmarks();
		}

		void ConcreteWorkspaceModel::RemoveBookmark(UInt32 Line, String^ BookmarkText)
		{
			auto ToRemove = LookupBookmark(Line, BookmarkText);
			if (ToRemove)
			{
				Bookmarks->Remove(ToRemove);
				OnStateChangedBookmarks();
			}
		}

		void ConcreteWorkspaceModel::ClearBookmarks()
		{
			Bookmarks->Clear();
			OnStateChangedBookmarks();
		}

		bool ConcreteWorkspaceModel::GetBookmarks(UInt32 Line, List<ScriptBookmark^>^% OutBookmarks)
		{
			bool Result = false;
			for each (auto Itr in Bookmarks)
			{
				if (Itr->Line == Line)
				{
					OutBookmarks->Add(Itr);
					Result = true;
				}
			}

			return Result;
		}

		UInt32 ConcreteWorkspaceModel::GetBookmarkCount(UInt32 Line)
		{
			UInt32 Count = 0;
			for each (auto Itr in Bookmarks)
			{
				if (Itr->Line == Line)
					++Count;
			}

			return Count;
		}

		void ConcreteWorkspaceModel::AddFindResult(UInt32 Line, String^ PreviewText, UInt32 Hits)
		{
			if (Line > TextEditor->LineCount)
				Line = TextEditor->LineCount;

			auto LineAnchor = TextEditor->CreateLineAnchor(Line);
			auto NewFindResult= gcnew ScriptFindResult(LineAnchor, PreviewText->Replace("\t", ""), Hits);
			FindResults->Add(NewFindResult);
			OnStateChangedFindResults();
		}

		void ConcreteWorkspaceModel::ClearFindResults()
		{
			FindResults->Clear();
			OnStateChangedFindResults();
		}

		void ConcreteWorkspaceModel::BeginBatchUpdate(BatchUpdateSource Source)
		{
			Debug::Assert(ActiveBatchUpdateSource == BatchUpdateSource::None || ActiveBatchUpdateSource == Source);
			Debug::Assert(Source != BatchUpdateSource::None);

			++ActiveBatchUpdateSourceCounter;
			ActiveBatchUpdateSource = Source;
		}

		void ConcreteWorkspaceModel::EndBatchUpdate(BatchUpdateSource Source)
		{
			Debug::Assert(ActiveBatchUpdateSource != BatchUpdateSource::None);

			ActiveBatchUpdateSourceCounter--;
			Debug::Assert(ActiveBatchUpdateSourceCounter >= 0);

			if (ActiveBatchUpdateSourceCounter == 0)
			{
				auto CompletedBatchUpdateSource = ActiveBatchUpdateSource;
				ActiveBatchUpdateSource = BatchUpdateSource::None;

				switch (CompletedBatchUpdateSource)
				{
				case BatchUpdateSource::Messages:
					OnStateChangedMessages();
					break;
				case BatchUpdateSource::Bookmarks:
					OnStateChangedBookmarks();
					break;
				case BatchUpdateSource::FindResults:
					OnStateChangedFindResults();
					break;
				}
			}
		}

		void ConcreteWorkspaceModel::OnStateChangedDirty(bool Modified)
		{
			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->EventType = IWorkspaceModel::StateChangeEventArgs::Type::Dirty;
			E->Dirty = Modified;
			StateChanged(this, E);
		}

		void ConcreteWorkspaceModel::OnStateChangedByteCodeSize(UInt32 Size)
		{
			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->EventType = IWorkspaceModel::StateChangeEventArgs::Type::ByteCodeSize;
			E->ByteCodeSize = Size;
			StateChanged(this, E);
		}

		void ConcreteWorkspaceModel::OnStateChangedType(IWorkspaceModel::ScriptType Type)
		{
			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->EventType = IWorkspaceModel::StateChangeEventArgs::Type::ScriptType;
			E->ScriptType = Type;
			StateChanged(this, E);
		}

		void ConcreteWorkspaceModel::OnStateChangedDescription()
		{
			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->EventType = IWorkspaceModel::StateChangeEventArgs::Type::Description;
			E->ShortDescription = ShortDescription;
			E->LongDescription = LongDescription;
			StateChanged(this, E);
		}

		void ConcreteWorkspaceModel::OnStateChangedMessages()
		{
			if (ActiveBatchUpdateSource == BatchUpdateSource::Messages)
				return;

			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->EventType = IWorkspaceModel::StateChangeEventArgs::Type::Messages;
			E->Messages = Messages;
			StateChanged(this, E);
		}

		void ConcreteWorkspaceModel::OnStateChangedBookmarks()
		{
			if (ActiveBatchUpdateSource == BatchUpdateSource::Bookmarks)
				return;

			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->EventType = IWorkspaceModel::StateChangeEventArgs::Type::Bookmarks;
			E->Bookmarks = Bookmarks;
			StateChanged(this, E);
		}

		void ConcreteWorkspaceModel::OnStateChangedFindResults()
		{
			if (ActiveBatchUpdateSource == BatchUpdateSource::FindResults)
				return;

			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->EventType = IWorkspaceModel::StateChangeEventArgs::Type::FindResults;
			E->FindResults = FindResults;
			StateChanged(this, E);
		}

		void DummyOutputWrapper(int Line, String^ Message)
		{
			;//
		}

		System::String^ ConcreteWorkspaceModel::PreprocessScriptText(String^ ScriptText, bool SuppressErrors, bool% OutPreprocessResult, bool% OutContainsDirectives)
		{
			String^ Preprocessed = "";
			scriptPreprocessor::StandardOutputError^ ErrorOutput = gcnew scriptPreprocessor::StandardOutputError(&DummyOutputWrapper);
			if (SuppressErrors == false)
			{
				ErrorOutput = gcnew scriptPreprocessor::StandardOutputError(this, &ConcreteWorkspaceModel::TrackPreprocessorMessage);
				BeginBatchUpdate(BatchUpdateSource::Messages);
				ClearMessages(ScriptDiagnosticMessage::MessageSource::Preprocessor, ScriptDiagnosticMessage::MessageType::All);
			}


			auto PreprocessorParams = gcnew PreprocessorParams(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
																		 gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
																		 preferences::SettingsHolder::Get()->Preprocessor->AllowMacroRedefs,
																		 preferences::SettingsHolder::Get()->Preprocessor->NumPasses);

			bool Result = Preprocessor::Get()->PreprocessScript(ScriptText, Preprocessed, ErrorOutput, PreprocessorParams);

			if (SuppressErrors == false)
				EndBatchUpdate(BatchUpdateSource::Messages);

			OutPreprocessResult = Result;
			return Preprocessed;
		}

		void ConcreteWorkspaceModel::TrackPreprocessorMessage(int Line, String^ Message)
		{
			AddMessage(Line, Message, ScriptDiagnosticMessage::MessageType::Error, ScriptDiagnosticMessage::MessageSource::Preprocessor);
		}

		CompilationData^ ConcreteWorkspaceModel::BeginScriptCompilation()
		{
			Debug::Assert(CompilationInProgress == false);
			CompilationInProgress = true;

			BeginBatchUpdate(BatchUpdateSource::Messages);

			CompilationData^ Result = gcnew CompilationData;
			Result->UnpreprocessedScriptText = TextEditor->GetText();

			auto SemanticAnalysisData = BackgroundSemanticAnalyzer->DoSynchronousAnalysis(false);
			ClearMessages(ScriptDiagnosticMessage::MessageSource::Validator, ScriptDiagnosticMessage::MessageType::All);

			for each (auto Itr in SemanticAnalysisData->AnalysisMessages)
			{
				AddMessage(Itr->Line, Itr->Message,
						   Itr->Critical ? ScriptDiagnosticMessage::MessageType::Error : ScriptDiagnosticMessage::MessageType::Warning,
						   ScriptDiagnosticMessage::MessageSource::Validator);
			}

			if (!SemanticAnalysisData->HasCriticalMessages && !SemanticAnalysisData->MalformedStructure)
			{
				Result->PreprocessedScriptText = PreprocessScriptText(Result->UnpreprocessedScriptText, false, Result->CanCompile, Result->HasPreprocessorDirectives);
				if (Result->CanCompile)
				{
					auto Metadata = gcnew scriptEditor::ScriptTextMetadata;
					Metadata->CaretPos = TextEditor->Caret;
					Metadata->HasPreprocessorDirectives = Result->HasPreprocessorDirectives;
					for each (auto Itr in Bookmarks)
						Metadata->Bookmarks->Add(gcnew ScriptTextMetadata::Bookmark(Itr->Line, Itr->Text));

					Result->SerializedMetadata = scriptEditor::ScriptTextMetadataHelper::SerializeMetadata(Metadata);
					ClearMessages(ScriptDiagnosticMessage::MessageSource::Compiler, ScriptDiagnosticMessage::MessageType::All);
				}
			}

			// doesn't include compiler warnings for obvious reasons but it's okay since all compiler messages are errors
			Result->HasWarnings = GetWarningCount(0);
			return Result;
		}

		void ConcreteWorkspaceModel::EndScriptCompilation(CompilationData^ Data)
		{
			Debug::Assert(CompilationInProgress == true);
			CompilationInProgress = false;

			String^ kRepeatedString = "Compiled script not saved!";

			if (Data->CanCompile)
			{
				if (Data->CompileResult->CompilationSuccessful)
				{
					String^ OriginalText = Data->UnpreprocessedScriptText + Data->SerializedMetadata;
					CString OrgScriptText(OriginalText);
					nativeWrapper::g_CSEInterfaceTable->ScriptEditor.SetScriptText(ScriptNativeObject, OrgScriptText.c_str());

					// update model local state
					EditorID = gcnew String(Data->CompileResult->Script.EditorID);
					FormID = Data->CompileResult->Script.FormID;
					Bytecode = Data->CompileResult->Script.ByteCode;
					BytecodeLength = Data->CompileResult->Script.Length;

					SetType(safe_cast<IWorkspaceModel::ScriptType>(Data->CompileResult->Script.Type));
					OnStateChangedDescription();
					OnStateChangedByteCodeSize(BytecodeLength);
					TextEditor->Modified = false;
				}
				else
				{
					for (int i = 0; i < Data->CompileResult->CompileErrorData.Count; i++)
					{
						String^ Message = gcnew String(Data->CompileResult->CompileErrorData.ErrorListHead[i].Message);
						Message = Message->Replace(kRepeatedString, String::Empty);

						int Line = Data->CompileResult->CompileErrorData.ErrorListHead[i].Line;
						if (Line < 1)
							Line = 1;

						AddMessage(Line, Message, ScriptDiagnosticMessage::MessageType::Error, ScriptDiagnosticMessage::MessageSource::Compiler);
					}
				}
			}

			EndBatchUpdate(BatchUpdateSource::Messages);
		}

		void ConcreteWorkspaceModel::InitializeTextEditor(String^ RawScriptText)
		{
			ClearMessages(ScriptDiagnosticMessage::MessageSource::All, ScriptDiagnosticMessage::MessageType::All);
			ClearBookmarks();
			ClearFindResults();

			String^ ExtractedScriptText = "";
			auto ExtractedMetadata = gcnew ScriptTextMetadata();
			ScriptTextMetadataHelper::DeserializeRawScriptText(RawScriptText, ExtractedScriptText, ExtractedMetadata);

			if (ExtractedMetadata->CaretPos > ExtractedScriptText->Length)
				ExtractedMetadata->CaretPos = ExtractedScriptText->Length;
			else if (ExtractedMetadata->CaretPos < 0)
				ExtractedMetadata->CaretPos = 0;

			TextEditor->InitializeState(ExtractedScriptText, ExtractedMetadata->CaretPos);

			BeginBatchUpdate(BatchUpdateSource::Bookmarks);
			{
				for each (auto Itr in ExtractedMetadata->Bookmarks)
				{
					if (Itr->Line > 0 && Itr->Line <= TextEditor->LineCount)
						AddBookmark(Itr->Line, Itr->Text);
				}
			}
			EndBatchUpdate(BatchUpdateSource::Bookmarks);

			intellisense::IntelliSenseBackend::Get()->Refresh(false);
		}

		ScriptBookmark^ ConcreteWorkspaceModel::LookupBookmark(UInt32 Line, String^ Text)
		{
			for each (auto Itr in Bookmarks)
			{
				if (Itr->Line == Line && Itr->Text->Equals(Text))
					return Itr;
			}

			return nullptr;
		}

		String^ ConcreteWorkspaceModel::GetText(bool Preprocess, bool% PreprocessResult, bool SuppressPreprocessorErrors)
		{
			if (Preprocess)
			{
				bool Throwaway;
				return PreprocessScriptText(TextEditor->GetText(), SuppressPreprocessorErrors, PreprocessResult, Throwaway);
			}
			else
				return TextEditor->GetText();
		}

		// ConcreteWorkspaceModelController
		void ConcreteWorkspaceModelController::Bind(IWorkspaceModel^ Model, IWorkspaceView^ To)
		{
			Debug::Assert(Model != nullptr);
			Debug::Assert(To != nullptr);

			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->Bind(To);
		}

		void ConcreteWorkspaceModelController::Unbind(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);

			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->Unbind();
		}

		void ConcreteWorkspaceModelController::SetText(IWorkspaceModel^ Model, String^ Text, bool ResetUndoStack)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->TextEditor->SetText(Text, ResetUndoStack);
		}

		String^ ConcreteWorkspaceModelController::GetText(IWorkspaceModel^ Model, bool Preprocess, bool% PreprocessResult, bool SuppressPreprocessorErrors)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->GetText(Preprocess, PreprocessResult, SuppressPreprocessorErrors);
		}

		int ConcreteWorkspaceModelController::GetCaret(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->TextEditor->Caret;
		}

		void ConcreteWorkspaceModelController::SetCaret(IWorkspaceModel^ Model, int Index)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->TextEditor->Caret = Index;
		}

		String^ ConcreteWorkspaceModelController::GetSelection(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->TextEditor->GetSelectedText();
		}

		String^ ConcreteWorkspaceModelController::GetCaretToken(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->TextEditor->GetTokenAtCaretPos();
		}

		void ConcreteWorkspaceModelController::AcquireInputFocus(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->TextEditor->FocusTextArea();
		}

		void ConcreteWorkspaceModelController::New(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->NewScript();
		}

		void ConcreteWorkspaceModelController::Open(IWorkspaceModel^ Model, componentDLLInterface::ScriptData* Data)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->OpenScript(Data);
		}

		bool ConcreteWorkspaceModelController::Save(IWorkspaceModel^ Model, IWorkspaceModel::SaveOperation Operation, bool% HasWarnings)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			if (!Concrete->Dirty)
				return true;

			return Concrete->SaveScript(Operation, HasWarnings);
		}

		bool ConcreteWorkspaceModelController::Close(IWorkspaceModel^ Model, bool% OperationCancelled)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->CloseScript(OperationCancelled);
		}

		void ConcreteWorkspaceModelController::Next(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->NextScript();
		}

		void ConcreteWorkspaceModelController::Previous(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->PreviousScript();
		}

		void ConcreteWorkspaceModelController::CompileDepends(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			CString CEID(Concrete->EditorID);
			nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileDependencies(CEID.c_str());
		}

		void ConcreteWorkspaceModelController::SetType(IWorkspaceModel^ Model, IWorkspaceModel::ScriptType New)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			if (Concrete->Type != New)
				Concrete->TextEditor->Modified = true;

			Concrete->SetType(New);
		}

		void ConcreteWorkspaceModelController::GotoLine(IWorkspaceModel^ Model, UInt32 Line)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->TextEditor->ScrollToLine(Line);
		}

		UInt32 ConcreteWorkspaceModelController::GetLineCount(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->TextEditor->LineCount;
		}

		bool ConcreteWorkspaceModelController::Sanitize(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->Sanitize();
		}

		void ConcreteWorkspaceModelController::BindToForm(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			nativeWrapper::g_CSEInterfaceTable->ScriptEditor.BindScript((CString(Concrete->EditorID)).c_str(),
																		(HWND)Concrete->BoundParent->WindowHandle);
		}

		void ConcreteWorkspaceModelController::LoadFromDisk(IWorkspaceModel^ Model, String^ PathToFile)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->TextEditor->LoadFileFromDisk(PathToFile);
		}

		void ConcreteWorkspaceModelController::SaveToDisk(IWorkspaceModel^ Model, String^ PathToFile, bool PathIncludesFileName, String^ Extension)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->TextEditor->SaveScriptToDisk(PathToFile, PathIncludesFileName, Concrete->LongDescription, Extension);
		}

		textEditor::FindReplaceResult^ ConcreteWorkspaceModelController::FindReplace(IWorkspaceModel^ Model,
																						textEditor::eFindReplaceOperation Operation,
																						String^ Query,
																						String^ Replacement,
																						textEditor::eFindReplaceOptions Options)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			auto Result = Concrete->TextEditor->FindReplace(Operation, Query, Replacement, Options);
			if (!Result->HasError)
			{
				Concrete->BeginBatchUpdate(ConcreteWorkspaceModel::BatchUpdateSource::FindResults);
				Concrete->ClearFindResults();
				for each (auto Itr in Result->Hits)
					Concrete->AddFindResult(Itr->Line, Itr->Text, Itr->Hits);
				Concrete->EndBatchUpdate(ConcreteWorkspaceModel::BatchUpdateSource::FindResults);
			}

			return Result;
		}

		bool ConcreteWorkspaceModelController::GetOffsetViewerData(IWorkspaceModel^ Model, String^% OutText, void** OutBytecode, UInt32% OutLength)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			if (Concrete->TextEditor->Modified)
				return false;

			bool PP = false;
			String^ Preprocessed = Concrete->GetText(true, PP, true);
			if (PP == false)
				return false;

			OutText = Preprocessed;
			*OutBytecode = Concrete->Bytecode;
			OutLength = Concrete->BytecodeLength;
			return true;
		}

		bool ConcreteWorkspaceModelController::ApplyRefactor(IWorkspaceModel^ Model, IWorkspaceModel::RefactorOperation Operation, Object^ Arg)
		{
			Debug::Assert(Model != nullptr);
			Debug::Assert(Model->Bound == true);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			bool Result = false;

			// UI related code ought to be in the view but can't be bothered to refactor the refactor tools code
			switch (Operation)
			{
			case cse::scriptEditor::IWorkspaceModel::RefactorOperation::DocumentScript:
				{
					refactoring::EditScriptComponentDialog DocumentScriptData(Concrete->BoundParent->WindowHandle,
																			  Concrete->EditorID,
																			  refactoring::EditScriptComponentDialog::OperationType::DocumentScript,
																			  "Script Description");

					if (DocumentScriptData.HasResult)
					{
						String^ Description = "";
						DocumentScriptData.ResultData->LookupEditDataByName("Script Description", Description);
						obScriptParsing::Documenter^ Agent = gcnew obScriptParsing::Documenter(Concrete->TextEditor->GetText());
						Agent->Document(Description, DocumentScriptData.ResultData->AsTable());

						Concrete->TextEditor->SetText(Agent->Output, false);
						Result = true;
					}
				}

				break;
			case cse::scriptEditor::IWorkspaceModel::RefactorOperation::RenameVariables:
				{
					refactoring::EditScriptComponentDialog RenameVariablesData(Concrete->BoundParent->WindowHandle,
																			   Concrete->EditorID,
																			   refactoring::EditScriptComponentDialog::OperationType::RenameVariables,
																			   "");

					if (RenameVariablesData.HasResult)
					{
						List<CString^>^ StringAllocations = gcnew List<CString^>();
						List<refactoring::EditScriptComponentData::ScriptComponent^>^ RenameEntries = gcnew List<refactoring::EditScriptComponentData::ScriptComponent^>();

						for each (refactoring::EditScriptComponentData::ScriptComponent^ Itr in RenameVariablesData.ResultData->ScriptComponentList)
						{
							if (Itr->EditData != "")
								RenameEntries->Add(Itr);
						}

						if (RenameEntries->Count)
						{
							componentDLLInterface::ScriptVarRenameData* RenameData = nativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateVarRenameData(RenameEntries->Count);

							for (int i = 0; i < RenameData->ScriptVarListCount; i++)
							{
								componentDLLInterface::ScriptVarRenameData::ScriptVarInfo* Data = &RenameData->ScriptVarListHead[i];
								CString^ OldID = gcnew CString(RenameEntries[i]->ElementName);
								CString^ NewID = gcnew CString(RenameEntries[i]->EditData);

								Data->OldName = OldID->c_str();
								Data->NewName = NewID->c_str();

								StringAllocations->Add(OldID);
								StringAllocations->Add(NewID);
							}

							CString CEID(Concrete->EditorID);
							nativeWrapper::g_CSEInterfaceTable->ScriptEditor.UpdateScriptVarNames(CEID.c_str(), RenameData);

							for each (CString^ Itr in StringAllocations)
								delete Itr;

							nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(RenameData, false);

							Result = true;
						}

						StringAllocations->Clear();
						RenameEntries->Clear();
					}
				}

				break;
			case cse::scriptEditor::IWorkspaceModel::RefactorOperation::ModifyVariableIndices:
				{
					refactoring::ModifyVariableIndicesDialog ModifyIndicesData(Concrete->BoundParent->WindowHandle, Concrete->EditorID);

					if (ModifyIndicesData.IndicesUpdated)
					{
						Concrete->TextEditor->Modified = true;
						Result = true;
					}
				}

				break;
			case cse::scriptEditor::IWorkspaceModel::RefactorOperation::CreateUDF:
				{
					String^ UDFName = (String^)Arg;

					refactoring::CreateUDFImplementationDialog UDFData(Concrete->BoundParent->WindowHandle);

					if (UDFData.HasResult)
					{
						String^ UDFScriptText = "scn " + UDFName + "\n\n";
						String^ ParamList = "{ ";
						for each (refactoring::CreateUDFImplementationData::ParameterData^ Itr in UDFData.ResultData->ParameterList)
						{
							UDFScriptText += Itr->Type + " " + Itr->Name + "\n";
							ParamList += Itr->Name + " ";
						}
						ParamList += "}";
						if (UDFData.ResultData->ParameterList->Count)
							UDFScriptText += "\n";

						UDFScriptText += "begin function " + ParamList + "\n\nend\n";

						NewTabOperationArgs^ E = gcnew NewTabOperationArgs;
						E->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::SetText;
						E->NewText = UDFScriptText;

						Concrete->BoundParent->Controller->NewTab(Concrete->BoundParent, E);
						Result = true;
					}

					break;
				}
			}

			return Result;
		}

		void ConcreteWorkspaceModelController::JumpToScript(IWorkspaceModel^ Model, String^ ScriptEditorID)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			if (String::Compare(ScriptEditorID, Concrete->EditorID, true))
				Concrete->BoundParent->Controller->Jump(Concrete->BoundParent, Concrete, ScriptEditorID);
		}

		void ConcreteWorkspaceModelController::AddMessage(IWorkspaceModel^ Model, UInt32 Line, String^ Text, ScriptDiagnosticMessage::MessageType Type, ScriptDiagnosticMessage::MessageSource Source)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->AddMessage(Line, Text, Type, Source);
		}

		void ConcreteWorkspaceModelController::ClearMessages(IWorkspaceModel^ Model, ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->ClearMessages(SourceFilter, TypeFilter);
		}

		bool ConcreteWorkspaceModelController::GetMessages(IWorkspaceModel^ Model, UInt32 Line, ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter, List<ScriptDiagnosticMessage^>^% OutMessages)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			return Concrete->GetMessages(Line, SourceFilter, TypeFilter, OutMessages);
		}

		UInt32 ConcreteWorkspaceModelController::GetErrorCount(IWorkspaceModel^ Model, UInt32 Line)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			return Concrete->GetErrorCount(Line);
		}

		UInt32 ConcreteWorkspaceModelController::GetWarningCount(IWorkspaceModel^ Model, UInt32 Line)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			return Concrete->GetWarningCount(Line);
		}

		void ConcreteWorkspaceModelController::BeginUpdateMessages(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->BeginBatchUpdate(ConcreteWorkspaceModel::BatchUpdateSource::Messages);
		}

		void ConcreteWorkspaceModelController::EndUpdateMessages(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->EndBatchUpdate(ConcreteWorkspaceModel::BatchUpdateSource::Messages);
		}

		void ConcreteWorkspaceModelController::AddBookmark(IWorkspaceModel^ Model, UInt32 Line, String^ BookmarkText)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->AddBookmark(Line, BookmarkText);
		}

		void ConcreteWorkspaceModelController::RemoveBookmark(IWorkspaceModel^ Model, UInt32 Line, String^ BookmarkText)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->RemoveBookmark(Line, BookmarkText);
		}

		void ConcreteWorkspaceModelController::ClearBookmarks(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->ClearBookmarks();
		}

		bool ConcreteWorkspaceModelController::GetBookmarks(IWorkspaceModel^ Model, UInt32 Line, List<ScriptBookmark^>^% OutBookmarks)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			return Concrete->GetBookmarks(Line, OutBookmarks);
		}

		UInt32 ConcreteWorkspaceModelController::GetBookmarkCount(IWorkspaceModel^ Model, UInt32 Line)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			return Concrete->GetBookmarkCount(Line);
		}

		void ConcreteWorkspaceModelController::BeginUpdateBookmarks(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->BeginBatchUpdate(ConcreteWorkspaceModel::BatchUpdateSource::Bookmarks);
		}

		void ConcreteWorkspaceModelController::EndUpdateBookmarks(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->EndBatchUpdate(ConcreteWorkspaceModel::BatchUpdateSource::Bookmarks);
		}

		void ConcreteWorkspaceModelController::AddFindResult(IWorkspaceModel^ Model, UInt32 Line, String^ PreviewText, UInt32 Hits)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->AddFindResult(Line, PreviewText, Hits);
		}

		void ConcreteWorkspaceModelController::ClearFindResults(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->ClearFindResults();
		}

		void ConcreteWorkspaceModelController::BeginUpdateFindResults(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->BeginBatchUpdate(ConcreteWorkspaceModel::BatchUpdateSource::FindResults);
		}

		void ConcreteWorkspaceModelController::EndUpdateFindResults(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;
			Concrete->EndBatchUpdate(ConcreteWorkspaceModel::BatchUpdateSource::FindResults);
		}

		// ConcreteWorkspaceModelFactory
		ConcreteWorkspaceModelFactory::ConcreteWorkspaceModelFactory()
		{
			Allocations = gcnew List < ConcreteWorkspaceModel^ >;
		}

		ConcreteWorkspaceModelFactory::~ConcreteWorkspaceModelFactory()
		{
			for each (auto Itr in Allocations)
				delete Itr;

			Allocations->Clear();
		}

		IWorkspaceModel^ ConcreteWorkspaceModelFactory::CreateModel(componentDLLInterface::ScriptData* Data)
		{
			ConcreteWorkspaceModel^ New = gcnew ConcreteWorkspaceModel((ConcreteWorkspaceModelController^)CreateController(),
																	 this,
																	 Data);
			Allocations->Add(New);
			return New;
		}

		IWorkspaceModelController^ ConcreteWorkspaceModelFactory::CreateController()
		{
			return gcnew ConcreteWorkspaceModelController();
		}

		void ConcreteWorkspaceModelFactory::Remove(ConcreteWorkspaceModel^ Allocation)
		{
			Allocations->Remove(Allocation);
		}
	}
}