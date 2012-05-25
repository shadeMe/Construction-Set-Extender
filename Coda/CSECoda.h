#pragma once

#include <BGSEEScript\CodaVM.h>
#include <BGSEEScript\CodaInterpreter.h>
#include <BGSEEScript\Commands\CodaScriptCommand.h>

namespace ConstructionSetExtender
{
	namespace BGSEEScript
	{
		void ExtractFormArguments(UInt32 Count, ...);

		void Initialize();
	}
}