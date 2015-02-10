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
				SaveCompile,
				SaveDontCompile,
				SaveAndPlugin
			};

			TextEditors::IScriptTextEditor^			TextEditor;
			Timer^									AutoSaveTimer;

			void*									CurrentScript;
			Type									CurrentScriptType;
			String^									CurrentScriptEditorID;
			bool									NewScriptFlag;
			bool									Closed;

			ConcreteWorkspaceModelController^		ModelController;
			ConcreteWorkspaceModelFactory^			ModelFactory;

			IWorkspaceView^							BoundParent;
			property bool							IsBound
			{
				virtual bool get() { return BoundParent != nullptr; }
				virtual void set(bool e) {}
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

			void									SetType(Type New);

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