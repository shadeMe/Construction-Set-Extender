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
							Anchor_;
			String^			Message_;
			MessageType		Type_;
			MessageSource	Source_;
		public:
			ScriptDiagnosticMessage(textEditors::IScriptTextEditor::ILineAnchor^ Anchor, String^ Message, MessageType Type, MessageSource Source)
			{
				Anchor_ = Anchor;
				Message_ = Message;
				Type_ = Type;
				Source_ = Source;
			}

			virtual property UInt32 Line
			{
				UInt32 get() { return Anchor_->Line; }
				void set(UInt32) {}
			}

			virtual property bool Valid
			{
				bool get() { return Anchor_->Valid; }
				void set(bool) {}
			}

			property String^ Message
			{
				String^ get() { return Message_; }
			}

			property MessageType Type
			{
				MessageType get() { return Type_; }
			}

			property MessageSource Source
			{
				MessageSource get() { return Source_; }
			}
		};

		ref struct ScriptBookmark : public textEditors::IScriptTextEditor::ILineAnchor
		{
		protected:
			textEditors::IScriptTextEditor::ILineAnchor^
						Anchor_;
			String^		Description_;
		public:
			ScriptBookmark(textEditors::IScriptTextEditor::ILineAnchor^ Anchor, String^ Description)
			{
				Anchor_ = Anchor;
				Description_ = Description;
			}


			virtual property UInt32 Line
			{
				UInt32 get() { return Anchor_->Line; }
				void set(UInt32) {}
			}

			virtual property bool Valid
			{
				bool get() { return Anchor_->Valid; }
				void set(bool) {}
			}

			property String^ Description
			{
				String^ get() { return Description_; }
			}
		};

		ref struct ScriptTextMetadata
		{
			ref struct Bookmark
			{
				property UInt32	 Line;
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

	}
}