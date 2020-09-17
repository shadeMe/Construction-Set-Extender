#include "IntelliSenseItem.h"
#include "IntelliSenseDatabase.h"

#include "SemanticAnalysis.h"
#include "Preferences.h"
#include "SnippetManager.h"

#include "[Common]\NativeWrapper.h"

namespace cse
{
	namespace intellisense
	{
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
					IntelliSenseItemQuest^ NewItem = gcnew IntelliSenseItemQuest(Data, AttachedScript.get(), gcnew String(Data->FullName));

					Quests->Add(EditorID, NewItem);
				}

				for (int i = 0; i < DataHandlerData->GlobalCount; ++i)
				{
					componentDLLInterface::GlobalData* Data = &DataHandlerData->GlobalListHead[i];
					if (Data->IsValid() == false)
						continue;

					String^ EditorID = gcnew String(Data->EditorID);
					obScriptParsing::Variable::DataType VarType;

					switch (Data->Type)
					{
					case componentDLLInterface::GlobalData::kType_Int:
						VarType = obScriptParsing::Variable::DataType::Integer;
						break;
					case componentDLLInterface::GlobalData::kType_Float:
						VarType = obScriptParsing::Variable::DataType::Float;
						break;
					case componentDLLInterface::GlobalData::kType_String:
						VarType = obScriptParsing::Variable::DataType::StringVar;
						break;
					default:
						VarType = obScriptParsing::Variable::DataType::None;
					}

					auto NewItem = gcnew IntelliSenseItemGlobalVariable(Data, VarType, GetVariableValueString(Data));
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
			if (Item->GetItemType() == IntelliSenseItem::ItemType::Quest)
				return safe_cast<IntelliSenseItemQuest^>(Item)->HasAttachedScript();
			else if (Item->GetItemType() == IntelliSenseItem::ItemType::Form)
			{
				auto Form = safe_cast<IntelliSenseItemForm^>(Item);

				if (Form->MatchesToken("player", StringMatchType::FullMatch))
					return true;		// special-case

				return Form->IsObjectReference() && Form->HasAttachedScript();
			}

			return false;
		}

		bool IntelliSenseBackend::IsObjectReference(IntelliSenseItem^ Item)
		{
			if (Item->GetItemType() != IntelliSenseItem::ItemType::Form)
				return false;

			// special-case player
			return safe_cast<IntelliSenseItemForm^>(Item)->IsObjectReference() || Item->MatchesToken("player", StringMatchType::FullMatch);
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
			String^ Name;
			String^ Description;
			String^ ShortHand;
			String^ PluginName;
			int Count = 0, ReturnType = 0, CSCount = 0;
			IntelliSenseItemScriptCommand::SourceType Source;
			System::Globalization::TextInfo^ Locale = (gcnew System::Globalization::CultureInfo("en-US", false))->TextInfo;
			bool NameCollision = false;

			for (const componentDLLInterface::ObScriptCommandInfo* Itr = Data->CommandTableStart; Itr != Data->CommandTableEnd; ++Itr)
			{
				Name = gcnew String(Itr->longName);
				if (!String::Compare(Name, "", true))
					continue;

				const componentDLLInterface::CommandTableData::PluginInfo* Info = Data->GetParentPlugin(Itr);

				if (CSCount < 370)
				{
					Description = "[CS] ";				// 369 vanilla commands
					Source = IntelliSenseItemScriptCommand::SourceType::Vanilla;
				}
				else if (Info)
				{
					PluginName = gcnew String(Info->name);
					if (!String::Compare(PluginName, "OBSE_Kyoma_MenuQue", true))
						PluginName = "MenuQue";
					else if (!String::Compare(PluginName, "OBSE_Elys_Pluggy", true))
						PluginName = "Pluggy";

					Description = "[" + PluginName + " v" + Info->version + "] ";
					Source = IntelliSenseItemScriptCommand::SourceType::OBSE;
				}
				else
				{
					UInt32 OBSEVersion = Data->GetRequiredOBSEVersion(Itr);
					Description = "[OBSE v" + OBSEVersion + "] ";
					Source = IntelliSenseItemScriptCommand::SourceType::OBSE;
				}

				if (!String::Compare(gcnew String(Itr->helpText), "", true))
					Description += "No description";
				else
					Description += gcnew String(Itr->helpText);

				if (!String::Compare(gcnew String(Itr->shortName), "", true))
					ShortHand = "None";
				else
					ShortHand = gcnew String(Itr->shortName);

				ReturnType = Data->GetCommandReturnType(Itr);
				if (ReturnType == 6)
					ReturnType = 0;

				String^ Params = "";
				for (int i = 0; i < Itr->numParams; i++)
				{
					if (i == 0)
						Params += "\n";

					componentDLLInterface::ObScriptCommandInfo::ParamInfo* Param = &Itr->params[i];
					if (Param)
					{
						Params += "\t" + Locale->ToTitleCase(gcnew String((Param->typeStr ? Param->typeStr : "")) +
							" [" + gcnew String(Param->TypeIDString()) + "]" +
							(Param->isOptional ? " (Optional) " : "") + "\n");
					}
				}

				IntelliSenseItemScriptCommand^ NewCommand = gcnew IntelliSenseItemScriptCommand(Name, Description, ShortHand,
																Itr->numParams, Itr->needsParent, ReturnType, Source, Params, String::Empty);

				IntelliSenseItemScriptCommand^ ExistingCommand = nullptr;
				if (ScriptCommands->TryGetValue(Name, ExistingCommand))
				{
#ifndef NDEBUG
					DebugPrint("\tIdentifier '" + Name + "' was bound to more than one script command");
					DebugPrint("\t\tExisting Command: " + ExistingCommand->Describe());
					DebugPrint("\t\tNew Command: " + NewCommand->Describe());
#endif
					NameCollision = true;
					continue;
				}

				ScriptCommands->Add(Name, NewCommand);
				CSCount++;
				Count++;
			}

			DebugPrint(String::Format("\tParsed {0} Commands", Count));

#ifndef NDEBUG
			if (NameCollision)
				DebugPrint("\tErrors were encountered while parsing the command table!", false);
#endif

			int BoundDevURLCount = 0;
			for (int i = 0; i < Data->DeveloperURLDataListCount; ++i)
			{
				auto URLData = &Data->DeveloperURLDataListHead[i];
				String^ CommandName = gcnew String(URLData->CommandName);
				String^ URL= gcnew String(URLData->URL);

				IntelliSenseItemScriptCommand^ Command = nullptr;
				if (ScriptCommands->TryGetValue(CommandName, Command) == false)
				{
					DebugPrint("\tCouldn't bind developer URL '" + URL + "' to unknown script command '" + CommandName + "'!");
					continue;
				}

				Command->SetDeveloperURL(URL);
				++BoundDevURLCount;
			}

			DebugPrint("\tBound " + BoundDevURLCount + " developer URLs");
		}

		void IntelliSenseBackend::InitializeGameSettings(componentDLLInterface::IntelliSenseUpdateData* Data)
		{
			for (int i = 0; i < Data->GMSTCount; i++)
			{
				componentDLLInterface::GMSTData* GMST = &Data->GMSTListHead[i];
				if (!GMST->IsValid())
					continue;

				String^ EditorID = gcnew String(GMST->EditorID);
				obScriptParsing::Variable::DataType VarType;

				switch (GMST->Type)
				{
				case componentDLLInterface::GlobalData::kType_Int:
					VarType = obScriptParsing::Variable::DataType::Integer;
					break;
				case componentDLLInterface::GlobalData::kType_Float:
					VarType = obScriptParsing::Variable::DataType::Float;
					break;
				case componentDLLInterface::GlobalData::kType_String:
					VarType = obScriptParsing::Variable::DataType::StringVar;
					break;
				default:
					VarType = obScriptParsing::Variable::DataType::None;
				}

				auto NewItem = gcnew IntelliSenseItemGameSetting(GMST, VarType, GetVariableValueString(GMST));
				GameSettings->Add(EditorID, NewItem);
			}

			DebugPrint(String::Format("\tParsed {0} Game Settings", Data->GMSTCount));
		}

		System::String^ IntelliSenseBackend::GetScriptCommandDeveloperURL(String^ CommandName)
		{
			IntelliSenseItemScriptCommand^ Command = nullptr;
			if (ScriptCommands->TryGetValue(CommandName, Command) == false)
				return String::Empty;

			return Command->GetDeveloperURL();
		}

		bool IntelliSenseBackend::IsUserFunction(String^ Identifier)
		{
			IntelliSenseItemScript^ Script = nullptr;
			if (Scripts->TryGetValue(Identifier, Script) == false)
				return false;

			return Script->GetItemType() == IntelliSenseItem::ItemType::UserFunction;
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

		System::Collections::Generic::HashSet<String^>^ IntelliSenseBackend::CreateIndentifierSnapshot(DatabaseLookupFilter Categories)
		{
			auto Out = gcnew HashSet<String^>(StringComparer::CurrentCultureIgnoreCase);

			if (Categories.HasFlag(DatabaseLookupFilter::Command))
				AddIndentifierToCollection(ScriptCommands, Out);

			if (Categories.HasFlag(DatabaseLookupFilter::GlobalVariable))
				AddIndentifierToCollection(GlobalVariables, Out);

			if (Categories.HasFlag(DatabaseLookupFilter::Quest))
				AddIndentifierToCollection(Quests, Out);

			if (Categories.HasFlag(DatabaseLookupFilter::Script))
				AddIndentifierToCollection(Scripts, Out);

			if (Categories.HasFlag(DatabaseLookupFilter::UserFunction) &&
				Categories.HasFlag(DatabaseLookupFilter::Script) == false)
			{
				for each (auto% Itr in Scripts)
				{
					if (Itr.Value->GetItemType() == IntelliSenseItem::ItemType::UserFunction)
						Out->Add(Itr.Key);
				}
			}

			if (Categories.HasFlag(DatabaseLookupFilter::GameSetting))
				AddIndentifierToCollection(GameSettings, Out);

			if (Categories.HasFlag(DatabaseLookupFilter::Form))
				AddIndentifierToCollection(Forms, Out);

			if (Categories.HasFlag(DatabaseLookupFilter::ObjectReference) &&
				Categories.HasFlag(DatabaseLookupFilter::Form) == false)
			{
				for each (auto % Itr in Forms)
				{
					if (safe_cast<IntelliSenseItemForm^>(Itr.Value)->IsObjectReference())
						Out->Add(Itr.Key);
				}
			}

			if (Categories.HasFlag(DatabaseLookupFilter::Snippet))
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
			case IntelliSenseItem::ItemType::Quest:
			case IntelliSenseItem::ItemType::Form:
			{
				auto Form = safe_cast<IntelliSenseItemForm^>(Item);
				if (Form->HasAttachedScript())
				{
					if (Scripts->TryGetValue(Form->GetAttachedScriptEditorID(), Result) == false)
						DebugPrint("Coldn't find attached script for the following form:\n" + Form->Describe(), true);
				}

				break;
			}
			case IntelliSenseItem::ItemType::Script:
			case IntelliSenseItem::ItemType::UserFunction:
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
			List<IntelliSenseItem^>^ Fetched = gcnew List<IntelliSenseItem ^>;

			if (FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::Command))
			{
				List<IntelliSenseItem^>^ AllCommands = gcnew List<IntelliSenseItem^>;
				DoFetch(ScriptCommands, FetchArgs, AllCommands);

				if (!FetchArgs->Options.HasFlag(DatabaseLookupOptions::OnlyCommandsThatNeedCallingObject))
					Fetched = AllCommands;
				else for each (IntelliSenseItem ^ Itr in AllCommands)
				{
					if (safe_cast<IntelliSenseItemScriptCommand^>(Itr)->GetRequiresParent())
						Fetched->Add(Itr);
				}
			}

			if (FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::GlobalVariable))
				DoFetch(GlobalVariables, FetchArgs, Fetched);

			if (FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::Quest))
				DoFetch(Quests, FetchArgs, Fetched);

			if (FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::Script))
				DoFetch(Scripts, FetchArgs, Fetched);

			if (FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::UserFunction) &&
				FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::Script) == false)
			{
				List<IntelliSenseItem^>^ AllScriptTypes = gcnew List<IntelliSenseItem^>;
				DoFetch(Scripts, FetchArgs, AllScriptTypes);

				for each (IntelliSenseItem ^ Script in AllScriptTypes)
				{
					if (Script->GetItemType() == IntelliSenseItem::ItemType::UserFunction)
						Fetched->Add(Script);
				}
			}

			if (FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::GameSetting))
				DoFetch(GameSettings, FetchArgs, Fetched);

			if (FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::Form))
				DoFetch(Forms, FetchArgs, Fetched);

			if (FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::ObjectReference) &&
				FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::Form) == false)
			{
				List<IntelliSenseItem^>^ AllForms = gcnew List<IntelliSenseItem^>;
				DoFetch(Forms, FetchArgs, AllForms);

				for each (IntelliSenseItem ^ Form in AllForms)
				{
					if (safe_cast<IntelliSenseItemForm^>(Form)->IsObjectReference())
						Fetched->Add(Form);
				}
			}

			if (FetchArgs->FilterBy.HasFlag(DatabaseLookupFilter::Snippet))
				DoFetch(Snippets, FetchArgs, Fetched);

			return Fetched;
		}

		ContextualIntelliSenseLookupResult^ IntelliSenseBackend::ContextualIntelliSenseLookup(ContextualIntelliSenseLookupArgs^ LookupArgs)
		{
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
							Result->CurrentTokenIsCallableObject = RemoteVar->GetDataType() == obScriptParsing::Variable::DataType::Ref;
							Result->CurrentTokenIsObjectReference = RemoteVar->GetDataType() == obScriptParsing::Variable::DataType::Ref;
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

	}
}