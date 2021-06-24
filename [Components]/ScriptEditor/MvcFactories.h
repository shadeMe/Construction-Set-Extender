#pragma once

#include "IScriptEditorModel.h"
#include "IScriptEditorView.h"
#include "IScriptEditorController.h"

namespace cse
{


namespace scriptEditor
{

ref struct ScriptEditorModelFactory : public model::IFactory
{
	ScriptEditorModelFactory();
public:
	virtual model::IScriptEditorModel^ NewModel();

	static ScriptEditorModelFactory^ NewFactory();
};


ref struct ScriptEditorViewFactory : public view::IFactory
{
	ScriptEditorViewFactory();
public:
	virtual view::IScriptEditorView^ NewView(Rectangle InitialBounds);

	static ScriptEditorViewFactory^ NewFactory();
};


ref struct ScriptEditorControllerFactory : public controller::IFactory
{
	ScriptEditorControllerFactory();
public:
	virtual controller::IScriptEditorController^ NewController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory);

	static ScriptEditorControllerFactory^ NewFactory();
};


} // namespace scriptEditor


} // namespace cse