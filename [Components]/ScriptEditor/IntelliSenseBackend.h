#pragma once
#include "IntelliSenseItem.h"
#include "Utilities.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


ref struct CodeSnippetCollection;

[Flags]
static enum class eDatabaseLookupFilter
{
	None = 0,

	Command = 1 << 0,
	GlobalVariable = 1 << 1,
	Quest = 1 << 2,
	Script = 1 << 3,
	UserFunction = 1 << 4,
	GameSetting = 1 << 5,
	Form = 1 << 6,
	ObjectReference = 1 << 7,
	Snippet = 1 << 8,

	All = Command | GlobalVariable | Quest | Script | UserFunction | GameSetting | Form | ObjectReference | Snippet
};

[Flags]
static enum class eDatabaseLookupOptions
{
	None = 0,

	OnlyCommandsThatNeedCallingObject = 1 << 0,
};


public enum class eFilterMode
{
	Prefix,
	Fuzzy
};

ref struct FetchIntelliSenseItemsArgs
{
	property String^ IdentifierToMatch;
	property eFilterMode FilterMode;
	property eDatabaseLookupFilter FilterBy;
	property eDatabaseLookupOptions Options;
	property UInt32 FuzzyMatchMaxCost;
	property int NumItemsToFetch;

	FetchIntelliSenseItemsArgs();
};

ref struct ContextualIntelliSenseLookupArgs
{
	property String^ CurrentToken;
	property String^ PreviousToken;
	property bool DotOperatorInUse;
	property bool OnlyWithInsightInfo;

	ContextualIntelliSenseLookupArgs();
};

ref struct ContextualIntelliSenseLookupResult
{
	property IntelliSenseItem^ CurrentToken;
	property bool CurrentTokenIsCallableObject;	// Callable Object = Quests, References (w/t attached scripts)
	property bool CurrentTokenIsObjectReference;

	property IntelliSenseItem^ PreviousToken;
	property bool PreviousTokenIsCallableObject;
	property bool PreviousTokenIsObjectReference;

	ContextualIntelliSenseLookupResult();
};


// memory is cheap (for our use case, anyway), so we store both a case-insensitive hashmap and a radix trie
// for constant time lookup of individual identifiers and fast prefix/fuzzy matching respectively
ref struct IntelliSenseItemCollection
{
	using FuzzyMatchT = utilities::CaselessFuzzyTrie<IntelliSenseItem^>::FuzzyMatchResult;
private:
	static int FuzzyMatchResultSortKeySelector(FuzzyMatchT^ Item);
	static IntelliSenseItem^ FuzzyMatchResultMapToItemSelector(FuzzyMatchT^ Item);

	ref class PrefixToEditDistanceMatcher
	{
		int QueryLength;
		int MinimumEditDistance;
	public:
		PrefixToEditDistanceMatcher(String^ Query, int LevenshteinMaxEditDistance);

		FuzzyMatchT^ CreateFuzzyMatch(IntelliSenseItem^ Item);
		bool FuzzyMatchHasMinimumEditDistance(FuzzyMatchT^ Match);
	};

	Dictionary<String^, IntelliSenseItem^>^ Dict_;
	utilities::CaselessFuzzyTrie<IntelliSenseItem^>^ Trie_;
public:
	IntelliSenseItemCollection();

	property Dictionary<String^, IntelliSenseItem^>^ Dict
	{
		Dictionary<String^, IntelliSenseItem^>^ get() { return Dict_; }
	}

	property utilities::CaselessFuzzyTrie<IntelliSenseItem^>^ Trie
	{
		utilities::CaselessFuzzyTrie<IntelliSenseItem^>^ get() { return Trie_; }
	}

	property UInt32 Count
	{
		UInt32 get() { return Dict->Count; }
	}

	void Add(String^ Identifier, IntelliSenseItem^ Item);
	IEnumerable<FuzzyMatchT^>^ LevenshteinMatch(String^ Query, UInt32 MaxEditDistanceCost, System::Func<IntelliSenseItem^, bool>^ Predicate);
	IEnumerable<IntelliSenseItem^>^ PrefixMatch(String^ Query, System::Func<IntelliSenseItem^, bool>^ Predicate);
	IEnumerable<FuzzyMatchT^>^ FuzzyMatch(String^ Query, UInt32 MaxEditDistanceCost, System::Func<IntelliSenseItem^, bool>^ Predicate);
	IntelliSenseItem^ Lookup(String^ Identifier, bool IgnoreItemsWithoutInsightInfo);
	void Reset();

	static IEnumerable<IntelliSenseItem^>^ SortAndExtractFuzzyMatches(IEnumerable<FuzzyMatchT^>^ FuzzyMatches);
};


ref class IntelliSenseBackend
{
	ref struct UpdateTaskPayload
	{
		System::Diagnostics::Stopwatch^ Stopwatch;

		List<nativeWrapper::MarshalledScriptData^>^ IncomingScripts;
		List<nativeWrapper::MarshalledFormData^>^ IncomingQuests;
		List<nativeWrapper::MarshalledVariableData^>^ IncomingGlobals;
		List<nativeWrapper::MarshalledFormData^>^ IncomingForms;

		IntelliSenseItemCollection^ OutgoingScripts;
		IntelliSenseItemCollection^ OutgoingGlobalVariables;
		IntelliSenseItemCollection^ OutgoingQuests;
		IntelliSenseItemCollection^ OutgoingForms;

		UpdateTaskPayload();
	};

	static IntelliSenseBackend^ Singleton = nullptr;

	static const UInt32 kTimerPollInterval = 100;	// in ms

	static enum class eTimerMode
	{
		IdlePoll,
		IdleQueue,
	};

	using UpdateTaskT = System::Threading::Tasks::Task<UpdateTaskPayload^>;

	Timer^ UpdateTimer;
	eTimerMode UpdateTimerMode;
	DateTime LastUpdateTimestamp;
	UpdateTaskT^ ActiveUpdateTask;

	IntelliSenseItemCollection^ ScriptCommands;
	IntelliSenseItemCollection^ GameSettings;
	IntelliSenseItemCollection^ Scripts;
	IntelliSenseItemCollection^ GlobalVariables;
	IntelliSenseItemCollection^ Quests;
	IntelliSenseItemCollection^ Forms;
	IntelliSenseItemCollection^ Snippets;
	CodeSnippetCollection^ SnippetCollection;

	void Preferences_Saved(Object^ Sender, EventArgs^ E);
	void UpdateTimer_Tick(Object^ Sender, EventArgs^ E);

	static UpdateTaskPayload^ PerformBackgroundUpdateTask(Object^ Input);
	bool HandleActiveUpdateTaskPolling();
	bool HandleUpdateTaskQueuing(bool Force);
	void SetUpdateTimerMode(eTimerMode Mode);
	UpdateTaskPayload^ GenerateUpdateTaskPayload(componentDLLInterface::IntelliSenseUpdateData* NativeData);

	void RefreshCodeSnippetIntelliSenseItems();

	IntelliSenseItem^ LookupIntelliSenseItem(String^ Identifier, bool OnlyWithInsightInfo);
	bool IsCallableObject(IntelliSenseItem^ Item);
	bool IsObjectReference(IntelliSenseItem^ Item);
	bool TryGetAttachedScriptData(String^ Identifier, nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData>* OutData);
	IntelliSenseItem^ LookupIntelliSenseItem(String^ Indentifier);

	IntelliSenseBackend();
public:
	~IntelliSenseBackend();

	property DateTime LastUpdateTime
	{
		DateTime get() { return LastUpdateTimestamp; }
	}

	void InitializeScriptCommands(componentDLLInterface::CommandTableData* Data);
	void InitializeGameSettings(componentDLLInterface::IntelliSenseUpdateData* Data);

	ICollection<String^>^ CreateIndentifierSnapshot(eDatabaseLookupFilter Categories);
	String^ SanitizeIdentifier(String^ Identifier);

	bool HasAttachedScript(String^ Identifier);
	IntelliSenseItemScript^ GetAttachedScript(String^ Identifier);
	IntelliSenseItemScript^ GetAttachedScript(IntelliSenseItem^ Item);

	IEnumerable<IntelliSenseItem^>^ FetchIntelliSenseItems(FetchIntelliSenseItemsArgs^ FetchArgs);
	ContextualIntelliSenseLookupResult^ ContextualIntelliSenseLookup(ContextualIntelliSenseLookupArgs^ LookupArgs);

	void Refresh(bool Force);
	void ShowCodeSnippetManager();

	static IntelliSenseBackend^ Get();
	static void Deinitialize();
};


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse