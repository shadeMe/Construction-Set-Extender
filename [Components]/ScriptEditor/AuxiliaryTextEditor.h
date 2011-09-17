#pragma once

#include "NumberedRichTextBox.h"

public ref class ScriptOffsetViewer
{
	OffsetRichTextBox^						Viewer;
	bool									InitializationState;
public:
	ScriptOffsetViewer(Font^ FontData, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent);

	bool									InitializeViewer(String^% ScriptText, UInt32 Data, UInt32 Length);
	bool									Show(void);
	void									Hide(void);
	void									JumpToLine(String^ Line) { Viewer->JumpToLine(Line); }
	void									Reset(void);
};

public ref class SimpleTextViewer
{
	NumberedRichTextBox^					Viewer;
public:
	SimpleTextViewer(Font^ FontData, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent);

	void									Show(String^% Text);
	void									Hide(void);
	void									JumpToLine(String^ Line) { Viewer->JumpToLine(Line); }
};