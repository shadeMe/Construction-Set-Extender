#include "AvalonEditTextEditor.h"
#include "Globals.h"
#include "ScriptEditorPreferences.h"
#include "IntelliSenseDatabase.h"
#include "IntelliSenseItem.h"
#include "[Common]/CustomInputBox.h"
#include "RefactorTools.h"

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
					Invalid = 0,
					Curved,
					Square,
					Squiggly
				};
				static enum class						BracketKind
				{
					Invalid = 0,
					Opening,
					Closing
				};

				BracketSearchData(Char Symbol, int StartOffset) :
					Symbol(Symbol),
					StartOffset(StartOffset)
				{
					EndOffset = -1;
					Mismatching = false;
				}

				BracketType GetType()
				{
					switch (Symbol)
					{
					case '(':
					case ')':
						return BracketType::Curved;
					case '[':
					case ']':
						return BracketType::Square;
					case '{':
					case '}':
						return BracketType::Squiggly;
					default:
						return BracketType::Invalid;
					}
				}

				BracketKind GetKind()
				{
					switch (Symbol)
					{
					case '(':
					case '[':
					case '{':
						return BracketKind::Opening;
					case ')':
					case ']':
					case '}':
						return BracketKind::Closing;
					default:
						return BracketKind::Invalid;
					}
				}
				int GetStartOffset() { return StartOffset; }
			};

			int AvalonEditTextEditor::PerformFindReplaceOperationOnSegment(System::Text::RegularExpressions::Regex^ ExpressionParser,
																		   IScriptTextEditor::FindReplaceOperation Operation,
																		   AvalonEdit::Document::DocumentLine^ Line,
																		   String^ Replacement,
																		   UInt32 Options)
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

								if (Operation == IScriptTextEditor::FindReplaceOperation::Replace)
								{
									TextField->Document->Replace(Offset, Length, Replacement);
									CurrentLine = TextField->Document->GetText(Line);
									LineTracker->TrackFindResult(Offset, Offset + Replacement->Length, CurrentLine);
									SearchStartOffset = Itr->Index + Replacement->Length;
									Restart = true;
									break;
								}
								else if (Operation == IScriptTextEditor::FindReplaceOperation::Find)
								{
									LineTracker->TrackFindResult(Offset, Offset + Length, CurrentLine);
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
						if (ScriptParser::DefaultDelimiters->IndexOf(Source[i]) != -1)
						{
							SubStrStart = i + 1;
							break;
						}
					}

					for (int i = Index; i < SearchIndex; i++)
					{
						if (ScriptParser::DefaultDelimiters->IndexOf(Source[i]) != -1)
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

			bool AvalonEditTextEditor::GetCharIndexInsideStringSegment( int Index )
			{
				bool Result = true;

				if (Index < TextField->Text->Length)
				{
					AvalonEdit::Document::DocumentLine^ Line = TextField->Document->GetLineByOffset(Index);
					Result = ScriptParser::GetIndexInsideString(TextField->Document->GetText(Line), Index - Line->Offset);
				}

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
					MessageBox::Show("Invalid line number.", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
				}
			}

			void AvalonEditTextEditor::RefreshBGColorizerLayer()
			{
				TextField->TextArea->TextView->InvalidateLayer(ICSharpCode::AvalonEdit::Rendering::KnownLayer::Text);
			}

			void AvalonEditTextEditor::RefreshTextView()
			{
				TextField->TextArea->TextView->Redraw();
			}

			void AvalonEditTextEditor::HandleTextChangeEvent()
			{
				Modified = true;
				if (PreventTextChangedEventFlag == PreventTextChangeFlagState::AutoReset)
					PreventTextChangedEventFlag = PreventTextChangeFlagState::Disabled;
				else if (PreventTextChangedEventFlag == PreventTextChangeFlagState::Disabled)
				{
					if (TextField->SelectionStart - 1 >= 0 &&
						GetCharIndexInsideCommentSegment(TextField->SelectionStart - 1) == false &&
						GetCharIndexInsideStringSegment(TextField->SelectionStart - 1) == false)
					{
						if ((LastKeyThatWentDown != System::Windows::Input::Key::Back || GetTokenAtCaretPos() != "") &&
							TextField->TextArea->Selection->IsMultiline == false)
						{
							OnIntelliSenseShow(true, IIntelliSenseInterfaceModel::Operation::Default);
						}
						else
							OnIntelliSenseHide(false);
					}
				}
			}

			void AvalonEditTextEditor::StartMiddleMouseScroll(System::Windows::Input::MouseButtonEventArgs^ E)
			{
				IsMiddleMouseScrolling = true;

				MiddleMouseScrollStartPoint = E->GetPosition(TextField);

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
				if (IsFocused)
				{
					if (CodeFoldingStrategy != nullptr)
					{
#if BUILD_AVALONEDIT_VERSION >= AVALONEDIT_5_0_1
						int FirstErrorOffset = 0;
						IEnumerable<AvalonEdit::Folding::NewFolding^>^ Foldings = CodeFoldingStrategy->CreateNewFoldings(TextField->Document, FirstErrorOffset);
						CodeFoldingManager->UpdateFoldings(Foldings, FirstErrorOffset);
#else
						CodeFoldingStrategy->UpdateFoldings(CodeFoldingManager, TextField->Document);
#endif
					}
				}
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

				RTBitmap^ Bitmap = gcnew RTBitmap(Width, Height, DpiX, DpiY, ReturnFormat);
				Bitmap->Render(Visual);

				return Bitmap;
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
				case MoveSegmentDirection::Up:
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

						Caret = InsertOffset;
					}
					break;
				case MoveSegmentDirection::Down:
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

						Caret = InsertOffset + InsertText->Length;
					}
					break;
				}
			}

			void AvalonEditTextEditor::SearchBracesForHighlighting( int CaretPos )
			{
				BraceColorizer->ClearHighlight();

				if (TextField->TextArea->Selection->IsEmpty)
				{
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
							for (int i = 0; i < LocalParser->TokenCount; i++)
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

										if (CurrentBracket->GetType() == Buffer.GetType() && CurrentBracket->GetKind() == BracketSearchData::BracketKind::Opening)
											CurrentBracket->EndOffset = DelimiterIndex;
										else
											CurrentBracket->Mismatching = true;

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

					BracketStack->Clear();
					ParsedBracketList->Clear();
				}

				TextField->TextArea->TextView->InvalidateLayer(BraceColorizer->Layer);
			}

			AvalonEditHighlightingDefinition^ AvalonEditTextEditor::CreateSyntaxHighlightDefinitions( bool UpdateStableDefs )
			{
				if (UpdateStableDefs)
					SyntaxHighlightingManager->UpdateBaseDefinitions();

				List<String^>^ LocalVars = gcnew List<String^>();
				if (SemanticAnalysisCache)
				{
					for each (ObScriptParsing::Variable^ Itr in SemanticAnalysisCache->Variables)
						LocalVars->Add(Itr->Name);
				}

				AvalonEditHighlightingDefinition^ Result = SyntaxHighlightingManager->GenerateHighlightingDefinition(LocalVars);
				return Result;
			}

			String^ AvalonEditTextEditor::SanitizeUnicodeString( String^ In )
			{
				String^ Result = In->Replace((wchar_t)0xA0, (wchar_t)0x20);		// replace unicode non-breaking whitespaces with ANSI equivalents

				return Result;
			}

			void AvalonEditTextEditor::SetFont(Font^ FontObject)
			{
				TextField->FontFamily = gcnew Windows::Media::FontFamily(FontObject->FontFamily->Name);
				TextField->FontSize = FontObject->Size;
				if (FontObject->Style == Drawing::FontStyle::Bold)
					TextField->FontWeight = Windows::FontWeights::Bold;
				else
					TextField->FontWeight = Windows::FontWeights::Regular;
			}

			void AvalonEditTextEditor::SetTabCharacterSize(int PixelWidth)
			{
				TextField->Options->IndentationSize = PixelWidth;
			}

			String^ AvalonEditTextEditor::GetTextAtLine(int LineNumber)
			{
				if (LineNumber > TextField->Document->LineCount || LineNumber <= 0)
					return "";
				else
					return TextField->Document->GetText(TextField->Document->GetLineByNumber(LineNumber));
			}

			UInt32 AvalonEditTextEditor::GetTextLength(void)
			{
				return TextField->Text->Length;
			}

			void AvalonEditTextEditor::InsertText(String^ Text, int Index, bool PreventTextChangedEventHandling)
			{
				if (Index > GetTextLength())
					Index = GetTextLength();

				if (PreventTextChangedEventHandling)
					SetPreventTextChangedFlag(PreventTextChangeFlagState::AutoReset);

				TextField->Document->Insert(Index, Text);
			}

			void AvalonEditTextEditor::SetSelectionStart(int Index)
			{
				TextField->SelectionStart = Index;
			}

			void AvalonEditTextEditor::SetSelectionLength(int Length)
			{
				TextField->SelectionLength = Length;
			}

			bool AvalonEditTextEditor::GetInSelection(int Index)
			{
				return TextField->TextArea->Selection->Contains(Index);
			}

			int AvalonEditTextEditor::GetLineNumberFromCharIndex(int Index)
			{
				if (Index == -1 || TextField->Text->Length == 0)
					return 1;
				else if (Index >= TextField->Text->Length)
					Index = TextField->Text->Length - 1;

				return TextField->Document->GetLocation(Index).Line;
			}

			bool AvalonEditTextEditor::GetCharIndexInsideCommentSegment(int Index)
			{
				bool Result = true;

				if (Index >= 0 && Index < TextField->Text->Length)
				{
					AvalonEdit::Document::DocumentLine^ Line = TextField->Document->GetLineByOffset(Index);
					ScriptParser^ LocalParser = gcnew ScriptParser();
					LocalParser->Tokenize(TextField->Document->GetText(Line), false);
					if (LocalParser->GetCommentTokenIndex(LocalParser->GetTokenIndex(GetTextAtLocation(Index, false))) == -1)
						Result = false;
				}

				return Result;
			}

			String^ AvalonEditTextEditor::GetTokenAtMouseLocation()
			{
				return GetTextAtLocation(LastKnownMouseClickOffset, false)->Replace("\r\n", "")->Replace("\n", "");
			}

			array<String^>^ AvalonEditTextEditor::GetTokensAtMouseLocation()
			{
				return GetTextAtLocation(LastKnownMouseClickOffset);
			}

			int AvalonEditTextEditor::GetLastKnownMouseClickOffset()
			{
				if (LastKnownMouseClickOffset == -1)
					LastKnownMouseClickOffset = 0;
				else if (LastKnownMouseClickOffset >= GetTextLength())
					LastKnownMouseClickOffset = 0;

				return LastKnownMouseClickOffset;
			}

			void AvalonEditTextEditor::ToggleComment(int Line, ToggleCommentOperation Operation)
			{
				if (GetTextLength() == 0)
					return;

				AvalonEdit::Document::DocumentLine^ LineSegment = TextField->TextArea->Document->GetLineByNumber(Line);
				ISegment^ WhitespaceLeading = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(TextField->TextArea->Document, LineSegment);

				char FirstChar = TextField->TextArea->Document->GetCharAt(WhitespaceLeading->EndOffset);

				switch (Operation)
				{
				case ConstructionSetExtender::TextEditors::AvalonEditor::AvalonEditTextEditor::ToggleCommentOperation::Add:
					TextField->TextArea->Document->Insert(LineSegment->Offset, ";");
					break;
				case ConstructionSetExtender::TextEditors::AvalonEditor::AvalonEditTextEditor::ToggleCommentOperation::Remove:
					if (FirstChar == ';')
						TextField->TextArea->Document->Replace(WhitespaceLeading->EndOffset, 1, "");

					break;
				case ConstructionSetExtender::TextEditors::AvalonEditor::AvalonEditTextEditor::ToggleCommentOperation::Toggle:
					if (FirstChar == ';')
						TextField->TextArea->Document->Replace(WhitespaceLeading->EndOffset, 1, "");
					else if (FirstChar != ';')
						TextField->TextArea->Document->Insert(LineSegment->Offset, ";");

					break;
				}
			}

			void AvalonEditTextEditor::CommentLines(ToggleCommentOperation Operation)
			{
				BeginUpdate();

				AvalonEdit::Editing::Selection^ TextSelection = TextField->TextArea->Selection;
				if (TextSelection->IsEmpty == false)
				{
					List<UInt32>^ ProcessedLines = gcnew List<UInt32>;
					for each (AvalonEdit::Document::ISegment^ Itr in TextSelection->Segments)
					{
						AvalonEdit::Document::DocumentLine^ FirstLine = TextField->TextArea->Document->GetLineByOffset(Itr->Offset);
						AvalonEdit::Document::DocumentLine^ LastLine = TextField->TextArea->Document->GetLineByOffset(Itr->EndOffset);

						for (AvalonEdit::Document::DocumentLine^ Itr = FirstLine; Itr != LastLine->NextLine && Itr != nullptr; Itr = Itr->NextLine)
						{
							if (ProcessedLines->Contains(Itr->LineNumber) == false)
							{
								ToggleComment(Itr->LineNumber, Operation);
								ProcessedLines->Add(Itr->LineNumber);
							}
						}
					}
				}
				else
				{
					// always toggle single lines
					ToggleComment(TextField->TextArea->Document->GetLineByOffset(Caret)->LineNumber, ToggleCommentOperation::Toggle);
				}

				EndUpdate(false);
			}

			bool AvalonEditTextEditor::GetLineVisible(UInt32 LineNumber, bool CheckVisualLine)
			{
				if (CheckVisualLine)
					return TextField->TextArea->TextView->GetVisualLine(LineNumber) != nullptr;

				if (TextField->TextArea->TextView->IsMeasureValid == false)
					throw gcnew InvalidOperationException("GetLineVisible was called inside a Measure operation");

				System::Nullable< AvalonEdit::TextViewPosition> Start = TextField->TextArea->TextView->GetPosition(Windows::Point(0, 0) + TextField->TextArea->TextView->ScrollOffset);
				System::Nullable< AvalonEdit::TextViewPosition> End = TextField->TextArea->TextView->GetPosition(Windows::Point(TextField->TextArea->TextView->ActualWidth, TextField->TextArea->TextView->ActualHeight) + TextField->TextArea->TextView->ScrollOffset);

				if (Start.HasValue == false)
					return false;

				int StartLine = Start.Value.Line;
				int EndLine = (End.HasValue ? End.Value.Line : LineCount);

				return LineNumber >= StartLine && LineNumber <= EndLine;
			}

			UInt32 AvalonEditTextEditor::GetFirstVisibleLine()
			{
				DocumentLine^ Top = TextField->TextArea->TextView->GetDocumentLineByVisualTop(TextField->TextArea->TextView->ScrollOffset.Y);
				if (Top == nullptr)
					return 1;
				else
					return Top->LineNumber;
			}

			void AvalonEditTextEditor::AddBookmark(int Index)
			{
				int LineNo = GetLineNumberFromCharIndex(Index), Count = 0;

				String^ BookmarkDesc = "";
				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter A Description For The Bookmark", "Place Bookmark");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;
				else
					BookmarkDesc = Result->Text;

				LineTracker->AddBookmark(LineNo, BookmarkDesc);
			}

			String^ AvalonEditTextEditor::SerializeMetadata(bool AddPreprocessorSigil)
			{
				String^ Block = "";
				String^ Result = "";

				SerializeCaretPos(Block);
				SerializeBookmarks(Block);

				if (AddPreprocessorSigil)
					Block += Preprocessor::kPreprocessorSigil + "\n";

				if (Block != "")
				{
					Result += "\n;<" + kMetadataBlockMarker + ">\n";
					Result += Block;
					Result += ";</" + kMetadataBlockMarker + ">";
				}

				return Result;
			}

			void AvalonEditTextEditor::DeserializeMetadata(String^ Input, String^% OutMetadataBlock, String^% OutScriptText)
			{
				ScriptParser^ TextParser = gcnew ScriptParser();
				StringReader^ StringParser = gcnew StringReader(Input);
				String^ ReadLine = StringParser->ReadLine();
				String^ CSEBlock = "";
				String^ Result = "";
				bool ExtractingBlock = false;

				while (ReadLine != nullptr)
				{
					TextParser->Tokenize(ReadLine, false);

					if (ExtractingBlock)
					{
						if (!TextParser->GetTokenIndex(";</" + kMetadataBlockMarker + ">"))
							ExtractingBlock = false;
						else
							CSEBlock += ReadLine + "\n";

						ReadLine = StringParser->ReadLine();
						continue;
					}

					if (!TextParser->Valid)
					{
						Result += "\n" + ReadLine;
						ReadLine = StringParser->ReadLine();
						continue;
					}
					else if (!TextParser->GetTokenIndex(";<" + kMetadataBlockMarker + ">"))
					{
						ExtractingBlock = true;
						ReadLine = StringParser->ReadLine();
						continue;
					}

					Result += "\n" + ReadLine;
					ReadLine = StringParser->ReadLine();
				}

				if (Result != "")
					Result = Result->Substring(1);

				OutScriptText = Result;
				OutMetadataBlock = CSEBlock;
			}

			void CheckVariableNameCollision(String^ VarName, bool% HasCommandCollision, bool% HasFormCollision)
			{
				HasCommandCollision = ISDB->GetIsIdentifierScriptCommand(VarName);
				HasFormCollision = ISDB->GetIsIdentifierForm(VarName);
			}

			void AvalonEditTextEditor::QueueBackgroundTask()
			{
				if (CompilationInProgress)
					return;

				if (BackgroundTask)
				{
					// skip if the previous task is still executing
					if (BackgroundTask->Status == TaskStatus::Running ||
						BackgroundTask->Status == TaskStatus::WaitingForActivation ||
						BackgroundTask->Status == TaskStatus::WaitingToRun)
					{
						return;
					}
					else if (BackgroundTask->Status == TaskStatus::WaitingForChildrenToComplete)
						BackgroundTask->Wait();
					else if (BackgroundTask->Status == TaskStatus::RanToCompletion)
					{
						;//
					}
					else
						throw gcnew InvalidOperationException("Background task state = " + BackgroundTask->Status.ToString());
				}

				// the completion task clears the handle, so this should be true
				Debug::Assert(BackgroundTask == nullptr);

				BackgroundTaskInput^ DataIn = gcnew BackgroundTaskInput();
				DataIn->ScriptText = TextField->Document->CreateSnapshot();
				DataIn->ScriptType = ParentModel->Type;
				DataIn->CheckVarNameCollisionCommands = PREFERENCES->FetchSettingAsInt("VarCmdNameCollisions", "Validator");
				DataIn->CheckVarNameCollisionForms = PREFERENCES->FetchSettingAsInt("VarFormNameCollisions", "Validator");
				DataIn->CountVarReferences = PREFERENCES->FetchSettingAsInt("CountVarRefs", "Validator");
				DataIn->SkipVarRefCountsForQuests = PREFERENCES->FetchSettingAsInt("SuppressRefCountForQuestScripts", "Validator");

				System::Func<Object^, BackgroundTaskOutput^>^ TaskDelegate = gcnew System::Func<Object^, BackgroundTaskOutput^>(&AvalonEditTextEditor::PerformBackgroundTask);
				System::Action<Task<BackgroundTaskOutput^>^>^ ContinueDelegate = gcnew System::Action<Task<BackgroundTaskOutput^>^>(this, &AvalonEditTextEditor::ProcessBackgroundTaskOutput);

				BackgroundTask = Task<BackgroundTaskOutput^>::Factory->StartNew(TaskDelegate, DataIn);
				// the completion task must be executed in the UI thread, so we need to use its task scheduler
				BackgroundTask->ContinueWith(ContinueDelegate, Task<BackgroundTaskOutput^>::Factory->CancellationToken,
											 TaskContinuationOptions::AttachedToParent | TaskContinuationOptions::ExecuteSynchronously,
											 Globals::MainThreadTaskScheduler);
			}

			BackgroundTaskOutput^ AvalonEditTextEditor::PerformBackgroundTask(Object^ Input)
			{
				BackgroundTaskInput^ Data = (BackgroundTaskInput^)Input;
				Debug::Assert(Data != nullptr);

				ObScriptParsing::AnalysisData::Operation AnalysisOps = ObScriptParsing::AnalysisData::Operation::None;
				ObScriptParsing::ScriptType Type = ObScriptParsing::ScriptType::Object;

				AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::FillVariables;
				AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::FillControlBlocks;
				AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::PerformBasicValidation;

				if (Data->CheckVarNameCollisionCommands)
					AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::CheckVariableNameCommandCollisions;

				if (Data->CheckVarNameCollisionForms)
					AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::CheckVariableNameFormCollisions;

				if (Data->CountVarReferences)
					AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::CountVariableReferences;

				if (Data->SkipVarRefCountsForQuests)
					AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::SuppressQuestVariableRefCount;

				if (Data->ScriptType == ScriptEditor::IWorkspaceModel::ScriptType::MagicEffect)
					Type = ObScriptParsing::ScriptType::MagicEffect;
				else if (Data->ScriptType == ScriptEditor::IWorkspaceModel::ScriptType::Quest)
					Type = ObScriptParsing::ScriptType::Quest;

				BackgroundTaskOutput^ Out = gcnew BackgroundTaskOutput;
				Out->AnalysisOutput = gcnew ObScriptParsing::AnalysisData;
				// ### ISDB is not thread-safe, so no var name collision checking
				Out->AnalysisOutput->PerformAnalysis(Data->ScriptText->Text, Type, AnalysisOps, nullptr);

				return Out;
			}

			void AvalonEditTextEditor::ProcessBackgroundTaskOutput(Task<BackgroundTaskOutput^>^ Completed)
			{
				Debug::Assert(Completed->IsCompleted == true);
				Debug::Assert(Completed->IsCanceled == false);

				if (Completed->Status == TaskStatus::RanToCompletion)
				{
					if (Threading::Thread::CurrentThread->ManagedThreadId == OwnerThreadID)
					{
						if (SemanticAnalysisCache)
							SAFEDELETE_CLR(SemanticAnalysisCache);

						SemanticAnalysisCache = (ObScriptParsing::AnalysisData^)Completed->Result->AnalysisOutput;

						IntelliSenseModel->UpdateLocalVars(SemanticAnalysisCache);
						LineTracker->Cleanup();
						UpdateCodeFoldings();
						UpdateSyntaxHighlighting(false);

						LineTracker->BeginUpdate(LineTrackingManager::UpdateSource::Messages);
						LineTracker->ClearMessages(TextEditors::IScriptTextEditor::ScriptMessageSource::Validator,
												   TextEditors::IScriptTextEditor::ScriptMessageType::None);

						for each (ObScriptParsing::AnalysisData::UserMessage^ Itr in SemanticAnalysisCache->AnalysisMessages)
						{
							LineTracker->TrackMessage(Itr->Line,
													  (Itr->Critical == false ? TextEditors::IScriptTextEditor::ScriptMessageType::Warning : TextEditors::IScriptTextEditor::ScriptMessageType::Error),
													  TextEditors::IScriptTextEditor::ScriptMessageSource::Validator, Itr->Message);
						}

						LineTracker->EndUpdate(false);
					}
					else
					{
						// why the heck is this happening?
						Debugger::Log(1, "Error", "Background processing completion task " + Completed->Id + " called in a worker thread");
						Debugger::Break();
						return;
					}
				}
				else
					DebugPrint("BackgroundTask " + Completed->Id + " failed to complete successfully. Error Message - " + Completed->Exception->ToString());

				BackgroundTask = nullptr;
			}

			void AvalonEditTextEditor::WaitForBackgroundTask()
			{
				if (BackgroundTask)
				{
					BackgroundTask->Wait();
					Debug::Assert(BackgroundTask == nullptr);		// the task completion action ought to completed at this point
				}
			}

			void AvalonEditTextEditor::RoutePreprocessorMessages(int Line, String^ Message)
			{
				if (Line < 1)
					Line = 1;

				LineTracker->TrackMessage(Line,
										  TextEditors::IScriptTextEditor::ScriptMessageType::Error,
										  TextEditors::IScriptTextEditor::ScriptMessageSource::Preprocessor, Message);
			}

			void AvalonEditTextEditor::ToggleSearchPanel(bool State)
			{
				if (State)
				{
					if (InlineSearchPanel->IsClosed)
						InlineSearchPanel->Open();

					// cache beforehand as changing the search panel's property directly updates the preferences
					bool CaseInsensitive = PREFERENCES->FetchSettingAsInt("CaseInsensitive", "FindReplace");
					bool MatchWholeWord = PREFERENCES->FetchSettingAsInt("MatchWholeWord", "FindReplace");
					bool UseRegEx = PREFERENCES->FetchSettingAsInt("UseRegEx", "FindReplace");

					InlineSearchPanel->MatchCase = CaseInsensitive == false;
					InlineSearchPanel->WholeWords = MatchWholeWord;
					InlineSearchPanel->UseRegex = UseRegEx;

					String^ Query = GetSelectedText();
					if (Query == "")
						Query = GetTokenAtCaretPos();

					Query->Replace("\r\n", "")->Replace("\n", "");
					InlineSearchPanel->SearchPattern = Query;
					InlineSearchPanel->Reactivate();
				}
				else
				{
					if (InlineSearchPanel->IsClosed == false)
						InlineSearchPanel->Close();
				}
			}

#pragma region Events
			bool AvalonEditTextEditor::OnIntelliSenseKeyDown(System::Windows::Input::KeyEventArgs^ E)
			{
				Debug::Assert(IsFocused == true);
				Int32 KeyState = System::Windows::Input::KeyInterop::VirtualKeyFromKey(E->Key);

				if ((E->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Control) == System::Windows::Input::ModifierKeys::Control)
					KeyState |= (int)Keys::Control;
				if ((E->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Alt) == System::Windows::Input::ModifierKeys::Alt)
					KeyState |= (int)Keys::Alt;
				if ((E->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Shift) == System::Windows::Input::ModifierKeys::Shift)
					KeyState |= (int)Keys::Shift;

				IntelliSenseKeyEventArgs^ TunneledArgs = gcnew IntelliSenseKeyEventArgs((Keys)KeyState);
				if (IntelliSenseModel->GetTriggered(E->Key))
				{
					if (TextField->SelectionStart - 1 < 0 ||
						GetCharIndexInsideCommentSegment(TextField->SelectionStart - 1) ||
						GetCharIndexInsideStringSegment(TextField->SelectionStart - 1))
					{
						TunneledArgs->AllowForDisplay = false;
					}
				}
				else
					TunneledArgs->AllowForDisplay = false;

				IntelliSenseKeyDown(this, TunneledArgs);

				if (TunneledArgs->Handled)
				{
					HandleKeyEventForKey(E->Key);
					E->Handled = true;
				}

				if (TunneledArgs->PreventNextTextChangeEvent)
					SetPreventTextChangedFlag(PreventTextChangeFlagState::AutoReset);

				if (TunneledArgs->Display)
				{
					// new operation, show all valid items
					OnIntelliSenseShow(false, TunneledArgs->DisplayOperation);
				}

				return TunneledArgs->Handled;
			}

			void AvalonEditTextEditor::OnIntelliSenseShow(bool DefaultOperation, IntelliSense::IIntelliSenseInterfaceModel::Operation NewOperation)
			{
				if (IsFocused == false)
					return;

				IntelliSenseShowEventArgs^ E = gcnew IntelliSenseShowEventArgs;
				E->WindowHandle = WindowHandle;
				Point DisplayLocation = PointToScreen(GetPositionFromCharIndex(Caret, true));
				DisplayLocation.X += 5;

				VisualLine^ Current = TextField->TextArea->TextView->GetVisualLine(CurrentLine);
				if (Current)
					DisplayLocation.Y += Current->Height + 5;
				else
					DisplayLocation.Y += PREFERENCES->FetchSettingAsInt("FontSize", "Appearance") + 3;

				E->Location = DisplayLocation;
				E->UseActive = DefaultOperation;
				E->NewOperation = NewOperation;

				IntelliSenseShow(this, E);
			}

			void AvalonEditTextEditor::OnIntelliSenseHide(bool Reset)
			{
				if (IsFocused == false)
					return;

				IntelliSenseHideEventArgs^ E = gcnew IntelliSenseHideEventArgs;
				E->Reset = Reset;

				IntelliSenseHide(this, E);
			}

			void AvalonEditTextEditor::OnIntelliSenseRelocate()
			{
				if (IsFocused == false)
					return;

				IntelliSenseShowEventArgs^ E = gcnew IntelliSenseShowEventArgs;
				E->WindowHandle = WindowHandle;
				Point DisplayLocation = PointToScreen(GetPositionFromCharIndex(Caret, true));
				DisplayLocation.X += 5;

				VisualLine^ Current = TextField->TextArea->TextView->GetVisualLine(CurrentLine);
				if (Current)
					DisplayLocation.Y += Current->Height + 5;
				else
					DisplayLocation.Y += PREFERENCES->FetchSettingAsInt("FontSize", "Appearance") + 3;

				E->Location = DisplayLocation;

				IntelliSenseRelocate(this, E);
			}

			void AvalonEditTextEditor::OnScriptModified(bool ModificationState)
			{
				ScriptModified(this, gcnew TextEditorScriptModifiedEventArgs(ModificationState));
			}

			bool AvalonEditTextEditor::OnKeyDown(System::Windows::Input::KeyEventArgs^ E)
			{
				Int32 KeyState = System::Windows::Input::KeyInterop::VirtualKeyFromKey(E->Key);

				if ((E->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Control) == System::Windows::Input::ModifierKeys::Control)
					KeyState |= (int)Keys::Control;
				if ((E->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Alt) == System::Windows::Input::ModifierKeys::Alt)
					KeyState |= (int)Keys::Alt;
				if ((E->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Shift) == System::Windows::Input::ModifierKeys::Shift)
					KeyState |= (int)Keys::Shift;

				KeyEventArgs^ TunneledArgs = gcnew KeyEventArgs((Keys)KeyState);
				KeyDown(this, TunneledArgs);
				return TunneledArgs->Handled;
			}

			void AvalonEditTextEditor::OnMouseClick(System::Windows::Input::MouseButtonEventArgs^ E)
			{
				MouseButtons Buttons = MouseButtons::None;
				switch (E->ChangedButton)
				{
				case System::Windows::Input::MouseButton::Left:
					Buttons = MouseButtons::Left;
					break;
				case System::Windows::Input::MouseButton::Right:
					Buttons = MouseButtons::Right;
					break;
				case System::Windows::Input::MouseButton::Middle:
					Buttons = MouseButtons::Middle;
					break;
				case System::Windows::Input::MouseButton::XButton1:
					Buttons = MouseButtons::XButton1;
					break;
				case System::Windows::Input::MouseButton::XButton2:
					Buttons = MouseButtons::XButton2;
					break;
				}

				MouseClick(this, gcnew TextEditorMouseClickEventArgs(Buttons,
																	E->ClickCount,
																	E->GetPosition(TextField).X,
																	E->GetPosition(TextField).Y,
																	LastKnownMouseClickOffset));
			}
#pragma endregion

#pragma region Event Handlers
			void AvalonEditTextEditor::TextField_TextChanged(Object^ Sender, EventArgs^ E)
			{
				HandleTextChangeEvent();
				SearchBracesForHighlighting(Caret);
			}

			void AvalonEditTextEditor::TextField_CaretPositionChanged(Object^ Sender, EventArgs^ E)
			{
				if (TextField->TextArea->Caret->Line != PreviousLineBuffer)
				{
					PreviousLineBuffer = TextField->TextArea->Caret->Line;
					RefreshBGColorizerLayer();

					OnIntelliSenseHide(true);
				}

				if (TextField->TextArea->Selection->IsEmpty)
					SearchBracesForHighlighting(Caret);
			}

			void AvalonEditTextEditor::TextField_ScrollOffsetChanged(Object^ Sender, EventArgs^ E)
			{
				if (SynchronizingInternalScrollBars == false)
					SynchronizeExternalScrollBars();

				System::Windows::Vector CurrentOffset = TextField->TextArea->TextView->ScrollOffset;
				System::Windows::Vector Delta = CurrentOffset - PreviousScrollOffsetBuffer;
				PreviousScrollOffsetBuffer = CurrentOffset;

				if (GetLineVisible(CurrentLine, true) == false)
					OnIntelliSenseHide(true);
				else
					OnIntelliSenseRelocate();
			}

			void AvalonEditTextEditor::TextField_TextCopied( Object^ Sender, AvalonEdit::Editing::TextEventArgs^ E )
			{
				try
				{
					Clipboard::Clear();						// to remove HTML formatting
					Clipboard::SetText(E->Text);
				}
				catch (Exception^ X)
				{
					DebugPrint("Exception raised while accessing the clipboard.\n\tException: " + X->Message, true);
				}
			}

			void AvalonEditTextEditor::TextField_KeyDown(Object^ Sender, System::Windows::Input::KeyEventArgs^ E)
			{
				if (IsFocused == false)
					return;

				LastKeyThatWentDown = E->Key;

				if (IsMiddleMouseScrolling)
					StopMiddleMouseScroll();

				bool IntelliSenseHandled = OnIntelliSenseKeyDown(E);
				if (IntelliSenseHandled == false)
				{
					switch (E->Key)
					{
					case System::Windows::Input::Key::F:
						{
							bool Default = PREFERENCES->FetchSettingAsInt("DefaultInlineSearch", "General");
							if (Default && E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control ||
								(Default == false && E->KeyboardDevice->Modifiers.HasFlag(System::Windows::Input::ModifierKeys::Control) &&
								E->KeyboardDevice->Modifiers.HasFlag(System::Windows::Input::ModifierKeys::Shift)))
							{
								HandleKeyEventForKey(E->Key);
								E->Handled = true;
							}

							if (E->Handled)
								ToggleSearchPanel(true);
						}

						break;
					case System::Windows::Input::Key::B:
						if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
						{
							AddBookmark(TextField->SelectionStart);

							HandleKeyEventForKey(E->Key);
							E->Handled = true;
						}

						break;
					case System::Windows::Input::Key::Q:
					case System::Windows::Input::Key::W:
						if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
						{
							if (E->Key == System::Windows::Input::Key::Q)
								CommentLines(ToggleCommentOperation::Add);
							else
								CommentLines(ToggleCommentOperation::Remove);

							HandleKeyEventForKey(E->Key);
							E->Handled = true;
						}

						break;
					case System::Windows::Input::Key::Escape:
						LineTracker->ClearFindResults(true);
						ToggleSearchPanel(false);

						break;
					case System::Windows::Input::Key::Up:
						if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
						{
							SetPreventTextChangedFlag(PreventTextChangeFlagState::ManualReset);

							MoveTextSegment(TextField->Document->GetLineByOffset(Caret), MoveSegmentDirection::Up);

							SetPreventTextChangedFlag(PreventTextChangeFlagState::Disabled);

							HandleKeyEventForKey(E->Key);
							E->Handled = true;
						}

						break;
					case System::Windows::Input::Key::Down:
						if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
						{
							SetPreventTextChangedFlag(PreventTextChangeFlagState::ManualReset);

							MoveTextSegment(TextField->Document->GetLineByOffset(Caret), MoveSegmentDirection::Down);

							SetPreventTextChangedFlag(PreventTextChangeFlagState::Disabled);

							HandleKeyEventForKey(E->Key);
							E->Handled = true;
						}

						break;
					case System::Windows::Input::Key::Z:
					case System::Windows::Input::Key::Y:
						if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
							SetPreventTextChangedFlag(PreventTextChangeFlagState::AutoReset);

						break;
					case System::Windows::Input::Key::PageUp:
					case System::Windows::Input::Key::PageDown:
						if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
						{
							HandleKeyEventForKey(E->Key);
							E->Handled = true;
						}

						break;
					case System::Windows::Input::Key::OemPipe:
						if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
						{
							CString CStr(GetTokenAtCaretPos());
							ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CStr.c_str());
							if (Data && Data->IsValid())
								JumpScriptDelegate(GetTokenAtCaretPos());

							HandleKeyEventForKey(E->Key);
							E->Handled = true;
						}

						break;
					}
				}
				else
				{
					HandleKeyEventForKey(E->Key);
					E->Handled = true;
				}

				if (E->Handled == false)
				{
					if (OnKeyDown(E))
					{
						HandleKeyEventForKey(E->Key);
						E->Handled = true;
					}
				}
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
					LastKnownMouseClickOffset = TextField->Document->GetOffset(Location.Value.Line, Location.Value.Column);
				else
					Caret = GetTextLength();
			}

			void AvalonEditTextEditor::TextField_MouseUp(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E)
			{
				Nullable<AvalonEdit::TextViewPosition> Location = TextField->GetPositionFromPoint(E->GetPosition(TextField));
				if (Location.HasValue)
				{
					LastKnownMouseClickOffset = TextField->Document->GetOffset(Location.Value.Line, Location.Value.Column);

					if (E->ChangedButton == System::Windows::Input::MouseButton::Right && TextField->TextArea->Selection->IsEmpty)
						Caret = LastKnownMouseClickOffset;
				}
				else
					Caret = GetTextLength();

				OnIntelliSenseHide(true);
				OnMouseClick(E);
			}

			void AvalonEditTextEditor::TextField_MouseWheel(Object^ Sender, System::Windows::Input::MouseWheelEventArgs^ E)
			{
				;//
			}

			void AvalonEditTextEditor::TextField_MouseHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
			{
				Nullable<AvalonEdit::TextViewPosition> ViewLocation = TextField->GetPositionFromPoint(E->GetPosition(TextField));
				if (ViewLocation.HasValue)
				{
					int Offset = TextField->Document->GetOffset(ViewLocation.Value.Line, ViewLocation.Value.Column);
					ShowInsightPopup(Offset, E->GetPosition(TextField));
				}
			}

			void AvalonEditTextEditor::TextField_MouseHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
			{
				HideInsightPopup();
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

					System::Windows::Vector Delta = CurrentPosition - MiddleMouseScrollStartPoint;
					Delta.Y /= SlowScrollFactor;
					Delta.X /= SlowScrollFactor;

					MiddleMouseCurrentScrollOffset = Delta;
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
				static double AccelerateScrollFactor = 0.01;

				if (IsMiddleMouseScrolling)
				{
					TextField->ScrollToVerticalOffset(TextField->VerticalOffset + MiddleMouseCurrentScrollOffset.Y);
					TextField->ScrollToHorizontalOffset(TextField->HorizontalOffset + MiddleMouseCurrentScrollOffset.X);

					MiddleMouseCurrentScrollOffset += MiddleMouseCurrentScrollOffset * AccelerateScrollFactor;
				}
			}

			void AvalonEditTextEditor::ScrollBarSyncTimer_Tick( Object^ Sender, EventArgs^ E )
			{
				if (IsFocused == false)
					return;

				SynchronizingInternalScrollBars = false;
				SynchronizeExternalScrollBars();
			}

			void AvalonEditTextEditor::SemanticAnalysisTimer_Tick( Object^ Sender, EventArgs^ E )
			{
				if (IsFocused == false)
					return;

				QueueBackgroundTask();
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
					System::Windows::Duration(System::TimeSpan::FromSeconds(kSetTextFadeAnimationDuration)),
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
				if (CodeFoldingStrategy != nullptr)
				{
					SAFEDELETE_CLR(CodeFoldingStrategy);
					CodeFoldingManager->Clear();
				}
				if (TextField->TextArea->IndentationStrategy != nullptr)
					SAFEDELETE_CLR(TextField->TextArea->IndentationStrategy);

				UpdateSyntaxHighlighting(true);

				if (PREFERENCES->FetchSettingAsInt("CodeFolding", "Appearance"))
					CodeFoldingStrategy = gcnew ObScriptCodeFoldingStrategy(this);

				Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"),
											  PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"),
											  (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));
				SetFont(CustomFont);

				int TabSize = Decimal::ToInt32(PREFERENCES->FetchSettingAsInt("TabSize", "Appearance"));
				if (TabSize == 0)
					TabSize = 4;

				SetTabCharacterSize(TabSize);

				TextField->Options->CutCopyWholeLine = PREFERENCES->FetchSettingAsInt("CutCopyEntireLine", "General");
				TextField->Options->ShowSpaces = PREFERENCES->FetchSettingAsInt("ShowSpaces", "Appearance");
				TextField->Options->ShowTabs = PREFERENCES->FetchSettingAsInt("ShowTabs", "Appearance");
				TextField->WordWrap = PREFERENCES->FetchSettingAsInt("WordWrap", "Appearance");

				if (PREFERENCES->FetchSettingAsInt("AutoIndent", "General"))
					TextField->TextArea->IndentationStrategy = gcnew ObScriptIndentStrategy(this, true, true);
				else
					TextField->TextArea->IndentationStrategy = gcnew AvalonEdit::Indentation::DefaultIndentationStrategy();

				Color ForegroundColor = PREFERENCES->LookupColorByKey("ForegroundColor");
				Color BackgroundColor = PREFERENCES->LookupColorByKey("BackgroundColor");

				WPFHost->ForeColor = ForegroundColor;
				WPFHost->BackColor = BackgroundColor;
				WinFormsContainer->ForeColor = ForegroundColor;
				WinFormsContainer->BackColor = BackgroundColor;

				System::Windows::Media::SolidColorBrush^ ForegroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255,
																													ForegroundColor.R,
																													ForegroundColor.G,
																													ForegroundColor.B));
				System::Windows::Media::SolidColorBrush^ BackgroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255,
																													BackgroundColor.R,
																													BackgroundColor.G,
																													BackgroundColor.B));

				TextField->Foreground = ForegroundBrush;
				TextField->Background = BackgroundBrush;
				TextField->LineNumbersForeground = ForegroundBrush;
				TextFieldPanel->Background = BackgroundBrush;

				TextField->TextArea->TextView->ElementGenerators->Remove(StructureVisualizer);

				if (PREFERENCES->FetchSettingAsInt("EnableStructuralAnalysis", "Appearance"))
					TextField->TextArea->TextView->ElementGenerators->Add(StructureVisualizer);

				Color Buffer = PREFERENCES->LookupColorByKey("FindResultsHighlightColor");
				InlineSearchPanel->MarkerBrush = gcnew Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B));

				RefreshTextView();
			}

			void AvalonEditTextEditor::TextField_VisualLineConstructionStarting(Object^ Sender, VisualLineConstructionStartEventArgs^ E)
			{
				// invalidate block end lines to update the structural analysis visual element generator
				// this allows the skipping of blocks with visible starting lines
				for each (auto Itr in SemanticAnalysisCache->ControlBlocks)
				{
					if (Itr->IsMalformed() == false)
					{
						VisualLine^ Line = TextField->TextArea->TextView->GetVisualLine(Itr->EndLine);
						if (Line)
							TextField->TextArea->TextView->Redraw(Line, Windows::Threading::DispatcherPriority::Normal);
					}
				}
			}

			void AvalonEditTextEditor::SearchPanel_SearchOptionsChanged(Object^ Sender, AvalonEdit::Search::SearchOptionsChangedEventArgs^ E)
			{
				PREFERENCES->FetchSetting("CaseInsensitive", "FindReplace")->SetValue(((int)(E->MatchCase == false)).ToString());
				PREFERENCES->FetchSetting("MatchWholeWord", "FindReplace")->SetValue(((int)E->WholeWords).ToString());
				PREFERENCES->FetchSetting("UseRegEx", "FindReplace")->SetValue(((int)E->UseRegex).ToString());
			}

			void AvalonEditTextEditor::TextEditorContextMenu_Opening(Object^ Sender, CancelEventArgs^ E)
			{
				array<String^>^ Tokens = GetTokensAtMouseLocation();
				String^ MidToken = Tokens[1]->Replace("\n", "")->Replace("\t", " ")->Replace("\r", "");

				if (MidToken->Length > 20)
					ContextMenuWord->Text = MidToken->Substring(0, 17) + gcnew String("...");
				else
					ContextMenuWord->Text = MidToken;

				ContextMenuDirectLink->Tag = nullptr;
				if (ISDB->GetIsIdentifierScriptCommand(MidToken))
					ContextMenuDirectLink->Tag = ISDB->LookupDeveloperURLByCommand(MidToken);

				if (ContextMenuDirectLink->Tag == nullptr)
					ContextMenuDirectLink->Visible = false;
				else
					ContextMenuDirectLink->Visible = true;

				ContextMenuJumpToScript->Visible = true;

				CString CTUM(MidToken);
				ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CTUM.c_str());
				if (Data && Data->IsValid())
				{
					switch (Data->Type)
					{
					case ComponentDLLInterface::ScriptData::kScriptType_Object:
						if (Data->UDF)
							ContextMenuJumpToScript->Text = "Jump to Function script";
						else
							ContextMenuJumpToScript->Text = "Jump to Object script";
						break;
					case ComponentDLLInterface::ScriptData::kScriptType_Quest:
						ContextMenuJumpToScript->Text = "Jump to Quest script";
						break;
					}

					ContextMenuJumpToScript->Tag = gcnew String(Data->EditorID);
				}
				else
					ContextMenuJumpToScript->Visible = false;

				NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);

				ContextMenuRefactorCreateUDFImplementation->Visible = false;
				if ((ScriptParser::GetScriptTokenType(Tokens[0]) == ObScriptParsing::ScriptTokenType::Call ||
					(ScriptParser::GetScriptTokenType(Tokens[0]) == ObScriptParsing::ScriptTokenType::Call) &&
					GetCharIndexInsideCommentSegment(GetLastKnownMouseClickOffset()) == false))
				{
					if (ISDB->GetIsIdentifierUserFunction(MidToken) == false)
					{
						ContextMenuRefactorCreateUDFImplementation->Visible = true;
						ContextMenuRefactorCreateUDFImplementation->Tag = MidToken;
					}
				}

				String^ ImportFile = "";
				String^ Line = GetTextAtLine(GetLineNumberFromCharIndex(GetLastKnownMouseClickOffset()));

				bool IsImportDirective = Preprocessor::GetSingleton()->GetImportFilePath(Line, ImportFile,
																						 gcnew ScriptEditorPreprocessorData(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
																						 gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
																						 PREFERENCES->FetchSettingAsInt("AllowRedefinitions", "Preprocessor"),
																						 PREFERENCES->FetchSettingAsInt("NoOfPasses", "Preprocessor")));
				ContextMenuOpenImportFile->Visible = false;
				if (IsImportDirective)
				{
					ContextMenuOpenImportFile->Visible = true;
					ContextMenuOpenImportFile->Tag = ImportFile;
				}
			}

			void AvalonEditTextEditor::ContextMenuCopy_Click(Object^ Sender, EventArgs^ E)
			{
				try
				{
					Clipboard::Clear();

					String^ CopiedText = GetSelectedText();
					if (CopiedText == "")
						CopiedText = GetTokenAtMouseLocation();

					if (CopiedText != "")
						Clipboard::SetText(CopiedText->Replace("\n", "\r\n"));
				}
				catch (Exception^ E)
				{
					DebugPrint("Exception raised while accessing the clipboard.\n\tException: " + E->Message, true);
				}
			}

			void AvalonEditTextEditor::ContextMenuPaste_Click(Object^ Sender, EventArgs^ E)
			{
				try
				{
					if (Clipboard::GetText() != "")
						SetSelectedText(Clipboard::GetText(), false);
				}
				catch (Exception^ E)
				{
					DebugPrint("Exception raised while accessing the clipboard.\n\tException: " + E->Message, true);
				}
			}

			void AvalonEditTextEditor::ContextMenuToggleComment_Click(Object^ Sender, EventArgs^ E)
			{
				SetPreventTextChangedFlag(PreventTextChangeFlagState::AutoReset);
				ToggleComment(TextField->TextArea->Document->GetLineByOffset(GetLastKnownMouseClickOffset())->LineNumber,
							  ToggleCommentOperation::Toggle);
			}

			void AvalonEditTextEditor::ContextMenuAddBookmark_Click(Object^ Sender, EventArgs^ E)
			{
				AddBookmark(GetLastKnownMouseClickOffset());
			}

			void AvalonEditTextEditor::ContextMenuWikiLookup_Click(Object^ Sender, EventArgs^ E)
			{
				Process::Start("http://cs.elderscrolls.com/constwiki/index.php/Special:Search?search=" + GetTokenAtMouseLocation() + "&fulltext=Search");
			}

			void AvalonEditTextEditor::ContextMenuOBSEDocLookup_Click(Object^ Sender, EventArgs^ E)
			{
				Process::Start("http://obse.silverlock.org/obse_command_doc.html#" + GetTokenAtMouseLocation());
			}

			void AvalonEditTextEditor::ContextMenuDirectLink_Click(Object^ Sender, EventArgs^ E)
			{
				try
				{
					Process::Start((String^)ContextMenuDirectLink->Tag);
				}
				catch (Exception^ E)
				{
					DebugPrint("Exception raised while opening internet page.\n\tException: " + E->Message);
					MessageBox::Show("Couldn't open internet page. Mostly likely caused by an improperly formatted URL.",
									 SCRIPTEDITOR_TITLE,
									 MessageBoxButtons::OK,
									 MessageBoxIcon::Error);
				}
			}

			void AvalonEditTextEditor::ContextMenuJumpToScript_Click(Object^ Sender, EventArgs^ E)
			{
				JumpScriptDelegate((String^)ContextMenuJumpToScript->Tag);
			}

			void AvalonEditTextEditor::ContextMenuGoogleLookup_Click(Object^ Sender, EventArgs^ E)
			{
				Process::Start("http://www.google.com/search?hl=en&source=hp&q=" + GetTokenAtMouseLocation());
			}

			void AvalonEditTextEditor::ContextMenuOpenImportFile_Click(Object^ Sender, EventArgs^ E)
			{
				Process::Start((String^)ContextMenuOpenImportFile->Tag);
			}

			void AvalonEditTextEditor::ContextMenuRefactorAddVariable_Click(Object^ Sender, EventArgs^ E)
			{
				ToolStripMenuItem^ MenuItem = (ToolStripMenuItem^)Sender;
				ObScriptParsing::Variable::DataType VarType = (ObScriptParsing::Variable::DataType)MenuItem->Tag;
				String^ VarName = ContextMenuWord->Text;

				if (VarName->Length == 0)
				{
					InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter Variable Name", "Add Variable", VarName);
					if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
						return;
					else
						VarName = Result->Text;
				}

				InsertVariable(VarName, VarType);
			}

			void AvalonEditTextEditor::ContextMenuRefactorCreateUDFImplementation_Click(Object^ Sender, EventArgs^ E)
			{
				// ugly
				ParentModel->Controller->ApplyRefactor(ParentModel,
												  ScriptEditor::IWorkspaceModel::RefactorOperation::CreateUDF,
												  ContextMenuRefactorCreateUDFImplementation->Tag);
			}
#pragma endregion

			AvalonEditTextEditor::AvalonEditTextEditor(ScriptEditor::IWorkspaceModel^ ParentModel, JumpToScriptHandler^ JumpScriptDelegate, Font^ Font, int TabSize)
			{
				Debug::Assert(ParentModel != nullptr);
				this->ParentModel = ParentModel;
				this->JumpScriptDelegate = JumpScriptDelegate;

				OwnerThreadID = Threading::Thread::CurrentThread->ManagedThreadId;
				Debug::Assert(Globals::MainThreadID == OwnerThreadID);

				WinFormsContainer = gcnew Panel();
				WPFHost = gcnew ElementHost();
				TextFieldPanel = gcnew System::Windows::Controls::DockPanel();
				TextField = gcnew AvalonEdit::TextEditor();
				AnimationPrimitive = gcnew System::Windows::Shapes::Rectangle();
				IntelliSenseModel = gcnew IntelliSenseInterfaceModel(this);
				CodeFoldingManager = AvalonEdit::Folding::FoldingManager::Install(TextField->TextArea);
				CodeFoldingStrategy = nullptr;

				if (PREFERENCES->FetchSettingAsInt("CodeFolding", "Appearance"))
					CodeFoldingStrategy = gcnew ObScriptCodeFoldingStrategy(this);

				MiddleMouseScrollTimer = gcnew Timer();
				ExternalVerticalScrollBar = gcnew VScrollBar();
				ExternalHorizontalScrollBar = gcnew HScrollBar();
				ScrollBarSyncTimer = gcnew Timer();
				SemanticAnalysisTimer = gcnew Timer();

				TextFieldTextChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::TextField_TextChanged);
				TextFieldCaretPositionChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::TextField_CaretPositionChanged);
				TextFieldScrollOffsetChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::TextField_ScrollOffsetChanged);
				TextFieldTextCopiedHandler = gcnew AvalonEditTextEventHandler(this, &AvalonEditTextEditor::TextField_TextCopied);
				TextFieldKeyUpHandler = gcnew System::Windows::Input::KeyEventHandler(this, &AvalonEditTextEditor::TextField_KeyUp);
				TextFieldKeyDownHandler = gcnew System::Windows::Input::KeyEventHandler(this, &AvalonEditTextEditor::TextField_KeyDown);
				TextFieldMouseDownHandler = gcnew System::Windows::Input::MouseButtonEventHandler(this, &AvalonEditTextEditor::TextField_MouseDown);
				TextFieldMouseUpHandler = gcnew System::Windows::Input::MouseButtonEventHandler(this, &AvalonEditTextEditor::TextField_MouseUp);
				TextFieldMouseWheelHandler = gcnew System::Windows::Input::MouseWheelEventHandler(this, &AvalonEditTextEditor::TextField_MouseWheel);
				TextFieldMouseHoverHandler = gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MouseHover);
				TextFieldMouseHoverStoppedHandler = gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MouseHoverStopped);
				TextFieldSelectionChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::TextField_SelectionChanged);
				TextFieldLostFocusHandler = gcnew System::Windows::RoutedEventHandler(this, &AvalonEditTextEditor::TextField_LostFocus);
				TextFieldMiddleMouseScrollMoveHandler = gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MiddleMouseScrollMove);
				TextFieldMiddleMouseScrollDownHandler = gcnew System::Windows::Input::MouseButtonEventHandler(this, &AvalonEditTextEditor::TextField_MiddleMouseScrollDown);
				MiddleMouseScrollTimerTickHandler = gcnew EventHandler(this, &AvalonEditTextEditor::MiddleMouseScrollTimer_Tick);
				ScrollBarSyncTimerTickHandler = gcnew EventHandler(this, &AvalonEditTextEditor::ScrollBarSyncTimer_Tick);
				SemanticAnalysisTimerTickHandler = gcnew EventHandler(this, &AvalonEditTextEditor::SemanticAnalysisTimer_Tick);
				ExternalScrollBarValueChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::ExternalScrollBar_ValueChanged);
				SetTextAnimationCompletedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::SetTextAnimation_Completed);
				ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::ScriptEditorPreferences_Saved);
				TextFieldVisualLineConstructionStartingHandler = gcnew EventHandler<VisualLineConstructionStartEventArgs^>(this, &AvalonEditTextEditor::TextField_VisualLineConstructionStarting);
				SearchPanelSearchOptionsChangedHandler = gcnew EventHandler<AvalonEdit::Search::SearchOptionsChangedEventArgs^>(this, &AvalonEditTextEditor::SearchPanel_SearchOptionsChanged);

				System::Windows::NameScope::SetNameScope(TextFieldPanel, gcnew System::Windows::NameScope());
				TextFieldPanel->Background = gcnew Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255, 255, 255, 255));
				TextFieldPanel->VerticalAlignment = System::Windows::VerticalAlignment::Stretch;

				TextField->Name = "AvalonEditTextEditorInstance";
				TextField->Options->AllowScrollBelowDocument = false;
				TextField->Options->EnableEmailHyperlinks = false;
				TextField->Options->EnableHyperlinks = false;
				TextField->Options->RequireControlModifierForHyperlinkClick = false;
				TextField->Options->CutCopyWholeLine = PREFERENCES->FetchSettingAsInt("CutCopyEntireLine", "General");
				TextField->Options->ShowSpaces = PREFERENCES->FetchSettingAsInt("ShowSpaces", "Appearance");
				TextField->Options->ShowTabs = PREFERENCES->FetchSettingAsInt("ShowTabs", "Appearance");
				TextField->WordWrap = PREFERENCES->FetchSettingAsInt("WordWrap", "Appearance");
				TextField->ShowLineNumbers = true;
				TextField->HorizontalScrollBarVisibility = System::Windows::Controls::ScrollBarVisibility::Hidden;
				TextField->VerticalScrollBarVisibility = System::Windows::Controls::ScrollBarVisibility::Hidden;
				UpdateSyntaxHighlighting(true);

				Color ForegroundColor = PREFERENCES->LookupColorByKey("ForegroundColor");
				Color BackgroundColor = PREFERENCES->LookupColorByKey("BackgroundColor");

				WPFHost->ForeColor = ForegroundColor;
				WPFHost->BackColor = BackgroundColor;
				WPFHost->TabStop = false;
				WinFormsContainer->ForeColor = ForegroundColor;
				WinFormsContainer->BackColor = BackgroundColor;
				WinFormsContainer->TabStop = false;

				System::Windows::Media::SolidColorBrush^ ForegroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255,
																													ForegroundColor.R,
																													ForegroundColor.G,
																													ForegroundColor.B));
				System::Windows::Media::SolidColorBrush^ BackgroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255,
																													BackgroundColor.R,
																													BackgroundColor.G,
																													BackgroundColor.B));

				TextField->Foreground = ForegroundBrush;
				TextField->Background = BackgroundBrush;
				TextField->LineNumbersForeground = ForegroundBrush;

				TextField->TextArea->TextView->BackgroundRenderers->Add(BraceColorizer = gcnew BraceHighlightingBGColorizer(TextField,
																																KnownLayer::Background));
				TextField->TextArea->TextView->BackgroundRenderers->Add(gcnew SelectionBGColorizer(TextField, KnownLayer::Background));
				TextField->TextArea->TextView->BackgroundRenderers->Add(gcnew LineLimitBGColorizer(TextField, KnownLayer::Background));
				TextField->TextArea->TextView->BackgroundRenderers->Add(gcnew CurrentLineBGColorizer(TextField, KnownLayer::Background));

				TextField->TextArea->IndentationStrategy = nullptr;
				if (PREFERENCES->FetchSettingAsInt("AutoIndent", "General"))
					TextField->TextArea->IndentationStrategy = gcnew ObScriptIndentStrategy(this, true, true);
				else
					TextField->TextArea->IndentationStrategy = gcnew AvalonEdit::Indentation::DefaultIndentationStrategy();

				Color Buffer = PREFERENCES->LookupColorByKey("FindResultsHighlightColor");
				InlineSearchPanel = AvalonEdit::Search::SearchPanel::Install(TextField);
				InlineSearchPanel->MarkerBrush = gcnew Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B));
				InlineSearchPanel->SearchOptionsChanged += SearchPanelSearchOptionsChangedHandler;

				AnimationPrimitive->Name = "AnimationPrimitive";

				TextFieldPanel->RegisterName(AnimationPrimitive->Name, AnimationPrimitive);
				TextFieldPanel->RegisterName(TextField->Name, TextField);
				TextFieldPanel->Background = BackgroundBrush;

				TextFieldPanel->Children->Add(TextField);

				InitializingFlag = false;
				ModifiedFlag = false;
				PreventTextChangedEventFlag = PreventTextChangeFlagState::Disabled;
				KeyToPreventHandling = System::Windows::Input::Key::None;
				LastKeyThatWentDown = System::Windows::Input::Key::None;
				IsMiddleMouseScrolling = false;

				MiddleMouseScrollTimer->Interval = 16;

				IsFocused = false;

				LastKnownMouseClickOffset = 0;

				ScrollBarSyncTimer->Interval = 200;

				ExternalVerticalScrollBar->Dock = DockStyle::Right;
				ExternalVerticalScrollBar->SmallChange = 30;
				ExternalVerticalScrollBar->LargeChange = 155;

				ExternalHorizontalScrollBar->Dock = DockStyle::Bottom;
				ExternalHorizontalScrollBar->SmallChange = 30;
				ExternalHorizontalScrollBar->LargeChange = 155;

				SynchronizingInternalScrollBars = false;
				SynchronizingExternalScrollBars = false;
				PreviousScrollOffsetBuffer = System::Windows::Vector(0.0, 0.0);

				SetTextAnimating = false;
				SetTextPrologAnimationCache = nullptr;

				SemanticAnalysisTimer->Interval = 5000;

				TextFieldInUpdateFlag = false;
				PreviousLineBuffer = -1;
				SemanticAnalysisCache = gcnew ObScriptParsing::AnalysisData();

				InsightPopup = gcnew ToolTip;
				InsightPopup->AutoPopDelay = 500;
				InsightPopup->InitialDelay = 500;
				InsightPopup->ReshowDelay = 0;
				InsightPopup->ToolTipIcon = ToolTipIcon::None;
				InsightPopup->Tag = nullptr;

				LineTracker = gcnew LineTrackingManager(TextField);
				IconBarMargin = gcnew DefaultIconMargin(TextField, LineTracker, WindowHandle);
				TextField->TextArea->LeftMargins->Insert(0, IconBarMargin);

				StructureVisualizer = gcnew StructureVisualizerRenderer(this);

				if (PREFERENCES->FetchSettingAsInt("EnableStructuralAnalysis", "Appearance"))
					TextField->TextArea->TextView->ElementGenerators->Add(StructureVisualizer);

				CompilationInProgress = false;

				BackgroundTask = nullptr;

				TextEditorContextMenu = gcnew ContextMenuStrip();
				ContextMenuCopy = gcnew ToolStripMenuItem();
				ContextMenuPaste = gcnew ToolStripMenuItem();
				ContextMenuToggleComment = gcnew ToolStripMenuItem();
				ContextMenuAddBookmark = gcnew ToolStripMenuItem();
				ContextMenuWord = gcnew ToolStripMenuItem();
				ContextMenuWikiLookup = gcnew ToolStripMenuItem();
				ContextMenuOBSEDocLookup = gcnew ToolStripMenuItem();
				ContextMenuDirectLink = gcnew ToolStripMenuItem();
				ContextMenuJumpToScript = gcnew ToolStripMenuItem();
				ContextMenuGoogleLookup = gcnew ToolStripMenuItem();
				ContextMenuOpenImportFile = gcnew ToolStripMenuItem();
				ContextMenuRefactorAddVariable = gcnew ToolStripMenuItem();
				ContextMenuRefactorAddVariableInt = gcnew ToolStripMenuItem();
				ContextMenuRefactorAddVariableFloat = gcnew ToolStripMenuItem();
				ContextMenuRefactorAddVariableRef = gcnew ToolStripMenuItem();
				ContextMenuRefactorAddVariableString = gcnew ToolStripMenuItem();
				ContextMenuRefactorAddVariableArray = gcnew ToolStripMenuItem();
				ContextMenuRefactorCreateUDFImplementation = gcnew ToolStripMenuItem();

				SetupControlImage(ContextMenuCopy);
				SetupControlImage(ContextMenuPaste);
				SetupControlImage(ContextMenuToggleComment);
				SetupControlImage(ContextMenuAddBookmark);
				SetupControlImage(ContextMenuWikiLookup);
				SetupControlImage(ContextMenuOBSEDocLookup);
				SetupControlImage(ContextMenuDirectLink);
				SetupControlImage(ContextMenuJumpToScript);
				SetupControlImage(ContextMenuGoogleLookup);
				SetupControlImage(ContextMenuRefactorAddVariable);
				SetupControlImage(ContextMenuRefactorCreateUDFImplementation);

				AvalonEditTextEditorDefineClickHandler(ContextMenuCopy);
				AvalonEditTextEditorDefineClickHandler(ContextMenuPaste);
				AvalonEditTextEditorDefineClickHandler(ContextMenuToggleComment);
				AvalonEditTextEditorDefineClickHandler(ContextMenuAddBookmark);
				AvalonEditTextEditorDefineClickHandler(ContextMenuWikiLookup);
				AvalonEditTextEditorDefineClickHandler(ContextMenuOBSEDocLookup);
				AvalonEditTextEditorDefineClickHandler(ContextMenuDirectLink);
				AvalonEditTextEditorDefineClickHandler(ContextMenuJumpToScript);
				AvalonEditTextEditorDefineClickHandler(ContextMenuGoogleLookup);
				AvalonEditTextEditorDefineClickHandler(ContextMenuOpenImportFile);
				AvalonEditTextEditorDefineClickHandler(ContextMenuRefactorAddVariable);
				AvalonEditTextEditorDefineClickHandler(ContextMenuRefactorCreateUDFImplementation);

				TextEditorContextMenuOpeningHandler = gcnew CancelEventHandler(this, &AvalonEditTextEditor::TextEditorContextMenu_Opening);

				ContextMenuCopy->Text = "Copy";
				ContextMenuPaste->Text = "Paste";
				ContextMenuWord->Enabled = false;
				ContextMenuWikiLookup->Text = "Lookup on the Wiki";
				ContextMenuOBSEDocLookup->Text = "Lookup in the OBSE Docs";
				ContextMenuToggleComment->Text = "Toggle Comment";
				ContextMenuAddBookmark->Text = "Add Bookmark";
				ContextMenuDirectLink->Text = "Developer Page";
				ContextMenuJumpToScript->Text = "Jump into Script";
				ContextMenuGoogleLookup->Text = "Lookup on Google";

				ContextMenuRefactorAddVariable->Text = "Add Variable...";
				ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableInt);
				ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableFloat);
				ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableRef);
				ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableString);
				ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableArray);

				ContextMenuRefactorAddVariableInt->Text = "Integer";
				ContextMenuRefactorAddVariableInt->Tag = ObScriptParsing::Variable::DataType::Integer;
				ContextMenuRefactorAddVariableInt->DisplayStyle = ToolStripItemDisplayStyle::Text;
				ContextMenuRefactorAddVariableFloat->Text = "Float";
				ContextMenuRefactorAddVariableFloat->Tag = ObScriptParsing::Variable::DataType::Float;
				ContextMenuRefactorAddVariableFloat->DisplayStyle = ToolStripItemDisplayStyle::Text;
				ContextMenuRefactorAddVariableRef->Text = "Reference";
				ContextMenuRefactorAddVariableRef->Tag = ObScriptParsing::Variable::DataType::Ref;
				ContextMenuRefactorAddVariableRef->DisplayStyle = ToolStripItemDisplayStyle::Text;
				ContextMenuRefactorAddVariableString->Text = "String";
				ContextMenuRefactorAddVariableString->Tag = ObScriptParsing::Variable::DataType::StringVar;
				ContextMenuRefactorAddVariableString->DisplayStyle = ToolStripItemDisplayStyle::Text;
				ContextMenuRefactorAddVariableArray->Text = "Array";
				ContextMenuRefactorAddVariableArray->Tag = ObScriptParsing::Variable::DataType::ArrayVar;
				ContextMenuRefactorAddVariableArray->DisplayStyle = ToolStripItemDisplayStyle::Text;
				ContextMenuRefactorCreateUDFImplementation->Text = "Create UFD Implementation";
				ContextMenuRefactorCreateUDFImplementation->Visible = false;

				ContextMenuOpenImportFile->Text = "Open Import File";
				ContextMenuOpenImportFile->Visible = false;

				TextEditorContextMenu->Items->Add(ContextMenuCopy);
				TextEditorContextMenu->Items->Add(ContextMenuPaste);
				TextEditorContextMenu->Items->Add(ContextMenuToggleComment);
				TextEditorContextMenu->Items->Add(ContextMenuAddBookmark);
				TextEditorContextMenu->Items->Add(gcnew ToolStripSeparator());
				TextEditorContextMenu->Items->Add(ContextMenuWord);
				TextEditorContextMenu->Items->Add(ContextMenuWikiLookup);
				TextEditorContextMenu->Items->Add(ContextMenuOBSEDocLookup);
				TextEditorContextMenu->Items->Add(ContextMenuGoogleLookup);
				TextEditorContextMenu->Items->Add(ContextMenuDirectLink);
				TextEditorContextMenu->Items->Add(ContextMenuJumpToScript);
				TextEditorContextMenu->Items->Add(ContextMenuOpenImportFile);
				TextEditorContextMenu->Items->Add(gcnew ToolStripSeparator());
				TextEditorContextMenu->Items->Add(ContextMenuRefactorAddVariable);
				TextEditorContextMenu->Items->Add(ContextMenuRefactorCreateUDFImplementation);

				WinFormsContainer->Dock = DockStyle::Fill;
				WinFormsContainer->BorderStyle = BorderStyle::FixedSingle;
				WinFormsContainer->ContextMenuStrip = TextEditorContextMenu;
				WinFormsContainer->Controls->Add(WPFHost);
				WinFormsContainer->Controls->Add(ExternalVerticalScrollBar);
				WinFormsContainer->Controls->Add(ExternalHorizontalScrollBar);

				WPFHost->Dock = DockStyle::Fill;
				WPFHost->Child = TextFieldPanel;

				SetFont(Font);
				if (TabSize)
					SetTabCharacterSize(TabSize);

				TextField->TextChanged += TextFieldTextChangedHandler;
				TextField->TextArea->Caret->PositionChanged += TextFieldCaretPositionChangedHandler;
				TextField->TextArea->SelectionChanged += TextFieldSelectionChangedHandler;
				TextField->TextArea->TextCopied += TextFieldTextCopiedHandler;
				TextField->LostFocus += TextFieldLostFocusHandler;
				TextField->TextArea->TextView->ScrollOffsetChanged += TextFieldScrollOffsetChangedHandler;
				TextField->PreviewKeyUp += TextFieldKeyUpHandler;
				TextField->PreviewKeyDown += TextFieldKeyDownHandler;
				TextField->PreviewMouseDown += TextFieldMouseDownHandler;
				TextField->PreviewMouseUp += TextFieldMouseUpHandler;
				TextField->PreviewMouseWheel += TextFieldMouseWheelHandler;
				TextField->PreviewMouseHover += TextFieldMouseHoverHandler;
				TextField->PreviewMouseHoverStopped += TextFieldMouseHoverStoppedHandler;
				TextField->PreviewMouseMove += TextFieldMiddleMouseScrollMoveHandler;
				TextField->PreviewMouseDown += TextFieldMiddleMouseScrollDownHandler;
				MiddleMouseScrollTimer->Tick += MiddleMouseScrollTimerTickHandler;
				ScrollBarSyncTimer->Tick += ScrollBarSyncTimerTickHandler;
				ExternalVerticalScrollBar->ValueChanged += ExternalScrollBarValueChangedHandler;
				ExternalHorizontalScrollBar->ValueChanged += ExternalScrollBarValueChangedHandler;
				SemanticAnalysisTimer->Tick += SemanticAnalysisTimerTickHandler;
				PREFERENCES->PreferencesSaved += ScriptEditorPreferencesSavedHandler;
				TextField->TextArea->TextView->VisualLineConstructionStarting += TextFieldVisualLineConstructionStartingHandler;

				TextEditorContextMenu->Opening += TextEditorContextMenuOpeningHandler;
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuCopy);
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuPaste);
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuToggleComment);
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuAddBookmark);
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuWikiLookup);
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuOBSEDocLookup);
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuDirectLink);
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuJumpToScript);
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuGoogleLookup);
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuOpenImportFile);
				ContextMenuRefactorAddVariableInt->Click += ContextMenuRefactorAddVariableClickHandler;
				ContextMenuRefactorAddVariableFloat->Click += ContextMenuRefactorAddVariableClickHandler;
				ContextMenuRefactorAddVariableRef->Click += ContextMenuRefactorAddVariableClickHandler;
				ContextMenuRefactorAddVariableString->Click += ContextMenuRefactorAddVariableClickHandler;
				ContextMenuRefactorAddVariableArray->Click += ContextMenuRefactorAddVariableClickHandler;
				AvalonEditTextEditorSubscribeClickEvent(ContextMenuRefactorCreateUDFImplementation);

// 				System::Windows::PresentationSource^ src = System::Windows::PresentationSource::FromVisual(TextField);
// 				if (src)
// 				{
// 					double dpix = 96.0 * src->CompositionTarget->TransformToDevice.M11;
// 					double dpiy = 96.0 * src->CompositionTarget->TransformToDevice.M22;
// 					DebugPrint("WPF DPI (X|Y) = " + dpix + "|" + dpiy);
// 
// 				}
			}

			AvalonEditTextEditor::~AvalonEditTextEditor()
			{
				WaitForBackgroundTask();

				ParentModel = nullptr;

				HideInsightPopup();

				TextField->Clear();
				MiddleMouseScrollTimer->Stop();
				ScrollBarSyncTimer->Stop();
				SemanticAnalysisTimer->Stop();
				CodeFoldingManager->Clear();
				AvalonEdit::Folding::FoldingManager::Uninstall(CodeFoldingManager);
				InlineSearchPanel->Uninstall();

				for each (auto Itr in TextField->TextArea->TextView->BackgroundRenderers)
					delete Itr;

				TextField->TextArea->TextView->BackgroundRenderers->Clear();

				TextField->TextArea->TextView->ElementGenerators->Clear();

				TextField->TextArea->LeftMargins->Remove(IconBarMargin);

				TextField->TextChanged -= TextFieldTextChangedHandler;
				TextField->TextArea->Caret->PositionChanged -= TextFieldCaretPositionChangedHandler;
				TextField->TextArea->SelectionChanged -= TextFieldSelectionChangedHandler;
				TextField->TextArea->TextCopied -= TextFieldTextCopiedHandler;
				TextField->LostFocus -= TextFieldLostFocusHandler;
				TextField->TextArea->TextView->ScrollOffsetChanged -= TextFieldScrollOffsetChangedHandler;
				TextField->PreviewKeyUp -= TextFieldKeyUpHandler;
				TextField->PreviewKeyDown -= TextFieldKeyDownHandler;
				TextField->PreviewMouseDown -= TextFieldMouseDownHandler;
				TextField->PreviewMouseUp -= TextFieldMouseUpHandler;
				TextField->PreviewMouseWheel -= TextFieldMouseWheelHandler;
				TextField->PreviewMouseHover -= TextFieldMouseHoverHandler;
				TextField->PreviewMouseHoverStopped -= TextFieldMouseHoverStoppedHandler;
				TextField->PreviewMouseMove -= TextFieldMiddleMouseScrollMoveHandler;
				TextField->PreviewMouseDown -= TextFieldMiddleMouseScrollDownHandler;
				MiddleMouseScrollTimer->Tick -= MiddleMouseScrollTimerTickHandler;
				ScrollBarSyncTimer->Tick -= ScrollBarSyncTimerTickHandler;
				ExternalVerticalScrollBar->ValueChanged -= ExternalScrollBarValueChangedHandler;
				ExternalHorizontalScrollBar->ValueChanged -= ExternalScrollBarValueChangedHandler;
				SemanticAnalysisTimer->Tick -= SemanticAnalysisTimerTickHandler;
				PREFERENCES->PreferencesSaved -= ScriptEditorPreferencesSavedHandler;
				TextField->TextArea->TextView->VisualLineConstructionStarting -= TextFieldVisualLineConstructionStartingHandler;
				InlineSearchPanel->SearchOptionsChanged -= SearchPanelSearchOptionsChangedHandler;

				SAFEDELETE_CLR(TextFieldTextChangedHandler);
				SAFEDELETE_CLR(TextFieldCaretPositionChangedHandler);
				SAFEDELETE_CLR(TextFieldSelectionChangedHandler);
				SAFEDELETE_CLR(TextFieldTextCopiedHandler);
				SAFEDELETE_CLR(TextFieldLostFocusHandler);
				SAFEDELETE_CLR(TextFieldScrollOffsetChangedHandler);
				SAFEDELETE_CLR(TextFieldKeyUpHandler);
				SAFEDELETE_CLR(TextFieldKeyDownHandler);
				SAFEDELETE_CLR(TextFieldMouseDownHandler);
				SAFEDELETE_CLR(TextFieldMouseUpHandler);
				SAFEDELETE_CLR(TextFieldMouseWheelHandler);
				SAFEDELETE_CLR(TextFieldMouseHoverHandler);
				SAFEDELETE_CLR(TextFieldMouseHoverStoppedHandler);
				SAFEDELETE_CLR(TextFieldMiddleMouseScrollMoveHandler);
				SAFEDELETE_CLR(TextFieldMiddleMouseScrollDownHandler);
				SAFEDELETE_CLR(MiddleMouseScrollTimerTickHandler);
				SAFEDELETE_CLR(ScrollBarSyncTimerTickHandler);
				SAFEDELETE_CLR(ExternalScrollBarValueChangedHandler);
				SAFEDELETE_CLR(SemanticAnalysisTimerTickHandler);
				SAFEDELETE_CLR(ScriptEditorPreferencesSavedHandler);
				SAFEDELETE_CLR(TextFieldVisualLineConstructionStartingHandler);
				SAFEDELETE_CLR(SetTextAnimationCompletedHandler);
				SAFEDELETE_CLR(SearchPanelSearchOptionsChangedHandler);

				TextEditorContextMenu->Opening -= TextEditorContextMenuOpeningHandler;
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuCopy);
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuPaste);
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuToggleComment);
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuAddBookmark);
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuWikiLookup);
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuOBSEDocLookup);
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuDirectLink);
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuJumpToScript);
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuGoogleLookup);
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuOpenImportFile);
				ContextMenuRefactorAddVariableInt->Click -= ContextMenuRefactorAddVariableClickHandler;
				ContextMenuRefactorAddVariableFloat->Click -= ContextMenuRefactorAddVariableClickHandler;
				ContextMenuRefactorAddVariableRef->Click -= ContextMenuRefactorAddVariableClickHandler;
				ContextMenuRefactorAddVariableString->Click -= ContextMenuRefactorAddVariableClickHandler;
				ContextMenuRefactorAddVariableArray->Click -= ContextMenuRefactorAddVariableClickHandler;
				AvalonEditTextEditorUnsubscribeDeleteClickEvent(ContextMenuRefactorCreateUDFImplementation);

				SAFEDELETE_CLR(TextEditorContextMenuOpeningHandler);
				SAFEDELETE_CLR(ContextMenuRefactorAddVariableClickHandler);

				DisposeControlImage(ContextMenuCopy);
				DisposeControlImage(ContextMenuPaste);
				DisposeControlImage(ContextMenuToggleComment);
				DisposeControlImage(ContextMenuAddBookmark);
				DisposeControlImage(ContextMenuWikiLookup);
				DisposeControlImage(ContextMenuOBSEDocLookup);
				DisposeControlImage(ContextMenuDirectLink);
				DisposeControlImage(ContextMenuJumpToScript);
				DisposeControlImage(ContextMenuGoogleLookup);
				DisposeControlImage(ContextMenuRefactorAddVariable);
				DisposeControlImage(ContextMenuRefactorCreateUDFImplementation);

				TextFieldPanel->Children->Clear();
				WPFHost->Child = nullptr;
				WinFormsContainer->Controls->Clear();
				WinFormsContainer->ContextMenu = nullptr;

				SAFEDELETE_CLR(IconBarMargin);
				SAFEDELETE_CLR(LineTracker);
				SAFEDELETE_CLR(InsightPopup);
				SAFEDELETE_CLR(IntelliSenseModel);
				SAFEDELETE_CLR(JumpScriptDelegate);
				SAFEDELETE_CLR(StructureVisualizer);
				SAFEDELETE_CLR(InlineSearchPanel);

				SAFEDELETE_CLR(TextEditorContextMenu);
				SAFEDELETE_CLR(ContextMenuCopy);
				SAFEDELETE_CLR(ContextMenuPaste);
				SAFEDELETE_CLR(ContextMenuToggleComment);
				SAFEDELETE_CLR(ContextMenuAddBookmark);
				SAFEDELETE_CLR(ContextMenuWord);
				SAFEDELETE_CLR(ContextMenuWikiLookup);
				SAFEDELETE_CLR(ContextMenuOBSEDocLookup);
				SAFEDELETE_CLR(ContextMenuDirectLink);
				SAFEDELETE_CLR(ContextMenuJumpToScript);
				SAFEDELETE_CLR(ContextMenuGoogleLookup);
				SAFEDELETE_CLR(ContextMenuRefactorAddVariable);
				SAFEDELETE_CLR(ContextMenuRefactorAddVariableInt);
				SAFEDELETE_CLR(ContextMenuRefactorAddVariableFloat);
				SAFEDELETE_CLR(ContextMenuRefactorAddVariableRef);
				SAFEDELETE_CLR(ContextMenuRefactorAddVariableString);
				SAFEDELETE_CLR(ContextMenuRefactorAddVariableArray);
				SAFEDELETE_CLR(ContextMenuRefactorCreateUDFImplementation);

				SAFEDELETE_CLR(TextField->TextArea->IndentationStrategy);

				SAFEDELETE_CLR(WinFormsContainer);
				SAFEDELETE_CLR(WPFHost);
				SAFEDELETE_CLR(TextFieldPanel);
				SAFEDELETE_CLR(AnimationPrimitive);
				SAFEDELETE_CLR(MiddleMouseScrollTimer);
				SAFEDELETE_CLR(BraceColorizer);
				SAFEDELETE_CLR(CodeFoldingManager);
				SAFEDELETE_CLR(CodeFoldingStrategy);
				SAFEDELETE_CLR(ScrollBarSyncTimer);
				SAFEDELETE_CLR(SemanticAnalysisTimer);
				SAFEDELETE_CLR(ExternalVerticalScrollBar);
				SAFEDELETE_CLR(ExternalHorizontalScrollBar);
				SAFEDELETE_CLR(SemanticAnalysisCache);
				SAFEDELETE_CLR(TextField);
			}

			void AvalonEditTextEditor::UpdateSemanticAnalysisCache(bool FillVariables, bool FillControlBlocks, bool Validate)
			{
				ObScriptParsing::AnalysisData::Operation AnalysisOps = ObScriptParsing::AnalysisData::Operation::None;
				ObScriptParsing::ScriptType Type = ObScriptParsing::ScriptType::Object;

				if (FillVariables)
					AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::FillVariables;

				if (FillControlBlocks)
					AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::FillControlBlocks;

				if (Validate)
				{
					AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::PerformBasicValidation;

					if (PREFERENCES->FetchSettingAsInt("VarCmdNameCollisions", "Validator"))
						AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::CheckVariableNameCommandCollisions;

					if (PREFERENCES->FetchSettingAsInt("VarFormNameCollisions", "Validator"))
						AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::CheckVariableNameFormCollisions;

					if (PREFERENCES->FetchSettingAsInt("CountVarRefs", "Validator"))
						AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::CountVariableReferences;

					if (PREFERENCES->FetchSettingAsInt("SuppressRefCountForQuestScripts", "Validator"))
						AnalysisOps = AnalysisOps | ObScriptParsing::AnalysisData::Operation::SuppressQuestVariableRefCount;
				}

				if (ParentModel->Type == ScriptEditor::IWorkspaceModel::ScriptType::MagicEffect)
					Type = ObScriptParsing::ScriptType::MagicEffect;
				else if (ParentModel->Type == ScriptEditor::IWorkspaceModel::ScriptType::Quest)
					Type = ObScriptParsing::ScriptType::Quest;

				SemanticAnalysisCache->PerformAnalysis(GetText(), Type, AnalysisOps,
													   gcnew ObScriptParsing::AnalysisData::CheckVariableNameCollision(CheckVariableNameCollision));
			}

			void AvalonEditTextEditor::UpdateSyntaxHighlighting(bool Regenerate)
			{
				delete TextField->SyntaxHighlighting;
				TextField->SyntaxHighlighting = CreateSyntaxHighlightDefinitions(Regenerate);
			}

			void AvalonEditTextEditor::SerializeCaretPos(String^% Result)
			{
				if (PREFERENCES->FetchSettingAsInt("SaveLastKnownPos", "General"))
					Result += String::Format(";<" + kMetadataSigilCaret + "> {0} </" + kMetadataSigilCaret + ">\n", Caret);
			}
			void AvalonEditTextEditor::SerializeBookmarks(String^% Result)
			{
				Result += LineTracker->SerializeBookmarks();
			}

			void AvalonEditTextEditor::DeserializeCaretPos(String^ ExtractedBlock)
			{
				ScriptParser^ TextParser = gcnew ScriptParser();
				StringReader^ StringParser = gcnew StringReader(ExtractedBlock);
				String^ ReadLine = StringParser->ReadLine();
				int CaretPos = 0;

				while (ReadLine != nullptr)
				{
					TextParser->Tokenize(ReadLine, false);
					if (!TextParser->Valid)
					{
						ReadLine = StringParser->ReadLine();
						continue;
					}

					if (!TextParser->GetTokenIndex(";<" + kMetadataSigilCaret + ">"))
					{
						try { CaretPos = int::Parse(TextParser->Tokens[1]); }
						catch (...) { CaretPos = -1; }
						break;
					}

					ReadLine = StringParser->ReadLine();
				}

				if (CaretPos >= GetTextLength())
					CaretPos = GetTextLength() - 1;
				else if (CaretPos < 0)
					CaretPos = 0;

				Caret = CaretPos;
				ScrollToCaret();
			}

			void AvalonEditTextEditor::DeserializeBookmarks(String^ ExtractedBlock)
			{
				LineTracker->DeserializeBookmarks(ExtractedBlock, true);
			}

			void AvalonEditTextEditor::ShowInsightPopup(int Offset, Windows::Point Location)
			{
				int Line = GetLineNumberFromCharIndex(Offset);
				bool DisplayPopup = false;

				if (PREFERENCES->FetchSettingAsInt("UseQuickView", "IntelliSense") &&
					GetTextLength() > 0)
				{
					String^ DisplayText = "";
					String^ DisplayTitle = "";
					List<ScriptMessage^>^ Messages = gcnew List < ScriptMessage^ >;
					LineTracker->GetMessages(Line,
											 IScriptTextEditor::ScriptMessageSource::Validator,
											 IScriptTextEditor::ScriptMessageType::Error,
											 Messages);
					LineTracker->GetMessages(Line,
											 IScriptTextEditor::ScriptMessageSource::Compiler,
											 IScriptTextEditor::ScriptMessageType::Error,
											 Messages);

					if (Messages->Count)
					{
						for each (ScriptMessage^ Itr in Messages)
						{
							String^ Str = Itr->Message();
							DisplayText += Str + "\n";
						}

						if (DisplayText->Length)
							DisplayText->Remove(DisplayText->Length - 1);

						DisplayTitle = "Error" + (Messages->Count > 1 ? "s" : "");
						InsightPopup->ToolTipIcon = ToolTipIcon::Error;
						DisplayPopup = true;
					}
					else if (GetCharIndexInsideCommentSegment(Offset) == false)
					{
						array<String^>^ Tokens = GetTextAtLocation(Offset);
						String^ Main = Tokens[1];
						String^ Previous = Tokens[0];

						CString CStr(Previous);
						ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CStr.c_str());
						if (Data && Data->IsValid())
						{
							Previous = "" + gcnew String(Data->ParentID);
							ISDB->CacheRemoteScript(gcnew String(Data->ParentID), gcnew String(Data->Text));
						}
						NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);

						IntelliSenseItem^ Item = ISDB->LookupRemoteScriptVariable(Previous, Main);

						bool LocalVar = false;
						if (Item == nullptr)
						{
							ObScriptParsing::Variable^ Var = SemanticAnalysisCache->LookupVariable(Main);
							if (Var)
							{
								LocalVar = true;
								Item = gcnew IntelliSenseItemVariable(Var->Name,
																	Var->Comment,
																	Var->Type,
																	IntelliSense::IntelliSenseItem::IntelliSenseItemType::LocalVar);
							}
						}

						if (LocalVar == false && Item == nullptr)
						{
							for each (IntelliSenseItem^ Itr in ISDB->ItemRegistry)
							{
								if (Itr->GetIsQuickViewable(Main))
								{
									Item = Itr;
									break;
								}
							}
						}

						if (Item)
						{
							DisplayText = Item->Describe();
							DisplayTitle = Item->GetItemTypeID();
							InsightPopup->ToolTipIcon = ToolTipIcon::Info;
							DisplayPopup = true;
						}
					}

					if (DisplayPopup)
					{
						Point DisplayLocation;
						DisplayLocation.X = Location.X;
						DisplayLocation.Y = Location.Y;

						VisualLine^ Current = TextField->TextArea->TextView->GetVisualLine(CurrentLine);
						if (Current)
							DisplayLocation.Y += Current->Height;
						else
							DisplayLocation.Y += PREFERENCES->FetchSettingAsInt("FontSize", "Appearance");

						InsightPopup->ToolTipTitle = DisplayTitle;
						InsightPopup->Show(DisplayText, Control::FromHandle(WindowHandle), DisplayLocation, 8000);
					}
				}
			}

			void AvalonEditTextEditor::HideInsightPopup()
			{
				InsightPopup->Hide(Control::FromHandle(WindowHandle));
			}

#pragma region Interface
			void AvalonEditTextEditor::Bind(ListView^ MessageList, ListView^ BookmarkList, ListView^ FindResultList, IIntelliSenseInterfaceView^ IntelliSenseView)
			{
				IsFocused = true;
				SemanticAnalysisTimer->Start();
				ScrollBarSyncTimer->Start();

				LineTracker->Bind(MessageList, BookmarkList, FindResultList);
				IntelliSenseModel->Bind(IntelliSenseView);

				FocusTextArea();
			}

			void AvalonEditTextEditor::Unbind()
			{
				WaitForBackgroundTask();

				IsFocused = false;
				SemanticAnalysisTimer->Stop();
				ScrollBarSyncTimer->Stop();
				HideInsightPopup();

				LineTracker->Unbind();
				IntelliSenseModel->Unbind();

				Windows::Input::Keyboard::ClearFocus();
			}

			void DummyOutputWrapper(int Line, String^ Message)
			{
				;//
			}

			String^ AvalonEditTextEditor::GetPreprocessedText(bool% OutPreprocessResult, bool SuppressErrors)
			{
				String^ Preprocessed = "";
				ScriptPreprocessor::StandardOutputError^ ErrorOutput = gcnew ScriptPreprocessor::StandardOutputError(&DummyOutputWrapper);
				if (SuppressErrors == false)
				{
					ErrorOutput = gcnew ScriptPreprocessor::StandardOutputError(this, &AvalonEditTextEditor::RoutePreprocessorMessages);
					LineTracker->BeginUpdate(LineTrackingManager::UpdateSource::Messages);
					LineTracker->ClearMessages(TextEditors::IScriptTextEditor::ScriptMessageSource::Preprocessor,
											   TextEditors::IScriptTextEditor::ScriptMessageType::None);
				}

				bool Result = Preprocessor::GetSingleton()->PreprocessScript(GetText(),
																			 Preprocessed,
																			 ErrorOutput,
																			 gcnew ScriptEditorPreprocessorData(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
																			 gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
																			 PREFERENCES->FetchSettingAsInt("AllowRedefinitions", "Preprocessor"),
																			 PREFERENCES->FetchSettingAsInt("NoOfPasses", "Preprocessor")));

				if (SuppressErrors == false)
					LineTracker->EndUpdate(false);

				OutPreprocessResult = Result;
				return Preprocessed;
			}

			String^ AvalonEditTextEditor::GetText(void)
			{
				return SanitizeUnicodeString(TextField->Text);
			}

			String^ AvalonEditTextEditor::GetText(UInt32 LineNumber)
			{
				if (LineNumber >= LineCount || LineNumber == 0)
					return "";
				else
					return SanitizeUnicodeString(TextField->Document->GetText(TextField->Document->GetLineByNumber(LineNumber)));
			}

			void AvalonEditTextEditor::SetText(String^ Text, bool PreventTextChangedEventHandling, bool ResetUndoStack)
			{
				WaitForBackgroundTask();

				Text = SanitizeUnicodeString(Text);

				if (PreventTextChangedEventHandling)
					SetPreventTextChangedFlag(PreventTextChangeFlagState::AutoReset);

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

					UpdateSemanticAnalysisCache(true, true, false);
					UpdateCodeFoldings();
					UpdateSyntaxHighlighting(false);
				}
				else
				{
					SetTextAnimating = true;

					TextFieldPanel->Children->Add(AnimationPrimitive);

					AnimationPrimitive->Fill = gcnew System::Windows::Media::VisualBrush(TextField);
					AnimationPrimitive->Height = TextField->ActualHeight;
					AnimationPrimitive->Width = TextField->ActualWidth;

					TextFieldPanel->Children->Remove(TextField);

					System::Windows::Media::Animation::DoubleAnimation^ FadeOutAnimation = gcnew System::Windows::Media::Animation::DoubleAnimation(1.0,
																							0.0,
																							System::Windows::Duration(System::TimeSpan::FromSeconds(kSetTextFadeAnimationDuration)),
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

					UpdateSemanticAnalysisCache(true, true, false);
					UpdateCodeFoldings();
					UpdateSyntaxHighlighting(false);
				}
			}

			String^ AvalonEditTextEditor::GetSelectedText(void)
			{
				return TextField->SelectedText;
			}

			void AvalonEditTextEditor::SetSelectedText(String^ Text, bool PreventTextChangedEventHandling)
			{
				if (PreventTextChangedEventHandling)
					SetPreventTextChangedFlag(PreventTextChangeFlagState::AutoReset);

				TextField->SelectedText = Text;
			}

			int AvalonEditTextEditor::GetCharIndexFromPosition(Point Position)
			{
				Nullable<AvalonEdit::TextViewPosition> TextPos = TextField->TextArea->TextView->GetPosition(Windows::Point(Position.X, Position.Y));
				if (TextPos.HasValue)
					return TextField->Document->GetOffset(TextPos.Value.Line, TextPos.Value.Column);
				else
					return GetTextLength() + 1;
			}

			Point AvalonEditTextEditor::GetPositionFromCharIndex(int Index, bool Absolute)
			{
				if (Absolute)
				{
					Point Result = GetPositionFromCharIndex(Index, false);

					for each (System::Windows::UIElement^ Itr in TextField->TextArea->LeftMargins)
						Result.X += (dynamic_cast<System::Windows::FrameworkElement^>(Itr))->ActualWidth;

					return Result;
				}
				else
				{
					AvalonEdit::Document::TextLocation Location = TextField->Document->GetLocation(Index);
					Windows::Point Result = TextField->TextArea->TextView->GetVisualPosition(AvalonEdit::TextViewPosition(Location), AvalonEdit::Rendering::VisualYPosition::TextTop) - TextField->TextArea->TextView->ScrollOffset;
					return Point(Result.X, Result.Y);
				}
			}

			String^ AvalonEditTextEditor::GetTokenAtCharIndex(int Offset)
			{
				return GetTextAtLocation(Offset, false)->Replace("\r\n", "")->Replace("\n", "");
			}

			String^ AvalonEditTextEditor::GetTokenAtCaretPos()
			{
				return GetTextAtLocation(Caret - 1, false)->Replace("\r\n", "")->Replace("\n", "");
			}

			void AvalonEditTextEditor::SetTokenAtCaretPos(String^ Replacement)
			{
				GetTextAtLocation(Caret - 1, true);
				TextField->SelectedText = Replacement;
				Caret = TextField->SelectionStart + TextField->SelectionLength;
			}

			void AvalonEditTextEditor::ScrollToCaret()
			{
				TextField->TextArea->Caret->BringCaretToView();
			}

			void AvalonEditTextEditor::FocusTextArea()
			{
				WinFormsContainer->Focus();
				WPFHost->Focus();
				TextFieldPanel->Focus();
				TextField->Focus();

				WPFFocusHelper::Focus(TextField);
			}

			void AvalonEditTextEditor::LoadFileFromDisk(String^ Path)
			{
				try
				{
					SetPreventTextChangedFlag(PreventTextChangeFlagState::ManualReset);
					StreamReader^ Reader = gcnew StreamReader(Path);
					String^ FileText = Reader->ReadToEnd();
					SetText(FileText, false, false);
					Reader->Close();
					SetPreventTextChangedFlag(PreventTextChangeFlagState::Disabled);
				}
				catch (Exception^ E)
				{
					DebugPrint("Error encountered when opening file for read operation!\n\tError Message: " + E->Message);
					SetPreventTextChangedFlag(PreventTextChangeFlagState::Disabled);
				}
			}

			void AvalonEditTextEditor::SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^ DefaultName, String^ DefaultExtension)
			{
				if (PathIncludesFileName == false)
					Path += "\\" + DefaultName + "." + DefaultExtension;

				try
				{
					TextField->Save(Path);
				}
				catch (Exception^ E)
				{
					DebugPrint("Error encountered when opening file for write operation!\n\tError Message: " + E->Message);
				}
			}

			IScriptTextEditor::FindReplaceResult^ AvalonEditTextEditor::FindReplace(IScriptTextEditor::FindReplaceOperation Operation, String^ Query, String^ Replacement, UInt32 Options)
			{
				IScriptTextEditor::FindReplaceResult^ Result = gcnew IScriptTextEditor::FindReplaceResult;

				if (Operation != IScriptTextEditor::FindReplaceOperation::CountMatches)
				{
					BeginUpdate();
					LineTracker->ClearFindResults(false);
					LineTracker->BeginUpdate(LineTrackingManager::UpdateSource::FindResults);
				}

				try
				{
					String^ Pattern = "";

					if ((Options & (UInt32)IScriptTextEditor::FindReplaceOptions::RegEx))
						Pattern = Query;
					else
					{
						Pattern = System::Text::RegularExpressions::Regex::Escape(Query);
						if ((Options & (UInt32)IScriptTextEditor::FindReplaceOptions::MatchWholeWord))
							Pattern = "\\b" + Pattern + "\\b";
					}

					System::Text::RegularExpressions::Regex^ Parser = nullptr;
					if ((Options & (UInt32)IScriptTextEditor::FindReplaceOptions::CaseInsensitive))
					{
						Parser = gcnew System::Text::RegularExpressions::Regex(Pattern,
																			   System::Text::RegularExpressions::RegexOptions::IgnoreCase | System::Text::RegularExpressions::RegexOptions::Singleline);
					}
					else
					{
						Parser = gcnew System::Text::RegularExpressions::Regex(Pattern,
																			   System::Text::RegularExpressions::RegexOptions::Singleline);
					}

					AvalonEdit::Editing::Selection^ TextSelection = TextField->TextArea->Selection;

					if ((Options & (UInt32)IScriptTextEditor::FindReplaceOptions::InSelection))
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
									int Matches = PerformFindReplaceOperationOnSegment(Parser, Operation, Itr, Replacement, Options);
									if (Matches == -1)
									{
										Result->HasError = true;
										break;
									}
									else if (Matches)
										Result->Add(Itr->LineNumber, TextField->Document->GetText(Itr), Matches);
								}
							}
						}
					}
					else
					{
						for each (DocumentLine^ Line in TextField->Document->Lines)
						{
							int Matches = PerformFindReplaceOperationOnSegment(Parser, Operation, Line, Replacement, Options);
							if (Matches == -1)
							{
								Result->HasError = true;
								break;
							}
							else if (Matches)
								Result->Add(Line->LineNumber, TextField->Document->GetText(Line), Matches);
						}
					}
				}
				catch (Exception^ E)
				{
					Result->HasError = true;
					DebugPrint("Couldn't perform find/replace operation!\n\tException: " + E->Message);
				}

				if (Operation != IScriptTextEditor::FindReplaceOperation::CountMatches)
				{
					SetSelectionLength(0);
					RefreshBGColorizerLayer();
					EndUpdate(false);
					LineTracker->EndUpdate(true);
				}

				if (Result->HasError)
				{
					MessageBox::Show("An error occurred while performing the find/replace operation. Please recheck your search and/or replacement strings.",
									 SCRIPTEDITOR_TITLE,
									 MessageBoxButtons::OK,
									 MessageBoxIcon::Exclamation);
				}

				return Result;
			}

			void AvalonEditTextEditor::ScrollToLine(UInt32 LineNumber)
			{
				GotoLine(LineNumber);
			}

			Point AvalonEditTextEditor::PointToScreen(Point Location)
			{
				return WinFormsContainer->PointToScreen(Location);
			}

			void AvalonEditTextEditor::BeginUpdate(void)
			{
				if (TextFieldInUpdateFlag)
					throw gcnew CSEGeneralException("Text editor is already being updated.");

				TextFieldInUpdateFlag = true;
				TextField->Document->BeginUpdate();

				SetPreventTextChangedFlag(PreventTextChangeFlagState::ManualReset);
			}

			void AvalonEditTextEditor::EndUpdate(bool FlagModification)
			{
				if (TextFieldInUpdateFlag == false)
					throw gcnew CSEGeneralException("Text editor isn't being updated.");

				TextField->Document->EndUpdate();
				TextFieldInUpdateFlag = false;

				SetPreventTextChangedFlag(PreventTextChangeFlagState::Disabled);

				if (FlagModification)
					Modified = true;
			}

			UInt32 AvalonEditTextEditor::GetIndentLevel(UInt32 LineNumber)
			{
				if (Modified)
					UpdateSemanticAnalysisCache(false, true, false);

				return SemanticAnalysisCache->GetLineIndentLevel(LineNumber);
			}

			void AvalonEditTextEditor::InsertVariable(String^ VariableName, ObScriptParsing::Variable::DataType VariableType)
			{
				if (Modified)
					UpdateSemanticAnalysisCache(true, false, false);

				String^ Declaration = ObScriptParsing::Variable::GetVariableDataTypeToken(VariableType) + " " + VariableName + "\n";
				UInt32 InsertionLine = SemanticAnalysisCache->NextVariableLine;
				if (InsertionLine == 0)
					InsertText(Declaration, TextField->Document->GetOffset(TextField->Document->LineCount, 1), true);
				else
				{
					if (InsertionLine - 1 == TextField->Document->LineCount)
					{
						int Offset = TextField->Document->GetLineByNumber(TextField->Document->LineCount)->EndOffset;
						InsertText("\n", Offset, true);
					}
					else if (InsertionLine > TextField->Document->LineCount)
						InsertionLine = TextField->Document->LineCount;

					InsertText(Declaration, TextField->Document->GetOffset(InsertionLine, 1), true);
				}
			}

			ObScriptParsing::AnalysisData^ AvalonEditTextEditor::GetSemanticAnalysisCache(bool UpdateVars, bool UpdateControlBlocks)
			{
				if (UpdateVars || UpdateControlBlocks)
					UpdateSemanticAnalysisCache(UpdateVars, UpdateControlBlocks, false);

				return SemanticAnalysisCache;
			}

			CompilationData^ AvalonEditTextEditor::BeginScriptCompilation()
			{
				Debug::Assert(CompilationInProgress == false);
				CompilationInProgress = true;

				WaitForBackgroundTask();

				CompilationData^ Result = gcnew CompilationData;

				String^ Preprocessed = "";
				ScriptPreprocessor::StandardOutputError^ ErrorOutput = gcnew ScriptPreprocessor::StandardOutputError(this,
																													 &AvalonEditTextEditor::RoutePreprocessorMessages);

				ScriptEditorPreprocessorData^ Data = gcnew ScriptEditorPreprocessorData(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
																						gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
																						PREFERENCES->FetchSettingAsInt("AllowRedefinitions", "Preprocessor"),
																						PREFERENCES->FetchSettingAsInt("NoOfPasses", "Preprocessor"));

				LineTracker->BeginUpdate(LineTrackingManager::UpdateSource::Messages);

				UpdateSemanticAnalysisCache(true, true, true);
				LineTracker->ClearMessages(TextEditors::IScriptTextEditor::ScriptMessageSource::Validator,
										   TextEditors::IScriptTextEditor::ScriptMessageType::None);

				for each (ObScriptParsing::AnalysisData::UserMessage^ Itr in SemanticAnalysisCache->AnalysisMessages)
				{
					LineTracker->TrackMessage(Itr->Line,
											  (Itr->Critical == false ? TextEditors::IScriptTextEditor::ScriptMessageType::Warning : TextEditors::IScriptTextEditor::ScriptMessageType::Error),
											  TextEditors::IScriptTextEditor::ScriptMessageSource::Validator, Itr->Message);
				}

				if (SemanticAnalysisCache->HasCriticalMessages == false && SemanticAnalysisCache->MalformedStructure == false)
				{
					LineTracker->ClearMessages(TextEditors::IScriptTextEditor::ScriptMessageSource::Preprocessor,
											   TextEditors::IScriptTextEditor::ScriptMessageType::None);

					Result->CanCompile = Preprocessor::GetSingleton()->PreprocessScript(GetText(),
																			Preprocessed,
																			ErrorOutput,
																			Data);

					if (Result->CanCompile)
					{
						Result->PreprocessedScriptText = Preprocessed;
						Result->UnpreprocessedScriptText = GetText();
						Result->HasDirectives = Data->ContainsDirectives;
						Result->SerializedMetadata = SerializeMetadata(Result->HasDirectives);

						LineTracker->ClearMessages(TextEditors::IScriptTextEditor::ScriptMessageSource::Compiler,
												   TextEditors::IScriptTextEditor::ScriptMessageType::None);
					}
				}

				// doesn't include compiler warnings for obvious reasons but it's okay since all compiler messages are errors
				Result->HasWarnings = LineTracker->GetMessageCount(0,
															  TextEditors::IScriptTextEditor::ScriptMessageSource::None,
															  TextEditors::IScriptTextEditor::ScriptMessageType::Warning);

				return Result;
			}

			void AvalonEditTextEditor::EndScriptCompilation(CompilationData^ Data)
			{
				Debug::Assert(CompilationInProgress == true);
				CompilationInProgress = false;

				if (Data->CompileResult)
				{
					for (int i = 0; i < Data->CompileResult->CompileErrorData.Count; i++)
					{
						String^ Message = gcnew String(Data->CompileResult->CompileErrorData.ErrorListHead[i].Message);
						int Line = Data->CompileResult->CompileErrorData.ErrorListHead[i].Line;
						if (Line < 1)
							Line = 1;

						LineTracker->TrackMessage(Line,
												  TextEditors::IScriptTextEditor::ScriptMessageType::Error,
												  TextEditors::IScriptTextEditor::ScriptMessageSource::Compiler, Message);
					}
				}

				LineTracker->EndUpdate(true);
			}

			void AvalonEditTextEditor::InitializeState(String^ RawScriptText)
			{
				WaitForBackgroundTask();

				LineTracker->ClearMessages(TextEditors::IScriptTextEditor::ScriptMessageSource::None, TextEditors::IScriptTextEditor::ScriptMessageType::None);
				LineTracker->ClearBookmarks();
				LineTracker->ClearFindResults(false);

				String^ Extracted = "";
				String^ Metadata = "";

				DeserializeMetadata(RawScriptText, Metadata, Extracted);
				SetText(Extracted, true, true);
				DeserializeCaretPos(Metadata);
				DeserializeBookmarks(Metadata);
				Modified = false;

				FocusTextArea();
			}

#pragma endregion
		}
	}
}