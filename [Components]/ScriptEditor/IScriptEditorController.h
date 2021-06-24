#pragma once

#include "Macros.h"
#include "IScriptEditorModel.h"
#include "IScriptEditorView.h"

namespace cse
{


namespace scriptEditor
{


namespace controller
{


interface class IScriptEditorController
{
	property model::IScriptEditorModel^ Model;
	property view::IScriptEditorView^ View;
	property model::IScriptDocument^ ActiveDocument;

	IScriptEditorController^ New();
	IScriptEditorController^ New(Rectangle ViewInitialBounds);
	void RelocateDocument(model::IScriptDocument^ Document, IScriptEditorController^ Source);
	void ActivateOrCreateNewDocument(String^ ScriptEditorId);
};


interface class IFactory
{
	IScriptEditorController^ NewController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory);
};


} // namespace controller


} // namespace scriptEditor


} // namespace cse