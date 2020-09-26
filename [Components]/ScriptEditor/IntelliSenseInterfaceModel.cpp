#include "IntelliSenseInterfaceModel.h"
#include "IntelliSenseDatabase.h"
#include "Preferences.h"

namespace cse
{
	namespace intellisense
	{
		IntelliSenseInterfaceModel::IntelliSenseInterfaceModel(textEditors::IScriptTextEditor^ Parent)
		{
			Debug::Assert(Parent != nullptr);

			ParentEditor = Parent;
			BoundView = nullptr;

			Context = gcnew IntelliSenseModelContext;
			LastContextUpdateDiff = gcnew IntelliSenseModelContextUpdateDiff;
			CachedContextChangeEventArgs = gcnew Dictionary<IntelliSenseContextChangeEventArgs::Event, IntelliSenseContextChangeEventArgs ^>;
			EnumeratedItems = gcnew List<IntelliSenseItem^>;
			LocalVariables = gcnew List<IntelliSenseItemScriptVariable ^>;

			AutomaticallyPopup = preferences::SettingsHolder::Get()->IntelliSense->ShowSuggestions;
			PopupThresholdLength = preferences::SettingsHolder::Get()->IntelliSense->SuggestionCharThreshold;
			UseSubstringFiltering = preferences::SettingsHolder::Get()->IntelliSense->UseSubstringSearch;
			InsertSuggestionOnEnterKey = preferences::SettingsHolder::Get()->IntelliSense->InsertSuggestionsWithEnterKey;

			DisplayMode = AutomaticallyPopup ? PopupDisplayMode::Automatic : PopupDisplayMode::Manual;
			SuppressionMode = PopupSuppressionMode::NoSuppression;
			ShowReason = PopupShowReason::None;
			HideReason = PopupHideReason::None;
			ContextChangeHandlingMode = ContextChangeEventHandlingMode::Handle;

			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceModel::ScriptEditorPreferences_Saved);
			ParentEditorInputEventHandler = gcnew IntelliSenseInputEventHandler(this, &IntelliSenseInterfaceModel::ParentEditor_InputEventHandler);
			ParentEditorInsightHoverEventHandler = gcnew IntelliSenseInsightHoverEventHandler(this, &IntelliSenseInterfaceModel::ParentEditor_InsightHoverEventHandler);
			ParentEditorContextChangeEventHandler = gcnew IntelliSenseContextChangeEventHandler(this, &IntelliSenseInterfaceModel::ParentEditor_ContextChangeEventHandler);
			BoundViewItemSelectedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceModel::BoundView_ItemSelected);
			BoundViewDismissedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceModel::BoundView_Dismissed);

			preferences::SettingsHolder::Get()->SavedToDisk += ScriptEditorPreferencesSavedHandler;
			ParentEditor->IntelliSenseInput += ParentEditorInputEventHandler;
			ParentEditor->IntelliSenseContextChange += ParentEditorContextChangeEventHandler;
			ParentEditor->IntelliSenseInsightHover += ParentEditorInsightHoverEventHandler;
		}

		IntelliSenseInterfaceModel::~IntelliSenseInterfaceModel()
		{
			Unbind();
			ResetContext();

			preferences::SettingsHolder::Get()->SavedToDisk -= ScriptEditorPreferencesSavedHandler;
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
				HidePopup(PopupHideReason::None);
				HideInsightTooltip();
				BoundView->Hide();
			}

			ResetContext();

			AutomaticallyPopup = preferences::SettingsHolder::Get()->IntelliSense->ShowSuggestions;
			PopupThresholdLength = preferences::SettingsHolder::Get()->IntelliSense->SuggestionCharThreshold;
			UseSubstringFiltering = preferences::SettingsHolder::Get()->IntelliSense->UseSubstringSearch;
			InsertSuggestionOnEnterKey = preferences::SettingsHolder::Get()->IntelliSense->InsertSuggestionsWithEnterKey;

			DisplayMode = AutomaticallyPopup ? PopupDisplayMode::Automatic : PopupDisplayMode::Manual;
			SuppressionMode = PopupSuppressionMode::NoSuppression;
			ShowReason = PopupShowReason::None;
			HideReason = PopupHideReason::None;
		}


		void IntelliSenseInterfaceModel::ParentEditor_InputEventHandler(Object^ Sender, IntelliSenseInputEventArgs^ E)
		{
			Debug::Assert(Bound == true);

			switch (E->Type)
			{
			case IntelliSenseInputEventArgs::Event::KeyDown:
			case IntelliSenseInputEventArgs::Event::KeyUp:
				E->Handled = HandleKeyboardInput(E->Type, E->KeyEvent);
				break;
			case IntelliSenseInputEventArgs::Event::MouseDown:
			case IntelliSenseInputEventArgs::Event::MouseUp:
				E->Handled = HandleMouseInput(E->Type, E->MouseEvent);
				break;
			}
		}

		void IntelliSenseInterfaceModel::ParentEditor_InsightHoverEventHandler(Object^ Sender, IntelliSenseInsightHoverEventArgs^ E)
		{
			Debug::Assert(Bound == true);

			if (preferences::SettingsHolder::Get()->IntelliSense->ShowInsightToolTip == false)
				return;

			if (E->Type == IntelliSenseInsightHoverEventArgs::Event::HoverStart)
				ShowInsightTooltip(E);
			else
				HideInsightTooltip();
		}

		void IntelliSenseInterfaceModel::ParentEditor_ContextChangeEventHandler(Object^ Sender, IntelliSenseContextChangeEventArgs^ E)
		{
			if (!Bound)
				return;

			if (ContextChangeHandlingMode == ContextChangeEventHandlingMode::CacheAndIgnore)
			{
				// Cache the last received argument of each type
				CachedContextChangeEventArgs[E->Type] = E;
				return;
			}

			switch (E->Type)
			{
			case IntelliSenseContextChangeEventArgs::Event::Reset:
				OnParentReset();
				break;
			case IntelliSenseContextChangeEventArgs::Event::TextChanged:
				OnTextChanged(E);
				break;
			case IntelliSenseContextChangeEventArgs::Event::CaretPosChanged:
				OnCaretPosChanged(E);
				break;
			case IntelliSenseContextChangeEventArgs::Event::ScrollOffsetChanged:
				OnScrollOffsetChanged(E);
				break;
			case IntelliSenseContextChangeEventArgs::Event::SemanticAnalysisCompleted:
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

		void IntelliSenseInterfaceModel::SetContextChangeEventHandlingMode(ContextChangeEventHandlingMode Mode)
		{
			if (ContextChangeHandlingMode == Mode)
				return;

			switch (Mode)
			{
			case ContextChangeEventHandlingMode::Handle:
				ContextChangeHandlingMode = Mode;
				break;
			case ContextChangeEventHandlingMode::CacheAndIgnore:
				CachedContextChangeEventArgs->Clear();
				ContextChangeHandlingMode = Mode;
				break;
			}
		}

		void IntelliSenseInterfaceModel::OnParentReset()
		{
			HidePopup(PopupHideReason::ContextChanged);
			HideInsightTooltip();
			ResetContext();
			EnumeratedItems->Clear();
		}

		void IntelliSenseInterfaceModel::OnTriggerKeyPress()
		{
			if (Visible)
				return;

			if (DisplayMode == PopupDisplayMode::Manual)
				return;

			if (SuppressionMode == PopupSuppressionMode::UntilNextTriggerKeyPress)
				SuppressionMode = PopupSuppressionMode::NoSuppression;
		}

		void IntelliSenseInterfaceModel::OnCaretPosChanged(IntelliSenseContextChangeEventArgs^ E)
		{
			UpdateContext(E);

			if (!LastContextUpdateDiff->CaretPosChanged && !LastContextUpdateDiff->LineChanged)
				return;

			if (LastContextUpdateDiff->LineChanged && Visible)
				HidePopup(PopupHideReason::ContextChanged);

			if (Visible)
				UpdatePopup();
		}

		void IntelliSenseInterfaceModel::OnUserInvoked()
		{
			if (Visible)
				return;

			SuppressionMode = PopupSuppressionMode::NoSuppression;
			ShowPopup(PopupShowReason::UserInvoked);

		}

		void IntelliSenseInterfaceModel::OnUserDismissed()
		{
			if (!Visible)
				return;

			if (DisplayMode == PopupDisplayMode::Manual)
				return;

			SuppressionMode = PopupSuppressionMode::UntilNextTriggerKeyPress;
			HidePopup(PopupHideReason::UserDismissed);
		}

		void IntelliSenseInterfaceModel::OnSelectionCompleted()
		{
			Debug::Assert(Bound == true);

			if (!Visible)
				return;

			// Updating the parent editor's text will unleash a
			// flurry of context change events, most of which are superfluous
			SetContextChangeEventHandlingMode(ContextChangeEventHandlingMode::CacheAndIgnore);
			{
				// Cache the selection before hiding the view as it resets the selection
				auto Selection = BoundView->Selection;

				HidePopup(PopupHideReason::SelectionComplete);

				if (Selection)
					Selection->Insert(ParentEditor);

				ParentEditor->FocusTextArea();
			}
			SetContextChangeEventHandlingMode(ContextChangeEventHandlingMode::Handle);

			// Update the context with the last CaretPosChanged event args
			if (CachedContextChangeEventArgs->ContainsKey(IntelliSenseContextChangeEventArgs::Event::CaretPosChanged))
				OnCaretPosChanged(CachedContextChangeEventArgs[IntelliSenseContextChangeEventArgs::Event::CaretPosChanged]);
		}

		void IntelliSenseInterfaceModel::OnScrollOffsetChanged(IntelliSenseContextChangeEventArgs^ E)
		{
			UpdateContext(E);

			HideInsightTooltip();

			if (!Visible)
				return;

			// ### This currently only handles vertical scroll offset changes
			if (Context->LineVisible)
				RelocatePopup();
			else
				HidePopup(PopupHideReason::ContextChanged);
		}

		void IntelliSenseInterfaceModel::OnTextChanged(IntelliSenseContextChangeEventArgs^ E)
		{
			UpdateContext(E);

			if (!Visible)
			{
				if (DisplayMode == PopupDisplayMode::Manual)
					return;
				else if (LastContextUpdateDiff->LineChanged)
					return;

				if (LastContextUpdateDiff->OperationInvoked)
					ShowPopup(PopupShowReason::OperationInvoked);
				else if (Context->FilterString->Length >= PopupThresholdLength)
					ShowPopup(PopupShowReason::PopupThresholdReached);
			}
			else
			{
				if (!Context->Valid)
				{
					HidePopup(PopupHideReason::ContextChanged);
					return;
				}

				if (DisplayMode == PopupDisplayMode::Manual)
					UpdatePopup();
				else if (ShowReason != PopupShowReason::None)
					UpdatePopup();
			}
		}

		void IntelliSenseInterfaceModel::UpdateContext(IntelliSenseContextChangeEventArgs^ Args)
		{
			LastContextUpdateDiff->Reset();

			if (Context->CaretPos == Args->CaretPos && Context->Line == Args->CurrentLineNumber)
			{
				if (Args->Type == IntelliSenseContextChangeEventArgs::Event::ScrollOffsetChanged)
				{
					Context->LineVisible = Args->CurrentLineInsideViewport;
					Context->DisplayScreenCoords = Args->DisplayScreenCoords;
				}

				return;
			}

			LastContextUpdateDiff->OldCaretPos = Context->CaretPos;
			LastContextUpdateDiff->OldLine = Context->Line;

			Context->Reset();

			Context->CaretPos = Args->CaretPos;
			if (LastContextUpdateDiff->OldCaretPos != Context->CaretPos)
				LastContextUpdateDiff->CaretPosChanged = true;

			Context->Line = Args->CurrentLineNumber;
			if (LastContextUpdateDiff->OldLine != Context->Line)
				LastContextUpdateDiff->LineChanged = true;

			Context->LineVisible = Args->CurrentLineInsideViewport;
			Context->DisplayScreenCoords = Args->DisplayScreenCoords;

			ScriptParser^ Parser = gcnew ScriptParser();
			String^ CurrentLine = Args->ClippedLineText;
			if (Parser->Tokenize(CurrentLine, false) == false)
				return;

			if (Parser->GetCommentTokenIndex(-1) != -1
				|| Parser->IsIndexInsideString(Args->CaretPos - Args->CurrentLineStartPos))
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

			bool UsingPreviousToken = PreviousToken != "";
			bool ResolvedOp = false;

			Context->FilterString = CurrentToken;
			Context->Operation = IntelliSenseModelContext::OperationType::Default;


			if (!ResolvedOp)
			{
				switch (ScriptParser::GetScriptTokenType(UsingPreviousToken ? PreviousToken : CurrentToken))
				{
				case obScriptParsing::ScriptTokenType::Call:
					if ((UsingPreviousToken ? PreviousTokenDelimiter : CurrentTokenDelimiter) == ' ')
					{
						Context->Operation = IntelliSenseModelContext::OperationType::Call;

						ResolvedOp = PossibleOperationInvocation;
					}
					break;
				case obScriptParsing::ScriptTokenType::Set:
				case obScriptParsing::ScriptTokenType::Let:
					if ((UsingPreviousToken ? PreviousTokenDelimiter : CurrentTokenDelimiter) == ' ')
					{
						Context->Operation = IntelliSenseModelContext::OperationType::Assign;

						ResolvedOp = PossibleOperationInvocation;
					}
					break;
				}
			}

			if ((UsingPreviousToken ? PreviousTokenDelimiter : CurrentTokenDelimiter) == '.')
			{
				Context->Operation = IntelliSenseModelContext::OperationType::Dot;

				IntelliSenseItemScriptVariable^ RefVar = LookupLocalVariable(UsingPreviousToken ? PreviousToken : CurrentToken);
				if (RefVar && RefVar->GetDataType() == obScriptParsing::Variable::DataType::Ref)
				{
					Context->CallingObject = RefVar;
					Context->CallingObjectIsObjectReference = true;
				}
				else
				{
					ContextualIntelliSenseLookupArgs^ LookupArgs = gcnew ContextualIntelliSenseLookupArgs;
					LookupArgs->CurrentToken = UsingPreviousToken ? PreviousToken : CurrentToken;
					auto LookupResult = IntelliSenseBackend::Get()->ContextualIntelliSenseLookup(LookupArgs);

					if (LookupResult->CurrentToken == nullptr)
						Context->Operation = IntelliSenseModelContext::OperationType::Default;
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

			if (!ResolvedOp)
			{
				if (CurrentToken->Length == 0)
					CurrentToken = PreviousToken;

				if (CurrentToken[0] == '`')
				{
					Context->Operation = IntelliSenseModelContext::OperationType::Snippet;
					if (CurrentToken->Length > 1)
						Context->FilterString = CurrentToken->Remove(0, 1);
					else
						Context->FilterString = "";

					ResolvedOp = true;
					PossibleOperationInvocation = true;
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

		bool IntelliSenseInterfaceModel::HandleKeyboardInput(IntelliSenseInputEventArgs::Event Type, KeyEventArgs^ E)
		{
			Debug::Assert(Bound == true);

			if (IsTriggerKey(E))
				OnTriggerKeyPress();

			bool KeyUp = Type == IntelliSenseInputEventArgs::Event::KeyUp;
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
				BoundView->ChangeSelection(IIntelliSenseInterfaceView::MoveDirection::Up);
				E->Handled = true;

				break;
			case Keys::Down:
				BoundView->ChangeSelection(IIntelliSenseInterfaceView::MoveDirection::Down);
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

		bool IntelliSenseInterfaceModel::HandleMouseInput(IntelliSenseInputEventArgs::Event Type, MouseEventArgs^ E)
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

			if (E->UseOverrideParams)
			{
				HasInsight = true;
				ShowArgs->Text = E->OverrideText;
				ShowArgs->Title = E->OverrideTitle;
				ShowArgs->Icon = E->OverrideIcon;
			}
			else
			{
				ContextualIntelliSenseLookupArgs^ LookupArgs = gcnew ContextualIntelliSenseLookupArgs;
				LookupArgs->CurrentToken = E->CurrentToken;
				LookupArgs->PreviousToken = E->PreviousToken;
				LookupArgs->DotOperatorInUse = E->DotOperatorInUse;
				LookupArgs->OnlyWithInsightInfo = true;

				auto ContextualLookup = intellisense::IntelliSenseBackend::Get()->ContextualIntelliSenseLookup(LookupArgs);
				IntelliSenseItem^ ResolvedItem = nullptr;

				if (ContextualLookup->CurrentToken)
					ResolvedItem = ContextualLookup->CurrentToken;

				if (ResolvedItem == nullptr)
					ResolvedItem = LookupLocalVariable(E->CurrentToken);

				if (ResolvedItem)
				{
					HasInsight = true;
					ShowArgs->Text = ResolvedItem->Describe();
					ShowArgs->Title = ResolvedItem->GetItemTypeName();
					ShowArgs->Icon = ToolTipIcon::Info;
				}
			}

			if (HasInsight)
				BoundView->ShowInsightToolTip(ShowArgs);
		}

		void IntelliSenseInterfaceModel::HideInsightTooltip()
		{
			BoundView->HideInsightToolTip();
		}

		void IntelliSenseInterfaceModel::ShowPopup(PopupShowReason Reason)
		{
			switch (SuppressionMode)
			{
			case PopupSuppressionMode::NoSuppression:
				break;
			case PopupSuppressionMode::UntilNextTriggerKeyPress:
				return;
			}

			UpdatePopup();
			ShowReason = Reason;
			HideReason = PopupHideReason::None;
		}

		void IntelliSenseInterfaceModel::UpdatePopup()
		{
			PopulateDataStore();
			if (EnumeratedItems->Count > 0)
			{
				BoundView->Update();
				BoundView->Show(Context->DisplayScreenCoords, ParentEditor->WindowHandle);
				ParentEditor->FocusTextArea();
			}
			else
				HidePopup(PopupHideReason::ContextChanged);
		}

		void IntelliSenseInterfaceModel::HidePopup(PopupHideReason Reason)
		{
			BoundView->Hide();
			EnumeratedItems->Clear();

			HideReason = Reason;
			ShowReason = PopupShowReason::None;
		}

		void IntelliSenseInterfaceModel::RelocatePopup()
		{
			if (!Visible)
				return;

			if (Context->LineVisible)
			{
				BoundView->Show(Context->DisplayScreenCoords, ParentEditor->WindowHandle);
				ParentEditor->FocusTextArea();
			}
			else
				HidePopup(PopupHideReason::ContextChanged);
		}

		void IntelliSenseInterfaceModel::PopulateDataStore()
		{
			StringMatchType MatchType = UseSubstringFiltering ? StringMatchType::Substring : StringMatchType::StartsWith;

			FetchIntelliSenseItemsArgs^ FetchArgs = gcnew FetchIntelliSenseItemsArgs;
			FetchArgs->IdentifierToMatch = Context->FilterString;
			FetchArgs->MatchType = MatchType;

			EnumeratedItems->Clear();

			switch (Context->Operation)
			{
			case IntelliSenseModelContext::OperationType::Default:
				EnumerateIntelliSenseItems(LocalVariables, MatchType);

				FetchArgs->FilterBy = FetchArgs->FilterBy & ~DatabaseLookupFilter::Snippet;
				EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));

				break;
			case IntelliSenseModelContext::OperationType::Call:
				FetchArgs->FilterBy = DatabaseLookupFilter::UserFunction;
				EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));

				break;
			case IntelliSenseModelContext::OperationType::Assign:
				EnumerateIntelliSenseItems(LocalVariables, MatchType);

				FetchArgs->FilterBy = FetchArgs->FilterBy & ~DatabaseLookupFilter::Command;
				FetchArgs->FilterBy = FetchArgs->FilterBy & ~DatabaseLookupFilter::Script;
				FetchArgs->FilterBy = FetchArgs->FilterBy & ~DatabaseLookupFilter::UserFunction;
				FetchArgs->FilterBy = FetchArgs->FilterBy & ~DatabaseLookupFilter::GameSetting;
				FetchArgs->FilterBy = FetchArgs->FilterBy & ~DatabaseLookupFilter::Snippet;
				FetchArgs->FilterBy = FetchArgs->FilterBy & ~DatabaseLookupFilter::Form;
				EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));

				break;
			case IntelliSenseModelContext::OperationType::Dot:
				if (Context->CallingObjectScript != nullptr)
					EnumerateIntelliSenseItems(Context->CallingObjectScript->GetVariables(), MatchType);

				if (Context->CallingObjectIsObjectReference)
				{
					FetchArgs->FilterBy = DatabaseLookupFilter::Command;
					FetchArgs->Options = DatabaseLookupOptions::OnlyCommandsThatNeedCallingObject;
					EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));
				}

				break;
			case IntelliSenseModelContext::OperationType::Snippet:
				FetchArgs->FilterBy = DatabaseLookupFilter::Snippet;
				EnumeratedItems->AddRange(IntelliSenseBackend::Get()->FetchIntelliSenseItems(FetchArgs));

				break;
			}

			if (EnumeratedItems->Count == 1)
			{
				IntelliSenseItem^ Item = EnumeratedItems[0];
				if (Item->MatchesToken(Context->FilterString, StringMatchType::FullMatch))
					EnumeratedItems->Clear();			// do not show when enumerable == current token
			}
		}

		generic <typename T> where T : IntelliSenseItem
		void IntelliSenseInterfaceModel::EnumerateIntelliSenseItems(IEnumerable<T>^ Items, StringMatchType MatchType)
		{
			for each (IntelliSenseItem ^ Itr in Items)
			{
				if (Context->FilterString->Length == 0 || Itr->MatchesToken(Context->FilterString, MatchType))
					EnumeratedItems->Add(Itr);
			}
		}

		IntelliSenseItemScriptVariable^ IntelliSenseInterfaceModel::LookupLocalVariable(String^ Identifier)
		{
			for each (IntelliSenseItemScriptVariable ^ Itr in LocalVariables)
			{
				if (Itr->MatchesToken(Identifier, StringMatchType::FullMatch))
					return Itr;
			}

			return nullptr;
		}

		void IntelliSenseInterfaceModel::UpdateLocalVariables(obScriptParsing::AnalysisData^ Data)
		{
			if (Data == nullptr)
				return;

			LocalVariables->Clear();
			for each (auto Itr in Data->Variables)
			{
				auto NewVar = gcnew IntelliSenseItemScriptVariable(Itr->Name, Itr->Comment, Itr->Type, String::Empty);
				LocalVariables->Add(NewVar);
			}
		}

		void IntelliSenseInterfaceModel::Bind(IIntelliSenseInterfaceView^ To)
		{
			Debug::Assert(Bound == false);

			BoundView = To;
			BoundView->Bind(this);

			BoundView->ItemSelected += BoundViewItemSelectedHandler;
			BoundView->Dismissed += BoundViewDismissedHandler;

			SuppressionMode = PopupSuppressionMode::NoSuppression;
			ShowReason = PopupShowReason::None;
			HideReason = PopupHideReason::None;
			ContextChangeHandlingMode = ContextChangeEventHandlingMode::Handle;
		}

		void IntelliSenseInterfaceModel::Unbind()
		{
			if (Bound)
			{
				HideInsightTooltip();
				HidePopup(PopupHideReason::ContextChanged);

				BoundView->ItemSelected -= BoundViewItemSelectedHandler;
				BoundView->Dismissed -= BoundViewDismissedHandler;
				BoundView->Hide();

				BoundView->Unbind();
				BoundView = nullptr;

				SuppressionMode = PopupSuppressionMode::NoSuppression;
				ShowReason = PopupShowReason::None;
				HideReason = PopupHideReason::None;
				ContextChangeHandlingMode = ContextChangeEventHandlingMode::Handle;
			}
		}


	}
}