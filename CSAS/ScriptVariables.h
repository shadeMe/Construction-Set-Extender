#pragma once

#include "Array.h"
#include "mpValue.h"

namespace CSAutomationScript
{
	class ScriptVariable
	{
		mup::Value										Value;
		std::string										Name;
	public:
		ScriptVariable(std::string& Name) : Name(Name), Value(0.0) {}

		const char*										GetName() const { return Name.c_str(); }
		UInt8											GetDataType() const;
		const mup::Value&								GetValue() const;
		bool											SetValue(mup::IValue& Value);
	};
}