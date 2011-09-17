#include "AuxiliaryTextEditor.h"
#include "NumberedRichTextBox.h"

ScriptOffsetViewer::ScriptOffsetViewer(Font^ FontData, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent)
{
	Viewer = gcnew OffsetRichTextBox(6, gcnew Font(FontData->FontFamily, FontData->Size + 2, FontStyle::Regular), ForegroundColor, BackgroundColor, HighlightColor);
	InitializationState = false;

	Viewer->OffsetFlag = true;
	Viewer->GetTextField()->ReadOnly = true;
	Viewer->GetTextField()->Enabled = true;
	Viewer->GetContainer()->Visible = false;

	Parent->Controls->Add(Viewer->GetContainer());
}

bool ScriptOffsetViewer::InitializeViewer(String^% ScriptText, UInt32 Data, UInt32 Length)
{
	Viewer->GetTextField()->Text = ScriptText;
	if (Viewer->CalculateLineOffsetsForTextField(Data, Length))
		InitializationState = true;
	else
		InitializationState = false;

	return InitializationState;
}

void ScriptOffsetViewer::Hide()
{
	Viewer->GetContainer()->Hide();
}

bool ScriptOffsetViewer::Show()
{
	if (InitializationState == false)
	{
		MessageBox::Show("Offset Viewer was not initialized successfully!\n\nPlease recompile the current script.", "CSE Script Editor", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
		return false;
	}

	Viewer->GetContainer()->BringToFront();
	Viewer->GetContainer()->Show();
	return true;
}

void ScriptOffsetViewer::Reset( void )
{
	InitializationState = false;
}

SimpleTextViewer::SimpleTextViewer(Font^ FontData, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent)
{
	Viewer = gcnew NumberedRichTextBox(6, gcnew Font(FontData->FontFamily, FontData->Size + 2, FontStyle::Regular), ForegroundColor, BackgroundColor, HighlightColor);

	Viewer->GetTextField()->ReadOnly = true;
	Viewer->GetTextField()->Enabled = true;
	Viewer->GetContainer()->Visible = false;

	Parent->Controls->Add(Viewer->GetContainer());
}

void SimpleTextViewer::Hide()
{
	Viewer->GetContainer()->Hide();
}

void SimpleTextViewer::Show(String^% Text)
{
	Viewer->GetTextField()->Text = Text;

	Viewer->GetContainer()->BringToFront();
	Viewer->GetContainer()->Show();
}