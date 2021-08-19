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
	ref struct InstantiationParams
	{
		[Flags]
		static enum class eInitOperation
		{
			None = 0,

			CreateNewScript = 1 << 0,
			LoadExistingScript = 1 << 1,

			PerformFind = 1 << 2,
		};

		eInitOperation Operations;
		Rectangle InitialBounds;
		List<String^>^ ExistingScriptEditorIds;
		String^ FindQuery;

		InstantiationParams();
	};

	property model::IScriptEditorModel^ Model;
	property view::IScriptEditorView^ View;
	property model::IScriptDocument^ ActiveDocument;

	IScriptEditorController^ New();
	IScriptEditorController^ New(InstantiationParams^ Params);
	void RelocateDocument(model::IScriptDocument^ Document, IScriptEditorController^ Source);
	void ActivateOrCreateNewDocument(String^ ScriptEditorId);
};


interface class IFactory
{
	IScriptEditorController^ NewController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory);
	IScriptEditorController^ NewController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory, IScriptEditorController::InstantiationParams^ InitParams);

};


} // namespace controller


} // namespace scriptEditor


} // namespace cse