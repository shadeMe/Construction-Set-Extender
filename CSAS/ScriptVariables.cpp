#include "ScriptVariables.h"

namespace CSAutomationScript
{

	const mup::Value& ScriptVariable::GetValue() const
	{
		return this->Value;
	}

	bool ScriptVariable::SetValue(mup::IValue& Value)
	{
		this->Value = Value;
		return true;
	}

	UInt8 ScriptVariable::GetDataType() const
	{
		switch (this->Value.GetType())
		{
		case 'i':
			return CSASDataElement::kParamType_Reference;
		case 'f':
		case 'c':
			return CSASDataElement::kParamType_Numeric;
		case 'a':
			return CSASDataElement::kParamType_Array;
		case 's':
			return CSASDataElement::kParamType_String;
		default:
			return CSASDataElement::kParamType_Invalid;
		}
	}
}