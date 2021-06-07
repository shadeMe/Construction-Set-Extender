#include "WorkspaceModelComponents.h"
#include "Preferences.h"

namespace cse
{
	namespace scriptEditor
	{
		void ScriptDiagnosticMessage::PopulateImageListWithMessageTypeImages(ImageList^ Destination)
		{
			Destination->Images->Clear();
			Destination->Images->AddRange(MessageTypeImages);
		}

		void ScriptTextMetadataHelper::SeparateScriptTextFromMetadataBlock(String^ RawScriptText, String^% OutScriptText, String^% OutMetadata)
		{
			ScriptParser^ TextParser = gcnew ScriptParser();
			StringReader^ StringParser = gcnew StringReader(RawScriptText);
			String^ ReadLine = StringParser->ReadLine();
			String^ CSEBlock = "";
			String^ Result = "";
			bool ExtractingBlock = false;

			while (ReadLine != nullptr)
			{
				TextParser->Tokenize(ReadLine, false);

				if (ExtractingBlock)
				{
					if (!TextParser->GetTokenIndex(";</" + kMetadataBlockMarker + ">"))
						ExtractingBlock = false;
					else
						CSEBlock += ReadLine + "\n";

					ReadLine = StringParser->ReadLine();
					continue;
				}

				if (!TextParser->Valid)
				{
					Result += "\n" + ReadLine;
					ReadLine = StringParser->ReadLine();
					continue;
				}
				else if (!TextParser->GetTokenIndex(";<" + kMetadataBlockMarker + ">"))
				{
					ExtractingBlock = true;
					ReadLine = StringParser->ReadLine();
					continue;
				}

				Result += "\n" + ReadLine;
				ReadLine = StringParser->ReadLine();
			}

			if (Result != "")
				Result = Result->Substring(1);

			OutScriptText = Result;
			OutMetadata = CSEBlock;
		}

		void ScriptTextMetadataHelper::DeserializeRawScriptText(String^ RawScriptText, String^% OutScriptText, ScriptTextMetadata^% OutMetadata)
		{
			String^ MetadataBlock = "";
			SeparateScriptTextFromMetadataBlock(RawScriptText, OutScriptText, MetadataBlock);

			ScriptParser^ TextParser = gcnew ScriptParser();
			StringReader^ StringParser = gcnew StringReader(MetadataBlock);
			String^ ReadLine = StringParser->ReadLine();

			while (ReadLine != nullptr)
			{
				TextParser->Tokenize(ReadLine, false);
				if (!TextParser->Valid)
				{
					ReadLine = StringParser->ReadLine();
					continue;
				}

				if (TextParser->GetTokenIndex(";<" + kMetadataSigilCaret + ">") == 0)
				{
					try { OutMetadata->CaretPos = int::Parse(TextParser->Tokens[1]); }
					catch (...) { OutMetadata->CaretPos = -1; }
				}
				else if (TextParser->GetTokenIndex(";<" + kMetadataSigilBookmark + ">") == 0)
				{
					array<String^>^ Splits = ReadLine->Substring(TextParser->Indices[0])->Split((String("\t")).ToCharArray());
					int LineNo = 0;
					try { LineNo = int::Parse(Splits[1]); }
					catch (...) { LineNo = 1; }

					OutMetadata->Bookmarks->Add(gcnew ScriptTextMetadata::Bookmark(LineNo, Splits[2]));
				}
				else if (TextParser->GetTokenIndex(Preprocessor::kPreprocessorSigil) == 0)
					OutMetadata->HasPreprocessorDirectives = true;

				ReadLine = StringParser->ReadLine();
			}


		}

		System::String^ ScriptTextMetadataHelper::SerializeMetadata(ScriptTextMetadata^ Metadata)
		{
			String^ Block = "";
			String^ Result = "";

			if (Metadata->CaretPos != -1 && preferences::SettingsHolder::Get()->General->SaveRestoreCaret)
				Block += String::Format(";<" + kMetadataSigilCaret + "> {0} </" + kMetadataSigilCaret + ">\n", Metadata->CaretPos);


			for each (ScriptTextMetadata::Bookmark ^ Itr in Metadata->Bookmarks)
				Block += ";<" + kMetadataSigilBookmark + ">\t" + Itr->Line + "\t" + Itr->Text + "\t</" + kMetadataSigilBookmark + ">\n";


			if (Metadata->HasPreprocessorDirectives)
				Block += Preprocessor::kPreprocessorSigil + "\n";

			if (Block != "")
			{
				Result += "\n;<" + kMetadataBlockMarker + ">\n";
				Result += Block;
				Result += ";</" + kMetadataBlockMarker + ">";
			}

			return Result;
		}



	}
}

