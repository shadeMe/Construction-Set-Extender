#pragma once

#include "ScriptTextEditorInterface.h"
#include "AvalonEditXSHD.h"
#include "AvalonEditComponents.h"
#include "IntelliSense.h"

using namespace ICSharpCode;
using namespace System::Windows::Forms::Integration;
using namespace AvalonEditXSHD;
using namespace AvalonEditComponents;

typedef System::Windows::Media::Imaging::RenderTargetBitmap RTBitmap;

public ref class AvalonEditTextEditor : public IScriptTextEditor
{
	static AvalonEditXSHDManager^				SyntaxHighlightingManager = gcnew AvalonEditXSHDManager();

	static void									InitializeSyntaxHighlightingManager(bool Reset);
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

	static enum class							MoveSegmentDirection
													{
														e_Up = 0,
														e_Down
													};

	Panel^										Container;
	ElementHost^								WPFHost;
	System::Windows::Controls::DockPanel^		TextFieldPanel;
	AvalonEdit::TextEditor^						TextField;
	System::Windows::Shapes::Rectangle^			AnimationPrimitive;

	AvalonEditScriptErrorBGColorizer^			ErrorColorizer;
	AvalonEditFindReplaceBGColorizer^			FindReplaceColorizer;
	AvalonEdit::Folding::FoldingManager^		CodeFoldingManager;
	AvalonEditObScriptCodeFoldingStrategy^		CodeFoldingStrategy;

	bool										InitializingFlag;
	bool										ModifiedFlag;
	PreventTextChangeFlagState					PreventTextChangedEventFlag;
	System::Windows::Input::Key					KeyToPreventHandling;
	IntelliSense::IntelliSenseInterface^		IntelliSenseBox;
	Point										LastKnownMouseClickLocation;
	System::Windows::Input::Key					LastKeyThatWentDown;

	System::Windows::Point						ScrollStartPoint;
	System::Windows::Vector						CurrentScrollOffset;
	bool										IsMiddleMouseScrolling;
	Timer^										MiddleMouseScrollTimer;

	bool										IsFocused;
	Timer^										FoldingTimer;

	Timer^										ScrollBarSyncTimer;
	VScrollBar^									ExternalVerticalScrollBar;
	HScrollBar^									ExternalHorizontalScrollBar;
	bool										SynchronizingExternalScrollBars;
	bool										SynchronizingInternalScrollBars;

	bool										SetTextAnimating;

	virtual void								OnScriptModified(ScriptModifiedEventArgs^ E);
	virtual void								OnKeyDown(KeyEventArgs^ E);

	void										TextField_TextChanged(Object^ Sender, EventArgs^ E);
	void										TextField_CaretPositionChanged(Object^ Sender, EventArgs^ E);
	void										TextField_ScrollOffsetChanged(Object^ Sender, EventArgs^ E);

	void										TextField_KeyUp(Object^ Sender, System::Windows::Input::KeyEventArgs^ E);
	void										TextField_KeyDown(Object^ Sender, System::Windows::Input::KeyEventArgs^ E);
	void										TextField_MouseDown(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E);
	void										TextField_MouseWheel(Object^ Sender, System::Windows::Input::MouseWheelEventArgs^ E);

	void										TextField_MouseHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
	void										TextField_MouseHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);

	void										TextField_SelectionChanged(Object^ Sender, EventArgs^ E);
	void										TextField_LostFocus(Object^ Sender, System::Windows::RoutedEventArgs^ E);

	void										TextField_MiddleMouseScrollMove(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
	void										TextField_MiddleMouseScrollDown(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E);

	void										MiddleMouseScrollTimer_Tick(Object^ Sender, EventArgs^ E);
	void										FoldingTimer_Tick(Object^ Sender, EventArgs^ E);
	void										ScrollBarSyncTimer_Tick(Object^ Sender, EventArgs^ E);

	void										ExternalScrollBar_ValueChanged(Object^ Sender, EventArgs^ E);
	void										SetTextAnimation_Completed(Object^ Sender, EventArgs^ E);

	String^										GetTokenAtIndex(int Index, bool SelectText, int% StartIndexOut, int% EndIndexOut);
	String^										GetTextAtLocation(Point Location, bool SelectText);		// line breaks need to be replaced by the caller
	String^										GetTextAtLocation(int Index, bool SelectText);
	array<String^>^								GetTextAtLocation(int Index);							// gets three of the closest tokens surrounding the offset

	void										SetPreventTextChangedFlag(PreventTextChangeFlagState State) { PreventTextChangedEventFlag = State; }
	void										HandleKeyEventForKey(System::Windows::Input::Key Key) { KeyToPreventHandling = Key; }

	void										HandleTextChangeEvent();
	void										GotoLine(int Line);						// line numbers start at 1

	void										RefreshUI() { TextField->TextArea->TextView->Redraw(); }
	UInt32										PerformReplaceOnSegment(IScriptTextEditor::FindReplaceOperation Operation, AvalonEdit::Document::DocumentLine^ Line, String^ Query, String^ Replacement, IScriptTextEditor::FindReplaceOutput^ Output);
	void										StartMiddleMouseScroll(System::Windows::Input::MouseButtonEventArgs^ E);
	void										StopMiddleMouseScroll();

	void										UpdateCodeFoldings();
	void										SynchronizeExternalScrollBars();

	void										MoveTextSegment(AvalonEdit::Document::ISegment^ Segment, MoveSegmentDirection Direction);

	RTBitmap^									RenderFrameworkElement(System::Windows::FrameworkElement^ Element);
	void										ClearFindResultIndicators(void);
	void										Destroy();

	static double								SetTextFadeAnimationDuration = 0.1;		// in seconds
public:
	~AvalonEditTextEditor()
	{
		Destroy();
	}

	// interface methods
	virtual void								SetFont(Font^ FontObject);
	virtual void								SetTabCharacterSize(int PixelWidth);	// AvalonEdit uses character lengths
	virtual void								SetContextMenu(ContextMenuStrip^% Strip);

	virtual void								AddControl(Control^ ControlObject);

	virtual String^								GetText(void);
	virtual UInt32								GetTextLength(void);
	virtual void								SetText(String^ Text, bool PreventTextChangedEventHandling);
	virtual void								InsertText(String^ Text, int Index);

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
	virtual array<String^>^						GetTokensAtMouseLocation();		// gets three of the closest tokens surrounding the mouse loc

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

	virtual UInt32								FindReplace(IScriptTextEditor::FindReplaceOperation Operation, String^ Query, String^ Replacement, IScriptTextEditor::FindReplaceOutput^ Output);
	virtual void								ToggleComment(int StartIndex);
	virtual void								UpdateIntelliSenseLocalDatabase(void);

	virtual Control^							GetContainer() { return Container; }
	virtual void								ScrollToLine(String^ LineNumber);
	virtual Point								PointToScreen(Point Location);

	virtual void								HighlightScriptError(int Line);
	virtual void								ClearScriptErrorHighlights(void);
	virtual void								SetEnabledState(bool State);

	virtual void								OnGotFocus(void);
	virtual void								OnLostFocus(void);
	virtual void								OnPositionSizeChange(void);

	AvalonEditTextEditor(Font^ Font, Object^% Parent);
};