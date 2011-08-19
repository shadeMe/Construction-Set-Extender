#pragma once
#include "ComponentDLLInterface.h"

struct OBSEInterface;

namespace CLIWrapper
{
	namespace Interfaces
	{
		extern ComponentDLLInterface::ScriptEditorInterface*	SE;
		extern ComponentDLLInterface::UseInfoListInterface*		USE;
		extern ComponentDLLInterface::BSAViewerInterface*		BSA;
		extern ComponentDLLInterface::BatchEditorInterface*		BE;
		extern ComponentDLLInterface::TagBrowserInterface*		TAG;
	}

	bool														ImportInterfaces(const OBSEInterface* obse);
	void														QueryInterfaces(void);
}