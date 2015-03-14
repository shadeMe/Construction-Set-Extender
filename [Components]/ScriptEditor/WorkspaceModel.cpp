#include "WorkspaceModel.h"
#include "[Common]\CustomInputBox.h"
#include "ScriptEditorPreferences.h"
#include "IntelliSense\IntelliSenseDatabase.h"
#include "RefactorTools.h"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ConcreteWorkspaceModel::ConcreteWorkspaceModel(ConcreteWorkspaceModelController^ Controller, ConcreteWorkspaceModelFactory^ Factory,
													   ComponentDLLInterface::ScriptData* Data)
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
			AutoSaveTimer->Interval = PREFERENCES->FetchSettingAsInt("AutoRecoverySavePeriod", "Backup") * 1000 * 60;

			Color ForeColor = Color::Black;
			Color BackColor = Color::White;
			Color HighlightColor = Color::Maroon;
			Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"),
										  PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"),
										  (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));
			int TabSize = Decimal::ToInt32(PREFERENCES->FetchSettingAsInt("TabSize", "Appearance"));

			TextEditor = gcnew TextEditors::AvalonEditor::AvalonEditTextEditor(this,
																			   gcnew TextEditors::AvalonEditor::JumpToScriptHandler(this, &ConcreteWorkspaceModel::JumpToScript),
																			   CustomFont, TabSize);

			TextEditorKeyDownHandler = gcnew KeyEventHandler(this, &ConcreteWorkspaceModel::TextEditor_KeyDown);
			TextEditorScriptModifiedHandler = gcnew TextEditors::TextEditorScriptModifiedEventHandler(this, &ConcreteWorkspaceModel::TextEditor_ScriptModified);
			TextEditorMouseClickHandler = gcnew TextEditors::TextEditorMouseClickEventHandler(this, &ConcreteWorkspaceModel::TextEditor_MouseClick);

			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ScriptEditorPreferences_Saved);
			AutoSaveTimerTickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::AutoSaveTimer_Tick);

			TextEditor->KeyDown += TextEditorKeyDownHandler;
			TextEditor->ScriptModified += TextEditorScriptModifiedHandler;
			TextEditor->MouseClick += TextEditorMouseClickHandler;

			PREFERENCES->PreferencesSaved += ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick += AutoSaveTimerTickHandler;

			AutoSaveTimer->Start();

			if (Data && Data->ParentForm)
				Setup(Data, false);

			NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
		}

		ConcreteWorkspaceModel::~ConcreteWorkspaceModel()
		{
			Unbind();

			AutoSaveTimer->Stop();

			TextEditor->KeyDown -= TextEditorKeyDownHandler;
			TextEditor->ScriptModified -= TextEditorScriptModifiedHandler;
			TextEditor->MouseClick -= TextEditorMouseClickHandler;

			PREFERENCES->PreferencesSaved -= ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick -= AutoSaveTimerTickHandler;

			delete TextEditor;
			delete AutoSaveTimer;
			TextEditor = nullptr;

			ModelFactory->Remove(this);

			ModelController = nullptr;
			ModelFactory = nullptr;
		}

		void ConcreteWorkspaceModel::TextEditor_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			BoundParent->Controller->BubbleKeyDownEvent(BoundParent, E);
		}

		void ConcreteWorkspaceModel::TextEditor_ScriptModified(Object^ Sender, TextEditors::TextEditorScriptModifiedEventArgs^ E)
		{
			OnStateChangedDirty(E->ModifiedStatus);
		}

		void ConcreteWorkspaceModel::TextEditor_MouseClick(Object^ Sender, TextEditors::TextEditorMouseClickEventArgs^ E)
		{
			if (Control::ModifierKeys == Keys::Control && E->Button == MouseButtons::Left)
			{
				String^ Token = TextEditor->GetTokenAtCharIndex(E->ScriptTextOffset);

				CString EID(Token);
				ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str());

				if (Data && Data->IsValid())
				{
					BoundParent->Controller->Jump(BoundParent, this, gcnew String(Data->EditorID));
				}

				NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
			}
		}

		void ConcreteWorkspaceModel::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
		{
			AutoSaveTimer->Stop();
			AutoSaveTimer->Interval = PREFERENCES->FetchSettingAsInt("AutoRecoverySavePeriod", "Backup") * 1000 * 60;
			AutoSaveTimer->Start();
		}

		void ConcreteWorkspaceModel::AutoSaveTimer_Tick(Object^ Sender, EventArgs^ E)
		{
			if (PREFERENCES->FetchSettingAsInt("UseAutoRecovery", "Backup"))
			{
				if (Initialized == true && New == false && TextEditor->Modified == true)
				{
					TextEditor->SaveScriptToDisk(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()),
												 false, LongDescription, "txt");
				}
			}
		}

		void ConcreteWorkspaceModel::CheckAutoRecovery()
		{
			String^ CachePath = gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()) + LongDescription + ".txt";
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
				System::IO::File::Delete(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()) + LongDescription + ".txt");
			}
			catch (...) {}
		}

		void ConcreteWorkspaceModel::JumpToScript(String^ TargetEditorID)
		{
			if (Bound)
				BoundParent->Controller->Jump(BoundParent, this, TargetEditorID);
		}

		void ConcreteWorkspaceModel::Setup(ComponentDLLInterface::ScriptData* Data, bool PartialUpdate)
		{
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
				NewScriptFlag = false;

				TextEditor->ClearTrackedData(true, true, true, true, true);
				TextEditor->DeserializeMetadata(ScriptText, true);

				if (Bound)
					BoundParent->Enabled = true;
			}

			CurrentScriptEditorID = ScriptName;
			CurrentScriptFormID = FormID;
			OnStateChangedDescription();
			CurrentScriptBytecode = ByteCode;
			CurrentScriptBytecodeLength = ByteCodeLength;
			OnStateChangedByteCodeSize(CurrentScriptBytecodeLength);

			SetType((IWorkspaceModel::ScriptType)ScriptType);
			TextEditor->Modified = false;

			if (PartialUpdate == false && Data->Compiled == false && PREFERENCES->FetchSettingAsInt("WarnUncompiledScripts", "General"))
			{
				if (Bound)
				{
					BoundParent->Controller->MessageBox("The current script has not been compiled. It cannot be executed in-game until it has been compiled at least once.",
														MessageBoxButtons::OK,
														MessageBoxIcon::Exclamation);
				}
			}

			if (PREFERENCES->FetchSettingAsInt("UseAutoRecovery", "Backup") && PartialUpdate == false && Bound)
				CheckAutoRecovery();
		}

		bool ConcreteWorkspaceModel::DoHouseKeeping()
		{
			if (TextEditor->Modified)
			{
				Debug::Assert(Bound == true);
				DialogResult Result = BoundParent->Controller->MessageBox("The current script '" + CurrentScriptEditorID + "' has unsaved changes.\n\nDo you wish to save them?",
													   MessageBoxButtons::YesNoCancel,
													   MessageBoxIcon::Exclamation);

				if (Result == DialogResult::Yes)
					return SaveScript(IWorkspaceModel::SaveOperation::Default);
				else if (Result == DialogResult::No)
				{
					if (NewScriptFlag)
					{
						NativeWrapper::g_CSEInterfaceTable->ScriptEditor.DestroyScriptInstance(CurrentScript);
						CurrentScript = 0;
					}

					ClearAutoRecovery();
					return true;
				}
				else
					return false;
			}

			return true;
		}

		void ConcreteWorkspaceModel::Bind(IWorkspaceView^ To)
		{
			Debug::Assert(Bound == false);

			BoundParent = To;
			BoundParent->Controller->AttachModelInternalView(BoundParent, this);
			SetType(CurrentScriptType);
			CheckAutoRecovery();

			TextEditor->Bind(BoundParent->ListViewMessages, BoundParent->ListViewBookmarks, BoundParent->ListViewFindResults);
		}

		void ConcreteWorkspaceModel::Unbind()
		{
			if (Bound)
			{
				TextEditor->Unbind();

				BoundParent->Controller->DettachModelInternalView(BoundParent, this);
				BoundParent = nullptr;
			}
		}

		void ConcreteWorkspaceModel::NewScript()
		{
			if (DoHouseKeeping())
			{
				ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CreateNewScript();
				Setup(Data, false);
				NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);

				NewScriptFlag = true;
				TextEditor->Modified = true;
			}
		}

		void ConcreteWorkspaceModel::OpenScript(ComponentDLLInterface::ScriptData* Data)
		{
			Debug::Assert(Data != nullptr);

			if (DoHouseKeeping())
				Setup(Data, false);

			NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
		}

		bool ConcreteWorkspaceModel::SaveScript(IWorkspaceModel::SaveOperation Operation)
		{
			bool Result = false;
			String^ Preprocessed = "";
			String^ Unpreprocessed = "";
			bool HasDirectives = false;

			if (CurrentScript && TextEditor->CanCompile(HasDirectives))
			{
				bool Throwaway = false;
				Preprocessed = TextEditor->GetPreprocessedText(Throwaway, true);
				Unpreprocessed = TextEditor->GetText();

				if (Operation == IWorkspaceModel::SaveOperation::NoCompile)
				{
					NativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(false);
				}

				ComponentDLLInterface::ScriptCompileData* CompileData = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateCompileData();

				CString ScriptText(Preprocessed->Replace("\n", "\r\n"));
				CompileData->Script.Text = ScriptText.c_str();
				CompileData->Script.Type = (int)Type;
				CompileData->Script.ParentForm = (TESForm*)CurrentScript;

				TextEditor->ClearTrackedData(true, false, false, false, false);
				if (NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileScript(CompileData))
				{
					Setup(&CompileData->Script, true);

					String^ OriginalText = Unpreprocessed + TextEditor->SerializeMetadata(HasDirectives);
					CString OrgScriptText(OriginalText);
					NativeWrapper::g_CSEInterfaceTable->ScriptEditor.SetScriptText(CurrentScript, OrgScriptText.c_str());
					Result = true;
				}
				else
				{
					for (int i = 0; i < CompileData->CompileErrorData.Count; i++)
					{
						TextEditor->TrackCompilerError(CompileData->CompileErrorData.ErrorListHead[i].Line,
														gcnew String(CompileData->CompileErrorData.ErrorListHead[i].Message));
					}
				}

				if (Operation == IWorkspaceModel::SaveOperation::NoCompile)
				{
					NativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(true);
					NativeWrapper::g_CSEInterfaceTable->ScriptEditor.RemoveScriptBytecode(CurrentScript);
				}
				else if (Operation == IWorkspaceModel::SaveOperation::SavePlugin)
					NativeWrapper::g_CSEInterfaceTable->EditorAPI.SaveActivePlugin();

				NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(CompileData, false);
			}

			if (Result)
				ClearAutoRecovery();

			return Result;
		}

		bool ConcreteWorkspaceModel::CloseScript()
		{
			if (DoHouseKeeping())
			{
				Closed = true;
				return true;
			}

			return false;
		}

		void ConcreteWorkspaceModel::NextScript()
		{
			if (DoHouseKeeping())
			{
				ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetNextScriptInList(CurrentScript);
				if (Data)
					Setup(Data, false);

				NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
			}
		}

		void ConcreteWorkspaceModel::PreviousScript()
		{
			if (DoHouseKeeping())
			{
				ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreviousScriptInList(CurrentScript);
				if (Data)
					Setup(Data, false);

				NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
			}
		}

		void ConcreteWorkspaceModel::SetType(IWorkspaceModel::ScriptType New)
		{
			CurrentScriptType = New;
			OnStateChangedType(New);
		}

		String^ GetSanitizedIdentifier(String^ Identifier)
		{
			return IntelliSense::ISDB->SanitizeIdentifier(Identifier);
		}

		bool ConcreteWorkspaceModel::Sanitize()
		{
			ObScriptSemanticAnalysis::Sanitizer^ Agent = gcnew ObScriptSemanticAnalysis::Sanitizer(TextEditor->GetText());
			ObScriptSemanticAnalysis::Sanitizer::Operation Operation;

			if (PREFERENCES->FetchSettingAsInt("AnnealCasing", "Sanitize"))
				Operation = Operation | ObScriptSemanticAnalysis::Sanitizer::Operation::AnnealCasing;

			if (PREFERENCES->FetchSettingAsInt("EvalifyIfs", "Sanitize"))
				Operation = Operation | ObScriptSemanticAnalysis::Sanitizer::Operation::EvalifyIfs;

			if (PREFERENCES->FetchSettingAsInt("CompilerOverrideBlocks", "Sanitize"))
				Operation = Operation | ObScriptSemanticAnalysis::Sanitizer::Operation::CompilerOverrideBlocks;

			if (PREFERENCES->FetchSettingAsInt("IndentLines", "Sanitize"))
				Operation = Operation | ObScriptSemanticAnalysis::Sanitizer::Operation::IndentLines;

			bool Result = Agent->SanitizeScriptText(Operation, gcnew ObScriptSemanticAnalysis::Sanitizer::GetSanitizedIdentifier(GetSanitizedIdentifier));
			if (Result)
				TextEditor->SetText(Agent->Output, false, false);

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

			Concrete->TextEditor->SetText(Text, true, ResetUndoStack);
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

		void ConcreteWorkspaceModelController::Open(IWorkspaceModel^ Model, ComponentDLLInterface::ScriptData* Data)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->OpenScript(Data);
		}

		bool ConcreteWorkspaceModelController::Save(IWorkspaceModel^ Model, IWorkspaceModel::SaveOperation Operation)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->SaveScript(Operation);
		}

		bool ConcreteWorkspaceModelController::Close(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->CloseScript();
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
			NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileDependencies(CEID.c_str());
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

			NativeWrapper::g_CSEInterfaceTable->ScriptEditor.BindScript((CString(Concrete->CurrentScriptEditorID)).c_str(),
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

		int ConcreteWorkspaceModelController::FindReplace(IWorkspaceModel^ Model, TextEditors::IScriptTextEditor::FindReplaceOperation Operation, String^ Query, String^ Replacement, UInt32 Options)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			int Hits = Concrete->TextEditor->FindReplace(Operation, Query, Replacement, Options);
			return Hits;
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
			case ConstructionSetExtender::ScriptEditor::IWorkspaceModel::RefactorOperation::DocumentScript:
				{
					Refactoring::EditScriptComponentDialog DocumentScriptData(Concrete->BoundParent->WindowHandle,
																			  Concrete->CurrentScriptEditorID,
																			  Refactoring::EditScriptComponentDialog::OperationType::DocumentScript,
																			  "Script Description");

					if (DocumentScriptData.HasResult)
					{
						String^ Description = "";
						DocumentScriptData.ResultData->LookupEditDataByName("Script Description", Description);
						ObScriptSemanticAnalysis::Documenter^ Agent = gcnew ObScriptSemanticAnalysis::Documenter(Concrete->TextEditor->GetText());
						Agent->Document(Description, DocumentScriptData.ResultData->AsTable());

						Concrete->TextEditor->SetText(Agent->Output, false, false);
						Result = true;
					}
				}

				break;
			case ConstructionSetExtender::ScriptEditor::IWorkspaceModel::RefactorOperation::RenameVariables:
				{
					Refactoring::EditScriptComponentDialog RenameVariablesData(Concrete->BoundParent->WindowHandle,
																			   Concrete->CurrentScriptEditorID,
																			   Refactoring::EditScriptComponentDialog::OperationType::RenameVariables,
																			   "");

					if (RenameVariablesData.HasResult)
					{
						List<CString^>^ StringAllocations = gcnew List<CString^>();
						List<Refactoring::EditScriptComponentData::ScriptComponent^>^ RenameEntries = gcnew List<Refactoring::EditScriptComponentData::ScriptComponent^>();

						for each (Refactoring::EditScriptComponentData::ScriptComponent^ Itr in RenameVariablesData.ResultData->ScriptComponentList)
						{
							if (Itr->EditData != "")
								RenameEntries->Add(Itr);
						}

						if (RenameEntries->Count)
						{
							ComponentDLLInterface::ScriptVarRenameData* RenameData = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateVarRenameData(RenameEntries->Count);

							for (int i = 0; i < RenameData->ScriptVarListCount; i++)
							{
								ComponentDLLInterface::ScriptVarRenameData::ScriptVarInfo* Data = &RenameData->ScriptVarListHead[i];
								CString^ OldID = gcnew CString(RenameEntries[i]->ElementName);
								CString^ NewID = gcnew CString(RenameEntries[i]->EditData);

								Data->OldName = OldID->c_str();
								Data->NewName = NewID->c_str();

								StringAllocations->Add(OldID);
								StringAllocations->Add(NewID);
							}

							CString CEID(Concrete->CurrentScriptEditorID);
							NativeWrapper::g_CSEInterfaceTable->ScriptEditor.UpdateScriptVarNames(CEID.c_str(), RenameData);

							for each (CString^ Itr in StringAllocations)
								delete Itr;

							NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(RenameData, false);

							Result = true;
						}

						StringAllocations->Clear();
						RenameEntries->Clear();
					}
				}

				break;
			case ConstructionSetExtender::ScriptEditor::IWorkspaceModel::RefactorOperation::ModifyVariableIndices:
				{
					Refactoring::ModifyVariableIndicesDialog ModifyIndicesData(Concrete->BoundParent->WindowHandle, Concrete->CurrentScriptEditorID);

					if (ModifyIndicesData.IndicesUpdated)
					{
						Concrete->TextEditor->Modified = true;
						Result = true;
					}
				}

				break;
			case ConstructionSetExtender::ScriptEditor::IWorkspaceModel::RefactorOperation::CreateUDF:
				{
					String^ UDFName = (String^)Arg;

					Refactoring::CreateUDFImplementationDialog UDFData(Concrete->BoundParent->WindowHandle);

					if (UDFData.HasResult)
					{
						String^ UDFScriptText = "scn " + UDFName + "\n\n";
						String^ ParamList = "{ ";
						for each (Refactoring::CreateUDFImplementationData::ParameterData^ Itr in UDFData.ResultData->ParameterList)
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

		IWorkspaceModel^ ConcreteWorkspaceModelFactory::CreateModel(ComponentDLLInterface::ScriptData* Data)
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