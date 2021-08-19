#include "IScriptEditorController.h"

namespace cse
{


namespace scriptEditor
{


namespace controller
{


IScriptEditorController::InstantiationParams::InstantiationParams()
{
	Operations = eInitOperation::None;
	ExistingScriptEditorIds = gcnew List<String^>;
	FindQuery = String::Empty;
}


} // namespace controller


} // namespace scriptEditor


} // namespace cse