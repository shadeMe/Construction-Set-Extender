#pragma once
#include "ITextEditor.h"
#include "IScriptEditorModel.h"


namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


ref struct Factory
{
	static enum class eTextEditor
	{
		AvalonEdit
	};

	static ITextEditor^ Create(eTextEditor Type, model::IScriptDocument^ ParentScriptDocument);
};


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse