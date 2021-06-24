#pragma once

#include "AvalonEditSyntaxHighlighting.h"
#include "AvalonEditComponents.h"
#include "IScriptEditorModel.h"

namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


namespace avalonEdit
{


using namespace ICSharpCode;
using namespace System::Windows::Forms::Integration;
using namespace System::Threading::Tasks;

using RTBitmap = System::Windows::Media::Imaging::RenderTargetBitmap;
using AvalonEditTextEventHandler = System::EventHandler<AvalonEdit::Editing::TextEventArgs^>;

ref class AvalonEditTextEditor : public ITextEditor
{
	static const UInt32	kSetTextFadeAnimationDuration = 150;		// in ms

	static AvalonEditXSHDManager^ SyntaxHighlightingManager = gcnew AvalonEditXSHDManager();
protected:
	static enum class eIntelliSenseTextChangeEventHandling
	{
		Propagate,
		SuppressOnce,
		SuppressAlways
	};

	static enum class eMoveSegmentDirection
	{
		Up,
		Down
	};

	static enum class eCommentOperation
	{
		Add,
		Remove,
		Toggle,
	};

	Panel^												WinFormsContainer;
	ElementHost^										WPFHost;
	System::Windows::Controls::DockPanel^				TextFieldPanel;
	AvalonEdit::TextEditor^								TextField;
	System::Windows::Shapes::Rectangle^					AnimationPrimitive;

	AvalonEdit::Folding::FoldingManager^				CodeFoldingManager;
	ObScriptCodeFoldingStrategy^						CodeFoldingStrategy;

	bool												InitializingFlag;
	bool												ModifiedFlag;
	eIntelliSenseTextChangeEventHandling				IntelliSenseTextChangeEventHandlingMode;
	System::Windows::Input::Key							KeyToPreventHandling;
	int													LastKnownMouseClickOffset;
	System::Windows::Input::Key							LastKeyThatWentDown;
	int													PreviousLineBuffer;

	System::Windows::Point								MiddleMouseScrollStartPoint;
	System::Windows::Vector								MiddleMouseCurrentScrollOffset;
	bool												IsMiddleMouseScrolling;
	Timer^												MiddleMouseScrollTimer;
	int													OffsetAtCurrentMousePos;
	int													LastMouseHoverOffset;

	bool												IsFocused;

	Timer^												ScrollBarSyncTimer;
	VScrollBar^											ExternalVerticalScrollBar;
	HScrollBar^											ExternalHorizontalScrollBar;
	bool												SynchronizingExternalScrollBars;
	bool												SynchronizingInternalScrollBars;
	System::Windows::Vector								PreviousScrollOffsetBuffer;

	bool												SetTextAnimating;
	System::Windows::Media::Animation::DoubleAnimation^	SetTextPrologAnimationCache;

	bool												TextFieldInUpdateFlag;
	bool												TextFieldDisplayingStaticText;

	model::IScriptDocument^								ParentScriptDocument;
	LineTrackingManager^								LineTracker;
	DefaultIconMargin^									IconBarMargin;
	StructureVisualizerRenderer^						StructureVisualizer;
	AvalonEdit::Search::SearchPanel^					InlineSearchPanel;
	obScriptParsing::AnalysisData^						SemanticAnalysisCache;

	EventHandler^										TextFieldTextChangedHandler;
	EventHandler^										TextFieldCaretPositionChangedHandler;
	EventHandler^										TextFieldScrollOffsetChangedHandler;
	System::Windows::Input::KeyEventHandler^			TextFieldKeyUpHandler;
	System::Windows::Input::KeyEventHandler^			TextFieldKeyDownHandler;
	System::Windows::Input::MouseButtonEventHandler^	TextFieldMouseDownHandler;
	System::Windows::Input::MouseButtonEventHandler^	TextFieldMouseUpHandler;
	System::Windows::Input::MouseWheelEventHandler^		TextFieldMouseWheelHandler;
	System::Windows::Input::MouseEventHandler^			TextFieldMouseHoverHandler;
	System::Windows::Input::MouseEventHandler^			TextFieldMouseHoverStoppedHandler;
	System::Windows::Input::MouseEventHandler^			TextFieldMouseMoveHandler;
	EventHandler^										TextFieldSelectionChangedHandler;
	System::Windows::RoutedEventHandler^				TextFieldLostFocusHandler;
	System::Windows::Input::MouseEventHandler^			TextFieldMiddleMouseScrollMoveHandler;
	System::Windows::Input::MouseButtonEventHandler^	TextFieldMiddleMouseScrollDownHandler;
	EventHandler^										MiddleMouseScrollTimerTickHandler;
	EventHandler^										ScrollBarSyncTimerTickHandler;
	EventHandler^										SemanticAnalysisTimerTickHandler;
	EventHandler^										ExternalScrollBarValueChangedHandler;
	EventHandler^										SetTextAnimationCompletedHandler;
	EventHandler^										ScriptEditorPreferencesSavedHandler;
	AvalonEditTextEventHandler^							TextFieldTextCopiedHandler;
	EventHandler<VisualLineConstructionStartEventArgs^>^
														TextFieldVisualLineConstructionStartingHandler;
	EventHandler<AvalonEdit::Search::SearchOptionsChangedEventArgs^>^
														SearchPanelSearchOptionsChangedHandler;

	model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler^ BackgroundAnalyzerAnalysisCompleteHandler;


	bool	RaiseIntelliSenseInput(intellisense::IntelliSenseInputEventArgs::Event Type, System::Windows::Input::KeyEventArgs^ K, System::Windows::Input::MouseButtonEventArgs^ M);
	void	RaiseIntelliSenseInsightHover(intellisense::IntelliSenseInsightHoverEventArgs::Event Type, int Offset, Windows::Point Location);
	void	RaiseIntelliSenseContextChange(intellisense::IntelliSenseContextChangeEventArgs::Event Type);

	void	OnScriptModified(bool ModificationState);
	bool	OnKeyDown(System::Windows::Input::KeyEventArgs^ E);			// returns true if handled
	void	OnMouseClick(System::Windows::Input::MouseButtonEventArgs^ E);
	void	OnLineChanged();
	void	OnTextUpdated();
	void	OnLineAnchorInvalidated();

	void	TextField_TextChanged(Object^ Sender, EventArgs^ E);
	void	TextField_CaretPositionChanged(Object^ Sender, EventArgs^ E);
	void	TextField_ScrollOffsetChanged(Object^ Sender, EventArgs^ E);
	void	TextField_TextCopied(Object^ Sender, AvalonEdit::Editing::TextEventArgs^ E);

	void	TextField_KeyDown(Object^ Sender, System::Windows::Input::KeyEventArgs^ E);
	void	TextField_KeyUp(Object^ Sender, System::Windows::Input::KeyEventArgs^ E);
	void	TextField_MouseDown(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E);
	void	TextField_MouseUp(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E);
	void	TextField_MouseWheel(Object^ Sender, System::Windows::Input::MouseWheelEventArgs^ E);

	void	TextField_MouseHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
	void	TextField_MouseHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
	void	TextField_MouseMove(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);

	void	TextField_SelectionChanged(Object^ Sender, EventArgs^ E);
	void	TextField_LostFocus(Object^ Sender, System::Windows::RoutedEventArgs^ E);

	void	TextField_MiddleMouseScrollMove(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
	void	TextField_MiddleMouseScrollDown(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E);

	void	TextField_VisualLineConstructionStarting(Object^ Sender, VisualLineConstructionStartEventArgs^ E);
	void	SearchPanel_SearchOptionsChanged(Object^ Sender, AvalonEdit::Search::SearchOptionsChangedEventArgs^ E);

	void	MiddleMouseScrollTimer_Tick(Object^ Sender, EventArgs^ E);
	void	ScrollBarSyncTimer_Tick(Object^ Sender, EventArgs^ E);

	void	ExternalScrollBar_ValueChanged(Object^ Sender, EventArgs^ E);
	void	SetTextAnimation_Completed(Object^ Sender, EventArgs^ E);
	void	ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);
	void	BackgroundAnalysis_AnalysisComplete(Object^ Sender, model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs^ E);

	String^				GetTokenAtIndex(int Index, bool SelectText,
										int% OutStartIndex, int% OutEndIndex,
										Char% OutStartDelimiter, Char% OutEndDelimiter);
	String^				GetTokenAtLocation(Point Location, bool SelectText);		// line breaks need to be replaced by the caller
	String^				GetTokenAtLocation(int Index, bool SelectText);
	array<String^>^		GetTokenAtLocation(int Index);								// gets three of the closest tokens surrounding the offset
	array<String^>^		GetTokenAtLocation(int Index, array<Tuple<Char, Char>^>^% OutDelimiters);	// same as above; also returns the leading and trailing delimiters of each token
	Char				GetDelimiterAtLocation(int Index);
	bool				GetCharIndexInsideStringSegment(int Index);
	void				SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling State);
	void				HandleKeyEventForKey(System::Windows::Input::Key Key);
	void				HandleTextChangeEvent();
	void				GotoLine(int Line);											// line numbers start at 1
	void				RefreshTextView();
	int					PerformFindReplaceOperationOnSegment(System::Text::RegularExpressions::Regex^ ExpressionParser,
															 eFindReplaceOperation Operation,
															 AvalonEdit::Document::DocumentLine^ Line,
															 String^ Replacement,
															 eFindReplaceOptions Options);
	void				StartMiddleMouseScroll(System::Windows::Input::MouseButtonEventArgs^ E);
	void				StopMiddleMouseScroll();
	void				UpdateCodeFoldings();
	void				UpdateSyntaxHighlighting(bool Regenerate);
	void				SynchronizeExternalScrollBars();
	void				ResetExternalScrollBars();
	void				MoveTextSegment(AvalonEdit::Document::ISegment^ Segment, eMoveSegmentDirection Direction);
	RTBitmap^			RenderFrameworkElement(System::Windows::FrameworkElement^ Element);
	void				SearchBracesForHighlighting(int CaretPos);
	AvalonEditHighlightingDefinition^ CreateSyntaxHighlightDefinitions(bool UpdateStableDefs);
	String^				SanitizeUnicodeString(String^ In);							// removes unsupported characters
	void				SetFont(Font^ FontObject);
	void				SetTabCharacterSize(int PixelWidth);						// AvalonEdit uses character lengths
	String^				GetTextAtLine(int LineNumber);
	UInt32				GetTextLength(void);
	void				InsertText(String^ Text, int Index, bool PreventTextChangedEventHandling);			// performs bounds check
	void				SetSelectionStart(int Index);
	void				SetSelectionLength(int Length);
	bool				GetInSelection(int Index);
	int					GetLineNumberFromCharIndex(int Index);
	bool				GetCharIndexInsideCommentSegment(int Index);
	String^				GetTokenAtMouseLocation();
	array<String^>^		GetTokensAtMouseLocation();									// gets three of the closest tokens surrounding the mouse loc
	int					GetLastKnownMouseClickOffset(void);
	bool				GetLineVisible(UInt32 LineNumber, bool CheckVisualLine);	// inside the text field's viewable area
	UInt32				GetFirstVisibleLine();
	Point				PointToScreen(Point Location);
	void				PerformCommentOperationOnSingleLine(int Line, eCommentOperation Operation);
	void				PerformCommentOperationOnSelection(eCommentOperation Operation);
	void				ToggleSearchPanel(bool State);
	String^				GetCurrentLineText(bool ClipAtCaretPos);
	int					GetCharIndexFromPosition(Point Position);
	Point				GetPositionFromCharIndex(int Index, bool Absolute);
	void				FadeOutCurrentTextView();
public:
	AvalonEditTextEditor(model::IScriptDocument^ ParentScriptDocument);
	~AvalonEditTextEditor();

	virtual event intellisense::IntelliSenseInputEventHandler^			IntelliSenseInput;
	virtual event intellisense::IntelliSenseInsightHoverEventHandler^	IntelliSenseInsightHover;
	virtual event intellisense::IntelliSenseContextChangeEventHandler^	IntelliSenseContextChange;

	virtual event TextEditorScriptModifiedEventHandler^	ScriptModified;
	virtual event KeyEventHandler^						KeyDown;
	virtual event TextEditorMouseClickEventHandler^		MouseClick;
	virtual event EventHandler^							LineChanged;
	virtual event EventHandler^							TextUpdated;
	virtual event EventHandler^							LineAnchorInvalidated;

	ImplPropertyGetOnly(Control^, Container, WinFormsContainer);
	ImplPropertyGetOnly(IntPtr, WindowHandle, WinFormsContainer->Handle);
	ImplPropertySimple(bool, Enabled, WPFHost->Enabled);
	ImplPropertyGetOnly(int, CurrentLine, TextField->TextArea->Caret->Line);
	ImplPropertyGetOnly(int, LineCount, TextField->Document->LineCount);
	property int Caret
	{
		virtual int get() { return TextField->TextArea->Caret->Offset; }
		virtual void set(int Index)
		{
			TextField->SelectionLength = 0;
			if (Index > GetTextLength())
				Index = GetTextLength() - 1;

			if (Index > -1)
				TextField->TextArea->Caret->Offset = Index;
			else
				TextField->TextArea->Caret->Offset = 0;

			ScrollToCaret();
		}
	}
	property bool Modified
	{
		virtual bool get() { return ModifiedFlag; }
		virtual void set(bool State)
		{
			ModifiedFlag = State;
			OnScriptModified(Modified);
		}
	}
	property UInt32	FirstVisibleLine
	{
		UInt32 get() { return GetFirstVisibleLine(); }
	}
	property obScriptParsing::AnalysisData^ SemanticAnalysisData
	{
		obScriptParsing::AnalysisData^ get() { return SemanticAnalysisCache; }
	}
	ImplPropertyGetOnly(bool, DisplayingStaticText, TextFieldDisplayingStaticText);

	virtual void	Bind();
	virtual void	Unbind();

	virtual String^ GetText();
	virtual String^ GetText(UInt32 LineNumber);
	virtual void SetText(String^ Text, bool ResetUndoStack);
	virtual String^ GetSelectedText(void);
	virtual void SetSelectedText(String^ Text);
	virtual String^ GetTokenAtCharIndex(int Offset);
	virtual String^ GetTokenAtCaretPos();
	virtual void SetTokenAtCaretPos(String^ Replacement);
	virtual void ScrollToCaret();
	virtual void ScrollToLine(UInt32 LineNumber);
	virtual void FocusTextArea();

	virtual FindReplaceResult^ FindReplace(eFindReplaceOperation Operation, String^ Query, String^ Replacement, eFindReplaceOptions Options);

	virtual void BeginUpdate(void);
	virtual void EndUpdate(bool FlagModification);

	virtual UInt32 GetIndentLevel(UInt32 LineNumber);
	virtual void InsertVariable(String^ VariableName, obScriptParsing::Variable::DataType VariableType);

	virtual void InitializeState(String^ ScriptText, int CaretPosition);
	virtual ILineAnchor^ CreateLineAnchor(UInt32 Line);

	virtual void InvokeDefaultCopy();
	virtual void InvokeDefaultPaste();
	virtual void CommentLine(UInt32 Line);
	virtual void CommentSelection();
	virtual void UncommentLine(UInt32 Line);
	virtual void UncommentSelection();

	virtual void BeginDisplayingStaticText(String^ TextToDisplay);
	virtual void EndDisplayingStaticText();
};


} // namespace avalonEdit


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse