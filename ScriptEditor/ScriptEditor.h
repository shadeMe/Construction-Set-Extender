#pragma once
#include "Common\Includes.h"
#include "ScriptParser.h"


struct ScriptData;

// TODO: ++++++++++++++++++


using namespace DevComponents;
using namespace ICSharpCode::AvalonEdit;

ref class ScriptListDialog;
ref class ScriptParser;
ref class SyntaxBox;


namespace ScriptEditor
{
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
																e_Open
															};
private:
	void												EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E);
	void												ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::TabStripActionEventArgs^ E);
	void												ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::TabStripTabChangedEventArgs^ E);
	void												ScriptStrip_SelectedTabChanging(Object^ Sender, DotNetBar::TabStripTabChangingEventArgs^ E);
	void												ScriptStrip_TabRemoved(Object^ Sender, EventArgs^ E);
	void												NewTabButton_Click(Object^ Sender, EventArgs^ E);
	 
	Stack<UInt32>^										BackStack;
	Stack<UInt32>^										ForwardStack;

	bool												Destroying;
	bool												RemovingTab;
	static ImageList^									FileFlags = gcnew ImageList();
public:
	Form^												EditorForm;
	DotNetBar::TabControl^								ScriptStrip;
	DotNetBar::TabItem^									NewTabButton;

	TabContainer(UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height);

	UInt32												CreateNewTab(String^ ScriptName);
	void												NavigateStack(UInt32 AllocatedIndex, NavigationDirection Direction);
	void												JumpToScript(UInt32 AllocatedIndex, String^% ScriptName);
	void												PerformRemoteOperation(RemoteOperation Operation);
	void												SaveAllTabs();
	void												CloseAllTabs();

	void												Destroy();
};

public ref class Workspace
{
public:
	void												ToolBarConsole_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarOffsetToggle_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E);
private:
	static enum class									MessageType
														{
															e_Warning	= 0,
															e_Error
														};
	static ImageList^									MessageIcon = gcnew ImageList();

														// EVENT HANDLERS

	void												EditorBox_TextChanged(Object^ Sender, EventArgs^ E);
	void												EditorBox_VScroll(Object^ Sender, EventArgs^ E);
	void												EditorBox_Resize(Object^ Sender, EventArgs^ E);
	void												EditorBox_KeyUp(Object^ Sender, KeyEventArgs^ E);
	void												EditorBox_MouseDown(Object^ Sender, MouseEventArgs^ E);
	void												EditorBox_MouseUp(Object^ Sender, MouseEventArgs^ E);
	void												EditorBox_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void												EditorBox_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
	void												EditorBox_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);
	void												EditorBox_HScroll(Object^ Sender, EventArgs^ E);
	

	void												EditorLineNo_MouseDown(Object^ Sender, MouseEventArgs^ E);

	void												ToolBarEditMenuContentsFind_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarEditMenuContentsReplace_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarCommonTextBox_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void												ToolBarCommonTextBox_KeyUp(Object^ Sender, KeyEventArgs^ E);
	void												ToolBarCommonTextBox_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
	void												ToolBarCommonTextBox_LostFocus(Object^ Sender, EventArgs^ E);
	void												ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarOptions_Click(Object^ Sender, EventArgs^ E);

	void												ToolBarErrorList_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarFindList_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E);

	void												ToolBarNewScript_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarNextScript_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E);

	void												ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E);

	void												ToolBarNavBack_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarNavForward_Click(Object^ Sender, EventArgs^ E);

	void												ToolBarGetVarIndices_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarUpdateVarIndices_Click(Object^ Sender, EventArgs^ E);
	void												ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E);

	void												EditorContextMenu_Opening(Object^ Sender, CancelEventArgs^ E);


	void												ContextMenuCopy_Click(Object^ Sender, EventArgs^ E);
	void												ContextMenuPaste_Click(Object^ Sender, EventArgs^ E);
	void												ContextMenuWikiLookup_Click(Object^ Sender, EventArgs^ E);
	void												ContextMenuOBSEDocLookup_Click(Object^ Sender, EventArgs^ E);
	void												ContextMenuCopyToCTB_Click(Object^ Sender, EventArgs^ E);
	void												ContextMenuFind_Click(Object^ Sender, EventArgs^ E);
	void												ContextMenuToggleComment_Click(Object^ Sender, EventArgs^ E);
	void												ContextMenuToggleBookmark_Click(Object^ Sender, EventArgs^ E);
	void												ContextMenuDirectLink_Click(Object^ Sender, EventArgs^ E);
	void												ContextMenuJumpToScript_Click(Object^ Sender, EventArgs^ E);
	
	void												ErrorBox_DoubleClick(Object^ Sender, EventArgs^ E);
	void												FindBox_DoubleClick(Object^ Sender, EventArgs^ E);
	void												BookmarkBox_DoubleClick(Object^ Sender, EventArgs^ E);
	void												ErrorBox_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
	void												FindBox_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
	void												BookmarkBox_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
	void												VariableBox_DoubleClick(Object^ Sender, EventArgs^ E);
	void												VariableBox_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);


	void												UpdateLineNumbers(void);
	void												FindAndReplace(bool Replace);
	void												JumpToLine(String^ LineStr, bool OffsetSearch);
	int													FindLineNumberInLineBox(UInt32 Line);
	void												PerformLineNumberHighlights(void);
	int													CalculateIndents(int EndPos, bool& ExdentLine, bool CullEmptyLines);
	void												ExdentLine(void);
	void												AddMessageToPool(MessageType Type, UInt32 Line, String^ Message);
	bool												TabIndent();
	void												ToggleComment(int CaretPos);
	void												UpdateImagePointers(void);
	void												PlaceFindImagePointer(int Index);
	bool												IsDelimiterKey(Keys KeyCode);
	void												MoveCaretToValidHome(void);
	void												ToggleBookmark(int CaretPos);
	void												ReadBookmarks(String^ ExtractedBlock);
	void												DumpBookmarks(void);
	void												SaveCaretPos(void);
	void												LoadSavedCaretPos(String^ ExtractedBlock);
	bool												IsCursorInsideCommentSeg(bool OneLessIdx);
	bool												HasLineChanged();
	void												ValidateLineLimit(void);
public:							
	SyntaxBox^											ISBox;
	DotNetBar::TabItem^									EditorTab;
	DotNetBar::TabControlPanel^							EditorControlBox;

	SplitContainer^										EditorBoxSplitter;
		SplitContainer^										EditorSplitter;	
			RichTextBox^										EditorLineNo;
			RichTextBox^										EditorBox;
		ListView^											FindBox;
		ListView^											BookmarkBox;
		ListView^											ErrorBox;
		TextBox^											ConsoleBox;
		ListView^											VariableBox;

	ToolStrip^											EditorToolBar;
		ToolStripTextBox^									ToolBarCommonTextBox;
		ToolStripDropDownButton^							ToolBarEditMenu;
			ToolStripDropDown^									ToolBarEditMenuContents;
				ToolStripButton^									ToolBarEditMenuContentsFind;
				ToolStripButton^									ToolBarEditMenuContentsReplace;
				ToolStripButton^									ToolBarEditMenuContentsGotoLine;
				ToolStripButton^									ToolBarEditMenuContentsGotoOffset;
		ToolStripButton^									ToolBarErrorList;
		ToolStripButton^									ToolBarFindList;
		ToolStripButton^									ToolBarBookmarkList;
		ToolStripButton^									ToolBarDumpScript;
		ToolStripButton^									ToolBarLoadScript;
		ToolStripButton^									ToolBarOptions;
		ToolStripButton^									ToolBarConsole;

		ToolStripButton^									ToolBarNewScript;
		ToolStripButton^									ToolBarOpenScript;
		ToolStripButton^									ToolBarPreviousScript;
		ToolStripButton^									ToolBarNextScript;
		ToolStripButton^									ToolBarSaveScript;
		ToolStripButton^									ToolBarRecompileScripts;
		ToolStripButton^									ToolBarDeleteScript;

		ToolStripButton^									ToolBarOffsetToggle;
		ToolStripProgressBar^								ToolBarByteCodeSize;

		ToolStripButton^									ToolBarNavigationBack;
		ToolStripButton^									ToolBarNavigationForward;

		ToolStripButton^									ToolBarGetVarIndices;
		ToolStripButton^									ToolBarUpdateVarIndices;
		ToolStripButton^									ToolBarSaveAll;

		ToolStripDropDownButton^							ToolBarScriptType;
			ToolStripDropDown^									ToolBarScriptTypeContents;
				ToolStripButton^									ToolBarScriptTypeContentsObject;
				ToolStripButton^									ToolBarScriptTypeContentsQuest;
				ToolStripButton^									ToolBarScriptTypeContentsMagicEffect;

	ToolStrip^											BoxToolBar;

	ContextMenuStrip^									EditorContextMenu;
		ToolStripMenuItem^									ContextMenuCopy;
		ToolStripMenuItem^									ContextMenuPaste;
		ToolStripMenuItem^									ContextMenuFind;
		ToolStripMenuItem^									ContextMenuToggleComment;		
		ToolStripMenuItem^									ContextMenuToggleBookmark;		
		ToolStripMenuItem^									ContextMenuWord;
		ToolStripMenuItem^									ContextMenuWikiLookup;
		ToolStripMenuItem^									ContextMenuOBSEDocLookup;
		ToolStripMenuItem^									ContextMenuCopyToCTB;
		ToolStripMenuItem^									ContextMenuDirectLink;
		ToolStripMenuItem^									ContextMenuJumpToScript;


	ScriptParser^										ScriptTextParser;
	ScriptListDialog^									ScriptListBox;
	PictureBox^											ScriptLineLimitIndicator;

	UInt32												AllocatedIndex;
	bool												HasChanged;
	bool												TextSet;
	bool												Destroying;
	Keys												HandleKeyEditorBox;			
	Keys												HandleKeyCTB;	
	bool												HandleTextChanged;			// must be set to false before modifying box text outside of the TextChanged handler
																					// keeps the syntaxbox from showing up
	int													Indents;
	String^												PreProcessedText;
	List<PictureBox^>^									IndexPointers;
	List<int>^											BookmarkedLines;
	UInt32												CurrentLineNo;
	List<UInt16>^										LineOffsets;
	bool												GetVariableData;
	UInt32												ScriptType;
	TabContainer^										ParentStrip;
	
	void												ValidateScript(UInt32 ScriptType);
	void												Destroy();
	void												EnableControls();
	UInt16												GetScriptType();
	void												SetScriptType(UInt16 ScriptType);
	void												PreProcessScriptText(PreProcessor::PreProcessOp Operation, String^ ScriptText);
	String^												GetTextAtLoc(Point Loc, bool FromMouse, bool SelectText, int Index, bool ReplaceLineBreaks);
	void												CalculateLineOffsets(UInt32 Data, UInt32 Length, String^% ScriptText);
	void												GetVariableIndices(void);
	void												ClearFindImagePointers(void);

	Workspace(UInt32 Index, TabContainer^% Parent);
	Workspace(UInt32 Index);

	static Workspace^									NullSE = gcnew Workspace(0);
};

}