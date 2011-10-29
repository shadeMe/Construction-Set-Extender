#include "IntelliSenseItem.h"
#include "IntelliSenseDatabase.h"
#include "IntelliSenseInterface.h"
#include "[Common]\NativeWrapper.h"
#include "..\ScriptParser.h"
#include "..\ScriptEditorPreferences.h"

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		IntelliSenseDatabase^% IntelliSenseDatabase::GetSingleton()
		{
			if (Singleton == nullptr)
			{
				Singleton = gcnew IntelliSenseDatabase();
			}
			return Singleton;
		}

		IntelliSenseDatabase::IntelliSenseDatabase()
		{
			DebugPrint("Initializing IntelliSense");

			Enumerables = gcnew LinkedList<IntelliSenseItem^>();
			UserFunctionList = gcnew LinkedList<UserFunction^>();
			DeveloperURLMap = gcnew Dictionary<String^, String^>();
			RemoteScripts = gcnew Dictionary<String^, Script^>();

			DatabaseUpdateThread = gcnew BackgroundWorker();
			DatabaseUpdateThread->DoWork += gcnew DoWorkEventHandler(this, &IntelliSenseDatabase::DatabaseUpdateThread_DoWork);
			DatabaseUpdateThread->RunWorkerCompleted += gcnew RunWorkerCompletedEventHandler(this, &IntelliSenseDatabase::DatabaseUpdateThread_RunWorkerCompleted);

			UpdateThreadTimerInterval = 10;
			DatabaseUpdateTimer = gcnew Timers::Timer(UpdateThreadTimerInterval * 60 * 1000);	// init to 10 minutes
			DatabaseUpdateTimer->Elapsed += gcnew Timers::ElapsedEventHandler(this, &IntelliSenseDatabase::DatabaseUpdateTimer_OnTimed);
			DatabaseUpdateTimer->AutoReset = true;
			DatabaseUpdateTimer->SynchronizingObject = nullptr;
			ForceUpdateFlag = false;
		}

		IntelliSenseDatabase::ParsedUpdateData^ IntelliSenseDatabase::InitializeDatabaseUpdate()
		{
			System::Diagnostics::Stopwatch^ Profiler = gcnew System::Diagnostics::Stopwatch();
			Profiler->Start();

			ParsedUpdateData^ Data = gcnew ParsedUpdateData();

			ComponentDLLInterface::IntelliSenseUpdateData* DataHandlerData = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetIntelliSenseUpdateData();

			for (ComponentDLLInterface::ScriptData* Itr = DataHandlerData->ScriptListHead; Itr != DataHandlerData->ScriptListHead + DataHandlerData->ScriptCount; ++Itr)
			{
				if (!Itr->IsValid())		continue;

				Data->UDFList->AddLast(gcnew UserFunction(gcnew String(Itr->Text)));
			}

			for (ComponentDLLInterface::QuestData* Itr = DataHandlerData->QuestListHead; Itr != DataHandlerData->QuestListHead + DataHandlerData->QuestCount; ++Itr)
			{
				if (!Itr->IsValid())		continue;

				Data->Enumerables->AddLast(gcnew IntelliSenseItemQuest(gcnew String(Itr->EditorID), gcnew String(Itr->FullName), gcnew String(Itr->ScriptName)));
			}

			for (ComponentDLLInterface::GlobalData* Itr = DataHandlerData->GlobalListHead; Itr != DataHandlerData->GlobalListHead + DataHandlerData->GlobalCount; ++Itr)
			{
				if (!Itr->IsValid())		continue;

				if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Int)
					Data->Enumerables->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID), gcnew String(""), IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_Int, IntelliSenseItem::IntelliSenseItemType::e_GlobalVar));
				else if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Float)
					Data->Enumerables->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID), gcnew String(""), IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_Float, IntelliSenseItem::IntelliSenseItemType::e_GlobalVar));
				else
					Data->Enumerables->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID), gcnew String(""), IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_String, IntelliSenseItem::IntelliSenseItemType::e_GlobalVar));
			}

			for each (IntelliSenseItem^ Itr in Enumerables)
			{
				if (Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_Cmd ||
					Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_GMST)
				{
					Data->Enumerables->AddLast(Itr);
				}
			}

			for each (UserFunction^% Itr in Data->UDFList)
			{
				Data->Enumerables->AddLast(gcnew IntelliSenseItemUserFunction(Itr));
			}

			for (ComponentDLLInterface::FormData* Itr = DataHandlerData->EditorIDListHead; Itr != DataHandlerData->EditorIDListHead + DataHandlerData->EditorIDCount; ++Itr)
			{
				if (!Itr->IsValid())		continue;

				Data->Enumerables->AddLast(gcnew IntelliSenseItemEditorIDForm(Itr));
			}

			NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(DataHandlerData, false);

			Profiler->Stop();
			return Data;
		}

		void IntelliSenseDatabase::FinalizeDatabaseUpdate(ParsedUpdateData^ Data)
		{
			UserFunctionList->Clear();
			Enumerables->Clear();
			RemoteScripts->Clear();

			UserFunctionList = Data->UDFList;
			Enumerables = Data->Enumerables;

			NativeWrapper::WriteToMainWindowStatusBar(2, "IntelliSense database updated");
		}

		void IntelliSenseDatabase::DatabaseUpdateTimer_OnTimed(Object^ Sender, Timers::ElapsedEventArgs^ E)
		{
			if (ForceUpdateFlag)
			{
				DatabaseUpdateTimer->Interval = UpdateThreadTimerInterval * 60 * 1000;
				ForceUpdateFlag = false;
			}

			UpdateDatabase();
		}

		void IntelliSenseDatabase::DatabaseUpdateThread_DoWork(Object^ Sender, DoWorkEventArgs^ E)
		{
			E->Result = InitializeDatabaseUpdate();
		}

		void IntelliSenseDatabase::DatabaseUpdateThread_RunWorkerCompleted(Object^ Sender, RunWorkerCompletedEventArgs^ E)
		{
			if (E->Error != nullptr)
				DebugPrint("ISDatabaseUpdate thread raised an exception!\n\tException: " + E->Error->Message, true);
			else if (E->Cancelled)
				DebugPrint("Huh?! ISDatabaseUpdate thread was canceled", true);
			else if (E->Result == nullptr)
				DebugPrint("Something seriously went wrong when updating the IntelliSense database!", true);
			else
			{
				FinalizeDatabaseUpdate(dynamic_cast<ParsedUpdateData^>(E->Result));
			}

			if (E->Result == nullptr)
			{
				NativeWrapper::WriteToMainWindowStatusBar(2, "Error encountered while updating IntelliSense database !");
			}
		}

		void IntelliSenseDatabase::UpdateDatabase()
		{
			if (!DatabaseUpdateThread->IsBusy && NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CanUpdateIntelliSenseDatabase())
			{
				DatabaseUpdateThread->RunWorkerAsync();
				NativeWrapper::WriteToMainWindowStatusBar(2, "Updating IntelliSense database ...");
			}
		}

		void IntelliSenseDatabase::ParseScript(String^% SourceText, IntelliSenseParseScriptData^ Box)
		{
			ScriptParser^ ScriptTextParser = gcnew ScriptParser();
			StringReader^ TextParser = gcnew StringReader(SourceText);

			String^ ReadLine = TextParser->ReadLine(), ^FirstToken, ^SecondToken = "", ^Comment = "", ^Description = "", ^ScriptName;
			bool GrabDef = false, LocalVars = false;

			switch (Box->Type)
			{
			case IntelliSenseParseScriptData::DataType::e_Script:
			case IntelliSenseParseScriptData::DataType::e_UserFunction:
				Box->SourceScript->ClearVariableList();
				break;
			case IntelliSenseParseScriptData::DataType::e_IntelliSenseInterface:
				Box->SourceIntelliSenseInterface->ClearLocalVariableDatabase();
				LocalVars = true;
				break;
			}

			while (ReadLine != nullptr)
			{
				ScriptTextParser->Tokenize(ReadLine, false);
				if (!ScriptTextParser->Valid)
				{
					ReadLine = TextParser->ReadLine();
					continue;
				}

				if (ScriptTextParser->Tokens[0][0] == ';' && ReadLine->IndexOf(";<CSE") != 0 && ReadLine->IndexOf(";</CSE") != 0 && GrabDef)
					Description += ReadLine->Substring(ScriptTextParser->Indices[0] + 1) + "\n";

				FirstToken = ScriptTextParser->Tokens[0],
					SecondToken = (ScriptTextParser->Tokens->Count > 1)?ScriptTextParser->Tokens[1]:"";

				ScriptParser::TokenType Type = ScriptTextParser->GetTokenType(FirstToken);

				switch (Type)
				{
				case ScriptParser::TokenType::e_Variable:
					GrabDef = false;
					if (ScriptTextParser->LookupVariableByName(SecondToken) == nullptr && SecondToken != "")
					{
						Comment = "";
						if (ScriptTextParser->Tokens->Count > 2 && ScriptTextParser->GetCommentTokenIndex(ScriptTextParser->Tokens->Count) == 2)
						{
							Comment = ReadLine->Substring(ReadLine->IndexOf(";") + 1);
							ScriptTextParser->Tokenize(Comment, false);
							Comment = (ScriptTextParser->Indices->Count > 0)?Comment->Substring(ScriptTextParser->Indices[0]):Comment;
						}

						ScriptTextParser->Variables->AddLast(gcnew ScriptParser::VariableRefCountData(SecondToken, 0));
						IntelliSenseItemVariable::IntelliSenseItemVariableDataType DataType;

						if (!String::Compare(FirstToken, "ref", true) || !String::Compare(FirstToken, "reference", true))
							DataType = IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_Ref;
						else if	(!String::Compare(FirstToken, "short", true) || !String::Compare(FirstToken, "long", true) || !String::Compare(FirstToken, "int", true))
							DataType = IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_Int;
						else if	(!String::Compare(FirstToken, "float", true))
							DataType = IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_Float;
						else if	(!String::Compare(FirstToken, "string_var", true))
							DataType = IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_String;
						else
							DataType = IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_Array;

						switch (Box->Type)
						{
						case IntelliSenseParseScriptData::DataType::e_UserFunction:
						case IntelliSenseParseScriptData::DataType::e_Script:
							Box->SourceScript->AddVariable(gcnew IntelliSenseItemVariable(SecondToken, Comment, DataType, (LocalVars)?(IntelliSenseItem::IntelliSenseItemType::e_LocalVar):(IntelliSenseItem::IntelliSenseItemType::e_RemoteVar)));
							break;
						case IntelliSenseParseScriptData::DataType::e_IntelliSenseInterface:
							Box->SourceIntelliSenseInterface->AddLocalVariableToDatabase(gcnew IntelliSenseItemVariable(SecondToken, Comment, DataType, (LocalVars)?(IntelliSenseItem::IntelliSenseItemType::e_LocalVar):(IntelliSenseItem::IntelliSenseItemType::e_RemoteVar)));
							break;
						}
					}
					break;
				case ScriptParser::TokenType::e_ScriptName:
					GrabDef = true;
					ScriptName = SecondToken;
					break;
				case ScriptParser::TokenType::e_Begin:
					GrabDef = false;
					if (Box->Type == IntelliSenseParseScriptData::DataType::e_UserFunction)
					{
						if (!String::Compare(SecondToken, "function", true))
						{
							String^ ParamList = ReadLine->Substring(ReadLine->IndexOf("{"), ReadLine->IndexOf("}") - ReadLine->IndexOf("{"));
							ScriptTextParser->Tokenize(ParamList, false);
							int ParamIdx = 0;
							for each (String^% Itr in ScriptTextParser->Tokens)
							{
								int VarIdx = 0;
								for each (ScriptParser::VariableRefCountData^% ItrEx in ScriptTextParser->Variables)
								{
									if (!String::Compare(ItrEx->Name, Itr, true) && ParamIdx < 10)
									{
										(dynamic_cast<UserFunction^>(Box->SourceScript))->AddParameter(VarIdx, ParamIdx);
										ParamIdx++;
									}
									VarIdx++;
								}
							}
						}
					}
					break;
				case ScriptParser::TokenType::e_Comment:
					break;
				case ScriptParser::TokenType::e_SetFunctionValue:
					GrabDef = false;
					if (Box->Type == IntelliSenseParseScriptData::DataType::e_UserFunction)
					{
						(dynamic_cast<UserFunction^>(Box->SourceScript))->SetReturnVariable(-9);						// ambiguous
						int VarIdx = 0;
						for each (ScriptParser::VariableRefCountData^% Itr in ScriptTextParser->Variables)
						{
							if (!String::Compare(SecondToken, Itr->Name, true))
							{
								(dynamic_cast<UserFunction^>(Box->SourceScript))->SetReturnVariable(VarIdx);
								break;
							}
							VarIdx++;
						}
					}
					break;
				default:
					GrabDef = false;
					break;
				}

				ReadLine = TextParser->ReadLine();
			}

			switch (Box->Type)
			{
			case IntelliSenseParseScriptData::DataType::e_Script:
			case IntelliSenseParseScriptData::DataType::e_UserFunction:
				Box->SourceScript->SetName(ScriptName);
				Box->SourceScript->SetCommentDescription(Description);
				break;
			}
		}

		void IntelliSenseDatabase::InitializeCommandTableDatabase(ComponentDLLInterface::CommandTableData* Data)
		{
			String^ Name, ^Desc, ^SH, ^PluginName;
			int Count = 0, ReturnType = 0, CSCount = 0;
			IntelliSenseItemScriptCommand::IntelliSenseCommandItemSourceType Source;

			for (const ComponentDLLInterface::ObScriptCommandInfo* Itr = Data->CommandTableStart; Itr != Data->CommandTableEnd; ++Itr)
			{
				Name = gcnew String(Itr->longName);
				if (!String::Compare(Name, "", true))
					continue;

				const ComponentDLLInterface::CommandTableData::PluginInfo* Info = Data->GetParentPlugin(Itr);

				if (CSCount < 370)
				{
					Desc = "[CS] ";				// 369 vanilla commands
					Source = IntelliSenseItemScriptCommand::IntelliSenseCommandItemSourceType::e_Vanilla;
				}
				else if (Info)
				{
					PluginName = gcnew String(Info->name);
					if (!String::Compare(PluginName, "OBSE_Kyoma_MenuQue", true))
						PluginName = "MenuQue";
					else if (!String::Compare(PluginName, "OBSE_Elys_Pluggy", true))
						PluginName = "Pluggy";

					Desc = "[" + PluginName + " v" + Info->version + "] ";
					Source = IntelliSenseItemScriptCommand::IntelliSenseCommandItemSourceType::e_OBSE;
				}
				else
				{
					UInt32 OBSEVersion = Data->GetRequiredOBSEVersion(Itr);
					Desc = "[OBSE v" + OBSEVersion + "] ";
					Source = IntelliSenseItemScriptCommand::IntelliSenseCommandItemSourceType::e_OBSE;
				}

				if (!String::Compare(gcnew String(Itr->helpText), "", true))
					Desc += "No description";
				else
					Desc += gcnew String(Itr->helpText);

				if (!String::Compare(gcnew String(Itr->shortName), "", true))
					SH = "None";
				else
					SH = gcnew String(Itr->shortName);

				ReturnType = Data->GetCommandReturnType(Itr);
				if (ReturnType == 6)
					ReturnType = 0;

				Enumerables->AddLast(gcnew IntelliSenseItemScriptCommand(Name, Desc, SH, Itr->numParams, Itr->needsParent, ReturnType, Source));

				CSCount++;
				Count++;
			}

			DebugPrint(String::Format("\tParsed {0} Commands", Count));
		}

		void IntelliSenseDatabase::InitializeGMSTDatabase(ComponentDLLInterface::IntelliSenseUpdateData* GMSTCollection)
		{
			for (int i = 0; i < GMSTCollection->GMSTCount; i++)
			{
				ComponentDLLInterface::GMSTData* Itr = &GMSTCollection->GMSTListHead[i];
				if (!Itr->IsValid())
					continue;

				if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Int)
					Enumerables->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID), gcnew String(""), IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_Int, IntelliSenseItem::IntelliSenseItemType::e_GMST));
				else if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Float)
					Enumerables->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID), gcnew String(""), IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_Float, IntelliSenseItem::IntelliSenseItemType::e_GMST));
				else
					Enumerables->AddLast(gcnew IntelliSenseItemVariable(gcnew String(Itr->EditorID), gcnew String(""), IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_String, IntelliSenseItem::IntelliSenseItemType::e_GMST));
			}

			DebugPrint(String::Format("\tParsed {0} Game Settings", GMSTCollection->GMSTCount));
		}

		void IntelliSenseDatabase::RegisterDeveloperURL(String^% CmdName, String^% URL)
		{
			for each (KeyValuePair<String^, String^>% Itr in DeveloperURLMap)
			{
				if (!String::Compare(CmdName, Itr.Key, true))
					return;
			}

			DeveloperURLMap->Add(CmdName, URL);
		}

		String^	IntelliSenseDatabase::LookupDeveloperURLByCommand(String^% CmdName)
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

		String^	IntelliSenseDatabase::SanitizeCommandIdentifier(String^% CmdName)
		{
			for each (IntelliSenseItem^ Itr in Enumerables)
			{
				if (Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_Cmd)
				{
					if (!String::Compare(Itr->GetIdentifier(), CmdName, true))
					{
						return Itr->GetIdentifier();
					}
				}
			}

			return CmdName;
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

		bool IntelliSenseDatabase::GetIsIdentifierUserFunction(String^% Name)
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

		bool IntelliSenseDatabase::GetIsIdentifierScriptCommand(String^% Name)
		{
			bool Result = false;

			for each (IntelliSenseItem^ Itr in Enumerables)
			{
				if (Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_Cmd)
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
			ForceUpdateFlag = true;
			DatabaseUpdateTimer->Interval = 500;
		}

		void IntelliSenseDatabase::InitializeDatabaseUpdateThread()
		{
			static bool DatabaseTimerInitialized = false;
			if (DatabaseTimerInitialized)
				return;

			UpdateThreadTimerInterval = PREFERENCES->FetchSettingAsInt("DatabaseUpdateInterval", "IntelliSense");
			DatabaseUpdateTimer->Start();
			DatabaseTimerInitialized = true;
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
						if (RemoteVarItr->Current->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_RemoteVar)
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
	}
}