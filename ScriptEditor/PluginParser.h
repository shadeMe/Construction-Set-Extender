#pragma once
#include "Common\Includes.h"

// TODO: ++++++++++++++++++++++
//	


public ref class CSEPluginParserException : Exception
{
public:
	CSEPluginParserException(String^ Message) : Exception(Message) {};
};


public ref class CSERecord
{
protected:
	String^													Name;
	UInt32													FormID;
	String^													EditorID;
public:
	UInt32													GetFormID() { return FormID; }
	String^%												GetName() { return Name; }
	String^%												GetEditorID() { return EditorID; }

	UInt32													GetCommonMembers(UInt32 Size, BinaryReader^% Stream);
	UInt32													ParseSubRecord(String^% RecordName, Array^% RecordData, BinaryReader^% Stream);

	static String^											ReadRecordName(BinaryReader^% Stream);
	static String^											ByteArrayToString(Array^% Data);
	static UInt32											ByteArrayToUInt32(Array^% Data, UInt32 Offset);

	virtual	String^											Describe()	{ return nullptr; }
};

public ref class TES4Record : CSERecord
{
public:
	LinkedList<String^>^									MasterList;

	TES4Record(UInt32 Size, BinaryReader^% Stream);
	virtual String^											Describe() override;
};

public ref class QUSTRecord : CSERecord
{
public:
	UInt32													QuestScript;

	QUSTRecord(UInt32 Size, BinaryReader^% Stream);
	virtual String^											Describe() override;
};

public ref class SCPTRecord : CSERecord
{
public:
	static enum class										ScriptType
																{
																	e_Object = 0,
																	e_Quest = 1,
																	e_MagicEffect = 0x100
																};
	String^													ScriptText;
	ScriptType												Type;
	bool													UDF;

	SCPTRecord(UInt32 Size, BinaryReader^% Stream);
	virtual String^											Describe() override;
};

public ref class PluginParser
{
	void													ParseTES4Header(BinaryReader^% Stream);
	bool													IsRelevantGroup(array<Byte>^ Label);
	void													ParseRecord(String^% Name, UInt32 Size, BinaryReader^% Stream);

	bool													Valid;
public:
	TES4Record^												PluginHeader;
	List<QUSTRecord^>^										QuestRecords;
	List<SCPTRecord^>^										ScriptRecords;

	PluginParser(String^ PluginName);

	bool													IsUsable() { return Valid; }
};

#define	REC_NAME(BW)										CSERecord::ReadRecordName(BW)
#define	ARR_STR(AR)											CSERecord::ByteArrayToString(AR)
#define	ARR_UINT(AR, OFST)									CSERecord::ByteArrayToUInt32(AR, OFST)
