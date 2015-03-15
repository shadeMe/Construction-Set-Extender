#pragma once
#include "SemanticAnalysis.h"
#include "IIntelliSenseInterface.h"

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		ref class TextEditorScriptModifiedEventArgs : public EventArgs
		{
		public:
			property bool ModifiedStatus;

			TextEditorScriptModifiedEventArgs(bool ModifiedStatus) : EventArgs()
			{
				this->ModifiedStatus = ModifiedStatus;
			}
		};

		ref class TextEditorMouseClickEventArgs : public MouseEventArgs
		{
		public:
			property int ScriptTextOffset;

			TextEditorMouseClickEventArgs(MouseButtons Button, int Clicks, int X, int Y, int ScriptTextOffset) : MouseEventArgs(Button, Clicks, X, Y, 0)
			{
				this->ScriptTextOffset = ScriptTextOffset;
			}
		};

		ref class IntelliSenseKeyEventArgs : public KeyEventArgs
		{
		public:
			property bool AllowForDisplay;

			// results
			property bool PreventNextTextChangeEvent;
			property bool Display;
			property IntelliSense::IIntelliSenseInterfaceModel::Operation DisplayOperation;

			IntelliSenseKeyEventArgs(Keys Data) : KeyEventArgs(Data)
			{
				Handled = false;
				AllowForDisplay = true;
				PreventNextTextChangeEvent = false;
				Display = false;
				DisplayOperation = IntelliSense::IIntelliSenseInterfaceModel::Operation::Default;
			}
		};

		ref class IntelliSensePositionEventArgs : public EventArgs
		{
		public:
			property Point	Location;			// screen coords
			property IntPtr	WindowHandle;		// parent

			IntelliSensePositionEventArgs() : EventArgs()
			{
				Location = Point(0, 0);
				WindowHandle = IntPtr::Zero;
			}
		};

		ref class IntelliSenseShowEventArgs : public IntelliSensePositionEventArgs
		{
		public:
			property bool UseActive;			// continue with the previous/active operation
			property IntelliSense::IIntelliSenseInterfaceModel::Operation NewOperation;

			IntelliSenseShowEventArgs() : IntelliSensePositionEventArgs()
			{
				UseActive = true;
				NewOperation = IntelliSense::IIntelliSenseInterfaceModel::Operation::Default;
			}
		};

		ref class IntelliSenseHideEventArgs : public EventArgs
		{
		public:
			property bool Reset;

			IntelliSenseHideEventArgs() : EventArgs()
			{
				Reset = false;
			}
		};

		delegate void TextEditorScriptModifiedEventHandler(Object^ Sender, TextEditorScriptModifiedEventArgs^ E);
		delegate void TextEditorMouseClickEventHandler(Object^ Sender, TextEditorMouseClickEventArgs^ E);
		delegate void IntelliSenseKeyEventHandler(Object^ Sender, IntelliSenseKeyEventArgs^ E);
		delegate void IntelliSensePositionEventHandler(Object^ Sender, IntelliSensePositionEventArgs^ E);
		delegate void IntelliSenseShowEventHandler(Object^ Sender, IntelliSenseShowEventArgs^ E);
		delegate void IntelliSenseHideEventHandler(Object^ Sender, IntelliSenseHideEventArgs^ E);

		interface class IScriptTextEditor
		{
			static enum class ScriptMessageType
			{
				Warning = 0,
				Error = 1,
			};

			static enum class ScriptMessageSource
			{
				None = 0,
				Compiler,
				Validator,
				Preprocessor,
			};

			static enum class FindReplaceOperation
			{
				Find,
				Replace,
				CountMatches
			};

			[Flags]
			static enum class FindReplaceOptions
			{
				InSelection		=		1 << 0,
				MatchWholeWord	=		1 << 1,
				CaseInsensitive	=		1 << 2,
				RegEx			=		1 << 3
			};

			event IntelliSenseKeyEventHandler^						IntelliSenseKeyDown;		// this has to be a separate event as we need to reliably return values from the handler

			// these events are raised as requests
			event IntelliSenseShowEventHandler^						IntelliSenseShow;
			event IntelliSenseHideEventHandler^						IntelliSenseHide;
			event IntelliSensePositionEventHandler^					IntelliSenseRelocate;

			event TextEditorScriptModifiedEventHandler^				ScriptModified;
			event KeyEventHandler^									KeyDown;
			event TextEditorMouseClickEventHandler^					MouseClick;

			property Control^							Container;
			property IntPtr								WindowHandle;
			property bool								Enabled;

			property int								CurrentLine;
			property int								LineCount;
			property int								Caret;

			property bool								Modified;

			// methods
			void										Bind(ListView^ MessageList,
															 ListView^ BookmarkList,
															 ListView^ FindResultList,
															 IntelliSense::IIntelliSenseInterfaceView^ IntelliSenseView);		// called when the parent model is bound to a view, i.e., when the text editor is activated
			void										Unbind();	// opposite of the above

			String^										GetText();
			String^										GetPreprocessedText(bool% OutPreprocessResult, bool SuppressErrors);
			void										SetText(String^ Text, bool PreventTextChangedEventHandling, bool ResetUndoStack);

			String^										GetSelectedText(void);
			void										SetSelectedText(String^ Text, bool PreventTextChangedEventHandling);

			int											GetCharIndexFromPosition(Point Position);
			Point										GetPositionFromCharIndex(int Index, bool Absolute);

			String^										GetTokenAtCharIndex(int Offset);
			String^										GetTokenAtCaretPos();
			void										SetTokenAtCaretPos(String^ Replacement);

			void										ScrollToCaret();

			void										FocusTextArea();
			void										LoadFileFromDisk(String^ Path);
			void										SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^ DefaultName, String^ DefaultExtension);

			int											FindReplace(FindReplaceOperation Operation,
																	String^ Query,
																	String^ Replacement,
																	UInt32 Options);		// returns the number of matches, -1 if an error was encountered

			void										ScrollToLine(UInt32 LineNumber);
			Point										PointToScreen(Point Location);

			void										BeginUpdate(void);
			void										EndUpdate(bool FlagModification);

			UInt32										GetIndentLevel(UInt32 LineNumber);
			void										InsertVariable(String^ VariableName, ObScriptSemanticAnalysis::Variable::DataType VariableType);

			String^										SerializeMetadata(bool AddPreprocessorSigil);			// returns the metadata block for the current script, if any. includes caret pos, bookmarks, etc
			String^										DeserializeMetadata(String^ Input, bool SetText);		// extracts and processes the metadata block from the script text, after clearing the current state
																												// returns the rest of the text

			bool										CanCompile(bool% OutContainsPreprocessorDirectives);	// preprocesses and validates the script text, returns true if successful

			void										ClearTrackedData(bool CompilerMessages,
																		 bool PreprocessorMessages,
																		 bool ValidatorMessages,
																		 bool Bookmarks,
																		 bool FindResults);
			void										TrackCompilerError(int Line, String^ Message);
			ObScriptSemanticAnalysis::AnalysisData^		GetSemanticAnalysisCache(bool UpdateVars, bool UpdateControlBlocks);
		};
}
}