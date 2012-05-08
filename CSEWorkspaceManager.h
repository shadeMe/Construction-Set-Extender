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
		class CSEWorkspaceResetter : public VoidRFunctorBase
		{
		public:
			virtual ~CSEWorkspaceResetter();

			void					operator()();
		};

		class CSEWorkspaceReloader : public BGSEditorExtender::BGSEEWorkspaceManager::ReloadPluginsFunctor
		{
		public:
			virtual ~CSEWorkspaceReloader();

			virtual void					operator()(const char* WorkspacePath, bool ResetPluginList, bool LoadESPs);
		};

		void								Initialize(void);
	}
}
