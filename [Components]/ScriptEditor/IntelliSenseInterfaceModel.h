#pragma once

#include "IntelliSenseItem.h"
#include "IIntelliSenseInterface.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


ref struct IntelliSenseModelContext
{
	static enum class OperationType
	{
		Default,
		Call,
		Dot,
		Assign,
		Snippet
	};

	property bool						Valid;
	property int						CaretPos;
	property UInt32						Line;
	property bool						InsideCommentOrStringLiteral;
	property bool						LineVisible;

	property OperationType				Operation;

	property IntelliSenseItem^			CallingObject;
	property IntelliSenseItemScript^	CallingObjectScript;
	property bool						CallingObjectIsObjectReference;

	property String^					FilterString;
	property Point						DisplayScreenCoords;


	IntelliSenseModelContext()
	{
		Reset();
	}

	void Reset()
	{
		Valid = false;
		CaretPos = -1;
		Line = 0;
		LineVisible = true;

		Operation = OperationType::Default;

		CallingObject = nullptr;
		CallingObjectScript = nullptr;
		CallingObjectIsObjectReference = false;

		FilterString = String::Empty;
		DisplayScreenCoords = Point(0, 0);
	}
};

ref struct IntelliSenseModelContextUpdateDiff
{
	property bool		CaretPosChanged;
	property UInt32		OldCaretPos;
	property bool		LineChanged;
	property UInt32		OldLine;
	property bool		OperationInvoked;

	IntelliSenseModelContextUpdateDiff()
	{
		Reset();
	}

	void Reset()
	{
		CaretPosChanged = false;
		OldCaretPos = -1;
		LineChanged = false;
		OldLine = 0;
		OperationInvoked = false;
	}
};

ref class IntelliSenseInterfaceModel : public IIntelliSenseInterfaceModel
{
	static enum class PopupDisplayMode
	{
		Manual,
		Automatic
	};

	static enum class PopupShowReason
	{
		None,
		UserInvoked,
		OperationInvoked,
		PopupThresholdReached,
	};

	static enum class PopupHideReason
	{
		None,
		UserDismissed,
		SelectionComplete,
		ContextChanged,
	};

	static enum class PopupSuppressionMode
	{
		NoSuppression,
		UntilNextTriggerKeyPress,
	};

	static enum class ContextChangeEventHandlingMode
	{
		Handle,
		CacheAndIgnore,
	};


	textEditor::ITextEditor^		ParentEditor;
	IIntelliSenseInterfaceView^			BoundView;

	IntelliSenseModelContext^			Context;
	IntelliSenseModelContextUpdateDiff^	LastContextUpdateDiff;

	Dictionary<IntelliSenseContextChangeEventArgs::Event, IntelliSenseContextChangeEventArgs^>^
										CachedContextChangeEventArgs;


	List<IntelliSenseItemScriptVariable^>^
										LocalVariables;
	List<IntelliSenseItem^>^			EnumeratedItems;

	PopupDisplayMode					DisplayMode;
	PopupSuppressionMode				SuppressionMode;
	PopupShowReason						ShowReason;
	PopupHideReason						HideReason;
	ContextChangeEventHandlingMode		ContextChangeHandlingMode;

	property bool						AutomaticallyPopup;
	property UInt32						PopupThresholdLength;
	property bool						UseSubstringFiltering;
	property bool						InsertSuggestionOnEnterKey;

	property bool Bound
	{
		bool get() { return BoundView != nullptr; }
	}
	property bool Visible
	{
		bool get() { return BoundView->Visible; }
	}

	void									ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);
	void									ParentEditor_InputEventHandler(Object^ Sender, IntelliSenseInputEventArgs^ E);
	void									ParentEditor_InsightHoverEventHandler(Object^ Sender, IntelliSenseInsightHoverEventArgs^ E);
	void									ParentEditor_ContextChangeEventHandler(Object^ Sender, IntelliSenseContextChangeEventArgs^ E);
	void									BoundView_ItemSelected(Object^ Sender, EventArgs^ E);
	void									BoundView_Dismissed(Object^ Sender, EventArgs^ E);

	EventHandler^							ScriptEditorPreferencesSavedHandler;
	IntelliSenseInputEventHandler^			ParentEditorInputEventHandler;
	IntelliSenseInsightHoverEventHandler^	ParentEditorInsightHoverEventHandler;
	IntelliSenseContextChangeEventHandler^	ParentEditorContextChangeEventHandler;
	EventHandler^							BoundViewItemSelectedHandler;
	EventHandler^							BoundViewDismissedHandler;

	void						SetContextChangeEventHandlingMode(ContextChangeEventHandlingMode Mode);

	void						OnParentReset();
	void						OnTriggerKeyPress();
	void						OnCaretPosChanged(IntelliSenseContextChangeEventArgs^ E);
	void						OnUserInvoked();
	void						OnUserDismissed();
	void						OnSelectionCompleted();
	void						OnScrollOffsetChanged(IntelliSenseContextChangeEventArgs^ E);
	void						OnTextChanged(IntelliSenseContextChangeEventArgs^ E);


	void						UpdateContext(IntelliSenseContextChangeEventArgs^ Args);
	void						ResetContext();

	bool						HandleKeyboardInput(IntelliSenseInputEventArgs::Event Type, KeyEventArgs^ E);
	bool						IsTriggerKey(KeyEventArgs^ E);
	bool						HandleMouseInput(IntelliSenseInputEventArgs::Event Type, MouseEventArgs^ E);

	void						ShowInsightTooltip(IntelliSenseInsightHoverEventArgs^ E);
	void						HideInsightTooltip();

	void						ShowPopup(PopupShowReason Reason);
	bool						UpdatePopup();
	void						HidePopup(PopupHideReason Reason);
	void						RelocatePopup();

	void						PopulateDataStore();
	generic <typename T> where T : IntelliSenseItem
	void						EnumerateIntelliSenseItems(IEnumerable<T>^ Items, eStringMatchType MatchType);

	IntelliSenseItemScriptVariable^
								LookupLocalVariable(String^ Identifier);
	void						UpdateLocalVariables(obScriptParsing::AnalysisData^ Data);
public:
	IntelliSenseInterfaceModel(textEditor::ITextEditor^ Parent);
	~IntelliSenseInterfaceModel();

	virtual property List<IntelliSenseItem^>^ DataStore
	{
		virtual List<IntelliSenseItem^>^ get() { return EnumeratedItems; }
		virtual void set(List<IntelliSenseItem^>^) {}
	}

	virtual void	Bind(IIntelliSenseInterfaceView^ To);
	virtual void	Unbind();
	virtual bool	IsLocalVariable(String^ Identifier);
};


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse