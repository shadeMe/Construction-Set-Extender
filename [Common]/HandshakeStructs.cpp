#include "HandShakeStructs.h"

namespace componentDLLInterface
{
	enum
	{
		kCounter_FormData = 0,
		kCounter_FormListData,
		kCounter_ScriptData,
		kCounter_ScriptListData,
		kCounter_QuestData,
		kCounter_VarData,
		kCounter_ISUpdateData,
		kCounter_ScriptVarListData,
		kCounter_ScriptCompileData,
		kCounter_ScriptVarRenameData,
		kCounter_UseInfoCellItemData,
		kCounter_UseInfoCellItemListData,
		kCounter_CellObjectData,
		kCounter_BatchRefData,
		kCounter_TagBrowserInstantiationData,

		kCounter__MAX
	};

	static UInt32		kHandShakeStructCounters[kCounter__MAX] = { 0 };
	static const char*	kHandShakeStructTypeIDs[kCounter__MAX] =
	{
		"FormData",
		"FormListData",
		"ScriptData",
		"ScriptListData",
		"QuestData",
		"VarData",
		"ISUpdateData",
		"ScriptVarListData",
		"ScriptCompileData",
		"ScriptVarRenameData",
		"UseInfoCellItemData",
		"UseInfoCellItemListData",
		"CellObjectData",
		"BatchRefData",
		"TagBrowserInstantiationData",
	};

	void FormData::FillFormData(TESForm* Form)
	{
		EditorID = Form->editorID.c_str();
		FormID = Form->formID;
		TypeID = Form->formType;
		Flags = Form->formFlags;
		ParentForm = Form;
		ObjectReference = Form->IsReference();

		if (ObjectReference)
			BaseFormEditorID = (CS_CAST(Form, TESForm, TESObjectREFR))->baseForm->editorID.c_str();

		auto ParentFile = Form->GetOverrideFile(-1);
		if (ParentFile)
			ParentPluginName = ParentFile->fileName;
	}

	FormData::FormData() : EditorID(nullptr), FormID(0), TypeID(0), Flags(0), ParentForm(nullptr), ParentPluginName(nullptr), ObjectReference(false), BaseFormEditorID(nullptr)
	{
		kHandShakeStructCounters[kCounter_FormData]++;
	}

	FormData::FormData(TESForm* Parent)
	{
		kHandShakeStructCounters[kCounter_FormData]++;
		FillFormData(Parent);
	}

	FormData::~FormData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_FormData] > 0);
		kHandShakeStructCounters[kCounter_FormData]--;
	}

	bool TryGetUDF(Script* Form, bool& IsUDF)
	{
		// need to SEH-wrap this to be safe
		bool Result = false;
		IsUDF = false;

		__try
		{
			UInt8* data = (UInt8*)Form->data;
			if (data && *(data + 8) == 7)
				IsUDF = true;

			Result = true;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			Result = false;
		}

		return Result;
	}

	void ScriptData::FillScriptData(Script* Form)
	{
		FillFormData(Form);

		Text = Form->text;
		Type = Form->info.type;
		ByteCode = Form->data;
		Length = Form->info.dataLength;
		UDF = false;
		if (Form->info.type == Script::kScriptType_Object && Form->info.dataLength >= 15)
			TryGetUDF(Form, UDF);

		Compiled = (Form->data != nullptr && Length > 0);
	}

	ScriptData::ScriptData() : FormData(), Text(0), Type(0), ModifiedFlag(0), ByteCode(0), Length(0), ParentID(0), UDF(false), Compiled(false)
	{
		kHandShakeStructCounters[kCounter_ScriptData]++;
	}

	ScriptData::ScriptData(Script* Parent)
	{
		kHandShakeStructCounters[kCounter_ScriptData]++;
		FillScriptData(Parent);
	}

	ScriptData::~ScriptData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_ScriptData] > 0);
		kHandShakeStructCounters[kCounter_ScriptData]--;
	}

	void VariableData::FillVariableData(GameSetting* GMST)
	{
		const char* VariableName = GMST->editorID.c_str();
		if (*VariableName == 's' || *VariableName == 'S')
		{
			Type = kType_String;
			Value.s = GMST->value.s;
		}
		else if (*VariableName == 'i')
		{
			Type = kType_Int;
			Value.i = GMST->value.i;
		}
		else if (*VariableName == 'f')
		{
			Type = kType_Float;
			Value.f = GMST->value.f;
		}
		else
			throw std::runtime_error("Unknown value type for GMST " + std::string(VariableName));
	}

	void VariableData::FillVariableData(TESGlobal* Global)
	{
		if (Global->globalType == TESGlobal::kGlobalType_Float)
		{
			Type = kType_Float;
			Value.f = Global->value;
		}
		else
		{
			Type = kType_Int;
			Value.i = static_cast<int>(Global->value);
		}
	}

	VariableData::VariableData() : FormData()
	{
		kHandShakeStructCounters[kCounter_VarData]++;
		Type = kType_Int;
		Value.i = 0;
	}

	VariableData::~VariableData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_VarData] > 0);
		kHandShakeStructCounters[kCounter_VarData]--;
	}

	FormListData::FormListData() : FormListHead(0), FormCount(0)
	{
		kHandShakeStructCounters[kCounter_FormListData]++;
	}

	FormListData::~FormListData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_FormListData] > 0);
		kHandShakeStructCounters[kCounter_FormListData]--;
		delete[] FormListHead;
	}

	ScriptListData::ScriptListData()
	{
		kHandShakeStructCounters[kCounter_ScriptListData]++;
		ScriptListHead = 0;
		ScriptCount = 0;
	}

	ScriptListData::~ScriptListData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_ScriptListData] > 0);
		kHandShakeStructCounters[kCounter_ScriptListData]--;
		delete[] ScriptListHead;
	}

	QuestData::QuestData() : FormData()
	{
		kHandShakeStructCounters[kCounter_QuestData]++;
		FullName = 0;
		ScriptName = 0;
	}

	QuestData::~QuestData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_QuestData] > 0);
		kHandShakeStructCounters[kCounter_QuestData]--;
	}

	IntelliSenseUpdateData::IntelliSenseUpdateData()
	{
		kHandShakeStructCounters[kCounter_ISUpdateData]++;
		ScriptListHead = 0;
		ScriptCount = 0;

		QuestListHead = 0;
		QuestCount = 0;

		GlobalListHead = 0;
		GlobalCount = 0;

		GMSTListHead = 0;
		GMSTCount = 0;

		MiscFormListHead = 0;
		MiscFormListCount = 0;
	}

	IntelliSenseUpdateData::~IntelliSenseUpdateData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_ISUpdateData] > 0);
		kHandShakeStructCounters[kCounter_ISUpdateData]--;
		delete[] ScriptListHead;
		delete[] QuestListHead;
		delete[] GlobalListHead;
		delete[] GMSTListHead;
		delete[] MiscFormListHead;
	}

	ScriptVarListData::ScriptVarListData()
	{
		kHandShakeStructCounters[kCounter_ScriptVarListData]++;

		ScriptVarListHead = 0;
		ScriptVarListCount = 0;
	}

	ScriptVarListData::~ScriptVarListData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_ScriptVarListData] > 0);
		kHandShakeStructCounters[kCounter_ScriptVarListData]--;
		delete[] ScriptVarListHead;
	}

	ScriptErrorListData::ScriptErrorListData()
	{
		ErrorListHead = 0;
		Count = 0;
	}

	ScriptErrorListData::~ScriptErrorListData()
	{
		delete[] ErrorListHead;
	}

	ScriptVarRenameData::ScriptVarRenameData()
	{
		kHandShakeStructCounters[kCounter_ScriptVarRenameData]++;

		ParentScriptID = 0;
		ScriptVarListHead = 0;
		ScriptVarListCount = 0;
	}

	ScriptVarRenameData::~ScriptVarRenameData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_ScriptVarRenameData] > 0);
		kHandShakeStructCounters[kCounter_ScriptVarRenameData]--;
		delete[] ScriptVarListHead;
	}

	CommandTableData::CommandTableData()
	{
		CommandTableStart = nullptr;
		CommandTableEnd = nullptr;
		GetCommandReturnType = nullptr;
		GetParentPlugin = nullptr;
		GetRequiredOBSEVersion = nullptr;
		DeveloperURLDataListHead = nullptr;
	}

	CommandTableData::~CommandTableData()
	{
		delete[] DeveloperURLDataListHead;
	}

	UseInfoListCellItemData::UseInfoListCellItemData() : FormData()
	{
		kHandShakeStructCounters[kCounter_UseInfoCellItemData]++;

		WorldEditorID = 0;
		RefEditorID = 0;
		RefFormID = 0;
		XCoord = 0;
		YCoord = 0;
		UseCount = 0;
	}

	UseInfoListCellItemData::~UseInfoListCellItemData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_UseInfoCellItemData] > 0);
		kHandShakeStructCounters[kCounter_UseInfoCellItemData]--;
	}

	UseInfoListCellItemListData::UseInfoListCellItemListData()
	{
		kHandShakeStructCounters[kCounter_UseInfoCellItemListData]++;

		UseInfoListCellItemListHead = 0;
		UseInfoListCellItemListCount = 0;
	}

	UseInfoListCellItemListData::~UseInfoListCellItemListData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_UseInfoCellItemListData] > 0);
		kHandShakeStructCounters[kCounter_UseInfoCellItemListData]--;
		delete[] UseInfoListCellItemListHead;
	}

	CellObjectData::CellObjectData() : FormData()
	{
		kHandShakeStructCounters[kCounter_CellObjectData]++;
		Selected = false;
	}

	CellObjectData::~CellObjectData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_CellObjectData] > 0);
		kHandShakeStructCounters[kCounter_CellObjectData]--;
	}

	BatchRefData::BatchRefData()
	{
		kHandShakeStructCounters[kCounter_BatchRefData]++;

		CellObjectListHead = 0;
		ObjectCount = 0;
	}

	BatchRefData::~BatchRefData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_BatchRefData] > 0);
		kHandShakeStructCounters[kCounter_BatchRefData]--;

		delete[] CellObjectListHead;
	}

	ScriptCompileData::ScriptCompileData() : Script(), CompileResult(false), CompileErrorData()
	{
		kHandShakeStructCounters[kCounter_ScriptCompileData]++;
	}

	ScriptCompileData::~ScriptCompileData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_ScriptCompileData] > 0);
		kHandShakeStructCounters[kCounter_ScriptCompileData]--;
	}

	TagBrowserInstantiationData::TagBrowserInstantiationData()
	{
		kHandShakeStructCounters[kCounter_TagBrowserInstantiationData]++;

		FormListHead = 0;
		FormCount = 0;
	}

	TagBrowserInstantiationData::~TagBrowserInstantiationData()
	{
		SME_ASSERT(kHandShakeStructCounters[kCounter_TagBrowserInstantiationData] > 0);
		kHandShakeStructCounters[kCounter_TagBrowserInstantiationData]--;
		delete[] FormListHead;
	}

	void DumpInstanceCounters(void)
	{
		bool Leakage = false;

		BGSEECONSOLE_MESSAGE("HandShakeStruct Counters:");
		BGSEECONSOLE->Indent();

		for (int i = kCounter_FormData; i < kCounter__MAX; i++)
		{
			UInt32 Count = kHandShakeStructCounters[i];
			if (Count)
			{
				BGSEECONSOLE_MESSAGE("%s = %d", kHandShakeStructTypeIDs[i], Count);
				Leakage = true;
			}
		}

		if (Leakage)
			BGSEEUI->MsgBoxE("HandShakeStructs leakage! Counters dumped to console");
		else
			BGSEECONSOLE_MESSAGE("No delinquents");

		BGSEECONSOLE->Outdent();
	}
}
