#include "WorkspaceModel.h"
#include "[Common]\CustomInputBox.h"
#include "Preferences.h"
#include "IntelliSenseInterfaceModel.h"
#include "IntelliSenseDatabase.h"
#include "RefactorTools.h"

namespace cse
{
	namespace scriptEditor
	{
		ConcreteWorkspaceModel::ConcreteWorkspaceModel(ConcreteWorkspaceModelController^ Controller, ConcreteWorkspaceModelFactory^ Factory,
													   componentDLLInterface::ScriptData* Data)
		{
			ModelController = Controller;
			ModelFactory = Factory;

			CurrentScript = nullptr;
			CurrentScriptType = IWorkspaceModel::ScriptType::Object;
			CurrentScriptEditorID = FIRSTRUNSCRIPTID;
			CurrentScriptFormID = 0;
			CurrentScriptBytecode = 0;
			CurrentScriptBytecodeLength = 0;
			NewScriptFlag = false;
			Closed = false;
			BoundParent = nullptr;

			AutoSaveTimer = gcnew Timer();
			AutoSaveTimer->Interval = preferences::SettingsHolder::Get()->Backup->AutoRecoveryInterval * 1000 * 60;

			Font^ CustomFont = safe_cast<Font^>(preferences::SettingsHolder::Get()->Appearance->TextFont->Clone());
			int TabSize = preferences::SettingsHolder::Get()->Appearance->TabSize;

			TextEditor = gcnew textEditors::avalonEditor::AvalonEditTextEditor(this,
																			   gcnew textEditors::avalonEditor::JumpToScriptHandler(this, &ConcreteWorkspaceModel::JumpToScript),
																			   CustomFont, TabSize);
			IntelliSenseModel = gcnew intellisense::IntelliSenseInterfaceModel(TextEditor);


			TextEditorKeyDownHandler = gcnew KeyEventHandler(this, &ConcreteWorkspaceModel::TextEditor_KeyDown);
			TextEditorScriptModifiedHandler = gcnew textEditors::TextEditorScriptModifiedEventHandler(this, &ConcreteWorkspaceModel::TextEditor_ScriptModified);
			TextEditorMouseClickHandler = gcnew textEditors::TextEditorMouseClickEventHandler(this, &ConcreteWorkspaceModel::TextEditor_MouseClick);

			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ScriptEditorPreferences_Saved);
			AutoSaveTimerTickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::AutoSaveTimer_Tick);

			TextEditor->KeyDown += TextEditorKeyDownHandler;
			TextEditor->ScriptModified += TextEditorScriptModifiedHandler;
			TextEditor->MouseClick += TextEditorMouseClickHandler;

			preferences::SettingsHolder::Get()->SavedToDisk += ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick += AutoSaveTimerTickHandler;

			AutoSaveTimer->Start();

			if (Data && Data->ParentForm)
				Setup(Data, false, false);

			nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
		}

		ConcreteWorkspaceModel::~ConcreteWorkspaceModel()
		{
			Unbind();

			AutoSaveTimer->Stop();

			TextEditor->KeyDown -= TextEditorKeyDownHandler;
			TextEditor->ScriptModified -= TextEditorScriptModifiedHandler;
			TextEditor->MouseClick -= TextEditorMouseClickHandler;

			preferences::SettingsHolder::Get()->SavedToDisk -= ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick -= AutoSaveTimerTickHandler;

			SAFEDELETE_CLR(TextEditorKeyDownHandler);
			SAFEDELETE_CLR(TextEditorScriptModifiedHandler);
			SAFEDELETE_CLR(TextEditorMouseClickHandler);
			SAFEDELETE_CLR(ScriptEditorPreferencesSavedHandler);
			SAFEDELETE_CLR(AutoSaveTimerTickHandler);

			SAFEDELETE_CLR(AutoSaveTimer);
			SAFEDELETE_CLR(IntelliSenseModel);
			SAFEDELETE_CLR(TextEditor);

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
					obScriptParsing::AnalysisData^ CurrentData = TextEditor->GetSemanticAnalysisCache(false, false);
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

		void ConcreteWorkspaceModel::TextEditor_ScriptModified(Object^ Sender, textEditors::TextEditorScriptModifiedEventArgs^ E)
		{
			OnStateChangedDirty(E->ModifiedStatus);
		}

		void ConcreteWorkspaceModel::TextEditor_MouseClick(Object^ Sender, textEditors::TextEditorMouseClickEventArgs^ E)
		{
			if (Control::ModifierKeys == Keys::Control && E->Button == MouseButtons::Left)
			{
				String^ Token = TextEditor->GetTokenAtCharIndex(E->ScriptTextOffset);
				auto AttachedScript = intellisense::IntelliSenseBackend::Get()->GetAttachedScript(Token);
				if (AttachedScript)
					BoundParent->Controller->Jump(BoundParent, this, AttachedScript->GetIdentifier());
			}
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

		void ConcreteWorkspaceModel::JumpToScript(String^ TargetEditorID)
		{
			if (String::Compare(TargetEditorID, CurrentScriptEditorID, true) && Bound)
				BoundParent->Controller->Jump(BoundParent, this, TargetEditorID);
		}

		String^ ConcreteWorkspaceModel::GetLineText(UInt32 Line)
		{
			return TextEditor->GetText(Line);
		}

		void ConcreteWorkspaceModel::Setup(componentDLLInterface::ScriptData* Data, bool PartialUpdate, bool NewScript)
		{
			if (Data == nullptr)
			{
				CurrentScript = nullptr;
				CurrentScriptType = IWorkspaceModel::ScriptType::Object;
				CurrentScriptEditorID = FIRSTRUNSCRIPTID;
				CurrentScriptFormID = 0;
				CurrentScriptBytecode = 0;
				CurrentScriptBytecodeLength = 0;
				NewScriptFlag = false;
				Closed = false;

				if (Bound)
					BoundParent->Enabled = false;

				return;
			}

			String^ ScriptText = gcnew String(Data->Text);
			UInt16 ScriptType = Data->Type;
			String^ ScriptName = gcnew String(Data->EditorID);
			UInt32 ByteCode = (UInt32)Data->ByteCode;
			UInt32 ByteCodeLength = Data->Length;
			UInt32 FormID = Data->FormID;

			if (ScriptName->Length == 0)
				ScriptName = NEWSCRIPTID;

			if (PartialUpdate == false)
			{
				CurrentScript = Data->ParentForm;
				NewScriptFlag = NewScript;

				TextEditor->InitializeState(ScriptText);

				if (Bound)
					BoundParent->Enabled = true;
			}
			else
				Debug::Assert(NewScript == false);

			CurrentScriptEditorID = ScriptName;
			CurrentScriptFormID = FormID;
			OnStateChangedDescription();
			CurrentScriptBytecode = ByteCode;
			CurrentScriptBytecodeLength = ByteCodeLength;
			OnStateChangedByteCodeSize(CurrentScriptBytecodeLength);

			SetType((IWorkspaceModel::ScriptType)ScriptType);
			TextEditor->Modified = false;

			if (PartialUpdate == false &&
				NewScriptFlag == false &&
				Data->Compiled == false &&
				Data->FormID >= 0x800) 			// skip default forms
			{
				if (Bound)
				{
					BoundParent->Controller->MessageBox("The current script has not been compiled. It cannot be executed in-game until it has been compiled at least once.",
														MessageBoxButtons::OK,
														MessageBoxIcon::Exclamation);
				}
			}

			if (preferences::SettingsHolder::Get()->Backup->UseAutoRecovery && PartialUpdate == false && Bound)
				CheckAutoRecovery();

			if (Bound)
				TextEditor->FocusTextArea();
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
				DialogResult Result = BoundParent->Controller->MessageBox("The current script '" + CurrentScriptEditorID + "' has unsaved changes.\n\nDo you wish to save them?",
													   MessageBoxButtons::YesNoCancel,
													   MessageBoxIcon::Exclamation);
				bool HasWarnings = false;
				if (Result == DialogResult::Yes)
					return SaveScript(IWorkspaceModel::SaveOperation::Default, HasWarnings);
				else if (Result == DialogResult::No)
				{
					if (NewScriptFlag)
					{
						nativeWrapper::g_CSEInterfaceTable->ScriptEditor.DestroyScriptInstance(CurrentScript);
						CurrentScript = 0;
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
			OnStateChangedType(CurrentScriptType);
			OnStateChangedByteCodeSize(CurrentScriptBytecodeLength);
			OnStateChangedDescription();
			OnStateChangedDirty(TextEditor->Modified);

			TextEditor->Bind(BoundParent->ListViewMessages,
							 BoundParent->ListViewBookmarks,
							 BoundParent->ListViewFindResults);

			BoundParent->BreadcrumbManager->Bind(TextEditor);
			IntelliSenseModel->Bind(BoundParent->IntelliSenseInterfaceView);

			TextEditor->FocusTextArea();
			TextEditor->ScrollToCaret();
		}

		void ConcreteWorkspaceModel::Unbind()
		{
			if (Bound)
			{
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
				Setup(Data, false, true);
				nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);

				TextEditor->Modified = true;
			}
		}

		void ConcreteWorkspaceModel::OpenScript(componentDLLInterface::ScriptData* Data)
		{
			Debug::Assert(Data != nullptr);

			if (PerformHouseKeeping())
				Setup(Data, false, false);

			nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
		}

		bool ConcreteWorkspaceModel::SaveScript(IWorkspaceModel::SaveOperation Operation, bool% HasWarnings)
		{
			bool Result = false;

			if (CurrentScript)
			{
				componentDLLInterface::ScriptCompileData* CompileData = nullptr;
				textEditors::CompilationData^ Data = TextEditor->BeginScriptCompilation();
				{
					if (Data->CanCompile)
					{
						if (Operation == IWorkspaceModel::SaveOperation::NoCompile)
							nativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(false);

						CompileData = nativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateCompileData();

						CString ScriptText(Data->PreprocessedScriptText->Replace("\n", "\r\n"));
						CompileData->Script.Text = ScriptText.c_str();
						CompileData->Script.Type = (int)Type;
						CompileData->Script.ParentForm = (TESForm*)CurrentScript;

						if (nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileScript(CompileData))
						{
							Setup(&CompileData->Script, true, false);

							String^ OriginalText = Data->UnpreprocessedScriptText + Data->SerializedMetadata;
							CString OrgScriptText(OriginalText);
							nativeWrapper::g_CSEInterfaceTable->ScriptEditor.SetScriptText(CurrentScript, OrgScriptText.c_str());
							Result = true;
						}
						else
							Data->CompileResult = CompileData;

						if (Operation == IWorkspaceModel::SaveOperation::NoCompile)
						{
							nativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(true);
							nativeWrapper::g_CSEInterfaceTable->ScriptEditor.RemoveScriptBytecode(CurrentScript);
						}
						else if (Operation == IWorkspaceModel::SaveOperation::SavePlugin)
							nativeWrapper::g_CSEInterfaceTable->EditorAPI.SaveActivePlugin();
					}
				}
				TextEditor->EndScriptCompilation(Data);
				nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(CompileData, false);

				HasWarnings = Data->HasWarnings;
			}

			if (Result)
				ClearAutoRecovery();

			return Result;
		}

		bool ConcreteWorkspaceModel::CloseScript(bool% OperationCancelled)
		{
			if (PerformHouseKeeping(OperationCancelled))
			{
				Closed = true;
				return true;
			}

			return false;
		}

		void ConcreteWorkspaceModel::NextScript()
		{
			if (PerformHouseKeeping())
			{
				componentDLLInterface::ScriptData* Data = nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetNextScriptInList(CurrentScript);
				if (Data)
					Setup(Data, false, false);
				nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
			}
		}

		void ConcreteWorkspaceModel::PreviousScript()
		{
			if (PerformHouseKeeping())
			{
				componentDLLInterface::ScriptData* Data = nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreviousScriptInList(CurrentScript);
				if (Data)
					Setup(Data, false, false);
				nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
			}
		}

		void ConcreteWorkspaceModel::SetType(IWorkspaceModel::ScriptType New)
		{
			CurrentScriptType = New;
			OnStateChangedType(New);
		}

		String^ GetSanitizedIdentifier(String^ Identifier)
		{
			return intellisense::IntelliSenseBackend::Get()->SanitizeIdentifier(Identifier);
		}

		bool ConcreteWorkspaceModel::Sanitize()
		{
			obScriptParsing::Sanitizer^ Agent = gcnew obScriptParsing::Sanitizer(TextEditor->GetText());
			obScriptParsing::Sanitizer::Operation Operation;

			if (preferences::SettingsHolder::Get()->Sanitizer->NormalizeIdentifiers)
				Operation = Operation | obScriptParsing::Sanitizer::Operation::AnnealCasing;

			if (preferences::SettingsHolder::Get()->Sanitizer->PrefixIfElseIfWithEval)
				Operation = Operation | obScriptParsing::Sanitizer::Operation::EvalifyIfs;

			if (preferences::SettingsHolder::Get()->Sanitizer->ApplyCompilerOverride)
				Operation = Operation | obScriptParsing::Sanitizer::Operation::CompilerOverrideBlocks;

			if (preferences::SettingsHolder::Get()->Sanitizer->IndentLines)
				Operation = Operation | obScriptParsing::Sanitizer::Operation::IndentLines;

			bool Result = Agent->SanitizeScriptText(Operation, gcnew obScriptParsing::Sanitizer::GetSanitizedIdentifier(GetSanitizedIdentifier));
			if (Result)
				TextEditor->SetText(Agent->Output, false);

			return Result;
		}

		void ConcreteWorkspaceModel::OnStateChangedDirty(bool Modified)
		{
			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->Dirty = Modified;
			StateChangedDirty(this, E);
		}

		void ConcreteWorkspaceModel::OnStateChangedByteCodeSize(UInt32 Size)
		{
			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->ByteCodeSize = Size;
			StateChangedByteCodeSize(this, E);
		}

		void ConcreteWorkspaceModel::OnStateChangedType(IWorkspaceModel::ScriptType Type)
		{
			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->Type = Type;
			StateChangedType(this, E);
		}

		void ConcreteWorkspaceModel::OnStateChangedDescription()
		{
			IWorkspaceModel::StateChangeEventArgs^ E = gcnew IWorkspaceModel::StateChangeEventArgs;
			E->ShortDescription = ShortDescription;
			E->LongDescription = LongDescription;
			StateChangedDescription(this, E);
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

		String^ ConcreteWorkspaceModelController::GetText(IWorkspaceModel^ Model, bool Preprocess, bool% PreprocessResult)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			if (Preprocess)
				return Concrete->TextEditor->GetPreprocessedText(PreprocessResult, false);
			else
				return Concrete->TextEditor->GetText();
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

			CString CEID(Concrete->CurrentScriptEditorID);
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

			nativeWrapper::g_CSEInterfaceTable->ScriptEditor.BindScript((CString(Concrete->CurrentScriptEditorID)).c_str(),
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

		textEditors::IScriptTextEditor::FindReplaceResult^ ConcreteWorkspaceModelController::FindReplace(IWorkspaceModel^ Model,
																						textEditors::IScriptTextEditor::FindReplaceOperation Operation,
																						String^ Query,
																						String^ Replacement,
																						textEditors::IScriptTextEditor::FindReplaceOptions Options)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->TextEditor->FindReplace(Operation, Query, Replacement, Options);
		}

		bool ConcreteWorkspaceModelController::GetOffsetViewerData(IWorkspaceModel^ Model, String^% OutText, UInt32% OutBytecode, UInt32% OutLength)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			if (Concrete->TextEditor->Modified)
				return false;

			bool PP = false;
			String^ Preprocessed = Concrete->TextEditor->GetPreprocessedText(PP, true);
			if (PP == false)
				return false;

			OutText = Preprocessed;
			OutBytecode = Concrete->CurrentScriptBytecode;
			OutLength = Concrete->CurrentScriptBytecodeLength;
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
																			  Concrete->CurrentScriptEditorID,
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
																			   Concrete->CurrentScriptEditorID,
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

							CString CEID(Concrete->CurrentScriptEditorID);
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
					refactoring::ModifyVariableIndicesDialog ModifyIndicesData(Concrete->BoundParent->WindowHandle, Concrete->CurrentScriptEditorID);

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