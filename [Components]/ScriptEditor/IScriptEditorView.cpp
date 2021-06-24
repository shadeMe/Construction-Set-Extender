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
	Warning = IconResources->CreateImage("Warning");
	Error = IconResources->CreateImage("Error");
	Invalid = IconResources->CreateImage("Invalid");
	Success = IconResources->CreateImage("Success");
}


CommonIcons^ CommonIcons::Get()
{
	return Singleton;
}

} // namespace components


} // namespace view


} // namespace scriptEditor


} // namespace cse