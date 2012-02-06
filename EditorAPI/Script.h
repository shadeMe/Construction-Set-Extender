#pragma once

#include "TESForm.h"

//	EditorAPI: Script class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// 54
class Script : public TESForm
{
public:
	// 10
	struct RefVariable
	{
		/*00*/ BSString			name;			// name of the reference variable/editorID of the referenced object (extracted directly from script text)
		/*08*/ TESForm*				form;			// referenced object, in case of explict references
		/*0C*/ UInt32				variableIndex;	// variable index of the reference variable, if any
	};
	typedef tList<RefVariable> RefVariableListT;

	enum VariableTypes
	{
		kVariableTypes_Float	= 0x0,
		kVariableTypes_Integer	= 0x1
	};

	// 20
	struct VariableInfo
	{
		/*00*/ UInt32					index;		// one-based index
		/*04*/ UInt32					unk04;
		/*08*/ double					data;
		/*10*/ UInt8					type;
		/*11*/ UInt8					pad11;
		/*14*/ UInt32					unk14;
		/*18*/ BSString				name;
	};
	typedef tList<VariableInfo> VariableListT;

	enum ScriptType
	{
		kScriptType_Object		= 0x0,
		kScriptType_Quest		= 0x1,
		kScriptType_Magic		= 0x100
	};

	// 14
	struct ScriptInfo
	{
		/*00*/ UInt32					unk00;
		/*04*/ UInt32					refCount;			// number of reference variables
		/*08*/ UInt32					dataLength;
		/*0C*/ UInt32					lastVarIdx;
		/*10*/ UInt32					type;
	};

	// members
	//     /*00*/ TESForm
	/*24*/ ScriptInfo					info;
	/*38*/ char*						text;
	/*3C*/ void*						data;
	/*40*/ RefVariableListT				refList;
	/*48*/ VariableListT				varList;
	/*50*/ UInt8						compileResult;		//	set to 1 after successful compilation, zero otherwise
	/*51*/ UInt8						pad51[3];

	// methods
	VariableInfo*						LookupVariableInfoByName(const char* Name);
	RefVariable*						LookupRefVariableByIndex(UInt32 Index);

	bool								IsObjectScript() const {return info.type == kScriptType_Object; }
	bool								IsQuestScript() const { return info.type == kScriptType_Quest; }
	bool								IsMagicScript() const { return info.type == kScriptType_Magic; }

	bool								Compile(bool AsResultScript = false);
	void								SetText(const char* Text);

	UInt32								GetScriptableFormUseCount(void);
};

enum ScriptCompileErrors
{
	kScriptCompileErrors_MissingScriptName		= 0x01,
	kScriptCompileErrors_ScriptCmdNotFound		= 0x0D,
	kScriptCompileErrors_MaxLineLengthExceeded	= 0x10
};

// 41C
struct ScriptLineBuffer
{
	// members
	/*000*/ UInt32				lineNumber;			// counts blank lines too
	/*004*/ char				paramText[0x200];	// portion of line text following command
	/*204*/ UInt32				paramTextLen;
	/*208*/ UInt32				lineOffset;
	/*20C*/ UInt8				dataBuf[0x200];
	/*40C*/ UInt32				dataOffset;
	/*410*/ UInt32				cmdOpcode;			// opcode of command being parsed
	/*414*/ UInt32				callingRefIndex;	// index of the ref var corresponding to the calling ref, zero if cmd not invoked with dot syntax
	/*418*/ UInt32				errorFlag;
};

// 58? Nothing initialized beyond 0x50
struct ScriptBuffer
{
	typedef tList<ScriptLineBuffer> ScriptLineBufferListT;

	// members
	/*00*/ char*						scriptText;
	/*04*/ UInt32						textOffset;
	/*08*/ UInt32						unk8;				// set when script is compiled in-game (from the console)?
	/*0C*/ BSString					scriptName;
	/*14*/ UInt32						errorFlag;			// set to ScriptLineBuffer::errorFlag
	/*18*/ UInt8						resultScript;		// set to 1 when compiling result scripts, used to skip certain syntax checks
	/*19*/ UInt8						pad19;
	/*1A*/ UInt16						unk1A;
	/*1C*/ UInt32						curLineNumber;
	/*20*/ UInt8*						scriptData;			// pointer to the bytecode array
	/*24*/ UInt32						dataOffset;
	/*28*/ UInt32						unk28;
	/*2C*/ UInt32						numRefs;
	/*30*/ UInt32						lastDataOffset;		// set to this->dataOffset after each successful call to ScriptCompiler::WriteByteCode
	/*34*/ UInt32						lastVarIdx;
	/*38*/ UInt8						scriptType;
	/*39*/ UInt8						unk39;				// initialized to script->unk35
	/*3A*/ UInt8						pad3A[2];
	/*3C*/ Script::VariableListT		vars;
	/*44*/ Script::RefVariableListT		refVars;
	/*4C*/ Script*						parentScript;
	/*50*/ ScriptLineBufferListT		lines;
};

class TESScriptCompiler
{
	typedef void						(__cdecl *_ShowMessage)(ScriptBuffer* Buffer, const char* Format, ...);
public:
	// methods
	static _ShowMessage					ShowMessage;
	static void							ToggleScriptCompilation(bool State);
};