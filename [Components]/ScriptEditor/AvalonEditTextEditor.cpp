#include "AvalonEditTextEditor.h"
#include "ScriptParser.h"
#include "Globals.h"
#include "OptionsDialog.h"

using namespace IntelliSense;
using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;

#pragma region Interface Methods
void AvalonEditTextEditor::SetFont(Font^ FontObject)
{
	TextField->FontFamily = gcnew Windows::Media::FontFamily(FontObject->FontFamily->Name);
	TextField->FontSize = FontObject->Size + 5;
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

void AvalonEditTextEditor::SetText(String^ Text, bool PreventTextChangedEventHandling)
{
	if (PreventTextChangedEventHandling)
		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);

	TextField->Text = Text;
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
		if (LocalParser->IsComment(LocalParser->HasToken(GetTextAtLocation(Index, false))) == -1)
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

int AvalonEditTextEditor::GetCaretPos()
{
	return TextField->TextArea->Caret->Offset;
}

void AvalonEditTextEditor::SetCaretPos(int Index)
{
	TextField->SelectionLength = 0;

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

void AvalonEditTextEditor::LoadFileFromDisk(String^ Path, UInt32 AllocatedIndex)
{
	try
	{
		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_ManualReset);
		TextField->Load(Path);
		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_Disabled);
		DebugPrint("Loaded text from " + Path + " to editor " + AllocatedIndex);
	}
	catch (Exception^ E)
	{
		DebugPrint("Error encountered when opening file for read operation!\n\tError Message: " + E->Message);
	}
}

void AvalonEditTextEditor::SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^% DefaultName, UInt32 AllocatedIndex)
{
	if (PathIncludesFileName == false)
		Path += "\\" + DefaultName + ".txt";

	try
	{
		TextField->Save(Path);
		DebugPrint("Dumped editor " + AllocatedIndex + "'s text to " + Path);
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
		ClearFindResultIndicators();
		ErrorColorizer->ClearLines();
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

UInt32 AvalonEditTextEditor::FindReplace(ScriptTextEditorInterface::FindReplaceOperation Operation, String^ Query, String^ Replacement, ScriptTextEditorInterface::FindReplaceOutput^ Output)
{
	UInt32 Hits = 0;
	ClearFindResultIndicators();

	AvalonEdit::Editing::Selection^ TextSelection = TextField->TextArea->Selection;
	if (TextSelection->IsEmpty)
	{
		for each (DocumentLine^ Line in TextField->Document->Lines)
			Hits += PerformReplaceOnSegment(Operation, Line, Query, Replacement, Output);
	}
	else
	{
		AvalonEdit::Document::DocumentLine ^FirstLine = nullptr, ^LastLine = nullptr;

		for each (AvalonEdit::Document::ISegment^ Itr in TextSelection->Segments)
		{
			FirstLine = TextField->TextArea->Document->GetLineByOffset(Itr->Offset);
			LastLine = TextField->TextArea->Document->GetLineByOffset(Itr->EndOffset);

			for (AvalonEdit::Document::DocumentLine^ Itr = FirstLine; Itr != LastLine->NextLine && Itr != nullptr; Itr = Itr->NextLine)
				Hits += PerformReplaceOnSegment(Operation, Itr, Query, Replacement, Output);
		}
	}

	if (Operation == ScriptTextEditorInterface::FindReplaceOperation::e_Replace)
		FindReplaceColorizer->SetMatch(Replacement);
	else
		FindReplaceColorizer->SetMatch(Query);

	SetSelectionLength(0);

	RefreshUI();

	return Hits;
}

void AvalonEditTextEditor::ToggleComment(int StartIndex)
{
	SetPreventTextChangedFlag(PreventTextChangeFlagState::e_ManualReset);

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

	SetPreventTextChangedFlag(PreventTextChangeFlagState::e_Disabled);
}

void AvalonEditTextEditor::UpdateIntelliSenseLocalDatabase(void)
{
	IntelliSenseBox->UpdateLocalVars();
}

void AvalonEditTextEditor::ClearFindResultIndicators()
{
	FindReplaceColorizer->SetMatch("");
}

void AvalonEditTextEditor::ScrollToLine(String^ LineNumber)
{
	int LineNo = 0;
	try { LineNo = Int32::Parse(LineNumber); } catch (...) { return; }

	GotoLine(LineNo);
}

void AvalonEditTextEditor::HandleTabSwitchEvent(void)
{
	FocusTextArea();
}

void AvalonEditTextEditor::HighlightScriptError(int Line)
{
	ErrorColorizer->AddLine(Line);
	RefreshUI();
}

void AvalonEditTextEditor::ClearScriptErrorHighlights(void)
{
	ErrorColorizer->ClearLines();
	RefreshUI();
}

Point AvalonEditTextEditor::PointToScreen(Point Location)
{
	return Container->PointToScreen(Location);
}

void AvalonEditTextEditor::SetEnabledState(bool State)
{
	WPFHost->Enabled = State;
}

void AvalonEditTextEditor::HandleContainerPositionSizeChangedEvent(void)
{
	IntelliSenseBox->Hide();
}
#pragma endregion

#pragma region Methods
UInt32 AvalonEditTextEditor::PerformReplaceOnSegment(ScriptTextEditorInterface::FindReplaceOperation Operation, AvalonEdit::Document::DocumentLine^ Line, String^ Query, String^ Replacement, ScriptTextEditorInterface::FindReplaceOutput^ Output)
{
	UInt32 Hits = 0;
	String^ CurrentLine = TextField->Document->GetText(Line);

	int Index = 0, Start = 0;
	while ((Index = CurrentLine->IndexOf(Query, Start)) != -1)
	{
		Hits++;
		int EndIndex = Index + Query->Length;

		if (Operation == ScriptTextEditorInterface::FindReplaceOperation::e_Replace)
		{
			TextField->Document->Replace(Line->Offset + Index, Query->Length, Replacement);
		}

		Output(Line->LineNumber.ToString(), TextField->Document->GetText(Line));
		Start = Index + 1;
	}
	return Hits;
}

String^ AvalonEditTextEditor::GetTokenAtIndex(int Index, bool SelectText)
{
	String^% Source = TextField->Text;
	int SearchIndex = Source->Length, SubStrStart = 0, SubStrEnd = SearchIndex;

	if (Index < SearchIndex)
	{
		for (int i = Index; i > 0; i--)
		{
			if (Globals::Delimiters->IndexOf(Source[i]) != -1)
			{
				SubStrStart = i + 1;
				break;
			}
		}

		for (int i = Index; i < SearchIndex; i++)
		{
			if (Globals::Delimiters->IndexOf(Source[i]) != -1)
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

		return Source->Substring(SubStrStart, SubStrEnd - SubStrStart);
	}
}

String^ AvalonEditTextEditor::GetTextAtLocation(Point Location, bool SelectText)
{
	int Index =	GetCharIndexFromPosition(Location);
	return GetTokenAtIndex(Index, SelectText);
}

String^ AvalonEditTextEditor::GetTextAtLocation(int Index, bool SelectText)
{
	return GetTokenAtIndex(Index, SelectText);
}

void AvalonEditTextEditor::GotoLine(int Line)
{
	if (Line > 0 && Line <= TextField->LineCount)
	{
		TextField->TextArea->Caret->Line = Line;
		TextField->TextArea->Caret->Column = 0;
		TextField->Select(TextField->Document->GetLineByNumber(Line)->Offset, TextField->Document->GetLineByNumber(Line)->Length);
		ScrollToCaret();
	}
	else
	{
		MessageBox::Show("Invalid line number/offset", "Goto Line - CSE Editor");
	}
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
					IntelliSenseBox->Initialize(IntelliSenseBox->LastOperation, false, false);
				else
					IntelliSenseBox->Hide();
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
}

void AvalonEditTextEditor::TextField_CaretPositionChanged(Object^ Sender, EventArgs^ E)
{
	static UInt32 LineBuffer = 1;

	if (TextField->TextArea->Caret->Line != LineBuffer)
	{
		IntelliSenseBox->Enabled = true;
		IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;
		LineBuffer = TextField->TextArea->Caret->Line;
		RefreshUI();
	}
}

void AvalonEditTextEditor::TextField_ScrollOffsetChanged(Object^ Sender, EventArgs^ E)
{
	;//
}

void AvalonEditTextEditor::TextField_KeyDown(Object^ Sender, System::Windows::Input::KeyEventArgs^ E)
{
	LastKeyThatWentDown = E->Key;

	if (IsMiddleMouseScrolling)
	{
		StopMiddleMouseScroll();
	}

	int SelStart = TextField->SelectionStart, SelLength = TextField->SelectionLength;

	if (Globals::GetIsDelimiterKey(E->Key))
	{
		IntelliSenseBox->UpdateLocalVars();
		IntelliSenseBox->Enabled = true;

		if (TextField->SelectionStart - 1 >= 0 && !GetCharIndexInsideCommentSegment(TextField->SelectionStart - 1))
		{
			try
			{
				switch (E->Key)
				{
				case System::Windows::Input::Key::OemPeriod:
					{
						IntelliSenseBox->Initialize(IntelliSenseThingy::Operation::e_Dot, false, true);
						SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
						break;
					}
				case System::Windows::Input::Key::Space:
					{
						String^ CommandName = GetTextAtLocation(TextField->SelectionStart - 1, false)->Replace("\n", "");

						if (!String::Compare(CommandName, "call", true))
						{
							IntelliSenseBox->Initialize(IntelliSenseThingy::Operation::e_Call, false, true);
							SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
						}
						else if (!String::Compare(CommandName, "let", true) || !String::Compare(CommandName, "set", true))
						{
							IntelliSenseBox->Initialize(IntelliSenseThingy::Operation::e_Assign, false, true);
							SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
						}
						else
							IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;

						break;
					}
				default:
					{
						IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;
						break;
					}
				}
			}
			catch (Exception^ E)
			{
				DebugPrint("IntelliSense raised an exception while initializing.\n\tException: " + E->Message, true);
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
			if (!IntelliSenseBox->IsVisible())
				IntelliSenseBox->Initialize(IntelliSenseThingy::Operation::e_Default, true, false);

			HandleKeyEventForKey(E->Key);
			E->Handled = true;
		}
		break;
	case System::Windows::Input::Key::Escape:
		if (IntelliSenseBox->IsVisible())
		{
			IntelliSenseBox->Hide();
			IntelliSenseBox->Enabled = false;
			IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;

			HandleKeyEventForKey(E->Key);
			E->Handled = true;
		}
		break;
	case System::Windows::Input::Key::Tab:
		if (IntelliSenseBox->IsVisible())
		{
			SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
			IntelliSenseBox->PickIdentifier();
			FocusTextArea();

			IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;

			HandleKeyEventForKey(E->Key);
			E->Handled = true;
		}
		break;
	case System::Windows::Input::Key::Up:
		if (IntelliSenseBox->IsVisible())
		{
			IntelliSenseBox->MoveIndex(IntelliSenseThingy::Direction::e_Up);

			HandleKeyEventForKey(E->Key);
			E->Handled = true;
		}
		break;
	case System::Windows::Input::Key::Down:
		if (IntelliSenseBox->IsVisible())
		{
			IntelliSenseBox->MoveIndex(IntelliSenseThingy::Direction::e_Down);

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
		if (IntelliSenseBox->IsVisible())
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

	if (IntelliSenseBox->IsVisible())
	{
		IntelliSenseBox->Hide();
		IntelliSenseBox->Enabled = false;
		IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;
	}

	IntelliSenseBox->HideInfoTip();
}

void AvalonEditTextEditor::TextField_MouseWheel(Object^ Sender, System::Windows::Input::MouseWheelEventArgs^ E)
{
	if (IntelliSenseBox->IsVisible())
	{
		if (E->Delta < 0)
			IntelliSenseBox->MoveIndex(IntelliSenseThingy::Direction::e_Down);
		else
			IntelliSenseBox->MoveIndex(IntelliSenseThingy::Direction::e_Up);

		E->Handled = true;
	}
	else
		IntelliSenseBox->HideInfoTip();
}

void AvalonEditTextEditor::TextField_MouseHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	Nullable<AvalonEdit::TextViewPosition> ViewLocation = TextField->GetPositionFromPoint(E->GetPosition(TextField));
	if (ViewLocation.HasValue)
	{
		int Offset = TextField->Document->GetOffset(ViewLocation.Value.Line, ViewLocation.Value.Column);
		Point Location = GetPositionFromCharIndex(Offset);

		if (TextField->Text->Length > 0)
		{
			String^ TextUnderMouse = GetTextAtLocation(Offset, false);

			if (!GetCharIndexInsideCommentSegment(Offset))
				IntelliSenseBox->QuickView(TextUnderMouse, Location);
		}
	}
}

void AvalonEditTextEditor::TextField_MouseHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	IntelliSenseBox->HideInfoTip();
}

void AvalonEditTextEditor::TextField_SelectionChanged(Object^ Sender, EventArgs^ E)
{
	RefreshUI();
}

void AvalonEditTextEditor::TextField_LostFocus(Object^ Sender, System::Windows::RoutedEventArgs^ E)
{
	;//
}

void AvalonEditTextEditor::TextField_MiddleMouseScrollMove(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	static double SlowScrollFactor = 20;

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

void AvalonEditTextEditor::ScrollTimer_Tick(Object^ Sender, EventArgs^ E)
{
	static double AccelerateScrollFactor = 0.0456;

	if (IsMiddleMouseScrolling)
	{
		TextField->ScrollToVerticalOffset(TextField->VerticalOffset + CurrentScrollOffset.Y);
		TextField->ScrollToHorizontalOffset(TextField->HorizontalOffset + CurrentScrollOffset.X);

		CurrentScrollOffset += CurrentScrollOffset * AccelerateScrollFactor;
	}
}
#pragma endregion

AvalonEditTextEditor::AvalonEditTextEditor(Font^ Font, Object^% Parent)
{
	Container = gcnew Panel();
	WPFHost = gcnew ElementHost();
	TextField = gcnew AvalonEdit::TextEditor();
	ErrorColorizer = gcnew AvalonEditScriptErrorBGColorizer(TextField, KnownLayer::Background);
	FindReplaceColorizer = gcnew AvalonEditFindReplaceBGColorizer(TextField, KnownLayer::Background);
	MiddleMouseScrollTimer = gcnew Timer();

	InitializingFlag = false;
	ModifiedFlag = false;
	PreventTextChangedEventFlag = PreventTextChangeFlagState::e_Disabled;
	KeyToPreventHandling = System::Windows::Input::Key::None;
	LastKeyThatWentDown = System::Windows::Input::Key::None;
	IsMiddleMouseScrolling = false;

	MiddleMouseScrollTimer->Interval = 16;
	MiddleMouseScrollTimer->Tick += gcnew EventHandler(this, &AvalonEditTextEditor::ScrollTimer_Tick);

	IntelliSenseBox = gcnew IntelliSenseThingy(Parent);
	LastKnownMouseClickLocation = Point(0, 0);

	Container->Dock = DockStyle::Fill;
	Container->BorderStyle = BorderStyle::Fixed3D;
	Container->Controls->Add(WPFHost);

 	TextField->HorizontalScrollBarVisibility = System::Windows::Controls::ScrollBarVisibility::Hidden;
 	TextField->VerticalScrollBarVisibility = System::Windows::Controls::ScrollBarVisibility::Hidden;
	TextField->Options->AllowScrollBelowDocument = false;
	TextField->Options->EnableEmailHyperlinks = false;
	TextField->Options->EnableHyperlinks = true;
	TextField->Options->RequireControlModifierForHyperlinkClick = true;
	TextField->ShowLineNumbers = true;
	TextField->WordWrap = OPTIONS->FetchSettingAsInt("WordWrap");
	TextField->Options->CutCopyWholeLine = OPTIONS->FetchSettingAsInt("CutCopyEntireLine");
	TextField->Options->ShowSpaces = OPTIONS->FetchSettingAsInt("ShowSpaces");
	TextField->Options->ShowTabs = OPTIONS->FetchSettingAsInt("ShowTabs");

	InitializeSyntaxHighlightingManager(false);

	TextField->SyntaxHighlighting = AvalonEdit::Highlighting::HighlightingManager::Instance->GetDefinition("ObScript");

	TextField->TextChanged += gcnew EventHandler(this, &AvalonEditTextEditor::TextField_TextChanged);
	TextField->TextArea->Caret->PositionChanged += gcnew EventHandler(this, &AvalonEditTextEditor::TextField_CaretPositionChanged);
	TextField->TextArea->SelectionChanged += gcnew EventHandler(this, &AvalonEditTextEditor::TextField_SelectionChanged);
	TextField->LostFocus += gcnew System::Windows::RoutedEventHandler(this, &AvalonEditTextEditor::TextField_LostFocus);
	TextField->TextArea->TextView->ScrollOffsetChanged += gcnew EventHandler(this, &AvalonEditTextEditor::TextField_ScrollOffsetChanged);

	TextField->PreviewKeyUp += gcnew System::Windows::Input::KeyEventHandler(this, &AvalonEditTextEditor::TextField_KeyUp);
	TextField->PreviewKeyDown += gcnew System::Windows::Input::KeyEventHandler(this, &AvalonEditTextEditor::TextField_KeyDown);
	TextField->PreviewMouseDown += gcnew System::Windows::Input::MouseButtonEventHandler(this, &AvalonEditTextEditor::TextField_MouseDown);
	TextField->PreviewMouseWheel += gcnew System::Windows::Input::MouseWheelEventHandler(this, &AvalonEditTextEditor::TextField_MouseWheel);

	TextField->PreviewMouseHover += gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MouseHover);
	TextField->PreviewMouseHoverStopped += gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MouseHoverStopped);

	TextField->PreviewMouseMove += gcnew System::Windows::Input::MouseEventHandler(this, &AvalonEditTextEditor::TextField_MiddleMouseScrollMove);
	TextField->PreviewMouseDown += gcnew System::Windows::Input::MouseButtonEventHandler(this, &AvalonEditTextEditor::TextField_MiddleMouseScrollDown);

	TextField->TextArea->TextView->BackgroundRenderers->Add(ErrorColorizer);
	TextField->TextArea->TextView->BackgroundRenderers->Add(FindReplaceColorizer);
	TextField->TextArea->TextView->BackgroundRenderers->Add(gcnew AvalonEditSelectionBGColorizer(TextField, KnownLayer::Background));
	TextField->TextArea->TextView->BackgroundRenderers->Add(gcnew AvalonEditLineLimitBGColorizer(TextField, KnownLayer::Background));
	TextField->TextArea->TextView->BackgroundRenderers->Add(gcnew AvalonEditCurrentLineBGColorizer(TextField, KnownLayer::Background));

	TextField->TextArea->IndentationStrategy = gcnew AvalonEditObScriptIndentStrategy(true, true);

	WPFHost->Dock = DockStyle::Fill;
	WPFHost->Child = TextField;

	SetFont(Font);
}

void AvalonEditTextEditor::InitializeSyntaxHighlightingManager(bool Reset)
{
	if (SyntaxHighlightingManager->GetInitialized() && !Reset)
		return;
	else
		SyntaxHighlightingManager->Reset();

	SyntaxHighlightingManager->CreateCommentPreprocessorRuleset(OPTIONS->GetColor("SyntaxCommentsColor"), Color::GhostWhite, true, OPTIONS->GetColor("SyntaxPreprocessorColor"));
	SyntaxHighlightingManager->CreateRuleset(AvalonEditXSHDManager::Rulesets::e_Keywords, OPTIONS->GetColor("SyntaxKeywordsColor"), Color::GhostWhite, true);
	SyntaxHighlightingManager->CreateRuleset(AvalonEditXSHDManager::Rulesets::e_BlockTypes, OPTIONS->GetColor("SyntaxScriptBlocksColor"), Color::GhostWhite, true);
	SyntaxHighlightingManager->CreateRuleset(AvalonEditXSHDManager::Rulesets::e_Delimiter, OPTIONS->GetColor("SyntaxDelimitersColor"), Color::GhostWhite, true);
	SyntaxHighlightingManager->CreateRuleset(AvalonEditXSHDManager::Rulesets::e_Digit, OPTIONS->GetColor("SyntaxDigitsColor"), Color::GhostWhite, true);
	SyntaxHighlightingManager->CreateRuleset(AvalonEditXSHDManager::Rulesets::e_String, OPTIONS->GetColor("SyntaxStringsColor"), Color::GhostWhite, true);

	SyntaxHighlightingManager->RegisterDefinitions("ObScript");

	DebugPrint("Initialized syntax highlighting definitions");
}