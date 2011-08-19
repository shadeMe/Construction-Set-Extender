#include "MiscUtilities.h"
#include "NativeWrapper.h"

ComponentDLLInterface::CSEInterface*		g_CSEInterface = (ComponentDLLInterface::CSEInterface*)NativeWrapper::QueryInterface();

void NativeWrapper::ShowNonActivatingWindow(Control^ Window, IntPtr ParentHandle)
{
	ShowWindow(Window->Handle, SW_SHOWNOACTIVATE);
	if (ParentHandle != IntPtr::Zero)
		SetWindowPos(Window->Handle, ParentHandle.ToInt32(), Window->Left, Window->Top, Window->Width, Window->Height, SWP_NOACTIVATE);
	else
		SetWindowPos(Window->Handle, 0, Window->Left, Window->Top, Window->Width, Window->Height, SWP_NOACTIVATE);
}

void NativeWrapper::WriteToMainWindowStatusBar(int PanelIndex, String^ Message)
{
	CString CStr(Message);
	g_CSEInterface->CSEEditorAPI.WriteToStatusBar(PanelIndex, CStr.c_str());
}