#include "[Common]\ComponentDLLInterface.h"
#include "ScriptEditorManager.h"
#include "Exports.h"
#include "IntelliSense\IntelliSenseDatabase.h"
#include "Globals.h"

using namespace ComponentDLLInterface;
using namespace System::Reflection;
using namespace ConstructionSetExtender;
using namespace ConstructionSetExtender::IntelliSense;
using namespace ConstructionSetExtender::ScriptEditor;

extern ComponentDLLInterface::ScriptEditorInterface g_InteropInterface;

extern "C"
{
	__declspec(dllexport) void* QueryInterface(void)
	{
		return &g_InteropInterface;
	}
}

Assembly^ ResolvePreprocessorAssemblyLoad(Object^ Sender, ResolveEventArgs^ E)
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

void InitializeComponents(CommandTableData* Data, IntelliSenseUpdateData* GMSTData)
{
	try
	{
		System::Threading::Thread::CurrentThread->SetApartmentState(System::Threading::ApartmentState::STA);
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't set thread apartment state to STA\n\tException: " + E->Message);
	}

	AppDomain^ CurrentDomain = AppDomain::CurrentDomain;
	CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler(&ResolvePreprocessorAssemblyLoad);

	ISDB->InitializeCommandTableDatabase(Data);
	ISDB->InitializeGMSTDatabase(GMSTData);
}

void InitializeIntelliSenseDatabaseUpdateThread(void)
{
	ISDB->InitializeDatabaseUpdateThread();
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

	SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_AllocateWorkspaceContainer, Parameters);
}

void AddScriptCommandDeveloperURL(const char* ScriptCommandName, const char* URL)
{
	ISDB->RegisterDeveloperURL(gcnew String(ScriptCommandName), gcnew String(URL));
}

void CloseAllOpenEditors(void)
{
	ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
	SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_CloseAllOpenEditors, Parameters);
}

void UpdateIntelliSenseDatabase(void)
{
	ISDB->ForceUpdateDatabase();
}

UInt32 GetOpenEditorCount(void)
{
	return SEMGR->GetOpenEditorCount();
}

ComponentDLLInterface::ScriptEditorInterface g_InteropInterface =
{
	DeleteManagedHeapPointer,
	InitializeComponents,
	InitializeIntelliSenseDatabaseUpdateThread,
	InstantiateEditor,
	AddScriptCommandDeveloperURL,
	CloseAllOpenEditors,
	UpdateIntelliSenseDatabase,
	GetOpenEditorCount,
};