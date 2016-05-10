#pragma once
#include <BGSEEMain.h>
#include "[Common]\ComponentDLLInterface.h"

namespace ConstructionSetExtender
{
	extern bool								shadeMeMode;
	extern bool								IsWarholAGenius;

	class CSEReleaseNameTable : public bgsee::BGSEEReleaseNameTable
	{
	protected:
		CSEReleaseNameTable();
	public:
		virtual ~CSEReleaseNameTable();

		static CSEReleaseNameTable						Instance;
	};

	class CSEInitCallbackQuery : public bgsee::BGSEEDaemonCallback
	{
		const OBSEInterface*				OBSE;
	public:
		CSEInitCallbackQuery(const OBSEInterface* OBSE);
		virtual ~CSEInitCallbackQuery();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class CSEInitCallbackLoad : public bgsee::BGSEEDaemonCallback
	{
		const OBSEInterface*				OBSE;
	public:
		CSEInitCallbackLoad(const OBSEInterface* OBSE);
		virtual ~CSEInitCallbackLoad();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class CSEInitCallbackPostMainWindowInit : public bgsee::BGSEEDaemonCallback
	{
	public:
		virtual ~CSEInitCallbackPostMainWindowInit();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class CSEInitCallbackEpilog : public bgsee::BGSEEDaemonCallback
	{
	public:
		virtual ~CSEInitCallbackEpilog();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class CSEDeinitCallback : public bgsee::BGSEEDaemonCallback
	{
	public:
		virtual ~CSEDeinitCallback();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class CSECrashCallback : public bgsee::BGSEEDaemonCallback
	{
		enum
		{
			kCrashHandlerMode_Terminate = 0,
			kCrashHandlerMode_Resume,
			kCrashHandlerMode_Ask,
		};
	public:
		virtual ~CSECrashCallback();

		virtual bool						Handle(void* Parameter = NULL);
	};

	class CSEStartupManager
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
