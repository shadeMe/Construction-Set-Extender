#ifdef CSE_USER
#error interface only code included in user
#endif

#include "Common\HandShakeStructs.h"
#include "ExtenderInternals.h"

void FormData::FillFormData(TESForm* Form)
{
	EditorID = Form->editorData.editorID.m_data;
	FormID = Form->refID;
	TypeID = Form->typeID;
	Flags = Form->flags;
	ParentForm = Form;
}

void ScriptData::FillScriptData(Script* Form)
{
	Text = Form->text;
	Type = Form->Type();
	ByteCode = Form->data;
	Length = Form->info.dataLength;
	UDF = false;
	if (Form->Type() == Script::eType_Object && Form->info.dataLength >= 15)
	{
		UInt8* data = (UInt8*)Form->data;
		if (*(data + 8) == 7)
		{
			UDF = true;
		}
	}
}