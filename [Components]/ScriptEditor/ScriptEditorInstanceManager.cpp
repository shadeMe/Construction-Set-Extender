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

void ScriptEditorInstanceManager::NewInstance(controller::IScriptEditorController::InstantiationParams^ InitParams)
{
	auto NewController = ControllerFactory->NewController(ModelFactory, ViewFactory, InitParams);
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
	// we need to use a buffer as controllers will remove themselves from the collection upon disposal
	auto Buffer = gcnew List<controller::IScriptEditorController^>(ActiveControllers);

	for each (auto Controller in Buffer)
		delete Controller;

	Debug::Assert(ActiveControllers->Count == 0);
}

ScriptEditorInstanceManager^ ScriptEditorInstanceManager::Get()
{
	return Singleton;
}


} // namespace scriptEditor


} // namespace cse

