#pragma once
#include <bgsee\Main.h>
#include "[Common]\ComponentDLLInterface.h"

namespace cse
{
	extern bool								shadeMeMode;
	extern bool								IsWarholAGenius;

	class ReleaseNameTable : public bgsee::ReleaseNameTable
	{
	protected:
		ReleaseNameTable();
	public:
		virtual ~ReleaseNameTable();

		static ReleaseNameTable						Instance;
	};

	class InitCallbackQuery : public bgsee::DaemonCallback
	{
		const OBSEInterface*				OBSE;
	public:
		InitCallbackQuery(const OBSEInterface* OBSE);
		virtual ~InitCallbackQuery();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class InitCallbackLoad : public bgsee::DaemonCallback
	{
		const OBSEInterface*				OBSE;
	public:
		InitCallbackLoad(const OBSEInterface* OBSE);
		virtual ~InitCallbackLoad();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class InitCallbackPostMainWindowInit : public bgsee::DaemonCallback
	{
	public:
		virtual ~InitCallbackPostMainWindowInit();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class InitCallbackEpilog : public bgsee::DaemonCallback
	{
	public:
		virtual ~InitCallbackEpilog();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class DeinitCallback : public bgsee::DaemonCallback
	{
	public:
		virtual ~DeinitCallback();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class CrashCallback : public bgsee::DaemonCallback
	{
		enum
		{
			kCrashHandlerMode_Terminate = 0,
			kCrashHandlerMode_Resume,
			kCrashHandlerMode_Ask,
		};
	public:
		virtual ~CrashCallback();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class StartupManager
	{
	public:
		static void							LoadStartupWorkspace();
		static void							LoadStartupPlugin();
		static void							LoadStartupScript();
	};

	void CSEInteropHandler(OBSEMessagingInterface::Message* Msg);
	void OBSEMessageHandler(OBSEMessagingInterface::Message* Msg);

#define BGSEEMAIN_EXTENDERLONGNAME		"Construction Set Extender"
#define BGSEEMAIN_EXTENDERSHORTNAME		"CSE"

#define CSE_CODADEPOT					"Coda\\"
#define CSE_CODABGDEPOT					"Coda\\Background\\"
#define CSE_SEDEPOT						"Script Editor\\"
#define CSE_SEPREPROCDEPOT				"Script Editor\\Preprocessor\\"
#define CSE_SEPREPROCSTDDEPOT			"Script Editor\\Preprocessor\\STD\\"
#define CSE_SESNIPPETDEPOT				"Script Editor\\Snippets\\"
#define CSE_SEAUTORECDEPOT				"Script Editor\\Auto-Recovery Cache\\"
#define CSE_OPALDEPOT					"OPAL\\"
#define CSE_PREFABDEPOT					"Object Prefabs\\"
}

extern "C"
{
	__declspec(dllexport) bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info);
	__declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface * obse);
};
