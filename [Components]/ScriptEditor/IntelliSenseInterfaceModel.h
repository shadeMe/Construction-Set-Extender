#pragma once

#include "Macros.h"
#include "IntelliSenseItem.h"
#include "IIntelliSenseInterface.h"
#include "IntelliSenseBackend.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


ref struct IntelliSenseModelContext
{
	static enum class eOperationType
	{
		Default,
		Call,
		Dot,
		Assign,
		Snippet
	};

	property bool Valid;
	property int CaretPos;
	property UInt32 Line;
	property bool InsideCommentOrStringLiteral;
	property bool LineVisible;

	property eOperationType Operation;

	property IntelliSenseItem^ CallingObject;
	property IntelliSenseItemScript^ CallingObjectScript;
	property IntelliSenseItemCollection^ CallingObjectScriptVariables;
	property bool CallingObjectIsObjectReference;

	property String^ CaretToken;
	property Point DisplayScreenCoords;

	IntelliSenseModelContext();

	void Reset();
};

ref struct IntelliSenseModelContextUpdateDiff
{
	property bool CaretPosChanged;
	property UInt32 OldCaretPos;
	property bool LineChanged;
	property UInt32 OldLine;
	property IntelliSenseModelContext::eOperationType OldOperation;
	property bool OperationInvoked;
	property bool CallingObjectScriptChanged;
	property IntelliSenseItemScript^ OldCallingObjectScript;

	IntelliSenseModelContextUpdateDiff();

	void Reset();
};

ref class IntelliSenseInterfaceModel : public IIntelliSenseInterfaceModel
{
	static property int MaximumNumberOfSuggestions
	{
		int get() { return 100; }
	}

	static enum class ePopupDisplayMode
	{
		Manual,
		Automatic
	};

	static enum class ePopupShowReason
	{
		None,
		UserInvoked,
		OperationInvoked,
		PopupThresholdReached,
	};

	static enum class ePopupHideReason
	{
		None,
		UserDismissed,
		SelectionComplete,
		ContextChanged,
	};

	static enum class ePopupSuppressionMode
	{
		NoSuppression,
		UntilNextTriggerKeyPress,
	};

	static enum class eContextChangeEventHandlingMode
	{
		Handle,
		CacheAndIgnore,
	};


	textEditor::ITextEditor^ ParentEditor;
	IIntelliSenseInterfaceView^ BoundView;

	IntelliSenseModelContext^ Context;
	IntelliSenseModelContextUpdateDiff^ LastContextUpdateDiff;

	Dictionary<IntelliSenseContextChangeEventArgs::eEvent, IntelliSenseContextChangeEventArgs^>^ CachedContextChangeEventArgs;

	IntelliSenseItemCollection^ LocalVariables;
	List<IntelliSenseItem^>^ EnumeratedItems;

	ePopupDisplayMode DisplayMode;
	ePopupSuppressionMode SuppressionMode;
	ePopupShowReason ShowReason;
	ePopupHideReason HideReason;
	eContextChangeEventHandlingMode ContextChangeHandlingMode;

	property bool AutomaticallyPopup;
	property UInt32 PopupThresholdLength;
	property bool InsertSuggestionOnEnterKey;

	property bool Bound
	{
		bool get() { return BoundView != nullptr; }
	}
	property bool Visible
	{
		bool get() { return BoundView->Visible; }
	}

	void ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);
	void ParentEditor_InputEventHandler(Object^ Sender, IntelliSenseInputEventArgs^ E);
	void ParentEditor_InsightHoverEventHandler(Object^ Sender, IntelliSenseInsightHoverEventArgs^ E);
	void ParentEditor_ContextChangeEventHandler(Object^ Sender, IntelliSenseContextChangeEventArgs^ E);
	void BoundView_ItemSelected(Object^ Sender, EventArgs^ E);
	void BoundView_Dismissed(Object^ Sender, EventArgs^ E);
	void BoundView_FuzzySearchToggled(Object^ Sender, EventArgs^ E);

	EventHandler^ ScriptEditorPreferencesSavedHandler;
	IntelliSenseInputEventHandler^ ParentEditorInputEventHandler;
	IntelliSenseInsightHoverEventHandler^ ParentEditorInsightHoverEventHandler;
	IntelliSenseContextChangeEventHandler^ ParentEditorContextChangeEventHandler;
	EventHandler^ BoundViewItemSelectedHandler;
	EventHandler^ BoundViewDismissedHandler;
	EventHandler^ BoundViewFuzzySearchToggledHandler;

	void SetContextChangeEventHandlingMode(eContextChangeEventHandlingMode Mode);

	void OnParentReset();
	void OnTriggerKeyPress();
	void OnCaretPosChanged(IntelliSenseContextChangeEventArgs^ E);
	void OnUserInvoked();
	void OnUserDismissed();
	void OnSelectionCompleted();
	void OnScrollOffsetChanged(IntelliSenseContextChangeEventArgs^ E);
	void OnTextChanged(IntelliSenseContextChangeEventArgs^ E);
	void OnFuzzySearchToggled();


	void UpdateContext(IntelliSenseContextChangeEventArgs^ Args);
	void ResetContext();

	bool HandleKeyboardInput(IntelliSenseInputEventArgs::eEvent Type, KeyEventArgs^ E);
	bool IsTriggerKey(KeyEventArgs^ E);
	bool HandleMouseInput(IntelliSenseInputEventArgs::eEvent Type, MouseEventArgs^ E);

	void ShowInsightTooltip(IntelliSenseInsightHoverEventArgs^ E);
	void HideInsightTooltip();

	void ShowPopup(ePopupShowReason Reason);
	bool UpdatePopup();
	void HidePopup(ePopupHideReason Reason);
	void RelocatePopup();

	void PopulateDataStore();

	IntelliSenseItemScriptVariable^ LookupLocalVariable(String^ Identifier);
	void UpdateLocalVariables(obScriptParsing::AnalysisData^ Data);
	void EnumerateVariables(IntelliSenseItemCollection^ ScriptVariables, String^ MatchIdentifier, eFilterMode MatchType, UInt32 FuzzyMatchingMaxCost);
public:
	IntelliSenseInterfaceModel(textEditor::ITextEditor^ ParentEditor);
	~IntelliSenseInterfaceModel();

	ImplPropertyGetOnly(List<IntelliSenseItem^>^, DataStore, EnumeratedItems);

	virtual void Bind(IIntelliSenseInterfaceView^ To);
	virtual void Unbind();
	virtual bool IsLocalVariable(String^ Identifier);
};


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse