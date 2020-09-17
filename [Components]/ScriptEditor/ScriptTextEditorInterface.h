#pragma once
#include "SemanticAnalysis.h"
#include "IIntelliSenseInterface.h"
#include "[Common]\NativeWrapper.h"

namespace cse
{
	namespace textEditors
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

		delegate void TextEditorScriptModifiedEventHandler(Object^ Sender, TextEditorScriptModifiedEventArgs^ E);
		delegate void TextEditorMouseClickEventHandler(Object^ Sender, TextEditorMouseClickEventArgs^ E);

		ref struct CompilationData
		{
			String^			UnpreprocessedScriptText;
			String^			PreprocessedScriptText;
			String^			SerializedMetadata;

			bool			CanCompile;
			bool			HasDirectives;
			bool			HasWarnings;

			componentDLLInterface::ScriptCompileData* CompileResult;

			CompilationData() : UnpreprocessedScriptText(""), PreprocessedScriptText(""), SerializedMetadata(""),
								CanCompile(false), HasDirectives(false), HasWarnings(false), CompileResult(nullptr) {}
		};

		interface class IScriptTextEditor : public intellisense::IIntelliSenseInterfaceConsumer
		{
			interface class ILineAnchor
			{
				property UInt32	Line;
				property bool	Valid;
			};

			static enum class ScriptMessageType
			{
				None = -1,
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
				RegEx			=		1 << 3,
				IgnoreComments	=		1 << 4
			};

			ref struct FindReplaceResult
			{
				ref struct HitData
				{
					UInt32				Line;
					String^				Text;		// the line's text after replacement
					UInt32				Hits;		// no of hits in the line

					HitData(UInt32 Line, String^ Text, UInt32 Hits) : Line(Line), Text(Text), Hits(Hits) {}
				};

				List<HitData^>^		Hits;
				bool				HasError;

				FindReplaceResult() : Hits(gcnew List<HitData^>), HasError(false) {}

				void Add(UInt32 Line, String^ Text, UInt32 HitsInLine)
				{
					Text->Replace("\t", "")->Replace("\r\n", "")->Replace("\n", "");
					Hits->Add(gcnew HitData(Line, Text, HitsInLine));
				}

				property int TotalHitCount
				{
					int get()
					{
						int Count = 0;

						for each (auto Itr in Hits)
							Count += Itr->Hits;

						return Count;
					}
				}
			};

			event intellisense::IntelliSenseInputEventHandler^			IntelliSenseInput;
			event intellisense::IntelliSenseInsightHoverEventHandler^	IntelliSenseInsightHover;
			event intellisense::IntelliSenseContextChangeEventHandler^	IntelliSenseContextChange;

			event TextEditorScriptModifiedEventHandler^
														ScriptModified;
			event KeyEventHandler^						KeyDown;
			event TextEditorMouseClickEventHandler^		MouseClick;
			event EventHandler^							LineChanged;					// raised when the current line changes
			event EventHandler^							TextUpdated;					// raised after the editor's entire text has been updated

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
															 ListView^ FindResultList);		// called when the parent model is bound to a view, i.e., when the text editor is activated
			void										Unbind();	// opposite of the above

			String^										GetText();
			String^										GetText(UInt32 LineNumber);
			String^										GetPreprocessedText(bool% OutPreprocessResult, bool SuppressErrors);
			void										SetText(String^ Text, bool ResetUndoStack);
			String^										GetSelectedText(void);
			void										SetSelectedText(String^ Text);
			int											GetCharIndexFromPosition(Point Position);
			Point										GetPositionFromCharIndex(int Index, bool Absolute);
			String^										GetTokenAtCharIndex(int Offset);
			String^										GetTokenAtCaretPos();
			void										SetTokenAtCaretPos(String^ Replacement);
			void										ScrollToCaret();
			void										ScrollToLine(UInt32 LineNumber);
			void										FocusTextArea();

			void										LoadFileFromDisk(String^ Path);
			void										SaveScriptToDisk(String^ Path, bool PathIncludesFileName, String^ DefaultName, String^ DefaultExtension);

			FindReplaceResult^							FindReplace(FindReplaceOperation Operation,
																	String^ Query,
																	String^ Replacement,
																	FindReplaceOptions Options);


			void										BeginUpdate(void);
			void										EndUpdate(bool FlagModification);

			UInt32										GetIndentLevel(UInt32 LineNumber);
			void										InsertVariable(String^ VariableName, obScriptParsing::Variable::DataType VariableType);

			void										InitializeState(String^ RawScriptText);			// clears tracked data and deserializes any metadata found in the script text
			CompilationData^							BeginScriptCompilation();
			void										EndScriptCompilation(CompilationData^ Data);
			ILineAnchor^								CreateAnchor(UInt32 Line);
		};
	}
}