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
		ScriptVariable(std::string& Name, double Value) : Name(Name), Value(Value) {}
		ScriptVariable(std::string& Name, std::string& Value) : Name(Name), Value(Value) {}
		ScriptVariable(const ScriptVariable& rhs);

		const char*										GetName() const { return Name.c_str(); }
		void											SetName(std::string& Name) { this->Name = Name; }
		UInt8											GetDataType() const;
		const mup::Value&								GetValue() const;
		bool											SetValue(mup::IValue& Value);
	};
}