#pragma once

public ref class ScriptModifiedEventArgs : public EventArgs
{
public:
	property bool								ModifiedStatus;

	ScriptModifiedEventArgs(bool ModifiedStatus) : EventArgs()
	{
		this->ModifiedStatus = ModifiedStatus;
	}
};

public delegate void							ScriptModifiedEventHandler(Object^ Sender, ScriptModifiedEventArgs^ E);

public interface class IScriptTextEditor
{
	static enum class							FindReplaceOperation
												{
													e_Find = 0,
													e_Replace
												};

	delegate void								FindReplaceOutput(String^ Line, String^ Text);

	void										SetFont(Font^ FontObject);
	void										SetTabCharacterSize(int PixelWidth);
	void										SetContextMenu(ContextMenuStrip^% Strip);

	void										AddControl(Control^ ControlObject);

	String^										GetText(void);
	UInt32										GetTextLength(void);
	void										SetText(String^ Text, bool PreventTextChangedEventHandling);
	void										InsertText(String^ Text, int Index);			// performs bounds check

	String^										GetSelectedText(void);
	void										SetSelectedText(String^ Text, bool PreventTextChangedEventHandling);

	void										SetSelectionStart(int Index);
	void										SetSelectionLength(int Length);

	int											GetCharIndexFromPosition(Point Position);
	Point										GetPositionFromCharIndex(int Index);
	Point										GetAbsolutePositionFromCharIndex(int Index);
	int											GetLineNumberFromCharIndex(int Index);
	bool										GetCharIndexInsideCommentSegment(int Index);
	int											GetCurrentLineNumber(void);

	String^										GetTokenAtCaretPos();
	void										SetTokenAtCaretPos(String^ Replacement);
	String^										GetTokenAtMouseLocation();
	array<String^>^								GetTokensAtMouseLocation();						// gets three of the closest tokens surrounding the mouse loc

	int											GetCaretPos();
	void										SetCaretPos(int Index);
	void										ScrollToCaret();

	IntPtr										GetHandle();

	void										FocusTextArea();
	void										LoadFileFromDisk(String^ Path, UInt32 AllocatedIndex);
	void										SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^% DefaultName, UInt32 AllocatedIndex);

	bool										GetModifiedStatus();
	void										SetModifiedStatus(bool Modified);

	bool										GetInitializingStatus();
	void										SetInitializingStatus(bool Initializing);

	Point										GetLastKnownMouseClickLocation(void);

	UInt32										FindReplace(FindReplaceOperation Operation, String^ Query, String^ Replacement, FindReplaceOutput^ Output);
	void										ToggleComment(int StartIndex);
	void										UpdateIntelliSenseLocalDatabase(void);

	Control^									GetContainer();
	void										ScrollToLine(String^ LineNumber);
	Point										PointToScreen(Point Location);
	void										SetEnabledState(bool State);

	void										BeginUpdate(void);
	void										EndUpdate(void);

	// Events
	void										OnGotFocus(void);					// called when the workspace's is brought to focus
	void										OnLostFocus(void);					// the opposite of the above
	void										OnPositionSizeChange(void);			// called when the workspace's container's position or size changes

	// AvalonEdit specific
	void										HighlightScriptError(int Line);
	void										ClearScriptErrorHighlights(void);
};