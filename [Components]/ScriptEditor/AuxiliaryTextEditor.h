#pragma once

#include "NumberedRichTextBox.h"

public ref class ScriptOffsetViewer
{
	OffsetRichTextBox^						Viewer;
	bool									InitializationState;
public:
	ScriptOffsetViewer(Font^ Font, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent);

	bool									InitializeViewer(String^% ScriptText, UInt32 Data, UInt32 Length);
	bool									Show(void);
	void									Hide(void);
	void									JumpToLine(String^ Line) { Viewer->JumpToLine(Line); }
};

public ref class SimpleTextViewer
{
	NumberedRichTextBox^					Viewer;
public:
	SimpleTextViewer(Font^ Font, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent);

	void									Show(String^% Text);
	void									Hide(void);
	void									JumpToLine(String^ Line) { Viewer->JumpToLine(Line); }
};