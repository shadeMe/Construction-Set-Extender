#include "IntelliSenseInterfaceModel.h"
#include "IntelliSenseBackend.h"
#include "Preferences.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


IntelliSenseModelContext::IntelliSenseModelContext()
{
	CallingObjectScriptVariables = gcnew IntelliSenseItemCollection;
	Reset();
}

void IntelliSenseModelContext::Reset()
{
	Valid = false;
	CaretPos = -1;
	Line = 0;
	LineVisible = true;

	Operation = eOperationType::Default;

	CallingObject = nullptr;
	CallingObjectScript = nullptr;
	CallingObjectIsObjectReference = false;
	// CallingObjectScriptVariables is reset elsewhere

	FilterString = String::Empty;
	DisplayScreenCoords = Point(0, 0);
}

IntelliSenseModelContextUpdateDiff::IntelliSenseModelContextUpdateDiff()
{
	Reset();
}

void IntelliSenseModelContextUpdateDiff::Reset()
{
	CaretPosChanged = false;
	OldCaretPos = -1;
	LineChanged = false;
	OldLine = 0;
	OperationInvoked = false;
	OldCallingObjectScript = nullptr;
	CallingObjectScriptChanged = false;
}

IntelliSenseInterfaceModel::IntelliSenseInterfaceModel(textEditor::ITextEditor^ ParentEditor)
{
	Debug::Assert(ParentEditor != nullptr);

	this->ParentEditor = ParentEditor;
	this->BoundView = nullptr;

	Context = gcnew IntelliSenseModelContext;
	LastContextUpdateDiff = gcnew IntelliSenseModelContextUpdateDiff;
	CachedContextChangeEventArgs = gcnew Dictionary<IntelliSenseContextChangeEventArgs::eEvent, IntelliSenseContextChangeEventArgs ^>;
	EnumeratedItems = gcnew List<IntelliSenseItem^>;
	LocalVariables = gcnew IntelliSenseItemCollection;

	AutomaticallyPopup = preferences::SettingsHolder::Get()->IntelliSense->ShowSuggestions;
	PopupThresholdLength = preferences::SettingsHolder::Get()->IntelliSense->SuggestionCharThreshold;
	InsertSuggestionOnEnterKey = preferences::SettingsHolder::Get()->IntelliSense->InsertSuggestionsWithEnterKey;

	DisplayMode = AutomaticallyPopup ? ePopupDisplayMode::Automatic : ePopupDisplayMode::Manual;
	SuppressionMode = ePopupSuppressionMode::NoSuppression;
	ShowReason = ePopupShowReason::None;
	HideReason = ePopupHideReason::None;
	ContextChangeHandlingMode = eContextChangeEventHandlingMode::Handle;

	ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceModel::ScriptEditorPreferences_Saved);
	ParentEditorInputEventHandler = gcnew IntelliSenseInputEventHandler(this, &IntelliSenseInterfaceModel::ParentEditor_InputEventHandler);
	ParentEditorInsightHoverEventHandler = gcnew IntelliSenseInsightHoverEventHandler(this, &IntelliSenseInterfaceModel::ParentEditor_InsightHoverEventHandler);
	ParentEditorContextChangeEventHandler = gcnew IntelliSenseContextChangeEventHandler(this, &IntelliSenseInterfaceModel::ParentEditor_ContextChangeEventHandler);
	BoundViewItemSelectedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceModel::BoundView_ItemSelected);
	BoundViewDismissedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceModel::BoundView_Dismissed);

	preferences::SettingsHolder::Get()->PreferencesChanged += ScriptEditorPreferencesSavedHandler;
	ParentEditor->IntelliSenseInput += ParentEditorInputEventHandler;
	ParentEditor->IntelliSenseContextChange += ParentEditorContextChangeEventHandler;
	ParentEditor->IntelliSenseInsightHover += ParentEditorInsightHoverEventHandler;
}

IntelliSenseInterfaceModel::~IntelliSenseInterfaceModel()
{
	Unbind();
	ResetContext();

	preferences::SettingsHolder::Get()->PreferencesChanged -= ScriptEditorPreferencesSavedHandler;
	ParentEditor->IntelliSenseInput -= ParentEditorInputEventHandler;
	ParentEditor->IntelliSenseContextChange -= ParentEditorContextChangeEventHandler;
	ParentEditor->IntelliSenseInsightHover -= ParentEditorInsightHoverEventHandler;

	SAFEDELETE_CLR(ScriptEditorPreferencesSavedHandler);
	SAFEDELETE_CLR(ParentEditorInputEventHandler);
	SAFEDELETE_CLR(ParentEditorContextChangeEventHandler);
	SAFEDELETE_CLR(ParentEditorInsightHoverEventHandler);
	SAFEDELETE_CLR(BoundViewItemSelectedHandler);
	SAFEDELETE_CLR(BoundViewDismissedHandler);

	ParentEditor = nullptr;
}

void IntelliSenseInterfaceModel::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
{
	if (Bound)
	{
		HidePopup(ePopupHideReason::None);
		HideInsightTooltip();
		BoundView->Hide();
	}

	AutomaticallyPopup = preferences::SettingsHolder::Get()->IntelliSense->ShowSuggestions;
	PopupThresholdLength = preferences::SettingsHolder::Get()->IntelliSense->SuggestionCharThreshold;
	InsertSuggestionOnEnterKey = preferences::SettingsHolder::Get()->IntelliSense->InsertSuggestionsWithEnterKey;

	DisplayMode = AutomaticallyPopup ? ePopupDisplayMode::Automatic : ePopupDisplayMode::Manual;
	SuppressionMode = ePopupSuppressionMode::NoSuppression;
	ShowReason = ePopupShowReason::None;
	HideReason = ePopupHideReason::None;
}


void IntelliSenseInterfaceModel::ParentEditor_InputEventHandler(Object^ Sender, IntelliSenseInputEventArgs^ E)
{
	Debug::Assert(Bound == true);

	switch (E->Type)
	{
	case IntelliSenseInputEventArgs::eEvent::KeyDown:
	case IntelliSenseInputEventArgs::eEvent::KeyUp:
		E->Handled = HandleKeyboardInput(E->Type, E->KeyEvent);
		break;
	case IntelliSenseInputEventArgs::eEvent::MouseDown:
	case IntelliSenseInputEventArgs::eEvent::MouseUp:
		E->Handled = HandleMouseInput(E->Type, E->MouseEvent);
		break;
	}
}

void IntelliSenseInterfaceModel::ParentEditor_InsightHoverEventHandler(Object^ Sender, IntelliSenseInsightHoverEventArgs^ E)
{
	Debug::Assert(Bound == true);

	if (preferences::SettingsHolder::Get()->IntelliSense->ShowInsightToolTip == false)
		return;

	if (E->Type == IntelliSenseInsightHoverEventArgs::eEvent::HoverStart)
		ShowInsightTooltip(E);
	else
		HideInsightTooltip();
}

void IntelliSenseInterfaceModel::ParentEditor_ContextChangeEventHandler(Object^ Sender, IntelliSenseContextChangeEventArgs^ E)
{
	if (!Bound)
		return;

	if (ContextChangeHandlingMode == eContextChangeEventHandlingMode::CacheAndIgnore)
	{
		// Cache the last received argument of each type
		CachedContextChangeEventArgs[E->Type] = E;
		return;
	}

	switch (E->Type)
	{
	case IntelliSenseContextChangeEventArgs::eEvent::Reset:
		OnParentReset();
		break;
	case IntelliSenseContextChangeEventArgs::eEvent::TextChanged:
		OnTextChanged(E);
		break;
	case IntelliSenseContextChangeEventArgs::eEvent::CaretPosChanged:
		OnCaretPosChanged(E);
		break;
	case IntelliSenseContextChangeEventArgs::eEvent::ScrollOffsetChanged:
		OnScrollOffsetChanged(E);
		break;
	case IntelliSenseContextChangeEventArgs::eEvent::SemanticAnalysisCompleted:
		UpdateLocalVariables(E->SemanticAnalysisData);
		break;
	}
}

void IntelliSenseInterfaceModel::BoundView_ItemSelected(Object^ Sender, EventArgs^ E)
{
	OnSelectionCompleted();
}


void IntelliSenseInterfaceModel::BoundView_Dismissed(Object^ Sender, EventArgs^ E)
{
	OnUserDismissed();
}

void IntelliSenseInterfaceModel::SetContextChangeEventHandlingMode(eContextChangeEventHandlingMode Mode)
{
	if (ContextChangeHandlingMode == Mode)
		return;

	switch (Mode)
	{
	case eContextChangeEventHandlingMode::Handle:
		ContextChangeHandlingMode = Mode;
		break;
	case eContextChangeEventHandlingMode::CacheAndIgnore:
		CachedContextChangeEventArgs->Clear();
		ContextChangeHandlingMode = Mode;
		break;
	}
}

void IntelliSenseInterfaceModel::OnParentReset()
{
	HidePopup(ePopupHideReason::ContextChanged);
	HideInsightTooltip();
	ResetContext();
	EnumeratedItems->Clear();
}

void IntelliSenseInterfaceModel::OnTriggerKeyPress()
{
	if (Visible)
		return;

	if (DisplayMode == ePopupDisplayMode::Manual)
		return;

	if (SuppressionMode == ePopupSuppressionMode::UntilNextTriggerKeyPress)
		SuppressionMode = ePopupSuppressionMode::NoSuppression;
}

void IntelliSenseInterfaceModel::OnCaretPosChanged(IntelliSenseContextChangeEventArgs^ E)
{
	UpdateContext(E);

	if (!LastContextUpdateDiff->CaretPosChanged && !LastContextUpdateDiff->LineChanged)
		return;

	HideInsightTooltip();

	if (LastContextUpdateDiff->LineChanged && Visible)
	{
		HidePopup(ePopupHideReason::ContextChanged);
		return;
	}

	if (Visible)
		UpdatePopup();
}

void IntelliSenseInterfaceModel::OnUserInvoked()
{
	if (Visible)
		return;

	SuppressionMode = ePopupSuppressionMode::NoSuppression;
	ShowPopup(ePopupShowReason::UserInvoked);

}

void IntelliSenseInterfaceModel::OnUserDismissed()
{
	if (!Visible)
		return;

	if (DisplayMode == ePopupDisplayMode::Manual)
		return;

	SuppressionMode = ePopupSuppressionMode::UntilNextTriggerKeyPress;
	HidePopup(ePopupHideReason::UserDismissed);
}

void IntelliSenseInterfaceModel::OnSelectionCompleted()
{
	Debug::Assert(Bound == true);

	if (!Visible)
		return;

	// Updating the parent editor's text will unleash a
	// flurry of context change events, most of which are superfluous
	SetContextChangeEventHandlingMode(eContextChangeEventHandlingMode::CacheAndIgnore);
	{
		// Cache the selection before hiding the view as it resets the selection
		auto Selection = BoundView->Selection;

		HidePopup(ePopupHideReason::SelectionComplete);

		if (Selection)
			Selection->Insert(ParentEditor);

		ParentEditor->FocusTextArea();
	}
	SetContextChangeEventHandlingMode(eContextChangeEventHandlingMode::Handle);

	// Update the context with the last CaretPosChanged event args
	if (CachedContextChangeEventArgs->ContainsKey(IntelliSenseContextChangeEventArgs::eEvent::CaretPosChanged))
		OnCaretPosChanged(CachedContextChangeEventArgs[IntelliSenseContextChangeEventArgs::eEvent::CaretPosChanged]);
}

void IntelliSenseInterfaceModel::OnScrollOffsetChanged(IntelliSenseContextChangeEventArgs^ E)
{
	UpdateContext(E);

	HideInsightTooltip();

	if (!Visible)
		return;

	// ### TODO This currently only handles vertical scroll offset changes
	if (Context->LineVisible)
		RelocatePopup();
	else
		HidePopup(ePopupHideReason::ContextChanged);
}

void IntelliSenseInterfaceModel::OnTextChanged(IntelliSenseContextChangeEventArgs^ E)
{
	UpdateContext(E);

	HideInsightTooltip();

	if (!Visible)
	{
		if (DisplayMode == ePopupDisplayMode::Manual)
			return;
		else if (LastContextUpdateDiff->LineChanged)
			return;

		if (LastContextUpdateDiff->OperationInvoked)
			ShowPopup(ePopupShowReason::OperationInvoked);
		else if (Context->FilterString->Length >= PopupThresholdLength)
			ShowPopup(ePopupShowReason::PopupThresholdReached);
	}
	else
	{
		if (!Context->Valid || (!LastContextUpdateDiff->OperationInvoked && Context->FilterString->Length == 0))
		{
			HidePopup(ePopupHideReason::ContextChanged);
			return;
		}

		if (DisplayMode == ePopupDisplayMode::Manual)
			UpdatePopup();
		else if (ShowReason != ePopupShowReason::None)
			UpdatePopup();
	}
}

void IntelliSenseInterfaceModel::UpdateContext(IntelliSenseContextChangeEventArgs^ Args)
{
	LastContextUpdateDiff->Reset();

	if (Context->CaretPos == Args->CaretPos && Context->Line == Args->CurrentLineNumber)
	{
		if (Args->Type == IntelliSenseContextChangeEventArgs::eEvent::ScrollOffsetChanged)
		{
			Context->LineVisible = Args->CurrentLineInsideViewport;
			Context->DisplayScreenCoords = Args->DisplayScreenCoords;
		}

		return;
	}

	LastContextUpdateDiff->OldCaretPos = Context->CaretPos;
	LastContextUpdateDiff->OldLine = Context->Line;
	LastContextUpdateDiff->OldCallingObjectScript = Context->CallingObjectScript;

	Context->Reset();

	Context->CaretPos = Args->CaretPos;
	if (LastContextUpdateDiff->OldCaretPos != Context->CaretPos)
		LastContextUpdateDiff->CaretPosChanged = true;

	Context->Line = Args->CurrentLineNumber;
	if (LastContextUpdateDiff->OldLine != Context->Line)
		LastContextUpdateDiff->LineChanged = true;

	Context->LineVisible = Args->CurrentLineInsideViewport;
	Context->DisplayScreenCoords = Args->DisplayScreenCoords;

	obScriptParsing::LineTokenizer^ Parser = gcnew obScriptParsing::LineTokenizer();
	String^ CurrentLine = Args->ClippedLineText;
	if (Parser->Tokenize(CurrentLine, false) == false)
		return;

	Context->InsideCommentOrStringLiteral = Parser->GetCommentTokenIndex(-1) != -1 || Parser->IsIndexInsideString(Args->CaretPos - Args->CurrentLineStartPos);
	if (Context->InsideCommentOrStringLiteral)
		return;

	String^ CurrentToken = Parser->Tokens[Parser->TokenCount - 1];
	Char CurrentTokenDelimiter = Parser->Delimiters[Parser->TokenCount - 1];
	if (CurrentTokenDelimiter == '\n')
		CurrentTokenDelimiter = Char::MinValue;

	String^ PreviousToken = Parser->TokenCount > 1 ? Parser->Tokens[Parser->TokenCount - 2] : String::Empty;
	Char PreviousTokenDelimiter = Parser->TokenCount > 1 ? Parser->Delimiters[Parser->TokenCount - 2] : Char::MinValue;

	bool PossibleOperationInvocation = false;
	if (CurrentTokenDelimiter != Char::MinValue)
	{
		PreviousToken = CurrentToken;
		PreviousTokenDelimiter = CurrentTokenDelimiter;

		CurrentToken = "";
		CurrentTokenDelimiter = Char::MinValue;
		PossibleOperationInvocation = true;
	}

	Context->FilterString = CurrentToken;
	Context->Operation = IntelliSenseModelContext::eOperationType::Default;

	bool ResolvedOp = false;
	bool UsingPreviousToken = PreviousToken != "";
	auto TokenToLookAt = UsingPreviousToken ? PreviousToken : CurrentToken;
	auto DelimiterToLookAt = UsingPreviousToken ? PreviousTokenDelimiter : CurrentTokenDelimiter;

	if (!ResolvedOp)
	{
		switch (obScriptParsing::LineTokenizer::GetScriptTokenType(TokenToLookAt))
		{
		case obScriptParsing::eScriptTokenType::Call:
			if (DelimiterToLookAt == ' ')
			{
				Context->Operation = IntelliSenseModelContext::eOperationType::Call;
				ResolvedOp = PossibleOperationInvocation;
			}
			break;
		case obScriptParsing::eScriptTokenType::Set:
		case obScriptParsing::eScriptTokenType::Let:
			if (DelimiterToLookAt == ' ')
			{
				Context->Operation = IntelliSenseModelContext::eOperationType::Assign;
				ResolvedOp = PossibleOperationInvocation;
			}
			break;
		}
	}

	if (DelimiterToLookAt == '.')
	{
		int Throwaway;
		bool IsTokenANumber = Int32::TryParse(TokenToLookAt, Throwaway);
		if (!IsTokenANumber)
		{
			Context->Operation = IntelliSenseModelContext::eOperationType::Dot;

			IntelliSenseItemScriptVariable^ RefVar = LookupLocalVariable(TokenToLookAt);
			if (RefVar && RefVar->GetDataType() == obScriptParsing::Variable::eDataType::Reference)
			{
				Context->CallingObject = RefVar;
				Context->CallingObjectIsObjectReference = true;
			}
			else
			{
				ContextualIntelliSenseLookupArgs^ LookupArgs = gcnew ContextualIntelliSenseLookupArgs;
				LookupArgs->CurrentToken = TokenToLookAt;
				auto LookupResult = IntelliSenseBackend::Get()->ContextualIntelliSenseLookup(LookupArgs);

				if (LookupResult->CurrentToken == nullptr)
					Context->Operation = IntelliSenseModelContext::eOperationType::Default;
				else
				{
					Context->CallingObject = LookupResult->CurrentToken;
					Context->CallingObjectIsObjectReference = LookupResult->CurrentTokenIsObjectReference;

					if (LookupResult->CurrentTokenIsCallableObject == false)
						Context->CallingObjectScript = IntelliSenseItemScript::Default;
					else
						Context->CallingObjectScript = IntelliSenseBackend::Get()->GetAttachedScript(LookupResult->CurrentToken);
				}
			}

			ResolvedOp = PossibleOperationInvocation;
		}
	}

	if (!ResolvedOp)
	{
		if (CurrentToken->Length == 0)
			CurrentToken = PreviousToken;

		if (CurrentToken[0] == '`')
		{
			Context->Operation = IntelliSenseModelContext::eOperationType::Snippet;
			if (CurrentToken->Length > 1)
				Context->FilterString = CurrentToken->Remove(0, 1);
			else
				Context->FilterString = "";

			ResolvedOp = true;
			PossibleOperationInvocation = true;
		}
	}

	if (Context->CallingObjectScript != LastContextUpdateDiff->OldCallingObjectScript)
	{
		LastContextUpdateDiff->CallingObjectScriptChanged = true;

		// cache the calling object's variables for quick lookup/retrieval
		Context->CallingObjectScriptVariables->Reset();
		if (Context->CallingObjectScript != nullptr)
		{
			for each (auto Var in Context->CallingObjectScript->GetVariables())
				Context->CallingObjectScriptVariables->Add(Var->GetIdentifier(), Var);
		}
	}

	LastContextUpdateDiff->OperationInvoked = PossibleOperationInvocation && ResolvedOp;
	Context->Valid = true;
}

void IntelliSenseInterfaceModel::ResetContext()
{
	Context->Reset();
	LastContextUpdateDiff->Reset();
}

bool IntelliSenseInterfaceModel::HandleKeyboardInput(IntelliSenseInputEventArgs::eEvent Type, KeyEventArgs^ E)
{
	Debug::Assert(Bound == true);

	if (IsTriggerKey(E))
		OnTriggerKeyPress();

	bool KeyUp = Type == IntelliSenseInputEventArgs::eEvent::KeyUp;
	bool PopupInvocation = E->KeyCode == Keys::Enter && E->Control;

	if (!Visible && !PopupInvocation)
		return false;

	if (KeyUp)
	{
		switch (E->KeyCode)
		{
		case Keys::LControlKey:
		case Keys::RControlKey:
			BoundView->ResetOpacity();
			break;
		}

		return false;
	}

	switch (E->KeyCode)
	{
	case Keys::Enter:
		if (E->Control)
		{
			OnUserInvoked();
			E->Handled = true;

			break;
		}

		if (!InsertSuggestionOnEnterKey)
			break;
	case Keys::Tab:
		OnSelectionCompleted();
		E->Handled = true;

		break;
	case Keys::Escape:
		OnUserDismissed();
		E->Handled = true;

		break;
	case Keys::Up:
		BoundView->ChangeSelection(IIntelliSenseInterfaceView::eMoveDirection::Up);
		E->Handled = true;

		break;
	case Keys::Down:
		BoundView->ChangeSelection(IIntelliSenseInterfaceView::eMoveDirection::Down);
		E->Handled = true;

		break;
	case Keys::LControlKey:
	case Keys::RControlKey:
		BoundView->DimOpacity();

		break;
	}

	return E->Handled;
}

bool IntelliSenseInterfaceModel::IsTriggerKey(KeyEventArgs^ E)
{
	switch (E->KeyCode)
	{
	case Keys::Oemtilde:
	case Keys::OemPeriod:
	case Keys::Oemcomma:
	case Keys::Space:
	case Keys::OemOpenBrackets:
	case Keys::OemCloseBrackets:
	case Keys::Tab:
	case Keys::Enter:
		if (E->Control || E->Alt || E->Shift)
			return false;
		else
			return true;
	default:
		return false;
	}
}

bool IntelliSenseInterfaceModel::HandleMouseInput(IntelliSenseInputEventArgs::eEvent Type, MouseEventArgs^ E)
{
	;// Nothing right now
	return false;
}

void IntelliSenseInterfaceModel::ShowInsightTooltip(IntelliSenseInsightHoverEventArgs^ E)
{
	IntelliSenseShowInsightToolTipArgs^ ShowArgs = gcnew IntelliSenseShowInsightToolTipArgs;
	ShowArgs->DisplayScreenCoords = E->DisplayScreenCoords;
	ShowArgs->ParentWindowHandle = ParentEditor->WindowHandle;

	bool HasInsight = false;
	auto Mb = gcnew utilities::TextMarkupBuilder;

	if (E->ErrorMessagesForHoveredLine->Count > 0 &&
		preferences::SettingsHolder::Get()->IntelliSense->ShowErrorsInInsightToolTip)
	{
		HasInsight = true;
		ShowArgs->TooltipHeaderText = Mb->Font(3, true)
										->Bold()
										->Text(E->ErrorMessagesForHoveredLine->Count.ToString())
										->Text(" Error")
										->Text(E->ErrorMessagesForHoveredLine->Count != 1 ? "s" : "")
										->PopTag(2)
										->ToMarkup();

		Mb->Reset()->Table(1, 350);
		{
			for each (auto Error in E->ErrorMessagesForHoveredLine)
			{
				Mb->TableNextRow();
				{
					Mb->TableNextColumn();
					{
						Mb->Font(1, true)->Text("» ")->Text(Error)->PopTag();
					}
					Mb->TableNextColumn();
				}
			}
		}
		Mb->PopTable();

		ShowArgs->TooltipBodyText = Mb->ToMarkup();
		ShowArgs->TooltipBgColor = preferences::SettingsHolder::Get()->Appearance->TooltipBackColorError;
		ShowArgs->TooltipTextColor = preferences::SettingsHolder::Get()->Appearance->TooltipForeColorError;
		ShowArgs->TooltipBodyImage = view::components::CommonIcons::Get()->ErrorLarge;
	}
	else if (!E->HoveringOverComment)
	{
		ContextualIntelliSenseLookupArgs^ LookupArgs = gcnew ContextualIntelliSenseLookupArgs;
		LookupArgs->CurrentToken = E->HoveredToken;
		LookupArgs->PreviousToken = E->PreviousToken;
		LookupArgs->DotOperatorInUse = E->DotOperatorInUse;
		LookupArgs->OnlyWithInsightInfo = true;

		auto ContextualLookup = intellisense::IntelliSenseBackend::Get()->ContextualIntelliSenseLookup(LookupArgs);
		IntelliSenseItem^ ResolvedItem = nullptr;

		if (ContextualLookup->CurrentToken)
			ResolvedItem = ContextualLookup->CurrentToken;

		if (ResolvedItem == nullptr)
			ResolvedItem = LookupLocalVariable(E->HoveredToken);

		if (ResolvedItem)
		{
			HasInsight = true;

			ShowArgs->TooltipHeaderText = ResolvedItem->TooltipHeaderText;
			ShowArgs->TooltipBodyText = ResolvedItem->TooltipBodyText;
			ShowArgs->TooltipBodyImage = ResolvedItem->TooltipBodyImage;
			ShowArgs->TooltipFooterText= ResolvedItem->TooltipFooterText;
			ShowArgs->TooltipFooterImage = ResolvedItem->TooltipFooterImage;
			ShowArgs->TooltipBgColor = ResolvedItem->TooltipBgColor;
			ShowArgs->TooltipTextColor = ResolvedItem->TooltipTextColor;
		}
	}

	if (HasInsight)
		BoundView->ShowInsightToolTip(ShowArgs);
}

void IntelliSenseInterfaceModel::HideInsightTooltip()
{
	BoundView->HideInsightToolTip();
}

void IntelliSenseInterfaceModel::ShowPopup(ePopupShowReason Reason)
{
	switch (SuppressionMode)
	{
	case ePopupSuppressionMode::NoSuppression:
		break;
	case ePopupSuppressionMode::UntilNextTriggerKeyPress:
		return;
	}

	if (UpdatePopup())
	{
		ShowReason = Reason;
		HideReason = ePopupHideReason::None;
	}
}

bool IntelliSenseInterfaceModel::UpdatePopup()
{
	PopulateDataStore();
	if (EnumeratedItems->Count == 0)
	{
		HidePopup(ePopupHideReason::ContextChanged);
		return false;
	}

	BoundView->Update();
	BoundView->Show(Context->DisplayScreenCoords);
	ParentEditor->FocusTextArea();

	return true;
}

void IntelliSenseInterfaceModel::HidePopup(ePopupHideReason Reason)
{
	BoundView->Hide();
	EnumeratedItems->Clear();

	HideReason = Reason;
	ShowReason = ePopupShowReason::None;
}

void IntelliSenseInterfaceModel::RelocatePopup()
{
	if (!Visible)
		return;

	if (Context->LineVisible)
	{
		BoundView->Show(Context->DisplayScreenCoords);
		ParentEditor->FocusTextArea();
	}
	else
		HidePopup(ePopupHideReason::ContextChanged);
}

void IntelliSenseInterfaceModel::PopulateDataStore()
{
	FetchIntelliSenseItemsArgs^ FetchArgs = gcnew FetchIntelliSenseItemsArgs;
	FetchArgs->IdentifierToMatch = Context->FilterString;
	FetchArgs->FilterMode = preferences::SettingsHolder::Get()->IntelliSense->SuggestionsFilter;
	FetchArgs->FuzzyMatchMaxCost = preferences::SettingsHolder::Get()->IntelliSense->FuzzyFilterMaxCost;
	FetchArgs->NumItemsToFetch = MaximumNumberOfSuggestions;

	EnumeratedItems->Clear();

	switch (Context->Operation)
	{
	case IntelliSenseModelContext::eOperationType::Default:
		EnumerateVariables(LocalVariables, FetchArgs->IdentifierToMatch, FetchArgs->FilterMode, FetchArgs->FuzzyMatchMaxCost);

		FetchArgs->FilterBy = FetchArgs->FilterBy & ~eDatabaseLookupFilter::Snippet;
		EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));

		break;
	case IntelliSenseModelContext::eOperationType::Call:
		FetchArgs->FilterBy = eDatabaseLookupFilter::UserFunction;
		EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));

		break;
	case IntelliSenseModelContext::eOperationType::Assign:
		EnumerateVariables(LocalVariables, FetchArgs->IdentifierToMatch, FetchArgs->FilterMode, FetchArgs->FuzzyMatchMaxCost);

		FetchArgs->FilterBy = FetchArgs->FilterBy & ~eDatabaseLookupFilter::Command;
		FetchArgs->FilterBy = FetchArgs->FilterBy & ~eDatabaseLookupFilter::Script;
		FetchArgs->FilterBy = FetchArgs->FilterBy & ~eDatabaseLookupFilter::UserFunction;
		FetchArgs->FilterBy = FetchArgs->FilterBy & ~eDatabaseLookupFilter::GameSetting;
		FetchArgs->FilterBy = FetchArgs->FilterBy & ~eDatabaseLookupFilter::Snippet;
		FetchArgs->FilterBy = FetchArgs->FilterBy & ~eDatabaseLookupFilter::Form;
		EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));

		break;
	case IntelliSenseModelContext::eOperationType::Dot:
		if (Context->CallingObjectScript != nullptr)
			EnumerateVariables(Context->CallingObjectScriptVariables, FetchArgs->IdentifierToMatch, FetchArgs->FilterMode, FetchArgs->FuzzyMatchMaxCost);

		if (Context->CallingObjectIsObjectReference)
		{
			FetchArgs->FilterBy = eDatabaseLookupFilter::Command;
			FetchArgs->Options = eDatabaseLookupOptions::OnlyCommandsThatNeedCallingObject;
			EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));
		}

		break;
	case IntelliSenseModelContext::eOperationType::Snippet:
		FetchArgs->FilterBy = eDatabaseLookupFilter::Snippet;
		EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));

		break;
	}

	if (EnumeratedItems->Count == 1)
	{
		IntelliSenseItem^ Item = EnumeratedItems[0];
		if (!String::Compare(Item->GetIdentifier(), Context->FilterString, true))
			EnumeratedItems->Clear();			// do not show when enumerable == current token
	}
}

IntelliSenseItemScriptVariable^ IntelliSenseInterfaceModel::LookupLocalVariable(String^ Identifier)
{
	return safe_cast<IntelliSenseItemScriptVariable^>(LocalVariables->Lookup(Identifier, false));
}

void IntelliSenseInterfaceModel::UpdateLocalVariables(obScriptParsing::AnalysisData^ Data)
{
	if (Data == nullptr)
		return;

	LocalVariables->Reset();
	for each (auto Itr in Data->Variables)
	{
		auto NewVar = gcnew IntelliSenseItemScriptVariable(Itr->Name, Itr->Comment, Itr->Type, String::Empty);
		LocalVariables->Add(Itr->Name, NewVar);
	}
}

void IntelliSenseInterfaceModel::EnumerateVariables(IntelliSenseItemCollection^ ScriptVariables, String^ MatchIdentifier, eFilterMode MatchType, UInt32 FuzzyMatchingMaxCost)
{
	if (MatchIdentifier->Length == 0)
	{
		EnumeratedItems->AddRange(ScriptVariables->PrefixMatch(String::Empty, nullptr));
		return;
	}

	switch (MatchType)
	{
	case eFilterMode::Prefix:
		EnumeratedItems->AddRange(ScriptVariables->PrefixMatch(MatchIdentifier, nullptr));
		break;
	case eFilterMode::Fuzzy:
	{
		auto Matches = ScriptVariables->FuzzyMatch(MatchIdentifier, FuzzyMatchingMaxCost, nullptr);
		EnumeratedItems->AddRange(ScriptVariables->SortAndExtractFuzzyMatches(Matches));

		break;
	}
	}
}

void IntelliSenseInterfaceModel::Bind(IIntelliSenseInterfaceView^ To)
{
	Debug::Assert(Bound == false);

	BoundView = To;
	BoundView->Bind(this);

	BoundView->ItemSelected += BoundViewItemSelectedHandler;
	BoundView->Dismissed += BoundViewDismissedHandler;

	SuppressionMode = ePopupSuppressionMode::NoSuppression;
	ShowReason = ePopupShowReason::None;
	HideReason = ePopupHideReason::None;
	ContextChangeHandlingMode = eContextChangeEventHandlingMode::Handle;
}

void IntelliSenseInterfaceModel::Unbind()
{
	if (Bound)
	{
		HideInsightTooltip();
		HidePopup(ePopupHideReason::ContextChanged);

		BoundView->ItemSelected -= BoundViewItemSelectedHandler;
		BoundView->Dismissed -= BoundViewDismissedHandler;

		BoundView->Unbind();
		BoundView = nullptr;

		SuppressionMode = ePopupSuppressionMode::NoSuppression;
		ShowReason = ePopupShowReason::None;
		HideReason = ePopupHideReason::None;
		ContextChangeHandlingMode = eContextChangeEventHandlingMode::Handle;
	}
}

bool IntelliSenseInterfaceModel::IsLocalVariable(String^ Identifier)
{
	return LookupLocalVariable(Identifier) != nullptr;
}


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse