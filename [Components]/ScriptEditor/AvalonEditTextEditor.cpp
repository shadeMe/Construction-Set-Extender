#include "AvalonEditTextEditor.h"
#include "Preferences.h"
#include "IntelliSenseBackend.h"
#include "IntelliSenseItem.h"

namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


namespace avalonEdit
{


using namespace intellisense;
using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;
using namespace System::Text::RegularExpressions;


int AvalonEditTextEditor::PerformFindReplaceOperationOnSegment(System::Text::RegularExpressions::Regex^ ExpressionParser,
																eFindReplaceOperation Operation,
																AvalonEdit::Document::DocumentLine^ Line,
																String^ Replacement,
																eFindReplaceOptions Options)
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

					if (Options.HasFlag(eFindReplaceOptions::IgnoreComments) == false || GetCharIndexInsideCommentSegment(Offset) == false)
					{
						if (Operation == eFindReplaceOperation::Replace)
						{
							TextField->Document->Replace(Offset, Length, Replacement);
							CurrentLine = TextField->Document->GetText(Line);
							LineTracker->TrackFindResultSegment(Offset, Offset + Replacement->Length);
							SearchStartOffset = Itr->Index + Replacement->Length;
							Restart = true;
							break;
						}
						else if (Operation == eFindReplaceOperation::Find)
						{
							LineTracker->TrackFindResultSegment(Offset, Offset + Length);
						}
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

String^ AvalonEditTextEditor::GetTokenAtIndex(int Index, bool SelectText,
											int% OutStartIndex, int% OutEndIndex,
											Char% OutStartDelimiter, Char% OutEndDelimiter)
{
	String^% Source = TextField->Text;
	int SearchIndex = Source->Length, SubStrStart = 0, SubStrEnd = SearchIndex;
	OutStartIndex = -1; OutEndIndex = -1;
	OutStartDelimiter = Char::MinValue, OutEndDelimiter = Char::MinValue;

	if (Index < SearchIndex && Index >= 0)
	{
		for (int i = Index; i > 0; i--)
		{
			if (obScriptParsing::LineTokenizer::DefaultDelimiters->IndexOf(Source[i]) != -1)
			{
				SubStrStart = i + 1;
				OutStartDelimiter = Source[i];
				break;
			}
		}

		for (int i = Index; i < SearchIndex; i++)
		{
			if (obScriptParsing::LineTokenizer::DefaultDelimiters->IndexOf(Source[i]) != -1)
			{
				SubStrEnd = i;
				OutEndDelimiter = Source[i];
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

		OutStartIndex = SubStrStart; OutEndIndex = SubStrEnd;
		return Source->Substring(SubStrStart, SubStrEnd - SubStrStart);
	}
}

String^ AvalonEditTextEditor::GetTokenAtLocation(Point Location, bool SelectText)
{
	int Index =	GetCharIndexFromPosition(Location), OffsetA = 0, OffsetB = 0;
	Char Throwaway;
	return GetTokenAtIndex(Index, SelectText, OffsetA, OffsetB, Throwaway, Throwaway);
}

String^ AvalonEditTextEditor::GetTokenAtLocation(int Index, bool SelectText)
{
	int OffsetA = 0, OffsetB = 0;
	Char Throwaway;
	return GetTokenAtIndex(Index, SelectText, OffsetA, OffsetB, Throwaway, Throwaway);
}

array<String^>^ AvalonEditTextEditor::GetTokenAtLocation( int Index )
{
	int OffsetA = 0, OffsetB = 0, Throwaway = 0;
	Char ThrowawayChar;
	array<String^>^ Result = gcnew array<String^>(3);

	Result[1] = GetTokenAtIndex(Index, false, OffsetA, OffsetB, ThrowawayChar, ThrowawayChar);
	Result[0] = GetTokenAtIndex(OffsetA - 2, false, Throwaway, Throwaway, ThrowawayChar, ThrowawayChar);
	Result[2] = GetTokenAtIndex(OffsetB + 2, false, Throwaway, Throwaway, ThrowawayChar, ThrowawayChar);

	return Result;
}

array<String^>^ AvalonEditTextEditor::GetTokenAtLocation(int Index, array<Tuple<Char, Char>^>^% OutDelimiters)
{
	int OffsetA = 0, OffsetB = 0, Throwaway = 0;
	Char LeadingDelimiter, TrailingDelimiter;
	array<String^>^ Result = gcnew array<String^>(3);

	Result[1] = GetTokenAtIndex(Index, false, OffsetA, OffsetB, LeadingDelimiter, TrailingDelimiter);
	OutDelimiters[1] = gcnew Tuple<Char, Char>(LeadingDelimiter, TrailingDelimiter);

	Result[0] = GetTokenAtIndex(OffsetA - 2, false, Throwaway, Throwaway, LeadingDelimiter, TrailingDelimiter);
	OutDelimiters[0] = gcnew Tuple<Char, Char>(LeadingDelimiter, TrailingDelimiter);

	Result[2] = GetTokenAtIndex(OffsetB + 2, false, Throwaway, Throwaway, LeadingDelimiter, TrailingDelimiter);
	OutDelimiters[2] = gcnew Tuple<Char, Char>(LeadingDelimiter, TrailingDelimiter);

	return Result;
}

System::Char AvalonEditTextEditor::GetDelimiterAtLocation(int Index)
{
	int StartOffset = 0, EndOffset = 0;
	Char LeadingDelimiter, TrailingDelimiter;

	String^ Token = GetTokenAtIndex(Index, false, StartOffset, EndOffset, LeadingDelimiter, TrailingDelimiter);
	return TrailingDelimiter;
}

bool AvalonEditTextEditor::GetCharIndexInsideStringSegment(int Index)
{
	bool Result = true;

	if (Index < TextField->Text->Length)
	{
		AvalonEdit::Document::DocumentLine^ Line = TextField->Document->GetLineByOffset(Index);
		Result = obScriptParsing::LineTokenizer::GetIndexInsideString(TextField->Document->GetText(Line), Index - Line->Offset);
	}

	return Result;
}

void AvalonEditTextEditor::SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling State)
{
	if (State == eIntelliSenseTextChangeEventHandling::SuppressOnce && IntelliSenseTextChangeEventHandlingMode == eIntelliSenseTextChangeEventHandling::SuppressAlways)
		return;

	IntelliSenseTextChangeEventHandlingMode = State;
}

void AvalonEditTextEditor::HandleKeyEventForKey(System::Windows::Input::Key Key)
{
	KeyToPreventHandling = Key;
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
}

void AvalonEditTextEditor::RefreshTextView()
{
	TextField->TextArea->TextView->Redraw();
}

void AvalonEditTextEditor::HandleTextChangeEvent()
{
	if (TextField->IsReadOnly || TextFieldDisplayingStaticText)
		return;

	Modified = true;

	switch (IntelliSenseTextChangeEventHandlingMode)
	{
	case eIntelliSenseTextChangeEventHandling::Propagate:
		break;
	case eIntelliSenseTextChangeEventHandling::SuppressOnce:
		IntelliSenseTextChangeEventHandlingMode = eIntelliSenseTextChangeEventHandling::Propagate;
		return;
	case eIntelliSenseTextChangeEventHandling::SuppressAlways:
		return;
	}

	RaiseIntelliSenseContextChange(intellisense::IntelliSenseContextChangeEventArgs::Event::TextChanged);
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
	if (ActivatedInView)
	{
		if (CodeFoldingStrategy != nullptr)
		{
			int FirstErrorOffset = 0;
			IEnumerable<AvalonEdit::Folding::NewFolding^>^ Foldings = CodeFoldingStrategy->CreateNewFoldings(TextField->Document, FirstErrorOffset);
			CodeFoldingManager->UpdateFoldings(Foldings, FirstErrorOffset);
		}
	}
}

void AvalonEditTextEditor::SynchronizeExternalScrollBars()
{
	int ScrollBarHeight = TextField->ExtentHeight - TextField->ViewportHeight + 155;
	int ScrollBarWidth = TextField->ExtentWidth - TextField->ViewportWidth + 155;
	int VerticalOffset = TextField->VerticalOffset;
	int HorizontalOffset = TextField->HorizontalOffset;

	SynchronizingExternalScrollBars = true;
	{
		if (ScrollBarHeight <= 0 || VerticalOffset < 0 || VerticalOffset >= ScrollBarHeight)
		{
			//ExternalVerticalScrollBar->Enabled = false;
			ExternalVerticalScrollBar->Visible = false;
		}
		else if (!ExternalVerticalScrollBar->Visible)
		{
			//ExternalVerticalScrollBar->Enabled = true;
			ExternalVerticalScrollBar->Visible = true;
		}

		if (ExternalVerticalScrollBar->Visible)
		{
			ExternalVerticalScrollBar->Maximum = ScrollBarHeight;
			ExternalVerticalScrollBar->Minimum = 0;

			if (VerticalOffset < ExternalVerticalScrollBar->Minimum)
				VerticalOffset = ExternalVerticalScrollBar->Minimum;
			else if (VerticalOffset > ExternalVerticalScrollBar->Maximum)
				VerticalOffset = ExternalVerticalScrollBar->Maximum;

			ExternalVerticalScrollBar->Value = VerticalOffset;
		}

		if (ScrollBarWidth <= 0 || HorizontalOffset < 0 || HorizontalOffset >= ScrollBarWidth)
		{
			//ExternalHorizontalScrollBar->Enabled = false;
			ExternalHorizontalScrollBar->Visible = false;
		}
		else if (!ExternalHorizontalScrollBar->Visible)
		{
			//ExternalHorizontalScrollBar->Enabled = true;
			ExternalHorizontalScrollBar->Visible = true;
		}

		if (ExternalHorizontalScrollBar->Visible)
		{
			ExternalHorizontalScrollBar->Maximum = ScrollBarWidth;
			ExternalHorizontalScrollBar->Minimum = 0;

			if (HorizontalOffset < ExternalHorizontalScrollBar->Minimum)
				HorizontalOffset = ExternalHorizontalScrollBar->Minimum;
			else if (HorizontalOffset > ExternalHorizontalScrollBar->Maximum)
				HorizontalOffset = ExternalHorizontalScrollBar->Maximum;

			ExternalHorizontalScrollBar->Value = HorizontalOffset;
		}
	}
	SynchronizingExternalScrollBars = false;
}

void AvalonEditTextEditor::ResetExternalScrollBars()
{
	ExternalVerticalScrollBar->Maximum = 1;
	ExternalVerticalScrollBar->Minimum = 0;
	ExternalVerticalScrollBar->Value = 0;

	ExternalHorizontalScrollBar->Maximum = 1;
	ExternalHorizontalScrollBar->Minimum = 0;
	ExternalHorizontalScrollBar->Value = 0;
}

RTBitmap^ AvalonEditTextEditor::RenderFrameworkElement(System::Windows::FrameworkElement^ Element)
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

void AvalonEditTextEditor::MoveTextSegment( AvalonEdit::Document::ISegment^ Segment, eMoveSegmentDirection Direction )
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
	case eMoveSegmentDirection::Up:
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
	case eMoveSegmentDirection::Down:
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
	LineTracker->LineBackgroundRenderer->OpenCloseBraces->Enabled = false;

	if (!TextField->TextArea->Selection->IsEmpty)
		LineTracker->LineBackgroundRenderer->Redraw();

	DocumentLine^ CurrentLine = TextField->Document->GetLineByOffset(CaretPos);
	int OpenBraceOffset = -1, CloseBraceOffset = -1, RelativeCaretPos = -1;
	obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();
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

						if (CurrentBracket->GetType() == Buffer.GetType() && CurrentBracket->GetKind() == BracketSearchData::eBracketState::Opening)
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

	LineTracker->LineBackgroundRenderer->OpenCloseBraces->StartOffset = OpenBraceOffset;
	LineTracker->LineBackgroundRenderer->OpenCloseBraces->EndOffset = CloseBraceOffset;

	BracketStack->Clear();
	ParsedBracketList->Clear();

	LineTracker->LineBackgroundRenderer->Redraw();
}

AvalonEditHighlightingDefinition^ AvalonEditTextEditor::CreateSyntaxHighlightDefinitions( bool UpdateStableDefs )
{
	if (UpdateStableDefs)
		SyntaxHighlightingManager->UpdateBaseDefinitions();

	List<String^>^ LocalVars = gcnew List<String^>();
	if (SemanticAnalysisCache)
	{
		for each (obScriptParsing::Variable^ Itr in SemanticAnalysisCache->Variables)
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
		SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::SuppressOnce);

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
	else if (Index > TextField->Text->Length)
		Index = TextField->Text->Length - 1;

	return TextField->Document->GetLocation(Index).Line;
}

bool AvalonEditTextEditor::GetCharIndexInsideCommentSegment(int Index)
{
	bool Result = true;

	if (Index >= 0 && Index < TextField->Text->Length)
	{
		AvalonEdit::Document::DocumentLine^ Line = TextField->Document->GetLineByOffset(Index);
		obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();
		LocalParser->Tokenize(TextField->Document->GetText(Line), false);
		if (LocalParser->GetCommentTokenIndex(LocalParser->GetTokenIndex(GetTokenAtLocation(Index, false))) == -1)
			Result = false;
	}

	return Result;
}

String^ AvalonEditTextEditor::GetTokenAtMouseLocation()
{
	return GetTokenAtLocation(GetLastKnownMouseClickOffset(), false)->Replace("\r\n", "")->Replace("\n", "");
}

array<String^>^ AvalonEditTextEditor::GetTokensAtMouseLocation()
{
	return GetTokenAtLocation(GetLastKnownMouseClickOffset());
}

int AvalonEditTextEditor::GetLastKnownMouseClickOffset()
{
	if (LastKnownMouseClickOffset < 0 || LastKnownMouseClickOffset > GetTextLength())
		LastKnownMouseClickOffset = GetTextLength();

	return LastKnownMouseClickOffset;
}

void AvalonEditTextEditor::PerformCommentOperationOnSingleLine(int Line, eCommentOperation Operation)
{
	if (GetTextLength() == 0)
		return;
	else if (Line < 1 || Line > LineCount)
		return;

	AvalonEdit::Document::DocumentLine^ LineSegment = TextField->TextArea->Document->GetLineByNumber(Line);
	ISegment^ WhitespaceLeading = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(TextField->TextArea->Document, LineSegment);

	char FirstChar = WhitespaceLeading->EndOffset >= TextField->TextArea->Document->TextLength ? Char::MinValue :
		TextField->TextArea->Document->GetCharAt(WhitespaceLeading->EndOffset);

	switch (Operation)
	{
	case eCommentOperation::Add:
		TextField->TextArea->Document->Insert(LineSegment->Offset, ";");
		break;
	case eCommentOperation::Remove:
		if (FirstChar == ';')
			TextField->TextArea->Document->Replace(WhitespaceLeading->EndOffset, 1, "");

		break;
	case eCommentOperation::Toggle:
		if (FirstChar == ';')
			TextField->TextArea->Document->Replace(WhitespaceLeading->EndOffset, 1, "");
		else if (FirstChar != ';')
			TextField->TextArea->Document->Insert(LineSegment->Offset, ";");

		break;
	}
}

void AvalonEditTextEditor::PerformCommentOperationOnSelection(eCommentOperation Operation)
{
	auto TextSelection = TextField->TextArea->Selection;
	if (TextSelection->IsEmpty)
		return;

	BeginUpdate();

	auto ProcessedLines = gcnew List<UInt32>;
	for each (AvalonEdit::Document::ISegment^ Itr in TextSelection->Segments)
	{
		AvalonEdit::Document::DocumentLine^ FirstLine = TextField->TextArea->Document->GetLineByOffset(Itr->Offset);
		AvalonEdit::Document::DocumentLine^ LastLine = TextField->TextArea->Document->GetLineByOffset(Itr->EndOffset);

		for (AvalonEdit::Document::DocumentLine^ Itr = FirstLine; Itr != LastLine->NextLine && Itr != nullptr; Itr = Itr->NextLine)
		{
			if (ProcessedLines->Contains(Itr->LineNumber) == false)
			{
				PerformCommentOperationOnSingleLine(Itr->LineNumber, Operation);
				ProcessedLines->Add(Itr->LineNumber);
			}
		}
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

void AvalonEditTextEditor::ToggleSearchPanel(bool State)
{
	if (State)
	{
		if (InlineSearchPanel->IsClosed)
			InlineSearchPanel->Open();

		// cache beforehand as changing the search panel's property directly updates the preferences
		bool CaseInsensitive = preferences::SettingsHolder::Get()->FindReplace->CaseInsensitive;
		bool MatchWholeWord = preferences::SettingsHolder::Get()->FindReplace->MatchWholeWord;
		bool UseRegEx = preferences::SettingsHolder::Get()->FindReplace->UseRegEx;

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

System::String^ AvalonEditTextEditor::GetCurrentLineText(bool ClipAtCaretPos)
{
	auto Line = TextField->Document->GetLineByOffset(Caret);
	if (Line == nullptr)
		return String::Empty;

	String^ Contents = TextField->Document->GetText(Line);
	if (ClipAtCaretPos)
		Contents = Contents->Substring(0, Caret - Line->Offset);

	return Contents;
}

bool AvalonEditTextEditor::RaiseIntelliSenseInput(intellisense::IntelliSenseInputEventArgs::Event Type, System::Windows::Input::KeyEventArgs^ K, System::Windows::Input::MouseButtonEventArgs^ M)
{
	Debug::Assert(ActivatedInView == true);

	if (TextFieldDisplayingStaticText)
		return false;

	intellisense::IntelliSenseInputEventArgs^ E = nullptr;
	switch (Type)
	{
	case intellisense::IntelliSenseInputEventArgs::Event::KeyDown:
	case intellisense::IntelliSenseInputEventArgs::Event::KeyUp:
		{
			Int32 KeyState = System::Windows::Input::KeyInterop::VirtualKeyFromKey(K->Key);

			if ((K->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Control) == System::Windows::Input::ModifierKeys::Control)
				KeyState |= (int)Keys::Control;
			if ((K->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Alt) == System::Windows::Input::ModifierKeys::Alt)
				KeyState |= (int)Keys::Alt;
			if ((K->KeyboardDevice->Modifiers & System::Windows::Input::ModifierKeys::Shift) == System::Windows::Input::ModifierKeys::Shift)
				KeyState |= (int)Keys::Shift;

			E = gcnew intellisense::IntelliSenseInputEventArgs(Type, gcnew Windows::Forms::KeyEventArgs((Keys)KeyState));

			break;
		}
	case intellisense::IntelliSenseInputEventArgs::Event::MouseDown:
	case intellisense::IntelliSenseInputEventArgs::Event::MouseUp:
		{
			// Left unimplemented as it's currently not consumed by IntelliSense
			break;
		}
	}

	if (E == nullptr)
		return false;

	IntelliSenseInput(this, E);

	return E->Handled;
}

void AvalonEditTextEditor::RaiseIntelliSenseInsightHover(intellisense::IntelliSenseInsightHoverEventArgs::Event Type, int Offset, Windows::Point Location)
{
	Debug::Assert(ActivatedInView == true);

	if (GetTextLength() == 0)
		return;

	intellisense::IntelliSenseInsightHoverEventArgs^ E = gcnew intellisense::IntelliSenseInsightHoverEventArgs(Type);
	E->Type = Type;

	if (E->Type == intellisense::IntelliSenseInsightHoverEventArgs::Event::HoverStop)
	{
		IntelliSenseInsightHover(this, E);
		return;
	}

	E->Line = GetLineNumberFromCharIndex(Offset);
	E->HoveringOverComment = GetCharIndexInsideCommentSegment(Offset);

	auto ValidatorErrors = ParentScriptDocument->GetMessages(E->Line,
															 model::components::ScriptDiagnosticMessage::eMessageSource::Validator,
															 model::components::ScriptDiagnosticMessage::eMessageType::Error);
	auto CompilerErrors = ParentScriptDocument->GetMessages(E->Line,
															model::components::ScriptDiagnosticMessage::eMessageSource::Compiler,
															model::components::ScriptDiagnosticMessage::eMessageType::Error);

	if (E->HoveringOverComment && CompilerErrors->Count == 0 && ValidatorErrors->Count == 0)
		return;

	for each (auto Itr in CompilerErrors)
		E->ErrorMessagesForHoveredLine->Add(Itr->Text);
	for each (auto Itr in ValidatorErrors)
		E->ErrorMessagesForHoveredLine->Add(Itr->Text);

	array<Tuple<Char, Char>^>^ Delimiters = gcnew array<Tuple<Char, Char>^>(3);
	array<String^>^ Tokens = GetTokenAtLocation(Offset, Delimiters);

	E->HoveredToken = Tokens[1];
	E->PreviousToken = Tokens[0];
	E->DotOperatorInUse = Delimiters[1]->Item1 == '.';

	//Location.X += System::Windows::SystemParameters::CursorWidth;
	VisualLine^ Current = TextField->TextArea->TextView->GetVisualLine(CurrentLine);
	if (Current)
		Location.Y += Current->Height;
	else
		Location.Y += preferences::SettingsHolder::Get()->Appearance->TextFont->Size;

	Location = TextField->PointToScreen(Location);
	E->DisplayScreenCoords = Point(Location.X, Location.Y);

	IntelliSenseInsightHover(this, E);
}

void AvalonEditTextEditor::RaiseIntelliSenseContextChange(intellisense::IntelliSenseContextChangeEventArgs::Event Type)
{
	if (TextFieldDisplayingStaticText)
		return;

	intellisense::IntelliSenseContextChangeEventArgs^ E = gcnew intellisense::IntelliSenseContextChangeEventArgs(Type);

	if (Type == intellisense::IntelliSenseContextChangeEventArgs::Event::Reset)
	{
		IntelliSenseContextChange(this, E);
		return;
	}

	E->CaretPos = Caret;
	E->CurrentLineNumber = CurrentLine;
	E->CurrentLineStartPos = TextField->Document->GetLineByNumber(CurrentLine)->Offset;
	E->ClippedLineText = GetCurrentLineText(true);

	if (Type == intellisense::IntelliSenseContextChangeEventArgs::Event::ScrollOffsetChanged)
		E->CurrentLineInsideViewport = GetLineVisible(CurrentLine, true);

	if (Type == intellisense::IntelliSenseContextChangeEventArgs::Event::SemanticAnalysisCompleted)
		E->SemanticAnalysisData = SemanticAnalysisCache;

	auto DisplayScreenCoords = PointToScreen(GetPositionFromCharIndex(Caret, true));
	DisplayScreenCoords.X += 5;

	VisualLine^ Current = TextField->TextArea->TextView->GetVisualLine(CurrentLine);
	if (Current)
		DisplayScreenCoords.Y += Current->Height + 5;
	else
		DisplayScreenCoords.Y += preferences::SettingsHolder::Get()->Appearance->TextFont->Size + 3;

	E->DisplayScreenCoords = DisplayScreenCoords;
	IntelliSenseContextChange(this, E);
}

void AvalonEditTextEditor::OnScriptModified(bool ModificationState)
{
	Debug::Assert(!TextFieldDisplayingStaticText);

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

	auto RelativePos = E->GetPosition(TextField);
	auto EventArgs = gcnew TextEditorMouseClickEventArgs(Buttons,
														 E->ClickCount,
														 RelativePos.X,
														 RelativePos.Y,
														 LastKnownMouseClickOffset);
	auto ScreenPos = TextField->PointToScreen(RelativePos);
	EventArgs->ScreenCoords = Drawing::Point(ScreenPos.X, ScreenPos.Y);

	MouseClick(this, EventArgs);
}

void AvalonEditTextEditor::OnLineChanged()
{
	LineChanged(this, EventArgs::Empty);
}

void AvalonEditTextEditor::OnColumnChanged()
{
	ColumnChanged(this, EventArgs::Empty);
}

void AvalonEditTextEditor::OnTextUpdated()
{
	TextUpdated(this, EventArgs::Empty);
}

void AvalonEditTextEditor::OnLineAnchorInvalidated()
{
	LineAnchorInvalidated(this, EventArgs::Empty);
}

void AvalonEditTextEditor::OnStaticTextDisplayChanged()
{
	StaticTextDisplayChanged(this, EventArgs::Empty);
}

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
		LineTracker->LineBackgroundRenderer->Redraw();
		OnLineChanged();
	}

	if (TextField->TextArea->Caret->Column != PreviousColumnBuffer)
	{
		PreviousColumnBuffer = TextField->TextArea->Caret->Column;
		OnColumnChanged();
	}

	if (TextField->TextArea->Selection->IsEmpty)
		SearchBracesForHighlighting(Caret);

	RaiseIntelliSenseContextChange(intellisense::IntelliSenseContextChangeEventArgs::Event::CaretPosChanged);
}

void AvalonEditTextEditor::TextField_ScrollOffsetChanged(Object^ Sender, EventArgs^ E)
{
	if (SynchronizingInternalScrollBars == false)
		SynchronizeExternalScrollBars();

	System::Windows::Vector CurrentOffset = TextField->TextArea->TextView->ScrollOffset;
	System::Windows::Vector Delta = CurrentOffset - PreviousScrollOffsetBuffer;
	PreviousScrollOffsetBuffer = CurrentOffset;

	RaiseIntelliSenseContextChange(intellisense::IntelliSenseContextChangeEventArgs::Event::ScrollOffsetChanged);
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
	if (ActivatedInView == false)
		return;

	LastKeyThatWentDown = E->Key;

	if (IsMiddleMouseScrolling)
		StopMiddleMouseScroll();

	// return early if someone further up the event chain handled the event
	if (OnKeyDown(E))
	{
		HandleKeyEventForKey(E->Key);
		E->Handled = true;
		return;
	}

	if (RaiseIntelliSenseInput(intellisense::IntelliSenseInputEventArgs::Event::KeyDown, E, nullptr))
	{
		HandleKeyEventForKey(E->Key);
		E->Handled = true;
		return;
	}


	switch (E->Key)
	{
	case System::Windows::Input::Key::F:
	{
		bool Default = preferences::SettingsHolder::Get()->FindReplace->ShowInlineSearchPanel;
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
	case System::Windows::Input::Key::Escape:
		LineTracker->ClearFindResultSegments();
		ToggleSearchPanel(false);

		break;
	case System::Windows::Input::Key::Up:
		if (TextFieldDisplayingStaticText)
			break;

		if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
		{
			SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::SuppressAlways);

			MoveTextSegment(TextField->Document->GetLineByOffset(Caret), eMoveSegmentDirection::Up);

			SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::Propagate);

			HandleKeyEventForKey(E->Key);
			E->Handled = true;
		}

		break;
	case System::Windows::Input::Key::Down:
		if (TextFieldDisplayingStaticText)
			break;

		if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
		{
			SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::SuppressAlways);

			MoveTextSegment(TextField->Document->GetLineByOffset(Caret), eMoveSegmentDirection::Down);

			SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::Propagate);

			HandleKeyEventForKey(E->Key);
			E->Handled = true;
		}

		break;
	case System::Windows::Input::Key::Z:
	case System::Windows::Input::Key::Y:
		if (TextFieldDisplayingStaticText)
			break;

		if (E->KeyboardDevice->Modifiers == System::Windows::Input::ModifierKeys::Control)
			SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::SuppressOnce);

		break;
	}
}

void AvalonEditTextEditor::TextField_KeyUp(Object^ Sender, System::Windows::Input::KeyEventArgs^ E)
{
	if (ActivatedInView == false)
		return;

	if (E->Key == KeyToPreventHandling)
	{
		E->Handled = true;
		KeyToPreventHandling = System::Windows::Input::Key::None;
	}
	else if (RaiseIntelliSenseInput(intellisense::IntelliSenseInputEventArgs::Event::KeyUp, E, nullptr))
		E->Handled = true;
}

void AvalonEditTextEditor::TextField_MouseDown(Object^ Sender, System::Windows::Input::MouseButtonEventArgs^ E)
{
	Nullable<AvalonEdit::TextViewPosition> Location = TextField->GetPositionFromPoint(E->GetPosition(TextField));
	if (Location.HasValue)
		LastKnownMouseClickOffset = TextField->Document->GetOffset(Location.Value.Line, Location.Value.Column);
	else
	{
		Caret = GetTextLength();
		LastKnownMouseClickOffset = -1;
	}
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
	{
		Caret = GetTextLength();
		LastKnownMouseClickOffset = -1;
	}

	OnMouseClick(E);
}

void AvalonEditTextEditor::TextField_MouseWheel(Object^ Sender, System::Windows::Input::MouseWheelEventArgs^ E)
{
	;//
}

void AvalonEditTextEditor::TextField_MouseHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	RaiseIntelliSenseInsightHover(intellisense::IntelliSenseInsightHoverEventArgs::Event::HoverStop,
								-1, Windows::Point(0, 0));

	Nullable<AvalonEdit::TextViewPosition> ViewLocation = TextField->GetPositionFromPoint(E->GetPosition(TextField));
	if (ViewLocation.HasValue)
	{
		LastMouseHoverOffset = TextField->Document->GetOffset(ViewLocation.Value.Line, ViewLocation.Value.Column);
		RaiseIntelliSenseInsightHover(intellisense::IntelliSenseInsightHoverEventArgs::Event::HoverStart,
									LastMouseHoverOffset, TransformToPixels(E->GetPosition(TextField)));
	}
	else
		LastMouseHoverOffset = -1;
}

void AvalonEditTextEditor::TextField_MouseHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	// hacky workaround to prevent the insight tooltip from appearing and disappearing in rapid succession
	// this can happen if the intellisense tooltip appears right under the cursor, triggering the hover stop event right away
	if (LastMouseHoverOffset == OffsetAtCurrentMousePos)
		return;

	RaiseIntelliSenseInsightHover(intellisense::IntelliSenseInsightHoverEventArgs::Event::HoverStop,
								-1, Windows::Point(0, 0));
}

void AvalonEditTextEditor::TextField_MouseMove(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	Nullable<AvalonEdit::TextViewPosition> ViewLocation = TextField->GetPositionFromPoint(E->GetPosition(TextField));
	if (ViewLocation.HasValue)
		OffsetAtCurrentMousePos = TextField->Document->GetOffset(ViewLocation.Value.Line, ViewLocation.Value.Column);
	else
		OffsetAtCurrentMousePos = -1;
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
	if (ActivatedInView == false)
		return;

	SynchronizingInternalScrollBars = false;
	SynchronizeExternalScrollBars();
}

void AvalonEditTextEditor::ExternalScrollBar_ValueChanged( Object^ Sender, EventArgs^ E )
{
	if (SynchronizingExternalScrollBars == false)
	{
		SynchronizingInternalScrollBars = true;

		int VerticalOffset = ExternalVerticalScrollBar->Value;
		int HorizontalOffset = ExternalHorizontalScrollBar->Value;

		if (Sender == ExternalVerticalScrollBar)
			TextField->ScrollToVerticalOffset(VerticalOffset);
		else if (Sender == ExternalHorizontalScrollBar)
			TextField->ScrollToHorizontalOffset(HorizontalOffset);
	}
}

void AvalonEditTextEditor::SetTextAnimation_Completed( Object^ Sender, EventArgs^ E )
{
	if (Disposing)
		return;

	SetTextPrologAnimationCache->Completed -= SetTextAnimationCompletedHandler;
	SetTextPrologAnimationCache = nullptr;

	SAFEDELETE_CLR(AnimationPrimitive->Fill);
	TextFieldPanel->Children->Remove(AnimationPrimitive);
	TextFieldPanel->Children->Add(TextField);

	System::Windows::Media::Animation::DoubleAnimation^ FadeInAnimation = gcnew System::Windows::Media::Animation::DoubleAnimation(0.0,
		1.0,
		System::Windows::Duration(System::TimeSpan::FromMilliseconds(kSetTextFadeAnimationDuration)),
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

	if (preferences::SettingsHolder::Get()->Appearance->ShowCodeFolding)
		CodeFoldingStrategy = gcnew ObScriptCodeFoldingStrategy(this);

	Font^ CustomFont = safe_cast<Font^>(preferences::SettingsHolder::Get()->Appearance->TextFont->Clone());
	SetFont(CustomFont);

	int TabSize = preferences::SettingsHolder::Get()->Appearance->TabSize;

	SetTabCharacterSize(TabSize);

	TextField->Options->CutCopyWholeLine = preferences::SettingsHolder::Get()->General->CutCopyEntireLine;
	TextField->Options->ShowSpaces = preferences::SettingsHolder::Get()->Appearance->ShowSpaces;
	TextField->Options->ShowTabs = preferences::SettingsHolder::Get()->Appearance->ShowTabs;
	TextField->WordWrap = preferences::SettingsHolder::Get()->Appearance->WordWrap;

	if (preferences::SettingsHolder::Get()->General->AutoIndent)
		TextField->TextArea->IndentationStrategy = gcnew ObScriptIndentStrategy(this, true, true);
	else
		TextField->TextArea->IndentationStrategy = gcnew AvalonEdit::Indentation::DefaultIndentationStrategy();

	Color ForegroundColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
	Color BackgroundColor = preferences::SettingsHolder::Get()->Appearance->BackColor;

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

	if (preferences::SettingsHolder::Get()->Appearance->ShowIconMargin)
	{
		if (!TextField->TextArea->LeftMargins->Contains(IconBarMargin))
			TextField->TextArea->LeftMargins->Insert(0, IconBarMargin);
	}
	else
	{
		if (TextField->TextArea->LeftMargins->Contains(IconBarMargin))
			TextField->TextArea->LeftMargins->RemoveAt(0);
	}

	TextField->TextArea->TextView->ElementGenerators->Remove(StructureVisualizer);

	if (preferences::SettingsHolder::Get()->Appearance->ShowBlockVisualizer)
		TextField->TextArea->TextView->ElementGenerators->Add(StructureVisualizer);

	Color Buffer = preferences::SettingsHolder::Get()->Appearance->BackColorFindResults;

	InlineSearchPanel->MarkerBrush = gcnew Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B));

	RefreshTextView();
}

void AvalonEditTextEditor::BackgroundAnalysis_AnalysisComplete(Object^ Sender, model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs^ E)
{
	if (SemanticAnalysisCache)
		SAFEDELETE_CLR(SemanticAnalysisCache);

	SemanticAnalysisCache = E->Result->Clone();

	UpdateCodeFoldings();
	UpdateSyntaxHighlighting(false);

	RaiseIntelliSenseContextChange(intellisense::IntelliSenseContextChangeEventArgs::Event::SemanticAnalysisCompleted);
}

void AvalonEditTextEditor::LineTrackingManager_LineAnchorInvalidated(Object^ Sender, EventArgs^ E)
{
	OnLineAnchorInvalidated();
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
	preferences::SettingsHolder::Get()->FindReplace->CaseInsensitive = E->MatchCase == false;
	preferences::SettingsHolder::Get()->FindReplace->MatchWholeWord = E->WholeWords;
	preferences::SettingsHolder::Get()->FindReplace->UseRegEx = E->UseRegex;
}

AvalonEditTextEditor::AvalonEditTextEditor(model::IScriptDocument^ ParentScriptDocument)
{
	Debug::Assert(ParentScriptDocument != nullptr);
	this->ParentScriptDocument = ParentScriptDocument;

	WinFormsContainer = gcnew Panel();
	WPFHost = gcnew ElementHost();
	TextFieldPanel = gcnew System::Windows::Controls::DockPanel();
	TextField = gcnew AvalonEdit::TextEditor();
	AnimationPrimitive = gcnew System::Windows::Shapes::Rectangle();
	CodeFoldingManager = AvalonEdit::Folding::FoldingManager::Install(TextField->TextArea);
	CodeFoldingStrategy = nullptr;

	if (preferences::SettingsHolder::Get()->Appearance->ShowCodeFolding)
		CodeFoldingStrategy = gcnew ObScriptCodeFoldingStrategy(this);

	MiddleMouseScrollTimer = gcnew Timer();
	ExternalVerticalScrollBar = gcnew DotNetBar::VScrollBarAdv();
	ExternalHorizontalScrollBar = gcnew DotNetBar::ScrollBar::HScrollBarAdv();
	ScrollBarSyncTimer = gcnew Timer();

	BackgroundAnalyzerAnalysisCompleteHandler = gcnew model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler(this, &AvalonEditTextEditor::BackgroundAnalysis_AnalysisComplete);
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
	TextFieldMouseMoveHandler = gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MouseMove);
	TextFieldSelectionChangedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::TextField_SelectionChanged);
	TextFieldLostFocusHandler = gcnew System::Windows::RoutedEventHandler(this, &AvalonEditTextEditor::TextField_LostFocus);
	TextFieldMiddleMouseScrollMoveHandler = gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MiddleMouseScrollMove);
	TextFieldMiddleMouseScrollDownHandler = gcnew System::Windows::Input::MouseButtonEventHandler(this, &AvalonEditTextEditor::TextField_MiddleMouseScrollDown);
	MiddleMouseScrollTimerTickHandler = gcnew EventHandler(this, &AvalonEditTextEditor::MiddleMouseScrollTimer_Tick);
	ScrollBarSyncTimerTickHandler = gcnew EventHandler(this, &AvalonEditTextEditor::ScrollBarSyncTimer_Tick);
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
	TextField->Options->CutCopyWholeLine = preferences::SettingsHolder::Get()->General->CutCopyEntireLine;
	TextField->Options->ShowSpaces = preferences::SettingsHolder::Get()->Appearance->ShowSpaces;
	TextField->Options->ShowTabs = preferences::SettingsHolder::Get()->Appearance->ShowTabs;
	TextField->WordWrap = preferences::SettingsHolder::Get()->Appearance->WordWrap;
	TextField->ShowLineNumbers = true;
	TextField->HorizontalScrollBarVisibility = System::Windows::Controls::ScrollBarVisibility::Hidden;
	TextField->VerticalScrollBarVisibility = System::Windows::Controls::ScrollBarVisibility::Hidden;
	UpdateSyntaxHighlighting(true);

	Color ForegroundColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
	Color BackgroundColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
	auto ForegroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255, ForegroundColor.R, ForegroundColor.G, ForegroundColor.B));
	auto BackgroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255, BackgroundColor.R, BackgroundColor.G, BackgroundColor.B));

	TextField->Foreground = ForegroundBrush;
	TextField->Background = BackgroundBrush;
	TextField->LineNumbersForeground = ForegroundBrush;

	TextField->TextArea->IndentationStrategy = nullptr;
	if (preferences::SettingsHolder::Get()->General->AutoIndent)
		TextField->TextArea->IndentationStrategy = gcnew ObScriptIndentStrategy(this, true, true);
	else
		TextField->TextArea->IndentationStrategy = gcnew AvalonEdit::Indentation::DefaultIndentationStrategy();

	Color Buffer = preferences::SettingsHolder::Get()->Appearance->BackColorFindResults;

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
	IntelliSenseTextChangeEventHandlingMode = eIntelliSenseTextChangeEventHandling::Propagate;
	KeyToPreventHandling = System::Windows::Input::Key::None;
	LastKeyThatWentDown = System::Windows::Input::Key::None;
	IsMiddleMouseScrolling = false;

	MiddleMouseScrollTimer->Interval = 16;

	ActivatedInView = false;

	LastKnownMouseClickOffset = 0;
	OffsetAtCurrentMousePos = -1;
	LastMouseHoverOffset = -1;

	ScrollBarSyncTimer->Interval = 200;

	ExternalVerticalScrollBar->Dock = DockStyle::Right;
	ExternalVerticalScrollBar->Size = Size(16, 100);
	ExternalVerticalScrollBar->SmallChange = 30;
	ExternalVerticalScrollBar->LargeChange = 155;

	ExternalHorizontalScrollBar->Dock = DockStyle::Bottom;
	ExternalHorizontalScrollBar->Size = Size(100, 16);
	ExternalHorizontalScrollBar->SmallChange = 30;
	ExternalHorizontalScrollBar->LargeChange = 155;

	SynchronizingInternalScrollBars = false;
	SynchronizingExternalScrollBars = false;
	PreviousScrollOffsetBuffer = System::Windows::Vector(0.0, 0.0);

	SetTextAnimating = false;
	SetTextPrologAnimationCache = nullptr;

	TextFieldInUpdateFlag = false;
	TextFieldDisplayingStaticText = false;
	PreviousLineBuffer = -1;
	PreviousColumnBuffer = -1;
	SemanticAnalysisCache = gcnew obScriptParsing::AnalysisData();

	LineTracker = gcnew LineTrackingManager(TextField, ParentScriptDocument);
	LineTrackingManagerLineAnchorInvalidatedHandler = gcnew EventHandler(this, &AvalonEditTextEditor::LineTrackingManager_LineAnchorInvalidated);
	LineTracker->LineAnchorInvalidated += LineTrackingManagerLineAnchorInvalidatedHandler;

	IconBarMargin = gcnew DefaultIconMargin(TextField, ParentScriptDocument, WindowHandle);

	if (preferences::SettingsHolder::Get()->Appearance->ShowIconMargin)
		TextField->TextArea->LeftMargins->Insert(0, IconBarMargin);

	BytecodeOffsetMargin = gcnew ScriptBytecodeOffsetMargin(ParentScriptDocument);


	StructureVisualizer = gcnew StructureVisualizerRenderer(this);

	if (preferences::SettingsHolder::Get()->Appearance->ShowBlockVisualizer)
		TextField->TextArea->TextView->ElementGenerators->Add(StructureVisualizer);

	WinFormsContainer->ForeColor = ForegroundColor;
	WinFormsContainer->BackColor = BackgroundColor;
	WinFormsContainer->TabStop = false;
	WinFormsContainer->Dock = DockStyle::Fill;
	WinFormsContainer->BorderStyle = BorderStyle::None;
	WinFormsContainer->Controls->Add(WPFHost);
	WinFormsContainer->Controls->Add(ExternalVerticalScrollBar);
	WinFormsContainer->Controls->Add(ExternalHorizontalScrollBar);

	WPFHost->Dock = DockStyle::Fill;
	WPFHost->Child = TextFieldPanel;
	WPFHost->ForeColor = ForegroundColor;
	WPFHost->BackColor = BackgroundColor;
	WPFHost->TabStop = false;

	Disposing = false;

	SetFont(preferences::SettingsHolder::Get()->Appearance->TextFont);
	SetTabCharacterSize(preferences::SettingsHolder::Get()->Appearance->TabSize);

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
	TextField->PreviewMouseMove += TextFieldMouseMoveHandler;

	ParentScriptDocument->BackgroundAnalyzer->SemanticAnalysisComplete += BackgroundAnalyzerAnalysisCompleteHandler;
	MiddleMouseScrollTimer->Tick += MiddleMouseScrollTimerTickHandler;
	ScrollBarSyncTimer->Tick += ScrollBarSyncTimerTickHandler;
	ExternalVerticalScrollBar->ValueChanged += ExternalScrollBarValueChangedHandler;
	ExternalHorizontalScrollBar->ValueChanged += ExternalScrollBarValueChangedHandler;
	preferences::SettingsHolder::Get()->PreferencesChanged += ScriptEditorPreferencesSavedHandler;
	TextField->TextArea->TextView->VisualLineConstructionStarting += TextFieldVisualLineConstructionStartingHandler;
}

AvalonEditTextEditor::~AvalonEditTextEditor()
{
	Disposing = true;

	if (DisplayingStaticText)
		EndDisplayingStaticText();

	ParentScriptDocument->BackgroundAnalyzer->SemanticAnalysisComplete -= BackgroundAnalyzerAnalysisCompleteHandler;
	ParentScriptDocument = nullptr;

	TextField->Clear();
	MiddleMouseScrollTimer->Stop();
	ScrollBarSyncTimer->Stop();
	CodeFoldingManager->Clear();
	AvalonEdit::Folding::FoldingManager::Uninstall(CodeFoldingManager);
	InlineSearchPanel->Uninstall();

	for each (auto Itr in TextField->TextArea->TextView->BackgroundRenderers)
		delete Itr;

	TextField->TextArea->TextView->BackgroundRenderers->Clear();

	TextField->TextArea->TextView->ElementGenerators->Clear();

	TextField->TextArea->LeftMargins->Clear();

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
	TextField->PreviewMouseMove -= TextFieldMouseMoveHandler;
	MiddleMouseScrollTimer->Tick -= MiddleMouseScrollTimerTickHandler;
	ScrollBarSyncTimer->Tick -= ScrollBarSyncTimerTickHandler;
	ExternalVerticalScrollBar->ValueChanged -= ExternalScrollBarValueChangedHandler;
	ExternalHorizontalScrollBar->ValueChanged -= ExternalScrollBarValueChangedHandler;
	preferences::SettingsHolder::Get()->PreferencesChanged -= ScriptEditorPreferencesSavedHandler;
	TextField->TextArea->TextView->VisualLineConstructionStarting -= TextFieldVisualLineConstructionStartingHandler;
	InlineSearchPanel->SearchOptionsChanged -= SearchPanelSearchOptionsChangedHandler;
	LineTracker->LineAnchorInvalidated -= LineTrackingManagerLineAnchorInvalidatedHandler;

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
	SAFEDELETE_CLR(TextFieldMouseMoveHandler);
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
	SAFEDELETE_CLR(LineTrackingManagerLineAnchorInvalidatedHandler);
	SAFEDELETE_CLR(BackgroundAnalyzerAnalysisCompleteHandler);

	TextFieldPanel->Children->Clear();
	WPFHost->Child = nullptr;
	WinFormsContainer->Controls->Clear();
	WinFormsContainer->ContextMenu = nullptr;

	SAFEDELETE_CLR(BytecodeOffsetMargin);
	SAFEDELETE_CLR(IconBarMargin);
	SAFEDELETE_CLR(LineTracker);
	SAFEDELETE_CLR(StructureVisualizer);
	SAFEDELETE_CLR(InlineSearchPanel);

	SAFEDELETE_CLR(TextField->TextArea->IndentationStrategy);

	SAFEDELETE_CLR(WinFormsContainer);
	SAFEDELETE_CLR(WPFHost);
	SAFEDELETE_CLR(TextFieldPanel);
	SAFEDELETE_CLR(AnimationPrimitive);
	SAFEDELETE_CLR(MiddleMouseScrollTimer);
	SAFEDELETE_CLR(CodeFoldingManager);
	SAFEDELETE_CLR(CodeFoldingStrategy);
	SAFEDELETE_CLR(ScrollBarSyncTimer);
	SAFEDELETE_CLR(ExternalVerticalScrollBar);
	SAFEDELETE_CLR(ExternalHorizontalScrollBar);
	SAFEDELETE_CLR(SemanticAnalysisCache);
	SAFEDELETE_CLR(TextField);
}

void AvalonEditTextEditor::UpdateSyntaxHighlighting(bool Regenerate)
{
	SAFEDELETE_CLR(TextField->SyntaxHighlighting);
	TextField->SyntaxHighlighting = CreateSyntaxHighlightDefinitions(Regenerate);
}

void AvalonEditTextEditor::Bind()
{
	ActivatedInView = true;
	ScrollBarSyncTimer->Start();
	FocusTextArea();
	ScrollToCaret();

	if (!SemanticAnalysisData->Valid)
		ParentScriptDocument->BackgroundAnalyzer->QueueImmediaterBgAnalysis();
}

void AvalonEditTextEditor::Unbind()
{
	ActivatedInView = false;
	ScrollBarSyncTimer->Stop();
	Windows::Input::Keyboard::ClearFocus();
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

void AvalonEditTextEditor::SetText(String^ Text, bool ResetUndoStack)
{
	if (TextFieldDisplayingStaticText)
		throw gcnew InvalidOperationException("Cannot modify text in read-only mode");

	Text = SanitizeUnicodeString(Text);

	SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::SuppressOnce);
	RaiseIntelliSenseContextChange(intellisense::IntelliSenseContextChangeEventArgs::Event::Reset);
	FadeOutCurrentTextView();

	if (ResetUndoStack)
		TextField->Text = Text;
	else
	{
		SetSelectionStart(0);
		SetSelectionLength(GetTextLength());
		SetSelectedText(Text);
		SetSelectionLength(0);
	}

	if (ActivatedInView)
		ParentScriptDocument->BackgroundAnalyzer->QueueImmediaterBgAnalysis();

	OnTextUpdated();
}

String^ AvalonEditTextEditor::GetSelectedText(void)
{
	return TextField->SelectedText;
}

void AvalonEditTextEditor::SetSelectedText(String^ Text)
{
	if (TextFieldDisplayingStaticText)
		throw gcnew InvalidOperationException("Cannot modify text in read-only mode");

	SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::SuppressOnce);

	TextField->SelectedText = Text;
}

int AvalonEditTextEditor::GetCharIndexFromPosition(Point Position)
{
	System::Windows::Point CorrectedPosition(Position.X + TextField->TextArea->TextView->ScrollOffset.X, Position.Y + TextField->TextArea->TextView->ScrollOffset.Y);
	for each (auto Itr in TextField->TextArea->LeftMargins)
		CorrectedPosition.X -= (safe_cast<System::Windows::FrameworkElement^>(Itr))->ActualWidth;

	Nullable<AvalonEdit::TextViewPosition> TextPos = TextField->TextArea->TextView->GetPosition(CorrectedPosition);
	if (TextPos.HasValue)
		return TextField->Document->GetOffset(TextPos.Value.Line, TextPos.Value.Column);
	else
		return GetTextLength();
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

		Result = TransformToPixels(Result.X, Result.Y);
		return Point(Result.X, Result.Y);
	}
}

Point AvalonEditTextEditor::ScreenToClient(Point ScreenPosition)
{
	auto ClientCoords = TextField->PointFromScreen(Windows::Point(ScreenPosition.X, ScreenPosition.Y));
	return Point(ClientCoords.X, ClientCoords.Y);
}

void AvalonEditTextEditor::FadeOutCurrentTextView()
{
	if (Disposing)
		return;
	else if (SetTextAnimating)
		return;
	else if (!ActivatedInView)
		return;

	SetTextAnimating = true;

	TextFieldPanel->Children->Add(AnimationPrimitive);

	AnimationPrimitive->Fill = gcnew System::Windows::Media::VisualBrush(TextField);
	AnimationPrimitive->Height = TextField->ActualHeight;
	AnimationPrimitive->Width = TextField->ActualWidth;

	TextFieldPanel->Children->Remove(TextField);

	System::Windows::Media::Animation::DoubleAnimation^ FadeOutAnimation = gcnew System::Windows::Media::Animation::DoubleAnimation(1.0, 0.0,
		System::Windows::Duration(System::TimeSpan::FromMilliseconds(kSetTextFadeAnimationDuration)),
		System::Windows::Media::Animation::FillBehavior::Stop);
	SetTextPrologAnimationCache = FadeOutAnimation;

	FadeOutAnimation->Completed += SetTextAnimationCompletedHandler;
	System::Windows::Media::Animation::Storyboard^ FadeOutStoryBoard = gcnew System::Windows::Media::Animation::Storyboard();
	FadeOutStoryBoard->Children->Add(FadeOutAnimation);
	FadeOutStoryBoard->SetTargetName(FadeOutAnimation, AnimationPrimitive->Name);
	FadeOutStoryBoard->SetTargetProperty(FadeOutAnimation, gcnew System::Windows::PropertyPath(AnimationPrimitive->OpacityProperty));
	FadeOutStoryBoard->Begin(TextFieldPanel);
}

String^ AvalonEditTextEditor::GetTokenAtCharIndex(int Offset)
{
	return GetTokenAtLocation(Offset, false)->Replace("\r\n", "")->Replace("\n", "");
}

String^ AvalonEditTextEditor::GetTokenAtCaretPos()
{
	return GetTokenAtLocation(Caret - 1, false)->Replace("\r\n", "")->Replace("\n", "");
}

void AvalonEditTextEditor::SetTokenAtCaretPos(String^ Replacement)
{
	if (TextFieldDisplayingStaticText)
		throw gcnew InvalidOperationException("Cannot modify text in read-only mode");

	GetTokenAtLocation(Caret - 1, true);
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

FindReplaceResult^ AvalonEditTextEditor::FindReplace(eFindReplaceOperation Operation, String^ Query, String^ Replacement, eFindReplaceOptions Options)
{
	if (Operation == eFindReplaceOperation::Replace && TextFieldDisplayingStaticText)
		throw gcnew InvalidOperationException("Cannot replace text in read-only mode");

	auto Result = gcnew FindReplaceResult;

	if (Operation != eFindReplaceOperation::CountMatches)
	{
		BeginUpdate();
		LineTracker->ClearFindResultSegments();
	}

	try
	{
		String^ Pattern = "";

		if (Options.HasFlag(eFindReplaceOptions::RegEx))
			Pattern = Query;
		else
		{
			Pattern = System::Text::RegularExpressions::Regex::Escape(Query);
			if (Options.HasFlag(eFindReplaceOptions::MatchWholeWord))
				Pattern = "\\b" + Pattern + "\\b";
		}

		System::Text::RegularExpressions::Regex^ Parser = nullptr;
		if (Options.HasFlag(eFindReplaceOptions::CaseInsensitive))
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

		if (Options.HasFlag(eFindReplaceOptions::InSelection))
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

	if (Operation != eFindReplaceOperation::CountMatches)
	{
		SetSelectionLength(0);
		LineTracker->LineBackgroundRenderer->Redraw();
		EndUpdate(false);
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
		throw gcnew InvalidOperationException("Text editor is already being updated.");

	TextFieldInUpdateFlag = true;
	TextField->Document->BeginUpdate();

	SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::SuppressAlways);
}

void AvalonEditTextEditor::EndUpdate(bool FlagModification)
{
	if (TextFieldInUpdateFlag == false)
		throw gcnew InvalidOperationException("Text editor isn't being updated.");

	TextField->Document->EndUpdate();
	TextFieldInUpdateFlag = false;

	SetIntelliSenseTextChangeEventHandlingMode(eIntelliSenseTextChangeEventHandling::Propagate);

	if (FlagModification)
		Modified = true;
}

UInt32 AvalonEditTextEditor::GetIndentLevel(UInt32 LineNumber)
{
	return SemanticAnalysisCache->GetLineIndentLevel(LineNumber);
}

bool AvalonEditTextEditor::InsertVariable(String^ VariableName, obScriptParsing::Variable::eDataType VariableType)
{
	if (SemanticAnalysisCache->LookupVariable(VariableName))
		return false;

	String^ Declaration = obScriptParsing::Variable::GetVariableDataTypeToken(VariableType) + " " + VariableName + "\n";
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

	return true;
}

ILineAnchor^ AvalonEditTextEditor::CreateLineAnchor(UInt32 Line)
{
	return LineTracker->CreateLineAnchor(Line, true);
}

void AvalonEditTextEditor::InvokeDefaultCopy()
{
	TextField->Copy();
}

void AvalonEditTextEditor::InvokeDefaultPaste()
{
	if (DisplayingStaticText)
		return;

	TextField->Paste();
}

void AvalonEditTextEditor::CommentLine(UInt32 Line)
{
	if (DisplayingStaticText)
		return;

	PerformCommentOperationOnSingleLine(Line, eCommentOperation::Add);
}

void AvalonEditTextEditor::CommentSelection()
{
	if (DisplayingStaticText)
		return;

	PerformCommentOperationOnSelection(eCommentOperation::Add);
}

void AvalonEditTextEditor::UncommentLine(UInt32 Line)
{
	if (DisplayingStaticText)
		return;

	PerformCommentOperationOnSingleLine(Line, eCommentOperation::Remove);
}

void AvalonEditTextEditor::UncommentSelection()
{
	if (DisplayingStaticText)
		return;

	PerformCommentOperationOnSelection(eCommentOperation::Remove);
}

void AvalonEditTextEditor::InitializeState(String^ ScriptText, int CaretPosition)
{
	ResetExternalScrollBars();
	SetText(ScriptText, true);
	Caret = CaretPosition;
	ScrollToCaret();
	Modified = false;
	FocusTextArea();
}

void AvalonEditTextEditor::BeginDisplayingStaticText(String^ TextToDisplay)
{
	Debug::Assert(!TextFieldDisplayingStaticText);

	TextField->IsReadOnly = true;
	TextField->Document->UndoStack->StartUndoGroup();
	SetText(TextToDisplay, false);

	LineTracker->ClearFindResultSegments();

	TextFieldDisplayingStaticText = true;
	OnStaticTextDisplayChanged();
}

void AvalonEditTextEditor::EndDisplayingStaticText()
{
	Debug::Assert(TextFieldDisplayingStaticText);

	FadeOutCurrentTextView();
	TextField->IsReadOnly = false;
	TextField->Document->UndoStack->EndUndoGroup();
	TextField->Document->UndoStack->Undo();
	TextField->Document->UndoStack->ClearRedoStack();
	Caret = 0;

	TextFieldDisplayingStaticText = false;
	OnStaticTextDisplayChanged();
}

void AvalonEditTextEditor::ToggleScriptBytecodeOffsetMargin(bool Enabled)
{
	if (Enabled)
		ScriptBytecodeOffsetMargin::AddToTextArea(TextField, BytecodeOffsetMargin);
	else
		ScriptBytecodeOffsetMargin::RemoveFromTextArea(TextField, BytecodeOffsetMargin);
}


} // namespace avalonEdit


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse