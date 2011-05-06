#pragma once

#include "ScriptTextEditorInterface.h"
#include "AvalonEditXSHD.h"
#include "AvalonEditComponents.h"

using namespace ICSharpCode;
using namespace System::Windows::Forms::Integration;
using namespace AvalonEditXSHD;
using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;
using namespace AvalonEditComponents;

ref class IntelliSenseThingy;

public ref class AvalonEditTextEditor : public ScriptTextEditorInterface
{
	static AvalonEditXSHDManager^				SyntaxHighlightingManager = gcnew AvalonEditXSHDManager();
public:
	event ScriptModifiedEventHandler^			ScriptModified;
	event KeyEventHandler^						KeyDown;
protected:
	static enum class							PreventTextChangeFlagState
												{
													e_Disabled = 0,
													e_AutoReset,
													e_ManualReset
												};

	Panel^										Container;
	ElementHost^								WPFHost;
	AvalonEdit::TextEditor^						TextField;

	AvalonEditScriptErrorBGColorizer^			ErrorColorizer;
	AvalonEditFindReplaceBGColorizer^			FindReplaceColorizer;

	bool										InitializingFlag;
	bool										ModifiedFlag;
	PreventTextChangeFlagState					PreventTextChangedEventFlag;
	System::Windows::Input::Key					KeyToPreventHandling;
	IntelliSenseThingy^							IntelliSenseBox;
	Point										LastKnownMouseClickLocation;

	virtual void								OnScriptModified(ScriptModifiedEventArgs^ E);
	virtual void								OnKeyDown(KeyEventArgs^ E);

	void										TextField_TextChanged(Object^ Sender, EventArgs^ E);
	void										TextField_CaretPositionChanged(Object^ Sender, EventArgs^ E);

	void										TextField_KeyUp(Object^ Sender, System::Windows::Input::KeyEventArgs^ E);
	void										TextField_KeyDown(Object^ Sender, System::Windows::Input::KeyEventArgs^ E);
	void										TextField_MouseDown(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E);
	void										TextField_MouseWheel(Object^ Sender, System::Windows::Input::MouseWheelEventArgs^ E);

	void										TextField_MouseHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
	void										TextField_MouseHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);

	void										TextField_SelectionChanged(Object^ Sender, EventArgs^ E);

	String^										GetTokenAtIndex(int Index, bool SelectText);
	String^										GetTextAtLocation(Point Location, bool SelectText);		// line breaks need to be replaced by the caller
	String^										GetTextAtLocation(int Index, bool SelectText);

	void										SetPreventTextChangedFlag(PreventTextChangeFlagState State) { PreventTextChangedEventFlag = State; }
	void										HandleKeyEventForKey(System::Windows::Input::Key Key) { KeyToPreventHandling = Key; }

	void										HandleTextChangeEvent();
	void										GotoLine(int Line);						// line numbers start at 1

	void										RefreshUI() { TextField->TextArea->TextView->Redraw(); }
public:
	// interface methods
	virtual void								SetFont(Font^ FontObject);
	virtual void								SetTabCharacterSize(int PixelWidth);	// AvalonEdit uses character lengths
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

	virtual void								FocusTextArea();
	virtual void								LoadFileFromDisk(String^ Path, UInt32 AllocatedIndex);
	virtual void								SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^% DefaultName, UInt32 AllocatedIndex);

	virtual bool								GetModifiedStatus();
	virtual void								SetModifiedStatus(bool Modified);

	virtual bool								GetInitializingStatus();
	virtual void								SetInitializingStatus(bool Initializing);

	virtual Point								GetLastKnownMouseClickLocation(void);

	virtual UInt32								FindReplace(ScriptTextEditorInterface::FindReplaceOperation Operation, String^ Query, String^ Replacement, ScriptTextEditorInterface::FindReplaceOutput^ Output);
	virtual void								ToggleComment(int StartIndex);
	virtual void								UpdateIntelliSenseLocalDatabase(void);
	virtual void								ClearFindResultIndicators(void);

	virtual Control^							GetContainer() { return Container; }
	virtual void								ScrollToLine(String^ LineNumber);
	virtual void								HandleTabSwitchEvent(void);

	virtual void								HighlightScriptError(int Line);
	virtual void								ClearScriptErrorHighlights(void);

	AvalonEditTextEditor(Font^ Font, Object^% Parent);

	static void									InitializeSyntaxHighlightingManager(void);
};
