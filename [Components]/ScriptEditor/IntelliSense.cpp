#include "IntelliSense.h"
#include "Globals.h"
#include "ScriptParser.h"
#include "ScriptEditor.h"
#include "[Common]\NativeWrapper.h"
#include "[Common]\HandShakeStructs.h"
#include "[Common]\ListViewUtilities.h"

namespace IntelliSense
{
	Script::Script()
	{
		VarList = gcnew List<VariableInfo^>();
	}

	Script::Script(String^% ScriptText)
	{
		VarList = gcnew List<VariableInfo^>();
		IntelliSenseDatabase::ParseScript(ScriptText, gcnew Boxer(this));
	}

	Script::Script(String^% ScriptText, String^% Name)
	{
		VarList = gcnew VarListT();
		IntelliSenseDatabase::ParseScript(ScriptText, gcnew Boxer(this));
		this->Name = Name;
	}

	String^ Script::Describe()
	{
		String^ Description;
		Description += Name + "\n\n" + this->Description + "\nNumber of variables: " + VarList->Count;
		return Description;
	}

	UserFunction::UserFunction(String^% ScriptText)
	{
		Parameters = Array::CreateInstance(int::typeid, 10);
		Parameters->SetValue(-1, 0);
		Parameters->SetValue(-1, 1);
		Parameters->SetValue(-1, 2);
		Parameters->SetValue(-1, 3);
		Parameters->SetValue(-1, 4);
		Parameters->SetValue(-1, 5);
		Parameters->SetValue(-1, 6);
		Parameters->SetValue(-1, 7);
		Parameters->SetValue(-1, 8);
		Parameters->SetValue(-1, 9);
		ReturnVar = -1;

		IntelliSenseDatabase::ParseScript(ScriptText, gcnew Boxer(this));
	}

	String^ UserFunction::Describe()
	{
		String^ Description, ^Scratch;

		int ParamIdx = 0;
		while (ParamIdx < 10)
		{
			int VarIdx = (int)Parameters->GetValue(ParamIdx);
			if (VarIdx == -1)	break;

			String^% Comment = VarList[VarIdx]->GetComment(), ^% Name = VarList[VarIdx]->GetName();
			Scratch += "\n\t" + ((Comment == "")?Name:Comment) + " [" + VarList[VarIdx]->GetTypeIdentifier() + "]";
			ParamIdx++;
		}

		Description += Name + "\n\nDescription: " + this->Description + "\n" + ParamIdx + " Parameters" + Scratch + "\n\n";
		if (ReturnVar == -1)			Description += "Does not return a value";
		else if (ReturnVar == -9)		Description += "Return Type: Ambiguous";
		else							Description += "Return Type: " + VarList[ReturnVar]->GetTypeIdentifier();

		return Description;
	}

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
		Enumerables = gcnew LinkedList<IntelliSenseItem^>();
		UserFunctionList = gcnew LinkedList<UserFunction^>();
		DeveloperURLMap = gcnew Dictionary<String^, String^>();
		RemoteScripts = gcnew Dictionary<String^, Script^>();

		DatabaseUpdateThread = gcnew BackgroundWorker();
		DatabaseUpdateThread->DoWork += gcnew DoWorkEventHandler(this, &IntelliSenseDatabase::DatabaseUpdateThread_DoWork);
		DatabaseUpdateThread->RunWorkerCompleted += gcnew RunWorkerCompletedEventHandler(this, &IntelliSenseDatabase::DatabaseUpdateThread_RunWorkerCompleted);

		UpdateThreadTimerInterval = 10;
		DatabaseUpdateTimer = gcnew Timers::Timer(UpdateThreadTimerInterval * 60 * 1000);	// init to 10 earth minutes
		DatabaseUpdateTimer->Elapsed += gcnew Timers::ElapsedEventHandler(this, &IntelliSenseDatabase::DatabaseUpdateTimer_OnTimed);
		DatabaseUpdateTimer->AutoReset = true;
		DatabaseUpdateTimer->SynchronizingObject = nullptr;
		ForceUpdateFlag = false;

		DebugPrint("Initialized IntelliSense");
	}

	IntelliSenseDatabase::ParsedUpdateData^ IntelliSenseDatabase::InitializeDatabaseUpdate()
	{
		ParsedUpdateData^ Data = gcnew ParsedUpdateData();

		ComponentDLLInterface::IntelliSenseUpdateData* DataHandlerData = g_CSEInterface->ScriptEditor.GetIntelliSenseUpdateData();

		for (ComponentDLLInterface::ScriptData* Itr = DataHandlerData->ScriptListHead; Itr != DataHandlerData->ScriptListHead + DataHandlerData->ScriptCount; ++Itr)
		{
			if (!Itr->IsValid())		continue;

			Data->UDFList->AddLast(gcnew UserFunction(gcnew String(Itr->Text)));
		}

		for (ComponentDLLInterface::QuestData* Itr = DataHandlerData->QuestListHead; Itr != DataHandlerData->QuestListHead + DataHandlerData->QuestCount; ++Itr)
		{
			if (!Itr->IsValid())		continue;

			Data->Enumerables->AddLast(gcnew Quest(gcnew String(Itr->EditorID), gcnew String(Itr->FullName), gcnew String(Itr->ScriptName)));
		}

		for (ComponentDLLInterface::GlobalData* Itr = DataHandlerData->GlobalListHead; Itr != DataHandlerData->GlobalListHead + DataHandlerData->GlobalCount; ++Itr)
		{
			if (!Itr->IsValid())		continue;

			if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Int)
				Data->Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_Int, IntelliSenseItem::ItemType::e_GlobalVar));
			else if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Float)
				Data->Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_Float, IntelliSenseItem::ItemType::e_GlobalVar));
			else
				Data->Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_String, IntelliSenseItem::ItemType::e_GlobalVar));
		}

		for each (IntelliSenseItem^ Itr in Enumerables)
		{
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd ||
				Itr->GetType() == IntelliSenseItem::ItemType::e_GMST)
			{
				Data->Enumerables->AddLast(Itr);
			}
		}

		for each (UserFunction^% Itr in Data->UDFList)
		{
			Data->Enumerables->AddLast(gcnew UserFunctionDelegate(Itr));
		}

		g_CSEInterface->DeleteNativeHeapPointer(DataHandlerData, false);
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
			DebugPrint("The ISDatabaseUpdate thread raised an exception!\n\tException: " + E->Error->Message, true);
		else if (E->Cancelled)
			DebugPrint("Huh?! ISDatabaseUpdate thread was cancelled", true);
		else if (E->Result == nullptr)
			DebugPrint("Something seriously went wrong when parsing the datahandler!", true);
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
		if (!DatabaseUpdateThread->IsBusy)
		{
			DatabaseUpdateThread->RunWorkerAsync();
			NativeWrapper::WriteToMainWindowStatusBar(2, "Updating IntelliSense database ...");
		}
	}

	void IntelliSenseDatabase::ParseScript(String^% SourceText, Boxer^ Box)
	{
		ScriptParser^ ScriptTextParser = gcnew ScriptParser();
		StringReader^ TextParser = gcnew StringReader(SourceText);

		String^ ReadLine = TextParser->ReadLine(), ^FirstToken, ^SecondToken = "", ^Comment = "", ^Description = "", ^ScriptName;
		bool GrabDef = false, LocalVars = false;

		switch (Box->Type)
		{
		case Boxer::BoxType::e_Script:
		case Boxer::BoxType::e_UserFunction:
			Box->SourceScript->ClearVariableList();
			break;
		case Boxer::BoxType::e_IntelliSenseInterface:
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
					VariableInfo::VariableType DataType;

					if (!String::Compare(FirstToken, "ref", true) || !String::Compare(FirstToken, "reference", true))
						DataType = VariableInfo::VariableType::e_Ref;
					else if	(!String::Compare(FirstToken, "short", true) || !String::Compare(FirstToken, "long", true) || !String::Compare(FirstToken, "int", true))
						DataType = VariableInfo::VariableType::e_Int;
					else if	(!String::Compare(FirstToken, "float", true))
						DataType = VariableInfo::VariableType::e_Float;
					else if	(!String::Compare(FirstToken, "string_var", true))
						DataType = VariableInfo::VariableType::e_String;
					else
						DataType = VariableInfo::VariableType::e_Array;

					switch (Box->Type)
					{
					case Boxer::BoxType::e_UserFunction:
					case Boxer::BoxType::e_Script:
						Box->SourceScript->AddVariable(gcnew VariableInfo(SecondToken, Comment, DataType, (LocalVars)?(IntelliSenseItem::ItemType::e_LocalVar):(IntelliSenseItem::ItemType::e_RemoteVar)));
						break;
					case Boxer::BoxType::e_IntelliSenseInterface:
						Box->SourceIntelliSenseInterface->AddLocalVariableToDatabase(gcnew VariableInfo(SecondToken, Comment, DataType, (LocalVars)?(IntelliSenseItem::ItemType::e_LocalVar):(IntelliSenseItem::ItemType::e_RemoteVar)));
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
				if (Box->Type == Boxer::BoxType::e_UserFunction)
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
				if (Box->Type == Boxer::BoxType::e_UserFunction)
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
		case Boxer::BoxType::e_Script:
		case Boxer::BoxType::e_UserFunction:
			Box->SourceScript->SetName(ScriptName);
			Box->SourceScript->SetDescription(Description);
			break;
		}
	}

	void IntelliSenseDatabase::InitializeCommandTableDatabase(ComponentDLLInterface::CommandTableData* Data)
	{
		String^ Name, ^Desc, ^SH, ^PluginName;
		int Count = 0, ReturnType = 0, CSCount = 0;
		CommandInfo::SourceType Source;

		for (const ComponentDLLInterface::ObScriptCommandInfo* Itr = Data->CommandTableStart; Itr != Data->CommandTableEnd; ++Itr)
		{
			Name = gcnew String(Itr->longName);
			if (!String::Compare(Name, "", true))	continue;

			const ComponentDLLInterface::CommandTableData::PluginInfo* Info = Data->GetParentPlugin(Itr);

			if (CSCount < 370)
			{
				Desc = "[CS] ";				// 369 vanilla commands
				Source = CommandInfo::SourceType::e_Vanilla;
			}
			else if (Info)
			{
				PluginName = gcnew String(Info->name);
				if (!String::Compare(PluginName, "OBSE_Kyoma_MenuQue", true))			PluginName = "MenuQue";
				else if (!String::Compare(PluginName, "OBSE_Elys_Pluggy", true))		PluginName = "Pluggy";

				Desc = "[" + PluginName + " v" + Info->version + "] ";
				Source = CommandInfo::SourceType::e_OBSE;
			}
			else
			{
				UInt32 OBSEVersion = Data->GetRequiredOBSEVersion(Itr);
				Desc = "[OBSE v" + OBSEVersion + "] ";
				Source = CommandInfo::SourceType::e_OBSE;
			}

			if (!String::Compare(gcnew String(Itr->helpText), "", true))		Desc += "No description";
			else																Desc += gcnew String(Itr->helpText);

			if (!String::Compare(gcnew String(Itr->shortName), "", true))		SH = "None";
			else																SH = gcnew String(Itr->shortName);

			ReturnType = Data->GetCommandReturnType(Itr);
			if (ReturnType == 6)												ReturnType = 0;

			Enumerables->AddLast(gcnew CommandInfo(Name, Desc, SH, Itr->numParams, Itr->needsParent, ReturnType, Source));

			CSCount++;
			Count++;
		}

		DebugPrint(String::Format("\tIntelliSense: Parsed {0} Commands", Count));
	}

	void IntelliSenseDatabase::InitializeGMSTDatabase(ComponentDLLInterface::IntelliSenseUpdateData* GMSTCollection)
	{
		for (int i = 0; i < GMSTCollection->GMSTCount; i++)
		{
			ComponentDLLInterface::GMSTData* Itr = &GMSTCollection->GMSTListHead[i];
			if (!Itr->IsValid())
				continue;

			if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Int)
				Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_Int, IntelliSenseItem::ItemType::e_GMST));
			else if (Itr->Type == ComponentDLLInterface::GlobalData::kType_Float)
				Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_Float, IntelliSenseItem::ItemType::e_GMST));
			else
				Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_String, IntelliSenseItem::ItemType::e_GMST));
		}

		DebugPrint(String::Format("\tIntelliSense: Parsed {0} Game Settings", GMSTCollection->GMSTCount));
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
		for each (IntelliSenseItem^% Itr in Enumerables)
		{
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd)
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

		for each (UserFunction^% Itr in UserFunctionList)
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

		for each (IntelliSenseItem^% Itr in Enumerables)
		{
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd)
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

		UpdateThreadTimerInterval = OPTIONS->FetchSettingAsInt("DatabaseUpdateInterval");
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
					if (RemoteVarItr->Current->GetType() == IntelliSenseItem::ItemType::e_RemoteVar)
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

	void NonActivatingImmovableAnimatedForm::FadeTimer_Tick( Object^ Sender, EventArgs^ E )
	{
		if (FadeOperation == FadeOperationType::e_FadeIn)
			this->Opacity += FadeTimer->Interval / (0.6 * 0.15 * 1000);
		else
			this->Opacity -= FadeTimer->Interval / (0.6 * 0.15 * 1000);

		if (this->Opacity >= 1.0 || this->Opacity <= 0.0)
		{
			FadeTimer->Stop();

			if (FadeOperation == FadeOperationType::e_FadeOut)
				this->Hide();
		}
	}

	void NonActivatingImmovableAnimatedForm::SetSize(Drawing::Size WindowSize)
	{
		ClientSize = WindowSize;

		WindowSize.Height += 3;
		MaximumSize = WindowSize;
		MinimumSize = WindowSize;
	}

	void NonActivatingImmovableAnimatedForm::ShowForm(Drawing::Point Position, IntPtr ParentHandle, bool Animate)
	{
		AllowMove = true;

		SetDesktopLocation(Position.X, Position.Y);
		if (this->Visible == false)
		{
			if (Animate)
				this->Opacity = 0.0;

			if (ParentHandle != IntPtr::Zero)
				Show(gcnew WindowHandleWrapper(ParentHandle));
			else
				Show();

			if (Animate)
			{
				FadeOperation = FadeOperationType::e_FadeIn;
				FadeTimer->Start();
			}
		}

		AllowMove = false;
	}

	void NonActivatingImmovableAnimatedForm::WndProc(Message% m)
	{
		const int WM_SYSCOMMAND = 0x0112;
		const int SC_MOVE = 0xF010;
		const int WM_MOVE = 0x003;
		const int WM_MOVING = 0x0216;

		switch(m.Msg)
		{
		case WM_MOVE:
		case WM_MOVING:
			if (!AllowMove)
				return;
			break;
		case WM_SYSCOMMAND:
			int Command = m.WParam.ToInt32() & 0xfff0;
			if (Command == SC_MOVE && !AllowMove)
				return;
			break;
		}

		Form::WndProc(m);
	}

	NonActivatingImmovableAnimatedForm::NonActivatingImmovableAnimatedForm() : Form()
	{
		AllowMove = false;
		FadeOperation = FadeOperationType::e_None;

		FadeTimer = gcnew Timer();
		FadeTimer->Interval = 10;
		FadeTimer->Tick += gcnew EventHandler(this, &NonActivatingImmovableAnimatedForm::FadeTimer_Tick);
		FadeTimer->Stop();
	}

	void NonActivatingImmovableAnimatedForm::HideForm(bool Animate)
	{
		if (this->Visible)
		{
			if (Animate)
			{
				FadeOperation = FadeOperationType::e_FadeOut;
				this->Opacity = 1.0;
				FadeTimer->Start();
			}
			else
				this->Hide();
		}
	}

	void NonActivatingImmovableAnimatedForm::Destroy()
	{
		FadeTimer->Stop();
		this->Close();
	}

	IntelliSenseInterface::IntelliSenseInterface(Object^% Parent)
	{
		IntelliSenseBox = gcnew NonActivatingImmovableAnimatedForm();
		LocalVariableDatabase = gcnew List<IntelliSenseItem^>();
		CurrentListContents = gcnew List<IntelliSenseItem^>();
		IntelliSenseList = gcnew ListView();

		if (IntelliSenseItemIcons->Images->Count == 0)
		{
			IntelliSenseItemIcons->TransparentColor = Color::White;
			IntelliSenseItemIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemEmpty"));
			IntelliSenseItemIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemCommand"));
			IntelliSenseItemIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemLocalVar"));
			IntelliSenseItemIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemRemoteVar"));
			IntelliSenseItemIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemUDF"));
			IntelliSenseItemIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemQuest"));
			IntelliSenseItemIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemGlobalVar"));
			IntelliSenseItemIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemGMST"));
		}

		IntelliSenseBox->FormBorderStyle = FormBorderStyle::SizableToolWindow;
		IntelliSenseBox->ShowInTaskbar = false;
		IntelliSenseBox->ShowIcon = false;
		IntelliSenseBox->ControlBox = false;
		IntelliSenseBox->Controls->Add(IntelliSenseList);
		IntelliSenseBox->Closing += gcnew CancelEventHandler(this, &IntelliSenseInterface::IntelliSenseBox_Cancel);

		IntelliSenseList->View = View::Details;
		IntelliSenseList->Dock = DockStyle::Fill;
		IntelliSenseList->MultiSelect = false;
		IntelliSenseList->SmallImageList = IntelliSenseItemIcons;
		IntelliSenseList->SelectedIndexChanged += gcnew EventHandler(this, &IntelliSenseInterface::IntelliSenseList_SelectedIndexChanged);
		IntelliSenseList->KeyDown += gcnew KeyEventHandler(this, &IntelliSenseInterface::IntelliSenseList_KeyDown);
		IntelliSenseList->MouseDoubleClick += gcnew MouseEventHandler(this, &IntelliSenseInterface::IntelliSenseList_MouseDoubleClick);
		IntelliSenseList->Location = Point(0, 0);
		IntelliSenseList->Font = gcnew Font("Consolas", 9, FontStyle::Regular);
		IntelliSenseList->LabelEdit = false;
		IntelliSenseList->AllowColumnReorder = false;
		IntelliSenseList->CheckBoxes = false;
		IntelliSenseList->FullRowSelect = true;
		IntelliSenseList->GridLines = false;
		IntelliSenseList->Sorting = SortOrder::None;
		IntelliSenseList->Columns->Add("IntelliSense Object", 200);
		IntelliSenseList->HeaderStyle = ColumnHeaderStyle::None;
		IntelliSenseList->HideSelection = false;

		InfoToolTip->AutoPopDelay = 5000;
		InfoToolTip->InitialDelay = 1000;
		InfoToolTip->ReshowDelay = 500;
		InfoToolTip->ToolTipIcon = ToolTipIcon::Info;

		Enabled = true;
		ParentEditor = Parent;
		LastOperation = Operation::e_Default;
		Destroying = false;

		IntelliSenseBox->SetSize(Size(0, 0));
		IntelliSenseBox->ShowForm(Point(0,0), IntelliSenseBox->Handle, false);
		IntelliSenseBox->Hide();

		RemoteScript = Script::NullScript;
		CallingObjectIsRef = false;
	}

	void IntelliSenseInterface::DisplayInfoToolTip(String^ Title, String^ Message, Point Location, IntPtr ParentHandle, UInt32 Duration)
	{
		HideInfoToolTip();

		InfoToolTip->Tag = ParentHandle;
		InfoToolTip->ToolTipTitle = Title;
		InfoToolTip->Show(Message, Control::FromHandle(ParentHandle), Location, Duration);
	}

	void IntelliSenseInterface::HideInfoToolTip()
	{
		try		// to account for disposed controls
		{
			if (InfoToolTip->Tag != nullptr && Control::FromHandle((IntPtr)InfoToolTip->Tag) != nullptr)
				InfoToolTip->Hide(Control::FromHandle((IntPtr)InfoToolTip->Tag));
			else
				InfoToolTip->Tag = nullptr;
		}
		catch (...)
		{
			InfoToolTip->Tag = nullptr;
		}
	}

	void IntelliSenseInterface::ShowInterface( IntelliSenseInterface::Operation DisplayOperation, bool ForceDisplay, bool ShowAllItems )
	{
		if (!Enabled && !ForceDisplay)
			return;

		UInt32 ItemCount = 0;

		IntelliSenseList->BeginUpdate();

		CleanupInterface();

		ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

		String^ Extract = ParentEditor->GetCurrentToken();

		switch (DisplayOperation)
		{
		case Operation::e_Default:
			if (Extract->Length >= OPTIONS->FetchSettingAsInt("ThresholdLength") || ForceDisplay)
			{
				for each (IntelliSenseItem^% Itr in LocalVariableDatabase)
				{
					if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr))
					{
						IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
						CurrentListContents->Add(Itr);
						ItemCount++;
					}
				}
				for each (IntelliSenseItem^% Itr in ISDB->Enumerables)
				{
					if ((Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd && !dynamic_cast<CommandInfo^>(Itr)->GetRequiresParent()) ||
						Itr->GetType() == IntelliSenseItem::ItemType::e_Quest ||
						Itr->GetType() == IntelliSenseItem::ItemType::e_GlobalVar ||
						Itr->GetType() == IntelliSenseItem::ItemType::e_GMST)
					{
						if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr))
						{
							IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
							CurrentListContents->Add(Itr);
							ItemCount++;
						}
					}
				}
			}
			break;
		case Operation::e_Call:
			for each (IntelliSenseItem^% Itr in ISDB->Enumerables)
			{
				if (Itr->GetType() == IntelliSenseItem::ItemType::e_UserFunct)
				{
					if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
					{
						IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
						CurrentListContents->Add(Itr);
						ItemCount++;
					}
				}
			}
			break;
		case Operation::e_Dot:
			if (ShowAllItems)
			{
				VariableInfo^ RefVar = LookupLocalVariableByIdentifier(Extract);
				if (RefVar != nullptr && RefVar->GetVariableType() == VariableInfo::VariableType::e_Ref)
				{
					CallingObjectIsRef = true;
				}
				else if (!String::Compare(Extract, "player", true))
				{
					CallingObjectIsRef = true;
				}
				else
				{
					CString CStr(Extract);						// extract = calling ref
					ComponentDLLInterface::ScriptData* Data = g_CSEInterface->CSEEditorAPI.LookupScriptableFormByEditorID(CStr.c_str());
					if (Data && !Data->IsValid())
					{
						LastOperation = Operation::e_Default;
						break;
					}
					else if (Data)
					{
						RemoteScript = ISDB->CacheRemoteScript(gcnew String(Data->ParentID), gcnew String(Data->Text));			// cache form data for subsequent calls
						CallingObjectIsRef = g_CSEInterface->CSEEditorAPI.GetIsFormReference(CStr.c_str());
					}
					else
					{
						CallingObjectIsRef = false;
						RemoteScript = Script::NullScript;
					}
					g_CSEInterface->DeleteNativeHeapPointer(Data, false);
				}
			}

			for (Script::VarListT::Enumerator^ RemoteVarItr = RemoteScript->GetVariableListEnumerator(); RemoteVarItr->MoveNext();)
			{
				if (RemoteVarItr->Current->GetType() == IntelliSenseItem::ItemType::e_RemoteVar)
				{
					if (RemoteVarItr->Current->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
					{
						IntelliSenseList->Items->Add(gcnew ListViewItem(RemoteVarItr->Current->GetIdentifier(), (int)RemoteVarItr->Current->GetType()));
						CurrentListContents->Add(RemoteVarItr->Current);
						ItemCount++;
					}
				}
			}

			for each (IntelliSenseItem^% Itr in ISDB->Enumerables)
			{
				if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd && CallingObjectIsRef)
				{
					if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
					{
						IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
						CurrentListContents->Add(Itr);
						ItemCount++;
					}
				}
			}
			break;
		case Operation::e_Assign:
			for each (IntelliSenseItem^% Itr in LocalVariableDatabase)
			{
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
				{
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					CurrentListContents->Add(Itr);
					ItemCount++;
				}
			}

			for each (IntelliSenseItem^% Itr in ISDB->Enumerables)
			{
				if (Itr->GetType() == IntelliSenseItem::ItemType::e_Quest ||
					Itr->GetType() == IntelliSenseItem::ItemType::e_GlobalVar)
				{
					if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
					{
						IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
						CurrentListContents->Add(Itr);
						ItemCount++;
					}
				}
			}
			break;
		}

		IntelliSenseList->EndUpdate();

		if (ItemCount == 1 && !String::Compare(CurrentListContents[0]->GetIdentifier(), Extract, true))
			return;		// do not show when enumerable == extract

		if (ItemCount > 0)
		{
			Point Loc = ParentEditor->GetCaretLocation();
			Loc.X += 3; Loc.Y += OPTIONS->FetchSettingAsInt("FontSize") + 5;

			if (ItemCount > 8)
				ItemCount = 8;

			Size DisplaySize = ::Size(240, 158 - ((8 - ItemCount) * 18));
			IntelliSenseBox->SetSize(DisplaySize);
			IntelliSenseBox->ShowForm(ParentEditor->GetScreenPoint(Loc), ParentEditor->GetControlBoxHandle(), !IntelliSenseBox->Visible);

			ParentEditor->Focus();
			IntelliSenseList->Items[0]->Selected = true;
		}
		else
			HideInterface();

		LastOperation = DisplayOperation;
	}

	VariableInfo^ IntelliSenseInterface::LookupLocalVariableByIdentifier(String^% Identifier)
	{
		for each (IntelliSenseItem^% Itr in LocalVariableDatabase)
		{
			if (!String::Compare(Itr->GetIdentifier(), Identifier, true))
			{
				return dynamic_cast<VariableInfo^>(Itr);
			}
		}
		return nullptr;
	}

	void IntelliSenseInterface::IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E)
	{
		ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

		if (Visible)
		{
			if (GetListViewSelectedItemIndex(IntelliSenseList) == -1)
				return;

			Point Loc = Point(IntelliSenseList->Size.Width + 17, 0);

			DisplayInfoToolTip(CurrentListContents[GetListViewSelectedItemIndex(IntelliSenseList)]->GetTypeIdentifier(),
						CurrentListContents[GetListViewSelectedItemIndex(IntelliSenseList)]->Describe(),
						Loc,
						IntelliSenseBox->Handle,
						15000);
		}
	}

	void IntelliSenseInterface::IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E)
	{
		switch (E->KeyCode)
		{
		case Keys::Escape:
			HideInterface();
			break;
		case Keys::Tab:
		case Keys::Enter:
			PickSelection();
			break;
		}
	}

	void IntelliSenseInterface::IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E)
	{
		if (GetListViewSelectedItemIndex(IntelliSenseList) == -1)
			return;

		PickSelection();
	}

	void IntelliSenseInterface::IntelliSenseBox_Cancel(Object^ Sender, CancelEventArgs^ E)
	{
		if (!Destroying)
			E->Cancel = true;
	}

	void IntelliSenseInterface::ChangeCurrentSelection(IntelliSenseInterface::MoveDirection Direction)
	{
		int SelectedIndex = GetListViewSelectedItemIndex(IntelliSenseList);
		if (SelectedIndex == -1)		return;

		switch (Direction)
		{
		case MoveDirection::e_Down:
			if (SelectedIndex < (IntelliSenseList->Items->Count - 1))
			{
				IntelliSenseList->Items[SelectedIndex]->Selected = false;
				IntelliSenseList->Items[SelectedIndex + 1]->Selected = true;

				if (IntelliSenseList->TopItem->Index < IntelliSenseList->Items->Count - 1)
					IntelliSenseList->TopItem = IntelliSenseList->Items[IntelliSenseList->TopItem->Index + 1];
			}
			break;
		case MoveDirection::e_Up:
			if (SelectedIndex > 0)
			{
				IntelliSenseList->Items[SelectedIndex]->Selected = false;
				IntelliSenseList->Items[SelectedIndex - 1]->Selected = true;

				if (IntelliSenseList->TopItem->Index > 0 )
					IntelliSenseList->TopItem = IntelliSenseList->Items[IntelliSenseList->TopItem->Index - 1];
			}
			break;
		}
	}

	void IntelliSenseInterface::UpdateLocalVariableDatabase()
	{
		ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

		IntelliSenseDatabase::ParseScript(ParentEditor->GetScriptText(), gcnew Boxer(this));
	}

	void IntelliSenseInterface::PickSelection()
	{
		ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

		String^ Result;
		ParentEditor->Focus();

		if (GetListViewSelectedItemIndex(IntelliSenseList) != -1)
		{
			Result = CurrentListContents[GetListViewSelectedItemIndex(IntelliSenseList)]->GetIdentifier();
			CleanupInterface();
			HideInterface();
		}
		else
			return;

		try
		{
			NativeWrapper::LockWindowUpdate(ParentEditor->GetEditorBoxHandle());
			ParentEditor->SetCurrentToken(Result);
		}
		finally
		{
			NativeWrapper::LockWindowUpdate(IntPtr::Zero);
		}
	}

	void IntelliSenseInterface::HideInterface()
	{
		IntelliSenseBox->HideForm(true);
		HideInfoToolTip();
	}

	void IntelliSenseInterface::CleanupInterface()
	{
		IntelliSenseList->Items->Clear();
		CurrentListContents->Clear();
	}

	bool IntelliSenseInterface::ShowQuickInfoTip(String^ MainToken, String^ ParentToken, Point TipLoc)
	{
		if (OPTIONS->FetchSettingAsInt("UseQuickView") == 0)
			return false;

		ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);
		CString CStr(ParentToken);
		ComponentDLLInterface::ScriptData* Data = g_CSEInterface->CSEEditorAPI.LookupScriptableFormByEditorID(CStr.c_str());
		if (Data && Data->IsValid())
			ISDB->CacheRemoteScript(gcnew String(Data->ParentID), gcnew String(Data->Text));

		g_CSEInterface->DeleteNativeHeapPointer(Data, false);
		IntelliSenseItem^ Item = ISDB->LookupRemoteScriptVariable(ParentToken, MainToken);

		if (Item == nullptr)
			Item = LookupLocalVariableByIdentifier(MainToken);
		if (Item == nullptr)
		{
			for each (IntelliSenseItem^% Itr in ISDB->Enumerables)
			{
				if (!String::Compare(Itr->GetIdentifier(), MainToken, true))
				{
					Item = Itr;
					break;
				}
			}
		}
		if (Item != nullptr)
		{
			TipLoc.Y += OPTIONS->FetchSettingAsInt("FontSize") + 5;
			DisplayInfoToolTip(Item->GetTypeIdentifier(),
						Item->Describe(),
						TipLoc,
						ParentEditor->GetEditorBoxHandle(),
						8000);
			return true;
		}
		else
			return false;
	}

	bool IntelliSenseInterface::ShowQuickViewTooltip(String^ MainToken, String^ ParentToken)
	{
		ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

		return ShowQuickInfoTip(MainToken, ParentToken, ParentEditor->GetCaretLocation());
	}

	bool IntelliSenseInterface::ShowQuickViewTooltip(String^ MainToken, String^ ParentToken, Point MouseLocation)
	{
		return ShowQuickInfoTip(MainToken, ParentToken, MouseLocation);
	}

	void IntelliSenseInterface::Destroy()
	{
		Destroying = true;
		delete IntelliSenseBox;
	}
}