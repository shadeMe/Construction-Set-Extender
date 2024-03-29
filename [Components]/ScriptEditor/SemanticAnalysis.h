#pragma once
#include "[Common]\IncludesCLR.h"


namespace cse
{


namespace scriptEditor
{


namespace obScriptParsing
{


class ByteCodeParser
{

	static enum class eBuiltInOpcodes : UInt16
	{
		End = 0x11,
		Else = 0x17,
		EndIf = 0x19,
		DotOperator = 0x1C,
		ScriptName = 0x1D,
		Return = 0x1E,
	};

	static UInt16 Read16(UInt8* Data, UInt16% CurrentOffset);
	static bool LineHasData(String^ Line);
public:
	static UInt16 GetOffsetForLine(String^ Line, UInt8* Data, UInt16% CurrentOffset);
};


using DiagnosticMessageCode = UInt32;


ref struct DiagnosticMessage
{
	static enum class eType
	{
		Error, Warning, Info
	};

	property UInt32 Line;
	property String^ Message;
	property eType Type;
	property DiagnosticMessageCode MessageCode;

	DiagnosticMessage();

	property bool IsError
	{
		bool get() { return Type == eType::Error; }
	}
	property bool IsWarning
	{
		bool get() { return Type == eType::Warning; }
	}
	property bool IsInfo
	{
		bool get() { return Type == eType::Info; }
	}
};


ref struct DiagnosticMessageCodes
{
	static property DiagnosticMessageCode Invalid
	{
		DiagnosticMessageCode get() { return 0; }
	}

	// used as a placeholder for messages that do not require a unique message code
	// for the purposes of identification (for filtering, etc)
	static property DiagnosticMessageCode Default
	{
		DiagnosticMessageCode get() { return DiagnosticMessageCode::MaxValue; }
	}

	// same as the ones found in the xOBSE source
	static enum class eObseWarning : DiagnosticMessageCode
	{
		UnquotedStringArgument = 19,
		FunctionPointer = 20,
		DeprecatedCommand = 21,
	};

	static enum class eValidatorError : DiagnosticMessageCode
	{
		None,

		ScriptNameRedeclaration,
		InvalidScriptName,
		VariableDeclarationInsideScriptBlock,
		VariableRedeclaration,
		VariableNameCollisionWithForm,
		VariableNameCollisionWithCommand,
		VariableNameAllNumeric,
		NestedBeginBlock,
		RedundantExpressionPastBlockEnd,
		MismatchingBlock,
		MissingBlockEnd,
		ControlBlockOutsideScriptBlock,
		InvalidScriptBlockForScriptType,
		MissingIfCondition,
		InvalidOperatorNotEquals,
	};

	static enum class eValidatorWarning : DiagnosticMessageCode
	{
		None,

		UnusedLocalVariable,
	};

	static String^ GetObseWarningDescription(eObseWarning Code);
	static String^ GetValidatorWarningDescription(eValidatorWarning Code);
private:
	static Dictionary<eObseWarning, String^>^ ObseWarningDescriptions = gcnew Dictionary<eObseWarning, String^>;
	static Dictionary<eValidatorWarning, String^>^ ValidatorWarningDescriptions = gcnew Dictionary<eValidatorWarning, String^>;
};


static enum	class eScriptType
{
	None = 0,
	Object,
	Quest,
	MagicEffect
};

static enum	class eScriptTokenType
{
	None = 0,
	ScriptName,
	Variable,
	Comment,
	Begin,
	End,
	While,
	ForEach,
	Loop,
	If,
	ElseIf,
	Else,
	EndIf,
	Return,
	SetFunctionValue,
	Set,
	Let,
	Call,
	Player,
	SetEventHandler,
	RemoveEventHandler,
	DispatchEvent,
};

ref class LineTokenizer
{
	bool Good;
	String^ ReferenceDelimiters;
	String^ ReferenceControlChars;
public:
	List<String^>^ Tokens;
	List<UInt32>^ Indices;					// the position of each token relative to its parent line
	List<Char>^ Delimiters;

	property UInt32 TokenCount
	{
		virtual UInt32 get() { return Tokens->Count; }
	}
	property bool Valid
	{
		virtual bool get() { return Good; }
	}

	LineTokenizer();
	LineTokenizer(String^ InputDelimiters);
	LineTokenizer(String^ InputDelimiters, String^ InputControlChars);

	bool Tokenize(String^ Source, bool CollectEmptyTokens);
	void ResetState();

	eScriptTokenType GetFirstTokenType(void);
	int GetCommentTokenIndex(int SearchEndIndex);		// returns the index of the token that contains the comment delimiter. argument specifies the end token index for the search (pass -1 for full search)
	int GetTokenIndex(String^ Source);					// returns the token index corresponding to the first match
	bool IsIndexInsideString(int Index);

	static eScriptTokenType GetScriptTokenType(String^ ScriptToken);
	static bool GetIndexInsideString(String^ Source, int Index);	// returns true if the index follows a " character or if it is wrapped b'ween two
	static bool HasIllegalChar(String^ Source, String^ Includes, String^ Excludes);

	static String^ DefaultDelimiters = "., (){}[]\t\r\n";
	static String^ DefaultControlChars = " \t";
};

ref struct Variable
{
	[System::Runtime::Serialization::DataContract]
	static enum class eDataType
	{
		[System::Runtime::Serialization::EnumMember]
		None = 0,
		[System::Runtime::Serialization::EnumMember]
		Integer,
		[System::Runtime::Serialization::EnumMember]
		Float,
		[System::Runtime::Serialization::EnumMember]
		Reference,
		[System::Runtime::Serialization::EnumMember]
		String,
		[System::Runtime::Serialization::EnumMember]
		Array
	};

	String^ Name;
	String^ Comment;
	eDataType Type;
	bool UDFParameter;
	UInt32 ParameterIndex;		// index of the variable in the UDF parameter list (0-9)
	UInt32 Line;
	UInt32 RefCount;

	Variable(String^ Name, eDataType Type, String^ Comment, UInt32 Line);

	static Variable::eDataType GetVariableDataType(String^ TypeToken);
	static String^ GetVariableDataTypeToken(Variable::eDataType Type);
	static String^ GetVariableDataTypeDescription(Variable::eDataType Type);
};

ref struct ControlBlock
{
	static enum	class eControlBlockType
	{
		None = 0,
		ScriptBlock,
		If,
		ElseIf,
		Else,
		While,
		ForEach
	};

	eControlBlockType Type;
	UInt32 StartLine;
	UInt32 EndLine;				// line with the block end specifier
	UInt32 IndentLevel;			// indent count for the block's contents
	ControlBlock^ Parent;		// nullptr for script blocks
	bool BasicBlock;			// set to true for all blocks except IFs with ELSE/ELSEIF clauses
	UInt32 OuterEndLine;		// set to the ENDIF line for IF's with ELSE/ELSEIF clauses
	List<ControlBlock^>^ ChildBlocks;

	ControlBlock(eControlBlockType Type, UInt32 Start, UInt32 Indents, ControlBlock^ Parent);

	bool IsMalformed();
};

ref struct ScriptBlock : public ControlBlock
{
	static enum	class eScriptBlockAttribute
	{
		None = 0,
		Universal,
		ReferenceSpecific,
		MagicEffect
	};

	static enum class eScriptBlockType
	{
		None = 0,
		GameMode,
		MenuMode,
		OnActivate,
		OnActorEquip,
		OnActorUnequip,
		OnAdd,
		OnAlarm,
		OnAlarmVictim,
		OnDeath,
		OnDrop,
		OnEquip,
		OnHit,
		OnHitWith,
		OnKnockout,
		OnLoad,
		OnMagicEffectHit,
		OnMurder,
		OnPackageChange,
		OnPackageDone,
		OnPackageStart,
		OnReset,
		OnSell,
		OnTrigger,
		OnTriggerActor,
		OnTriggerMob,
		OnUnequip,
		ScriptEffectFinish,
		ScriptEffectStart,
		ScriptEffectUpdate,
		Function,
	};

	eScriptBlockAttribute ScriptBlockAttribute;
	eScriptBlockType ScriptBlockType;
	bool CompilerOverride;

	ScriptBlock(UInt32 Start, UInt32 Indents, eScriptBlockType Type, bool Override);

	bool IsBlockValid(eScriptType Type);

	static eScriptBlockType GetScriptBlockType(String^ TypeToken);
	static String^ GetScriptBlockTypeToken(eScriptBlockType Type);
	static bool HasCompilerOverride(String^ TypeToken);
};

ref struct AnalysisData
{
private:
	static Dictionary<DiagnosticMessageCodes::eValidatorError, String^>^ ErrorCodeToFormatString = gcnew Dictionary<DiagnosticMessageCodes::eValidatorError, String^>;
	static Dictionary<DiagnosticMessageCodes::eValidatorWarning, String^>^ WarningCodeToFormatString = gcnew Dictionary<DiagnosticMessageCodes::eValidatorWarning, String^>;

	void InitializeFormatStrings();
	void LogError(DiagnosticMessageCodes::eValidatorError Code, UInt32 Line, ...array<String^>^ Args);
	void LogWarning(DiagnosticMessageCodes::eValidatorWarning Code, UInt32 Line, ...array<String^>^ Args);
	bool ParseConditionExpression(UInt32 Line, String^ Expression);
public:
	String^ Name;
	String^ Description;
	List<Variable^>^ Variables;
	UInt32 NextVariableLine;			// line where the next variable can be inserted
	List<ControlBlock^>^ ControlBlocks;
	bool MalformedStructure;
	UInt32 FirstStructuralErrorLine;
	bool IsUDF;
	Variable^ UDFResult;
	bool UDFAmbiguousResult;
	List<DiagnosticMessage^>^ AnalysisErrors;
	List<DiagnosticMessage^>^ AnalysisWarnings;

	property bool HasErrors
	{
		bool get() { return AnalysisErrors->Count > 0; }
	}
	property bool Valid
	{
		bool get() { return Name->Length != 0; }
	}

	[Flags]
	static enum class eOperation
	{
		None = 0,
		FillVariables = 1 << 0,
		FillControlBlocks = 1 << 1,
		FillUDFData = 1 << 2,

		PerformBasicValidation = 1 << 3,
		CountVariableReferences = 1 << 4,
		SuppressQuestVariableRefCount = 1 << 5,
		CheckVariableNameCommandCollisions = 1 << 6,
		CheckVariableNameFormCollisions = 1 << 7,
	};

	ref struct Params
	{
		property String^ ScriptText;
		property eScriptType Type;
		property eOperation Ops;
		property System::Collections::Generic::ICollection<String^>^ ScriptCommandIdentifiers;
		property System::Collections::Generic::ICollection<String^>^ FormIdentifiers;

		Params();
	};

	AnalysisData();
	AnalysisData(Params^ Parameters);
	~AnalysisData();

	AnalysisData^ PerformAnalysis(Params^ Parameters);

	ControlBlock^ GetBlockStartingAt(UInt32 Line);
	ControlBlock^ GetBlockEndingAt(UInt32 Line);
	UInt32 GetLineIndentLevel(UInt32 Line);
	Variable^ LookupVariable(String^ VarName);
	AnalysisData^ Clone();
};

ref class Sanitizer
{
	String^ InputText;
	AnalysisData^ Data;
	String^ SanitizedText;
public:
	Sanitizer(String^ Source);

	[Flags]
	static enum class eOperation
	{
		IndentLines = 1 << 0,
		AnnealCasing = 1 << 1,
		EvalifyIfs = 1 << 2,
		CompilerOverrideBlocks = 1 << 3,
	};

	property String^ Output
	{
		virtual String^	get() { return SanitizedText; }
	}

	delegate String^ GetSanitizedIdentifier(String^ Identifier);

	bool SanitizeScriptText(eOperation Operations, GetSanitizedIdentifier^ Delegate);		// returns false if unsuccessful

	static String^ PerformLocalizedIndenting(String^ Source, UInt32 DefaultIndentLevel);
};

ref class Documenter
{
	String^ InputText;
	String^ DocumentedText;

	String^ GetVariableDescription(String^ Identifier, Dictionary<String^, String^>^ Descriptions);
public:
	Documenter(String^ Source);

	property String^ Output
	{
		virtual String^	get() { return DocumentedText; }
	}

	void Document(String^ ScriptDescription, Dictionary<String^, String^>^ VariableDescriptions);
};

ref class Structurizer
{
public:
	ref struct Node
	{
		enum class eNodeType
		{
			Invalid,
			VariableDeclaration,
			ScriptBlock,
			BasicConditionalBlock,
			LoopBlock
		};

		String^ Description;
		eNodeType Type;
		UInt32 StartLine;
		UInt32 EndLine;
		List<Node^>^ Children;

		Node(eNodeType Type, UInt32 StartLine, UInt32 EndLine, String^ Desc);
	};

	delegate String^ GetLineText(UInt32 Line);
private:
	AnalysisData^ InputData;
	List<Node^>^ ParsedTree;
	GetLineText^ FetchLineText;
	UInt32 CurrentLine;

	void ParseStructure();
	void ParseControlBlock(ControlBlock^ Block, Node^ Parent);
	Node^ GetContainingNode(Node^ Source, UInt32 Line, Node^ LastHit);
public:
	Structurizer(AnalysisData^ Input, GetLineText^ Delegate, UInt32 CurrentLine);

	property String^ RootName
	{
		String^ get() { return InputData->Name; }
	}
	property List<Node^>^ Output
	{
		List<Node^>^ get() { return ParsedTree; }
	}

	property bool Valid;
	property Node^ CurrentScope;

	Node^ GetContainingNode(UInt32 Line);
};


interface class IObScriptIdentifiers
{
	property IEnumerable<String^>^ ScriptKeywords
	{
		IEnumerable<String^>^ get();
	}

	property IEnumerable<String^>^ ScriptBlocks
	{
		IEnumerable<String^>^ get();
	}
};


ref class ObseIdentifiers : public IObScriptIdentifiers
{
	List<String^>^ Keywords;
	List<String^>^ Blocks;
public:
	ObseIdentifiers();

	property IEnumerable<String^>^ ScriptKeywords
	{
		virtual IEnumerable<String^>^ get() { return Keywords; }
	}

	property IEnumerable<String^>^ ScriptBlocks
	{
		virtual IEnumerable<String^>^ get() { return Blocks; }
	}
};


} // namespace obScriptParsing


} // namespace scriptEditor


} // namespace cse