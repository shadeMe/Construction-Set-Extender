#include "PluginParser.h"
#include "Common\NativeWrapper.h"
#include "Globals.h"

using namespace System::Text;

String^ CSERecord::ByteArrayToString(Array^% Data)							// 1251 - codepage int for English
{
	if (static_cast< array<Byte>^ >(Data)[Data->Length - 1] == '\0')
		return Encoding::GetEncoding(1251)->GetString(static_cast< array<Byte>^ >(Data), 0, Data->Length - 1);		
	else
		return Encoding::GetEncoding(1251)->GetString(static_cast< array<Byte>^ >(Data));		
}

UInt32 CSERecord::ByteArrayToUInt32(Array^% Data, UInt32 Offset)
{
	return BitConverter::ToUInt32(static_cast< array<Byte>^ >(Data), Offset);
}

String^ CSERecord::ReadRecordName(BinaryReader^% Stream)
{
	array<Byte>^ RecordName = {'z','z','z','z'};
	Stream->Read(RecordName, 0, 4);
	return ARR_STR(static_cast<Array^>(RecordName));
}

UInt32 CSERecord::ParseSubRecord(String^% RecordName, Array^% RecordData, BinaryReader^% Stream)
{
	UInt32 AmountRead = 0, RecordSize = 0;
	RecordName = REC_NAME(Stream);
	RecordSize = Stream->ReadUInt16();

	if (RecordName == "XXXX") {
		DebugPrint("\tEncountered subrecord XXXX", false, false);
		RecordSize = Stream->ReadUInt32();
		RecordName = REC_NAME(Stream);
		Stream->BaseStream->Position += 2;
	}

	RecordData = Array::CreateInstance(Byte::typeid, RecordSize);
	Stream->Read(static_cast< array<Byte>^ >(RecordData), 0, RecordData->Length);
	AmountRead += (UInt32)(6 + RecordData->Length + (RecordData->Length > UInt16::MaxValue ? 10:0));	
	return AmountRead;
}

UInt32 CSERecord::GetCommonMembers(UInt32 Size, BinaryReader^% Stream)
{
	UInt32 Flags1 = Stream->ReadUInt32(), AmountRead = 0;
	FormID = Stream->ReadUInt32();
	Stream->ReadUInt32();															// skip Flags2
	
	if ((Flags1 & 0x00040000) > 0)				throw gcnew CSEPluginParserException("Compressed record encountered");	

	String^ RecordName = nullptr;
	Array^ Data = nullptr;
	AmountRead += ParseSubRecord(RecordName, Data, Stream);

	if (RecordName != "EDID")					throw gcnew CSEPluginParserException("Unexpected first subrecord " + RecordName + "! Expected EDID");	

	EditorID = ARR_STR(Data);	
	return AmountRead;	
}

TES4Record::TES4Record(UInt32 Size, BinaryReader^% Stream)
{
	Name = "TES4";
	EditorID = "NULL";
	MasterList = gcnew LinkedList<String^>();

	Stream->ReadUInt32();															// skip Flags1&2 and formID
	Stream->ReadUInt32();
	Stream->ReadUInt32();

	UInt32 AmountRead = 0;
	while (AmountRead < Size) {
		String^ RecordName = nullptr;
		Array^ Data = nullptr;
		AmountRead += ParseSubRecord(RecordName, Data, Stream);

		if (RecordName == "MAST") {
			MasterList->AddLast(ARR_STR(Data));
		}
	}

	if (AmountRead > Size)		throw gcnew CSEPluginParserException("Subrecord block did not match the size specified in the record header");
}

String^	TES4Record::Describe()
{
	String^ Description = "\n[" + GetName() + "]\tFormID: " + GetFormID().ToString("x8") + "\tEditorID: " + GetEditorID() + "\n\nMaster List:\n";
	for each (String^% Itr in MasterList) {
		Description += "\t" + Itr + "\n";
	}
	return Description;
}

QUSTRecord::QUSTRecord(UInt32 Size, BinaryReader^% Stream)
{
	Name = "QUST";
	UInt32 AmountRead = GetCommonMembers(Size, Stream);

	while (AmountRead < Size) {
		String^ RecordName = nullptr;
		Array^ Data = nullptr;
		AmountRead += ParseSubRecord(RecordName, Data, Stream);

		if (RecordName == "SCRI") {
			QuestScript = ARR_UINT(Data, 0);
		}	
	}

	if (AmountRead > Size)		throw gcnew CSEPluginParserException("Subrecord block did not match the size specified in the record header");
}

String^	QUSTRecord::Describe()
{
	String^ Description = "[" + GetName() + "]\tFormID: " + GetFormID().ToString("x8") + "\tEditorID: " + GetEditorID() + "\tQuest Script: " + QuestScript.ToString("x8");
	return Description;
}

SCPTRecord::SCPTRecord(UInt32 Size, BinaryReader^% Stream)
{
	Name = "SCPT";
	UDF = false;
	UInt32 AmountRead = GetCommonMembers(Size, Stream);

	while (AmountRead < Size) {
		String^ RecordName = nullptr;
		Array^ Data = nullptr;
		AmountRead += ParseSubRecord(RecordName, Data, Stream);

		if (RecordName == "SCHR") {
			UInt32 Type = ARR_UINT(Data, 16);
			if (Type == 0 || Type == 1 || Type == 0x100)		this->Type = (ScriptType)Type;
			else												throw gcnew CSEPluginParserException("Unknown script type " + Type.ToString("x8"));
		} else if (RecordName == "SCDA") {
			if (Data->Length >= 15) {
				BinaryReader^ DataReader = gcnew BinaryReader(gcnew MemoryStream(static_cast<array<Byte>^>(Data)));
				DataReader->ReadUInt64();						// skip to the opcode of the first begin block
				if (DataReader->ReadUInt16() == 7)				UDF = true;
				DataReader->Close();
			}
		} else if (RecordName == "SCTX") {
			ScriptText = ARR_STR(static_cast<Array^>(Data));
		}
	}

	if (AmountRead > Size)		throw gcnew CSEPluginParserException("Subrecord block did not match the size specified in the record header");
}

String^	SCPTRecord::Describe()
{
	String^ Description = "[" + GetName() + "] FormID: " + GetFormID().ToString("x8") + "\tEditorID: " + GetEditorID() + "\tScript Type: " + (int)this->Type + "\tUDF: " + UDF;
	return Description;
}


PluginParser::PluginParser(String^ PluginName)
{
	QuestRecords = gcnew List<QUSTRecord^>();
	ScriptRecords = gcnew List<SCPTRecord^>();
	Valid = false;

	DebugPrint("Parsing plugin " + PluginName, false, false);
	FileInfo^ FI = gcnew FileInfo(GLOB->AppPath + "Data\\" + PluginName);
	BinaryReader^ Stream = gcnew BinaryReader(FI->OpenRead());

	try {
		String^ Name;
		UInt32 Size, GroupType;
		array<Byte>^ Label = {'z','z','z','z'};
	    
		ParseTES4Header(Stream);

		while(Stream->PeekChar()!= -1) {
			Name = REC_NAME(Stream);
			Size = Stream->ReadUInt32();

			if (Name == "GRUP") {
				Label = Stream->ReadBytes(4);
				GroupType = Stream->ReadUInt32();
				Stream->ReadUInt32();												// skip dateStamp
				if (GroupType || !IsRelevantGroup(Label)) {							// not a top level type
					Stream->BaseStream->Seek(Size - 20, SeekOrigin::Current);
				}
				else {
					DebugPrint("Reading GRUP " + ARR_STR(static_cast<Array^>(Label)), false, false);
					UInt32 AmountRead = 0, RecSize = 0;
					while (AmountRead < Size - 20) {		
						Name = REC_NAME(Stream);
						RecSize = Stream->ReadUInt32();
						ParseRecord(Name, RecSize, Stream);
						AmountRead += (UInt32)(RecSize + 20);
					}

					if (AmountRead > Size)			throw gcnew CSEPluginParserException("Record block did not match the size specified in the group header");
				}
			}
			else {
				DebugPrint("Unexpected record " + Name, false, false);
				Stream->BaseStream->Seek(Size, SeekOrigin::Current);				// skip record
			}
		}
		Valid = true;
	} catch (CSEPluginParserException^ E) {
		String^ Message = "Parser Exception raised while parsing plugin " + PluginName + ":\n\t " + E->Message;
		DebugPrint(Message, false, true);
	} catch (Exception^ E) {
		String^ Message = "Unknown Exception raised while parsing plugin " + PluginName + ":\n\t " + E->Message;
		DebugPrint(Message, false, true);
	}
	finally {
		Stream->Close();
		DebugPrint("Finished parsing " + PluginName, false, false);	

#ifdef _DEBUG
		DebugPrint("Dumping records ...", false, false);	
		DebugPrint(PluginHeader->Describe(), false, false);
		for each (QUSTRecord^% Itr in QuestRecords) {
			DebugPrint(Itr->Describe(), false, false);
		}
		for each (SCPTRecord^% Itr in ScriptRecords) {
			DebugPrint(Itr->Describe(), false, false);
		}
		DebugPrint("Dumped all saved records!", false, false);
#endif
	}
}

void PluginParser::ParseTES4Header(System::IO::BinaryReader ^%Stream)
{
	String^ Name = REC_NAME(Stream);
	if (Name != "TES4")					throw gcnew CSEPluginParserException("File is not a valid TES4 plugin");
	PluginHeader = gcnew TES4Record(Stream->ReadUInt32(), Stream);
}

bool PluginParser::IsRelevantGroup(array<Byte>^ Label)
{
	String^ GroupName(ARR_STR(static_cast<Array^>(Label)));

	if (!String::Compare(GroupName, "QUST"))			return true;
	else if (!String::Compare(GroupName, "SCPT"))		return true;
	else 												return false;
}

void PluginParser::ParseRecord(String^% Name, UInt32 Size, BinaryReader^% Stream)
{

	if (!String::Compare(Name, "QUST"))					QuestRecords->Add(gcnew QUSTRecord(Size, Stream));
	else if (!String::Compare(Name, "SCPT"))			ScriptRecords->Add(gcnew SCPTRecord(Size, Stream));
	else												throw gcnew CSEPluginParserException("Unexpected record " + Name);
}

