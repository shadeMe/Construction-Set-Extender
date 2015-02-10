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

			TODO("move context menu to texteditor");
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
			case Keys::O:									// Open script
				if (E->Modifiers == Keys::Control)
					ToolBarOpenScript->PerformClick();

				break;
			case Keys::S:									// Save script
				if (E->Modifiers == Keys::Control)
					SaveScript(ScriptSaveOperation::SaveAndCompile);

				break;
			case Keys::D:									// Delete script
				if (E->Modifiers == Keys::Control)
					ToolBarDeleteScript->PerformClick();

				break;
			case Keys::Left:								// Previous script
				if (E->Control && E->Alt)
				{
					ToolBarPreviousScript->PerformClick();
					TextEditor->FocusTextArea();
				}

				break;
			case Keys::Right:								// Next script
				if (E->Control && E->Alt)
				{
					ToolBarNextScript->PerformClick();
					TextEditor->FocusTextArea();
				}

				break;
			case Keys::N:									// New script
				if (E->Modifiers == Keys::Control)
					ToolBarNewScript->PerformClick();

				break;
			case Keys::B:									// Toggle bookmark
				if (E->Modifiers == Keys::Control)
					ContextMenuToggleBookmark->PerformClick();

				break;
			case Keys::F:									// Find
			case Keys::H:									// Replace
				if (E->Modifiers == Keys::Control)
					ContextMenuFind->PerformClick();

				break;
			case Keys::G:									// Goto Line
				if (E->Modifiers == Keys::Control)
					ToolBarEditMenuContentsGotoLine->PerformClick();

				break;
			case Keys::E:									// Goto Offset
				if (E->Modifiers == Keys::Control)
					ToolBarEditMenuContentsGotoOffset->PerformClick();

				break;
			}

			BoundParent->Controller->BubbleKeyDownEvent(BoundParent, E);
			HandlingKeyDownEvent = false;
		}

		void ConcreteWorkspaceModel::TextEditor_ScriptModified(Object^ Sender, TextEditors::TextEditorScriptModifiedEventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::TextEditor_MouseClick(Object^ Sender, TextEditors::TextEditorMouseClickEventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarNewScript_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarNextScript_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E)
		{
		}

		void ConcreteWorkspaceModel::ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E)
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