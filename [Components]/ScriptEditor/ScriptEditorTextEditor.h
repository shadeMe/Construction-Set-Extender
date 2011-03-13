#pragma once
#include "NumberedRichTextBox.h"
#include "ScriptEditorInterface.h"


ref class IntelliSenseThingy;

public ref class ScriptEditorTextEditor : public NumberedRichTextBox, public ScriptEditorInterface
{
public:
	event EventHandler^							TextChanged;
	event EventHandler^							VScroll;
	event EventHandler^							HScroll;
	event ScriptModifiedEventHandler^			ScriptModified;

	event KeyEventHandler^						KeyUp;
	event KeyEventHandler^						KeyDown;
	event KeyPressEventHandler^					KeyPress;

	event MouseEventHandler^					MouseDown;
	event MouseEventHandler^					MouseDoubleClick;
protected:
	static enum class							PreventTextChangeFlagState
												{
													e_Disabled = 0,
													e_AutoReset,
													e_ManualReset
												};

	int											IndentCountBuffer;
	bool										InitializingFlag;
	bool										ModifiedFlag;
	PreventTextChangeFlagState					PreventTextChangedEventFlag;
	Keys										KeyToPreventHandling;
	PictureBox^									LineLimitIndicator;
	List<PictureBox^>^							FindReplaceResultIndicators;
	IntelliSenseThingy^							IntelliSenseBox;
	Point										LastKnownMouseClickLocation;

	virtual void								OnTextChanged(EventArgs^ E);
	virtual void								OnVScroll(EventArgs^ E);
	virtual void								OnHScroll(EventArgs^ E);
	virtual void								OnScriptModified(ScriptModifiedEventArgs^ E);
	virtual void								OnKeyUp(KeyEventArgs^ E);
	virtual void								OnKeyDown(KeyEventArgs^ E);
	virtual void								OnKeyPress(KeyPressEventArgs^ E);
	virtual void								OnMouseDown(MouseEventArgs^ E);
	virtual void								OnMouseDoubleClick(MouseEventArgs^ E);

	void										TextField_TextChanged(Object^ Sender, EventArgs^ E);
	void										TextField_Resize(Object^ Sender, EventArgs^ E);

	void										TextField_VScroll(Object^ Sender, EventArgs^ E);
	void										TextField_HScroll(Object^ Sender, EventArgs^ E);

	void										TextField_KeyUp(Object^ Sender, KeyEventArgs^ E);
	void										TextField_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void										TextField_KeyPress(Object^ Sender, KeyPressEventArgs^ E);

	void										TextField_MouseDown(Object^ Sender, MouseEventArgs^ E);
	void										TextField_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);

	void										TextField_LineChanged(Object^ Sender, LineChangedEventArgs^ E);

	void										ValidateLineLimit();

	void										PlaceFindReplaceResultIndicatorAtCharIndex(int Index);
	void										UpdateFindReplaceResultLineLimitIndicatorPostions();
	void										RemoveFindReplaceResultIndicators();

	void										HandleHomeKey(void);

	void										PerformAutoIndentationProlog(bool CullEmptyLines);		// updates IndentCountBuffer
	void										PerformAutoIndentationEpilog(void);

	bool										PerformTabIndent(void);

	String^										GetTokenAtIndex(int Index, bool SelectText);
	String^										GetTextAtLocation(Point Location, bool SelectText);		// line breaks need to be replaced by the caller
	String^										GetTextAtLocation(int Index, bool SelectText);

	void										SetPreventTextChangedFlag(PreventTextChangeFlagState State) { PreventTextChangedEventFlag = State; }	
	void										HandleKeyEventForKey(Keys Key) { KeyToPreventHandling = Key; }
public:
	// interface methods
	virtual void								SetFont(Font^ FontObject);
	virtual void								SetTabCharacterSize(int PixelWidth);
	virtual void								SetContextMenu(ContextMenuStrip^% Strip);

	virtual void								AddControl(Control^ ControlObject);

	virtual String^								GetText(void);
	virtual UInt32								GetTextLength(void);
	virtual void								SetText(String^ Text, bool PreventTextChangedEventHandling);

	virtual String^								GetSelectedText(void);
	virtual void								SetSelectedText(String^ Text, bool PreventTextChangedEventHandling);

	virtual void								SetSelectionStart(int Index);
	virtual void								SetSelectionLength(int Length);

	virtual int									GetCharIndexFromPosition(Point Position);
	virtual Point								GetPositionFromCharIndex(int Index);
	virtual int									GetLineNumberFromCharIndex(int Index);
	virtual bool								GetCharIndexInsideCommentSegment(int Index);
	virtual int									GetCurrentLineNumber(void);

	virtual String^								GetTokenAtCaretPos();
	virtual void								SetTokenAtCaretPos(String^ Replacement);
	virtual String^								GetTokenAtMouseLocation();

	virtual int									GetCaretPos();
	virtual void								SetCaretPos(int Index);
	virtual void								ScrollToCaret();

	virtual IntPtr								GetHandle();
	virtual Rectangle							GetBounds(void);

	virtual void								FocusTextArea();
	virtual void								LoadFileFromDisk(String^ Path, UInt32 AllocatedIndex);
	virtual void								SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^% DefaultName, UInt32 AllocatedIndex);

	virtual bool								GetModifiedStatus();
	virtual void								SetModifiedStatus(bool Modified);

	virtual bool								GetInitializingStatus();
	virtual void								SetInitializingStatus(bool Initializing);

	virtual Point								GetLastKnownMouseClickLocation(void);							

	virtual UInt32								FindReplace(ScriptEditorInterface::FindReplaceOperation Operation, String^ Query, String^ Replacement, ScriptEditorInterface::FindReplaceOutput^ Output);
	virtual void								ToggleComment(int StartIndex);
	virtual void								UpdateIntelliSenseLocalDatabase(void);
	virtual void								ClearFindResultIndicators(void) { RemoveFindReplaceResultIndicators(); }

	ScriptEditorTextEditor(UInt32 LinesToScroll, Font^ Font, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Object^% Parent);
};