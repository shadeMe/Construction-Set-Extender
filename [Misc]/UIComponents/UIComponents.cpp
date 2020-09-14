// UIComponents.cpp : main project file.

#include "stdafx.h"
#include "SESnippetManager.h"
#include "SEScriptList.h"
#include "SEVariableIndex.h"
#include "UseInfoList.h"
#include "SESyncUI.h"
#include "SEOptions.h"

using namespace UIComponents;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	// Create the main window and run it
	Application::Run(gcnew OptionsDialog());
	return 0;
}
