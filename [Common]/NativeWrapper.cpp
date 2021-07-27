#include "MiscUtilities.h"
#include "NativeWrapper.h"

namespace cse
{


namespace nativeWrapper
{


componentDLLInterface::CSEInterfaceTable* g_CSEInterfaceTable = nullptr;

void Initialize()
{
	g_CSEInterfaceTable = (componentDLLInterface::CSEInterfaceTable*)nativeWrapper::QueryInterface();
	Debug::Assert(g_CSEInterfaceTable != nullptr);
}

void nativeWrapper::ShowNonActivatingWindow(Control^ Window, IntPtr ParentHandle)
{
	const int SW_SHOWNOACTIVATE = 4;
	const int HWND_TOPMOST = -1;
	const UInt32 SWP_NOACTIVATE = 0x0010;

	ShowWindow(Window->Handle, SW_SHOWNOACTIVATE);
	if (ParentHandle != IntPtr::Zero)
		SetWindowPos(Window->Handle, ParentHandle.ToInt32(), Window->Left, Window->Top, Window->Width, Window->Height, SWP_NOACTIVATE);
	else
		SetWindowPos(Window->Handle, 0, Window->Left, Window->Top, Window->Width, Window->Height, SWP_NOACTIVATE);
}

void nativeWrapper::WriteToMainWindowStatusBar(int PanelIndex, String^ Message)
{
	CString CStr(Message);
	g_CSEInterfaceTable->EditorAPI.WriteToStatusBar(PanelIndex, CStr.c_str());
}

void PrintToConsole(UInt8 Source, String^% Message)
{
	CString CStr(Message);
	nativeWrapper::g_CSEInterfaceTable->EditorAPI.DebugPrint(Source, CStr.c_str());
}

void SetControlRedraw(Control^ Window, bool Enabled)
{
	const int WM_SETREDRAW = 11;

	SendMessageA(Window->Handle, WM_SETREDRAW, static_cast<IntPtr>(Enabled), static_cast<IntPtr>(0));
}

System::Windows::Forms::Control^ GetControlWithFocus()
{
	Control^ FocusedControl = nullptr;
	IntPtr FocusedHandle = GetFocus();

	if (FocusedHandle != IntPtr::Zero)
		FocusedControl = Control::FromHandle(FocusedHandle);

#if 0
	DebugPrint("Control with focus : " + (FocusedControl ? (FocusedControl->ToString() + " | " + FocusedControl->GetType()->ToString()): "NONE"));
#endif

	return FocusedControl;
}

MarshalledFormData::MarshalledFormData(componentDLLInterface::FormData* NativeData)
{
	EditorId = gcnew String(NativeData->EditorID);
	FormId = NativeData->FormID;
	FormType = NativeData->TypeID;
	FormFlags = NativeData->Flags;
	SourcePluginName = gcnew String(NativeData->ParentPluginName);
	NameComponent = gcnew String(NativeData->NameComponent);
	DescriptionComponent = gcnew String(NativeData->DescriptionComponent);
	IsObjectRef = NativeData->ObjectReference;
	BaseFormEditorId = gcnew String(NativeData->BaseFormEditorID);
	AttachedScriptEditorId = gcnew String(NativeData->AttachedScriptEditorID);
}

MarshalledScriptData::MarshalledScriptData(componentDLLInterface::ScriptData* NativeData)
	: MarshalledFormData(NativeData)
{
	switch (NativeData->Type)
	{
	case componentDLLInterface::ScriptData::kScriptType_Object:
		ScriptType = eScriptType::Object;
		break;
	case componentDLLInterface::ScriptData::kScriptType_Quest:
		ScriptType = eScriptType::Quest;
		break;
	case componentDLLInterface::ScriptData::kScriptType_Magic:
		ScriptType = eScriptType::MagicEffect;
		break;
	}

	ScriptText = gcnew String(NativeData->Text);
	IsUdf = NativeData->UDF;
}

MarshalledVariableData::MarshalledVariableData(componentDLLInterface::VariableData* NativeData)
	: MarshalledFormData(NativeData)
{
	switch (NativeData->Type)
	{
	case componentDLLInterface::VariableData::kType_Int:
		VariableType = eVariableType::Integer;
		Integer = NativeData->Value.i;
		break;
	case componentDLLInterface::VariableData::kType_Float:
		VariableType = eVariableType::Float;
		Float = NativeData->Value.f;
		break;
	case componentDLLInterface::VariableData::kType_String:
		VariableType = eVariableType::String;
		String = gcnew System::String(NativeData->Value.s);
		break;
	}
}

System::String^ MarshalledVariableData::ValueAsString::get()
{
	switch (VariableType)
	{
	case eVariableType::Integer:
		return Integer.ToString();
	case eVariableType::Float:
		return Float.ToString();
	case eVariableType::String:
		return String;
	default:
		return String::Empty;
	}
}


} // namespace nativeWrapper


namespace log
{


namespace scriptEditor
{


void DebugPrint(String^ Message, bool Achtung)
{
	if (Achtung)
		Media::SystemSounds::Hand->Play();

	nativeWrapper::PrintToConsole(e_SE, Message);
}


} // namespace scriptEditor


namespace useInfoList
{


void DebugPrint(String^ Message, bool Achtung)
{
	if (Achtung)
		Media::SystemSounds::Hand->Play();

	nativeWrapper::PrintToConsole(e_UL, Message);
}


} // namespace useInfoList


namespace bsaViewer
{


void DebugPrint(String^ Message, bool Achtung)
{
	if (Achtung)
		Media::SystemSounds::Hand->Play();

	nativeWrapper::PrintToConsole(e_BSA, Message);
}


} // namespace bsaViewer


namespace tagBrowser
{


void DebugPrint(String^ Message, bool Achtung)
{
	if (Achtung)
		Media::SystemSounds::Hand->Play();

	nativeWrapper::PrintToConsole(e_TAG, Message);
}


} // namespace tagBrowser


} // namespace log


} // namespace cse