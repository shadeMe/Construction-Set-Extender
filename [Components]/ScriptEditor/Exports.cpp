#include "Exports.h"
#include "IntelliSenseDatabase.h"
#include "Globals.h"
#include "WorkspaceModel.h"
#include "WorkspaceView.h"
#include "ScriptSync.h"
#include "Preferences.h"

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
	String^	AppPath = gcnew String(nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetAppPath());

	ExecutingAssemblies = Assembly::GetExecutingAssembly();

	for each(AssemblyName^ AssmbName in ExecutingAssemblies->GetReferencedAssemblies())
	{
		if (AssmbName->FullName->Substring(0, AssmbName->FullName->IndexOf(",")) == E->Name->Substring(0, E->Name->IndexOf(",")))
		{
			TempPath = AppPath + COMPONENTDLLFOLDER + E->Name->Substring(0, E->Name->IndexOf(",")) + ".dll";
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

void InitializeComponents(CommandTableData* ScriptCommandData, IntelliSenseUpdateData* GMSTData)
{
	if (Threading::SynchronizationContext::Current == nullptr)
		Threading::SynchronizationContext::SetSynchronizationContext(gcnew Threading::SynchronizationContext());

	Globals::MainThreadID = Threading::Thread::CurrentThread->ManagedThreadId;
	Globals::MainThreadTaskScheduler = Threading::Tasks::TaskScheduler::FromCurrentSynchronizationContext();

	nativeWrapper::Initialize();

	AppDomain^ CurrentDomain = AppDomain::CurrentDomain;
	CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler(&ResolvePreprocessorAssemblyLoad);
	CurrentDomain->UnhandledException += gcnew UnhandledExceptionEventHandler(&CLRUnhandledExceptionFilter);
	System::Windows::Media::RenderOptions::ProcessRenderMode = System::Windows::Interop::RenderMode::Default;

	preferences::SettingsHolder::Get()->LoadFromDisk();

	intellisense::IntelliSenseBackend::Get()->InitializeScriptCommands(ScriptCommandData);
	intellisense::IntelliSenseBackend::Get()->InitializeGameSettings(GMSTData);
}

void InstantiateEditor(componentDLLInterface::ScriptData* InitializerScript, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)ConcreteWorkspaceViewFactory::Instance->CreateView(Left, Top, Width, Height);
	IWorkspaceModel^ Model = ConcreteWorkspaceModelFactory::Instance->CreateModel(InitializerScript);

	New->AssociateModel(Model, true);
}

void InstantiateEditorAndHighlight(componentDLLInterface::ScriptData* InitializerScript, const char* SearchQuery, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)ConcreteWorkspaceViewFactory::Instance->CreateView(Left, Top, Width, Height);
	IWorkspaceModel^ Model = ConcreteWorkspaceModelFactory::Instance->CreateModel(InitializerScript);

	New->AssociateModel(Model, true);
	New->Controller->FindReplace(New,
								 cse::textEditors::ITextEditor::eFindReplaceOperation::Find,
								 gcnew String(SearchQuery), "",
								 cse::textEditors::ITextEditor::FindReplaceOptions::CaseInsensitive,
								 false);
}

void InstantiateEditors(componentDLLInterface::ScriptData** InitializerScripts, UInt32 ScriptCount, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)ConcreteWorkspaceViewFactory::Instance->CreateView(Left, Top, Width, Height);
	for (int i = 0; i < ScriptCount; i++)
	{
		IWorkspaceModel^ Model = ConcreteWorkspaceModelFactory::Instance->CreateModel(InitializerScripts[i]);
		New->AssociateModel(Model, i == 0);
	}

	nativeWrapper::g_CSEInterfaceTable->DeleteData(InitializerScripts, true);
}

void InstantiateEditorsAndHighlight(componentDLLInterface::ScriptData** InitializerScripts, UInt32 ScriptCount, const char* SearchQuery, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)ConcreteWorkspaceViewFactory::Instance->CreateView(Left, Top, Width, Height);
	for (int i = 0; i < ScriptCount; i++)
	{
		IWorkspaceModel^ Model = ConcreteWorkspaceModelFactory::Instance->CreateModel(InitializerScripts[i]);
		New->AssociateModel(Model, i == 0);
	}

	New->Controller->FindReplace(New,
								 cse::textEditors::ITextEditor::eFindReplaceOperation::Find,
								 gcnew String(SearchQuery), "",
								 cse::textEditors::ITextEditor::FindReplaceOptions::CaseInsensitive,
								 true);

	nativeWrapper::g_CSEInterfaceTable->DeleteData(InitializerScripts, true);
}

bool IsDiskSyncInProgress(void)
{
	return scriptSync::DiskSync::Get()->InProgress;
}

void ShowDiskSyncDialog(void)
{
	scriptSync::DiskSyncDialog::Show();
}

void CloseAllOpenEditors(void)
{
	ConcreteWorkspaceViewFactory::Instance->Clear();
	scriptSync::DiskSyncDialog::Close();
}

void UpdateIntelliSenseDatabase(void)
{
	intellisense::IntelliSenseBackend::Get()->Refresh(true);
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
								preferences::SettingsHolder::Get()->Preprocessor->AllowMacroRedefs,
								preferences::SettingsHolder::Get()->Preprocessor->NumPasses));

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

	preferences::SettingsHolder::Get()->SaveToDisk();
	intellisense::IntelliSenseBackend::Deinitialize();
}

componentDLLInterface::ScriptEditorInterface g_InteropInterface =
{
	InitializeComponents,
	InstantiateEditor,
	InstantiateEditorAndHighlight,
	InstantiateEditors,
	InstantiateEditorsAndHighlight,
	IsDiskSyncInProgress,
	ShowDiskSyncDialog,
	CloseAllOpenEditors,
	UpdateIntelliSenseDatabase,
	GetOpenEditorCount,
	PreprocessScript,
	Deinitalize,
};