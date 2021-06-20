#pragma once

#include "Macros.h"
#include "IScriptEditorController.h"
#include "ScriptEditorControllerImplComponents.h"

namespace cse
{


namespace scriptEditor
{


namespace controller
{


ref class ScriptEditorController : public IScriptEditorController
{
	void BindScriptDocument(model::IScriptDocument^ Document, view::IScriptEditorView^ View);
	void UnbindScriptDocument(model::IScriptDocument^ Document, view::IScriptEditorView^ View);

	bool HandleUnsavedChangesBeforeDestructiveOperation(model::IScriptDocument^ Document);
	bool HandleUnsavedChangesBeforeDestructiveOperation(model::IScriptDocument^ Document, bool% OutOperationCancelled);

	void NewScript(model::IScriptDocument^ Document);
	void OpenScript(model::IScriptDocument^ Document, componentDLLInterface::ScriptData* Data);
	bool SaveScript(model::IScriptDocument^ Document);
	bool CloseScript(model::IScriptDocument^ Document, bool% OutOperationCancelled);
	void LoadNextScript(model::IScriptDocument^ Document);
	void LoadPreviousScript(model::IScriptDocument^ Document);

	bool HandleInvalidViewEvent(view::ViewComponentEvent^ E);

	void ViewEventHandler_MainWindow(view::ViewComponentEvent^ E);
	void ViewEventHandler_MainTabStrip(view::ViewComponentEvent^ E);
	void ViewEventHandler_MainToolbar(view::ViewComponentEvent^ E);
	void ViewEventHandler_MainToolbarMenuEdit(view::ViewComponentEvent^ E);
	void ViewEventHandler_MainToolbarMenuView(view::ViewComponentEvent^ E);
	void ViewEventHandler_MainToolbarMenuTools(view::ViewComponentEvent^ E);
	void ViewEventHandler_MainToolbarMenuHelp(view::ViewComponentEvent^ E);
	void ViewEventHandler_MessagesPanel(view::ViewComponentEvent^ E);
	void ViewEventHandler_BookmarksPanel(view::ViewComponentEvent^ E);
	void ViewEventHandler_OutlineViewPanel(view::ViewComponentEvent^ E);
	void ViewEventHandler_TextEditorContextMenu(view::ViewComponentEvent^ E);

	void ModelEventHandler_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E);
	void ModelEventHandler_ActiveDocumentChanged(Object^ Sender, model::IScriptEditorModel::ActiveDocumentChangedEventArgs^ E);
	void ModelEventHandler_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void ModelEventHandler_MouseClick(Object^ Sender, textEditor::TextEditorMouseClickEventArgs^ E);

	void InputHandling_KeyDown(KeyEventArgs^ E);
	void InputHandling_MouseClick(MouseEventArgs^ E);
};


} // namespace controller


} // namespace scriptEditor


} // namespace cse