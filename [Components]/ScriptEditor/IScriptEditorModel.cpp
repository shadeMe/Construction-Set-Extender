#include "IScriptEditorModel.h"
#include "Preferences.h"
#include "ScriptPreprocessor.h"


namespace cse
{


namespace scriptEditor
{


namespace model
{


namespace components
{


ScriptCompilationData::ScriptCompilationData()
{
	UnpreprocessedScriptText = "";
	PreprocessedScriptText = "";
	SerializedMetadata = "";
	CanCompile = false;
	HasPreprocessorDirectives = false;
	HasWarnings = false;
	CompileResult = nullptr;
}

ScriptLineAnnotation::ScriptLineAnnotation(textEditor::ILineAnchor^ Anchor, String^ Text)
{
	Anchor_ = Anchor;
	Text_ = Text;
}

ScriptDiagnosticMessage::ScriptDiagnosticMessage(textEditor::ILineAnchor^ Anchor, String^ Message, eMessageType Type, eMessageSource Source, obScriptParsing::DiagnosticMessageCode MessageCode)
	: ScriptLineAnnotation(Anchor, Message)
{
	Type_ = Type;
	Source_ = Source;
	MessageCode_ = MessageCode;
}

void SuppressibleScriptDiagnosticMessages::InitSuppressibleMessages()
{
	using namespace obScriptParsing;

	auto CompilerWarnings = gcnew SuppressionStateTableT;
	CompilerWarnings->Add(static_cast<DiagnosticMessageCode>(DiagnosticMessageCodes::eObseWarning::UnquotedStringArgument),
						  gcnew MessageState(DiagnosticMessageCodes::GetObseWarningDescription(DiagnosticMessageCodes::eObseWarning::UnquotedStringArgument)));
	CompilerWarnings->Add(static_cast<DiagnosticMessageCode>(DiagnosticMessageCodes::eObseWarning::FunctionPointer),
						  gcnew MessageState(DiagnosticMessageCodes::GetObseWarningDescription(DiagnosticMessageCodes::eObseWarning::FunctionPointer)));
	CompilerWarnings->Add(static_cast<DiagnosticMessageCode>(DiagnosticMessageCodes::eObseWarning::DeprecatedCommand),
						  gcnew MessageState(DiagnosticMessageCodes::GetObseWarningDescription(DiagnosticMessageCodes::eObseWarning::DeprecatedCommand)));
	SuppressibleMessages->Add(gcnew FilterPairT(ScriptDiagnosticMessage::eMessageSource::Compiler, ScriptDiagnosticMessage::eMessageType::Warning), CompilerWarnings);

	auto ValidatorWarnings = gcnew SuppressionStateTableT;
	ValidatorWarnings->Add(static_cast<DiagnosticMessageCode>(DiagnosticMessageCodes::eValidatorWarning::UnusedLocalVariable),
						   gcnew MessageState(DiagnosticMessageCodes::GetValidatorWarningDescription(DiagnosticMessageCodes::eValidatorWarning::UnusedLocalVariable)));
	SuppressibleMessages->Add(gcnew FilterPairT(ScriptDiagnosticMessage::eMessageSource::Validator, ScriptDiagnosticMessage::eMessageType::Warning), ValidatorWarnings);
}

SuppressibleScriptDiagnosticMessages::SuppressibleScriptDiagnosticMessages()
{
	SuppressibleMessages = gcnew Dictionary<FilterPairT^, SuppressionStateTableT^>;

	InitSuppressibleMessages();
}

SuppressibleScriptDiagnosticMessages::MessageState::MessageState(String^ Description)
{
	MessageDescription = Description;
	Suppressed = false;
}

SuppressibleScriptDiagnosticMessages::SuppressibleMessageInfo::SuppressibleMessageInfo()
{
	Source = ScriptDiagnosticMessage::eMessageSource::All;
	Type = ScriptDiagnosticMessage::eMessageType::All;
	Code = obScriptParsing::DiagnosticMessageCodes::Invalid;
	Description = String::Empty;
}

bool SuppressibleScriptDiagnosticMessages::IsMessageSuppressed(ScriptDiagnosticMessage::eMessageSource Source, ScriptDiagnosticMessage::eMessageType Type, obScriptParsing::DiagnosticMessageCode Code)
{
	Debug::Assert(Source != ScriptDiagnosticMessage::eMessageSource::All);
	Debug::Assert(Type != ScriptDiagnosticMessage::eMessageType::All);

	if (Code == obScriptParsing::DiagnosticMessageCodes::Default)
		return false;

	SuppressionStateTableT^ SuppressedCodes;
	if (!SuppressibleMessages->TryGetValue(gcnew FilterPairT(Source, Type), SuppressedCodes))
		return false;

	MessageState^ State;
	if (!SuppressedCodes->TryGetValue(Code, State))
		return false;

	return State->Suppressed;
}

bool SuppressibleScriptDiagnosticMessages::IsMessageSuppressed(SuppressibleMessageInfo^ MessageInfo)
{
	return IsMessageSuppressed(MessageInfo->Source, MessageInfo->Type, MessageInfo->Code);
}

void SuppressibleScriptDiagnosticMessages::SuppressMessage(ScriptDiagnosticMessage::eMessageSource Source, ScriptDiagnosticMessage::eMessageType Type, obScriptParsing::DiagnosticMessageCode Code, bool Suppressed)
{
	Debug::Assert(Source != ScriptDiagnosticMessage::eMessageSource::All);
	Debug::Assert(Type != ScriptDiagnosticMessage::eMessageType::All);
	Debug::Assert(Code != obScriptParsing::DiagnosticMessageCodes::Default);

	SuppressionStateTableT^ SuppressedCodes;
	if (!SuppressibleMessages->TryGetValue(gcnew FilterPairT(Source, Type), SuppressedCodes))
		throw gcnew ArgumentException("Unknown source-type filter pair (" + Source.ToString() + ", " + Type.ToString() + ")");

	MessageState^ State;
	if (!SuppressedCodes->TryGetValue(Code, State))
		throw gcnew ArgumentException("Unknown diagnostic message code " + Code);

	State->Suppressed = Suppressed;
}

void SuppressibleScriptDiagnosticMessages::SuppressMessage(SuppressibleMessageInfo^ MessageInfo, bool Suppressed)
{
	SuppressMessage(MessageInfo->Source, MessageInfo->Type, MessageInfo->Code, Suppressed);
}

ICollection<SuppressibleScriptDiagnosticMessages::SuppressibleMessageInfo^>^ SuppressibleScriptDiagnosticMessages::GetSuppressibleMessageInfos(ScriptDiagnosticMessage::eMessageSource Source, ScriptDiagnosticMessage::eMessageType Type)
{
	auto Out = gcnew List<SuppressibleMessageInfo^>;

	for each (auto % FilterItr in SuppressibleMessages)
	{
		if (Source != ScriptDiagnosticMessage::eMessageSource::All && FilterItr.Key->Item1 != Source)
			continue;
		else if (Type != ScriptDiagnosticMessage::eMessageType::All && FilterItr.Key->Item2 != Type)
			continue;

		for each (auto % CodeItr in FilterItr.Value)
		{
			auto NewInfo = gcnew SuppressibleMessageInfo;
			NewInfo->Code = CodeItr.Key;
			NewInfo->Source = FilterItr.Key->Item1;
			NewInfo->Type = FilterItr.Key->Item2;
			NewInfo->Description = CodeItr.Value->MessageDescription;

			Out->Add(NewInfo);
		}
	}

	return Out;
}

SuppressibleScriptDiagnosticMessages^ SuppressibleScriptDiagnosticMessages::Get()
{
	return Singleton;
}

ScriptBookmark::ScriptBookmark(textEditor::ILineAnchor^ Anchor, String^ Description)
	: ScriptLineAnnotation(Anchor, Description)
{
}

ScriptFindResult::ScriptFindResult(textEditor::ILineAnchor^ Anchor, String^ Description, UInt32 Hits, String^ Query)
	: ScriptLineAnnotation(Anchor, Description), Hits_(Hits), Query_(Query)
{
}

ScriptTextMetadata::Bookmark::Bookmark(UInt32 Line, String^ Message)
{
	this->Line = Line;
	this->Text = Message;
}

ScriptTextMetadata::ScriptTextMetadata()
{
	CaretPos = -1;
	Bookmarks = gcnew List<Bookmark^>;
	HasPreprocessorDirectives = false;
}

void ScriptTextMetadataHelper::SeparateScriptTextFromMetadataBlock(String^ RawScriptText, String^% OutScriptText, String^% OutMetadata)
{
	obScriptParsing::LineTokenizer^ TextParser = gcnew obScriptParsing::LineTokenizer();
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

	obScriptParsing::LineTokenizer^ TextParser = gcnew obScriptParsing::LineTokenizer();
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
		else if (TextParser->GetTokenIndex(preprocessor::Preprocessor::kPreprocessorSigil) == 0)
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
		Block += preprocessor::Preprocessor::kPreprocessorSigil + "\n";

	if (Block != "")
	{
		Result += "\n;<" + kMetadataBlockMarker + ">\n";
		Result += Block;
		Result += ";</" + kMetadataBlockMarker + ">";
	}

	return Result;
}

IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs::AnalysisCompleteEventArgs(obScriptParsing::AnalysisData^ Result, bool BackgroundAnalysis)
{
	this->Result = Result->Clone();
	this->BackgroundAnalysis = BackgroundAnalysis;
}

INavigationHelper::NavigationChangedEventArgs::NavigationChangedEventArgs(obScriptParsing::Structurizer^ StructureData, UInt32 CurrentLine)
{
	StructureChanged = false;
	LineChanged = false;

	this->StructureData = StructureData;
	this->CurrentLine = CurrentLine;
}

System::String^ ScriptTextAutoRecoveryCache::GetCacheFilePath(String^ ScriptEditorId)
{
	if (ScriptEditorId->Length == 0)
		return String::Empty;

	auto AutoRecoveryDir = gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath());
	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> ScriptData(
		nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(
			CString(ScriptEditorId).c_str()));

	Debug::Assert(ScriptData.get() != nullptr);

	auto ParentPluginFileName = ScriptData->ParentPluginName ? gcnew String(ScriptData->ParentPluginName) : "no-plugin";
	return AutoRecoveryDir + "\\" + ParentPluginFileName + "__" + ScriptEditorId + ".obscript";
}

ScriptTextAutoRecoveryCache::ScriptTextAutoRecoveryCache(String^ ScriptEditorId)
{
	this->ScriptEditorId = ScriptEditorId;
	this->Filepath = GetCacheFilePath(ScriptEditorId);
}

void ScriptTextAutoRecoveryCache::Write(String^ Text)
{
	try
	{
		System::IO::File::WriteAllText(Filepath, Text);
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't write to auto-recovery file @ '" + Filepath + "'!\n\tException: " + E->Message, true);
	}
}

System::String^ ScriptTextAutoRecoveryCache::Read()
{
	try
	{
		return System::IO::File::ReadAllText(Filepath);
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't read from auto-recovery file @ '" + Filepath + "'!\n\tException: " + E->Message, true);
	}

	return String::Empty;
}

void ScriptTextAutoRecoveryCache::Delete(bool SendToRecycleBin)
{
	if (!Exists)
		return;

	try
	{
		if (!SendToRecycleBin)
			System::IO::File::Delete(Filepath);
		else
		{
			Microsoft::VisualBasic::FileIO::FileSystem::DeleteFile(Filepath,
																   Microsoft::VisualBasic::FileIO::UIOption::OnlyErrorDialogs,
																   Microsoft::VisualBasic::FileIO::RecycleOption::SendToRecycleBin);
		}
	}
	catch (...) {}
}

bool ScriptTextAutoRecoveryCache::Exists::get()
{
	return System::IO::File::Exists(Filepath);
}

System::DateTime ScriptTextAutoRecoveryCache::LastWriteTime::get()
{
	if (Exists)
		return System::IO::File::GetLastWriteTime(Filepath);
	else
		return DateTime::MaxValue;
}


} // namespace components


IScriptDocument::PerLineAnnotationCounts::PerLineAnnotationCounts()
{
	ErrorCount = 0;
	WarningCount = 0;
	BookmarkCount = 0;
}

IScriptDocument::StateChangeEventArgs::StateChangeEventArgs()
{
	EventType = eEventType::None;

	Dirty = false;
	BytecodeData = nullptr;
	BytecodeLength = 0;
	ScriptType = eScriptType::Object;
	EditorId = nullptr;
	FormId = 0;
	Messages = nullptr;
	Bookmarks = nullptr;
	FindResults = nullptr;
	DisplayingPreprocessorOutput = false;
	Line = Column = 0;
}


} // namespace model


} // namespace scriptEditor


} // namespace cse