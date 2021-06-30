#include "ScriptEditorInstanceManager.h"
#include "ScriptEditorModelImpl.h"
#include "ScriptEditorViewImpl.h"
#include "ScriptEditorControllerImpl.h"


namespace cse
{


namespace scriptEditor
{


ScriptEditorInstanceManager::ScriptEditorInstanceManager()
{
	ModelFactory = modelImpl::ScriptEditorModelFactory::NewFactory();
	ViewFactory = viewImpl::ScriptEditorViewFactory::NewFactory();
	ControllerFactory = controllerImpl::ScriptEditorControllerFactory::NewFactory();
	ActiveControllers = gcnew List<controller::IScriptEditorController^>;
}

ScriptEditorInstanceManager::~ScriptEditorInstanceManager()
{
	ActiveControllers->Clear();
}

controller::IScriptEditorController^ ScriptEditorInstanceManager::NewInstance()
{
	return ControllerFactory->NewController(ModelFactory, ViewFactory);
}

void ScriptEditorInstanceManager::RegisterController(controller::IScriptEditorController^ Controller)
{
	if (ActiveControllers->Contains(Controller))
		throw gcnew InvalidOperationException("Controller has already been registered");

	ActiveControllers->Add(Controller);
}

void ScriptEditorInstanceManager::DeregisterController(controller::IScriptEditorController^ Controller)
{
	if (!ActiveControllers->Contains(Controller))
		throw gcnew InvalidOperationException("Controller has not been registered");

	ActiveControllers->Remove(Controller);
}

void ScriptEditorInstanceManager::ReleaseAllActiveControllers()
{
	for each (auto Controller in ActiveControllers)
		delete Controller;

	ActiveControllers->Clear();
}

ScriptEditorInstanceManager^ ScriptEditorInstanceManager::Get()
{
	return Singleton;
}


} // namespace scriptEditor


} // namespace cse

