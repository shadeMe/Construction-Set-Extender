#include "WorkspaceModel.h"
#include "[Common]\CustomInputBox.h"
#include "ScriptEditorPreferences.h"

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
			CurrentScriptType = Type::Object;
			CurrentScriptEditorID = NEWSCRIPTID;
			CurrentScriptFormID = 0;
			NewScriptFlag = false;
			Closed = false;
			BoundParent = nullptr;

			AutoSaveTimer = gcnew Timer();
			AutoSaveTimer->Interval = PREFERENCES->FetchSettingAsInt("AutoRecoverySavePeriod", "Backup") * 1000 * 60;

			Color ForeColor = Color::Black;
			Color BackColor = Color::White;
			Color HighlightColor = Color::Maroon;
			Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"), PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"), (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));
			int TabSize = Decimal::ToInt32(PREFERENCES->FetchSettingAsInt("TabSize", "Appearance"));

			TODO("move context menu, bookmarks, cse block serialization, code validation and preprocessing to AvalonEditTextEditor");
			TODO("handle teh relevant shortcuts there");
			TextEditor = gcnew TextEditors::AvalonEditor::AvalonEditTextEditor(CustomFont);
			if (TabSize)
				TextEditor->SetTabCharacterSize(TabSize);

			TextEditorKeyDownHandler = gcnew KeyEventHandler(this, &ConcreteWorkspaceModel::TextEditor_KeyDown);
			TextEditorScriptModifiedHandler = gcnew TextEditors::TextEditorScriptModifiedEventHandler(this, &ConcreteWorkspaceModel::TextEditor_ScriptModified);
			TextEditorMouseClickHandler = gcnew TextEditors::TextEditorMouseClickEventHandler(this, &ConcreteWorkspaceModel::TextEditor_MouseClick);

			ToolBarNewScriptClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarNewScript_Click);
			ToolBarOpenScriptClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarOpenScript_Click);
			ToolBarPreviousScriptClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarPreviousScript_Click);
			ToolBarNextScriptClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarNextScript_Click);
			ToolBarSaveScriptClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarSaveScript_Click);
			ToolBarSaveScriptNoCompileClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarSaveScriptNoCompile_Click);
			ToolBarSaveScriptAndPluginClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarSaveScriptAndPlugin_Click);
			ToolBarRecompileScriptsClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarRecompileScripts_Click);
			ToolBarCompileDependenciesClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarCompileDependencies_Click);
			ToolBarDeleteScriptClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarDeleteScript_Click);

			ToolBarScriptTypeContentsObjectClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarScriptTypeContentsObject_Click);
			ToolBarScriptTypeContentsQuestClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarScriptTypeContentsQuest_Click);
			ToolBarScriptTypeContentsMagicEffectClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarScriptTypeContentsMagicEffect_Click);

			ToolBarEditMenuContentsFindReplaceClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarEditMenuContentsFindReplace_Click);
			ToolBarEditMenuContentsGotoLineClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarEditMenuContentsGotoLine_Click);
			ToolBarEditMenuContentsGotoOffsetClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarEditMenuContentsGotoOffset_Click);

			ToolBarDumpScriptClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarDumpScript_Click);
			ToolBarLoadScriptClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarLoadScript_Click);
			ToolBarShowOffsetsClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarShowOffsets_Click);
			ToolBarShowPreprocessedTextClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarShowPreprocessedText_Click);
			ToolBarSanitizeScriptTextClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarSanitizeScriptText_Click);
			ToolBarBindScriptClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarBindScript_Click);
			ToolBarSnippetManagerClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ToolBarSnippetManager_Click);

			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::ScriptEditorPreferences_Saved);
			AutoSaveTimerTickHandler = gcnew EventHandler(this, &ConcreteWorkspaceModel::AutoSaveTimer_Tick);

			TextEditor->KeyDown += TextEditorKeyDownHandler;
			TextEditor->ScriptModified += TextEditorScriptModifiedHandler;
			TextEditor->MouseClick += TextEditorMouseClickHandler;

			PREFERENCES->PreferencesSaved += ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick += AutoSaveTimerTickHandler;

			AutoSaveTimer->Start();

			if (Data && Data->ParentForm)
				Setup(Data);

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
			switch (E->KeyCode)
			{
			case Keys::N:
				if (E->Modifiers == Keys::Control)
					NewScript();

				break;
			case Keys::O:
				if (E->Modifiers == Keys::Control)
					OpenScript();

				break;
			case Keys::S:
				if (E->Modifiers == Keys::Control)
					SaveScript(SaveOperation::Default);

				break;
			case Keys::D:
				if (E->Modifiers == Keys::Control)
					DeleteScript();

				break;
			case Keys::Left:
				if (E->Control && E->Alt)
				{
					PreviousScript();
					TextEditor->FocusTextArea();
				}

				break;
			case Keys::Right:
				if (E->Control && E->Alt)
				{
					NextScript();
					TextEditor->FocusTextArea();
				}

				break;
			case Keys::F:
			case Keys::H:
				if (E->Modifiers == Keys::Control)
					ShowFindReplace();

				break;
			case Keys::G:
				if (E->Modifiers == Keys::Control)
					GotoLine();

				break;
			case Keys::E:
				if (E->Modifiers == Keys::Control)
					GotoOffset();

				break;
			}

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

		void ConcreteWorkspaceModel::ToolBarNewScript_Click(Object^ Sender, EventArgs^ E)
		{
			NewScript();
		}

		void ConcreteWorkspaceModel::ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E)
		{
			OpenScript();
		}

		void ConcreteWorkspaceModel::ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E)
		{
			PreviousScript();
		}

		void ConcreteWorkspaceModel::ToolBarNextScript_Click(Object^ Sender, EventArgs^ E)
		{
			NextScript();
		}

		void ConcreteWorkspaceModel::ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E)
		{
			SaveScript(SaveOperation::Default);
		}

		void ConcreteWorkspaceModel::ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E)
		{
			if (New == true || Initialized == false)
			{
				MessageBox::Show("You may only perform this operation on an existing script.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);

				return;
			}

			SaveScript(SaveOperation::NoCompile);
		}

		void ConcreteWorkspaceModel::ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E)
		{
			SaveScript(SaveOperation::SavePlugin);
		}

		void ConcreteWorkspaceModel::ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E)
		{
			RecompileScripts();
		}

		void ConcreteWorkspaceModel::ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E)
		{
			if (New == false && Initialized == true)
			{
				CString CEID(CurrentScriptEditorID);
				NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileDependencies(CEID.c_str());
				MessageBox::Show("Operation complete!\n\nScript variables used as condition parameters will need to be corrected manually. The results have been logged to the console.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Information);
			}
			else
			{
				MessageBox::Show("The current script needs to be compiled before its dependencies can be updated.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);
			}
		}

		void ConcreteWorkspaceModel::ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E)
		{
			DeleteScript();
		}

		void ConcreteWorkspaceModel::ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E)
		{
			SetType(Type::Object, true);
		}

		void ConcreteWorkspaceModel::ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E)
		{
			SetType(Type::Quest, true);
		}

		void ConcreteWorkspaceModel::ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E)
		{
			SetType(Type::MagicEffect, true);
		}

		void ConcreteWorkspaceModel::ToolBarEditMenuContentsFindReplace_Click(Object^ Sender, EventArgs^ E)
		{
			ShowFindReplace();
		}

		void ConcreteWorkspaceModel::ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E)
		{
			GotoLine();
		}

		void ConcreteWorkspaceModel::ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E)
		{
			GotoOffset();
		}

		void ConcreteWorkspaceModel::ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E)
		{
			SaveFileDialog^ SaveManager = gcnew SaveFileDialog();

			SaveManager->DefaultExt = "*.txt";
			SaveManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
			SaveManager->FileName = Description;
			SaveManager->RestoreDirectory = true;

			if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->FileName->Length > 0)
				SaveToDisk(SaveManager->FileName, true, gcnew String("txt"));
		}

		void ConcreteWorkspaceModel::ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E)
		{
			OpenFileDialog^ LoadManager = gcnew OpenFileDialog();

			LoadManager->DefaultExt = "*.txt";
			LoadManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
			LoadManager->RestoreDirectory = true;

			if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileName->Length > 0)
				LoadFromDisk(LoadManager->FileName);
		}

		void ConcreteWorkspaceModel::ToolBarShowOffsets_Click(Object^ Sender, EventArgs^ E)
		{
			if (BounToolBarShowOffsets->Checked)
			{
				int Caret = OffsetViewer->Hide();
				TextEditor->FocusTextArea();
				TextEditor->SetCaretPos(Caret);
				ToolBarShowOffsets->Checked = false;
			}
			else
			{
				if (OffsetViewer->Show((TextEditor->GetCaretPos() != -1 ? TextEditor->GetCaretPos() : 0)))
					ToolBarShowOffsets->Checked = true;
			}
		}

		void ConcreteWorkspaceModel::ToolBarShowPreprocessedText_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarSanitizeScriptText_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarBindScript_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarSnippetManager_Click(Object^ Sender, EventArgs^ E)
		{
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