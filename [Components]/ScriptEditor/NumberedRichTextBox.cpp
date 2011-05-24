#include "NumberedRichTextBox.h"
#include "ScriptParser.h"
#include "[Common]\NativeWrapper.h"

void NumberedRichTextBox::SimpleScrollRTB::WndProc(Message% e)
{
	switch(e.Msg)
	{
	case 0x20A:					// WM_MOUSEWHEEL
		int ZDelta = (int)e.WParam;
		ZDelta = -Math::Sign(ZDelta) * LinesToScroll;
		Message Scroll(e);
		Scroll.Msg = 0xB6;		// EM_LINESCROLL
		Scroll.LParam = (IntPtr)ZDelta;
		Scroll.WParam = IntPtr::Zero;
		RichTextBox::WndProc(Scroll);
		return;
	}
	RichTextBox::WndProc(e);
}

NumberedRichTextBox::NumberedRichTextBox(UInt32 LinesToScroll, Font^ Font, Color ForegroundColor, Color BackgroundColor, Color HighlightColor)
{
	Splitter = gcnew SplitContainer();
	TextField = gcnew SimpleScrollRTB();
	LineField = gcnew RichTextBox();

	this->ForegroundColor = ForegroundColor;
	this->BackgroundColor = BackgroundColor;
	this->HighlightColor = HighlightColor;

	Splitter->Dock = DockStyle::Fill;
	Splitter->FixedPanel = FixedPanel::Panel1;
    Splitter->IsSplitterFixed = true;
    Splitter->SplitterDistance = 40;
    Splitter->SplitterWidth = 1;
	Splitter->BorderStyle = BorderStyle::None;
	
	TextField->LinesToScroll = LinesToScroll;
	TextField->Dock = DockStyle::Fill;
	TextField->Multiline = true;
	TextField->WordWrap = false;
	TextField->BorderStyle = BorderStyle::Fixed3D;
	TextField->AutoWordSelection = true;
	TextField->Font = Font;

	TextField->TextChanged += gcnew EventHandler(this, &NumberedRichTextBox::TextField_TextChanged);

	TextField->Resize += gcnew EventHandler(this, &NumberedRichTextBox::UpdateLineNumbers_EventHandler);
	TextField->VScroll += gcnew EventHandler(this, &NumberedRichTextBox::UpdateLineNumbers_EventHandler);
	TextField->HScroll += gcnew EventHandler(this, &NumberedRichTextBox::UpdateLineNumbers_EventHandler);
	
	TextField->MouseDown += gcnew MouseEventHandler(this, &NumberedRichTextBox::TextField_MouseDownAndUp);
	TextField->MouseUp += gcnew MouseEventHandler(this, &NumberedRichTextBox::TextField_MouseDownAndUp);
	TextField->KeyUp += gcnew KeyEventHandler(this, &NumberedRichTextBox::TextField_KeyDownAndUp);
	
	this->LineChanged += gcnew LineChangedEventHandler(this, &NumberedRichTextBox::NumberedRichTextBox_LineChanged);

	LineField->Multiline = true;
	LineField->WordWrap = false;
	LineField->ReadOnly = true;
	LineField->Cursor = Cursors::Default;
    LineField->Text = "";
	LineField->ScrollBars = RichTextBoxScrollBars::None;
	LineField->BorderStyle = BorderStyle::Fixed3D;
	LineField->BackColor = BackgroundColor;
	LineField->ForeColor = ForegroundColor;
	LineField->MouseDown += gcnew MouseEventHandler(this, &NumberedRichTextBox::LineField_MouseDown);
	LineField->SelectionAlignment = HorizontalAlignment::Right;
	LineField->Dock = DockStyle::Fill;
	LineField->HideSelection = true;
	LineField->Font = Font;

	Splitter->Panel1->Controls->Add(LineField);
	Splitter->Panel1->BorderStyle = BorderStyle::None;
	Splitter->Panel2->Controls->Add(TextField);

	LineChangeBuffer = 0;
}

void NumberedRichTextBox::ValidateLineChange(void)
{
	int LineNo = TextField->GetLineFromCharIndex(TextField->SelectionStart);
	if (LineNo != LineChangeBuffer)
	{
		OnLineChanged(gcnew LineChangedEventArgs(LineNo, LineChangeBuffer));
		LineChangeBuffer = LineNo;
	}
}

void NumberedRichTextBox::OnLineChanged(LineChangedEventArgs^ E)
{
	LineChanged(this, E);
}


void NumberedRichTextBox::TextField_TextChanged(Object^ Sender, EventArgs^ E)
{
	ValidateLineChange();
}

void NumberedRichTextBox::TextField_KeyDownAndUp(Object^ Sender, KeyEventArgs^ E)
{
	ValidateLineChange();
}

void NumberedRichTextBox::TextField_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
{
	ValidateLineChange();
}

void NumberedRichTextBox::TextField_MouseDownAndUp(Object^ Sender, MouseEventArgs^ E)
{
	ValidateLineChange();
}

void NumberedRichTextBox::NumberedRichTextBox_LineChanged(Object^ Sender, LineChangedEventArgs^ E)
{
	UpdateLineNumbers();
}

void NumberedRichTextBox::LineField_MouseDown(Object^ Sender, MouseEventArgs^ E)
{
	TextField->Focus();
	int LineNo = 0, SelStart = LineField->SelectionStart; 

	try 
	{ 
		if (SelStart != -1 &&
			LineField->GetLineFromCharIndex(SelStart) < LineField->Lines->Length && LineField->Lines->Length > 0)
		{
			String^ Selection = LineField->Lines[LineField->GetLineFromCharIndex(SelStart)]->Replace(" ", "");
			LineNo = int::Parse(Selection) - 1;
		}
		else
			return;

		TextField->SelectionStart = TextField->GetFirstCharIndexFromLine(LineNo);
		TextField->SelectionLength = TextField->Lines[LineNo]->Length + 1;
	}
	catch (...)
	{
		return;
	}
}

void NumberedRichTextBox::DrawLineNumbers(void)
{
	int FirstLine = TextField->GetLineFromCharIndex(TextField->GetCharIndexFromPosition(Point(5, 5))) + 1;
	int LastLine = TextField->GetLineFromCharIndex(TextField->GetCharIndexFromPosition(Point(TextField->Width, TextField->Height))) + 2;

	LineField->Clear();
	LineField->SelectionColor = ForegroundColor;
	LineField->SelectionFont = LineField->Font;

	for (int i = FirstLine; i <= LastLine; i++)
	{
		if (i <= TextField->Lines->Length)
			LineField->Text += i + " \n";
	}
}

void NumberedRichTextBox::UpdateLineNumbers(void)
{
	if (TextField->Text == "")		return;

	try
	{
		NativeWrapper::LockWindowUpdate(LineField->Handle);
		DrawLineNumbers();
		HighlightLineNumbers();
	}
	finally
	{
		NativeWrapper::LockWindowUpdate(IntPtr::Zero);
	}
}

void NumberedRichTextBox::HighlightLineNumbers(void)
{
	if (TextField->Text != "")
	{
		int CurrentLine = 0;
		Font^ BoldStyle = gcnew Font(LineField->Font->FontFamily, LineField->Font->Size, FontStyle::Bold);

		CurrentLine = TextField->GetLineFromCharIndex(TextField->SelectionStart) + 1;
		if (SelectLineNumberInLineField(CurrentLine) != -1) 
		{
			LineField->SelectionColor = HighlightColor;
			LineField->SelectionFont = BoldStyle;
		}
	}
}

int NumberedRichTextBox::SelectLineNumberInLineField(UInt32 Line)
{
	return LineField->Find(Line.ToString(), 0, LineField->Text->Length, RichTextBoxFinds::WholeWord);
}

void NumberedRichTextBox::GotoLine(int Line)
{
	if (Line > TextField->Lines->Length || !Line) 
	{
		MessageBox::Show("Invalid line number/offset", "Goto Line - CSE Editor");
	}
	else
	{
		Line -= 1;
		TextField->Focus();
		TextField->SelectionStart = TextField->GetFirstCharIndexFromLine(Line);
		TextField->SelectionLength = TextField->Lines[Line]->Length + 1;
		TextField->ScrollToCaret();
	}
}

void NumberedRichTextBox::JumpToLine(String^ Line)
{
	int LineNo = 0;
	try { LineNo = Int32::Parse(Line); } catch (...) { return; }

	GotoLine(LineNo);
}



OffsetRichTextBox::OffsetRichTextBox(UInt32 LinesToScroll, Font^ Font, Color ForegroundColor, Color BackgroundColor, Color HighlightColor) : NumberedRichTextBox(LinesToScroll, Font, ForegroundColor, BackgroundColor, HighlightColor)
{
	OffsetFlag = false;
	LineOffsets = gcnew List<UInt16>();
}

UInt16 OffsetRichTextBox::LookupOffsetByIndex(UInt32 Index)
{
	if (Index < LineOffsets->Count)
		return LineOffsets[Index];
	else
		return 0xFFFF;
}

void OffsetRichTextBox::JumpToLine(String^ Line)
{
	int LineNo = 0;
	try 
	{ 
		if (!OffsetFlag)
			LineNo = Int32::Parse(Line); 
		else 
		{
			UInt16 Offset = UInt16::Parse(Line, System::Globalization::NumberStyles::HexNumber);
			LineNo = GetIndexOfOffset(Offset) + 1;
		}
	} catch (...) { return;}

	GotoLine(LineNo);
}

int OffsetRichTextBox::GetIndexOfOffset(UInt16 Offset)
{
	int Result = -1, Count = 0;
	
	for each (UInt16 Itr in LineOffsets)
	{
		if (Itr == Offset)
		{
			Result = Count;
			break;
		}
		else
			Count++;
	}

	return Result;
}

void OffsetRichTextBox::LineField_MouseDown(Object^ Sender, MouseEventArgs^ E)
{
	TextField->Focus();
	int LineNo = 0, SelStart = LineField->SelectionStart; 

	try 
	{ 
		if (SelStart != -1 &&
			LineField->GetLineFromCharIndex(SelStart) < LineField->Lines->Length && LineField->Lines->Length > 0)
		{
			if (OffsetFlag)
			{
				UInt16 Offset = int::Parse(LineField->Lines[LineField->GetLineFromCharIndex(SelStart)], System::Globalization::NumberStyles::AllowHexSpecifier);
				LineNo = GetIndexOfOffset(Offset);
			}
			else
			{
				String^ Selection = LineField->Lines[LineField->GetLineFromCharIndex(SelStart)]->Replace(" ", "");;
				LineNo = int::Parse(Selection) - 1;
			}
		}
		else
			return;

		TextField->SelectionStart = TextField->GetFirstCharIndexFromLine(LineNo);
		TextField->SelectionLength = TextField->Lines[LineNo]->Length + 1;
	}
	catch (...)
	{
		return;
	}		
}

void OffsetRichTextBox::DrawLineNumbers(void)
{
	NativeWrapper::LockWindowUpdate(LineField->Handle);
	int FirstLine = TextField->GetLineFromCharIndex(TextField->GetCharIndexFromPosition(Point(5, 5))) + 1;
	int LastLine = TextField->GetLineFromCharIndex(TextField->GetCharIndexFromPosition(Point(TextField->Width, TextField->Height))) + 2;

	LineField->Clear();
	LineField->SelectionColor = ForegroundColor;
	LineField->SelectionFont = LineField->Font;

	for (int i = FirstLine; i <= LastLine; i++)
	{
		if (i <= TextField->Lines->Length)
		{
			if (OffsetFlag)
			{
				try 
				{
					UInt32 Offset = LookupOffsetByIndex(i - 1);
					LineField->Text += ((Offset == 0xFFFF)? "":Offset.ToString("X4")) + "\n";
				} 
				catch (...)	{}
			}
			else
				LineField->Text += i + " \n";
		}
	}
}

void OffsetRichTextBox::HighlightLineNumbers(void)
{
	if (TextField->Text != "")
	{
		int CurrentLine = 0;
		Font^ BoldStyle = gcnew Font(LineField->Font->FontFamily, LineField->Font->Size, FontStyle::Bold);

		CurrentLine = TextField->GetLineFromCharIndex(TextField->SelectionStart) + 1;
		if (SelectLineNumberInLineField(CurrentLine) != -1) 
		{
			LineField->SelectionColor = HighlightColor;
			LineField->SelectionFont = BoldStyle;
		}
	}
}

int OffsetRichTextBox::SelectLineNumberInLineField(UInt32 Line)
{
	if (OffsetFlag && Line - 1 < TextField->Lines->Length)
	{
		try 
		{
			UInt32 Offset = LookupOffsetByIndex(Line - 1);
			if (Offset != 0xFFFF)
				return LineField->Find(Offset.ToString("X4"), 0, RichTextBoxFinds::WholeWord);
		} 
		catch (...) {}
	} 
	else
		return LineField->Find(Line.ToString(), 0, LineField->Text->Length, RichTextBoxFinds::WholeWord);

	return -1;
}

bool OffsetRichTextBox::CalculateLineOffsetsForTextField(UInt32 Data, UInt32 Length)
{
	bool Result = false;

	try
	{
		ClearOffsets();
		if (Data)
		{
			UInt8* DataPtr = (UInt8*)Data;
			Array^ ByteCode = Array::CreateInstance(Byte::typeid, Length);

			for (UInt32 i = 0; i < Length; i++) {
				ByteCode->SetValue(*(DataPtr + i), (int)i);
			}

			UInt32 ScriptOffset = 0, CurrentOffset = 0, SkipOffset = 0;

			array<String^>^ Lines = TextField->Text->Split('\n');
			ScriptParser^ TextParser = gcnew ScriptParser();
			for each (String^% Itr in Lines)
			{
				TextParser->Tokenize(Itr, false);
				CurrentOffset = ByteCodeParser::GetOffsetForLine(Itr, ByteCode, ScriptOffset);
				AddOffsetToList(CurrentOffset);
			}
		}

		if (GetOffsetCount() < 1)				Result = false;
		else									Result = true;
	}
	catch (...)		// exceptions raised when bytecode size doesn't correspond to text length
	{				// can't be predicted as scripts can be saved without being compiled
		Result = false;
	}	
	return Result;
}