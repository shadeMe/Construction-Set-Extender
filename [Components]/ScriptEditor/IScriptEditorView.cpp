#include "IScriptEditorView.h"

namespace cse
{


namespace scriptEditor
{


namespace view
{


namespace components
{


IScriptSelectionDialog::Params::Params()
{
	this->SelectedScriptEditorID = "";
	this->FilterString = "";
	this->ShowDeletedScripts = true;
	this->PreventSyncedScriptSelection = false;
}

IScriptSelectionDialog::Result::Result()
{
	this->SelectedScriptEditorIDs = gcnew List<String^>;
}

CommonIcons::CommonIcons()
{
	IconResources = gcnew ImageResourceManager("ScriptEditor.Icons");

	Transparent = IconResources->CreateImage("Transparent");
	UnsavedChanges = IconResources->CreateImage("Dirty");
	Info = IconResources->CreateImage("Info");
	InfoLarge = IconResources->CreateImage("Info-Large");
	Warning = IconResources->CreateImage("Warning");
	WarningLarge = IconResources->CreateImage("Warning-Large");
	Error = IconResources->CreateImage("Error");
	ErrorLarge = IconResources->CreateImage("Error-Large");
	BlockedLarge = IconResources->CreateImage("Blocked-Large");
	Invalid = IconResources->CreateImage("Invalid");
	Success = IconResources->CreateImage("Success");
	SuccessLarge = IconResources->CreateImage("Success-Large");
	InProgress = IconResources->CreateImage("InProgress");
}


CommonIcons^ CommonIcons::Get()
{
	return Singleton;
}

} // namespace components


} // namespace view


} // namespace scriptEditor


} // namespace cse