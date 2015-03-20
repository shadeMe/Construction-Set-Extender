#include "IntelliSenseItem.h"
#include "IntelliSenseDatabase.h"
#include "IntelliSenseInterface.h"

#include "SemanticAnalysis.h"
#include "ScriptEditorPreferences.h"
#include "SnippetManager.h"

#include "[Common]\NativeWrapper.h"

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		IntelliSenseDatabase^ IntelliSenseDatabase::GetSingleton()
		{
			if (Singleton == nullptr)
				Singleton = gcnew IntelliSenseDatabase();

			return Singleton;
		}

		IntelliSenseDatabase::IntelliSenseDatabase()
		{
			DebugPrint("Initializing IntelliSense");

			Enumerables = gcnew LinkedList<IntelliSenseItem^>();
			UserFunctionList = gcnew LinkedList<UserFunction^>();
			DeveloperURLMap = gcnew Dictionary<String^, String^>();
			RemoteScripts = gcnew Dictionary<String^, Script^>();
			CodeSnippets = gcnew CodeSnippetCollection();
			ScriptCommands = gcnew LinkedList<IntelliSenseItemScriptCommand^>();
			GameSettings = gcnew LinkedList<IntelliSenseItemVariable^>();

			UpdateTimerInterval = PREFERENCES->FetchSettingAsInt("DatabaseUpdateInterval", "IntelliSense");

			DatabaseUpdateTimer = gcnew Timer();
			DatabaseUpdateTimer->Tick += gcnew EventHandler(this, &IntelliSenseDatabase::DatabaseUpdateTimer_Tick);
			DatabaseUpdateTimer->Interval = UpdateTimerInterval * 60 * 1000;

			DatabaseUpdateTimer->Start();
			CodeSnippets->Load(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetSnippetCachePath()));

			DebugPrint("\tLoaded " + CodeSnippets->LoadedSnippets->Count + " Code Snippet(s)");
		}

		IntelliSenseDatabase::~IntelliSenseDatabase()
		{
			DebugPrint("Deinitializing IntelliSense");

			DatabaseUpdateTimer->Stop();

			CodeSnippets->Save(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetSnippetCachePath()));
			SAFEDELETE_CLR(CodeSnippets);

			Enumerables->Clear();
			UserFunctionList->Clear();
			DeveloperURLMap->Clear();
			RemoteScripts->Clear();
			ScriptCommands->Clear();
			GameSettings->Clear();

			Singleton = nullptr;
		}

		void IntelliSenseDatabase::DatabaseUpdateTimer_Tick(Object^ Sender, EventArgs^ E)
		{
			UpdateDatabase();
		}

		void IntelliSenseDatabase::UpdateDatabase()
		{
			if (NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CanUpdateIntelliSenseDatabase())
			{
				System::Diagnostics::Stopwatch^ Profiler = gcnew System::Diagnostics::Stopwatch();
				Profiler->Start();

				try
				{
					NativeWrapper::WriteToMainWindowStatusBar(2, "Updating IntelliSense DB...");

					ComponentDLLInterface::IntelliSenseUpdateData* DataHandlerData = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetIntelliSenseUpdateData();

					UserFunctionList->Clear();
					RemoteScripts->Clear();
					Enumerables->Clear();

					for (ComponentDLLInterface::ScriptData* Itr = DataHandlerData->ScriptListHead;
															Itr != DataHandlerData->ScriptListHead + DataHandlerData->ScriptCount;
															++Itr)
					{
						if (!Itr->IsValid())
							continue;

						UserFunctionList->AddLast(gcnew UserFunction(gcnew String(Itr->Text)));
					}
					for (ComponentDLLInterface::QuestData* Itr = DataHandlerData->QuestListHead;
															Itr != DataHandlerData->QuestListHead + DataHandlerData->QuestCount;
															++Itr)
					{
						if (!Itr->IsValid())
							continue;

						Enumerables->AddLast(gcnew IntelliSenseItemQuest(gcnew String(Itr->EditorID),
																			gcnew String(Itr->FullName),
																			gcnew String(Itr->ScriptName)));
					}

					for (ComponentDLLInterface::GlobalData* Itr = DataHandlerData->GlobalListHead;
															Itr != DataHandlerData->GlobalListHead + DataHandlerData->GlobalCount;
															++Itr)
					{
						if (!Itr->IsValid())
							continue;

						if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Int)
						{
							Enumerables->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID),
																			gcnew String(""),
																			ObScriptSemanticAnalysis::Variable::DataType::Integer,
																			IntelliSenseItem::IntelliSenseItemType::GlobalVar));
						}
						else if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Float)
						{
							Enumerables->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID),
																			gcnew String(""),
																			ObScriptSemanticAnalysis::Variable::DataType::Float,
																			IntelliSenseItem::IntelliSenseItemType::GlobalVar));
						}
						else
						{
							Enumerables->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID),
																			gcnew String(""),
																			ObScriptSemanticAnalysis::Variable::DataType::StringVar,
																			IntelliSenseItem::IntelliSenseItemType::GlobalVar));
						}
					}

					for each (IntelliSenseItem^ Itr in ScriptCommands)
						Enumerables->AddLast(Itr);

					for each (IntelliSenseItem^ Itr in GameSettings)
						Enumerables->AddLast(Itr);

					for each (UserFunction^ Itr in UserFunctionList)
					{
						Enumerables->AddLast(gcnew IntelliSenseItemUserFunction(Itr));
					}

					for (ComponentDLLInterface::FormData* Itr = DataHandlerData->EditorIDListHead;
						 Itr != DataHandlerData->EditorIDListHead + DataHandlerData->EditorIDCount;
						 ++Itr)
					{
						if (!Itr->IsValid())
							continue;

						Enumerables->AddLast(gcnew IntelliSenseItemEditorIDForm(Itr));
					}

					for each (CodeSnippet^ Itr in CodeSnippets->LoadedSnippets)
						Enumerables->AddLast(gcnew IntelliSenseItemCodeSnippet(Itr));

					NativeWrapper::WriteToMainWindowStatusBar(2, "IntelliSense DB updated.");
					NativeWrapper::WriteToMainWindowStatusBar(3, "[" +
									Profiler->ElapsedMilliseconds.ToString() + "ms | " +
									DataHandlerData->ScriptCount + " UDF(s) | " +
									DataHandlerData->QuestCount + " Quest(s) | " +
									DataHandlerData->GlobalCount + " Global(s) | " +
									DataHandlerData->EditorIDCount + " Forms" +
									"]");

					NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(DataHandlerData, false);
				}
				catch (Exception^ E)
				{
					DebugPrint("Couldn't update IntelliSense DB!\n\tException: " + E->Message, true);
					NativeWrapper::WriteToMainWindowStatusBar(2, "Error encountered while updating IntelliSense DB!");
				}

				Profiler->Stop();
			}
		}

		UInt32 IntelliSenseDatabase::InitializeCommandTableDatabase(ComponentDLLInterface::CommandTableData* Data)
		{
			String^ Name;
			String^ Description;
			String^ ShortHand;
			String^ PluginName;
			int Count = 0, ReturnType = 0, CSCount = 0;
			IntelliSenseItemScriptCommand::IntelliSenseCommandItemSourceType Source;
			System::Globalization::TextInfo^ Locale = (gcnew System::Globalization::CultureInfo("en-US", false))->TextInfo;

			for (const ComponentDLLInterface::ObScriptCommandInfo* Itr = Data->CommandTableStart; Itr != Data->CommandTableEnd; ++Itr)
			{
				Name = gcnew String(Itr->longName);
				if (!String::Compare(Name, "", true))
					continue;

				const ComponentDLLInterface::CommandTableData::PluginInfo* Info = Data->GetParentPlugin(Itr);

				if (CSCount < 370)
				{
					Description = "[CS] ";				// 369 vanilla commands
					Source = IntelliSenseItemScriptCommand::IntelliSenseCommandItemSourceType::Vanilla;
				}
				else if (Info)
				{
					PluginName = gcnew String(Info->name);
					if (!String::Compare(PluginName, "OBSE_Kyoma_MenuQue", true))
						PluginName = "MenuQue";
					else if (!String::Compare(PluginName, "OBSE_Elys_Pluggy", true))
						PluginName = "Pluggy";

					Description = "[" + PluginName + " v" + Info->version + "] ";
					Source = IntelliSenseItemScriptCommand::IntelliSenseCommandItemSourceType::OBSE;
				}
				else
				{
					UInt32 OBSEVersion = Data->GetRequiredOBSEVersion(Itr);
					Description = "[OBSE v" + OBSEVersion + "] ";
					Source = IntelliSenseItemScriptCommand::IntelliSenseCommandItemSourceType::OBSE;
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

					ComponentDLLInterface::ObScriptCommandInfo::ParamInfo* Param = &Itr->params[i];
					if (Param)
					{
						Params += "\t" + Locale->ToTitleCase(gcnew String((Param->typeStr ? Param->typeStr : "")) +
															 " [" + gcnew String(Param->TypeIDString()) + "]" +
															 (Param->isOptional ? " (Optional) " : "") + "\n");
					}
				}

				ScriptCommands->AddLast(gcnew IntelliSenseItemScriptCommand(Name,
					Description,
					ShortHand,
					Itr->numParams,
					Itr->needsParent,
					ReturnType,
					Source,
					Params));

				CSCount++;
				Count++;
			}

			DebugPrint(String::Format("\tParsed {0} Commands", Count));

			return Count;
		}

		void IntelliSenseDatabase::InitializeGMSTDatabase(ComponentDLLInterface::IntelliSenseUpdateData* GMSTCollection)
		{
			for (int i = 0; i < GMSTCollection->GMSTCount; i++)
			{
				ComponentDLLInterface::GMSTData* Itr = &GMSTCollection->GMSTListHead[i];
				if (!Itr->IsValid())
					continue;

				if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Int)
					GameSettings->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID),
										gcnew String(""), ObScriptSemanticAnalysis::Variable::DataType::Integer, IntelliSenseItem::IntelliSenseItemType::GMST));
				else if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Float)
					GameSettings->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID),
										gcnew String(""), ObScriptSemanticAnalysis::Variable::DataType::Float, IntelliSenseItem::IntelliSenseItemType::GMST));
				else
					GameSettings->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID),
										gcnew String(""), ObScriptSemanticAnalysis::Variable::DataType::StringVar, IntelliSenseItem::IntelliSenseItemType::GMST));
			}

			DebugPrint(String::Format("\tParsed {0} Game Settings", GMSTCollection->GMSTCount));
		}

		void IntelliSenseDatabase::RegisterDeveloperURL(String^ CmdName, String^ URL)
		{
			for each (KeyValuePair<String^, String^>% Itr in DeveloperURLMap)
			{
				if (!String::Compare(CmdName, Itr.Key, true))
					return;
			}

			DeveloperURLMap->Add(CmdName, URL);
		}

		String^	IntelliSenseDatabase::LookupDeveloperURLByCommand(String^ CmdName)
		{
			String^ Result = nullptr;
			for each (KeyValuePair<String^, String^>% Itr in DeveloperURLMap)
			{
				if (!String::Compare(CmdName, Itr.Key, true))
				{
					Result = Itr.Value;
					break;
				}
			}
			return Result;
		}

		String^	IntelliSenseDatabase::SanitizeIdentifier(String^ Name)
		{
			for each (IntelliSenseItem^ Itr in Enumerables)
			{
				if (Name->Length == Itr->GetIdentifier()->Length &&
					!String::Compare(Itr->GetIdentifier(), Name, true))
				{
					return Itr->GetIdentifier();
				}
				else if (Itr->GetItemType() == IntelliSenseItem::IntelliSenseItemType::Command)
				{
					IntelliSenseItemScriptCommand^ Current = (IntelliSenseItemScriptCommand^)Itr;
					String^ Alias = Current->GetShorthand();
					if (Alias->Length == Name->Length && !String::Compare(Alias, Name, true))
						return Alias;
				}
			}

			return Name;
		}

		Script^ IntelliSenseDatabase::CacheRemoteScript(String^ BaseEditorID, String^ ScriptText)
		{
			for each (KeyValuePair<String^, Script^>% Itr in RemoteScripts)
			{
				if (!String::Compare(BaseEditorID, Itr.Key, true))
				{
					return Itr.Value;
				}
			}

			RemoteScripts->Add(BaseEditorID, gcnew Script(ScriptText));
			return CacheRemoteScript(BaseEditorID, nullptr);
		}

		bool IntelliSenseDatabase::GetIsIdentifierUserFunction(String^ Name)
		{
			bool Result = false;

			for each (UserFunction^ Itr in UserFunctionList)
			{
				if (!String::Compare(Name, Itr->GetIdentifier(), true))
				{
					Result = true;
					break;
				}
			}

			return Result;
		}

		bool IntelliSenseDatabase::GetIsIdentifierScriptCommand(String^ Name)
		{
			bool Result = false;

			for each (IntelliSenseItem^ Itr in Enumerables)
			{
				if (Itr->GetItemType() == IntelliSenseItem::IntelliSenseItemType::Command)
				{
					if (!String::Compare(Itr->GetIdentifier(), Name, true))
					{
						Result = true;
						break;
					}
				}
			}

			return Result;
		}

		void IntelliSenseDatabase::ForceUpdateDatabase()
		{
			DatabaseUpdateTimer->Start();
			UpdateDatabase();
		}

		IntelliSenseItem^ IntelliSenseDatabase::LookupRemoteScriptVariable( String^ BaseEditorID, String^ Variable )
		{
			for each (KeyValuePair<String^, Script^>% Itr in RemoteScripts)
			{
				if (!String::Compare(BaseEditorID, Itr.Key, true))
				{
					Script^ RemoteScript = Itr.Value;
					for (Script::VarListT::Enumerator^ RemoteVarItr = RemoteScript->GetVariableListEnumerator(); RemoteVarItr->MoveNext();)
					{
						if (RemoteVarItr->Current->GetItemType() == IntelliSenseItem::IntelliSenseItemType::RemoteVar)
						{
							if (!String::Compare(RemoteVarItr->Current->GetIdentifier(), Variable, true))
							{
								return RemoteVarItr->Current;
							}
						}
					}
				}
			}

			return nullptr;
		}

		bool IntelliSenseDatabase::GetIsIdentifierScriptableForm( String^ Name )
		{
			return GetIsIdentifierScriptableForm(Name, 0);
		}

		bool IntelliSenseDatabase::GetIsIdentifierScriptableForm( String^ Name, ComponentDLLInterface::ScriptData** OutScriptData )
		{
			bool Result = false;

			CString EID(Name);
			ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str());

			if (Data && Data->IsValid())
			{
				Result = true;

				if (OutScriptData)
					*OutScriptData = Data;
			}

			if (OutScriptData == 0)
				NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);

			return Result;
		}

		void IntelliSenseDatabase::ShowCodeSnippetManager()
		{
			CodeSnippetManagerDialog ManagerDialog(CodeSnippets);

			ForceUpdateDatabase();
		}

		bool IntelliSenseDatabase::GetIsIdentifierForm( String^ Name )
		{
			bool Result = false;

			CString EID(Name);
			ComponentDLLInterface::FormData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupFormByEditorID(EID.c_str());

			if (Data && Data->IsValid())
				Result = true;

			NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);

			return Result;
		}
	}
}