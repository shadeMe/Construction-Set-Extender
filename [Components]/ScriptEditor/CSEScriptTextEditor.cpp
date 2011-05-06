#include "[Common]\NativeWrapper.h"
#include "CSEScriptTextEditor.h"
#include "ScriptParser.h"
#include "OptionsDialog.h"
#include "IntelliSense.h"
#include "Globals.h"
#include "ScriptEditor.h"

#pragma region Interface Methods
void CSEScriptTextEditor::SetFont(Font^ FontObject)
{
	LineField->Font = FontObject;
	TextField->Font = FontObject;
}

void CSEScriptTextEditor::SetTabCharacterSize(int PixelWidth)
{
	Array^ TabStops = Array::CreateInstance(int::typeid, 32);
	for (int i = 0; i < 32; i++)
		TabStops->SetValue(PixelWidth * i, i);
	TextField->SelectionTabs = static_cast<array<int>^>(TabStops);
}

void CSEScriptTextEditor::SetContextMenu(ContextMenuStrip^% Strip)
{
	TextField->ContextMenuStrip = Strip;
}

void CSEScriptTextEditor::AddControl(Control^ ControlObject)
{
	TextField->Controls->Add(ControlObject);
}

String^ CSEScriptTextEditor::GetText(void)
{
	return TextField->Text;
}

UInt32 CSEScriptTextEditor::GetTextLength(void)
{
	return TextField->Text->Length;
}

void CSEScriptTextEditor::SetText(String^ Text, bool PreventTextChangedEventHandling)
{
	if (PreventTextChangedEventHandling)
		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);

	TextField->Text = Text;
}

String^ CSEScriptTextEditor::GetSelectedText(void)
{
	return TextField->SelectedText;
}

void CSEScriptTextEditor::SetSelectedText(String^ Text, bool PreventTextChangedEventHandling)
{
	if (PreventTextChangedEventHandling)
		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);

	TextField->SelectedText = Text;
}

void CSEScriptTextEditor::SetSelectionStart(int Index)
{
	TextField->SelectionStart = Index;
}

void CSEScriptTextEditor::SetSelectionLength(int Length)
{
	TextField->SelectionLength = Length;
}

int CSEScriptTextEditor::GetCharIndexFromPosition(Point Position)
{
	return TextField->GetCharIndexFromPosition(Position);
}

Point CSEScriptTextEditor::GetPositionFromCharIndex(int Index)
{
	return TextField->GetPositionFromCharIndex(Index);
}

int CSEScriptTextEditor::GetLineNumberFromCharIndex(int Index)
{
	return TextField->GetLineFromCharIndex(Index);
}

bool CSEScriptTextEditor::GetCharIndexInsideCommentSegment(int Index)
{
	bool Result = true;

	Point Location = TextField->GetPositionFromCharIndex(TextField->SelectionStart);
	int LineNo = TextField->GetLineFromCharIndex(TextField->SelectionStart);

	if (LineNo < TextField->Lines->Length)
	{
		ScriptParser^ LocalParser = gcnew ScriptParser();

		LocalParser->Tokenize(TextField->Lines[LineNo], false);
		if (LocalParser->IsComment(LocalParser->HasToken(GetTextAtLocation(Index, false))) == -1)
			Result = false;
	}

	return Result;
}

int CSEScriptTextEditor::GetCurrentLineNumber(void)
{
	return TextField->GetLineFromCharIndex(TextField->SelectionStart);
}

String^ CSEScriptTextEditor::GetTokenAtCaretPos()
{
	return GetTextAtLocation(TextField->SelectionStart - 1, false)->Replace("\n", "");
}

void CSEScriptTextEditor::SetTokenAtCaretPos(String^ Replacement)
{
	GetTextAtLocation(TextField->SelectionStart - 1, true);
	TextField->SelectedText	= Replacement;
}

String^ CSEScriptTextEditor::GetTokenAtMouseLocation()
{
	return GetTextAtLocation(LastKnownMouseClickLocation, false)->Replace("\n", "");
}

int CSEScriptTextEditor::GetCaretPos()
{
	return TextField->SelectionStart;
}

void CSEScriptTextEditor::SetCaretPos(int Index)
{
	TextField->SelectionLength = 0;

	if (Index > -1)
		TextField->SelectionStart = Index;
	else
		TextField->SelectionStart = 0;

	TextField->ScrollToCaret();
}

void CSEScriptTextEditor::ScrollToCaret()
{
	TextField->ScrollToCaret();
}

IntPtr CSEScriptTextEditor::GetHandle()
{
	return TextField->Handle;
}

void CSEScriptTextEditor::FocusTextArea()
{
	TextField->Focus();
}

void CSEScriptTextEditor::LoadFileFromDisk(String^ Path, UInt32 AllocatedIndex)
{
	try
	{
		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_ManualReset);
		TextField->LoadFile(Path, RichTextBoxStreamType::PlainText);
		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_Disabled);
		DebugPrint("Loaded text from " + Path + " to editor " + AllocatedIndex);
	}
	catch (Exception^ E)
	{
		DebugPrint("Error encountered when opening file for read operation!\n\tError Message: " + E->Message);
	}
}

void CSEScriptTextEditor::SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^% DefaultName, UInt32 AllocatedIndex)
{
	if (PathIncludesFileName == false)
		Path += "\\" + DefaultName + ".txt";

	try
	{
		TextField->SaveFile(Path, RichTextBoxStreamType::PlainText);
		DebugPrint("Dumped editor " + AllocatedIndex + "'s text to " + Path);
	}
	catch (Exception^ E)
	{
		DebugPrint("Error encountered when opening file for write operation!\n\tError Message: " + E->Message);
	}
}

bool CSEScriptTextEditor::GetModifiedStatus()
{
	return ModifiedFlag;
}

void CSEScriptTextEditor::SetModifiedStatus(bool Modified)
{
	ModifiedFlag = Modified;

	switch (Modified)
	{
	case true:
		RemoveFindReplaceResultIndicators();
		break;
	case false:
		break;
	}

	OnScriptModified(gcnew ScriptModifiedEventArgs(Modified));
}

bool CSEScriptTextEditor::GetInitializingStatus()
{
	return InitializingFlag;
}

void CSEScriptTextEditor::SetInitializingStatus(bool Initializing)
{
	InitializingFlag = Initializing;
}

Point CSEScriptTextEditor::GetLastKnownMouseClickLocation()
{
	return LastKnownMouseClickLocation;
}

UInt32 CSEScriptTextEditor::FindReplace(ScriptTextEditorInterface::FindReplaceOperation Operation, String^ Query, String^ Replacement, ScriptTextEditorInterface::FindReplaceOutput^ Output)
{
	UInt32 Hits = 0;
	try
	{
		NativeWrapper::LockWindowUpdate(TextField->Handle);
		RemoveFindReplaceResultIndicators();

		int SelStart = TextField->SelectionStart, SelLength = Query->Length, Pos = -1;

		int Position = TextField->Find(Query, 0, TextField->Text->Length - 1, RichTextBoxFinds::NoHighlight);

		while (Position != -1)
		{
			PlaceFindReplaceResultIndicatorAtCharIndex(Position);

			if (Operation == ScriptTextEditorInterface::FindReplaceOperation::e_Replace)
			{
				SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
				TextField->SelectionStart = Position;
				TextField->SelectionLength = Query->Length;
				TextField->SelectedText = Replacement;
			}

			Output((TextField->GetLineFromCharIndex(Position) + 1).ToString(), TextField->Lines[TextField->GetLineFromCharIndex(Position)]->Replace("\t", ""));

			Position = TextField->Find(Query, Position + Query->Length, TextField->Text->Length - 1, RichTextBoxFinds::NoHighlight);
			Hits++;
		}

//		MessageBox::Show(String::Format("{0} hits in the current script.", Hits), "Find and Replace - CSE Editor");
	}
	finally
	{
		NativeWrapper::LockWindowUpdate(IntPtr::Zero);
	}

	return Hits;
}

void CSEScriptTextEditor::ToggleComment(int StartIndex)
{
	ScriptParser^ LocalParser = gcnew ScriptParser();

	int LineStartIdx = LocalParser->GetLineStartIndex(StartIndex - 1, TextField->Text), SelLength = TextField->SelectionLength;
	if (LineStartIdx < 0)	LineStartIdx = 0;
	int LineEndIdx = LocalParser->GetLineEndIndex(StartIndex, TextField->Text);
	if (LineEndIdx < 0)		LineEndIdx = TextField->Text->Length;

	if (SelLength == 0)
	{
		String^ Line = TextField->Text->Substring(LineStartIdx, LineEndIdx - LineStartIdx);
		LocalParser->Tokenize(Line, false);

		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
		if (!LocalParser->Valid)
		{
			TextField->SelectionStart = LineStartIdx;
			TextField->SelectionLength = 0;
			TextField->SelectedText = ";";
		}
		else
		{
			TextField->SelectionStart = LineStartIdx + LocalParser->Indices[0];

			if (Line->IndexOf(";") == LocalParser->Indices[0])				// is a comment
			{
				TextField->SelectionLength = 1;
				TextField->SelectedText = "";
			}
			else															// not a comment
			{
				TextField->SelectionStart = LineStartIdx;
				TextField->SelectionLength = 0;
				TextField->SelectedText = ";";
			}
		}
	}
	else																// comment each line
	{
		StartIndex = TextField->SelectionStart;
		LineStartIdx = LocalParser->GetLineStartIndex(StartIndex - 1, TextField->Text);
		if (LineStartIdx < 0)	LineStartIdx = 0;
		LineEndIdx = LocalParser->GetLineEndIndex(StartIndex + SelLength, TextField->Text);
		if (LineEndIdx < 0)		LineEndIdx = TextField->Text->Length;

		String^ Lines = TextField->Text->Substring(LineStartIdx, LineEndIdx - LineStartIdx);
		String^ Result;
		String^ ReadLine;

		StringReader^ CommentParser = gcnew StringReader(Lines); ReadLine = CommentParser->ReadLine();

		UInt32 Count = 0, ToggleType = 9;								// 0 - off, 1 - on
		while (ReadLine != nullptr)
		{
			LocalParser->Tokenize(ReadLine, false);
			if (!LocalParser->Valid)
			{
				Result += ";" + ReadLine + "\n";
				ReadLine = CommentParser->ReadLine();
				continue;
			}

			if (ReadLine->IndexOf(";") == LocalParser->Indices[0] && (!Count || !ToggleType))
			{
				Result += ReadLine->Substring(0, LocalParser->Indices[0]);
				if (!Count)		ToggleType = 0;
				Result += ReadLine->Substring(ReadLine->IndexOf(";") + 1);
			}
			else if (ReadLine->IndexOf(";") != LocalParser->Indices[0] && (!Count || ToggleType))
			{
				if (!Count)		ToggleType = 1;
				Result += ";" + ReadLine;
			}
			else
				Result += ReadLine;

			ReadLine = CommentParser->ReadLine();
			if (ReadLine != nullptr)
				Result += "\n";
			Count++;
		}

		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
		TextField->SelectionStart = LineStartIdx;
		TextField->SelectionLength = LineEndIdx - LineStartIdx;
		TextField->SelectedText = Result;
	}

	TextField->SelectionStart = StartIndex;
	TextField->SelectionLength = 0;
}

void CSEScriptTextEditor::UpdateIntelliSenseLocalDatabase(void)
{
	IntelliSenseBox->UpdateLocalVars();
}
#pragma endregion

#pragma region Methods
void CSEScriptTextEditor::ValidateLineLimit()
{
	LineLimitIndicator->Hide();

	UInt32 LineNo = TextField->GetLineFromCharIndex(TextField->GetFirstCharIndexOfCurrentLine()), Length = 0, Index = 0;
	if (LineNo < TextField->Lines->Length)
	{
		Length = TextField->Lines[LineNo]->Length;
		if (Length > 512)
		{
			Index = TextField->GetFirstCharIndexOfCurrentLine() + 511;
			LineLimitIndicator->Location = Point(TextField->GetPositionFromCharIndex(Index).X, TextField->GetPositionFromCharIndex(Index).Y - 15);
			LineLimitIndicator->Tag = String::Format("{0}", Index);
			LineLimitIndicator->Show();
			LineLimitIndicator->BringToFront();
		}
	}
}

void CSEScriptTextEditor::PlaceFindReplaceResultIndicatorAtCharIndex(int Index)
{
	FindReplaceResultIndicators->Add(gcnew PictureBox());

	PictureBox^% Indicator = FindReplaceResultIndicators[FindReplaceResultIndicators->Count - 1];

	Indicator->BorderStyle = BorderStyle::None;
	Indicator->Size = Size(12, 12);
	Indicator->SizeMode = PictureBoxSizeMode::CenterImage;
	Indicator->Location = Point(TextField->GetPositionFromCharIndex(Index).X, TextField->GetPositionFromCharIndex(Index).Y - 25);
	Indicator->Tag = String::Format("{0}", Index);
	Indicator->Image = Globals::ScriptEditorImageResourceManager->CreateImageFromResource("FindReplaceResultIndicator");

	TextField->Controls->Add(Indicator);
}

void CSEScriptTextEditor::UpdateFindReplaceResultLineLimitIndicatorPostions()
{
	if (FindReplaceResultIndicators != nullptr)
	{
		for each (PictureBox^% Itr in FindReplaceResultIndicators)
		{
			Point Loc = TextField->GetPositionFromCharIndex(Int32::Parse(Itr->Tag->ToString()));

			if (Loc.Y < TextField->Height && Loc.Y > 0 && Loc.X > 0 && Loc.Y < TextField->Width)
			{
				Loc.Y -= 8;
				Itr->Location = Loc;
				if (!Itr->Visible)
					Itr->Visible = true;
			}
			else if (Itr->Visible)
				Itr->Visible = false;
		}
	}

	if (LineLimitIndicator->Visible)
	{
		Point Loc = TextField->GetPositionFromCharIndex(Int32::Parse(LineLimitIndicator->Tag->ToString()));
		Loc.Y -= 15;
		LineLimitIndicator->Location = Loc;
	}
}

void CSEScriptTextEditor::RemoveFindReplaceResultIndicators()
{
	for each (PictureBox^% Itr in FindReplaceResultIndicators)
		Itr->Hide();

	FindReplaceResultIndicators->Clear();
}

void CSEScriptTextEditor::HandleHomeKey()
{
	ScriptParser^ LocalParser = gcnew ScriptParser();

	int CaretPos = TextField->SelectionStart, LineStartIdx = LocalParser->GetLineStartIndex(CaretPos - 1, TextField->Text);
	if (LineStartIdx < 0)	LineStartIdx = 0;
	int LineEndIdx = LocalParser->GetLineEndIndex(CaretPos, TextField->Text);
	if (LineEndIdx < 0)		LineEndIdx = TextField->Text->Length;

	String^ Line = TextField->Text->Substring(LineStartIdx, LineEndIdx - LineStartIdx);
	LocalParser->Tokenize(Line, false);
	if (!LocalParser->Valid)
		return;

	int HomeIdx = LineStartIdx + LocalParser->Indices[0];
	if (HomeIdx == CaretPos)
		TextField->SelectionStart = LineStartIdx;
	else
		TextField->SelectionStart = HomeIdx;

	if (Control::ModifierKeys == Keys::Shift)
		TextField->SelectionLength = LineEndIdx - LineStartIdx;
	else
		TextField->SelectionLength = 0;
}

void CSEScriptTextEditor::PerformAutoIndentationProlog(bool CullEmptyLines)
{
	IndentCountBuffer = 0;
	if (!OPTIONS->FetchSettingAsInt("AutoIndent"))
		return;

	int EndIndex = TextField->SelectionStart;

	StringReader^ IndentParser = gcnew StringReader(TextField->Text->Substring(0, EndIndex));
	String^ ReadLine = IndentParser->ReadLine();
	String^ LastLine;

	ScriptParser^ LocalParser = gcnew ScriptParser();
	while (ReadLine != nullptr)
	{
		LocalParser->Tokenize(ReadLine, false);

		if (!LocalParser->Valid)
		{
			LastLine = ReadLine;
			ReadLine = IndentParser->ReadLine();
			continue;
		}

		if (!String::Compare(LocalParser->Tokens[0], "begin", true) ||
			!String::Compare(LocalParser->Tokens[0], "if", true) ||
			!String::Compare(LocalParser->Tokens[0], "while", true) ||
			!String::Compare(LocalParser->Tokens[0], "forEach", true))
		{
			IndentCountBuffer++;
		}
		else if	(!String::Compare(LocalParser->Tokens[0], "loop", true) ||
				!String::Compare(LocalParser->Tokens[0], "endIf", true) ||
				!String::Compare(LocalParser->Tokens[0], "end", true))
		{
			IndentCountBuffer--;
		}

		LastLine = ReadLine;
		ReadLine = IndentParser->ReadLine();
	}

	if (EndIndex + 1 < TextField->Text->Length)
	{
		UInt32 EndChar = EndIndex;
		for (int i = EndIndex; i < TextField->Text->Length; i++)
		{
			if (TextField->Text[i] == '\t' || TextField->Text[i] == ' ')
			{
				EndChar++;
				continue;
			}
			else
				break;
		}

		TextField->SelectionStart = EndIndex;
		TextField->SelectionLength = EndChar - EndIndex;
		TextField->SelectedText = "";
		TextField->SelectionStart = EndIndex;
	}

	bool ExdentLastLine = false;
	if (LastLine != nullptr)					// last line needs to be checked separately for exdents
	{
		ReadLine = LastLine;
		LocalParser->Tokenize(ReadLine, false);
		if (LocalParser->Valid)
		{
			if (!String::Compare(LocalParser->Tokens[0], "loop", true) ||
				!String::Compare(LocalParser->Tokens[0], "endIf", true) ||
				!String::Compare(LocalParser->Tokens[0], "end", true))
			{
				ExdentLastLine = true;
			}
			else if (!String::Compare(LocalParser->Tokens[0], "elseIf", true) ||
					!String::Compare(LocalParser->Tokens[0], "else", true))
			{
				ExdentLastLine = true;
			}
		}
		else if (ReadLine->Replace("\t", "")->Length == 0 && CullEmptyLines)
		{
			int CaretPos = TextField->SelectionStart - 1,
				LineStart = LocalParser->GetLineStartIndex(CaretPos, TextField->Text);

			if (LineStart > -1 && LineStart < CaretPos)
			{
				TextField->SelectionStart = LineStart;
				TextField->SelectionLength = CaretPos - LineStart;
				TextField->SelectedText = "";
			}
		}
	}

	if (IndentCountBuffer < 0)
		IndentCountBuffer = 0;

	if (ExdentLastLine)
	{
		try
		{
			NativeWrapper::LockWindowUpdate(TextField->Handle);

			int SelStart = TextField->SelectionStart - 1, Index = 0;

			for (int i = SelStart; i > Index; --i)
			{
				if (TextField->Text[i] == '\n') {
					Index = i;
					break;
				}
			}

			UInt32 Count = LocalParser->GetTrailingTabCount(Index + 1, TextField->Text, nullptr),
				   Exdents = 0;

			TextField->SelectionStart = Index + 1;
			TextField->SelectionLength = 1;

			if (!String::Compare(LocalParser->Tokens[0], "loop", true) ||
				!String::Compare(LocalParser->Tokens[0], "endIf", true) ||
				!String::Compare(LocalParser->Tokens[0], "end", true))
			{
				Exdents = IndentCountBuffer;
			}
			else
				Exdents = IndentCountBuffer - 1;

			if (Count > Exdents)
				TextField->SelectedText = "";

			if (SelStart + 1 < TextField->Text->Length  && Count > Exdents)
				TextField->SelectionStart = SelStart;
			else
				TextField->SelectionStart = SelStart + 1;

			TextField->SelectionLength = 0;
		}
		finally
		{
			NativeWrapper::LockWindowUpdate(IntPtr::Zero);
		}
	}
}

void CSEScriptTextEditor::PerformAutoIndentationEpilog(void)
{
	if (!IndentCountBuffer)		return;

	try
	{
		NativeWrapper::LockWindowUpdate(TextField->Handle);

		int SelStart = TextField->SelectionStart;
		if (SelStart - 1 > 0)
		{
			UInt32 EndChar = SelStart;
			for (int i = SelStart; i > 0 && i < TextField->Text->Length; i--)
			{
				if (TextField->Text[i] == '\t' || TextField->Text[i] == ' ')
				{
					EndChar--;
					continue;
				}
				else
					break;
			}

			TextField->SelectionStart = SelStart;
			TextField->SelectionLength = SelStart - EndChar;
			TextField->SelectedText = "";
			TextField->SelectionStart = SelStart;
		}

		String^ IndentStr = "";
		for (int i = 0; i < IndentCountBuffer; i++)
			IndentStr += "\t";

		TextField->SelectedText = IndentStr;
	}
	finally
	{
		NativeWrapper::LockWindowUpdate(IntPtr::Zero);
	}
}

bool CSEScriptTextEditor::PerformTabIndent(void)
{
	int SelStart = TextField->SelectionStart,
		Operation = 0;

	String^ Source = TextField->SelectedText;
	String^ Result;

	ScriptParser^ LocalParser = gcnew ScriptParser();

	if (Control::ModifierKeys == Keys::Shift)	Operation = -1;				// exdent
	else										Operation = 1;				// indent

	switch (Operation)
	{
	case 0:
		return false;
	case 1:
	case -1:
		if (Source == nullptr)
			return false;
		else
		{
			LocalParser->Tokenize(Source, false);
			if (!LocalParser->Valid)
				return false;
		}
		break;
	}

	int LineStartIdx = LocalParser->GetLineStartIndex(SelStart - 1, TextField->Text), SelLength = TextField->SelectionLength;
	if (LineStartIdx < 0)	LineStartIdx = 0;
	int LineEndIdx = LocalParser->GetLineEndIndex(SelStart, TextField->Text);
	if (LineEndIdx < 0)		LineEndIdx = TextField->Text->Length;

	if (SelLength)
	{
		SelStart = TextField->SelectionStart;
		LineStartIdx = LocalParser->GetLineStartIndex(SelStart - 1, TextField->Text);
		if (LineStartIdx < 0)	LineStartIdx = 0;
		LineEndIdx = LocalParser->GetLineEndIndex(SelStart + SelLength, TextField->Text);
		if (LineEndIdx < 0)		LineEndIdx = TextField->Text->Length;

		String^ Lines = TextField->Text->Substring(LineStartIdx, LineEndIdx - LineStartIdx);
		StringReader^ TabIndentParser = gcnew StringReader(Lines);
		String^ ReadLine = TabIndentParser->ReadLine();

		while (ReadLine != nullptr)
		{
			LocalParser->Tokenize(ReadLine, false);

			if (!LocalParser->Valid)
			{
				Result += ReadLine + "\n";
				ReadLine = TabIndentParser->ReadLine();
				continue;
			}

			Char Itr = ReadLine[0];
			if (Itr != '\n' && Itr != '\r\n')
			{
				switch (Operation)
				{
				case 1:
					Result += "\t" + ReadLine;
					break;
				case -1:
					if (Itr == '\t')		Result += ReadLine->Substring(1);
					else					Result += ReadLine;
					break;
				}
			}

			ReadLine = TabIndentParser->ReadLine();
			if (ReadLine != nullptr)
				Result += "\n";
		}

		SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
		TextField->SelectionStart = LineStartIdx;
		TextField->SelectionLength = LineEndIdx - LineStartIdx;
		TextField->SelectedText = Result;
		TextField->SelectionStart = SelStart;
		TextField->SelectionLength = Result->Length;
	}
	return true;
}

String^ CSEScriptTextEditor::GetTokenAtIndex(int Index, bool SelectText)
{
	String^% Source = TextField->Text;
	int SearchIndex = Source->Length, SubStrStart = 0, SubStrEnd = SearchIndex;

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

String^ CSEScriptTextEditor::GetTextAtLocation(Point Location, bool SelectText)
{
	int Index =	TextField->GetCharIndexFromPosition(Location);
	return GetTokenAtIndex(Index, SelectText);
}

String^ CSEScriptTextEditor::GetTextAtLocation(int Index, bool SelectText)
{
	return GetTokenAtIndex(Index, SelectText);
}
#pragma endregion

#pragma region Events
void CSEScriptTextEditor::OnTextChanged(EventArgs^ E)
{
	TextChanged(this, E);
}

void CSEScriptTextEditor::OnVScroll(EventArgs^ E)
{
	VScroll(this, E);
}

void CSEScriptTextEditor::OnHScroll(EventArgs^ E)
{
	HScroll(this, E);
}

void CSEScriptTextEditor::OnScriptModified(ScriptModifiedEventArgs^ E)
{
	ScriptModified(this, E);
}

void CSEScriptTextEditor::OnKeyUp(KeyEventArgs^ E)
{
	KeyUp(this, E);
}

void CSEScriptTextEditor::OnKeyDown(KeyEventArgs^ E)
{
	KeyDown(this, E);
}

void CSEScriptTextEditor::OnKeyPress(KeyPressEventArgs^ E)
{
	KeyPress(this, E);
}

void CSEScriptTextEditor::OnMouseDown(MouseEventArgs^ E)
{
	MouseDown(this, E);
}

void CSEScriptTextEditor::OnMouseDoubleClick(MouseEventArgs^ E)
{
	MouseDoubleClick(this, E);
}
#pragma endregion

#pragma region Event Handlers
void CSEScriptTextEditor::TextField_TextChanged(Object^ Sender, EventArgs^ E)
{
	if (InitializingFlag)
	{
		InitializingFlag = false;
		SetModifiedStatus(false);
		if (FindReplaceResultIndicators != nullptr)
			RemoveFindReplaceResultIndicators();
		UpdateLineNumbers();
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
				IntelliSenseBox->Initialize(IntelliSenseBox->LastOperation, false, false);
			}

			ValidateLineLimit();
			OnTextChanged(E);
		}
	}
}

void CSEScriptTextEditor::TextField_Resize(Object^ Sender, EventArgs^ E)
{
	UpdateFindReplaceResultLineLimitIndicatorPostions();
}

void CSEScriptTextEditor::TextField_VScroll(Object^ Sender, EventArgs^ E)
{
	UpdateFindReplaceResultLineLimitIndicatorPostions();
	IntelliSenseBox->HideInfoTip();
	OnVScroll(E);
}

void CSEScriptTextEditor::TextField_HScroll(Object^ Sender, EventArgs^ E)
{
	UpdateFindReplaceResultLineLimitIndicatorPostions();
	IntelliSenseBox->HideInfoTip();
	OnHScroll(E);
}

void CSEScriptTextEditor::TextField_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	int SelStart = TextField->SelectionStart, SelLength = TextField->SelectionLength;

	if (Globals::GetIsDelimiterKey(E->KeyCode))
	{
		IntelliSenseBox->UpdateLocalVars();
		IntelliSenseBox->Enabled = true;

		if (TextField->SelectionStart - 1 >= 0 && !GetCharIndexInsideCommentSegment(TextField->SelectionStart - 1))
		{
			try
			{
				switch (E->KeyCode)
				{
				case Keys::OemPeriod:
					{
						IntelliSenseBox->Initialize(IntelliSenseThingy::Operation::e_Dot, false, true);
						SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
						break;
					}
				case Keys::Space:
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

	switch (E->KeyCode)
	{
	case Keys::V:
		if (E->Modifiers == Keys::Control)
		{
			try
			{
				if (Clipboard::ContainsText())
				{
					String^ CText = Clipboard::GetText(TextDataFormat::Text);
					Clipboard::SetText(CText);
				}
				else
				{
					HandleKeyEventForKey(E->KeyCode);
					E->Handled = true;
				}
			}
			catch (Exception^ Ex)
			{
				DebugPrint("Had trouble stripping RTF elements from clipboard.\n\tException: " + Ex->Message);

				HandleKeyEventForKey(E->KeyCode);
				E->Handled = true;
			}

			SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
		}
		break;
	case Keys::Q:
		if (E->Modifiers == Keys::Control)
		{
			ToggleComment(TextField->SelectionStart);

			HandleKeyEventForKey(E->KeyCode);
			E->Handled = true;
		}
		break;
	case Keys::Enter:
		if (E->Modifiers != Keys::Control)
		{
			PerformAutoIndentationProlog(true);

			if (E->Modifiers == Keys::Shift)
			{
				HandleKeyEventForKey(E->KeyCode);
				E->Handled = true;
			}
		}
		else
		{
			if (!IntelliSenseBox->IsVisible())
				IntelliSenseBox->Initialize(IntelliSenseThingy::Operation::e_Default, true, false);

			HandleKeyEventForKey(E->KeyCode);
			E->Handled = true;
		}
		break;
	case Keys::Escape:
		if (IntelliSenseBox->IsVisible())
		{
			IntelliSenseBox->Hide();
			IntelliSenseBox->Enabled = false;
			IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;

			HandleKeyEventForKey(E->KeyCode);
			E->Handled = true;
		}
		break;
	case Keys::Tab:
		if (IntelliSenseBox->IsVisible())
		{
			SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
			IntelliSenseBox->PickIdentifier();
			FocusTextArea();

			IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;

			HandleKeyEventForKey(E->KeyCode);
			E->Handled = true;
		}
		else
		{
			if (PerformTabIndent())
			{
				HandleKeyEventForKey(E->KeyCode);
				E->Handled = true;
			}
		}
		break;
	case Keys::Up:
		if (IntelliSenseBox->IsVisible())
		{
			IntelliSenseBox->MoveIndex(IntelliSenseThingy::Direction::e_Up);

			HandleKeyEventForKey(E->KeyCode);
			E->Handled = true;
		}
		break;
	case Keys::Down:
		if (IntelliSenseBox->IsVisible())
		{
			IntelliSenseBox->MoveIndex(IntelliSenseThingy::Direction::e_Down);

			HandleKeyEventForKey(E->KeyCode);
			E->Handled = true;
		}
		break;
	case Keys::Home:
		if (!E->Control && !E->Shift)
		{
			HandleHomeKey();

			HandleKeyEventForKey(E->KeyCode);
			E->Handled = true;
		}
		break;
	case Keys::Z:
	case Keys::Y:
		if (E->Modifiers == Keys::Control)
			SetPreventTextChangedFlag(PreventTextChangeFlagState::e_AutoReset);
		break;
	}

	OnKeyDown(E);
}

void CSEScriptTextEditor::TextField_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
{
	if (E->KeyChar == (Char)KeyToPreventHandling)
	{
		E->Handled = true;
		return;
	}

	switch (E->KeyChar)
	{
	case Keys::Enter:
		PerformAutoIndentationEpilog();
		break;
	}
}

void CSEScriptTextEditor::TextField_KeyUp(Object^ Sender, KeyEventArgs^ E)
{
	if (E->KeyCode == KeyToPreventHandling)
	{
		E->Handled = true;
		KeyToPreventHandling = Keys::None;
		return;
	}

	switch (E->KeyCode)
	{
	case Keys::End:
		if (TextField->SelectedText != "")
		{
			try
			{
				NativeWrapper::LockWindowUpdate(TextField->Handle);

				if (TextField->SelectedText[TextField->SelectedText->Length - 1] == '\n' ||
					TextField->SelectedText[TextField->SelectedText->Length - 1] == '\r\n')
				{
					TextField->SelectionLength -= 1;
				}
			}
			finally
			{
				NativeWrapper::LockWindowUpdate(IntPtr::Zero);
			}
		}
		break;
	}
}

void CSEScriptTextEditor::TextField_MouseDown(Object^ Sender, MouseEventArgs^ E)
{
	LastKnownMouseClickLocation = E->Location;

	if (Control::ModifierKeys == Keys::Control)
	{
		GetTextAtLocation(LastKnownMouseClickLocation, true);
	}

	if (IntelliSenseBox->IsVisible())
	{
		IntelliSenseBox->Hide();
		IntelliSenseBox->Enabled = false;
		IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;
	}

	IntelliSenseBox->HideInfoTip();
}

void CSEScriptTextEditor::TextField_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E)
{
	if (E->Button == MouseButtons::Left && TextField->Text->Length > 0)
	{
		String^ TextUnderMouse = GetTextAtLocation(TextField->SelectionStart, true);

		if (!GetCharIndexInsideCommentSegment(TextField->SelectionStart))
		{
			IntelliSenseBox->QuickView(TextUnderMouse);
		}
	}
}

void CSEScriptTextEditor::TextField_LineChanged(Object^ Sender, LineChangedEventArgs^ E)
{
	IntelliSenseBox->Enabled = true;
	IntelliSenseBox->LastOperation = IntelliSenseThingy::Operation::e_Default;
	ValidateLineLimit();
}
#pragma endregion

CSEScriptTextEditor::CSEScriptTextEditor(UInt32 LinesToScroll, Font^ Font, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Object^% Parent) : NumberedRichTextBox(LinesToScroll, Font, ForegroundColor, BackgroundColor, HighlightColor)
{
	IndentCountBuffer = 0;
	LineChangeBuffer = 0;
	InitializingFlag = false;
	ModifiedFlag = false;
	PreventTextChangedEventFlag = PreventTextChangeFlagState::e_Disabled;
	KeyToPreventHandling = Keys::None;

	FindReplaceResultIndicators = gcnew List<PictureBox^>();
	IntelliSenseBox = gcnew IntelliSenseThingy(dynamic_cast<ScriptEditor::Workspace^>(Parent));
	LastKnownMouseClickLocation = Point(0, 0);

	LineLimitIndicator = gcnew PictureBox();
	LineLimitIndicator->Image = Globals::ScriptEditorImageResourceManager->CreateImageFromResource("LineLimitIndicator");;
	LineLimitIndicator->Visible = false;
	LineLimitIndicator->BorderStyle = BorderStyle::None;
	LineLimitIndicator->SizeMode = PictureBoxSizeMode::AutoSize;

	TextField->Controls->Add(LineLimitIndicator);
	TextField->Controls->Add(IntelliSenseBox->InternalListView);

	TextField->TextChanged += gcnew EventHandler(this, &CSEScriptTextEditor::TextField_TextChanged);
	TextField->Resize += gcnew EventHandler(this, &CSEScriptTextEditor::TextField_Resize);

	TextField->VScroll += gcnew EventHandler(this, &CSEScriptTextEditor::TextField_VScroll);
	TextField->HScroll += gcnew EventHandler(this, &CSEScriptTextEditor::TextField_HScroll);

	TextField->KeyUp += gcnew KeyEventHandler(this, &CSEScriptTextEditor::TextField_KeyUp);
	TextField->KeyDown += gcnew KeyEventHandler(this, &CSEScriptTextEditor::TextField_KeyDown);
	TextField->KeyPress += gcnew KeyPressEventHandler(this, &CSEScriptTextEditor::TextField_KeyPress);

	TextField->MouseDown += gcnew MouseEventHandler(this, &CSEScriptTextEditor::TextField_MouseDown);
	TextField->MouseDoubleClick += gcnew MouseEventHandler(this, &CSEScriptTextEditor::TextField_MouseDoubleClick);

	this->LineChanged += gcnew LineChangedEventHandler(this, &CSEScriptTextEditor::TextField_LineChanged);

	IntelliSenseBox->Hide();
}