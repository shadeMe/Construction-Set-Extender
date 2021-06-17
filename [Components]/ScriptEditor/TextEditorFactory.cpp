#include "TextEditorFactory.h"
#include "AvalonEditTextEditor.h"


namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


ITextEditor^ Factory::Create(eTextEditor Type, model::IScriptDocument^ ParentScriptDocument)
{
	switch (Type)
	{
	case eTextEditor::AvalonEdit:
		//return gcnew avalonEdit::AvalonEditTextEditor(ParentScriptDocument);
		throw gcnew NotImplementedException;
	default:
		throw gcnew NotImplementedException;
	}

	return nullptr;
}


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse

