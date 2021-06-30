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
	MatchType = eStringMatchType::StartsWith;
	FilterBy = eDatabaseLookupFilter::All;
	Options = eDatabaseLookupOptions::None;
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

void IntelliSenseBackend::UpdateTimer_Tick(Object^ Sender, EventArgs^ E)
{
	UpdateDatabase();
}

void IntelliSenseBackend::UpdateDatabase()
{
	if (nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CanUpdateIntelliSenseDatabase() == false)
		return;

	System::Diagnostics::Stopwatch^ Profiler = gcnew System::Diagnostics::Stopwatch();
	Profiler->Start();

	try
	{
		nativeWrapper::WriteToMainWindowStatusBar(2, "Updating IntelliSense DB...");
		DisposibleDataAutoPtr<componentDLLInterface::IntelliSenseUpdateData> DataHandlerData
			(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetIntelliSenseUpdateData());

		Scripts->Clear();
		GlobalVariables->Clear();
		Quests->Clear();
		Forms->Clear();

		for (int i = 0; i < DataHandlerData->ScriptCount; ++i)
		{
			componentDLLInterface::ScriptData* Data = &DataHandlerData->ScriptListHead[i];
			if (Data->IsValid() == false)
				continue;

			String^ EditorID = gcnew String(Data->EditorID);
			IntelliSenseItemScript^ NewItem = Data->UDF ? gcnew IntelliSenseItemUserFunction(Data) : gcnew IntelliSenseItemScript(Data);

			IntelliSenseItemScript^ Existing = nullptr;
			if (!Scripts->TryGetValue(EditorID, Existing))
				Scripts->Add(EditorID, NewItem);
		}

		for (int i = 0; i < DataHandlerData->QuestCount; ++i)
		{
			componentDLLInterface::QuestData* Data = &DataHandlerData->QuestListHead[i];
			if (Data->IsValid() == false)
				continue;

			String^ EditorID = gcnew String(Data->EditorID);
			DisposibleDataAutoPtr<componentDLLInterface::ScriptData> AttachedScript
				(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CString(EditorID).c_str()));
			IntelliSenseItemQuest^ NewItem = gcnew IntelliSenseItemQuest(Data, AttachedScript.get());

			IntelliSenseItemQuest^ Existing = nullptr;
			if (!Quests->TryGetValue(EditorID, Existing))
				Quests->Add(EditorID, NewItem);
		}

		for (int i = 0; i < DataHandlerData->GlobalCount; ++i)
		{
			componentDLLInterface::GlobalData* Data = &DataHandlerData->GlobalListHead[i];
			if (Data->IsValid() == false)
				continue;

			String^ EditorID = gcnew String(Data->EditorID);
			obScriptParsing::Variable::eDataType VarType;

			switch (Data->Type)
			{
			case componentDLLInterface::GlobalData::kType_Int:
				VarType = obScriptParsing::Variable::eDataType::Integer;
				break;
			case componentDLLInterface::GlobalData::kType_Float:
				VarType = obScriptParsing::Variable::eDataType::Float;
				break;
			case componentDLLInterface::GlobalData::kType_String:
				VarType = obScriptParsing::Variable::eDataType::StringVar;
				break;
			default:
				VarType = obScriptParsing::Variable::eDataType::None;
			}

			auto NewItem = gcnew IntelliSenseItemGlobalVariable(Data, VarType, GetVariableValueString(Data));

			IntelliSenseItemGlobalVariable^ Existing = nullptr;
			if (!GlobalVariables->TryGetValue(EditorID, Existing))
				GlobalVariables->Add(EditorID, NewItem);
		}

		for (int i = 0; i < DataHandlerData->MiscFormListCount; ++i)
		{
			componentDLLInterface::FormData* Data = &DataHandlerData->MiscFormListHead[i];
			if (Data->IsValid() == false)
				continue;

			String^ EditorID = gcnew String(Data->EditorID);
			DisposibleDataAutoPtr<componentDLLInterface::ScriptData> AttachedScript
				(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CString(EditorID).c_str()));

			IntelliSenseItemForm^ NewItem = gcnew IntelliSenseItemForm(Data, AttachedScript.get());

			IntelliSenseItemForm^ Existing = nullptr;
			if (!Forms->TryGetValue(EditorID, Existing))
				Forms->Add(EditorID, NewItem);
		}

		nativeWrapper::WriteToMainWindowStatusBar(2, "IntelliSense DB updated.");
		nativeWrapper::WriteToMainWindowStatusBar(3, "[" +
			Profiler->ElapsedMilliseconds.ToString() + "ms | " +
			DataHandlerData->ScriptCount + " Script(s) | " +
			DataHandlerData->QuestCount + " Quest(s) | " +
			DataHandlerData->GlobalCount + " Global(s) | " +
			DataHandlerData->MiscFormListCount + " Forms" +
			"]");
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't update IntelliSense DB!\n\tException: " + E->Message, true);
		nativeWrapper::WriteToMainWindowStatusBar(2, "IntelliSense DB Error!");
	}

	Profiler->Stop();

	LastUpdateTimestamp = DateTime::Now;
}

System::String^ IntelliSenseBackend::GetVariableValueString(componentDLLInterface::VariableData* Data)
{
	switch (Data->Type)
	{
	case componentDLLInterface::GlobalData::kType_Int:
		return Data->Value.i.ToString();
	case componentDLLInterface::GlobalData::kType_Float:
		return Data->Value.f.ToString();
	case componentDLLInterface::GlobalData::kType_String:
		return gcnew String(Data->Value.s);
	default:
		return String::Empty;
	}
}

void IntelliSenseBackend::RefreshCodeSnippetIntelliSenseItems()
{
	Snippets->Clear();

	for each (CodeSnippet ^ Itr in SnippetCollection->LoadedSnippets)
		Snippets->Add(Itr->Name, gcnew IntelliSenseItemCodeSnippet(Itr));
}

generic <typename T> where T : IntelliSenseItem
void IntelliSenseBackend::DoFetch(Dictionary<String^, T>^% Source, FetchIntelliSenseItemsArgs^ Args, List<IntelliSenseItem^>^% OutFetched)
{
	if (Args->IdentifierToMatch->Length == 0)
	{
		OutFetched->AddRange(safe_cast<Collections::Generic::IEnumerable<IntelliSenseItem^>^>(Source->Values));
		return;
	}

	for each (T Itr in Source->Values)
	{
		if (Itr->MatchesToken(Args->IdentifierToMatch, Args->MatchType))
			OutFetched->Add(Itr);
	}
}

IntelliSenseItem^ IntelliSenseBackend::LookupIntelliSenseItem(String^ Identifier, bool OnlyWithInsightInfo)
{
	IntelliSenseItem^ Out = nullptr;
	if (Identifier == String::Empty)
		return Out;

	if (ScriptCommands->ContainsKey(Identifier))
	{
		Out = ScriptCommands[Identifier];
		if (OnlyWithInsightInfo == false || Out->HasInsightInfo())
			goto exit;
		else
			Out = nullptr;
	}

	if (GameSettings->ContainsKey(Identifier))
	{
		Out = GameSettings[Identifier];
		if (OnlyWithInsightInfo == false || Out->HasInsightInfo())
			goto exit;
		else
			Out = nullptr;
	}

	if (Scripts->ContainsKey(Identifier))
	{
		Out = Scripts[Identifier];
		if (OnlyWithInsightInfo == false || Out->HasInsightInfo())
			goto exit;
		else
			Out = nullptr;
	}

	if (GlobalVariables->ContainsKey(Identifier))
	{
		Out = GlobalVariables[Identifier];
		if (OnlyWithInsightInfo == false || Out->HasInsightInfo())
			goto exit;
		else
			Out = nullptr;
	}

	if (Quests->ContainsKey(Identifier))
	{
		Out = Quests[Identifier];
		if (OnlyWithInsightInfo == false || Out->HasInsightInfo())
			goto exit;
		else
			Out = nullptr;
	}

	if (Forms->ContainsKey(Identifier))
	{
		Out = Forms[Identifier];
		if (OnlyWithInsightInfo == false || Out->HasInsightInfo())
			goto exit;
		else
			Out = nullptr;
	}

	if (Snippets->ContainsKey(Identifier))
	{
		Out = Snippets[Identifier];
		if (OnlyWithInsightInfo == false || Out->HasInsightInfo())
			goto exit;
		else
			Out = nullptr;
	}

	exit:
	return Out;
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

		if (Form->MatchesToken("player", eStringMatchType::FullMatch))
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
	return safe_cast<IntelliSenseItemForm^>(Item)->IsObjectReference() || Item->MatchesToken("player", eStringMatchType::FullMatch);
}

IntelliSenseBackend::IntelliSenseBackend()
{
	DebugPrint("Initializing IntelliSense");

	ScriptCommands = gcnew Dictionary<String ^, IntelliSenseItemScriptCommand ^>(StringComparer::CurrentCultureIgnoreCase);
	GameSettings = gcnew Dictionary<String ^, IntelliSenseItemGameSetting ^>(StringComparer::CurrentCultureIgnoreCase);
	Scripts = gcnew Dictionary<String ^, IntelliSenseItemScript ^>(StringComparer::CurrentCultureIgnoreCase);
	GlobalVariables = gcnew Dictionary<String ^, IntelliSenseItemGlobalVariable^>(StringComparer::CurrentCultureIgnoreCase);
	Quests = gcnew Dictionary<String ^, IntelliSenseItemQuest ^>(StringComparer::CurrentCultureIgnoreCase);
	Forms = gcnew Dictionary<String^, IntelliSenseItemForm^>(StringComparer::CurrentCultureIgnoreCase);
	Snippets = gcnew Dictionary<String^, IntelliSenseItemCodeSnippet^>(StringComparer::CurrentCultureIgnoreCase);
	SnippetCollection = gcnew CodeSnippetCollection;

	UInt32 UpdateTimerInterval = preferences::SettingsHolder::Get()->IntelliSense->DatabaseUpdateInterval;

	UpdateTimer = gcnew Timer();
	UpdateTimer->Tick += gcnew EventHandler(this, &IntelliSenseBackend::UpdateTimer_Tick);
	UpdateTimer->Interval = UpdateTimerInterval * 60 * 1000;

	UpdateTimer->Start();
	SnippetCollection->Load(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetSnippetCachePath()));
	RefreshCodeSnippetIntelliSenseItems();

	LastUpdateTimestamp = DateTime::MinValue;

	DebugPrint("\tLoaded " + SnippetCollection->LoadedSnippets->Count + " Code Snippet(s)");
}

IntelliSenseBackend::~IntelliSenseBackend()
{
	DebugPrint("Deinitializing IntelliSense");

	UpdateTimer->Stop();

	SnippetCollection->Save(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetSnippetCachePath()));
	SAFEDELETE_CLR(SnippetCollection);

	ScriptCommands->Clear();
	GameSettings->Clear();
	Scripts->Clear();
	GlobalVariables->Clear();
	Quests->Clear();
	Forms->Clear();

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

		IntelliSenseItemScriptCommand^ ExistingCommand = nullptr;
		if (ScriptCommands->TryGetValue(Name, ExistingCommand))
		{
#ifndef NDEBUG
			DebugPrint("\tIdentifier '" + Name + "' was bound to more than one script command");
#endif
			CommandNameCollision = true;
			continue;
		}

		ScriptCommands->Add(Name, NewCommand);
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

		String^ EditorID = gcnew String(GMST->EditorID);
		obScriptParsing::Variable::eDataType VarType;

		switch (GMST->Type)
		{
		case componentDLLInterface::GlobalData::kType_Int:
			VarType = obScriptParsing::Variable::eDataType::Integer;
			break;
		case componentDLLInterface::GlobalData::kType_Float:
			VarType = obScriptParsing::Variable::eDataType::Float;
			break;
		case componentDLLInterface::GlobalData::kType_String:
			VarType = obScriptParsing::Variable::eDataType::StringVar;
			break;
		default:
			VarType = obScriptParsing::Variable::eDataType::None;
		}

		auto NewItem = gcnew IntelliSenseItemGameSetting(GMST, VarType, GetVariableValueString(GMST));

		IntelliSenseItemGameSetting^ Existing = nullptr;
		if (!GameSettings->TryGetValue(EditorID, Existing))
			GameSettings->Add(EditorID, NewItem);
	}

	DebugPrint(String::Format("\tParsed {0} Game Settings", Data->GMSTCount));
}

System::String^ IntelliSenseBackend::GetScriptCommandDeveloperURL(String^ CommandName)
{
	IntelliSenseItemScriptCommand^ Command = nullptr;
	if (ScriptCommands->TryGetValue(CommandName, Command) == false)
		return String::Empty;

	return Command->GetDocumentationUrl();
}

bool IntelliSenseBackend::IsUserFunction(String^ Identifier)
{
	IntelliSenseItemScript^ Script = nullptr;
	if (Scripts->TryGetValue(Identifier, Script) == false)
		return false;

	return Script->GetItemType() == IntelliSenseItem::eItemType::UserFunction;
}

bool IntelliSenseBackend::IsScriptCommand(String^ Identifier, bool CheckCommandShorthand)
{
	if (CheckCommandShorthand == false)
		return ScriptCommands->ContainsKey(Identifier);

	for each (IntelliSenseItemScriptCommand ^ Itr in ScriptCommands->Values)
	{
		if (Identifier->Equals(Itr->GetShorthand(), System::StringComparison::CurrentCultureIgnoreCase))
			return true;
	}

	return false;
}

bool IntelliSenseBackend::IsForm(String^ Identifier)
{
	DisposibleDataAutoPtr<componentDLLInterface::FormData> FormData
	(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupFormByEditorID(CString(Identifier).c_str()));

	return FormData && FormData->IsValid();
}

generic <typename T> where T: IntelliSenseItem
void AddIndentifierToCollection(Dictionary<String^, T>^ Source, ICollection<String^>^% Target)
{
	for each (auto% Itr in Source)
		Target->Add(Itr.Key);
}

System::Collections::Generic::HashSet<String^>^ IntelliSenseBackend::CreateIndentifierSnapshot(eDatabaseLookupFilter Categories)
{
	auto Out = gcnew HashSet<String^>(StringComparer::CurrentCultureIgnoreCase);

	if (Categories.HasFlag(eDatabaseLookupFilter::Command))
		AddIndentifierToCollection(ScriptCommands, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::GlobalVariable))
		AddIndentifierToCollection(GlobalVariables, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::Quest))
		AddIndentifierToCollection(Quests, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::Script))
		AddIndentifierToCollection(Scripts, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::UserFunction) &&
		Categories.HasFlag(eDatabaseLookupFilter::Script) == false)
	{
		for each (auto% Itr in Scripts)
		{
			if (Itr.Value->GetItemType() == IntelliSenseItem::eItemType::UserFunction)
				Out->Add(Itr.Key);
		}
	}

	if (Categories.HasFlag(eDatabaseLookupFilter::GameSetting))
		AddIndentifierToCollection(GameSettings, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::Form))
		AddIndentifierToCollection(Forms, Out);

	if (Categories.HasFlag(eDatabaseLookupFilter::ObjectReference) &&
		Categories.HasFlag(eDatabaseLookupFilter::Form) == false)
	{
		for each (auto % Itr in Forms)
		{
			if (safe_cast<IntelliSenseItemForm^>(Itr.Value)->IsObjectReference())
				Out->Add(Itr.Key);
		}
	}

	if (Categories.HasFlag(eDatabaseLookupFilter::Snippet))
		AddIndentifierToCollection(Snippets, Out);

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
			if (Scripts->TryGetValue(Form->GetAttachedScriptEditorID(), Result) == false)
				DebugPrint("Couldn't find attached script for the following form: " + Form->GetIdentifier(), true);
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

bool IntelliSenseBackend::TryGetAttachedScriptData(String^ Identifier, DisposibleDataAutoPtr<componentDLLInterface::ScriptData>* OutData)
{
	DisposibleDataAutoPtr<componentDLLInterface::ScriptData> ScriptData
	(nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CString(Identifier).c_str()));

	if (ScriptData && ScriptData->IsValid())
	{
		if (OutData)
			OutData->reset(ScriptData.release());

		return true;
	}

	return false;
}

List<IntelliSenseItem^>^ IntelliSenseBackend::FetchIntelliSenseItems(FetchIntelliSenseItemsArgs^ FetchArgs)
{
	Refresh(false);

	List<IntelliSenseItem^>^ Fetched = gcnew List<IntelliSenseItem ^>;

	if (FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::Command))
	{
		List<IntelliSenseItem^>^ AllCommands = gcnew List<IntelliSenseItem^>;
		DoFetch(ScriptCommands, FetchArgs, AllCommands);

		if (!FetchArgs->Options.HasFlag(eDatabaseLookupOptions::OnlyCommandsThatNeedCallingObject))
			Fetched = AllCommands;
		else for each (IntelliSenseItem ^ Itr in AllCommands)
		{
			if (safe_cast<IntelliSenseItemScriptCommand^>(Itr)->RequiresCallingRef())
				Fetched->Add(Itr);
		}
	}

	if (FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::GlobalVariable))
		DoFetch(GlobalVariables, FetchArgs, Fetched);

	if (FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::Quest))
		DoFetch(Quests, FetchArgs, Fetched);

	if (FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::Script))
		DoFetch(Scripts, FetchArgs, Fetched);

	if (FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::UserFunction) &&
		FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::Script) == false)
	{
		List<IntelliSenseItem^>^ AllScriptTypes = gcnew List<IntelliSenseItem^>;
		DoFetch(Scripts, FetchArgs, AllScriptTypes);

		for each (IntelliSenseItem ^ Script in AllScriptTypes)
		{
			if (Script->GetItemType() == IntelliSenseItem::eItemType::UserFunction)
				Fetched->Add(Script);
		}
	}

	if (FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::GameSetting))
		DoFetch(GameSettings, FetchArgs, Fetched);

	if (FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::Form))
		DoFetch(Forms, FetchArgs, Fetched);

	if (FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::ObjectReference) &&
		FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::Form) == false)
	{
		List<IntelliSenseItem^>^ AllForms = gcnew List<IntelliSenseItem^>;
		DoFetch(Forms, FetchArgs, AllForms);

		for each (IntelliSenseItem ^ Form in AllForms)
		{
			if (safe_cast<IntelliSenseItemForm^>(Form)->IsObjectReference())
				Fetched->Add(Form);
		}
	}

	if (FetchArgs->FilterBy.HasFlag(eDatabaseLookupFilter::Snippet))
		DoFetch(Snippets, FetchArgs, Fetched);

	return Fetched;
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
					Result->CurrentTokenIsCallableObject = RemoteVar->GetDataType() == obScriptParsing::Variable::eDataType::Ref;
					Result->CurrentTokenIsObjectReference = RemoteVar->GetDataType() == obScriptParsing::Variable::eDataType::Ref;
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
	auto ElapsedTimeSinceLastUpdate = DateTime::Now - LastUpdateTime;
	if (Force || ElapsedTimeSinceLastUpdate.TotalMilliseconds > UpdateTimer->Interval)
		UpdateDatabase();
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