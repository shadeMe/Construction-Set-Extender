#pragma once
#include "Globals.h"
#include "ScriptParser.h"
#include "OptionsDialog.h"
#include "ScriptListDialog.h"

#include "AvalonEditTextEditor.h"
#include "AuxiliaryTextEditor.h"

struct ScriptData;
using namespace DevComponents;
using namespace GlobalInputMonitor;
using namespace System::Reflection;

namespace ScriptEditor
{
	ref class Workspace;

	void													GlobalInputMonitor_MouseUp(Object^ Sender, MouseEventArgs^ E);
	Assembly^												ResolveMissingAssemblies(Object^ Sender, ResolveEventArgs^ E);

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
																	e_LoadNew
																};

		static MouseEventHandler^							GlobalMouseHook_MouseUpHandler = gcnew MouseEventHandler(&GlobalInputMonitor_MouseUp);
		static Rectangle									LastUsedBounds = Rectangle(100, 100, 100, 100);
	private:
		Stack<UInt32>^										BackStack;
		Stack<UInt32>^										ForwardStack;

		bool												RemovingTab;
		static ImageList^									FileFlags = gcnew ImageList();

		DotNetBar::TabItem^									GetMouseOverTab();

		Form^												EditorForm;
		DotNetBar::TabControl^								ScriptStrip;
		DotNetBar::TabItem^									NewTabButton;
		bool												DestructionFlag;

		void												EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E);
		void												EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E);

		void												ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::TabStripActionEventArgs^ E);
		void												ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::TabStripTabChangedEventArgs^ E);
		void												ScriptStrip_SelectedTabChanging(Object^ Sender, DotNetBar::TabStripTabChangingEventArgs^ E);
		void												ScriptStrip_TabRemoved(Object^ Sender, EventArgs^ E);
		void												ScriptStrip_MouseClick(Object^ Sender, MouseEventArgs^ E);
		void												ScriptStrip_MouseDown(Object^ Sender, MouseEventArgs^ E);
		void												ScriptStrip_MouseUp(Object^ Sender, MouseEventArgs^ E);

		void												NewTabButton_Click(Object^ Sender, EventArgs^ E);
	public:
		TabContainer(UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height);

		UInt32												CreateNewTab(String^ ScriptName);
		void												NavigateStack(UInt32 AllocatedIndex, NavigationDirection Direction);
		void												JumpToScript(UInt32 AllocatedIndex, String^% ScriptName);
		void												PerformRemoteOperation(RemoteOperation Operation, Object^ Arbitrary);
		void												SaveAllTabs();
		void												CloseAllTabs();

		void												DumpAllTabs(String^ FolderPath);
		void												LoadToTab(String^ FileName);

		void												Destroy();

		void												FlagDestruction(bool Destroying) { this->DestructionFlag = Destroying; }
		Workspace^											LookupWorkspaceByTab(UInt32 TabIndex);
		void												RemoveTab(DotNetBar::TabItem^ Tab);
		void												AddTab(DotNetBar::TabItem^ Tab);

		void												AddTabControlBox(DotNetBar::TabControlPanel^ Box);
		void												RemoveTabControlBox(DotNetBar::TabControlPanel^ Box);

		void												SelectTab(DotNetBar::TabItem^ Tab);
		void												RedrawContainer() { EditorForm->Invalidate(true); }
		void												SetWindowTitle(String^ Title) { EditorForm->Text = Title; }

		Rectangle											GetEditorFormRect();
		IntPtr												GetEditorFormHandle() { return EditorForm->Handle; }
		FormWindowState										GetEditorFormWindowState() { return EditorForm->WindowState; }
		void												SetEditorFormWindowState(FormWindowState State) { EditorForm->WindowState = State; }
	};

	public ref class Workspace
	{
		Workspace(UInt32 Index);
	public:
		static enum class									MessageType
															{
																e_Warning	= 0,
																e_Error,
																e_Message,
																e_CSEMessage
															};
		Workspace(UInt32 Index, TabContainer^ Parent);

		static Workspace^									NullWorkspace = gcnew Workspace(0);
	private:
		static ImageList^									MessageIcon = gcnew ImageList();
		static enum class									SanitizeOperation
															{
																e_Indent = 0,
																e_AnnealCasing
															};

		DotNetBar::TabItem^									EditorTab;
		DotNetBar::TabControlPanel^							EditorControlBox;

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

		ScriptListDialog^									ScriptListingDialog;
		TabContainer^										ParentContainer;

		UInt32												AllocatedIndex;
		bool												DestructionFlag;
		UInt32												ScriptType;
		String^												ScriptEditorID;
		bool												HandlingKeyDownEvent;

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

		void												EnableControls();
		void												ClearErrorsItemsFromMessagePool(void);

		void												FindReplaceOutput(String^ Line, String^ Text);
		void												FindReplaceWrapper(ScriptTextEditorInterface::FindReplaceOperation Operation);

		void												ToggleBookmark(int CaretPos);
		void												SetScriptType(UInt16 ScriptType);
		void												SerializeCaretPos(String^% Result);
		void												SerializeBookmarks(String^% Result);
		void												SerializeMessages(String^% Result);
		String^												DeserializeCSEBlock(String^% Source, String^% ExtractedBlock);
		void												DeserializeCaretPos(String^% ExtractedBlock);
		void												DeserializeBookmarks(String^% ExtractedBlock);
		void												DeserializeMessages(String^% ExtractedBlock);

		void												PreprocessorErrorOutputWrapper(String^ Message);
		void												SanitizeScriptText(SanitizeOperation Operation);
	public:
		UInt32												GetAllocatedIndex() { return AllocatedIndex; }
		bool												GetModifiedStatus() { return TextEditor->GetModifiedStatus(); }
		void												SetModifiedStatus(bool Modified) { TextEditor->SetModifiedStatus(Modified); }
		TabContainer^%										GetParentContainer() { return ParentContainer; }
		void												InitializeScript(String^ ScriptText, UInt16 ScriptType, String^ ScriptName, UInt32 Data, UInt32 DataLength, UInt32 FormID);
		void												UpdateScriptFromDataPackage(ScriptData* Package);
		void												AddItemToScriptListDialog(String^% ScriptName, UInt32 FormID, UInt16 Type, UInt32 Flags);
		void												AddItemToVariableIndexList(String^% Name, UInt32 Type, UInt32 Index);
		String^												GetScriptDescription() { return EditorTab->Text; }
		const String^										GetScriptID() { return ScriptEditorID; }
		bool												GetIsCurrentScriptNew(void) { return ScriptEditorID == "New Script"; }
		void												ShowScriptListBox(ScriptListDialog::Operation Op) { ScriptListingDialog->Show(Op); }
		void												LoadFileFromDisk(String^ Path);
		void												SaveScriptToDisk(String^ Path, bool PathIncludesFileName);
		void												MakeActiveInParentContainer() { ParentContainer->SelectTab(EditorTab); }
		bool												GetIsTabStripParent(DotNetBar::TabStrip^ Strip) { return Strip->Tabs->IndexOf(EditorTab) != -1; }
		void												PerformCompileAndSave() { ToolBarSaveScript_Click(nullptr, nullptr); }
		String^												GetCurrentToken() { return TextEditor->GetTokenAtCaretPos(); }
		Point												GetCaretLocation() { return TextEditor->GetPositionFromCharIndex(TextEditor->GetCaretPos()); }
		IntPtr												GetControlBoxHandle() { return EditorControlBox->Handle; }
		IntPtr												GetEditorBoxHandle() { return TextEditor->GetHandle(); }
		const String^										GetScriptText() { return TextEditor->GetText(); }
		void												SetCurrentToken(String^% Replacement) { TextEditor->SetTokenAtCaretPos(Replacement); }
		bool												ValidateScript(String^% PreprocessedScriptText);
		void												Destroy();
		UInt16												GetScriptType();
		bool												PreprocessScriptText(String^% PreprocessorResult);
		void												AddMessageToPool(MessageType Type, int Line, String^ Message);
		void												ClearCSEMessagesFromMessagePool(void);
		void												Relocate(TabContainer^ Destination);
		String^												SerializeCSEBlock(void);
		void												Focus() { TextEditor->FocusTextArea(); }
		void												HandleWorkspaceFocus() { TextEditor->HandleTabSwitchEvent(); }

		bool												IsValid() { return this != NullWorkspace; }
	};
}