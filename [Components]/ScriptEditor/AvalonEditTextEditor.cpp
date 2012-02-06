#include "AvalonEditTextEditor.h"
#include "ScriptParser.h"
#include "Globals.h"
#include "ScriptEditorPreferences.h"

using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;
using namespace System::Text::RegularExpressions;

namespace ConstructionSetExtender
{
	using namespace IntelliSense;

	namespace TextEditors
	{
		namespace AvalonEditor
		{
			ref class BracketSearchData
			{
				Char									Symbol;
				int										StartOffset;
			public:
				property int							EndOffset;
				property bool							Mismatching;

				static String^							ValidOpeningBrackets = "([{";
				static String^							ValidClosingBrackets = ")]}";

				static enum class						BracketType
				{
					e_Invalid = 0,
					e_Curved,
					e_Square,
					e_Squiggly
				};
				static enum class						BracketKind
				{
					e_Invalid = 0,
					e_Opening,
					e_Closing
				};

				BracketSearchData(Char Symbol, int StartOffset) : Symbol(Symbol), StartOffset(StartOffset)
				{
					EndOffset = -1;
					Mismatching = false;
				}

				BracketType								GetType()
				{
					switch (Symbol)
					{
					case '(':
					case ')':
						return BracketType::e_Curved;
					case '[':
					case ']':
						return BracketType::e_Square;
					case '{':
					case '}':
						return BracketType::e_Squiggly;
					default:
						return BracketType::e_Invalid;
					}
				}

				BracketKind								GetKind()
				{
					switch (Symbol)
					{
					case '(':
					case '[':
					case '{':
						return BracketKind::e_Opening;
					case ')':
					case ']':
					case '}':
						return BracketKind::e_Closing;
					default:
						return BracketKind::e_Invalid;
					}
				}
				int										GetStartOffset() { return StartOffset; }
			};

#pragma region Interface Methods
			void AvalonEditTextEditor::SetFont(Font^ FontObject)
			{
				TextField->FontFamily = gcnew Windows::Media::FontFamily(FontObject->FontFamily->Name);
				TextField->FontSize = FontObject->Size;
				if (FontObject->Style == Drawing::FontStyle::Bold)
					TextField->FontWeight = Windows::FontWeights::Bold;
			}

			void AvalonEditTextEditor::SetTabCharacterSize(int PixelWidth)
			{
				TextField->Options->IndentationSize = PixelWidth;
			}

			void AvalonEditTextEditor::SetContextMenu(ContextMenuStrip^% Strip)
			{
				Container->ContextMenuStrip = Strip;
			}

			void AvalonEditTextEditor::AddControl(Control^ ControlObject)
			{
				Container->Controls->Add(ControlObject);
			}

			String^ AvalonEditTextEditor::GetText(void)
			{
				return TextField->Text;
			}

			UInt32 AvalonEditTextEditor::GetTextLength(void)
			{
				return TextField->Text->Length;
			}

			void AvalonEditTextEditor::SetText(String^ Text, bool PreventTextChangedEventHandling, bool ResetUndoStack)
			{
				if (PreventTextChangedEventHandling)
					SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);

				if (SetTextAnimating)
				{
					if (ResetUndoStack)
						TextField->Text = Text;
					else
					{
						SetSelectionStart(0);
						SetSelectionLength(GetTextLength());
						SetSelectedText(Text, false);
						SetSelectionLength(0);
					}
					UpdateCodeFoldings();
				}
				else
				{
					SetTextAnimating = true;

					TextFieldPanel->Children->Add(AnimationPrimitive);

					AnimationPrimitive->Fill =  gcnew System::Windows::Media::VisualBrush(TextField);
					AnimationPrimitive->Height = TextField->ActualHeight;
					AnimationPrimitive->Width = TextField->ActualWidth;

					TextFieldPanel->Children->Remove(TextField);

					System::Windows::Media::Animation::DoubleAnimation^ FadeOutAnimation = gcnew System::Windows::Media::Animation::DoubleAnimation(1.0,
						0.0,
						System::Windows::Duration(System::TimeSpan::FromSeconds(SetTextFadeAnimationDuration)),
						System::Windows::Media::Animation::FillBehavior::Stop);
					SetTextPrologAnimationCache = FadeOutAnimation;

					FadeOutAnimation->Completed += SetTextAnimationCompletedHandler;
					System::Windows::Media::Animation::Storyboard^ FadeOutStoryBoard = gcnew System::Windows::Media::Animation::Storyboard();
					FadeOutStoryBoard->Children->Add(FadeOutAnimation);
					FadeOutStoryBoard->SetTargetName(FadeOutAnimation, AnimationPrimitive->Name);
					FadeOutStoryBoard->SetTargetProperty(FadeOutAnimation, gcnew System::Windows::PropertyPath(AnimationPrimitive->OpacityProperty));
					FadeOutStoryBoard->Begin(TextFieldPanel);

					if (ResetUndoStack)
						TextField->Text = Text;
					else
					{
						SetSelectionStart(0);
						SetSelectionLength(GetTextLength());
						SetSelectedText(Text, false);
						SetSelectionLength(0);
					}
					UpdateCodeFoldings();
				}
			}

			void AvalonEditTextEditor::InsertText( String^ Text, int Index, bool PreventTextChangedEventHandling )
			{
				if (Index > GetTextLength())
					Index = GetTextLength();

				if (PreventTextChangedEventHandling)
					SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);

				TextField->Document->Insert(Index, Text);
			}

			String^ AvalonEditTextEditor::GetSelectedText(void)
			{
				return TextField->SelectedText;
			}

			void AvalonEditTextEditor::SetSelectedText(String^ Text, bool PreventTextChangedEventHandling)
			{
				if (PreventTextChangedEventHandling)
					SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);

				TextField->SelectedText = Text;
			}

			void AvalonEditTextEditor::SetSelectionStart(int Index)
			{
				TextField->SelectionStart = Index;
			}

			void AvalonEditTextEditor::SetSelectionLength(int Length)
			{
				TextField->SelectionLength = Length;
			}

			int AvalonEditTextEditor::GetCharIndexFromPosition(Point Position)
			{
				Nullable<AvalonEdit::TextViewPosition> TextPos = TextField->TextArea->TextView->GetPosition(Windows::Point(Position.X, Position.Y));
				if (TextPos.HasValue)
					return TextField->Document->GetOffset(TextPos.Value.Line, TextPos.Value.Column);
				else
					return GetTextLength() + 1;
			}

			Point AvalonEditTextEditor::GetPositionFromCharIndex(int Index)
			{
				AvalonEdit::Document::TextLocation Location = TextField->Document->GetLocation(Index);
				Windows::Point Result = TextField->TextArea->TextView->GetVisualPosition(AvalonEdit::TextViewPosition(Location), AvalonEdit::Rendering::VisualYPosition::TextTop) - TextField->TextArea->TextView->ScrollOffset;

				return Point(Result.X, Result.Y);
			}

			Point AvalonEditTextEditor::GetAbsolutePositionFromCharIndex( int Index )
			{
				Point Result = GetPositionFromCharIndex(Index);

				for each (System::Windows::UIElement^ Itr in TextField->TextArea->LeftMargins)
					Result.X += (dynamic_cast<System::Windows::FrameworkElement^>(Itr))->ActualWidth;

				return Result;
			}

			int AvalonEditTextEditor::GetLineNumberFromCharIndex(int Index)
			{
				if (Index >= TextField->Text->Length)
					Index = TextField->Text->Length - 1;
				return TextField->Document->GetLocation(Index).Line - 1;
			}

			bool AvalonEditTextEditor::GetCharIndexInsideCommentSegment(int Index)
			{
				bool Result = true;

				if (Index < TextField->Text->Length)
				{
					AvalonEdit::Document::DocumentLine^ Line = TextField->Document->GetLineByOffset(Index);
					ScriptParser^ LocalParser = gcnew ScriptParser();
					LocalParser->Tokenize(TextField->Document->GetText(Line), false);
					if (LocalParser->GetCommentTokenIndex(LocalParser->GetTokenIndex(GetTextAtLocation(Index, false))) == -1)
						Result = false;
				}

				return Result;
			}

			int AvalonEditTextEditor::GetCurrentLineNumber(void)
			{
				return TextField->TextArea->Caret->Line - 1;
			}

			String^ AvalonEditTextEditor::GetTokenAtCaretPos()
			{
				return GetTextAtLocation(GetCaretPos() - 1, false)->Replace("\n", "");
			}

			void AvalonEditTextEditor::SetTokenAtCaretPos(String^ Replacement)
			{
				GetTextAtLocation(GetCaretPos() - 1, true);
				TextField->SelectedText	= Replacement;
				SetCaretPos(TextField->SelectionStart + TextField->SelectionLength);
			}

			String^ AvalonEditTextEditor::GetTokenAtMouseLocation()
			{
				return GetTextAtLocation(LastKnownMouseClickLocation, false)->Replace("\n", "");
			}

			array<String^>^ AvalonEditTextEditor::GetTokensAtMouseLocation()
			{
				return GetTextAtLocation(GetCharIndexFromPosition(LastKnownMouseClickLocation));
			}

			int AvalonEditTextEditor::GetCaretPos()
			{
				return TextField->TextArea->Caret->Offset;
			}

			void AvalonEditTextEditor::SetCaretPos(int Index)
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

			void AvalonEditTextEditor::ScrollToCaret()
			{
				TextField->TextArea->Caret->BringCaretToView();
			}

			IntPtr AvalonEditTextEditor::GetHandle()
			{
				return Container->Handle;
			}

			void AvalonEditTextEditor::FocusTextArea()
			{
				TextField->Focus();
			}

			void AvalonEditTextEditor::LoadFileFromDisk(String^ Path)
			{
				try
				{
					SetPreventTextChangedFlag(PreventTextChangeFlagState::e_ManualReset);
					StreamReader^ Reader = gcnew StreamReader(Path);
					String^ FileText = Reader->ReadToEnd();
					SetText(FileText, false, false);
					Reader->Close();
					SetPreventTextChangedFlag(PreventTextChangeFlagState::e_Disabled);
				}
				catch (Exception^ E)
				{
					DebugPrint("Error encountered when opening file for read operation!\n\tError Message: " + E->Message);
				}
			}

			void AvalonEditTextEditor::SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^% DefaultName)
			{
				if (PathIncludesFileName == false)
					Path += "\\" + DefaultName + ".txt";

				try
				{
					TextField->Save(Path);
				}
				catch (Exception^ E)
				{
					DebugPrint("Error encountered when opening file for write operation!\n\tError Message: " + E->Message);
				}
			}

			bool AvalonEditTextEditor::GetModifiedStatus()
			{
				return ModifiedFlag;
			}

			void AvalonEditTextEditor::SetModifiedStatus(bool Modified)
			{
				ModifiedFlag = Modified;

				switch (Modified)
				{
				case true:
					ErrorColorizer->ClearLines();
					if (TextFieldInUpdateFlag == false)
						ClearFindResultIndicators();
					break;
				case false:
					break;
				}

				OnScriptModified(gcnew ScriptModifiedEventArgs(Modified));
			}

			bool AvalonEditTextEditor::GetInitializingStatus()
			{
				return InitializingFlag;
			}

			void AvalonEditTextEditor::SetInitializingStatus(bool Initializing)
			{
				InitializingFlag = Initializing;
			}

			Point AvalonEditTextEditor::GetLastKnownMouseClickLocation()
			{
				return LastKnownMouseClickLocation;
			}

			int AvalonEditTextEditor::FindReplace(IScriptTextEditor::FindReplaceOperation Operation, String^ Query, String^ Replacement, IScriptTextEditor::FindReplaceOutput^ Output, UInt32 Options)
			{
				int Hits = 0;

				if (Operation != IScriptTextEditor::FindReplaceOperation::e_CountMatches)
				{
					ClearFindResultIndicators();
					BeginUpdate();
				}

				try
				{
					String^ Pattern = "";

					if ((Options & (UInt32)IScriptTextEditor::FindReplaceOptions::e_RegEx))
						Pattern = Query;
					else
					{
						Pattern = System::Text::RegularExpressions::Regex::Escape(Query);
						if ((Options & (UInt32)IScriptTextEditor::FindReplaceOptions::e_MatchWholeWord))
							Pattern = "\\b" + Pattern + "\\b";
					}

					System::Text::RegularExpressions::Regex^ Parser = nullptr;
					if ((Options & (UInt32)IScriptTextEditor::FindReplaceOptions::e_CaseInsensitive))
						Parser = gcnew System::Text::RegularExpressions::Regex(Pattern, System::Text::RegularExpressions::RegexOptions::IgnoreCase|System::Text::RegularExpressions::RegexOptions::Singleline);
					else
						Parser = gcnew System::Text::RegularExpressions::Regex(Pattern, System::Text::RegularExpressions::RegexOptions::Singleline);

					AvalonEdit::Editing::Selection^ TextSelection = TextField->TextArea->Selection;
					if ((Options & (UInt32)IScriptTextEditor::FindReplaceOptions::e_InSelection))
					{
						if (TextSelection->IsEmpty == false)
						{
							AvalonEdit::Document::DocumentLine ^FirstLine = nullptr, ^LastLine = nullptr;

							for each (AvalonEdit::Document::ISegment^ Itr in TextSelection->Segments)
							{
								FirstLine = TextField->TextArea->Document->GetLineByOffset(Itr->Offset);
								LastLine = TextField->TextArea->Document->GetLineByOffset(Itr->EndOffset);

								for (AvalonEdit::Document::DocumentLine^ Itr = FirstLine; Itr != LastLine->NextLine && Itr != nullptr; Itr = Itr->NextLine)
								{
									int Matches = PerformFindReplaceOperationOnSegment(Parser, Operation, Itr, Replacement, Output, Options);
									Hits += Matches;
									if (Matches == -1)
									{
										Hits = -1;
										break;
									}
								}
							}
						}
					}
					else
					{
						for each (DocumentLine^ Line in TextField->Document->Lines)
						{
							int Matches = PerformFindReplaceOperationOnSegment(Parser, Operation, Line, Replacement, Output, Options);
							Hits += Matches;
							if (Matches == -1)
							{
								Hits = -1;
								break;
							}
						}
					}
				}
				catch (Exception^ E)
				{
					Hits = -1;
					DebugPrint("Couldn't perform find/replace operation!\n\tException: " + E->Message);
				}

				if (Operation != IScriptTextEditor::FindReplaceOperation::e_CountMatches)
				{
					SetSelectionLength(0);
					RefreshBGColorizerLayer();
					EndUpdate();
				}

				if (Hits == -1)
				{
					MessageBox::Show("An error was encountered while performing the find/replace operation. Please recheck your search and/or replacement strings.", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
				}

				return Hits;
			}

			void AvalonEditTextEditor::ToggleComment(int StartIndex)
			{
				SetPreventTextChangedFlag(PreventTextChangeFlagState::e_ManualReset);
				BeginUpdate();

				AvalonEdit::Editing::Selection^ TextSelection = TextField->TextArea->Selection;
				if (TextSelection->IsEmpty)
				{
					AvalonEdit::Document::DocumentLine^ Line = TextField->TextArea->Document->GetLineByOffset(StartIndex);
					if (Line != nullptr)
					{
						int FirstOffset = -1;
						for (int i = Line->Offset; i <= Line->EndOffset; i++)
						{
							char FirstChar = TextField->TextArea->Document->GetCharAt(i);
							if (AvalonEdit::Document::TextUtilities::GetCharacterClass(FirstChar) != AvalonEdit::Document::CharacterClass::Whitespace &&
								AvalonEdit::Document::TextUtilities::GetCharacterClass(FirstChar) != AvalonEdit::Document::CharacterClass::LineTerminator)
							{
								FirstOffset = i;
								break;
							}
						}

						if (FirstOffset != -1)
						{
							char FirstChar = TextField->TextArea->Document->GetCharAt(FirstOffset);
							if (FirstChar == ';')
								TextField->TextArea->Document->Replace(FirstOffset, 1, "");
							else
								TextField->TextArea->Document->Insert(FirstOffset, ";");
						}
					}
				}
				else
				{
					int Count = 0, ToggleType = 9;								// 0 - off, 1 - on
					AvalonEdit::Document::DocumentLine ^FirstLine = nullptr, ^LastLine = nullptr;

					for each (AvalonEdit::Document::ISegment^ Itr in TextSelection->Segments)
					{
						FirstLine = TextField->TextArea->Document->GetLineByOffset(Itr->Offset);
						LastLine = TextField->TextArea->Document->GetLineByOffset(Itr->EndOffset);

						for (AvalonEdit::Document::DocumentLine^ Itr = FirstLine; Itr != LastLine->NextLine && Itr != nullptr; Itr = Itr->NextLine)
						{
							int FirstOffset = -1;
							for (int i = Itr->Offset; i < TextField->Text->Length && i <= Itr->EndOffset; i++)
							{
								char FirstChar = TextField->TextArea->Document->GetCharAt(i);
								if (AvalonEdit::Document::TextUtilities::GetCharacterClass(FirstChar) != AvalonEdit::Document::CharacterClass::Whitespace &&
									AvalonEdit::Document::TextUtilities::GetCharacterClass(FirstChar) != AvalonEdit::Document::CharacterClass::LineTerminator)
								{
									FirstOffset = i;
									break;
								}
							}

							if (FirstOffset != -1)
							{
								char FirstChar = TextField->TextArea->Document->GetCharAt(FirstOffset);
								if (FirstChar == ';' && (!Count || !ToggleType))
								{
									if (!Count)		ToggleType = 0;

									AvalonEdit::Document::DocumentLine^ Line = TextField->TextArea->Document->GetLineByOffset(FirstOffset);
									TextField->TextArea->Document->Replace(Line->Offset, 1, "");
								}
								else if (FirstChar != ';' && (!Count || ToggleType))
								{
									if (!Count)		ToggleType = 1;

									AvalonEdit::Document::DocumentLine^ Line = TextField->TextArea->Document->GetLineByOffset(FirstOffset);
									TextField->TextArea->Document->Insert(Line->Offset, ";");
								}
							}

							Count++;
						}
					}
				}

				EndUpdate();
				SetPreventTextChangedFlag(PreventTextChangeFlagState::e_Disabled);
			}

			void AvalonEditTextEditor::UpdateIntelliSenseLocalDatabase(void)
			{
				IntelliSenseBox->UpdateLocalVariableDatabase();
			}

			void AvalonEditTextEditor::ScrollToLine(String^ LineNumber)
			{
				int LineNo = 0;
				try { LineNo = Int32::Parse(LineNumber); } catch (...) { return; }

				GotoLine(LineNo);
			}

			void AvalonEditTextEditor::OnGotFocus(void)
			{
				FocusTextArea();

				IsFocused = true;
				FoldingTimer->Start();
				ScrollBarSyncTimer->Start();
			}

			void AvalonEditTextEditor::HighlightScriptError(int Line)
			{
				ErrorColorizer->AddLine(Line);
				RefreshBGColorizerLayer();
			}

			void AvalonEditTextEditor::OnLostFocus( void )
			{
				IsFocused = false;
				FoldingTimer->Stop();
				ScrollBarSyncTimer->Stop();
				IntelliSenseBox->HideInterface();
			}

			void AvalonEditTextEditor::ClearScriptErrorHighlights(void)
			{
				ErrorColorizer->ClearLines();
			}

			Point AvalonEditTextEditor::PointToScreen(Point Location)
			{
				return Container->PointToScreen(Location);
			}

			void AvalonEditTextEditor::SetEnabledState(bool State)
			{
				WPFHost->Enabled = State;
			}

			void AvalonEditTextEditor::OnPositionSizeChange(void)
			{
				IntelliSenseBox->HideInterface();
			}

			void AvalonEditTextEditor::BeginUpdate( void )
			{
				TextFieldInUpdateFlag = true;
				TextField->Document->BeginUpdate();
			}

			void AvalonEditTextEditor::EndUpdate( void )
			{
				TextField->Document->EndUpdate();
				TextFieldInUpdateFlag = false;
			}

			UInt32 AvalonEditTextEditor::GetTotalLineCount( void )
			{
				return TextField->Document->LineCount;
			}
#pragma endregion

#pragma region Methods
			void AvalonEditTextEditor::Destroy()
			{
				TextField->Clear();
				MiddleMouseScrollTimer->Stop();
				FoldingTimer->Stop();
				ScrollBarSyncTimer->Stop();
				LocalVarsDatabaseUpdateTimer->Stop();
				CodeFoldingManager->Clear();
				AvalonEdit::Folding::FoldingManager::Uninstall(CodeFoldingManager);

				for each (AvalonEdit::Rendering::IBackgroundRenderer^ Itr in TextField->TextArea->TextView->BackgroundRenderers)
					delete Itr;
				TextField->TextArea->TextView->BackgroundRenderers->Clear();

				TextField->TextChanged -= TextFieldTextChangedHandler;
				TextField->TextArea->Caret->PositionChanged -= TextFieldCaretPositionChangedHandler;
				TextField->TextArea->SelectionChanged -= TextFieldSelectionChangedHandler;
				TextField->LostFocus -= TextFieldLostFocusHandler;
				TextField->TextArea->TextView->ScrollOffsetChanged -= TextFieldScrollOffsetChangedHandler;
				TextField->PreviewKeyUp -= TextFieldKeyUpHandler;
				TextField->PreviewKeyDown -= TextFieldKeyDownHandler;
				TextField->PreviewMouseDown -= TextFieldMouseDownHandler;
				TextField->PreviewMouseWheel -= TextFieldMouseWheelHandler;
				TextField->PreviewMouseHover -= TextFieldMouseHoverHandler;
				TextField->PreviewMouseHoverStopped -= TextFieldMouseHoverStoppedHandler;
				TextField->PreviewMouseMove -= TextFieldMiddleMouseScrollMoveHandler;
				TextField->PreviewMouseDown -= TextFieldMiddleMouseScrollDownHandler;
				MiddleMouseScrollTimer->Tick -= MiddleMouseScrollTimerTickHandler;
				FoldingTimer->Tick -= FoldingTimerTickHandler;
				ScrollBarSyncTimer->Tick -= ScrollBarSyncTimerTickHandler;
				ExternalVerticalScrollBar->ValueChanged -= ExternalScrollBarValueChangedHandler;
				ExternalHorizontalScrollBar->ValueChanged -= ExternalScrollBarValueChangedHandler;
				LocalVarsDatabaseUpdateTimer->Tick -= LocalVarsDatabaseUpdateTimerTickHandler;
				PREFERENCES->PreferencesSaved -= ScriptEditorPreferencesSavedHandler;

				TextFieldPanel->Children->Clear();
				WPFHost->Child = nullptr;
				Container->Controls->Clear();

				delete Container;
				delete WPFHost;
				delete TextFieldPanel;
				delete AnimationPrimitive;
				delete TextField->TextArea->TextView;
				delete TextField->TextArea;
				delete TextField;
				delete IntelliSenseBox;
				delete MiddleMouseScrollTimer;
				delete FoldingTimer;
				delete ErrorColorizer;
				delete FindReplaceColorizer;
				delete BraceColorizer;
				delete CodeFoldingManager;
				delete CodeFoldingStrategy;
				delete ScrollBarSyncTimer;
				delete LocalVarsDatabaseUpdateTimer;
				delete ExternalVerticalScrollBar;
				delete ExternalHorizontalScrollBar;

				IntelliSenseBox = nullptr;
				TextField->TextArea->IndentationStrategy = nullptr;
				TextField->SyntaxHighlighting = nullptr;
				TextField->Document = nullptr;
				CodeFoldingStrategy = nullptr;
				ErrorColorizer = nullptr;
				FindReplaceColorizer = nullptr;
				CodeFoldingManager = nullptr;
				BraceColorizer = nullptr;
			}

			int AvalonEditTextEditor::PerformFindReplaceOperationOnSegment(System::Text::RegularExpressions::Regex^ ExpressionParser, IScriptTextEditor::FindReplaceOperation Operation, AvalonEdit::Document::DocumentLine^ Line, String^ Replacement, IScriptTextEditor::FindReplaceOutput^ Output, UInt32 Options)
			{
				int Hits = 0, SearchStartOffset = 0;
				String^ CurrentLine = TextField->Document->GetText(Line);

				try
				{
					while (true)
					{
						System::Text::RegularExpressions::MatchCollection^ PatternMatches = ExpressionParser->Matches(CurrentLine, SearchStartOffset);

						if (PatternMatches->Count)
						{
							bool Restart = false;

							for each (System::Text::RegularExpressions::Match^ Itr in PatternMatches)
							{
								int Offset = Line->Offset + Itr->Index, Length = Itr->Length;
								Hits++;

								if (Operation == IScriptTextEditor::FindReplaceOperation::e_Replace)
								{
									TextField->Document->Replace(Offset, Length, Replacement);
									CurrentLine = TextField->Document->GetText(Line);
									FindReplaceColorizer->AddSegment(Offset, Replacement->Length);
									SearchStartOffset = Itr->Index + Replacement->Length;
									Restart = true;
									break;
								}
								else if (Operation == IScriptTextEditor::FindReplaceOperation::e_Find)
								{
									FindReplaceColorizer->AddSegment(Offset, Length);
								}
							}

							if (Restart == false)
								break;
						}
						else
							break;
					}
				}
				catch (Exception^ E)
				{
					Hits = -1;
					DebugPrint("Couldn't perform find/replace operation!\n\tException: " + E->Message);
				}

				if (Hits)
					Output(Line->LineNumber.ToString(), TextField->Document->GetText(Line));

				return Hits;
			}

			String^ AvalonEditTextEditor::GetTokenAtIndex(int Index, bool SelectText, int% StartIndexOut, int% EndIndexOut)
			{
				String^% Source = TextField->Text;
				int SearchIndex = Source->Length, SubStrStart = 0, SubStrEnd = SearchIndex;
				StartIndexOut = -1; EndIndexOut = -1;

				if (Index < SearchIndex && Index >= 0)
				{
					for (int i = Index; i > 0; i--)
					{
						if (ScriptParser::ScriptTextDelimiters->IndexOf(Source[i]) != -1)
						{
							SubStrStart = i + 1;
							break;
						}
					}

					for (int i = Index; i < SearchIndex; i++)
					{
						if (ScriptParser::ScriptTextDelimiters->IndexOf(Source[i]) != -1)
						{
							SubStrEnd = i;
							break;
						}
					}
				}
				else
					return "";

				if (SubStrStart > SubStrEnd)
					return "";
				else
				{
					if (SelectText)
					{
						TextField->SelectionStart = SubStrStart;
						TextField->SelectionLength = SubStrEnd - SubStrStart;
					}

					StartIndexOut = SubStrStart; EndIndexOut = SubStrEnd;
					return Source->Substring(SubStrStart, SubStrEnd - SubStrStart);
				}
			}

			String^ AvalonEditTextEditor::GetTextAtLocation(Point Location, bool SelectText)
			{
				int Index =	GetCharIndexFromPosition(Location), OffsetA = 0, OffsetB = 0;
				return GetTokenAtIndex(Index, SelectText, OffsetA, OffsetB);
			}

			String^ AvalonEditTextEditor::GetTextAtLocation(int Index, bool SelectText)
			{
				int OffsetA = 0, OffsetB = 0;
				return GetTokenAtIndex(Index, SelectText, OffsetA, OffsetB);
			}

			array<String^>^ AvalonEditTextEditor::GetTextAtLocation( int Index )
			{
				int OffsetA = 0, OffsetB = 0, Throwaway = 0;
				array<String^>^ Result = gcnew array<String^>(3);
				Result[1] = GetTokenAtIndex(Index, false, OffsetA, OffsetB);
				Result[0] = GetTokenAtIndex(OffsetA - 2, false, Throwaway, Throwaway);
				Result[2] = GetTokenAtIndex(OffsetB + 2, false, Throwaway, Throwaway);

				return Result;
			}

			void AvalonEditTextEditor::GotoLine(int Line)
			{
				if (Line > 0 && Line <= TextField->LineCount)
				{
					TextField->TextArea->Caret->Line = Line;
					TextField->TextArea->Caret->Column = 0;
					ScrollToCaret();
					FocusTextArea();
				}
				else
				{
					MessageBox::Show("Invalid line number", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
				}
			}

			void AvalonEditTextEditor::RefreshBGColorizerLayer()
			{
				TextField->TextArea->TextView->InvalidateLayer(ICSharpCode::AvalonEdit::Rendering::KnownLayer::Selection);
			}

			void AvalonEditTextEditor::RefreshTextView()
			{
				TextField->TextArea->TextView->Redraw();
			}

			void AvalonEditTextEditor::HandleTextChangeEvent()
			{
				if (InitializingFlag)
				{
					InitializingFlag = false;
					SetModifiedStatus(false);
					ClearFindResultIndicators();
				}
				else
				{
					SetModifiedStatus(true);
					if (PreventTextChangedEventFlag == PreventTextChangeFlagState::e_AutoReset)
						PreventTextChangedEventFlag = PreventTextChangeFlagState::e_Disabled;
					else if (PreventTextChangedEventFlag == PreventTextChangeFlagState::e_Disabled)
					{
						if (TextField->SelectionStart - 1 >= 0 && !GetCharIndexInsideCommentSegment(TextField->SelectionStart - 1))
						{
							if (LastKeyThatWentDown != System::Windows::Input::Key::Back || GetTokenAtCaretPos() != "")
								IntelliSenseBox->ShowInterface(IntelliSenseBox->LastOperation, false, false);
							else
								IntelliSenseBox->HideInterface();
						}
					}
				}
			}

			void AvalonEditTextEditor::StartMiddleMouseScroll(System::Windows::Input::MouseButtonEventArgs^ E)
			{
				IsMiddleMouseScrolling = true;

				ScrollStartPoint = E->GetPosition(TextField);

				TextField->Cursor = (TextField->ExtentWidth > TextField->ViewportWidth) || (TextField->ExtentHeight > TextField->ViewportHeight) ? System::Windows::Input::Cursors::ScrollAll : System::Windows::Input::Cursors::IBeam;
				TextField->CaptureMouse();
				MiddleMouseScrollTimer->Start();
			}

			void AvalonEditTextEditor::StopMiddleMouseScroll()
			{
				TextField->Cursor = System::Windows::Input::Cursors::IBeam;
				TextField->ReleaseMouseCapture();
				MiddleMouseScrollTimer->Stop();
				IsMiddleMouseScrolling = false;
			}

			void AvalonEditTextEditor::UpdateCodeFoldings()
			{
				if (IsFocused && CodeFoldingStrategy != nullptr)
					CodeFoldingStrategy->UpdateFoldings(CodeFoldingManager, TextField->Document);
			}

			void AvalonEditTextEditor::SynchronizeExternalScrollBars()
			{
				SynchronizingExternalScrollBars = true;

				int ScrollBarHeight = TextField->ExtentHeight - TextField->ViewportHeight + 155;
				int ScrollBarWidth = TextField->ExtentWidth - TextField->ViewportWidth + 155;
				int VerticalOffset = TextField->VerticalOffset;
				int HorizontalOffset = TextField->HorizontalOffset;

				if (ScrollBarHeight <= 0)
					ExternalVerticalScrollBar->Enabled = false;
				else if (!ExternalVerticalScrollBar->Enabled)
					ExternalVerticalScrollBar->Enabled = true;

				if (ScrollBarWidth <= 0)
					ExternalHorizontalScrollBar->Enabled = false;
				else if (!ExternalHorizontalScrollBar->Enabled)
					ExternalHorizontalScrollBar->Enabled = true;

				ExternalVerticalScrollBar->Maximum = ScrollBarHeight;
				ExternalVerticalScrollBar->Minimum = 0;
				if (VerticalOffset >= 0 && VerticalOffset <= ScrollBarHeight)
					ExternalVerticalScrollBar->Value = VerticalOffset;

				ExternalHorizontalScrollBar->Maximum = ScrollBarWidth;
				ExternalHorizontalScrollBar->Minimum = 0;
				if (HorizontalOffset >= 0 && HorizontalOffset <= ScrollBarHeight)
					ExternalHorizontalScrollBar->Value = HorizontalOffset;

				SynchronizingExternalScrollBars = false;
			}

			RTBitmap^ AvalonEditTextEditor::RenderFrameworkElement( System::Windows::FrameworkElement^ Element )
			{
				double TopLeft = 0;
				double TopRight = 0;
				int Width = (int)Element->ActualWidth;
				int Height = (int)Element->ActualHeight;
				double DpiX = 96; // this is the magic number
				double DpiY = 96; // this is the magic number

				System::Windows::Media::PixelFormat ReturnFormat = System::Windows::Media::PixelFormats::Default;
				System::Windows::Media::VisualBrush^ ElementBrush = gcnew System::Windows::Media::VisualBrush(Element);
				System::Windows::Media::DrawingVisual^ Visual = gcnew System::Windows::Media::DrawingVisual();

				System::Windows::Media::DrawingContext^ Context = Visual->RenderOpen();
				Context->DrawRectangle(ElementBrush, nullptr, System::Windows::Rect(TopLeft, TopRight, Width, Height));
				Context->Close();

				System::Windows::Media::Imaging::RenderTargetBitmap^ Bitmap = gcnew System::Windows::Media::Imaging::RenderTargetBitmap(Width, Height, DpiX, DpiY, ReturnFormat);
				Bitmap->Render(Visual);

				return Bitmap;
			}

			void AvalonEditTextEditor::ClearFindResultIndicators()
			{
				FindReplaceColorizer->ClearSegments();
				RefreshBGColorizerLayer();
			}

			void AvalonEditTextEditor::MoveTextSegment( AvalonEdit::Document::ISegment^ Segment, MoveSegmentDirection Direction )
			{
				int StartOffset = Segment->Offset, EndOffset = Segment->EndOffset;
				AvalonEdit::Document::DocumentLine^ PreviousLine = nullptr;
				AvalonEdit::Document::DocumentLine^ NextLine = nullptr;

				if (StartOffset - 1 >= 0)
					PreviousLine = TextField->Document->GetLineByOffset(StartOffset - 1);
				if (EndOffset + 1 < GetTextLength())
					NextLine = TextField->Document->GetLineByOffset(EndOffset + 1);

				String^ SegmentText = TextField->Document->GetText(Segment);

				switch (Direction)
				{
				case MoveSegmentDirection::e_Up:
					if (PreviousLine != nullptr)
					{
						String^ PreviousText = TextField->Document->GetText(PreviousLine);
						int InsertOffset = PreviousLine->Offset;

						TextField->Document->Remove(PreviousLine);
						if (Segment->EndOffset + 1 >= GetTextLength())
							TextField->Document->Remove(Segment->Offset, Segment->Length);
						else
							TextField->Document->Remove(Segment->Offset, Segment->Length + 1);

						TextField->Document->Insert(InsertOffset, SegmentText + "\n" + PreviousText);

						SetCaretPos(InsertOffset);
					}
					break;
				case MoveSegmentDirection::e_Down:
					if (NextLine != nullptr)
					{
						String^ NextText = TextField->Document->GetText(NextLine);
						int InsertOffset = NextLine->EndOffset - Segment->Length - NextLine->Length;
						String^ InsertText = NextText + "\n" + SegmentText;

						if (NextLine->EndOffset + 1 >= GetTextLength())
							TextField->Document->Remove(NextLine->Offset, NextLine->Length);
						else
							TextField->Document->Remove(NextLine->Offset, NextLine->Length + 1);
						TextField->Document->Remove(Segment);

						if (InsertOffset - 1 >= 0)
							InsertOffset--;

						TextField->Document->Insert(InsertOffset, InsertText);

						SetCaretPos(InsertOffset + InsertText->Length);
					}
					break;
				}
			}

			void AvalonEditTextEditor::SearchBracesForHighlighting( int CaretPos )
			{
				BraceColorizer->ClearHighlight();

				DocumentLine^ CurrentLine = TextField->Document->GetLineByOffset(CaretPos);
				int OpenBraceOffset = -1, CloseBraceOffset = -1, RelativeCaretPos = -1;
				ScriptParser^ LocalParser = gcnew ScriptParser();
				Stack<BracketSearchData^>^ BracketStack = gcnew Stack<BracketSearchData^>();
				List<BracketSearchData^>^ ParsedBracketList = gcnew List<BracketSearchData^>();

				if (CurrentLine != nullptr)
				{
					RelativeCaretPos = CaretPos - CurrentLine->Offset;
					String^ Text = TextField->Document->GetText(CurrentLine);
					LocalParser->Tokenize(Text, true);
					if (LocalParser->Valid)
					{
						for (int i = 0; i < LocalParser->GetCurrentTokenCount(); i++)
						{
							String^ Token = LocalParser->Tokens[i];
							Char Delimiter = LocalParser->Delimiters[i];
							int TokenIndex = LocalParser->Indices[i];
							int DelimiterIndex = TokenIndex + Token->Length;

							if (LocalParser->GetCommentTokenIndex(-1) == i)
								break;

							if (BracketSearchData::ValidOpeningBrackets->IndexOf(Delimiter) != -1)
							{
								BracketStack->Push(gcnew BracketSearchData(Delimiter, DelimiterIndex));
							}
							else if (BracketSearchData::ValidClosingBrackets->IndexOf(Delimiter) != -1)
							{
								if (BracketStack->Count == 0)
								{
									BracketSearchData^ DelinquentBracket = gcnew BracketSearchData(Delimiter, -1);
									DelinquentBracket->EndOffset = DelimiterIndex;
									DelinquentBracket->Mismatching = true;
									ParsedBracketList->Add(DelinquentBracket);
								}
								else
								{
									BracketSearchData^ CurrentBracket = BracketStack->Pop();
									BracketSearchData Buffer(Delimiter, DelimiterIndex);

									if (CurrentBracket->GetType() == Buffer.GetType() && CurrentBracket->GetKind() == BracketSearchData::BracketKind::e_Opening)
									{
										CurrentBracket->EndOffset = DelimiterIndex;
									}
									else
									{
										CurrentBracket->Mismatching = true;
									}

									ParsedBracketList->Add(CurrentBracket);
								}
							}
						}

						while (BracketStack->Count)
						{
							BracketSearchData^ DelinquentBracket = BracketStack->Pop();
							DelinquentBracket->EndOffset = -1;
							DelinquentBracket->Mismatching = true;
							ParsedBracketList->Add(DelinquentBracket);
						}

						if (ParsedBracketList->Count)
						{
							for each (BracketSearchData^ Itr in ParsedBracketList)
							{
								if	((Itr->GetStartOffset() <= RelativeCaretPos && Itr->EndOffset >= RelativeCaretPos) ||
									(Itr->GetStartOffset() <= RelativeCaretPos && Itr->EndOffset == -1) ||
									(Itr->GetStartOffset() == -1 && Itr->EndOffset >= RelativeCaretPos))
								{
									OpenBraceOffset = Itr->GetStartOffset();
									CloseBraceOffset = Itr->EndOffset;
									break;
								}
							}
						}
					}
				}

				if (OpenBraceOffset != -1)
					OpenBraceOffset += CurrentLine->Offset;
				if (CloseBraceOffset != -1)
					CloseBraceOffset += CurrentLine->Offset;

				BraceColorizer->SetHighlight(OpenBraceOffset, CloseBraceOffset);
				TextField->TextArea->TextView->InvalidateLayer(BraceColorizer->Layer);
			}

			AvalonEditHighlightingDefinition^ AvalonEditTextEditor::CreateSyntaxHighlightDefinitions( void )
			{
				SyntaxHighlightingManager->PurgeSerializedHighlightingDataCache();
				bool BoldFaced = PREFERENCES->FetchSettingAsInt("BoldFacedHighlighting", "Appearance");

				SyntaxHighlightingManager->CreateSerializedHighlightingData(AvalonEditXSHDManager::Rulesets::e_CommentAndPreprocessor,
					PREFERENCES->LookupColorByKey("SyntaxCommentsColor"),
					Color::GhostWhite,
					PREFERENCES->LookupColorByKey("SyntaxPreprocessorColor"),
					BoldFaced);
				SyntaxHighlightingManager->CreateSerializedHighlightingData(AvalonEditXSHDManager::Rulesets::e_Keywords,
					PREFERENCES->LookupColorByKey("SyntaxKeywordsColor"),
					Color::GhostWhite,
					Color::GhostWhite,
					BoldFaced);
				SyntaxHighlightingManager->CreateSerializedHighlightingData(AvalonEditXSHDManager::Rulesets::e_BlockTypes,
					PREFERENCES->LookupColorByKey("SyntaxScriptBlocksColor"),
					Color::GhostWhite,
					Color::GhostWhite,
					BoldFaced);
				SyntaxHighlightingManager->CreateSerializedHighlightingData(AvalonEditXSHDManager::Rulesets::e_Delimiter,
					PREFERENCES->LookupColorByKey("SyntaxDelimitersColor"),
					Color::GhostWhite,
					Color::GhostWhite,
					BoldFaced);
				SyntaxHighlightingManager->CreateSerializedHighlightingData(AvalonEditXSHDManager::Rulesets::e_Digit,
					PREFERENCES->LookupColorByKey("SyntaxDigitsColor"),
					Color::GhostWhite,
					Color::GhostWhite,
					BoldFaced);
				SyntaxHighlightingManager->CreateSerializedHighlightingData(AvalonEditXSHDManager::Rulesets::e_String,
					PREFERENCES->LookupColorByKey("SyntaxStringsColor"),
					Color::GhostWhite,
					Color::GhostWhite,
					BoldFaced);

				AvalonEditHighlightingDefinition^ Result = SyntaxHighlightingManager->CreateDefinitionFromSerializedData("ObScript");
				SyntaxHighlightingManager->PurgeSerializedHighlightingDataCache();
				return Result;
			}
#pragma endregion

#pragma region Events
			void AvalonEditTextEditor::OnScriptModified(ScriptModifiedEventArgs^ E)
			{
				ScriptModified(this, E);
			}

			void AvalonEditTextEditor::OnKeyDown(KeyEventArgs^ E)
			{
				KeyDown(this, E);
			}
#pragma endregion

#pragma region Event Handlers
			void AvalonEditTextEditor::TextField_TextChanged(Object^ Sender, EventArgs^ E)
			{
				HandleTextChangeEvent();
				SearchBracesForHighlighting(GetCaretPos());
			}

			void AvalonEditTextEditor::TextField_CaretPositionChanged(Object^ Sender, EventArgs^ E)
			{
				static UInt32 LineBuffer = 1;

				if (TextField->TextArea->Caret->Line != LineBuffer)
				{
					IntelliSenseBox->Enabled = true;
					IntelliSenseBox->LastOperation = IntelliSenseInterface::Operation::e_Default;
					LineBuffer = TextField->TextArea->Caret->Line;
					RefreshBGColorizerLayer();
				}

				SearchBracesForHighlighting(GetCaretPos());
			}

			void AvalonEditTextEditor::TextField_ScrollOffsetChanged(Object^ Sender, EventArgs^ E)
			{
				if (SynchronizingInternalScrollBars == false)
					SynchronizeExternalScrollBars();

				IntelliSenseBox->HideInterface();
			}

			void AvalonEditTextEditor::TextField_KeyDown(Object^ Sender, System::Windows::Input::KeyEventArgs^ E)
			{
				LastKeyThatWentDown = E->Key;

				if (IsMiddleMouseScrolling)
				{
					StopMiddleMouseScroll();
				}

				int SelStart = TextField->SelectionStart, SelLength = TextField->SelectionLength;

				if (ScriptParser::GetIsDelimiterKey(E->Key))
				{
					IntelliSenseBox->Enabled = true;

					if (TextField->SelectionStart - 1 >= 0 && !GetCharIndexInsideCommentSegment(TextField->SelectionStart - 1))
					{
						try
						{
							switch (E->Key)
							{
							case System::Windows::Input::Key::OemPeriod:
								{
									IntelliSenseBox->ShowInterface(IntelliSenseInterface::Operation::e_Dot, false, true);
									SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
									break;
								}
							case System::Windows::Input::Key::Space:
								{
									String^ CommandName = GetTextAtLocation(TextField->SelectionStart - 1, false)->Replace("\n", "");

									if (!String::Compare(CommandName, "call", true))
									{
										IntelliSenseBox->ShowInterface(IntelliSenseInterface::Operation::e_Call, false, true);
										SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
									}
									else if (!String::Compare(CommandName, "let", true) || !String::Compare(CommandName, "set", true))
									{
										IntelliSenseBox->ShowInterface(IntelliSenseInterface::Operation::e_Assign, false, true);
										SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
									}
									else
										IntelliSenseBox->LastOperation = IntelliSenseInterface::Operation::e_Default;

									break;
								}
							default:
								{
									IntelliSenseBox->LastOperation = IntelliSenseInterface::Operation::e_Default;
									break;
								}
							}
						}
						catch (Exception^ E)
						{
							DebugPrint("IntelliSenseInterface raised an exception while initializing.\n\tException: " + E->Message, true);
						}
					}
				}

				switch (E->Key)
				{
				case System::Windows::Input::Key::Q:
					if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
					{
						ToggleComment(TextField->SelectionStart);

						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}
					break;
				case System::Windows::Input::Key::Enter:
					if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
					{
						if (!IntelliSenseBox->Visible)
							IntelliSenseBox->ShowInterface(IntelliSenseInterface::Operation::e_Default, true, false);

						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}
					break;
				case System::Windows::Input::Key::Escape:
					if (IntelliSenseBox->Visible)
					{
						IntelliSenseBox->HideInterface();
						IntelliSenseBox->Enabled = false;
						IntelliSenseBox->LastOperation = IntelliSenseInterface::Operation::e_Default;

						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}

					ClearFindResultIndicators();
					break;
				case System::Windows::Input::Key::Tab:
					if (IntelliSenseBox->Visible)
					{
						SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
						IntelliSenseBox->PickSelection();
						FocusTextArea();

						IntelliSenseBox->LastOperation = IntelliSenseInterface::Operation::e_Default;

						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}
					break;
				case System::Windows::Input::Key::Up:
					if (IntelliSenseBox->Visible)
					{
						IntelliSenseBox->ChangeCurrentSelection(IntelliSenseInterface::MoveDirection::e_Up);

						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}
					else if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
					{
						SetPreventTextChangedFlag(PreventTextChangeFlagState::e_ManualReset);
						AvalonEdit::Document::ISegment^ Segment = TextField->Document->GetLineByOffset(GetCaretPos());
						MoveTextSegment(Segment, MoveSegmentDirection::e_Up);
						SetPreventTextChangedFlag(PreventTextChangeFlagState::e_Disabled);

						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}
					break;
				case System::Windows::Input::Key::Down:
					if (IntelliSenseBox->Visible)
					{
						IntelliSenseBox->ChangeCurrentSelection(IntelliSenseInterface::MoveDirection::e_Down);

						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}
					else if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
					{
						SetPreventTextChangedFlag(PreventTextChangeFlagState::e_ManualReset);
						AvalonEdit::Document::ISegment^ Segment = TextField->Document->GetLineByOffset(GetCaretPos());
						MoveTextSegment(Segment, MoveSegmentDirection::e_Down);
						SetPreventTextChangedFlag(PreventTextChangeFlagState::e_Disabled);

						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}
					break;
				case System::Windows::Input::Key::Z:
				case System::Windows::Input::Key::Y:
					if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
						SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
					break;
				case System::Windows::Input::Key::PageUp:
				case System::Windows::Input::Key::PageDown:
					if (IntelliSenseBox->Visible || E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
					{
						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}
					break;
				}

				Int32 KeyState = System::Windows::Input::KeyInterop::VirtualKeyFromKey(E->Key);
				if ((E->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Control) == System::Windows::Input::ModifierKeys::Control)
					KeyState |= (int)Keys::Control;
				if ((E->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Alt) == System::Windows::Input::ModifierKeys::Alt)
					KeyState |= (int)Keys::Alt;
				if ((E->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Shift) == System::Windows::Input::ModifierKeys::Shift)
					KeyState |= (int)Keys::Shift;

				KeyEventArgs^ TunneledArgs = gcnew KeyEventArgs((Keys)KeyState);
				OnKeyDown(TunneledArgs);
			}

			void AvalonEditTextEditor::TextField_KeyUp(Object^ Sender, System::Windows::Input::KeyEventArgs^ E)
			{
				if (E->Key == KeyToPreventHandling)
				{
					E->Handled = true;
					KeyToPreventHandling = System::Windows::Input::Key::None;
					return;
				}
			}

			void AvalonEditTextEditor::TextField_MouseDown(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E)
			{
				Nullable<AvalonEdit::TextViewPosition> Location = TextField->GetPositionFromPoint(E->GetPosition(TextField));
				if (Location.HasValue)
				{
					int Offset = TextField->Document->GetOffset(Location.Value.Line, Location.Value.Column);
					LastKnownMouseClickLocation = GetPositionFromCharIndex(Offset);
					System::Windows::Point ScrollCorrectedLocation = System::Windows::Point(LastKnownMouseClickLocation.X, LastKnownMouseClickLocation.Y);
					ScrollCorrectedLocation += TextField->TextArea->TextView->ScrollOffset;
					LastKnownMouseClickLocation = Point(ScrollCorrectedLocation.X, ScrollCorrectedLocation.Y);
				}
				else
				{
					SetCaretPos(GetTextLength());
				}

				if (IntelliSenseBox->Visible)
				{
					IntelliSenseBox->HideInterface();
					IntelliSenseBox->Enabled = false;
					IntelliSenseBox->LastOperation = IntelliSenseInterface::Operation::e_Default;
				}

				IntelliSenseBox->HideInfoToolTip();
			}

			void AvalonEditTextEditor::TextField_MouseWheel(Object^ Sender, System::Windows::Input::MouseWheelEventArgs^ E)
			{
				if (IntelliSenseBox->Visible)
				{
					if (E->Delta < 0)
						IntelliSenseBox->ChangeCurrentSelection(IntelliSenseInterface::MoveDirection::e_Down);
					else
						IntelliSenseBox->ChangeCurrentSelection(IntelliSenseInterface::MoveDirection::e_Up);

					E->Handled = true;
				}
				else
					IntelliSenseBox->HideInfoToolTip();
			}

			void AvalonEditTextEditor::TextField_MouseHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
			{
				Nullable<AvalonEdit::TextViewPosition> ViewLocation = TextField->GetPositionFromPoint(E->GetPosition(TextField));
				if (ViewLocation.HasValue)
				{
					int Offset = TextField->Document->GetOffset(ViewLocation.Value.Line, ViewLocation.Value.Column);
					Point Location = GetAbsolutePositionFromCharIndex(Offset);

					if (TextField->Text->Length > 0)
					{
						array<String^>^ Tokens = GetTextAtLocation(Offset);
						if (!GetCharIndexInsideCommentSegment(Offset))
							IntelliSenseBox->ShowQuickViewTooltip(Tokens[1], Tokens[0], Location);
					}
				}
			}

			void AvalonEditTextEditor::TextField_MouseHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
			{
				IntelliSenseBox->HideInfoToolTip();
			}

			void AvalonEditTextEditor::TextField_SelectionChanged(Object^ Sender, EventArgs^ E)
			{
				;//
			}

			void AvalonEditTextEditor::TextField_LostFocus(Object^ Sender, System::Windows::RoutedEventArgs^ E)
			{
				;//
			}

			void AvalonEditTextEditor::TextField_MiddleMouseScrollMove(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
			{
				static double SlowScrollFactor = 9;

				if (TextField->IsMouseCaptured)
				{
					System::Windows::Point CurrentPosition = E->GetPosition(TextField);

					System::Windows::Vector Delta = CurrentPosition - ScrollStartPoint;
					Delta.Y /= SlowScrollFactor;
					Delta.X /= SlowScrollFactor;

					CurrentScrollOffset = Delta;
				}
			}

			void AvalonEditTextEditor::TextField_MiddleMouseScrollDown(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E)
			{
				if (!IsMiddleMouseScrolling && E->ChangedButton ==  System::Windows::Input::MouseButton::Middle)
				{
					StartMiddleMouseScroll(E);
				}
				else if (IsMiddleMouseScrolling)
				{
					StopMiddleMouseScroll();
				}
			}

			void AvalonEditTextEditor::MiddleMouseScrollTimer_Tick(Object^ Sender, EventArgs^ E)
			{
				static double AccelerateScrollFactor = 0.0;

				if (IsMiddleMouseScrolling)
				{
					TextField->ScrollToVerticalOffset(TextField->VerticalOffset + CurrentScrollOffset.Y);
					TextField->ScrollToHorizontalOffset(TextField->HorizontalOffset + CurrentScrollOffset.X);

					CurrentScrollOffset += CurrentScrollOffset * AccelerateScrollFactor;
				}
			}

			void AvalonEditTextEditor::FoldingTimer_Tick( Object^ Sender, EventArgs^ E )
			{
				UpdateCodeFoldings();
			}

			void AvalonEditTextEditor::ScrollBarSyncTimer_Tick( Object^ Sender, EventArgs^ E )
			{
				SynchronizingInternalScrollBars = false;
				SynchronizeExternalScrollBars();
			}

			void AvalonEditTextEditor::LocalVarsDatabaseUpdateTimer_Tick( Object^ Sender, EventArgs^ E )
			{
				IntelliSenseBox->UpdateLocalVariableDatabase();
			}

			void AvalonEditTextEditor::ExternalScrollBar_ValueChanged( Object^ Sender, EventArgs^ E )
			{
				if (SynchronizingExternalScrollBars == false)
				{
					SynchronizingInternalScrollBars = true;

					int VerticalOffset = ExternalVerticalScrollBar->Value;
					int HorizontalOffset = ExternalHorizontalScrollBar->Value;

					VScrollBar^ VertSender = dynamic_cast<VScrollBar^>(Sender);
					HScrollBar^ HortSender = dynamic_cast<HScrollBar^>(Sender);

					if (VertSender != nullptr)
					{
						TextField->ScrollToVerticalOffset(VerticalOffset);
					}
					else if (HortSender != nullptr)
					{
						TextField->ScrollToHorizontalOffset(HorizontalOffset);
					}
				}
			}

			void AvalonEditTextEditor::SetTextAnimation_Completed( Object^ Sender, EventArgs^ E )
			{
				SetTextPrologAnimationCache->Completed -= SetTextAnimationCompletedHandler;
				SetTextPrologAnimationCache = nullptr;

				TextFieldPanel->Children->Remove(AnimationPrimitive);
				TextFieldPanel->Children->Add(TextField);

				System::Windows::Media::Animation::DoubleAnimation^ FadeInAnimation = gcnew System::Windows::Media::Animation::DoubleAnimation(0.0,
					1.0,
					System::Windows::Duration(System::TimeSpan::FromSeconds(SetTextFadeAnimationDuration)),
					System::Windows::Media::Animation::FillBehavior::Stop);
				System::Windows::Media::Animation::Storyboard^ FadeInStoryBoard = gcnew System::Windows::Media::Animation::Storyboard();
				FadeInStoryBoard->Children->Add(FadeInAnimation);
				FadeInStoryBoard->SetTargetName(FadeInAnimation, TextField->Name);
				FadeInStoryBoard->SetTargetProperty(FadeInAnimation, gcnew System::Windows::PropertyPath(TextField->OpacityProperty));
				FadeInStoryBoard->Begin(TextFieldPanel);

				SetTextAnimating = false;
			}

			void AvalonEditTextEditor::ScriptEditorPreferences_Saved( Object^ Sender, EventArgs^ E )
			{
				if (TextField->SyntaxHighlighting != nullptr)
				{
					delete TextField->SyntaxHighlighting;
					TextField->SyntaxHighlighting = nullptr;
				}
				if (CodeFoldingStrategy != nullptr)
				{
					delete CodeFoldingStrategy;
					CodeFoldingStrategy = nullptr;
				}
				if (TextField->TextArea->IndentationStrategy != nullptr)
				{
					delete TextField->TextArea->IndentationStrategy;
					TextField->TextArea->IndentationStrategy = nullptr;
				}

				TextField->SyntaxHighlighting = CreateSyntaxHighlightDefinitions();
				if (PREFERENCES->FetchSettingAsInt("CodeFolding", "Appearance"))
					CodeFoldingStrategy = gcnew AvalonEditObScriptCodeFoldingStrategy();
				TextField->Options->CutCopyWholeLine = PREFERENCES->FetchSettingAsInt("CutCopyEntireLine", "General");
				TextField->Options->ShowSpaces = PREFERENCES->FetchSettingAsInt("ShowSpaces", "Appearance");
				TextField->Options->ShowTabs = PREFERENCES->FetchSettingAsInt("ShowTabs", "Appearance");
				TextField->WordWrap = PREFERENCES->FetchSettingAsInt("WordWrap", "Appearance");
				if (PREFERENCES->FetchSettingAsInt("AutoIndent", "General"))
					TextField->TextArea->IndentationStrategy = gcnew AvalonEditObScriptIndentStrategy(true, true);
				IntelliSenseBox->MaximumVisibleItemCount = PREFERENCES->FetchSettingAsInt("MaxVisibleItems", "IntelliSense");
				IntelliSenseBox->PreventActivation = PREFERENCES->FetchSettingAsInt("NoFocusUI", "IntelliSense") == 0;

				RefreshTextView();
			}
#pragma endregion

			AvalonEditTextEditor::AvalonEditTextEditor(Font^ Font, UInt32 ParentWorkspaceIndex)
			{
				this->ParentWorkspaceIndex = ParentWorkspaceIndex;

				Container = gcnew Panel();
				WPFHost = gcnew ElementHost();
				TextFieldPanel = gcnew System::Windows::Controls::DockPanel();
				TextField = gcnew AvalonEdit::TextEditor();
				AnimationPrimitive = gcnew System::Windows::Shapes::Rectangle();
				IntelliSenseBox = gcnew IntelliSenseInterface(ParentWorkspaceIndex);
				ErrorColorizer = gcnew AvalonEditScriptErrorBGColorizer(TextField, KnownLayer::Selection);
				FindReplaceColorizer = gcnew AvalonEditFindReplaceBGColorizer(TextField, KnownLayer::Selection);
				BraceColorizer = gcnew AvalonEditBraceHighlightingBGColorizer(TextField, KnownLayer::Caret);
				CodeFoldingManager = AvalonEdit::Folding::FoldingManager::Install(TextField->TextArea);
				CodeFoldingStrategy = nullptr;
				if (PREFERENCES->FetchSettingAsInt("CodeFolding", "Appearance"))
					CodeFoldingStrategy = gcnew AvalonEditObScriptCodeFoldingStrategy();
				MiddleMouseScrollTimer = gcnew Timer();
				FoldingTimer = gcnew Timer();
				ExternalVerticalScrollBar = gcnew VScrollBar();
				ExternalHorizontalScrollBar = gcnew HScrollBar();
				ScrollBarSyncTimer = gcnew Timer();
				LocalVarsDatabaseUpdateTimer = gcnew Timer();

				TextFieldTextChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::TextField_TextChanged);
				TextFieldCaretPositionChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::TextField_CaretPositionChanged);
				TextFieldScrollOffsetChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::TextField_ScrollOffsetChanged);
				TextFieldKeyUpHandler = gcnew System::Windows::Input::KeyEventHandler(this, &AvalonEditTextEditor::TextField_KeyUp);
				TextFieldKeyDownHandler = gcnew System::Windows::Input::KeyEventHandler(this, &AvalonEditTextEditor::TextField_KeyDown);
				TextFieldMouseDownHandler = gcnew System::Windows::Input::MouseButtonEventHandler(this, &AvalonEditTextEditor::TextField_MouseDown);
				TextFieldMouseWheelHandler = gcnew System::Windows::Input::MouseWheelEventHandler(this, &AvalonEditTextEditor::TextField_MouseWheel);
				TextFieldMouseHoverHandler = gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MouseHover);
				TextFieldMouseHoverStoppedHandler = gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MouseHoverStopped);
				TextFieldSelectionChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::TextField_SelectionChanged);
				TextFieldLostFocusHandler = gcnew System::Windows::RoutedEventHandler(this, &AvalonEditTextEditor::TextField_LostFocus);
				TextFieldMiddleMouseScrollMoveHandler = gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MiddleMouseScrollMove);
				TextFieldMiddleMouseScrollDownHandler = gcnew System::Windows::Input::MouseButtonEventHandler(this, &AvalonEditTextEditor::TextField_MiddleMouseScrollDown);
				MiddleMouseScrollTimerTickHandler = gcnew EventHandler(this, &AvalonEditTextEditor::MiddleMouseScrollTimer_Tick);
				FoldingTimerTickHandler = gcnew EventHandler(this, &AvalonEditTextEditor::FoldingTimer_Tick);
				ScrollBarSyncTimerTickHandler = gcnew EventHandler(this, &AvalonEditTextEditor::ScrollBarSyncTimer_Tick);
				LocalVarsDatabaseUpdateTimerTickHandler = gcnew EventHandler(this, &AvalonEditTextEditor::LocalVarsDatabaseUpdateTimer_Tick);
				ExternalScrollBarValueChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::ExternalScrollBar_ValueChanged);
				SetTextAnimationCompletedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::SetTextAnimation_Completed);
				ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::ScriptEditorPreferences_Saved);

				System::Windows::NameScope::SetNameScope(TextFieldPanel, gcnew System::Windows::NameScope());
				TextFieldPanel->Background = gcnew Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255, 255, 255, 255));
				TextFieldPanel->VerticalAlignment = System::Windows::VerticalAlignment::Stretch;

				TextField->Name = "AvalonEditTextEditorInstance";
				TextField->Options->AllowScrollBelowDocument = false;
				TextField->Options->EnableEmailHyperlinks = false;
				TextField->Options->EnableHyperlinks = true;
				TextField->Options->RequireControlModifierForHyperlinkClick = true;
				TextField->Options->CutCopyWholeLine = PREFERENCES->FetchSettingAsInt("CutCopyEntireLine", "General");
				TextField->Options->ShowSpaces = PREFERENCES->FetchSettingAsInt("ShowSpaces", "Appearance");
				TextField->Options->ShowTabs = PREFERENCES->FetchSettingAsInt("ShowTabs", "Appearance");
				TextField->WordWrap = PREFERENCES->FetchSettingAsInt("WordWrap", "Appearance");
				TextField->ShowLineNumbers = true;
				TextField->HorizontalScrollBarVisibility = System::Windows::Controls::ScrollBarVisibility::Hidden;
				TextField->VerticalScrollBarVisibility = System::Windows::Controls::ScrollBarVisibility::Hidden;
				TextField->SyntaxHighlighting = CreateSyntaxHighlightDefinitions();		// each editor instance gets its own unique highlight definition

				TextField->TextArea->TextView->BackgroundRenderers->Add(ErrorColorizer);
				TextField->TextArea->TextView->BackgroundRenderers->Add(FindReplaceColorizer);
				TextField->TextArea->TextView->BackgroundRenderers->Add(BraceColorizer);
				TextField->TextArea->TextView->BackgroundRenderers->Add(gcnew AvalonEditSelectionBGColorizer(TextField, KnownLayer::Selection));
				TextField->TextArea->TextView->BackgroundRenderers->Add(gcnew AvalonEditLineLimitBGColorizer(TextField, KnownLayer::Selection));
				TextField->TextArea->TextView->BackgroundRenderers->Add(gcnew AvalonEditCurrentLineBGColorizer(TextField, KnownLayer::Selection));

				TextField->TextArea->IndentationStrategy = nullptr;
				if (PREFERENCES->FetchSettingAsInt("AutoIndent", "General"))
					TextField->TextArea->IndentationStrategy = gcnew AvalonEditObScriptIndentStrategy(true, true);

				AnimationPrimitive->Name = "AnimationPrimitive";

				TextFieldPanel->RegisterName(AnimationPrimitive->Name, AnimationPrimitive);
				TextFieldPanel->RegisterName(TextField->Name, TextField);

				TextFieldPanel->Children->Add(TextField);

				InitializingFlag = false;
				ModifiedFlag = false;
				PreventTextChangedEventFlag = PreventTextChangeFlagState::e_Disabled;
				KeyToPreventHandling = System::Windows::Input::Key::None;
				LastKeyThatWentDown = System::Windows::Input::Key::None;
				IsMiddleMouseScrolling = false;

				MiddleMouseScrollTimer->Interval = 16;

				IsFocused = true;
				FoldingTimer->Interval = 5000;
				FoldingTimer->Start();

				IntelliSenseBox->MaximumVisibleItemCount = PREFERENCES->FetchSettingAsInt("MaxVisibleItems", "IntelliSense");
				IntelliSenseBox->PreventActivation = PREFERENCES->FetchSettingAsInt("NoFocusUI", "IntelliSense") == 0;
				LastKnownMouseClickLocation = Point(0, 0);

				ScrollBarSyncTimer->Interval = 200;
				ScrollBarSyncTimer->Start();

				ExternalVerticalScrollBar->Dock = DockStyle::Right;
				ExternalVerticalScrollBar->SmallChange = 30;
				ExternalVerticalScrollBar->LargeChange = 155;

				ExternalHorizontalScrollBar->Dock = DockStyle::Bottom;
				ExternalHorizontalScrollBar->SmallChange = 30;
				ExternalHorizontalScrollBar->LargeChange = 155;

				SynchronizingInternalScrollBars = false;
				SynchronizingExternalScrollBars = false;

				SetTextAnimating = false;
				SetTextPrologAnimationCache = nullptr;

				LocalVarsDatabaseUpdateTimer->Interval = 5000;
				LocalVarsDatabaseUpdateTimer->Start();

				TextFieldInUpdateFlag = false;

				Container->Dock = DockStyle::Fill;
				Container->BorderStyle = BorderStyle::FixedSingle;
				Container->Controls->Add(WPFHost);
				Container->Controls->Add(ExternalVerticalScrollBar);
				Container->Controls->Add(ExternalHorizontalScrollBar);

				WPFHost->Dock = DockStyle::Fill;
				WPFHost->Child = TextFieldPanel;

				SetFont(Font);

				TextField->TextChanged += TextFieldTextChangedHandler;
				TextField->TextArea->Caret->PositionChanged += TextFieldCaretPositionChangedHandler;
				TextField->TextArea->SelectionChanged += TextFieldSelectionChangedHandler;
				TextField->LostFocus += TextFieldLostFocusHandler;
				TextField->TextArea->TextView->ScrollOffsetChanged += TextFieldScrollOffsetChangedHandler;
				TextField->PreviewKeyUp += TextFieldKeyUpHandler;
				TextField->PreviewKeyDown += TextFieldKeyDownHandler;
				TextField->PreviewMouseDown += TextFieldMouseDownHandler;
				TextField->PreviewMouseWheel += TextFieldMouseWheelHandler;
				TextField->PreviewMouseHover += TextFieldMouseHoverHandler;
				TextField->PreviewMouseHoverStopped += TextFieldMouseHoverStoppedHandler;
				TextField->PreviewMouseMove += TextFieldMiddleMouseScrollMoveHandler;
				TextField->PreviewMouseDown += TextFieldMiddleMouseScrollDownHandler;
				MiddleMouseScrollTimer->Tick += MiddleMouseScrollTimerTickHandler;
				FoldingTimer->Tick += FoldingTimerTickHandler;
				ScrollBarSyncTimer->Tick += ScrollBarSyncTimerTickHandler;
				ExternalVerticalScrollBar->ValueChanged += ExternalScrollBarValueChangedHandler;
				ExternalHorizontalScrollBar->ValueChanged += ExternalScrollBarValueChangedHandler;
				LocalVarsDatabaseUpdateTimer->Tick += LocalVarsDatabaseUpdateTimerTickHandler;
				PREFERENCES->PreferencesSaved += ScriptEditorPreferencesSavedHandler;
			}
		}
	}
}