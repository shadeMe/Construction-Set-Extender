#pragma once

#include "ScriptTextEditorInterface.h"
#include "SemanticAnalysis.h"

namespace cse
{
	namespace scriptEditor
	{
		ref struct ScriptDiagnosticMessage : public textEditors::IScriptTextEditor::ILineAnchor
		{
			static enum class MessageType
			{
				Info,
				Warning,
				Error,
			};

			static enum class MessageSource
			{
				Compiler,
				Validator,
				Preprocessor,
			};
		protected:
			textEditors::IScriptTextEditor::ILineAnchor^
							_Anchor;
			String^			_Message;
			MessageType		_Type;
			MessageSource	_Source;
		public:
			ScriptDiagnosticMessage(textEditors::IScriptTextEditor::ILineAnchor^ Anchor, String^ Message, MessageType Type, MessageSource Source)
			{
				_Anchor = Anchor;
				_Message = Message;
				_Type = Type;
				_Source = Source;
			}


			virtual property UInt32 Line
			{
				UInt32 get() { return _Anchor->Line; }
				void set(UInt32) {}
			}

			virtual property bool Valid
			{
				bool get() { return _Anchor->Valid; }
				void set(bool) {}
			}

			property String^ Message
			{
				String^ get() { return _Message; }
			}

			property MessageType Type
			{
				MessageType get() { return _Type; }
			}

			property MessageSource Source
			{
				MessageSource get() { return _Source; }
			}
		};

		ref struct ScriptBookmark : public textEditors::IScriptTextEditor::ILineAnchor
		{
		protected:
			textEditors::IScriptTextEditor::ILineAnchor^
						_Anchor;
			String^		_Description;
		public:
			ScriptBookmark(textEditors::IScriptTextEditor::ILineAnchor^ Anchor, String^ Description)
			{
				_Anchor = Anchor;
				_Description = Description;
			}


			virtual property UInt32 Line
			{
				UInt32 get() { return _Anchor->Line; }
				void set(UInt32) {}
			}

			virtual property bool Valid
			{
				bool get() { return _Anchor->Valid; }
				void set(bool) {}
			}

			property String^ Description
			{
				String^ get() { return _Description; }
			}
		};

		ref struct ScriptTextMetadata
		{
			ref struct Bookmark
			{
				property UInt32		Line;
				property String^ Message;

				Bookmark(UInt32 Line, String^ Message)
				{
					this->Line = Line;
					this->Message = Message;
				}
			};

			using CaretPosition = int;


			property CaretPosition		CaretPos;
			property List<Bookmark^>^	Bookmarks;
			property bool				HasPreprocessorDirectives;

			ScriptTextMetadata()
			{
				this->CaretPos = -1;
				this->Bookmarks = gcnew List<Bookmark^>();
				this->HasPreprocessorDirectives = false;
			}
		};

		ref class ScriptTextMetadataHelper
		{
			static String^	kMetadataBlockMarker = "CSEBlock";
			static String^	kMetadataSigilCaret = "CSECaretPos";
			static String^	kMetadataSigilBookmark = "CSEBookmark";

			static void		SeparateScriptTextFromMetadataBlock(String^ RawScriptText, String^% OutScriptText, String^% OutMetadata);
		public:
			static void		DeserializeRawScriptText(String^ RawScriptText, String^% OutScriptText, ScriptTextMetadata^% OutMetadata);
			static String^	SerializeMetadata(ScriptTextMetadata^ Metadata);
		};


		// defer if there are no changes since last analysis
	}
}