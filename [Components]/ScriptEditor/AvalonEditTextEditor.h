#pragma once

#include "ScriptTextEditorInterface.h"
#include "AvalonEditXSHD.h"
#include "AvalonEditComponents.h"
#include "IntelliSense\IntelliSenseInterface.h"

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		namespace AvalonEditor
		{
			using namespace ICSharpCode;
			using namespace System::Windows::Forms::Integration;

			typedef System::Windows::Media::Imaging::RenderTargetBitmap		RTBitmap;

			ref class AvalonEditTextEditor : public IScriptTextEditor
			{
				static AvalonEditXSHDManager^						SyntaxHighlightingManager = gcnew AvalonEditXSHDManager();
			protected:
				static enum class									PreventTextChangeFlagState
				{
					e_Disabled = 0,
					e_AutoReset,
					e_ManualReset
				};

				static enum class									MoveSegmentDirection
				{
					e_Up = 0,
					e_Down
				};

				Panel^												Container;
				ElementHost^										WPFHost;
				System::Windows::Controls::DockPanel^				TextFieldPanel;
				AvalonEdit::TextEditor^								TextField;
				System::Windows::Shapes::Rectangle^					AnimationPrimitive;

				AvalonEditScriptErrorBGColorizer^					ErrorColorizer;
				AvalonEditFindReplaceBGColorizer^					FindReplaceColorizer;
				AvalonEdit::Folding::FoldingManager^				CodeFoldingManager;
				AvalonEditObScriptCodeFoldingStrategy^				CodeFoldingStrategy;
				AvalonEditBraceHighlightingBGColorizer^				BraceColorizer;

				bool												InitializingFlag;
				bool												ModifiedFlag;
				PreventTextChangeFlagState							PreventTextChangedEventFlag;
				System::Windows::Input::Key							KeyToPreventHandling;
				IntelliSense::IntelliSenseInterface^				IntelliSenseBox;
				int													LastKnownMouseClickOffset;
				System::Windows::Input::Key							LastKeyThatWentDown;

				System::Windows::Point								ScrollStartPoint;
				System::Windows::Vector								CurrentScrollOffset;
				bool												IsMiddleMouseScrolling;
				Timer^												MiddleMouseScrollTimer;

				bool												IsFocused;
				Timer^												FoldingTimer;

				Timer^												ScrollBarSyncTimer;
				VScrollBar^											ExternalVerticalScrollBar;
				HScrollBar^											ExternalHorizontalScrollBar;
				bool												SynchronizingExternalScrollBars;
				bool												SynchronizingInternalScrollBars;

				bool												SetTextAnimating;
				System::Windows::Media::Animation::DoubleAnimation^	SetTextPrologAnimationCache;
				UInt32												ParentWorkspaceIndex;

				Timer^												LocalVarsDatabaseUpdateTimer;
				bool												TextFieldInUpdateFlag;

				EventHandler^										TextFieldTextChangedHandler;
				EventHandler^										TextFieldCaretPositionChangedHandler;
				EventHandler^										TextFieldScrollOffsetChangedHandler;
				System::Windows::Input::KeyEventHandler^			TextFieldKeyUpHandler;
				System::Windows::Input::KeyEventHandler^			TextFieldKeyDownHandler;
				System::Windows::Input::MouseButtonEventHandler^	TextFieldMouseDownHandler;
				System::Windows::Input::MouseWheelEventHandler^		TextFieldMouseWheelHandler;
				System::Windows::Input::MouseEventHandler^			TextFieldMouseHoverHandler;
				System::Windows::Input::MouseEventHandler^			TextFieldMouseHoverStoppedHandler;
				EventHandler^										TextFieldSelectionChangedHandler;
				System::Windows::RoutedEventHandler^				TextFieldLostFocusHandler;
				System::Windows::Input::MouseEventHandler^			TextFieldMiddleMouseScrollMoveHandler;
				System::Windows::Input::MouseButtonEventHandler^	TextFieldMiddleMouseScrollDownHandler;
				EventHandler^										MiddleMouseScrollTimerTickHandler;
				EventHandler^										FoldingTimerTickHandler;
				EventHandler^										ScrollBarSyncTimerTickHandler;
				EventHandler^										LocalVarsDatabaseUpdateTimerTickHandler;
				EventHandler^										ExternalScrollBarValueChangedHandler;
				EventHandler^										SetTextAnimationCompletedHandler;
				EventHandler^										ScriptEditorPreferencesSavedHandler;
				System::EventHandler<AvalonEdit::Editing::TextEventArgs^>^
																	TextFieldTextCopiedHandler;

				virtual void								OnScriptModified(ScriptModifiedEventArgs^ E);
				virtual void								OnKeyDown(KeyEventArgs^ E);

				void										TextField_TextChanged(Object^ Sender, EventArgs^ E);
				void										TextField_CaretPositionChanged(Object^ Sender, EventArgs^ E);
				void										TextField_ScrollOffsetChanged(Object^ Sender, EventArgs^ E);
				void										TextField_TextCopied(Object^ Sender, AvalonEdit::Editing::TextEventArgs^ E);

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
				void										LocalVarsDatabaseUpdateTimer_Tick(Object^ Sender, EventArgs^ E);

				void										ExternalScrollBar_ValueChanged(Object^ Sender, EventArgs^ E);
				void										SetTextAnimation_Completed(Object^ Sender, EventArgs^ E);
				void										ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);

				String^										GetTokenAtIndex(int Index, bool SelectText, int% StartIndexOut, int% EndIndexOut);
				String^										GetTextAtLocation(Point Location, bool SelectText);		// line breaks need to be replaced by the caller
				String^										GetTextAtLocation(int Index, bool SelectText);
				array<String^>^								GetTextAtLocation(int Index);							// gets three of the closest tokens surrounding the offset

				void										SetPreventTextChangedFlag(PreventTextChangeFlagState State) { PreventTextChangedEventFlag = State; }
				void										HandleKeyEventForKey(System::Windows::Input::Key Key) { KeyToPreventHandling = Key; }

				void										HandleTextChangeEvent();
				void										GotoLine(int Line);										// line numbers start at 1

				void										RefreshBGColorizerLayer();
				void										RefreshTextView();
				int											PerformFindReplaceOperationOnSegment(System::Text::RegularExpressions::Regex^ ExpressionParser, IScriptTextEditor::FindReplaceOperation Operation, AvalonEdit::Document::DocumentLine^ Line, String^ Replacement, IScriptTextEditor::FindReplaceOutput^ Output, UInt32 Options);
				void										StartMiddleMouseScroll(System::Windows::Input::MouseButtonEventArgs^ E);
				void										StopMiddleMouseScroll();

				void										UpdateCodeFoldings();
				void										SynchronizeExternalScrollBars();

				void										MoveTextSegment(AvalonEdit::Document::ISegment^ Segment, MoveSegmentDirection Direction);

				RTBitmap^									RenderFrameworkElement(System::Windows::FrameworkElement^ Element);
				void										ClearFindResultIndicators(void);
				void										SearchBracesForHighlighting(int CaretPos);
				AvalonEditHighlightingDefinition^			CreateSyntaxHighlightDefinitions(void);
				virtual void								Destroy();

				static double								SetTextFadeAnimationDuration = 0.15;		// in seconds

			public:
				virtual ~AvalonEditTextEditor();

				// interface events
				virtual event ScriptModifiedEventHandler^	ScriptModified;
				virtual event KeyEventHandler^				KeyDown;

				// interface methods
				virtual void								SetFont(Font^ FontObject);
				virtual void								SetTabCharacterSize(int PixelWidth);	// AvalonEdit uses character lengths
				virtual void								SetContextMenu(ContextMenuStrip^% Strip);

				virtual void								AddControl(Control^ ControlObject);

				virtual String^								GetText(void);
				virtual UInt32								GetTextLength(void);
				virtual void								SetText(String^ Text, bool PreventTextChangedEventHandling, bool ResetUndoStack);
				virtual void								InsertText(String^ Text, int Index, bool PreventTextChangedEventHandling);

				virtual String^								GetSelectedText(void);
				virtual void								SetSelectedText(String^ Text, bool PreventTextChangedEventHandling);

				virtual void								SetSelectionStart(int Index);
				virtual void								SetSelectionLength(int Length);

				virtual int									GetCharIndexFromPosition(Point Position);
				virtual Point								GetPositionFromCharIndex(int Index);
				virtual Point								GetAbsolutePositionFromCharIndex(int Index);
				virtual int									GetLineNumberFromCharIndex(int Index);
				virtual bool								GetCharIndexInsideCommentSegment(int Index);
				virtual int									GetCurrentLineNumber(void);

				virtual String^								GetTokenAtCaretPos();
				virtual void								SetTokenAtCaretPos(String^ Replacement);
				virtual String^								GetTokenAtMouseLocation();
				virtual array<String^>^						GetTokensAtMouseLocation();

				virtual int									GetCaretPos();
				virtual void								SetCaretPos(int Index);
				virtual void								ScrollToCaret();

				virtual IntPtr								GetHandle();

				virtual void								FocusTextArea();
				virtual void								LoadFileFromDisk(String^ Path);
				virtual void								SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^% DefaultName);

				virtual bool								GetModifiedStatus();
				virtual void								SetModifiedStatus(bool Modified);

				virtual bool								GetInitializingStatus();
				virtual void								SetInitializingStatus(bool Initializing);

				virtual int									GetLastKnownMouseClickOffset(void);

				virtual int									FindReplace(IScriptTextEditor::FindReplaceOperation Operation, String^ Query, String^ Replacement, IScriptTextEditor::FindReplaceOutput^ Output, UInt32 Options);
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

				virtual void										BeginUpdate(void);
				virtual void										EndUpdate(void);
				virtual UInt32										GetTotalLineCount(void);
				virtual IntelliSense::IntelliSenseInterface^		GetIntelliSenseInterface(void);

				AvalonEditTextEditor(Font^ Font, UInt32 ParentWorkspaceIndex);
			};
		}
	}
}