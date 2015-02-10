#pragma once

#include "WorkspaceModelInterface.h"
#include "WorkspaceViewInterface.h"
#include "AvalonEditTextEditor.h"

#define NEWSCRIPTID					"New Script"
#define FIRSTRUNSCRIPTID			"New Workspace"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ref class ConcreteWorkspaceModelController;
		ref class ConcreteWorkspaceModelFactory;

		ref class ConcreteWorkspaceModel : public IWorkspaceModel
		{
			KeyEventHandler^									TextEditorKeyDownHandler;
			TextEditors::TextEditorScriptModifiedEventHandler^	TextEditorScriptModifiedHandler;
			TextEditors::TextEditorMouseClickEventHandler^		TextEditorMouseClickHandler;

			void										TextEditor_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void										TextEditor_ScriptModified(Object^ Sender, TextEditors::TextEditorScriptModifiedEventArgs^ E);
			void										TextEditor_MouseClick(Object^ Sender, TextEditors::TextEditorMouseClickEventArgs^ E);

			EventHandler^								ToolBarNewScriptClickHandler;
			EventHandler^								ToolBarOpenScriptClickHandler;
			EventHandler^								ToolBarPreviousScriptClickHandler;
			EventHandler^								ToolBarNextScriptClickHandler;
			EventHandler^								ToolBarSaveScriptClickHandler;
			EventHandler^								ToolBarSaveScriptNoCompileClickHandler;
			EventHandler^								ToolBarSaveScriptAndPluginClickHandler;
			EventHandler^								ToolBarRecompileScriptsClickHandler;
			EventHandler^								ToolBarCompileDependenciesClickHandler;
			EventHandler^								ToolBarDeleteScriptClickHandler;

			void                                        ToolBarNewScript_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarNextScript_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E);

			EventHandler^								ToolBarScriptTypeContentsObjectClickHandler;
			EventHandler^								ToolBarScriptTypeContentsQuestClickHandler;
			EventHandler^								ToolBarScriptTypeContentsMagicEffectClickHandler;

			void                                        ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E);

			EventHandler^								ToolBarEditMenuContentsFindReplaceClickHandler;
			EventHandler^								ToolBarEditMenuContentsGotoLineClickHandler;
			EventHandler^								ToolBarEditMenuContentsGotoOffsetClickHandler;

			void                                        ToolBarEditMenuContentsFindReplace_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E);

			EventHandler^								ToolBarDumpScriptClickHandler;
			EventHandler^								ToolBarLoadScriptClickHandler;
			EventHandler^								ToolBarShowOffsetsClickHandler;
			EventHandler^								ToolBarShowPreprocessedTextClickHandler;
			EventHandler^								ToolBarSanitizeScriptTextClickHandler;
			EventHandler^								ToolBarBindScriptClickHandler;
			EventHandler^								ToolBarSnippetManagerClickHandler;

			void                                        ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarShowOffsets_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarShowPreprocessedText_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarSanitizeScriptText_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarBindScript_Click(Object^ Sender, EventArgs^ E);
			void                                        ToolBarSnippetManager_Click(Object^ Sender, EventArgs^ E);

			EventHandler^								AutoSaveTimerTickHandler;
			EventHandler^								ScriptEditorPreferencesSavedHandler;

			void                                        ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);
			void										AutoSaveTimer_Tick(Object^ Sender, EventArgs^ E);
		public:
			static enum class Type
			{
				Object = 0,
				Quest = 1,
				MagicEffect = 0x100
			};

			static enum class SaveOperation
			{
				Default,
				NoCompile,
				SavePlugin,
			};

			TextEditors::IScriptTextEditor^			TextEditor;
			Timer^									AutoSaveTimer;

			void*									CurrentScript;
			Type									CurrentScriptType;
			String^									CurrentScriptEditorID;
			UInt32									CurrentScriptFormID;
			bool									NewScriptFlag;
			bool									Closed;

			ConcreteWorkspaceModelController^		ModelController;
			ConcreteWorkspaceModelFactory^			ModelFactory;

			IWorkspaceView^							BoundParent;
			property bool							Bound
			{
				virtual bool get() { return BoundParent != nullptr; }
				virtual void set(bool e) {}
			}

			property String^						Description
			{
				virtual String^ get() { return CurrentScriptEditorID + " [" + CurrentScriptFormID.ToString("X8") + "]"; }
				virtual void set(String^ e) {}
			}

			void									Setup(ComponentDLLInterface::ScriptData* Data);

			void									Bind(IWorkspaceView^ To);
			void									Unbind();

			void									NewScript();
			void									OpenScript();
			bool									SaveScript(SaveOperation Operation);
			void									DeleteScript();
			void									RecompileScripts();
			void									PreviousScript();
			void									NextScript();
			void									CloseScript();

			void									SetType(Type New, bool Redraw);
			void									ShowFindReplace();
			void									GotoLine();
			void									GotoOffset();

			void									LoadFromDisk(String^ Path);
			void									SaveToDisk(String^ Path, bool PathIncludesFileName, String^ Extension);

			ConcreteWorkspaceModel(ConcreteWorkspaceModelController^ Controller, ConcreteWorkspaceModelFactory^ Factory, ComponentDLLInterface::ScriptData* Data);
			~ConcreteWorkspaceModel();

#pragma region Interfaces
			property IWorkspaceModelFactory^		Factory
			{
				virtual IWorkspaceModelFactory^ get() { return ModelFactory; }
				virtual void set(IWorkspaceModelFactory^ e) {}
			}
			property IWorkspaceModelController^		Controller
			{
				virtual IWorkspaceModelController^ get() { return ModelController; }
				virtual void set(IWorkspaceModelController^ e) {}
			}

			property bool							Initialized
			{
				virtual bool get() { return CurrentScript != nullptr; }
				virtual void set(bool e) {}
			}
			property bool							New
			{
				virtual bool get() { return NewScriptFlag; }
				virtual void set(bool e) {}
			}
#pragma endregion
		};

		ref class ConcreteWorkspaceModelController : public IWorkspaceModelController
		{
		public:
			virtual void					Bind(IWorkspaceModel^ Model, IWorkspaceView^ To) override;
			virtual void					Unbind(IWorkspaceModel^ Model) override;

			virtual void					TunnelKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E) override;

			virtual void					SetText(IWorkspaceModel^ Model, String^ Text, bool ResetUndoStack) override;

			virtual void					New(IWorkspaceModel^ Model) override;
			virtual void					Open(IWorkspaceModel^ Model) override;
			virtual void					Save(IWorkspaceModel^ Model) override;
			virtual bool					Close(IWorkspaceModel^ Model) override;

			virtual void					LoadFromDisk(IWorkspaceModel^ Model, String^ PathToFile) override;
			virtual void					SaveToDisk(IWorkspaceModel^ Model, String^ PathToFile, bool PathIncludesFileName, String^ Extension) override;

			virtual int						FindReplace(IWorkspaceModel^ Model, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
														String^ Query, String^ Replacement, UInt32 Options) override;
		};

		ref class ConcreteWorkspaceModelFactory : public IWorkspaceModelFactory
		{
			List<ConcreteWorkspaceModel^>^				Allocations;

			ConcreteWorkspaceModelFactory();
			~ConcreteWorkspaceModelFactory();
		public:
			property UInt32								Count
			{
				virtual UInt32 get() { return Allocations->Count; }
				virtual void set(UInt32 e) {}
			}

			static ConcreteWorkspaceModelFactory^		Instance = gcnew ConcreteWorkspaceModelFactory;

			void										Remove(ConcreteWorkspaceModel^ Allocation);

			// IWorkspaceModelFactory
			virtual IWorkspaceModel^				CreateModel(ComponentDLLInterface::ScriptData* Data);
			virtual IWorkspaceModelController^		CreateController();
		};
	}
}