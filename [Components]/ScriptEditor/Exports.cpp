#include "Exports.h"
#include "IntelliSenseBackend.h"
#include "ScriptSync.h"
#include "Preferences.h"
#include "ScriptPreprocessor.h"
#include "ScriptEditorInstanceManager.h"

#include <memory.h>

using namespace componentDLLInterface;
using namespace System::Reflection;
using namespace cse;
using namespace cse::scriptEditor;

extern componentDLLInterface::ScriptEditorInterface g_InteropInterface;

extern "C"
{
	QUERYINTERFACE_EXPORT
	{
		return &g_InteropInterface;
	}
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
	AppDomain^ CurrentDomain = AppDomain::CurrentDomain;
	CurrentDomain->UnhandledException += gcnew UnhandledExceptionEventHandler(&CLRUnhandledExceptionFilter);

	nativeWrapper::Initialize();
	preferences::SettingsHolder::Get()->LoadFromDisk();
	intellisense::IntelliSenseBackend::Get()->InitializeScriptCommands(ScriptCommandData);
	intellisense::IntelliSenseBackend::Get()->InitializeGameSettings(GMSTData);
}

void InstantiateEditor(componentDLLInterface::ScriptData* InitializerScript, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	auto Params = gcnew controller::IScriptEditorController::InstantiationParams;
	Params->Operations = controller::IScriptEditorController::InstantiationParams::eInitOperation::LoadExistingScript;
	Params->InitialBounds = Rectangle(Top, Left, Width, Height);
	Params->ExistingScriptEditorIds->Add(gcnew String(InitializerScript->EditorID));

	ScriptEditorInstanceManager::Get()->NewInstance()->InstantiateEditor(Params);
}

void InstantiateEditorAndHighlight(componentDLLInterface::ScriptData* InitializerScript, const char* SearchQuery, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	auto Params = gcnew controller::IScriptEditorController::InstantiationParams;
	Params->Operations = controller::IScriptEditorController::InstantiationParams::eInitOperation::LoadExistingScript
						 | controller::IScriptEditorController::InstantiationParams::eInitOperation::PerformFind;
	Params->InitialBounds = Rectangle(Top, Left, Width, Height);
	Params->ExistingScriptEditorIds->Add(gcnew String(InitializerScript->EditorID));
	Params->FindQuery = gcnew String(SearchQuery);

	ScriptEditorInstanceManager::Get()->NewInstance()->InstantiateEditor(Params);
}

void InstantiateEditors(componentDLLInterface::ScriptData** InitializerScripts, UInt32 ScriptCount, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	auto Params = gcnew controller::IScriptEditorController::InstantiationParams;
	Params->Operations = controller::IScriptEditorController::InstantiationParams::eInitOperation::LoadExistingScript;
	Params->InitialBounds = Rectangle(Top, Left, Width, Height);

	for (int i = 0; i < ScriptCount; i++)
	{
		auto NextScript = InitializerScripts[i];
		Params->ExistingScriptEditorIds->Add(gcnew String(NextScript->EditorID));
	}

	ScriptEditorInstanceManager::Get()->NewInstance()->InstantiateEditor(Params);
	nativeWrapper::g_CSEInterfaceTable->DeleteData(InitializerScripts, true);
}

void InstantiateEditorsAndHighlight(componentDLLInterface::ScriptData** InitializerScripts, UInt32 ScriptCount, const char* SearchQuery, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	auto Params = gcnew controller::IScriptEditorController::InstantiationParams;
	Params->Operations = controller::IScriptEditorController::InstantiationParams::eInitOperation::LoadExistingScript
						 | controller::IScriptEditorController::InstantiationParams::eInitOperation::PerformFind;
	Params->InitialBounds = Rectangle(Top, Left, Width, Height);
	Params->FindQuery = gcnew String(SearchQuery);

	for (int i = 0; i < ScriptCount; i++)
	{
		auto NextScript = InitializerScripts[i];
		Params->ExistingScriptEditorIds->Add(gcnew String(NextScript->EditorID));
	}

	ScriptEditorInstanceManager::Get()->NewInstance()->InstantiateEditor(Params);
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
	ScriptEditorInstanceManager::Get()->ReleaseAllActiveControllers();
	scriptSync::DiskSyncDialog::Close();
}

void UpdateIntelliSenseDatabase(void)
{
	intellisense::IntelliSenseBackend::Get()->Refresh(true);
}

UInt32 GetOpenEditorCount(void)
{
	return ScriptEditorInstanceManager::Get()->Count;
}

void DummyPreprocessorErrorOutputWrapper(int Line, String^ Message)
{
	;//
}

bool PreprocessScript(const char* ScriptText, char* OutPreprocessed, UInt32 BufferSize)
{
	String^ PreprocessedResult = "";
	bool OperationResult = preprocessor::Preprocessor::Get()->PreprocessScript(gcnew String(ScriptText),
								PreprocessedResult,
								gcnew preprocessor::StandardOutputError(&DummyPreprocessorErrorOutputWrapper),
								gcnew preprocessor::PreprocessorParams(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
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