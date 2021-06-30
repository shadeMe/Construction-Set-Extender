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
			LoadExistingScript = 1 << 0,	// sets the same bit as above to ensure mutually-exclusivity

			PerformFind = 1 << 1,
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
	void RelocateDocument(model::IScriptDocument^ Document, IScriptEditorController^ Source);
	void ActivateOrCreateNewDocument(String^ ScriptEditorId);

	void InstantiateEditor(InstantiationParams^ Params);
};


interface class IFactory
{
	IScriptEditorController^ NewController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory);
};


} // namespace controller


} // namespace scriptEditor


} // namespace cse