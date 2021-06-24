#include "MvcFactories.h"
#include "ScriptEditorModelImpl.h"
#include "ScriptEditorViewImpl.h"
#include "ScriptEditorControllerImpl.h"


namespace cse
{


namespace scriptEditor
{


model::IScriptEditorModel^ ScriptEditorModelFactory::NewModel()
{
	return gcnew modelImpl::ScriptEditorDocumentModel;
}

ScriptEditorModelFactory^ ScriptEditorModelFactory::NewFactory()
{
	return gcnew ScriptEditorModelFactory;
}

view::IScriptEditorView^ ScriptEditorViewFactory::NewView(Rectangle InitialBounds)
{
	return gcnew viewImpl::ScriptEditorWorkspace(InitialBounds);
}

ScriptEditorViewFactory^ ScriptEditorViewFactory::NewFactory()
{
	return gcnew ScriptEditorViewFactory;
}

controller::IScriptEditorController^ ScriptEditorControllerFactory::NewController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory)
{
	return gcnew controllerImpl::ScriptEditorController(ModelFactory, ViewFactory);
}

ScriptEditorControllerFactory^ ScriptEditorControllerFactory::NewFactory()
{
	return gcnew ScriptEditorControllerFactory;
}


} // namespace scriptEditor


} // namespace cse

