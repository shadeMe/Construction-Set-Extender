#include "AuxiliaryTextEditor.h"
#include "NumberedRichTextBox.h"

ScriptOffsetViewer::ScriptOffsetViewer(Font^ Font, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent)
{
	Viewer = gcnew OffsetRichTextBox(6, Font, ForegroundColor, BackgroundColor, HighlightColor);
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
		MessageBox::Show("Offset Viewer was not initialized successfully!\n\nCheck the script and recompile it before trying again.", "CSE Editor", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	Viewer->GetContainer()->BringToFront();
	Viewer->GetContainer()->Show();
	return true;
}

SimpleTextViewer::SimpleTextViewer(Font^ Font, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent)
{
	Viewer = gcnew NumberedRichTextBox(6, Font, ForegroundColor, BackgroundColor, HighlightColor);

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