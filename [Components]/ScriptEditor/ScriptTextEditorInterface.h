#pragma once
#include "IntelliSense\IntelliSenseInterface.h"

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		ref class TextEditorScriptModifiedEventArgs : public EventArgs
		{
		public:
			property bool							ModifiedStatus;

			TextEditorScriptModifiedEventArgs(bool ModifiedStatus) : EventArgs()
			{
				this->ModifiedStatus = ModifiedStatus;
			}
		};

		ref class TextEditorMouseClickEventArgs : public MouseEventArgs
		{
		public:
			property int							ScriptTextOffset;

			TextEditorMouseClickEventArgs(MouseButtons Button, int Clicks, int X, int Y, int ScriptTextOffset) : MouseEventArgs(Button, Clicks, X, Y, 0)
			{
				this->ScriptTextOffset = ScriptTextOffset;
			}
		};

		delegate void									TextEditorScriptModifiedEventHandler(Object^ Sender, TextEditorScriptModifiedEventArgs^ E);
		delegate void									TextEditorMouseClickEventHandler(Object^ Sender, TextEditorMouseClickEventArgs^ E);

		interface class IScriptTextEditor
		{
			static enum class							FindReplaceOperation
			{
				e_Find = 0,
				e_Replace,
				e_CountMatches
			};
			static enum class							FindReplaceOptions
			{
				e_InSelection		=		1 << 0,
				e_MatchWholeWord	=		1 << 1,
				e_CaseInsensitive	=		1 << 2,
				e_RegEx				=		1 << 3
			};

			// events
			event TextEditorScriptModifiedEventHandler^				ScriptModified;
			event KeyEventHandler^									KeyDown;
			event TextEditorMouseClickEventHandler^					MouseClick;

			delegate void								FindReplaceOutput(String^ Line, String^ Text);

			// methods
			void										SetFont(Font^ FontObject);
			void										SetTabCharacterSize(int PixelWidth);
			void										SetContextMenu(ContextMenuStrip^% Strip);

			void										AddControl(Control^ ControlObject);

			String^										GetText(void);
			UInt32										GetTextLength(void);
			void										SetText(String^ Text, bool PreventTextChangedEventHandling, bool ResetUndoStack);
			void										InsertText(String^ Text, int Index, bool PreventTextChangedEventHandling);			// performs bounds check

			String^										GetSelectedText(void);
			void										SetSelectedText(String^ Text, bool PreventTextChangedEventHandling);

			void										SetSelectionStart(int Index);
			void										SetSelectionLength(int Length);

			int											GetCharIndexFromPosition(Point Position);
			Point										GetPositionFromCharIndex(int Index);
			Point										GetAbsolutePositionFromCharIndex(int Index);
			int											GetLineNumberFromCharIndex(int Index);
			bool										GetCharIndexInsideCommentSegment(int Index);
			int											GetCurrentLineNumber(void);

			String^										GetTokenAtCharIndex(int Offset);
			String^										GetTokenAtCaretPos();
			void										SetTokenAtCaretPos(String^ Replacement);
			String^										GetTokenAtMouseLocation();
			array<String^>^								GetTokensAtMouseLocation();						// gets three of the closest tokens surrounding the mouse loc

			int											GetCaretPos();
			void										SetCaretPos(int Index);
			void										ScrollToCaret();

			IntPtr										GetHandle();

			void										FocusTextArea();
			void										LoadFileFromDisk(String^ Path);
			void										SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^% DefaultName);

			bool										GetModifiedStatus();
			void										SetModifiedStatus(bool Modified);

			bool										GetInitializingStatus();
			void										SetInitializingStatus(bool Initializing);

			int											GetLastKnownMouseClickOffset(void);

			int											FindReplace(FindReplaceOperation Operation, String^ Query, String^ Replacement, FindReplaceOutput^ Output, UInt32 Options);		// returns the number of matches, -1 if an error was encountered
			void										ToggleComment(int StartIndex);
			void										UpdateIntelliSenseLocalDatabase(void);

			Control^									GetContainer();
			void										ScrollToLine(String^ LineNumber);
			Point										PointToScreen(Point Location);
			void										SetEnabledState(bool State);

			void										BeginUpdate(void);
			void										EndUpdate(void);

			UInt32										GetTotalLineCount(void);
			IntelliSense::IntelliSenseInterface^		GetIntelliSenseInterface(void);

			void										IndentLines(UInt32 BeginLine, UInt32 EndLine);

			// Event handlers
			void										OnGotFocus(void);					// called when the workspace's is brought to focus
			void										OnLostFocus(void);					// the opposite of the above
			void										OnPositionSizeChange(void);			// called when the workspace's container's position or size changes

			// AvalonEdit specific
			void										HighlightScriptError(int Line);
			void										ClearScriptErrorHighlights(void);
		};
	}
}