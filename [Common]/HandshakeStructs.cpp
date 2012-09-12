#include "HandShakeStructs.h"

namespace ComponentDLLInterface
{
	void FormData::FillFormData(TESForm* Form)
	{
		EditorID = Form->editorID.c_str();
		FormID = Form->formID;
		TypeID = Form->formType;
		Flags = Form->formFlags;
		ParentForm = Form;
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
		{
			UInt8* data = (UInt8*)Form->data;
			if (data && *(data + 8) == 7)
			{
				UDF = true;
			}
		}

		Compiled = Form->compileResult;
	}

	void VariableData::FillVariableData(const char* VariableName)
	{
		if (*VariableName == 's' || *VariableName == 'S')
			Type = kType_String;
		else if (*VariableName == 'i' || *VariableName == 'u')
			Type = kType_Int;
		else
			Type = kType_Float;
	}

	void VariableData::FillVariableData(TESGlobal* Global)
	{
		if (Global->globalType == TESGlobal::kGlobalType_Float)
			Type = kType_Float;
		else
			Type = kType_Int;
	}
}