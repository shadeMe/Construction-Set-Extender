#pragma once
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

		virtual bool						Handle(void* Parameter = nullptr);
	};

	class InitCallbackLoad : public bgsee::DaemonCallback
	{
		const OBSEInterface*				OBSE;
	public:
		InitCallbackLoad(const OBSEInterface* OBSE);
		virtual ~InitCallbackLoad();

		virtual bool						Handle(void* Parameter = nullptr);
	};

	class InitCallbackPostMainWindowInit : public bgsee::DaemonCallback
	{
		static int __CRTDECL CrtNewHandler(size_t);
		static void __CRTDECL CrtPureCallHandler(void);
		static void __CRTDECL  CrtInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);

		void RegisterCrtExceptionHandlers();
	public:
		virtual ~InitCallbackPostMainWindowInit();

		virtual bool						Handle(void* Parameter = nullptr);
	};

	class InitCallbackEpilog : public bgsee::DaemonCallback
	{
	public:
		virtual ~InitCallbackEpilog();

		virtual bool						Handle(void* Parameter = nullptr);
	};

	class DeinitCallback : public bgsee::DaemonCallback
	{
	public:
		virtual ~DeinitCallback();

		virtual bool						Handle(void* Parameter = nullptr);
	};

	class CrashCallback : public bgsee::DaemonCallback
	{
		bool				HandlerCalled;		// to prevent the handler from being called multiple times (which might happen in case of catastrophic failures)
	public:
		CrashCallback();
		virtual ~CrashCallback();

		virtual bool						Handle(void* Parameter = nullptr);
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

// bgsee::ResourceLocation paths
#define CSE_CODADEPOT					"Coda\\"
#define CSE_CODABGDEPOT					"Coda\\Background\\"
#define CSE_SEDEPOT						"Script Editor\\"
#define CSE_SEPREPROCDEPOT				"Script Editor\\Preprocessor\\"
#define CSE_SEPREPROCSTDDEPOT			"Script Editor\\Preprocessor\\STD\\"
#define CSE_SESNIPPETDEPOT				"Script Editor\\Snippets\\"
#define CSE_SEAUTORECDEPOT				"Script Editor\\Auto-Recovery Cache\\"
#define CSE_OPALDEPOT					"OPAL\\"
#define CSE_PREFABDEPOT					"Object Prefabs\\"
#define CSE_COSAVEDEPOT					"Plugin Cosaves\\"
}

extern "C"
{
	__declspec(dllexport) bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info);
	__declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface * obse);
};
