#pragma once

#include "NumberedRichTextBox.h"

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		ref class ScriptOffsetViewer
		{
			OffsetRichTextBox^						Viewer;
			bool									InitializationState;
		public:
			ScriptOffsetViewer(Font^ FontData, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent);
			virtual ~ScriptOffsetViewer()
			{
				SAFEDELETE_CLR(Viewer);
			}

			virtual bool							InitializeViewer(String^ ScriptText, UInt32 Data, UInt32 Length);
			bool									Show(int CaretPosition);
			int										Hide(void);
			void									JumpToLine(String^ Line) { Viewer->JumpToLine(Line); }
			void									Reset(void);
			void									SetFont(Font^ NewFont);
			void									SetForegroundColor(Color Foreground);
			void									SetBackgroundColor(Color Background);
			UInt16									GetLastOffset();
		};

		ref class SimpleTextViewer
		{
			NumberedRichTextBox^					Viewer;
		public:
			SimpleTextViewer(Font^ FontData, Color ForegroundColor, Color BackgroundColor, Color HighlightColor, Control^% Parent);
			virtual ~SimpleTextViewer()
			{
				SAFEDELETE_CLR(Viewer);
			}

			void									Show(String^ Text, int CaretPosition);
			int										Hide(void);
			void									JumpToLine(String^ Line) { Viewer->JumpToLine(Line); }
			void									SetFont(Font^ NewFont);
			void									SetForegroundColor(Color Foreground);
			void									SetBackgroundColor(Color Background);
			UInt32									GetLineCount(void) { return Viewer->GetLineCount(); }
		};
	}
}