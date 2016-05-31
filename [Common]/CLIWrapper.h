#pragma once
#include "ComponentDLLInterface.h"

struct OBSEInterface;
namespace cse
{
	namespace cliWrapper
	{
		namespace interfaces
		{
			extern componentDLLInterface::ScriptEditorInterface*	SE;
			extern componentDLLInterface::UseInfoListInterface*		USE;
			extern componentDLLInterface::BSAViewerInterface*		BSA;
			extern componentDLLInterface::BatchEditorInterface*		BE;
			extern componentDLLInterface::TagBrowserInterface*		TAG;
		}

		bool														ImportInterfaces(const OBSEInterface* obse);
		void														QueryInterfaces(void);
	}
}
