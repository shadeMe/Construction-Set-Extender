#include "Script.h"
#include "Hooks\Hooks-ScriptEditor.h"

using namespace cse;

TESScriptCompiler::_ShowMessage			TESScriptCompiler::ShowMessage = (TESScriptCompiler::_ShowMessage)0x004FFF40;
bool									TESScriptCompiler::PreventErrorDetours = false;
bool									TESScriptCompiler::PrintErrorsToConsole = true;
TESScriptCompiler::CompilerErrorArrayT	TESScriptCompiler::AuxiliaryErrorDepot;

Script::VariableInfo* Script::LookupVariableInfoByName(const char* Name)
{
	for (VariableListT::Iterator Itr = varList.Begin(); !Itr.End(); ++Itr)
	{
		VariableInfo* Variable = Itr.Get();

		if (Variable && !Variable->name.Compare(Name))
			return Variable;
	}

	return nullptr;
}

Script::RefVariable* Script::LookupRefVariableByIndex(UInt32 Index)
{
	UInt32 Idx = 1;	// yes, really starts at 1

	for (RefVariableListT::Iterator Itr = refList.Begin(); !Itr.End(); ++Itr)
	{
		RefVariable* Variable = Itr.Get();

		if (Variable && Idx == Index)
			return Variable;

		Idx++;
	}

	return nullptr;
}

bool Script::Compile(bool AsResultScript)
{
	if (this->text == nullptr)
		return false;

	if (AsResultScript)
		return thisCall<bool>(0x005034E0, 0x00A0B128, this, 0, 0);
	else
		return thisCall<bool>(0x00503450, 0x00A0B128, this, 0);
}

void Script::SetText(const char* Text)
{
	thisCall<UInt32>(0x004FC6C0, this, Text);
}

UInt32 Script::GetScriptableFormUseCount( void )
{
	return thisCall<UInt32>(0x004FC190, this);
}

bool Script::IsObjectScript() const
{
	return info.type == kScriptType_Object;
}

bool Script::IsQuestScript() const
{
	return info.type == kScriptType_Quest;
}

bool Script::IsMagicScript() const
{
	return info.type == kScriptType_Magic;
}

bool Script::IsUserDefinedFunctionScript() const
{
	if (!IsObjectScript())
		return false;
	else if (info.dataLength < 15)
		return false;

	// need to SEH-wrap this to be safe
	__try
	{
		auto Data = (UInt8*)data;
		if (Data && *(Data + 8) == 7)
			return true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {}

	return false;
}

void TESScriptCompiler::ToggleScriptCompilation( bool State )
{
	if (!State)
		hooks::_MemHdlr(ToggleScriptCompilingNewData).WriteBuffer();
	else
		hooks::_MemHdlr(ToggleScriptCompilingOriginalData).WriteBuffer();
}

UInt32 Script::GetEffectItemReferences(ScriptMagicItemCrossRefArrayT& OutList)
{
	OutList.clear();

	for (cseOverride::NiTMapIterator Itr = TESForm::FormIDMap->GetFirstPos(); Itr;)
	{
		UInt32 FormID = 0;
		TESForm* Form = nullptr;

		TESForm::FormIDMap->GetNext(Itr, FormID, Form);
		if (FormID && Form)
		{
			if (Form->formType == TESForm::kFormType_SigilStone ||
				Form->formType == TESForm::kFormType_Enchantment ||
				Form->formType == TESForm::kFormType_AlchemyItem ||
				Form->formType == TESForm::kFormType_Spell)
			{
				MagicItem* Item = CS_CAST(Form, TESForm, MagicItem);
				if (Item)
				{
					for (EffectItemList::EffectItemListT::Iterator Itr = Item->effects.Begin(); Itr.Get() && !Itr.End(); ++Itr)
					{
						EffectItem* Current = Itr.Get();
						if (Current->scriptInfo && Current->scriptInfo->scriptFormID == this->formID)
							OutList.push_back(Form);
					}
				}
			}
		}
	}

	return OutList.size();
}

void Script::RemoveCompiledData(void)
{
	FormHeap_Free(data);
	data = nullptr;

	info.dataLength = 0;
	info.lastVarIdx = 0;
	info.refCount = 0;
	compileResult = 0;

	thisCall<void>(0x004FF830, this);		// cleanup ref var list
	thisCall<void>(0x004FF7D0, this);		// cleanup var list
}