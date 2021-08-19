#pragma once

#include "IScriptEditorController.h"

namespace cse
{


namespace scriptEditor
{


ref class ScriptEditorInstanceManager
{
	static ScriptEditorInstanceManager^ Singleton = gcnew ScriptEditorInstanceManager;

	model::IFactory^ ModelFactory;
	view::IFactory^ ViewFactory;
	controller::IFactory^ ControllerFactory;
	List<controller::IScriptEditorController^>^ ActiveControllers;

	ScriptEditorInstanceManager();
public:
	~ScriptEditorInstanceManager();

	void NewInstance(controller::IScriptEditorController::InstantiationParams^ InitParams);

	void RegisterController(controller::IScriptEditorController^ Controller);
	void DeregisterController(controller::IScriptEditorController^ Controller);
	void ReleaseAllActiveControllers();

	ImplPropertyGetOnly(UInt32, Count, ActiveControllers->Count);

	static ScriptEditorInstanceManager^ Get();
};


} // namespace scriptEditor


} // namespace cse