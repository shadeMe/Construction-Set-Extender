#include "IntelliSenseItem.h"
#include "IntelliSenseBackend.h"

#include "SemanticAnalysis.h"
#include "Preferences.h"
#include "SnippetManager.h"

#include "[Common]\NativeWrapper.h"


namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


FetchIntelliSenseItemsArgs::FetchIntelliSenseItemsArgs()
{
	IdentifierToMatch = String::Empty;
	FilterMode = preferences::SettingsHolder::Get()->IntelliSense->SuggestionsFilter;
	FilterBy = eDatabaseLookupFilter::All;
	Options = eDatabaseLookupOptions::None;
	FuzzyMatchMaxCost = 0;
	NumItemsToFetch = -1;
}

ContextualIntelliSenseLookupArgs::ContextualIntelliSenseLookupArgs()
{
	CurrentToken = String::Empty;
	PreviousToken = String::Empty;
	DotOperatorInUse = false;
	OnlyWithInsightInfo = false;
}

ContextualIntelliSenseLookupResult::ContextualIntelliSenseLookupResult()
{
	CurrentToken = nullptr;
	CurrentTokenIsCallableObject = false;
	CurrentTokenIsObjectReference = false;

	PreviousToken = nullptr;
	PreviousTokenIsCallableObject = false;
	PreviousTokenIsObjectReference = false;
}

int IntelliSenseItemCollection::FuzzyMatchResultSortKeySelector(utilities::CaselessFuzzyTrie<IntelliSenseItem^>::FuzzyMatchResult^ Item)
{
	return Item->Cost;
}

IntelliSenseItem^ IntelliSenseItemCollection::FuzzyMatchResultMapToItemSelector(utilities::CaselessFuzzyTrie<IntelliSenseItem^>::FuzzyMatchResult^ Item)
{
	return Item->Value;
}

IntelliSenseItemCollection::PrefixToEditDistanceMatcher::PrefixToEditDistanceMatcher(String^ Query, int LevenshteinMaxEditDistance)
{
	QueryLength = Query->Length;
	MinimumEditDistance = LevenshteinMaxEditDistance + 1;
}

IntelliSenseItemCollection::FuzzyMatchT^ IntelliSenseItemCollection::PrefixToEditDistanceMatcher::CreateFuzzyMatch(IntelliSenseItem^ Item)
{
	return gcnew FuzzyMatchT(Item, Math::Abs(QueryLength - Item->GetIdentifier()->Length));
}

bool IntelliSenseItemCollection::PrefixToEditDistanceMatcher::FuzzyMatchHasMinimumEditDistance(FuzzyMatchT^ Match)
{
	return Match->Cost >= MinimumEditDistance;
}

IntelliSenseItemCollection::IntelliSenseItemCollection()
{
	Dict_ = gcnew Dictionary<String^, IntelliSenseItem^>(StringComparer::CurrentCultureIgnoreCase);
	Trie_ = gcnew utilities::CaselessFuzzyTrie<IntelliSenseItem^>;
}

void IntelliSenseItemCollection::Add(String^ Identifier, IntelliSenseItem^ Item)
{
	if (!Dict->ContainsKey(Identifier))
		Dict->Add(Identifier, Item);

	Trie->Add(Identifier, Item);
}

IEnumerable<IntelliSenseItemCollection::FuzzyMatchT^>^ IntelliSenseItemCollection::LevenshteinMatch(String^ Query,
																							  UInt32 MaxEditDistanceCost,
																							  System::Func<IntelliSenseItem^, bool>^ Predicate)
{
	auto Matches = Predicate ? Trie->LevenshteinMatch(Query, MaxEditDistanceCost, Predicate) : Trie->LevenshteinMatch(Query, MaxEditDistanceCost);
	return Matches;
}

IEnumerable<IntelliSenseItem^>^ IntelliSenseItemCollection::PrefixMatch(String^ Query,  System::Func<IntelliSenseItem^, bool>^ Predicate)
{
	return Predicate ? Trie->Retrieve(Query, Predicate) : Trie->Retrieve(Query);
}

IEnumerable<IntelliSenseItemCollection::FuzzyMatchT^>^ IntelliSenseItemCollection::FuzzyMatch(String^ Query, UInt32 MaxEditDistanceCost, System::Func<IntelliSenseItem^, bool>^ Predicate)
{
	/*
	* 1. Collect all prefix matches for the query.
	* 2. Calculate edit distance, i.e., difference in length between the match and the query, for all prefix matches and map to FuzzyMatchT.
	* 3. Collect all possible Levenshtein matches for the query.
	* 4. Combine matches.
	*
	* The results will eventually have to be sorted by edit distance.
	*/

	auto PrefixMatches = PrefixMatch(Query, Predicate);

	auto PrefixMatchesToEditDistConverter = gcnew PrefixToEditDistanceMatcher(Query, MaxEditDistanceCost);
	auto PrefixMatchesWithEditDistance = System::Linq::Enumerable::Select(PrefixMatches, gcnew Func<IntelliSenseItem^, FuzzyMatchT^>(PrefixMatchesToEditDistConverter, &PrefixToEditDistanceMatcher::CreateFuzzyMatch));
	//PrefixMatchesWithEditDistance = System::Linq::Enumerable::Where(PrefixMatchesWithEditDistance, gcnew Func<FuzzyMatchT^, bool>(PrefixMatcheToEditDistConverter, &PrefixToEditDistanceMatcher::FuzzyMatchHasMinimumEditDistance));

	if (Query->Length == 0)
		return PrefixMatchesWithEditDistance;

	auto LevenshteinMatches = LevenshteinMatch(Query, MaxEditDistanceCost, Predicate);
	auto CombinedResults = System::Linq::Enumerable::Concat(PrefixMatchesWithEditDistance, LevenshteinMatches);

	// Duplicate entries need to be removed (due to overlapping matches between the prefix and Levenshtein queries)
	// NOTE: This means the equality comparer of FuzzyMatchT must IGNORE the cost and only compare the value
	return System::Linq::Enumerable::Distinct(CombinedResults);
}

IntelliSenseItem^ IntelliSenseItemCollection::Lookup(String^ Identifier, bool IgnoreItemsWithoutInsightInfo)
{
	IntelliSenseItem^ Item;
	if (Dict->TryGetValue(Identifier, Item))
	{
		if (!IgnoreItemsWithoutInsightInfo || Item->HasInsightInfo())
			return Item;
	}

	return nullptr;
}

void IntelliSenseItemCollection::Reset()
{
	Dict_->Clear();
	Trie_ = gcnew utilities::CaselessFuzzyTrie<IntelliSenseItem^>;
}

System::Collections::Generic::IEnumerable<IntelliSenseItem^>^ IntelliSenseItemCollection::SortAndExtractFuzzyMatches(IEnumerable<FuzzyMatchT^>^ FuzzyMatches)
{
	auto Sorted = System::Linq::Enumerable::OrderBy(FuzzyMatches, gcnew Func<FuzzyMatchT^, int>(IntelliSenseItemCollection::FuzzyMatchResultSortKeySelector));
	//int i = 0;
	//DebugPrint("---------------------------------");
	//for each (auto Itr in Sorted)
	//{
	//	if (i > 5)
	//		break;

	//	DebugPrint(String::Format("[{0}] {1}", Itr->Cost, Itr->Value->GetIdentifier()));
	//	++i;
	//}

	auto MapToItems = System::Linq::Enumerable::Select(Sorted, gcnew Func<FuzzyMatchT^, IntelliSenseItem^>(IntelliSenseItemCollection::FuzzyMatchResultMapToItemSelector));
	return MapToItems;
}

IntelliSenseBackend::UpdateTaskPayload::UpdateTaskPayload()
{
	Stopwatch = gcnew System::Diagnostics::Stopwatch;

	IncomingScripts = gcnew List<nativeWrapper::MarshalledScriptData^>;
	IncomingQuests = gcnew List<nativeWrapper::MarshalledFormData^>;
	IncomingGlobals = gcnew List<nativeWrapper::MarshalledVariableData^>;
	IncomingForms = gcnew List<nativeWrapper::MarshalledFormData^>;

	OutgoingScripts = gcnew IntelliSenseItemCollection;
	OutgoingGlobalVariables = gcnew IntelliSenseItemCollection;
	OutgoingQuests = gcnew IntelliSenseItemCollection;
	OutgoingForms = gcnew IntelliSenseItemCollection;
}

void IntelliSenseBackend::Preferences_Saved(Object^ Sender, EventArgs^ E)
{
	if (UpdateTimerMode == eTimerMode::IdleQueue)
		SetUpdateTimerMode(eTimerMode::IdleQueue);		// re-init timer
}

void IntelliSenseBackend::UpdateTimer_Tick(Object^ Sender, EventArgs^ E)
{
	switch (UpdateTimerMode)
	{
	case eTimerMode::IdlePoll:
		HandleActiveUpdateTaskPolling();
		break;
	case eTimerMode::IdleQueue:
		HandleUpdateTaskQueuing(false);
		break;
	}
}

IntelliSenseBackend::UpdateTaskPayload^ IntelliSenseBackend::PerformBackgroundUpdateTask(Object^ Input)
{
	auto Payload = safe_cast<UpdateTaskPayload^>(Input);

	Payload->Stopwatch->Reset();
	Payload->Stopwatch->Start();
	{
		for each (auto Datum in Payload->IncomingScripts)
		{
			auto NewItem = Datum->IsUdf ? gcnew IntelliSenseItemUserFunction(Datum) : gcnew IntelliSenseItemScript(Datum);

			IntelliSenseItem^ Existing = nullptr;
			if (!Payload->OutgoingScripts->Dict->TryGetValue(Datum->EditorId, Existing))
				Payload->OutgoingScripts->Add(Datum->EditorId, NewItem);
		}

		for each (auto Datum in Payload->IncomingQuests)
		{
			auto NewItem = gcnew IntelliSenseItemQuest(Datum);

			IntelliSenseItem^ Existing = nullptr;
			if (!Payload->OutgoingQuests->Dict->TryGetValue(Datum->EditorId, Existing))
				Payload->OutgoingQuests->Add(Datum->EditorId, NewItem);
		}

		for each (auto Datum in Payload->IncomingGlobals)
		{
			auto NewItem = gcnew IntelliSenseItemGlobalVariable(Datum);

			IntelliSenseItem^ Existing = nullptr;
			if (!Payload->OutgoingGlobalVariables->Dict->TryGetValue(Datum->EditorId, Existing))
				Payload->OutgoingGlobalVariables->Add(Datum->EditorId, NewItem);
		}

		for each (auto Datum in Payload->IncomingForms)
		{
			auto NewItem = gcnew IntelliSenseItemForm(Datum);

			IntelliSenseItem^ Existing = nullptr;
			if (!Payload->OutgoingForms->Dict->TryGetValue(Datum->EditorId, Existing))
				Payload->OutgoingForms->Add(Datum->EditorId, NewItem);
		}
	}
	Payload->Stopwatch->Stop();

	return Payload;
}

bool IntelliSenseBackend::HandleActiveUpdateTaskPolling()
{
	Debug::Assert(ActiveUpdateTask != nullptr);

	bool Error = false;
	switch (ActiveUpdateTask->Status)
	{
	case System::Threading::Tasks::TaskStatus::Faulted:
		DebugPrint("IntelliSense Backend update task failed! Exception: " + ActiveUpdateTask->Exception->ToString(), true);
		nativeWrapper::WriteToMainWindowStatusBar(2, "IntelliSense Backend Error!");
		Error = true;

		break;
	case System::Threading::Tasks::TaskStatus::RanToCompletion:
	{
		Scripts = ActiveUpdateTask->Result->OutgoingScripts;
		GlobalVariables = ActiveUpdateTask->Result->OutgoingGlobalVariables;
		Quests = ActiveUpdateTask->Result->OutgoingQuests;
		Forms = ActiveUpdateTask->Result->OutgoingForms;

		nativeWrapper::WriteToMainWindowStatusBar(2, "IntelliSense Backend updated");
		String^ UpdateStr = "[" +
			ActiveUpdateTask->Result->Stopwatch->ElapsedMilliseconds.ToString() + " ms | " +
			Scripts->Count + " Script(s) | " +
			Quests->Count + " Quest(s) | " +
			GlobalVariables->Count + " Global(s) | " +
			Forms->Count + " Forms" +
			"]";

		nativeWrapper::WriteToMainWindowStatusBar(3, UpdateStr);
		//DebugPrint(UpdateStr);

		break;
	}
	default:
		return false;
	}

	ActiveUpdateTask = nullptr;
	LastUpdateTimestamp = DateTime::Now;

	SetUpdateTimerMode(eTimerMode::IdleQueue);
	return true;
}

bool IntelliSenseBackend::HandleUpdateTaskQueuing(bool Force)
{
	if (!nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CanUpdateIntelliSenseDatabase())
		return false;
	else if (!Force && (DateTime::Now - LastUpdateTimestamp).TotalMilliseconds < UpdateTimer->Interval)
		return false;

	Debug::Assert(ActiveUpdateTask == nullptr);

	nativeWrapper::WriteToMainWindowStatusBar(2, "Updating IntelliSense DB...");

	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::IntelliSenseUpdateData> DataHandlerData
		(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetIntelliSenseUpdateData());
	auto UpdatePayload = GenerateUpdateTaskPayload(DataHandlerData.get());
	auto TaskDelegate = gcnew System::Func<Object^, UpdateTaskPayload^>(&IntelliSenseBackend::PerformBackgroundUpdateTask);
	ActiveUpdateTask = UpdateTaskT::Factory->StartNew(TaskDelegate, UpdatePayload);

	SetUpdateTimerMode(eTimerMode::IdlePoll);
	return true;
}

void IntelliSenseBackend::SetUpdateTimerMode(eTimerMode Mode)
{
	this->UpdateTimerMode = Mode;

	switch (Mode)
	{
	case eTimerMode::IdlePoll:
		UpdateTimer->Interval = kTimerPollInterval;
		UpdateTimer->Stop();
		UpdateTimer->Start();

		break;
	case eTimerMode::IdleQueue:
		UpdateTimer->Interval = preferences::SettingsHolder::Get()->IntelliSense->DatabaseUpdateInterval * 60 * 1000;
		UpdateTimer->Stop();
		UpdateTimer->Start();

		break;
	default:
		break;

	}
}

IntelliSenseBackend::UpdateTaskPayload^ IntelliSenseBackend::GenerateUpdateTaskPayload(componentDLLInterface::IntelliSenseUpdateData* NativeData)
{
	auto Payload = gcnew UpdateTaskPayload;

	for (int i = 0; i < NativeData->ScriptCount; ++i)
	{
		componentDLLInterface::ScriptData* Data = &NativeData->ScriptListHead[i];
		if (Data->IsValid())
			Payload->IncomingScripts->Add(gcnew nativeWrapper::MarshalledScriptData(Data));
	}

	for (int i = 0; i < NativeData->QuestCount; ++i)
	{
		componentDLLInterface::QuestData* Data = &NativeData->QuestListHead[i];
		if (Data->IsValid())
			Payload->IncomingQuests->Add(gcnew nativeWrapper::MarshalledFormData(Data));
	}

	for (int i = 0; i < NativeData->GlobalCount; ++i)
	{
		componentDLLInterface::GlobalData* Data = &NativeData->GlobalListHead[i];
		if (Data->IsValid())
			Payload->IncomingGlobals->Add(gcnew nativeWrapper::MarshalledVariableData(Data));
	}

	for (int i = 0; i < NativeData->MiscFormListCount; ++i)
	{
		componentDLLInterface::FormData* Data = &NativeData->MiscFormListHead[i];
		if (Data->IsValid())
			Payload->IncomingForms->Add(gcnew nativeWrapper::MarshalledFormData(Data));
	}

	return Payload;
}

void IntelliSenseBackend::RefreshCodeSnippetIntelliSenseItems()
{
	Snippets->Reset();

	for each (auto Itr in SnippetCollection->LoadedSnippets)
	{
		auto Item = gcnew IntelliSenseItemCodeSnippet(Itr);
		Snippets->Dict->Add(Itr->Name, Item);
		Snippets->Trie->Add(Itr->Name, Item);
	}
}

IntelliSenseItem^ IntelliSenseBackend::LookupIntelliSenseItem(String^ Identifier, bool OnlyWithInsightInfo)
{
	if (Identifier == String::Empty)
		return nullptr;

	IntelliSenseItem^ Item = nullptr;

	if ((Item = ScriptCommands->Lookup(Identifier, OnlyWithInsightInfo)))
		return Item;

	if ((Item = GameSettings->Lookup(Identifier, OnlyWithInsightInfo)))
		return Item;

	if ((Item = Scripts->Lookup(Identifier, OnlyWithInsightInfo)))
		return Item;

	if ((Item = GlobalVariables->Lookup(Identifier, OnlyWithInsightInfo)))
		return Item;

	if ((Item = Quests->Lookup(Identifier, OnlyWithInsightInfo)))
		return Item;

	if ((Item = Forms->Lookup(Identifier, OnlyWithInsightInfo)))
		return Item;

	if ((Item = Snippets->Lookup(Identifier, OnlyWithInsightInfo)))
		return Item;

	return Item;
}

IntelliSenseItem^ IntelliSenseBackend::LookupIntelliSenseItem(String^ Indentifier)
{
	return LookupIntelliSenseItem(Indentifier, false);
}

bool IntelliSenseBackend::IsCallableObject(IntelliSenseItem^ Item)
{
	if (Item->GetItemType() == IntelliSenseItem::eItemType::Quest)
		return safe_cast<IntelliSenseItemQuest^>(Item)->HasAttachedScript();
	else if (Item->GetItemType() == IntelliSenseItem::eItemType::Form)
	{
		auto Form = safe_cast<IntelliSenseItemForm^>(Item);

		if (!String::Compare("player", Form->GetIdentifier(), true))
			return true;		// special-case

		return Form->IsObjectReference() && Form->HasAttachedScript();
	}

	return false;
}

bool IntelliSenseBackend::IsObjectReference(IntelliSenseItem^ Item)
{
	if (Item->GetItemType() != IntelliSenseItem::eItemType::Form)
		return false;

	// special-case player
	return safe_cast<IntelliSenseItemForm^>(Item)->IsObjectReference() || !String::Compare("player", Item->GetIdentifier(), true);
}

IntelliSenseBackend::IntelliSenseBackend()
{
	DebugPrint("Initializing IntelliSense");

	ScriptCommands = gcnew IntelliSenseItemCollection;
	GameSettings = gcnew IntelliSenseItemCollection;
	Scripts = gcnew IntelliSenseItemCollection;
	GlobalVariables = gcnew IntelliSenseItemCollection;
	Quests = gcnew IntelliSenseItemCollection;
	Forms = gcnew IntelliSenseItemCollection;
	Snippets = gcnew IntelliSenseItemCollection;
	SnippetCollection = gcnew CodeSnippetCollection;

	UpdateTimer = gcnew Timer();
	UpdateTimer->Tick += gcnew EventHandler(this, &IntelliSenseBackend::UpdateTimer_Tick);
	UpdateTimer->Interval = preferences::SettingsHolder::Get()->IntelliSense->DatabaseUpdateInterval * 60 * 1000;
	UpdateTimerMode = eTimerMode::IdleQueue;

	UpdateTimer->Start();
	SnippetCollection->Load(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetSnippetCachePath()));
	RefreshCodeSnippetIntelliSenseItems();

	preferences::SettingsHolder::Get()->PreferencesChanged += gcnew System::EventHandler(this, &IntelliSenseBackend::Preferences_Saved);

	LastUpdateTimestamp = DateTime::MinValue;

	DebugPrint("\tLoaded " + SnippetCollection->LoadedSnippets->Count + " Code Snippet(s)");
}

IntelliSenseBackend::~IntelliSenseBackend()
{
	DebugPrint("Deinitializing IntelliSense");

	UpdateTimer->Stop();

	SnippetCollection->Save(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetSnippetCachePath()));
	SAFEDELETE_CLR(SnippetCollection);

	ScriptCommands->Reset();
	GameSettings->Reset();
	Scripts->Reset();
	GlobalVariables->Reset();
	Quests->Reset();
	Forms->Reset();
	Snippets->Reset();

	Singleton = nullptr;
}

void IntelliSenseBackend::InitializeScriptCommands(componentDLLInterface::CommandTableData* Data)
{
	auto DeveloperUrls = gcnew Dictionary<String^, String^>;
	for (int i = 0; i < Data->DeveloperURLDataListCount; ++i)
	{
		auto URLData = &Data->DeveloperURLDataListHead[i];
		String^ CommandName = gcnew String(URLData->CommandName);
		String^ Url = gcnew String(URLData->URL);

		DeveloperUrls->Add(CommandName, Url);
	}

	bool CommandNameCollision = false;
	int CommandCount = 0, BoundDevUrlCount = 0;
	for (auto Itr = Data->CommandTableStart; Itr != Data->CommandTableEnd; ++Itr)
	{
		auto Name = gcnew String(Itr->longName);
		if (!String::Compare(Name, "", true))
			continue;

		String^ DevUrl = String::Empty;
		if (DeveloperUrls->ContainsKey(Name))
		{
			DevUrl = DeveloperUrls[Name];
			++BoundDevUrlCount;
		}

		auto NewCommand = gcnew IntelliSenseItemScriptCommand(Data, Itr, DevUrl);

		IntelliSenseItem^ ExistingCommand = nullptr;
		if (ScriptCommands->Dict->TryGetValue(Name, ExistingCommand))
		{
#ifndef NDEBUG
			DebugPrint("\tIdentifier '" + Name + "' was bound to more than one script command");
#endif
			CommandNameCollision = true;
			continue;
		}

		ScriptCommands->Add(Name, NewCommand);
		if (NewCommand->HasAlternateIdentifier() && ScriptCommands->Lookup(NewCommand->GetAlternateIdentifier(), false) == nullptr)
			ScriptCommands->Add(NewCommand->GetAlternateIdentifier(), NewCommand);

		++CommandCount;
	}

	DebugPrint("\tParsed " + CommandCount + " Commands");
	DebugPrint("\tBound " + BoundDevUrlCount + " developer URLs");

#ifndef NDEBUG
	if (CommandNameCollision)
		DebugPrint("\tErrors were encountered while parsing the command table!", false);
#endif
}

void IntelliSenseBackend::InitializeGameSettings(componentDLLInterface::IntelliSenseUpdateData* Data)
{
	for (int i = 0; i < Data->GMSTCount; i++)
	{
		componentDLLInterface::GMSTData* GMST = &Data->GMSTListHead[i];
		if (!GMST->IsValid())
			continue;

		auto Wrapper = gcnew nativeWrapper::MarshalledVariableData(GMST);
		auto NewItem = gcnew IntelliSenseItemGameSetting(Wrapper);

		IntelliSenseItem^ Existing = nullptr;
		if (!GameSettings->Dict->TryGetValue(Wrapper->EditorId, Existing))
			GameSettings->Add(Wrapper->EditorId, NewItem);
	}

	DebugPrint(String::Format("\tParsed {0} Game Settings", Data->GMSTCount));
}

generic <typename T> where T: IntelliSenseItem
void AddIndentifierToCollection(Dictionary<String^, T>^ Source, ICollection<String^>^% Target)
{
	for each (auto% Itr in Source)
		Target->Add(Itr.Key);
}

System::Collections::Generic::ICollection<String^>^ IntelliSenseBackend::CreateIndentifierSnapshot(eDatabaseLookupFilter Categories)
{
	auto Out = gcnew HashSet<String^>(StringComparer::CurrentCultureIgnoreCase);

	if (Categories.HasFlag(eDatabaseLookupFilter::Command))
		AddIndentifierToCollection(ScriptCommands->Dict, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::GlobalVariable))
		AddIndentifierToCollection(GlobalVariables->Dict, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::Quest))
		AddIndentifierToCollection(Quests->Dict, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::Script))
		AddIndentifierToCollection(Scripts->Dict, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::UserFunction) &&
		Categories.HasFlag(eDatabaseLookupFilter::Script) == false)
	{
		for each (auto% Itr in Scripts->Dict)
		{
			if (Itr.Value->GetItemType() == IntelliSenseItem::eItemType::UserFunction)
				Out->Add(Itr.Key);
		}
	}

	if (Categories.HasFlag(eDatabaseLookupFilter::GameSetting))
		AddIndentifierToCollection(GameSettings->Dict, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::Form))
		AddIndentifierToCollection(Forms->Dict, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::ObjectReference) &&
		Categories.HasFlag(eDatabaseLookupFilter::Form) == false)
	{
		for each (auto % Itr in Forms->Dict)
		{
			if (safe_cast<IntelliSenseItemForm^>(Itr.Value)->IsObjectReference())
				Out->Add(Itr.Key);
		}
	}

	if (Categories.HasFlag(eDatabaseLookupFilter::Snippet))
		AddIndentifierToCollection(Snippets->Dict, Out);

	return Out;
}

bool IntelliSenseBackend::HasAttachedScript(String^ Identifier)
{
	return TryGetAttachedScriptData(Identifier, nullptr);
}

IntelliSenseItemScript^ IntelliSenseBackend::GetAttachedScript(String^ Identifier)
{
	auto Item = LookupIntelliSenseItem(Identifier, false);
	if (Item == nullptr)
		return nullptr;

	return GetAttachedScript(Item);
}

IntelliSenseItemScript^ IntelliSenseBackend::GetAttachedScript(IntelliSenseItem^ Item)
{
	IntelliSenseItemScript^ Result = nullptr;
	switch (Item->GetItemType())
	{
	case IntelliSenseItem::eItemType::Quest:
	case IntelliSenseItem::eItemType::Form:
	{
		auto Form = safe_cast<IntelliSenseItemForm^>(Item);
		if (Form->HasAttachedScript())
		{
			IntelliSenseItem^ Existing;
			if (!Scripts->Dict->TryGetValue(Form->GetAttachedScriptEditorID(), Existing))
				DebugPrint("Couldn't find attached script for the following form: " + Form->GetIdentifier(), true);
			else
				Result = safe_cast<IntelliSenseItemScript^>(Existing);
		}

		break;
	}
	case IntelliSenseItem::eItemType::Script:
	case IntelliSenseItem::eItemType::UserFunction:
		Result = safe_cast<IntelliSenseItemScript^>(Item);
		break;
	default:
		break;
	}

	return Result;
}

bool IntelliSenseBackend::TryGetAttachedScriptData(String^ Identifier, nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData>* OutData)
{
	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> ScriptData
	(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CString(Identifier).c_str()));

	if (ScriptData && ScriptData->IsValid())
	{
		if (OutData)
			OutData->reset(ScriptData.release());

		return true;
	}

	return false;
}

bool PredicateCommandNeedsCallingRef(IntelliSenseItem^ Command)
{
	return safe_cast<IntelliSenseItemScriptCommand^>(Command)->RequiresCallingRef();
}

bool PredicateIsUserFunction(IntelliSenseItem^ Script)
{
	return safe_cast<IntelliSenseItemScript^>(Script)->IsUserDefinedFunction();
}

bool PredicateIsObjectReference(IntelliSenseItem^ Form)
{
	return safe_cast<IntelliSenseItemForm^>(Form)->IsObjectReference();
}


IEnumerable<IntelliSenseItem^>^ DoFetchPrefixMatch(IntelliSenseItemCollection^ DataStore,
												   FetchIntelliSenseItemsArgs^ FetchArgs,
												   Func<IntelliSenseItem^, bool>^ Predicate,
												   IEnumerable<IntelliSenseItem^>^ ConcatMatchesWith)
{
	auto Count = System::Linq::Enumerable::Count(ConcatMatchesWith);
	if (Count > FetchArgs->NumItemsToFetch)
		return System::Linq::Enumerable::Take(ConcatMatchesWith, FetchArgs->NumItemsToFetch);
	else if (Count == FetchArgs->NumItemsToFetch)
		return ConcatMatchesWith;

	auto Matches = DataStore->PrefixMatch(FetchArgs->IdentifierToMatch, Predicate);
	if (ConcatMatchesWith == nullptr)
		return Matches;

	return System::Linq::Enumerable::Concat(ConcatMatchesWith, Matches);
}

IEnumerable<IntelliSenseItemCollection::FuzzyMatchT^>^ DoFetchFuzzyMatch(IntelliSenseItemCollection^ DataStore,
																		 FetchIntelliSenseItemsArgs^ FetchArgs,
																		 Func<IntelliSenseItem^, bool>^ Predicate,
																		 IEnumerable<IntelliSenseItemCollection::FuzzyMatchT^>^ ConcatMatchesWith)
{
	auto Count = System::Linq::Enumerable::Count(ConcatMatchesWith);
	if (Count > FetchArgs->NumItemsToFetch)
		return System::Linq::Enumerable::Take(ConcatMatchesWith, FetchArgs->NumItemsToFetch);
	else if (Count == FetchArgs->NumItemsToFetch)
		return ConcatMatchesWith;

	auto Matches = DataStore->FuzzyMatch(FetchArgs->IdentifierToMatch, FetchArgs->FuzzyMatchMaxCost, Predicate);
	if (ConcatMatchesWith == nullptr)
		return Matches;

	return System::Linq::Enumerable::Concat(ConcatMatchesWith, Matches);
}


IEnumerable<IntelliSenseItem^>^ IntelliSenseBackend::FetchIntelliSenseItems(FetchIntelliSenseItemsArgs^ Args)
{
	Refresh(false);

	auto PredicateScriptCommandNeedsCallingRef = gcnew Func<IntelliSenseItem^, bool>(&PredicateCommandNeedsCallingRef);
	if (!Args->Options.HasFlag(eDatabaseLookupOptions::OnlyCommandsThatNeedCallingObject))
		PredicateScriptCommandNeedsCallingRef = nullptr;

	auto PredicateScriptIsUdf = gcnew Func<IntelliSenseItem^, bool>(&PredicateIsUserFunction);
	auto PredicateFormIsRef = gcnew Func<IntelliSenseItem^, bool>(&PredicateIsObjectReference);

	switch (Args->FilterMode)
	{
	case eFilterMode::Prefix:
	{
		auto Results = System::Linq::Enumerable::Empty<IntelliSenseItem^>();

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Command))
			Results = DoFetchPrefixMatch(ScriptCommands, Args, PredicateScriptCommandNeedsCallingRef, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::GlobalVariable))
			Results = DoFetchPrefixMatch(GlobalVariables, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Quest))
			Results = DoFetchPrefixMatch(Quests, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::UserFunction) &&
			!Args->FilterBy.HasFlag(eDatabaseLookupFilter::Script))
		{
			Results = DoFetchPrefixMatch(Scripts, Args, PredicateScriptIsUdf, Results);
		}
		else if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Script))
			Results = DoFetchPrefixMatch(Scripts, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::GameSetting))
			Results = DoFetchPrefixMatch(GameSettings, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::ObjectReference) &&
			!Args->FilterBy.HasFlag(eDatabaseLookupFilter::Form))
		{
			Results = DoFetchPrefixMatch(Forms, Args, PredicateFormIsRef, Results);
		}
		else if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Form))
			Results = DoFetchPrefixMatch(Forms, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Snippet))
			Results = DoFetchPrefixMatch(Snippets, Args, nullptr, Results);

		return System::Linq::Enumerable::Take(Results, Args->NumItemsToFetch);
	}
	case eFilterMode::Fuzzy:
	{
		auto Results = System::Linq::Enumerable::Empty<IntelliSenseItemCollection::FuzzyMatchT^>();
		auto Cost = Args->FuzzyMatchMaxCost;
		Debug::Assert(Cost > 0);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Command))
			Results = DoFetchFuzzyMatch(ScriptCommands, Args, PredicateScriptCommandNeedsCallingRef, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::GlobalVariable))
			Results = DoFetchFuzzyMatch(GlobalVariables, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Quest))
			Results = DoFetchFuzzyMatch(Quests, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::UserFunction) &&
			!Args->FilterBy.HasFlag(eDatabaseLookupFilter::Script))
		{
			Results = DoFetchFuzzyMatch(Scripts, Args, PredicateScriptIsUdf, Results);
		}
		else if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Script))
			Results = DoFetchFuzzyMatch(Scripts, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::GameSetting))
			Results = DoFetchFuzzyMatch(GameSettings, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::ObjectReference) &&
			!Args->FilterBy.HasFlag(eDatabaseLookupFilter::Form))
		{
			Results = DoFetchFuzzyMatch(Forms, Args, PredicateFormIsRef, Results);
		}
		else if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Form))
			Results = DoFetchFuzzyMatch(Forms, Args, nullptr, Results);

		if (Args->FilterBy.HasFlag(eDatabaseLookupFilter::Snippet))
			Results = DoFetchFuzzyMatch(Snippets, Args, nullptr, Results);

		auto Sorted = IntelliSenseItemCollection::SortAndExtractFuzzyMatches(Results);
		return System::Linq::Enumerable::Take(Sorted, Args->NumItemsToFetch);
	}
	}

	return nullptr;
}

ContextualIntelliSenseLookupResult^ IntelliSenseBackend::ContextualIntelliSenseLookup(ContextualIntelliSenseLookupArgs^ LookupArgs)
{
	Refresh(false);

	ContextualIntelliSenseLookupResult^ Result = gcnew ContextualIntelliSenseLookupResult;

	Result->PreviousToken = LookupIntelliSenseItem(LookupArgs->PreviousToken, LookupArgs->OnlyWithInsightInfo);
	if (Result->PreviousToken)
	{
		Result->PreviousTokenIsCallableObject = IsCallableObject(Result->PreviousToken);
		Result->PreviousTokenIsObjectReference = IsObjectReference(Result->PreviousToken);

		if (Result->PreviousTokenIsCallableObject && LookupArgs->DotOperatorInUse)
		{
			auto AttachedScript = GetAttachedScript(Result->PreviousToken);
			if (AttachedScript)
			{
				auto RemoteVar = AttachedScript->LookupVariable(LookupArgs->CurrentToken);
				if (RemoteVar)
				{
					Result->CurrentToken = RemoteVar;
					Result->CurrentTokenIsCallableObject = RemoteVar->GetDataType() == obScriptParsing::Variable::eDataType::Reference;
					Result->CurrentTokenIsObjectReference = RemoteVar->GetDataType() == obScriptParsing::Variable::eDataType::Reference;
				}
			}
		}
	}

	if (Result->CurrentToken == nullptr)
	{
		Result->CurrentToken = LookupIntelliSenseItem(LookupArgs->CurrentToken, LookupArgs->OnlyWithInsightInfo);
		if (Result->CurrentToken)
		{
			Result->CurrentTokenIsCallableObject = IsCallableObject(Result->CurrentToken);
			Result->CurrentTokenIsObjectReference = IsObjectReference(Result->CurrentToken);
		}
	}

	return Result;
}

System::String^ IntelliSenseBackend::SanitizeIdentifier(String^ Identifier)
{
	auto Item = LookupIntelliSenseItem(Identifier, false);
	if (Item == nullptr)
		return Identifier;

	return Item->GetIdentifier();
}

void IntelliSenseBackend::Refresh(bool Force)
{
	if (ActiveUpdateTask)
		return;

	HandleUpdateTaskQueuing(Force);
}

void IntelliSenseBackend::ShowCodeSnippetManager()
{
	CodeSnippetManagerDialog ManagerDialog(SnippetCollection);
	RefreshCodeSnippetIntelliSenseItems();
}

IntelliSenseBackend^ IntelliSenseBackend::Get()
{
	if (Singleton == nullptr)
		Singleton = gcnew IntelliSenseBackend;

	return Singleton;
}

void IntelliSenseBackend::Deinitialize()
{
	delete Singleton;
}


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse