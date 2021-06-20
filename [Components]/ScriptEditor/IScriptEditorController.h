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


namespace components
{

	// make the action primitive common across different domains (similar to how it's done in the render window)
	// text editor/model exports its own actions and presents them to teh controller
	// controller collates this and allows specific shortcuts to be bnound to them
interface class IBindableAction
{
	delegate void Action(Object^ UserParam);

};


interface class IScriptEditorActions
{

};


} // namespace components


interface class IScriptEditorController
{
	property model::IScriptEditorModel^ Model;
	property view::IScriptEditorView^ View;
};


} // namespace controller


} // namespace scriptEditor


} // namespace cse