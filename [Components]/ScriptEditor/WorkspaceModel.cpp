#include "WorkspaceModel.h"
#include "[Common]\CustomInputBox.h"
#include "ScriptEditorPreferences.h"
#include "IntelliSense\IntelliSenseDatabase.h"

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
			CurrentScriptEditorID = NEWSCRIPTID;
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

			TODO("move context menu, bookmarks, cse block serialization, code validation and preprocessing to AvalonEditTextEditor");
			TODO("handle teh relevant shortcuts there");
			TextEditor = gcnew TextEditors::AvalonEditor::AvalonEditTextEditor(CustomFont);
			if (TabSize)
				TextEditor->SetTabCharacterSize(TabSize);

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

			ModelController = nullptr;
			ModelFactory = nullptr;

			TextEditor->KeyDown -= TextEditorKeyDownHandler;
			TextEditor->ScriptModified -= TextEditorScriptModifiedHandler;
			TextEditor->MouseClick -= TextEditorMouseClickHandler;

			PREFERENCES->PreferencesSaved -= ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick -= AutoSaveTimerTickHandler;

			delete TextEditor;
			delete AutoSaveTimer;
			TextEditor = nullptr;

			ModelFactory->Remove(this);
		}

		void ConcreteWorkspaceModel::TextEditor_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			BoundParent->Controller->BubbleKeyDownEvent(BoundParent, E);
		}

		void ConcreteWorkspaceModel::TextEditor_ScriptModified(Object^ Sender, TextEditors::TextEditorScriptModifiedEventArgs^ E)
		{
			if (Bound)
				BoundParent->Controller->SetModifiedIndicator(BoundParent, this, E->ModifiedStatus);
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
					BoundParent->Controller->Jump(BoundParent, gcnew String(Data->EditorID));
				}

				NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
			}
		}

		void ConcreteWorkspaceModel::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
		{
			AutoSaveTimer->Stop();

			Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"), PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"), (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));
			TextEditor->SetFont(CustomFont);

			int TabSize = Decimal::ToInt32(PREFERENCES->FetchSettingAsInt("TabSize", "Appearance"));
			if (TabSize == 0)
				TabSize = 4;

			TextEditor->SetTabCharacterSize(TabSize);

			AutoSaveTimer->Interval = PREFERENCES->FetchSettingAsInt("AutoRecoverySavePeriod", "Backup") * 1000 * 60;
			AutoSaveTimer->Start();
		}

		void ConcreteWorkspaceModel::AutoSaveTimer_Tick(Object^ Sender, EventArgs^ E)
		{
			if (PREFERENCES->FetchSettingAsInt("UseAutoRecovery", "Backup"))
			{
				if (Initialized == true && New == false && TextEditor->GetModifiedStatus() == true)
				{
					TextEditor->SaveScriptToDisk(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()),
												 false, Description, "txt");
				}
			}
		}

		void ConcreteWorkspaceModel::ClearAutoRecovery()
		{
			try {
				System::IO::File::Delete(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()) + Description + ".txt");
			}
			catch (...) {}
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

				if (ScriptName != NEWSCRIPTID)
					TextEditor->SetInitializingStatus(true);

				TODO("clear editor's tracked data, i.e, bookmarks, etc")
				TextEditor->SetText(ScriptText, false, true);

				if (Bound)
					BoundParent->Enabled = true;
			}

			CurrentScriptEditorID = ScriptName;
			CurrentScriptFormID = FormID;
			CurrentScriptBytecode = ByteCode;
			CurrentScriptBytecodeLength = ByteCodeLength;

			if (Bound)
				BoundParent->Description = Description;

			SetType((IWorkspaceModel::ScriptType)ScriptType, Bound);
			TextEditor->SetModifiedStatus(false);

			if (Bound)
				BoundParent->Controller->SetByteCodeSize(BoundParent, ByteCodeLength);

			TODO("modify TextEditor::GetText to take another argument to preprocess")
			TextEditor->UpdateIntelliSenseLocalDatabase();

			if (PartialUpdate == false && Data->Compiled == false && PREFERENCES->FetchSettingAsInt("WarnUncompiledScripts", "General"))
			{
				MessageBox::Show("The current script has not been compiled. It cannot be executed in-game until it has been compiled at least once.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);
			}

			if (PREFERENCES->FetchSettingAsInt("UseAutoRecovery", "Backup") && PartialUpdate == false)
			{
				String^ CachePath = gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()) + Description + ".txt";
				if (System::IO::File::Exists(CachePath))
				{
					try
					{
						System::DateTime LastWriteTime = System::IO::File::GetLastWriteTime(CachePath);
						if (MessageBox::Show("An auto-recovery cache for the script '" + Description + "' was found, dated " + LastWriteTime.ToShortDateString() + " " + LastWriteTime.ToLongTimeString() + ".\n\nWould you like to load it instead?",
							SCRIPTEDITOR_TITLE,
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
						DebugPrint("Couldn't access auto-recovery cache '" + Description + "'!\n\tException: " + E->Message, true);
					}
				}
			}
		}

		bool ConcreteWorkspaceModel::DoHouseKeeping()
		{
			if (TextEditor->GetModifiedStatus())
			{
				DialogResult Result = MessageBox::Show("The current script '" + CurrentScriptEditorID + "' has unsaved changes.\n\nDo you wish to save them?",
													   SCRIPTEDITOR_TITLE,
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
			TODO("bind the listview to the text editor here");
			BoundParent->Controller->AttachModelInternalView(BoundParent, this);
		}

		void ConcreteWorkspaceModel::Unbind()
		{
			if (Bound)
			{
				BoundParent = nullptr;
				TODO("unbind the listview");
				BoundParent->Controller->DettachModelInternalView(BoundParent, this);
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
				TextEditor->SetModifiedStatus(true);
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
			TODO("implement TextEditor::SerializeMetaData and TextEditor::CanCompile - preprocesses the text and runs the validator");

			if (TextEditor->CanCompile())
			{
				bool Throwaway = false;
				Preprocessed = TextEditor->GetText(true, Throwaway);
				Unpreprocessed = TextEditor->GetText(false, Throwaway);

				if (CurrentScript)
				{
					if (Operation == IWorkspaceModel::SaveOperation::NoCompile)
					{
						NativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(false);
					}

					ComponentDLLInterface::ScriptCompileData* CompileData = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateCompileData();

					CString ScriptText(Preprocessed->Replace("\n", "\r\n"));
					CompileData->Script.Text = ScriptText.c_str();
					CompileData->Script.Type = (int)Type;
					CompileData->Script.ParentForm = (TESForm*)CurrentScript;

					if (NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileScript(CompileData))
					{
						Setup(&CompileData->Script, true);

						String^ OriginalText = Unpreprocessed + TextEditor->SerializeMetaData();
						CString OrgScriptText(OriginalText);
						NativeWrapper::g_CSEInterfaceTable->ScriptEditor.SetScriptText(CurrentScript, OrgScriptText.c_str());
						Result = true;
					}
					else
					{
						for (int i = 0; i < CompileData->CompileErrorData.Count; i++)
							TODO("track error message");
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
				else
					Result = true;
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

		void ConcreteWorkspaceModel::SetType(IWorkspaceModel::ScriptType New, bool UpdateView)
		{
			Type = New;
			if (UpdateView)
			{
				Debug::Assert(Bound == true);
				BoundParent->Controller->UpdateType(BoundParent, this);
			}
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

			return Concrete->TextEditor->GetText(Preprocess, PreprocessResult);
		}

		int ConcreteWorkspaceModelController::GetCaret(IWorkspaceModel^ Model)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			return Concrete->TextEditor->GetCaretPos();
		}

		void ConcreteWorkspaceModelController::SetCaret(IWorkspaceModel^ Model, int Index)
		{
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceModel^ Concrete = (ConcreteWorkspaceModel^)Model;

			Concrete->TextEditor->SetCaretPos(Index);
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

			Concrete->SetType(New, false);
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

			return Concrete->TextEditor->GetTotalLineCount();
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

			Concrete->TextEditor->SaveScriptToDisk(PathToFile, PathIncludesFileName, Concrete->Description, Extension);
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

			if (Concrete->TextEditor->GetModifiedStatus())
				return false;

			bool PP = false;
			String^ Preprocessed = Concrete->TextEditor->GetText(true, PP);
			if (PP == false)
				return false;

			OutText = Preprocessed;
			OutBytecode = Concrete->CurrentScriptBytecode;
			OutLength = Concrete->CurrentScriptBytecodeLength;
			return true;
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