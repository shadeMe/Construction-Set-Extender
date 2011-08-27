#pragma once
#pragma warning (disable: 4374 4965)

#include "Globals.h"
#include "ScriptParser.h"
#include "ScriptEditorPreferences.h"
#include "ScriptListDialog.h"

#include "AvalonEditTextEditor.h"
#include "AuxiliaryTextEditor.h"

using namespace DevComponents;
using namespace DevComponents::DotNetBar::Events;
using namespace GlobalInputMonitor;

#define NEWSCRIPTID					"New Script"
#define FIRSTRUNSCRIPTID			"New Workspace"

namespace ScriptEditor
{
	ref class Workspace;

	void													GlobalInputMonitor_MouseUp(Object^ Sender, MouseEventArgs^ E);

	public ref class TabContainer
	{
	public:
		static enum class									NavigationDirection
																{
																	e_Back = 0,
																	e_Forward
																};

		static enum class									RemoteOperation
																{
																	e_New = 0,
																	e_Open,
																	e_LoadNew,
																	e_NewText
																};

		static MouseEventHandler^							GlobalMouseHook_MouseUpHandler = gcnew MouseEventHandler(&GlobalInputMonitor_MouseUp);
		static Rectangle									LastUsedBounds = Rectangle(100, 100, 100, 100);
	private:
		Stack<UInt32>^										BackJumpStack;
		Stack<UInt32>^										ForwardJumpStack;

		static ImageList^									ScriptModifiedIcons = gcnew ImageList();

		DotNetBar::SuperTabItem^							GetMouseOverTab();

		AnimatedForm^										EditorForm;
		DotNetBar::SuperTabControl^							EditorTabStrip;
		DotNetBar::ButtonItem^								NewTabButton;
		DotNetBar::ButtonItem^								SortTabsButton;
		bool												DestructionFlag;
		bool												InitializedFlag;

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

		void												Destroy();
	public:
		TabContainer(ComponentDLLInterface::ScriptData* InitScript, UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height);
		~TabContainer()
		{
			Destroy();
		}

		Workspace^											InstantiateNewWorkspace(ComponentDLLInterface::ScriptData* InitScript);
		void												NavigateJumpStack(UInt32 AllocatedIndex, NavigationDirection Direction);
		void												JumpToWorkspace(UInt32 AllocatedIndex, String^% ScriptName);
		void												PerformRemoteTabOperation(RemoteOperation Operation, Object^ Arbitrary);
		void												SaveAllOpenWorkspaces();
		void												CloseAllOpenWorkspaces();

		void												SaveAllTabsToFolder(String^ FolderPath);
		void												LoadFileToNewTab(String^ FileName);

		void												FlagDestruction(bool Destroying) { this->DestructionFlag = Destroying; }
		Workspace^											LookupWorkspaceByTabIndex(UInt32 TabIndex);
		void												RemoveTab(DotNetBar::SuperTabItem^ Tab);
		void												AddTab(DotNetBar::SuperTabItem^ Tab);

		void												AddTabControlBox(DotNetBar::SuperTabControlPanel^ Box);
		void												RemoveTabControlBox(DotNetBar::SuperTabControlPanel^ Box);

		void												SelectTab(DotNetBar::SuperTabItem^ Tab);
		void												SelectNextTab();
		void												SelectPreviousTab();
		void												Redraw() { EditorForm->Invalidate(true); }
		void												SetWindowTitle(String^ Title) { EditorForm->Text = Title; }

		Rectangle											GetEditorFormRect();
		IntPtr												GetEditorFormHandle() { return EditorForm->Handle; }
		FormWindowState										GetEditorFormWindowState() { return EditorForm->WindowState; }
		void												SetEditorFormWindowState(FormWindowState State) { EditorForm->WindowState = State; }

		UInt32												GetTabCount() { return EditorTabStrip->Tabs->Count; }
	};

	public ref class Workspace
	{
	public:
		static enum class									MessageType
																{
																	e_Warning	= 0,
																	e_Error,
																	e_Message,
																	e_CSEMessage
																};

		static enum class									ScriptType
																{
																	e_Object	= 0,
																	e_Quest,
																	e_MagicEffect = 0x100
																};

		static enum class									SaveScriptOperation
																{
																	e_SaveAndCompile = 0,
																	e_SaveButDontCompile,
																	e_SaveActivePluginToo
																};

		Workspace(UInt32 Index, TabContainer^ Parent, ComponentDLLInterface::ScriptData* InitScript);
	private:
		static ImageList^									MessageListIcons = gcnew ImageList();

		static enum class									SanitizeOperation
																{
																	e_Indent = 0,
																	e_AnnealCasing
																};

		DotNetBar::SuperTabItem^							EditorTab;
		DotNetBar::SuperTabControlPanel^					EditorControlBox;

		SplitContainer^										WorkspaceSplitter;
			AvalonEditTextEditor^								TextEditor;
			ScriptOffsetViewer^									OffsetViewer;
			SimpleTextViewer^									PreprocessedTextViewer;

			ListView^											MessageList;
			ListView^											FindList;
			ListView^											BookmarkList;
			ListView^											VariableIndexList;
				TextBox^											VariableIndexEditBox;
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
			ToolStripTextBox^									ToolBarCommonTextBox;
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
			ToolStripButton^									ToolBarGetVarIndices;
			ToolStripButton^									ToolBarUpdateVarIndices;
			ToolStripButton^									ToolBarShowOffsets;
			ToolStripButton^									ToolBarShowPreprocessedText;
			ToolStripButton^									ToolBarSanitizeScriptText;
			ToolStripButton^									ToolBarBindScript;

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
			ToolStripMenuItem^									ContextMenuCopyToCTB;
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

		ScriptListDialog^									ScriptListBox;
		TabContainer^										ParentContainer;

		UInt32												AllocatedIndex;
		bool												DestructionFlag;
		void*												CurrentScript;
		ScriptType											CurrentScriptType;
		String^												CurrentScriptEditorID;
		bool												HandlingKeyDownEvent;
		bool												NewScriptFlag;

		void												TextEditor_KeyDown(Object^ Sender, KeyEventArgs^ E);
		void												TextEditor_ScriptModified(Object^ Sender, ScriptModifiedEventArgs^ E);

		void												MessageList_DoubleClick(Object^ Sender, EventArgs^ E);
		void												MessageList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
		void												FindList_DoubleClick(Object^ Sender, EventArgs^ E);
		void												FindList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
		void												BookmarkList_DoubleClick(Object^ Sender, EventArgs^ E);
		void												BookmarkList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
		void												VariableIndexList_DoubleClick(Object^ Sender, EventArgs^ E);
		void												VariableIndexList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
			void												VariableIndexEditBox_LostFocus(Object^ Sender, EventArgs^ E);
			void												VariableIndexEditBox_KeyDown(Object^ Sender, KeyEventArgs^ E);

		void												ToolBarNewScript_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarNextScript_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E);
			void												ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E);
			void												ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarNavigationBack_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarNavigationForward_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarOptions_Click(Object^ Sender, EventArgs^ E);

		void												ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E);

		void												TextEditorContextMenu_Opening(Object^ Sender, CancelEventArgs^ E);
			void												ContextMenuCopy_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuPaste_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuFind_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuToggleComment_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuToggleBookmark_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuAddMessage_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuWikiLookup_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuOBSEDocLookup_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuCopyToCTB_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuDirectLink_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuJumpToScript_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuGoogleLookup_Click(Object^ Sender, EventArgs^ E);

			void												ContextMenuRefactorAddVariable_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuRefactorDocumentScript_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuRefactorCreateUDFImplementation_Click(Object^ Sender, EventArgs^ E);
			void												ContextMenuRefactorRenameVariables_Click(Object^ Sender, EventArgs^ E);

		void												ToolBarCommonTextBox_KeyDown(Object^ Sender, KeyEventArgs^ E);
		void												ToolBarCommonTextBox_LostFocus(Object^ Sender, EventArgs^ E);

		void												ToolBarEditMenuContentsFind_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarEditMenuContentsReplace_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E);

		void												ToolBarMessageList_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarFindList_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E);
			void												ToolBarDumpAllScripts_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E);
			void												ToolBarLoadScriptsToTabs_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarGetVarIndices_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarUpdateVarIndices_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarShowOffsets_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarShowPreprocessedText_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarSanitizeScriptText_Click(Object^ Sender, EventArgs^ E);
		void												ToolBarBindScript_Click(Object^ Sender, EventArgs^ E);

		bool												PerformHouseKeeping(void);

		void												AddMessageToMessagePool(MessageType Type, int Line, String^ Message);
		void												ClearErrorsItemsFromMessagePool(void);
		void												ClearCSEMessagesFromMessagePool(void);

		void												FindReplaceOutput(String^ Line, String^ Text);
		void												FindReplaceWrapper(IScriptTextEditor::FindReplaceOperation Operation);

		void												ToggleBookmark(int CaretPos);

		void												SetScriptType(ScriptType Type);

		String^												SerializeCSEBlock(void);
		void												SerializeCaretPos(String^% Result);
		void												SerializeBookmarks(String^% Result);
		void												SerializeMessages(String^% Result);
		String^												DeserializeCSEBlock(String^% Source, String^% ExtractedBlock);
		void												DeserializeCaretPos(String^% ExtractedBlock);
		void												DeserializeBookmarks(String^% ExtractedBlock);
		void												DeserializeMessages(String^% ExtractedBlock);

		bool												ValidateScript(String^% PreprocessedScriptText);
		bool												PreprocessScriptText(String^% PreprocessorResult);
		void												PreprocessorErrorOutputWrapper(String^ Message);
		void												SanitizeScriptText(SanitizeOperation Operation);
		void												UpdateEnvironment(ComponentDLLInterface::ScriptData* Data, bool Initializing);
		void												Destroy();
	public:
		~Workspace()
		{
			Destroy();
		}

		// workspace actions
		void												NewScript();
		void												OpenScript();
		bool												SaveScript(SaveScriptOperation Operation);
		void												DeleteScript();
		void												RecompileScripts();
		void												PreviousScript();
		void												NextScript();
		void												CloseScript();

		void												DisableControls();
		void												EnableControls();

		UInt32												GetAllocatedIndex() { return AllocatedIndex; }
		bool												GetModifiedStatus() { return TextEditor->GetModifiedStatus(); }
		void												SetModifiedStatus(bool Modified) { TextEditor->SetModifiedStatus(Modified); }
		TabContainer^%										GetParentContainer() { return ParentContainer; }
		String^												GetScriptDescription() { return EditorTab->Tooltip; }
		String^												GetScriptID() { return CurrentScriptEditorID; }
		bool												GetIsFirstRun() { return CurrentScriptEditorID == FIRSTRUNSCRIPTID;  }		// returns true until a script's loaded/created into the workspace
		bool												GetIsCurrentScriptNew(void) { return CurrentScriptEditorID == NEWSCRIPTID; }
		bool												GetIsTabStripParent(DotNetBar::SuperTabStrip^ TabStrip) { return TabStrip->Tabs->IndexOf(EditorTab) != -1; }

		String^												GetCurrentToken() { return TextEditor->GetTokenAtCaretPos(); }
		Point												GetScreenPoint(Point Location) { return TextEditor->PointToScreen(Location); }
		Point												GetCaretLocation() { return TextEditor->GetPositionFromCharIndex(TextEditor->GetCaretPos()); }

		IntPtr												GetControlBoxHandle() { return EditorControlBox->Handle; }
		IntPtr												GetEditorBoxHandle() { return TextEditor->GetHandle(); }
		String^												GetScriptText() { return TextEditor->GetText(); }
		ScriptType											GetScriptType();

		void												SetScriptText(String^% Text) { TextEditor->SetText(Text, false); }
		void												SetCurrentToken(String^% Replacement) { TextEditor->SetTokenAtCaretPos(Replacement); }

		void												BringToFront() { ParentContainer->SelectTab(EditorTab); }
		void												Relocate(TabContainer^ Destination);
		void												Focus() { TextEditor->FocusTextArea(); }
		void												TunnelKeyDownEvent(KeyEventArgs^ E) { TextEditor_KeyDown(TextEditor, E); }

		void												HandleFocus(bool GotFocus);
		void												HandlePositionSizeChange() { TextEditor->OnPositionSizeChange(); }

		void												LoadFileFromDisk(String^ Path);
		void												SaveScriptToDisk(String^ Path, bool PathIncludesFileName);
	};
}