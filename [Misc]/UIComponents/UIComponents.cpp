// UIComponents.cpp : main project file.

#include "stdafx.h"
#include "UseInfoList.h"
#include "ScriptEditorWorkspace.h"

using namespace UIComponents;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	// Create the main window and run it
	Application::Run(gcnew ScriptEditorWorkspace());
	return 0;
}
