#pragma once

#include <bgsee\Script\CodaVM.h>
#include <bgsee\Script\CodaInterpreter.h>
#include <bgsee\Script\Commands\CodaScriptCommand.h>

namespace cse
{
	namespace script
	{
		void ExtractFormArguments(UInt32 Count, ...);

		void Initialize();
		void Deinitialize();
	}
}