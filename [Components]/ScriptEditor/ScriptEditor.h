#pragma once
#pragma warning (disable: 4374 4965)

#include "Globals.h"
#include "ScriptParser.h"
#include "ScriptTextEditorInterface.h"
#include "ScriptEditorPreferences.h"
#include "ScriptListDialog.h"
#include "FindReplaceDialog.h"
#include "AuxiliaryTextEditor.h"
#include "[Common]\AuxiliaryWindowsForm.h"

using namespace DevComponents;
using namespace DevComponents::DotNetBar::Events;
using namespace GlobalInputMonitor;

#define NEWSCRIPTID					"New Script"
#define FIRSTRUNSCRIPTID			"New Workspace"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ref class Workspace;

		void													WorkspaceTearingEventHandler(Object^ Sender, MouseEventArgs^ E);

		ref class WorkspaceContainer
		{
		public:
			static enum class									JumpStackNavigationDirection
			{
				e_Back = 0,
				e_Forward
			};

			static enum class									RemoteWorkspaceOperation
			{
				e_CreateNewWorkspaceAndScript = 0,
				e_CreateNewWorkspaceAndSelectScript,
				e_LoadFileIntoNewWorkspace,
				e_CreateNewWorkspaceAndScriptAndSetText,
				e_FindReplaceInOpenWorkspaces
			};

			static MouseEventHandler^							WorkspaceTearingEventDelegate = gcnew MouseEventHandler(&WorkspaceTearingEventHandler);
			static Rectangle									LastUsedBounds = Rectangle(100, 100, 100, 100);
		protected:
			ref struct WorkspaceJumpData
			{
				UInt32											CallingWorkspaceIndex;
				String^											JumpScriptName;

				WorkspaceJumpData(UInt32 WorkspaceIndex, String^ ScriptName);
			};

			Stack<UInt32>^										BackJumpStack;
			Stack<UInt32>^										ForwardJumpStack;

			DotNetBar::SuperTabItem^							GetMouseOverTab();

			AnimatedForm^										EditorForm;
			DotNetBar::SuperTabControl^							EditorTabStrip;
			DotNetBar::ButtonItem^								NewTabButton;
			DotNetBar::ButtonItem^								SortTabsButton;
			Timer^												WorkspaceJumpTimer;			// for deferred jumping, as the calling event needs to return before the active workspace is changed
			bool												DestructionFlag;
			bool												InitializedFlag;

			CancelEventHandler^									EditorFormCancelHandler;
			KeyEventHandler^									EditorFormKeyDownHandler;
			EventHandler^										EditorFormPositionChangedHandler;
			EventHandler^										EditorFormSizeChangedHandler;

			EventHandler<DotNetBar::SuperTabStripTabItemCloseEventArgs^>^			ScriptStripTabItemCloseHandler;
			EventHandler<DotNetBar::SuperTabStripSelectedTabChangedEventArgs^>^		ScriptStripSelectedTabChangedHandler;
			EventHandler<DotNetBar::SuperTabStripTabRemovedEventArgs^>^             ScriptStripTabRemovedHandler;
			EventHandler<MouseEventArgs^>^											ScriptStripMouseClickHandler;
			EventHandler<DotNetBar::SuperTabStripTabMovingEventArgs^>^              ScriptStripTabMovingHandler;

			EventHandler^										NewTabButtonClickHandler;
			EventHandler^										SortTabsButtonClickHandler;
			EventHandler^										ScriptEditorPreferencesSavedHandler;
			EventHandler^										WorkspaceJumpTimerTickHandler;

			void												EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E);
			void												EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void												EditorForm_SizeChanged(Object^ Sender, EventArgs^ E);
			void												EditorForm_PositionChanged(Object^ Sender, EventArgs^ E);

			void												ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs ^ E);
			void												ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs^ E);
			void												ScriptStrip_TabRemoved(Object^ Sender, DotNetBar::SuperTabStripTabRemovedEventArgs^ E);
			void												ScriptStrip_MouseClick(Object^ Sender, MouseEventArgs^ E);
			void												ScriptStrip_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E);

			void												NewTabButton_Click(Object^ Sender, EventArgs^ E);
			void												SortTabsButton_Click(Object^ Sender, EventArgs^ E);

			void												WorkspaceJumpTimer_Tick(Object^ Sender, EventArgs^ E);
			void												ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);

			virtual void										Destroy();
		public:
			WorkspaceContainer(ComponentDLLInterface::ScriptData* InitScript, UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height);
			virtual ~WorkspaceContainer();

			void												FlagDestruction(bool Destroying);
			virtual Workspace^									InstantiateNewWorkspace(ComponentDLLInterface::ScriptData* InitScript);		// takes ownership of pointer
			void												NavigateJumpStack(UInt32 AllocatedIndex, JumpStackNavigationDirection Direction);
			void												JumpToWorkspace(UInt32 AllocatedIndex, String^ ScriptName);
			virtual void										PerformRemoteWorkspaceOperation(RemoteWorkspaceOperation Operation, List<Object^>^ Parameters);
			void												SaveAllOpenWorkspaces();
			void												CloseAllOpenWorkspaces();

			void												DumpAllOpenWorkspacesToFolder(String^ FolderPath, String^ FileExtension);
			void												LoadFileIntoNewWorkspace(String^ FileName);

			Workspace^											LookupWorkspaceByTabIndex(UInt32 TabIndex);

			void												AddTab(DotNetBar::SuperTabItem^ Tab);
			void												RemoveTab(DotNetBar::SuperTabItem^ Tab);
			void												AddTabControlBox(DotNetBar::SuperTabControlPanel^ Box);
			void												RemoveTabControlBox(DotNetBar::SuperTabControlPanel^ Box);

			void												SelectTab(DotNetBar::SuperTabItem^ Tab);
			void												SelectTab(Keys Index);
			void												SelectTab(int Index);
			void												SelectNextTab();
			void												SelectPreviousTab();
			void												Redraw();
			void												SetWindowTitle(String^ Title);
			void												DisableControls(void);
			void												EnableControls(void);

			void												BeginUpdate(void);
			void												EndUpdate(void);

			Rectangle											GetBounds(bool UseRestoreBounds);
			IntPtr												GetHandle();
			FormWindowState										GetWindowState();
			void												SetWindowState(FormWindowState State);
			void												SetCursor(Cursor^ NewCursor);

			UInt32												GetTabCount();
		};

		ref class Workspace
		{
		public:
			static enum class									MessageListItemType
			{
				e_Warning		= 0,
				e_Error,
				e_RegularMessage,
				e_EditorMessage
			};

			static enum class									ScriptType
			{
				e_Object		= 0,
				e_Quest,
				e_MagicEffect	= 0x100
			};

			static enum class									ScriptSaveOperation
			{
				e_SaveAndCompile		= 0,
				e_SaveButDontCompile,
				e_SaveActivePluginToo
			};

			Workspace(UInt32 Index, WorkspaceContainer^ Parent, ComponentDLLInterface::ScriptData* InitScript);
		protected:
			static enum class									SanitizeOperation
			{
				e_Indent		= 0,
				e_AnnealCasing,
				e_EvalifyIfs,
				e_CompilerOverrideBlocks
			};

			DotNetBar::SuperTabItem^							WorkspaceTabItem;
			DotNetBar::SuperTabControlPanel^					WorkspaceControlBox;

			SplitContainer^										WorkspaceSplitter;
			TextEditors::IScriptTextEditor^						TextEditor;
			TextEditors::ScriptOffsetViewer^					OffsetViewer;
			TextEditors::SimpleTextViewer^						PreprocessedTextViewer;

			ListView^											MessageList;
			ListView^											FindList;
			ListView^											BookmarkList;
			Label^												SpoilerText;

			ToolStrip^											WorkspaceMainToolBar;
			ToolStripButton^									ToolBarNewScript;
			ToolStripButton^									ToolBarOpenScript;
			ToolStripButton^									ToolBarPreviousScript;
			ToolStripButton^									ToolBarNextScript;
			ToolStripSplitButton^								ToolBarSaveScript;
			ToolStripDropDown^									ToolBarSaveScriptDropDown;
			ToolStripButton^									ToolBarSaveScriptNoCompile;
			ToolStripButton^									ToolBarSaveScriptAndPlugin;
			ToolStripButton^									ToolBarRecompileScripts;
			ToolStripButton^									ToolBarCompileDependencies;
			ToolStripButton^									ToolBarDeleteScript;
			ToolStripButton^									ToolBarNavigationBack;
			ToolStripButton^									ToolBarNavigationForward;
			ToolStripButton^									ToolBarSaveAll;
			ToolStripButton^									ToolBarOptions;

			ToolStripDropDownButton^							ToolBarScriptType;
			ToolStripDropDown^									ToolBarScriptTypeContents;
			ToolStripButton^									ToolBarScriptTypeContentsObject;
			ToolStripButton^									ToolBarScriptTypeContentsQuest;
			ToolStripButton^									ToolBarScriptTypeContentsMagicEffect;

			ToolStrip^											WorkspaceSecondaryToolBar;
			ToolStripDropDownButton^							ToolBarEditMenu;
			ToolStripDropDown^									ToolBarEditMenuContents;
			ToolStripButton^									ToolBarEditMenuContentsFind;
			ToolStripButton^									ToolBarEditMenuContentsReplace;
			ToolStripButton^									ToolBarEditMenuContentsGotoLine;
			ToolStripButton^									ToolBarEditMenuContentsGotoOffset;
			ToolStripButton^									ToolBarMessageList;
			ToolStripButton^									ToolBarFindList;
			ToolStripButton^									ToolBarBookmarkList;
			ToolStripSplitButton^								ToolBarDumpScript;
			ToolStripDropDown^									ToolBarDumpScriptDropDown;
			ToolStripButton^									ToolBarDumpAllScripts;
			ToolStripSplitButton^								ToolBarLoadScript;
			ToolStripDropDown^									ToolBarLoadScriptDropDown;
			ToolStripButton^									ToolBarLoadScriptsToTabs;
			ToolStripButton^									ToolBarShowOffsets;
			ToolStripButton^									ToolBarShowPreprocessedText;
			ToolStripButton^									ToolBarSanitizeScriptText;
			ToolStripButton^									ToolBarBindScript;
			ToolStripButton^									ToolBarSnippetManager;
			ToolStripProgressBar^								ToolBarByteCodeSize;

			ContextMenuStrip^									TextEditorContextMenu;
			ToolStripMenuItem^									ContextMenuCopy;
			ToolStripMenuItem^									ContextMenuPaste;
			ToolStripMenuItem^									ContextMenuFind;
			ToolStripMenuItem^									ContextMenuToggleComment;
			ToolStripMenuItem^									ContextMenuToggleBookmark;
			ToolStripMenuItem^									ContextMenuAddMessage;
			ToolStripMenuItem^									ContextMenuWord;
			ToolStripMenuItem^									ContextMenuWikiLookup;
			ToolStripMenuItem^									ContextMenuOBSEDocLookup;
			ToolStripMenuItem^									ContextMenuDirectLink;
			ToolStripMenuItem^									ContextMenuJumpToScript;
			ToolStripMenuItem^									ContextMenuGoogleLookup;
			ToolStripMenuItem^									ContextMenuRefactorMenu;
			ToolStripMenuItem^									ContextMenuRefactorAddVariable;
			ToolStripMenuItem^									ContextMenuRefactorAddVariableInt;
			ToolStripMenuItem^									ContextMenuRefactorAddVariableFloat;
			ToolStripMenuItem^									ContextMenuRefactorAddVariableRef;
			ToolStripMenuItem^									ContextMenuRefactorAddVariableString;
			ToolStripMenuItem^									ContextMenuRefactorAddVariableArray;
			ToolStripMenuItem^									ContextMenuRefactorDocumentScript;
			ToolStripMenuItem^									ContextMenuRefactorCreateUDFImplementation;
			ToolStripMenuItem^									ContextMenuRefactorRenameVariables;
			ToolStripMenuItem^									ContextMenuRefactorModifyVariableIndices;
			ToolStripMenuItem^									ContextMenuOpenImportFile;

			ScriptListDialog^									ScriptListBox;
			FindReplaceDialog^									FindReplaceBox;
			WorkspaceContainer^									ParentContainer;
			Timer^												AutoSaveTimer;

			UInt32												WorkspaceHandleIndex;
			bool												DestructionFlag;
			void*												CurrentScript;
			ScriptType											CurrentScriptType;
			String^												CurrentScriptEditorID;
			bool												HandlingKeyDownEvent;
			bool												NewScriptFlag;

			KeyEventHandler^									TextEditorKeyDownHandler;
			TextEditors::TextEditorScriptModifiedEventHandler^	TextEditorScriptModifiedHandler;
			TextEditors::TextEditorMouseClickEventHandler^		TextEditorMouseClickHandler;
			EventHandler^										MessageListDoubleClickHandler;
			ColumnClickEventHandler^							MessageListColumnClickHandler;
			EventHandler^										FindListDoubleClickHandler;
			ColumnClickEventHandler^							FindListColumnClickHandler;
			EventHandler^										BookmarkListDoubleClickHandler;
			ColumnClickEventHandler^							BookmarkListColumnClickHandler;
			EventHandler^										ToolBarNewScriptClickHandler;
			EventHandler^										ToolBarOpenScriptClickHandler;
			EventHandler^										ToolBarPreviousScriptClickHandler;
			EventHandler^										ToolBarNextScriptClickHandler;
			EventHandler^										ToolBarSaveScriptClickHandler;
			EventHandler^										ToolBarSaveScriptNoCompileClickHandler;
			EventHandler^										ToolBarSaveScriptAndPluginClickHandler;
			EventHandler^										ToolBarRecompileScriptsClickHandler;
			EventHandler^										ToolBarCompileDependenciesClickHandler;
			EventHandler^										ToolBarDeleteScriptClickHandler;
			EventHandler^										ToolBarNavigationBackClickHandler;
			EventHandler^										ToolBarNavigationForwardClickHandler;
			EventHandler^										ToolBarSaveAllClickHandler;
			EventHandler^										ToolBarOptionsClickHandler;
			EventHandler^										ToolBarScriptTypeContentsObjectClickHandler;
			EventHandler^										ToolBarScriptTypeContentsQuestClickHandler;
			EventHandler^										ToolBarScriptTypeContentsMagicEffectClickHandler;
			CancelEventHandler^									TextEditorContextMenuOpeningHandler;
			EventHandler^										ContextMenuCopyClickHandler;
			EventHandler^										ContextMenuPasteClickHandler;
			EventHandler^										ContextMenuFindClickHandler;
			EventHandler^										ContextMenuToggleCommentClickHandler;
			EventHandler^										ContextMenuToggleBookmarkClickHandler;
			EventHandler^										ContextMenuAddMessageClickHandler;
			EventHandler^										ContextMenuWikiLookupClickHandler;
			EventHandler^										ContextMenuOBSEDocLookupClickHandler;
			EventHandler^										ContextMenuDirectLinkClickHandler;
			EventHandler^										ContextMenuJumpToScriptClickHandler;
			EventHandler^										ContextMenuGoogleLookupClickHandler;
			EventHandler^										ContextMenuRefactorAddVariableClickHandler;
			EventHandler^										ContextMenuRefactorDocumentScriptClickHandler;
			EventHandler^										ContextMenuRefactorCreateUDFImplementationClickHandler;
			EventHandler^										ContextMenuRefactorRenameVariablesClickHandler;
			EventHandler^										ContextMenuRefactorModifyVariableIndicesClickHandler;
			EventHandler^										ContextMenuOpenImportFileClickHandler;
			EventHandler^										ToolBarEditMenuContentsFindReplaceClickHandler;
			EventHandler^										ToolBarEditMenuContentsGotoLineClickHandler;
			EventHandler^										ToolBarEditMenuContentsGotoOffsetClickHandler;
			EventHandler^										ToolBarMessageListClickHandler;
			EventHandler^										ToolBarFindListClickHandler;
			EventHandler^										ToolBarBookmarkListClickHandler;
			EventHandler^										ToolBarDumpScriptClickHandler;
			EventHandler^										ToolBarDumpAllScriptsClickHandler;
			EventHandler^										ToolBarLoadScriptClickHandler;
			EventHandler^										ToolBarLoadScriptsToTabsClickHandler;
			EventHandler^										ToolBarShowOffsetsClickHandler;
			EventHandler^										ToolBarShowPreprocessedTextClickHandler;
			EventHandler^										ToolBarSanitizeScriptTextClickHandler;
			EventHandler^										ToolBarBindScriptClickHandler;
			EventHandler^										ToolBarSnippetManagerClickHandler;
			EventHandler^										ScriptEditorPreferencesSavedHandler;
			EventHandler^										AutoSaveTimerTickHandler;

			virtual void										TextEditor_KeyDown(Object^ Sender, KeyEventArgs^ E);
			virtual void										TextEditor_ScriptModified(Object^ Sender, TextEditors::TextEditorScriptModifiedEventArgs^ E);
			virtual void										TextEditor_MouseClick(Object^ Sender, TextEditors::TextEditorMouseClickEventArgs^ E);

			virtual void										MessageList_DoubleClick(Object^ Sender, EventArgs^ E);
			virtual void                                        MessageList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
			virtual void									    FindList_DoubleClick(Object^ Sender, EventArgs^ E);
			virtual void									    FindList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
			virtual void									    BookmarkList_DoubleClick(Object^ Sender, EventArgs^ E);
			virtual void									    BookmarkList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);

			virtual void                                        ToolBarNewScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarNextScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarNavigationBack_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarNavigationForward_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarOptions_Click(Object^ Sender, EventArgs^ E);

			virtual void                                        ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E);

			virtual void                                        TextEditorContextMenu_Opening(Object^ Sender, CancelEventArgs^ E);
			virtual void                                        ContextMenuCopy_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuPaste_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuFind_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuToggleComment_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuToggleBookmark_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuAddMessage_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuWikiLookup_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuOBSEDocLookup_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuDirectLink_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuJumpToScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuGoogleLookup_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuOpenImportFile_Click(Object^ Sender, EventArgs^ E);

			virtual void                                        ContextMenuRefactorAddVariable_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuRefactorDocumentScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuRefactorCreateUDFImplementation_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuRefactorRenameVariables_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ContextMenuRefactorModifyVariableIndices_Click(Object^ Sender, EventArgs^ E);

			virtual void                                        ToolBarEditMenuContentsFindReplace_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E);

			virtual void                                        ToolBarMessageList_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarFindList_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarDumpAllScripts_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarLoadScriptsToTabs_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarShowOffsets_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarShowPreprocessedText_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarSanitizeScriptText_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarBindScript_Click(Object^ Sender, EventArgs^ E);
			virtual void                                        ToolBarSnippetManager_Click(Object^ Sender, EventArgs^ E);

			virtual void                                        ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);
			virtual void										AutoSaveTimer_Tick(Object^ Sender, EventArgs^ E);

			virtual bool										PerformHouseKeeping(void);
			virtual void										CleanupAutoRecoveryCacheInstance(void);

			virtual void                                        AddMessageToMessagePool(MessageListItemType Type, int Line, String^ Message);
			virtual void                                        ClearErrorMessagesFromMessagePool(void);
			virtual void                                        ClearEditorMessagesFromMessagePool(void);

			virtual void                                        FindReplaceOutput(String^ Line, String^ Text);
			virtual void                                        ToggleBookmark(int CaretPos);
			virtual void                                        SetScriptType(ScriptType Type);

			virtual void										ToggleSplitter(bool Enabled);

			virtual String^										SerializeCSEBlock(void);
			virtual void                                        SerializeCaretPos(String^% Result);
			virtual void                                        SerializeBookmarks(String^% Result);
			virtual void                                        SerializeMessages(String^% Result);
			virtual String^										DeserializeCSEBlock(String^% Source, String^% ExtractedBlock);
			virtual void                                        DeserializeCaretPos(String^% ExtractedBlock);
			virtual void                                        DeserializeBookmarks(String^% ExtractedBlock);
			virtual void                                        DeserializeMessages(String^% ExtractedBlock);

			virtual bool										ValidateScript(String^% PreprocessedScriptText);
			virtual bool										PreprocessScriptText(String^% PreprocessorResult);
			virtual void                                        PreprocessorErrorOutputWrapper(String^ Message);
			virtual String^										SanitizeScriptText(SanitizeOperation Operation, String^ ScriptText);
			virtual void                                        UpdateEnvironment(ComponentDLLInterface::ScriptData* Data, bool Initializing);
			virtual void                                        Destroy();
		public:
			virtual ~Workspace();

			virtual void                                        NewScript();
			virtual void                                        OpenScript();
			virtual bool										SaveScript(ScriptSaveOperation Operation);
			virtual void                                        DeleteScript();
			virtual void                                        RecompileScripts();
			virtual void                                        PreviousScript();
			virtual void                                        NextScript();
			virtual void                                        CloseScript();

			void												DisableControls();
			void												EnableControls();

			UInt32												GetHandleIndex() { return WorkspaceHandleIndex; }
			TextEditors::IScriptTextEditor^						GetTextEditor() { return TextEditor; }
			bool												GetModifiedStatus() { return TextEditor->GetModifiedStatus(); }
			void												SetModifiedStatus(bool Modified) { TextEditor->SetModifiedStatus(Modified); }
			WorkspaceContainer^									GetParentContainer() { return ParentContainer; }
			String^												GetScriptDescription() { return WorkspaceTabItem->Tooltip; }
			String^												GetScriptID() { return CurrentScriptEditorID; }
			bool												GetIsUninitialized() { return CurrentScriptEditorID == FIRSTRUNSCRIPTID;  }		// returns true until a script's loaded/created into the workspace
			bool												GetIsScriptNew(void) { return CurrentScriptEditorID == NEWSCRIPTID; }
			bool												GetIsTabStripParent(DotNetBar::SuperTabStrip^ TabStrip) { return TabStrip->Tabs->IndexOf(WorkspaceTabItem) != -1; }

			String^												GetCurrentToken() { return TextEditor->GetTokenAtCaretPos(); }
			Point												GetScreenPoint(Point Location) { return TextEditor->PointToScreen(Location); }
			Point												GetCaretLocation(bool AbsoluteValue);

			IntPtr												GetControlBoxHandle() { return WorkspaceControlBox->Handle; }
			IntPtr												GetEditorBoxHandle() { return TextEditor->GetHandle(); }
			String^												GetScriptText() { return TextEditor->GetText(); }
			ScriptType											GetScriptType();

			void												SetScriptText(String^% Text, bool ResetUndoStack) { TextEditor->SetText(Text, false, ResetUndoStack); }
			void												SetCurrentToken(String^% Replacement) { TextEditor->SetTokenAtCaretPos(Replacement); }

			void												BringToFront() { ParentContainer->SelectTab(WorkspaceTabItem); }
			void												Relocate(WorkspaceContainer^ Destination);
			void												Focus() { TextEditor->FocusTextArea(); }
			void												TunnelKeyDownEvent(KeyEventArgs^ E) { TextEditor_KeyDown(TextEditor, E); }

			void												HandleFocus(bool GotFocus);
			void												HandlePositionSizeChange() { TextEditor->OnPositionSizeChange(); }

			void												LoadFileFromDisk(String^ Path);
			void												SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^ Extension);

			int													PerformFindReplace(TextEditors::IScriptTextEditor::FindReplaceOperation Operation, String^ Query, String^ Replacement, UInt32 Options);
			void												InsertVariable(String^ VariableName, ScriptParser::VariableType VariableType);
		};
	}
}