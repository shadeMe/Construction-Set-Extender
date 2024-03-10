#pragma once

#include "[BGSEEBase]\Script\CodaVM.h"
#include "[BGSEEBase]\Script\Commands\CodaScriptCommand.h"

namespace cse
{
	namespace script
	{
		void ExtractFormArguments(UInt32 Count, ...);

		void Initialize();
		void Deinitialize();
	}
}