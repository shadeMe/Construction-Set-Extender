#pragma once
#include <BGSEEMain.h>
#include "[Common]\ComponentDLLInterface.h"

namespace ConstructionSetExtender
{
	extern OBSEMessagingInterface*						XSEMsgIntfc;
	extern PluginHandle									XSEPluginHandle;

	extern OBSECommandTableInterface*					XSECommandTableIntfc;
	extern ComponentDLLInterface::CommandTableData		XSECommandTableData;

	class InitCallbackQuery : public BoolRFunctorBase
	{
		const OBSEInterface*				OBSE;
	public:
		InitCallbackQuery(const OBSEInterface* OBSE);
		virtual ~InitCallbackQuery();

		virtual bool						operator()();
	};

	class InitCallbackLoad : public BoolRFunctorBase
	{
		const OBSEInterface*				OBSE;
	public:
		InitCallbackLoad(const OBSEInterface* OBSE);
		virtual ~InitCallbackLoad();

		virtual bool						operator()();
	};

	class InitCallbackEpilog : public BoolRFunctorBase
	{
	public:
		virtual ~InitCallbackEpilog();

		virtual bool						operator()();
	};

	class DeinitCallback : public BoolRFunctorBase
	{
	public:
		virtual ~DeinitCallback();

		virtual bool						operator()();
	};

	class CrashCallback : public BoolRFunctorBase
	{
	public:
		virtual ~CrashCallback();

		virtual bool						operator()();
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
}

extern "C"
{
	__declspec(dllexport) bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info);
	__declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface * obse);
};
