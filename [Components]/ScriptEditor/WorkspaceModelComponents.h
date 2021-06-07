#pragma once

#include "ScriptTextEditorInterface.h"
#include "SemanticAnalysis.h"
#include "Globals.h"

namespace cse
{
	namespace scriptEditor
	{
		ref struct CompilationData
		{
			String^	UnpreprocessedScriptText;
			String^	PreprocessedScriptText;
			String^	SerializedMetadata;

			bool CanCompile;
			bool HasPreprocessorDirectives;
			bool HasWarnings;

			componentDLLInterface::ScriptCompileData* CompileResult;

			CompilationData()
			{
				UnpreprocessedScriptText = "";
				PreprocessedScriptText = "";
				SerializedMetadata = "";
				CanCompile = false;
				HasPreprocessorDirectives = false;
				HasWarnings = false;
				CompileResult = nullptr;
			}
		};

		ref struct ScriptLineAnnotation : public textEditors::ILineAnchor
		{
		protected:
			textEditors::ILineAnchor^ Anchor_;
			String^ Text_;
		public:
			ScriptLineAnnotation(textEditors::ILineAnchor^ Anchor, String^ Text)
			{
				Anchor_ = Anchor;
				Text_ = Text;
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

			property String^ Text
			{
				String^ get() { return Text_; }
			}
		};

		ref struct ScriptDiagnosticMessage : public ScriptLineAnnotation
		{
			static enum class MessageType
			{
				All,
				Info,
				Warning,
				Error,
			};

			static enum class MessageSource
			{
				All,
				Compiler,
				Validator,
				Preprocessor,
			};

			static void		PopulateImageListWithMessageTypeImages(ImageList^ Destination);
		protected:
			static array<Image^>^ MessageTypeImages =
			{
				Globals::ImageResources()->CreateImage("IntelliSenseItemEmpty"),
				Globals::ImageResources()->CreateImage("MessageListInfo"),
				Globals::ImageResources()->CreateImage("MessageListWarning"),
				Globals::ImageResources()->CreateImage("MessageListError"),
			};

			MessageType		Type_;
			MessageSource	Source_;
		public:
			ScriptDiagnosticMessage(textEditors::ILineAnchor^ Anchor,
									String^ Message,
									MessageType Type,
									MessageSource Source)
				: ScriptLineAnnotation(Anchor, Message)
			{
				Type_ = Type;
				Source_ = Source;
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

		ref struct ScriptBookmark : public ScriptLineAnnotation
		{
		public:
			ScriptBookmark(textEditors::ILineAnchor^ Anchor, String^ Description)
				: ScriptLineAnnotation(Anchor, Description) {}
		};

		ref struct ScriptFindResult : public ScriptLineAnnotation
		{
			UInt32 Hits_;
		public:
			ScriptFindResult(textEditors::ILineAnchor^ Anchor, String^ Description, UInt32 Hits)
				: ScriptLineAnnotation(Anchor, Description), Hits_(Hits) {}

			property UInt32 Hits
			{
				UInt32 get() { return Hits_; }
			}
		};

		ref struct ScriptTextMetadata
		{
			using CaretPosition = int;

			ref struct Bookmark
			{
				property UInt32	 Line;
				property String^ Text;

				Bookmark(UInt32 Line, String^ Message)
				{
					this->Line = Line;
					this->Text = Message;
				}
			};

			property CaretPosition CaretPos;
			property List<Bookmark^>^ Bookmarks;
			property bool HasPreprocessorDirectives;

			ScriptTextMetadata()
			{
				CaretPos = -1;
				Bookmarks = gcnew List<Bookmark^>;
				HasPreprocessorDirectives = false;
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