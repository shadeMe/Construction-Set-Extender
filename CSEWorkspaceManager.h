#pragma once
#include <BGSEEWorkspaceManager.h>

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		enum
		{
			kStartupWorkspace_SetWorkspace = 0,
			kStartupWorkspace_WorkspacePath,

			kStartupWorkspace__MAX
		};
		extern const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kStartupWorkspaceINISettings[kStartupWorkspace__MAX];
		BGSEditorExtender::BGSEEINIManagerSettingFactory*								GetStartupWorkspace(void);
	}

	namespace WorkspaceManager
	{
		class CSEWorkspaceManagerOperator : public BGSEditorExtender::BGSEEWorkspaceManagerOperator
		{
		public:
			virtual ~CSEWorkspaceManagerOperator();

			virtual void						ResetCurrentWorkspace(void);
			virtual void						ReloadPlugins(const char* WorkspacePath, bool ResetPluginList, bool LoadESPs);
		};

		void								Initialize(void);
	}
}
