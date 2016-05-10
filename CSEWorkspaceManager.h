#pragma once
#include <BGSEEWorkspaceManager.h>

namespace ConstructionSetExtender
{
	namespace WorkspaceManager
	{
		class CSEWorkspaceManagerOperator : public bgsee::BGSEEWorkspaceManagerOperator
		{
		public:
			virtual ~CSEWorkspaceManagerOperator();

			virtual void						ResetCurrentWorkspace(void);
			virtual void						ReloadPlugins(const char* WorkspacePath, bool ResetPluginList, bool LoadESPs);
		};

		void								Initialize(void);
	}
}
