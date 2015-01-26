#include "ScriptEditorManager.h"
#include "Exports.h"
#include "IntelliSense\IntelliSenseDatabase.h"
#include "Globals.h"

#include <memory.h>

using namespace ComponentDLLInterface;
using namespace System::Reflection;
using namespace ConstructionSetExtender;
using namespace ConstructionSetExtender::IntelliSense;
using namespace ConstructionSetExtender::ScriptEditor;

extern ComponentDLLInterface::ScriptEditorInterface g_InteropInterface;

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
					"ConstruKction Set Extender",
					MessageBoxButtons::OK,
					MessageBoxIcon::Error);
}

void InitializeComponents(CommandTableData* Data, IntelliSenseUpdateData* GMSTData)
{
	AppDomain^ CurrentDomain = AppDomain::CurrentDomain;
	CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler(&ResolvePreprocessorAssemblyLoad);
	CurrentDomain->UnhandledException += gcnew UnhandledExceptionEventHandler(&CLRUnhandledExceptionFilter);

	ISDB->InitializeCommandTableDatabase(Data);
	ISDB->InitializeGMSTDatabase(GMSTData);
	ISDB->ForceUpdateDatabase();

	System::Windows::Media::RenderOptions::ProcessRenderMode = System::Windows::Interop::RenderMode::Default;
}

void InstantiateEditor(ComponentDLLInterface::ScriptData* InitializerScript, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
	Parameters->EditorHandleIndex = 0;
	Parameters->ParameterList->Add((UInt32)InitializerScript);
	Parameters->ParameterList->Add(Left);
	Parameters->ParameterList->Add(Top);
	Parameters->ParameterList->Add(Width);
	Parameters->ParameterList->Add(Height);

	SEMGR->PerformOperation(ScriptEditorManager::OperationType::AllocateWorkspaceContainer, Parameters);
}

void AddScriptCommandDeveloperURL(const char* ScriptCommandName, const char* URL)
{
	ISDB->RegisterDeveloperURL(gcnew String(ScriptCommandName), gcnew String(URL));
}

void CloseAllOpenEditors(void)
{
	ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
	SEMGR->PerformOperation(ScriptEditorManager::OperationType::CloseAllOpenEditors, Parameters);
}

void UpdateIntelliSenseDatabase(void)
{
	ISDB->ForceUpdateDatabase();
}

UInt32 GetOpenEditorCount(void)
{
	return SEMGR->GetOpenEditorCount();
}

void DummyPreprocessorErrorOutputWrapper(String^ Message)
{
	;//
}

bool PreprocessScript(const char* ScriptText, char* OutPreprocessed, UInt32 BufferSize)
{
	String^ PreprocessedResult = "";
	bool OperationResult = Preprocessor::GetSingleton()->PreprocessScript(gcnew String(ScriptText),
								PreprocessedResult,
								gcnew ScriptPreprocessor::StandardOutputError(&DummyPreprocessorErrorOutputWrapper),
								gcnew ScriptEditorPreprocessorData(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
								gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
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
		String^ Cache = gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath());

		for each (String^ Path in System::IO::Directory::GetFiles(Cache))
			System::IO::File::Delete(Path);
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't purge auto-recovery cache!\n\tException: " + E->Message, true);
	}

	delete ISDB;
}

ComponentDLLInterface::ScriptEditorInterface g_InteropInterface =
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