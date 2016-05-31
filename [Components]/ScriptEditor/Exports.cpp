#include "Exports.h"
#include "IntelliSenseDatabase.h"
#include "Globals.h"
#include "WorkspaceModel.h"
#include "WorkspaceView.h"
#include "ScriptEditorPreferences.h"

#include <memory.h>

using namespace componentDLLInterface;
using namespace System::Reflection;
using namespace cse;
using namespace cse::intellisense;
using namespace cse::scriptEditor;

extern componentDLLInterface::ScriptEditorInterface g_InteropInterface;

extern "C"
{
	QUERYINTERFACE_EXPORT
	{
		return &g_InteropInterface;
	}
}

Assembly^ ResolvePreprocessorAssemblyLoad(Object^, ResolveEventArgs^ E)
{
	Assembly^ PreprocAssembly, ^ExecutingAssemblies;
	String^ TempPath = "";

	ExecutingAssemblies = Assembly::GetExecutingAssembly();

	for each(AssemblyName^ AssmbName in ExecutingAssemblies->GetReferencedAssemblies())
	{
		if (AssmbName->FullName->Substring(0, AssmbName->FullName->IndexOf(",")) == E->Name->Substring(0, E->Name->IndexOf(",")))
		{
			TempPath = Globals::AppPath + COMPONENTDLLFOLDER + E->Name->Substring(0, E->Name->IndexOf(",")) + ".dll";
			PreprocAssembly = Assembly::LoadFrom(TempPath);
			return PreprocAssembly;
		}
	}

	return nullptr;
}

void CLRUnhandledExceptionFilter(Object^, UnhandledExceptionEventArgs^ E)
{
	DebugPrint("CLR Unhandled Exception Handler Invoked:");
	DebugPrint("Exception:");
	DebugPrint(E->ExceptionObject->ToString());

	MessageBox::Show("The editor crashed while executing managed code! Check the debug log for details.",
					"Construction Set Extender",
					MessageBoxButtons::OK,
					MessageBoxIcon::Error);
}

UInt32 InitializeComponents(CommandTableData* Data, IntelliSenseUpdateData* GMSTData)
{
	AppDomain^ CurrentDomain = AppDomain::CurrentDomain;
	CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler(&ResolvePreprocessorAssemblyLoad);
	CurrentDomain->UnhandledException += gcnew UnhandledExceptionEventHandler(&CLRUnhandledExceptionFilter);
	Application::EnableVisualStyles();

	UInt32 NonVanillaCommandCount = ISDB->InitializeCommandTableDatabase(Data);
	ISDB->InitializeGMSTDatabase(GMSTData);
	ISDB->ForceUpdateDatabase();

	Globals::MainThreadID = Threading::Thread::CurrentThread->ManagedThreadId;
	Globals::MainThreadTaskScheduler = Threading::Tasks::TaskScheduler::FromCurrentSynchronizationContext();

	System::Windows::Media::RenderOptions::ProcessRenderMode = System::Windows::Interop::RenderMode::Default;

	return NonVanillaCommandCount;
}

void InstantiateEditor(componentDLLInterface::ScriptData* InitializerScript, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)ConcreteWorkspaceViewFactory::Instance->CreateView(Left, Top, Width, Height);
	IWorkspaceModel^ Model = ConcreteWorkspaceModelFactory::Instance->CreateModel(InitializerScript);

	New->AssociateModel(Model, true);
}

void AddScriptCommandDeveloperURL(const char* ScriptCommandName, const char* URL)
{
	ISDB->RegisterDeveloperURL(gcnew String(ScriptCommandName), gcnew String(URL));
}

void CloseAllOpenEditors(void)
{
	ConcreteWorkspaceViewFactory::Instance->Clear();
}

void UpdateIntelliSenseDatabase(void)
{
	ISDB->ForceUpdateDatabase();
}

UInt32 GetOpenEditorCount(void)
{
	return ConcreteWorkspaceModelFactory::Instance->Count;
}

void DummyPreprocessorErrorOutputWrapper(int Line, String^ Message)
{
	;//
}

bool PreprocessScript(const char* ScriptText, char* OutPreprocessed, UInt32 BufferSize)
{
	String^ PreprocessedResult = "";
	bool OperationResult = Preprocessor::GetSingleton()->PreprocessScript(gcnew String(ScriptText),
								PreprocessedResult,
								gcnew scriptPreprocessor::StandardOutputError(&DummyPreprocessorErrorOutputWrapper),
								gcnew ScriptEditorPreprocessorData(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
								gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
								PREFERENCES->FetchSettingAsInt("AllowRedefinitions", "Preprocessor"),
								PREFERENCES->FetchSettingAsInt("NoOfPasses", "Preprocessor")));

	if (OperationResult)
	{
		PreprocessedResult->Replace("\n", "\r\n");
		CString OutText(PreprocessedResult);
		memcpy_s(OutPreprocessed, BufferSize, OutText.c_str(), PreprocessedResult->Length);
	}

	return OperationResult;
}

void Deinitalize(void)
{
	try
	{
		String^ Cache = gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath());

		for each (String^ Path in System::IO::Directory::GetFiles(Cache))
			System::IO::File::Delete(Path);
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't purge auto-recovery cache!\n\tException: " + E->Message, true);
	}

	delete ISDB;
}

componentDLLInterface::ScriptEditorInterface g_InteropInterface =
{
	InitializeComponents,
	InstantiateEditor,
	AddScriptCommandDeveloperURL,
	CloseAllOpenEditors,
	UpdateIntelliSenseDatabase,
	GetOpenEditorCount,
	PreprocessScript,
	Deinitalize,
};