#pragma once

class TESForm;
#ifdef CSE
class Script;
struct CommandInfo;
struct PluginInfo;
class TESGlobal;
#endif

struct FormData
{
	const char*										EditorID;
	UInt32											FormID;
	UInt8											TypeID;
	UInt32											Flags;
	TESForm*										ParentForm;

	virtual bool									IsValid() { return (EditorID) ? true : false; }
	void											FillFormData(TESForm* Form);
};

struct ScriptData : public FormData
{
	const char*										Text;
	UInt16											Type;
	bool											ModifiedFlag;
	void*											ByteCode;
	UInt32											Length;
	const char*										ParentID;
	bool											UDF;
#ifdef CSE
	void											FillScriptData(Script* Form);
#endif
};

struct QuestData : public FormData
{
	const char*										FullName;
	const char*										ScriptName;
};

struct VariableData : public FormData
{
	enum
	{
		kType_Int = 0,
		kType_Float,
		kType_String
	};

	UInt8											Type;
#ifdef CSE
	void											FillVariableData(const char* VariableName);
	void											FillVariableData(TESGlobal* Global);
#endif
};
typedef VariableData GlobalData;
typedef VariableData GMSTData;

struct IntelliSenseUpdateData
{
	ScriptData*										ScriptListHead;
	UInt32											ScriptCount;

	QuestData*										QuestListHead;
	UInt32											QuestCount;

	GlobalData*										GlobalListHead;
	UInt32											GlobalCount;

	GMSTData*										GMSTListHead;
	UInt32											GMSTCount;
};

struct ScriptVarIndexData
{
	struct ScriptVarInfo
	{
		const char*									Name;
		UInt8										Type;
		UInt32										Index;
	};

	ScriptVarInfo*									Data;
};

#ifndef CSE
struct ObScriptCommandInfo
{
	const char*										longName;
	const char*										shortName;
	UInt32											opcode;
	const char* 									helpText;
	UInt16											needsParent;
	UInt16											numParams;
	void*											params;

	void*											HandlerA;
	void*											HandlerB;
	void*											HandlerC;

	UInt32											flags;
};
#endif

struct CommandTableData
{
#ifdef CSE
	const CommandInfo*								CommandTableStart;
	const CommandInfo*								CommandTableEnd;
	UInt32											(* GetCommandReturnType)(const CommandInfo* cmd);
	const PluginInfo*								(* GetParentPlugin)(const CommandInfo* cmd);
	UInt32											(* GetRequiredOBSEVersion)(const CommandInfo* cmd);
#else
	struct PluginInfo
	{
		UInt32			infoVersion;
		const char *	name;
		UInt32			version;
	};

	const ObScriptCommandInfo*						CommandTableStart;
	const ObScriptCommandInfo*						CommandTableEnd;
	UInt32											(* GetCommandReturnType)(const ObScriptCommandInfo* cmd);
	const PluginInfo*								(* GetParentPlugin)(const ObScriptCommandInfo* cmd);
	UInt32											(* GetRequiredOBSEVersion)(const ObScriptCommandInfo* cmd);
#endif

	void											DumpData();
};

struct UseListCellItemData : public FormData
{
	const char*										WorldEditorID;
	const char*										RefEditorID;
	int												XCoord;
	int												YCoord;
	UInt32											UseCount;
};

struct CellObjectData : public FormData
{
	bool											Selected;

	bool											IsValid() { return (FormID) ? true : false; }
};

struct BatchRefData
{
	CellObjectData*									CellObjectListHead;
	UInt32											ObjectCount;

	enum
	{
		kListID_NPC = 0,
		kListID_Global,
		kListID_Faction
	};

	struct _3DData
	{
		float										RotX, RotY, RotZ;
		float										PosX, PosY, PosZ;
		float										Scale;

		enum
		{
			kFlag_UsePosX = 1 << 0,
			kFlag_UsePosY = 1 << 1,
			kFlag_UsePosZ = 1 << 2,

			kFlag_UseRotX = 1 << 3,
			kFlag_UseRotY = 1 << 4,
			kFlag_UseRotZ = 1 << 5,

			kFlag_UseScale = 1 << 6,
		};
		UInt32										Flags;

		bool UsePosX() { return (Flags & kFlag_UsePosX) ? true : false; }
		bool UsePosY() { return (Flags & kFlag_UsePosY) ? true : false; }
		bool UsePosZ() { return (Flags & kFlag_UsePosZ) ? true : false; }

		bool UseRotX() { return (Flags & kFlag_UseRotX) ? true : false; }
		bool UseRotY() { return (Flags & kFlag_UseRotY) ? true : false; }
		bool UseRotZ() { return (Flags & kFlag_UseRotZ) ? true : false; }
		bool UseScale() { return (Flags & kFlag_UseScale) ? true : false; }
	};
	struct _Flags
	{
		bool										Persistent;
		bool										VWD;
		bool										Disabled;

		enum
		{
			kFlag_UsePersistent = 1 << 0,
			kFlag_UseVWD = 1 << 1,
			kFlag_UseDisabled = 1 << 2
		};
		UInt32										Flags;

		bool UsePersistent() { return (Flags & kFlag_UsePersistent) ? true : false; }
		bool UseVWD() { return (Flags & kFlag_UseVWD) ? true : false; }
		bool UseDisabled() { return (Flags & kFlag_UseDisabled) ? true : false; }
	};

	struct _Extra
	{
		float										Health,
													TimeLeft;
		int											Count,
													Charge;
		enum Soul
		 {
			kSoul_None = 0,
			kSoul_Petty,
			kSoul_Lesser,
			kSoul_Common,
			kSoul_Greater,
			kSoul_Grand,
		};
		UInt8										SoulLevel;

		enum
		{
			kFlag_UseHealth = 1 << 0,
			kFlag_UseTimeLeft = 1 << 1,
			kFlag_UseCount = 1 << 2,
			kFlag_UseCharge = 1 << 3,
			kFlag_UseSoulLevel= 1 << 4
		};
		UInt32										Flags;

		bool UseHealth() { return (Flags & kFlag_UseHealth) ? true : false; }
		bool UseTimeLeft() { return (Flags & kFlag_UseTimeLeft) ? true : false; }
		bool UseCount() { return (Flags & kFlag_UseCount) ? true : false; }
		bool UseCharge() { return (Flags & kFlag_UseCharge) ? true : false; }
		bool UseSoulLevel() { return (Flags & kFlag_UseSoulLevel) ? true : false; }
	};

	struct _EnableParent
	{
		void*										Parent;				// TESObjectREFR*
		bool										OppositeState;

		enum
		{
			kFlag_UseEnableParent = 1 << 0
		};
		UInt32										Flags;

		bool UseEnableParent() { return (Flags & kFlag_UseEnableParent) ? true : false; }
	};

	struct _Ownership
	{
		void*										Owner;				// TESNPC* or TESFaction*;
		void*										Global;				// TESGlobal*
		int											Rank;

		enum
		{
			kFlag_UseOwnership = 1 << 0,
			kFlag_UseNPCOwner = 1 << 1
		};
		UInt32										Flags;

		bool UseOwnership() { return (Flags & kFlag_UseOwnership) ? true : false; }
		bool UseNPCOwner() { return (Flags & kFlag_UseNPCOwner) ? true : false; }
	};

	_3DData											World3DData;
	_Flags											Flags;
	_Extra											Extra;
	_EnableParent									EnableParent;
	_Ownership										Ownership;
};

struct TagBrowserInstantiationData
{
#ifdef CSE
	POINT											InsertionPoint;
#else
	Point											InsertionPoint;
#endif

	FormData*										FormListHead;
	UInt32											FormCount;
};