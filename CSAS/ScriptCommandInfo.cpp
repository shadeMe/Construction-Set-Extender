#include "ScriptCommandInfo.h"

namespace CSAutomationScript
{
	const char* CSASCommandInfo::GetCommandPrototype()
	{
		if (Prototype == "")
		{
			Prototype.clear();

			switch (ReturnType)
			{
			case CSASDataElement::kParamType_Numeric:
				Prototype += "f:";
				break;
			case CSASDataElement::kParamType_String:
				Prototype += "s:";
				break;
			case CSASDataElement::kParamType_Invalid:
			case CSASDataElement::kParamType_Reference:
				Prototype += "i:";
				break;
			case CSASDataElement::kParamType_Array:
				Prototype += "a:";
				break;
			}

			for (int i = 0; i < NoOfParams; i++)
			{
				CSASParamInfo* CurrentParam = &Parameters[i];
				switch (CurrentParam->ParamType)
				{
				case CSASDataElement::kParamType_Numeric:
					Prototype += "f";
					break;
				case CSASDataElement::kParamType_String:
					Prototype += "s";
					break;
				case CSASDataElement::kParamType_Reference:
					Prototype += "i";
					break;
				}
			}

			if (NoOfParams < 1)
				Prototype += "v";
		}

		return Prototype.c_str();
	}

	void CSASCommandInfo::DumpDocumentation(std::string& Out)
	{
		// ### TODO
	}
}