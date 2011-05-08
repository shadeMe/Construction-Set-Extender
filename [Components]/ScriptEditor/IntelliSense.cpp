#include "Intellisense.h"
#include "[Common]\NativeWrapper.h"
#include "Globals.h"
#include "ScriptParser.h"
#include "ScriptEditor.h"
#include "PluginParser.h"
#include "[Common]\HandShakeStructs.h"

ScriptData*							g_ScriptDataPackage = new ScriptData();

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
	VarList = gcnew List<VariableInfo^>();
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
	URLMap = gcnew Dictionary<String^, String^>();
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

IntelliSenseDatabase::ParsedUpdateData^ IntelliSenseDatabase::DoUpdateDatabase()
{
	ParsedUpdateData^ Data = gcnew ParsedUpdateData();

	IntelliSenseUpdateData* DataHandlerData = NativeWrapper::ScriptEditor_BeginIntelliSenseDatabaseUpdate();

	for (ScriptData* Itr = DataHandlerData->ScriptListHead; Itr != DataHandlerData->ScriptListHead + DataHandlerData->ScriptCount; ++Itr)
	{
		if (!Itr->IsValid())		continue;

		Data->UDFList->AddLast(gcnew UserFunction(gcnew String(Itr->Text)));
	}

	for (QuestData* Itr = DataHandlerData->QuestListHead; Itr != DataHandlerData->QuestListHead + DataHandlerData->QuestCount; ++Itr)
	{
		if (!Itr->IsValid())		continue;

		Data->Enumerables->AddLast(gcnew Quest(gcnew String(Itr->EditorID), gcnew String(Itr->FullName), gcnew String(Itr->ScriptName)));
	}

	for (GlobalData* Itr = DataHandlerData->GlobalListHead; Itr != DataHandlerData->GlobalListHead + DataHandlerData->GlobalCount; ++Itr)
	{
		if (!Itr->IsValid())		continue;

		if (Itr->Type == GlobalData::kType_Int)
			Data->Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_Int, IntelliSenseItem::ItemType::e_GlobalVar));
		else if (Itr->Type == GlobalData::kType_Float)
			Data->Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_Float, IntelliSenseItem::ItemType::e_GlobalVar));
		else
			Data->Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_String, IntelliSenseItem::ItemType::e_GlobalVar));
	}

	for (GMSTData* Itr = DataHandlerData->GMSTListHead; Itr != DataHandlerData->GMSTListHead + DataHandlerData->GMSTCount; ++Itr)
	{
		if (!Itr->IsValid())		continue;

		if (Itr->Type == GlobalData::kType_Int)
			Data->Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_Int, IntelliSenseItem::ItemType::e_GMST));
		else if (Itr->Type == GlobalData::kType_Float)
			Data->Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_Float, IntelliSenseItem::ItemType::e_GMST));
		else
			Data->Enumerables->AddLast(gcnew VariableInfo(gcnew String(Itr->EditorID), gcnew String(""), VariableInfo::VariableType::e_String, IntelliSenseItem::ItemType::e_GMST));
	}


	for each (IntelliSenseItem^ Itr in Enumerables)
	{
		if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd)
			Data->Enumerables->AddLast(Itr);
	}

	for each (UserFunction^% Itr in Data->UDFList)
	{
		Data->Enumerables->AddLast(gcnew UserFunctionDelegate(Itr));
	}

	NativeWrapper::ScriptEditor_EndIntelliSenseDatabaseUpdate(DataHandlerData);

	return Data;
}

void IntelliSenseDatabase::PostUpdateDatabase(ParsedUpdateData^ Data)
{
	UserFunctionList->Clear();
	Enumerables->Clear();
	RemoteScripts->Clear();

	UserFunctionList = Data->UDFList;
	Enumerables = Data->Enumerables;

	NativeWrapper::PrintToCSStatusBar(2, "IntelliSense database updated");
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
	E->Result = DoUpdateDatabase();
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
		PostUpdateDatabase(dynamic_cast<ParsedUpdateData^>(E->Result));
	}

	if (E->Result == nullptr)
	{
		NativeWrapper::PrintToCSStatusBar(2, "Error encountered while updating IntelliSense database !");
	}
}

void IntelliSenseDatabase::UpdateDatabase()
{
	if (!DatabaseUpdateThread->IsBusy)
	{
		DatabaseUpdateThread->RunWorkerAsync();
		NativeWrapper::PrintToCSStatusBar(2, "Updating IntelliSense database ...");
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
		Box->ScptBox->VarList->Clear();
		break;
	case Boxer::BoxType::e_IntelliSenseThingy:
		Box->ISBox->VarList->Clear();
		LocalVars = true;
		break;
	case Boxer::BoxType::e_UserFunct:
		Box->FunctBox->VarList->Clear();
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
			if (!ScriptTextParser->FindVariable(SecondToken)->IsValid() && SecondToken != "")
			{
				Comment = "";
				if (ScriptTextParser->Tokens->Count > 2 && ScriptTextParser->IsComment(ScriptTextParser->Tokens->Count) == 2)
				{
					Comment = ReadLine->Substring(ReadLine->IndexOf(";") + 1);
					ScriptTextParser->Tokenize(Comment, false);
					Comment = (ScriptTextParser->Indices->Count > 0)?Comment->Substring(ScriptTextParser->Indices[0]):Comment;
				}

				ScriptTextParser->Variables->AddLast(gcnew ScriptParser::VariableInfo(SecondToken, 0));
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
				case Boxer::BoxType::e_Script:
					Box->ScptBox->VarList->Add(gcnew VariableInfo(SecondToken, Comment, DataType, (LocalVars)?(IntelliSenseItem::ItemType::e_LocalVar):(IntelliSenseItem::ItemType::e_RemoteVar)));
					break;
				case Boxer::BoxType::e_IntelliSenseThingy:
					Box->ISBox->VarList->Add(gcnew VariableInfo(SecondToken, Comment, DataType, (LocalVars)?(IntelliSenseItem::ItemType::e_LocalVar):(IntelliSenseItem::ItemType::e_RemoteVar)));
					break;
				case Boxer::BoxType::e_UserFunct:
					Box->FunctBox->VarList->Add(gcnew VariableInfo(SecondToken, Comment, DataType, (LocalVars)?(IntelliSenseItem::ItemType::e_LocalVar):(IntelliSenseItem::ItemType::e_RemoteVar)));
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
			if (Box->Type == Boxer::BoxType::e_UserFunct)
			{
				if (!String::Compare(SecondToken, "function", true))
				{
					String^ ParamList = ReadLine->Substring(ReadLine->IndexOf("{"), ReadLine->IndexOf("}") - ReadLine->IndexOf("{"));
					ScriptTextParser->Tokenize(ParamList, false);
					int ParamIdx = 0;
					for each (String^% Itr in ScriptTextParser->Tokens)
					{
						int VarIdx = 0;
						for each (ScriptParser::VariableInfo^% ItrEx in ScriptTextParser->Variables)
						{
							if (!String::Compare(ItrEx->VarName, Itr, true) && ParamIdx < 10)
							{
								Box->FunctBox->Parameters->SetValue(VarIdx, ParamIdx);
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
			if (Box->Type == Boxer::BoxType::e_UserFunct)
			{
				Box->FunctBox->ReturnVar = -9;						// ambiguous
				int VarIdx = 0;
				for each (ScriptParser::VariableInfo^% Itr in ScriptTextParser->Variables)
				{
					if (!String::Compare(SecondToken, Itr->VarName, true))
					{
						Box->FunctBox->ReturnVar = VarIdx;
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
		Box->ScptBox->Name = ScriptName;
		Box->ScptBox->Description = Description;
		break;
	case Boxer::BoxType::e_UserFunct:
		Box->FunctBox->Name = ScriptName;
		Box->FunctBox->Description = Description;
		break;
	}
}

void IntelliSenseDatabase::ParseCommandTable(CommandTableData* Data)
{
	try
	{
		String^ Name, ^Desc, ^SH, ^PluginName;
		int Count = 0, ReturnType = 0, CSCount = 0;
		CommandInfo::SourceType Source;

		for (const CommandInfoCLI* Itr = Data->CommandTableStart; Itr != Data->CommandTableEnd; ++Itr)
		{
			Name = gcnew String(Itr->longName);
			if (!String::Compare(Name, "", true))	continue;

			const CommandTableData::PluginInfo* Info = Data->GetParentPlugin(Itr);

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
																				Desc = "[OBSE] ";
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

		DebugPrint(String::Format("\tSuccessfully parsed {0} commands!", Count));
	}
	catch (Exception^ E)
	{
		DebugPrint("Exception raised!\n\tMessage: " + E->Message, true);
	}
}

void IntelliSenseDatabase::AddToURLMap(String^% CmdName, String^% URL)
{
	for each (KeyValuePair<String^, String^>% Itr in URLMap)
	{
		if (!String::Compare(CmdName, Itr.Key, true))
		{
			return;
		}
	}

	URLMap->Add(CmdName, URL);
	DebugPrint("Bound " + CmdName + " to URL " + URL);
}

String^	IntelliSenseDatabase::GetCommandURL(String^% CmdName)
{
	String^ Result = nullptr;
	for each (KeyValuePair<String^, String^>% Itr in URLMap)
	{
		if (!String::Compare(CmdName, Itr.Key, true))
		{
			Result = Itr.Value;
			break;
		}
	}
	return Result;
}

String^	IntelliSenseDatabase::SanitizeCommandName(String^% CmdName)
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

Script^ IntelliSenseDatabase::GetRemoteScript(String^ BaseEditorID, String^ ScriptText)
{
	for each (KeyValuePair<String^, Script^>% Itr in RemoteScripts)
	{
		if (!String::Compare(BaseEditorID, Itr.Key, true))
		{
			return Itr.Value;
		}
	}

	DebugPrint("Caching scriptable object " + BaseEditorID + "'s script");
	RemoteScripts->Add(BaseEditorID, gcnew Script(ScriptText));
	return GetRemoteScript(BaseEditorID, nullptr);
}

bool IntelliSenseDatabase::IsUDF(String^% Name)
{
	bool Result = false;

	for each (UserFunction^% Itr in UserFunctionList)
	{
		if (!String::Compare(Name, Itr->Name, true))
		{
			Result = true;
			break;
		}
	}

	return Result;
}

bool IntelliSenseDatabase::IsCommand(String^% Name)
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

void IntelliSenseDatabase::InitializeDatabaseUpdateTimer()
{
	static bool DatabaseTimerInitialized = false;
	if (DatabaseTimerInitialized)	return;

	UpdateThreadTimerInterval = OPTIONS->FetchSettingAsInt("DatabaseUpdateInterval");
	DatabaseUpdateTimer->Start();
	DatabaseTimerInitialized = true;
}

void NonActivatingImmovableForm::SetSize(Drawing::Size WindowSize)
{
	MaximumSize = Drawing::Size(640, 480);
	MinimumSize = Drawing::Size(640, 480);

	ClientSize = WindowSize;

	WindowSize.Height += 3;
	MaximumSize = WindowSize;
	MinimumSize = WindowSize;
}

void NonActivatingImmovableForm::ShowAtLocation(Drawing::Point Position, IntPtr ParentHandle)
{
	AllowMove = true;

	SetDesktopLocation(Position.X, Position.Y);

	if (ParentHandle != IntPtr::Zero)
		Show(gcnew WindowHandleWrapper(ParentHandle));

	NativeWrapper::ShowNonActivatingWindow(this, IntPtr::Zero);

	AllowMove = false;
}

void NonActivatingImmovableForm::WndProc(Message% m)
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

// INTELLISENSE THINGY

IntelliSenseThingy::IntelliSenseThingy(Object^% Parent)
{
	IntelliSenseBox = gcnew NonActivatingImmovableForm();
	VarList = gcnew List<IntelliSenseItem^>();
	ListContents = gcnew List<IntelliSenseItem^>();
	IntelliSenseList = gcnew ListView();

	if (Icons->Images->Count == 0)
	{
		Icons->TransparentColor = Color::White;
		Icons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemEmpty"));
		Icons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemCommand"));
		Icons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemLocalVar"));
		Icons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemRemoteVar"));
		Icons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemUDF"));
		Icons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemQuest"));
		Icons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemGlobalVar"));
		Icons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemGMST"));	
	}

	IntelliSenseBox->FormBorderStyle = FormBorderStyle::SizableToolWindow;
	IntelliSenseBox->ShowInTaskbar = false;
	IntelliSenseBox->ShowIcon = false;
	IntelliSenseBox->ControlBox = false;
	IntelliSenseBox->Controls->Add(IntelliSenseList);
	IntelliSenseBox->Closing += gcnew CancelEventHandler(this, &IntelliSenseThingy::IntelliSenseBox_Cancel);

	IntelliSenseList->View = View::Details;
	IntelliSenseList->Dock = DockStyle::Fill;
	IntelliSenseList->MultiSelect = false;
	IntelliSenseList->SmallImageList = Icons;
	IntelliSenseList->SelectedIndexChanged += gcnew EventHandler(this, &IntelliSenseThingy::IntelliSenseList_SelectedIndexChanged);
	IntelliSenseList->KeyDown += gcnew KeyEventHandler(this, &IntelliSenseThingy::IntelliSenseList_KeyDown);
	IntelliSenseList->MouseDoubleClick += gcnew MouseEventHandler(this, &IntelliSenseThingy::IntelliSenseList_MouseDoubleClick);
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

	InfoTip->AutoPopDelay = 5000;
	InfoTip->InitialDelay = 1000;
	InfoTip->ReshowDelay = 500;
	InfoTip->ToolTipIcon = ToolTipIcon::Info;

	Enabled = true;
	ParentEditor = Parent;
	LastOperation = Operation::e_Default;
	Destroying = false;
}

void IntelliSenseThingy::Initialize(IntelliSenseThingy::Operation Op, bool Force, bool InitAll)
{
	if (!Enabled && !Force)		return;

	UInt32 ItemCount = 0;
	Cleanup();

	IntelliSenseList->BeginUpdate();

	ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

	String^ Extract = ParentEditor->GetCurrentToken();

	switch (Op)
	{
	case Operation::e_Default:
		if (Extract->Length >= OPTIONS->FetchSettingAsInt("ThresholdLength") || Force)
		{
			for each (IntelliSenseItem^% Itr in VarList)
			{
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr))
				{
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					ListContents->Add(Itr);
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
						ListContents->Add(Itr);
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
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll)
				{
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					ListContents->Add(Itr);
					ItemCount++;
				}
			}
		}
		break;
	case Operation::e_Dot:
		if (InitAll)
		{
			RemoteScript = Script::NullScript;
			VariableInfo^ RefVar = GetLocalVar(Extract);
			if (RefVar != nullptr && RefVar->GetType() == VariableInfo::VariableType::e_Ref)
			{
				IsObjRefr = true;
			}
			else if (!String::Compare(Extract, "player", true))
			{
				IsObjRefr = true;
			}
			else
			{
				CStringWrapper^ CStr = gcnew CStringWrapper(Extract);														// extract = calling ref
				ScriptData* Data = NativeWrapper::FetchScriptFromForm(CStr->String());

				if (!Data->IsValid())
				{
					LastOperation = Operation::e_Default;
					break;
				}
				else
				{
					RemoteScript = ISDB->GetRemoteScript(gcnew String(Data->ParentID), gcnew String(Data->Text));			// cache form data for subsequent calls
					IsObjRefr = NativeWrapper::IsFormAnObjRefr(CStr->String());
				}
			}
		}

		for each (IntelliSenseItem^% Itr in RemoteScript->VarList)
		{
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_RemoteVar)
			{
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll)
				{
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					ListContents->Add(Itr);
					ItemCount++;
				}
			}
		}

		for each (IntelliSenseItem^% Itr in ISDB->Enumerables)
		{
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd && IsObjRefr)
			{
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll)
				{
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					ListContents->Add(Itr);
					ItemCount++;
				}
			}
		}
		break;
	case Operation::e_Assign:
		for each (IntelliSenseItem^% Itr in VarList)
		{
			if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll)
			{
				IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
				ListContents->Add(Itr);
				ItemCount++;
			}
		}

		for each (IntelliSenseItem^% Itr in ISDB->Enumerables)
		{
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_Quest ||
				Itr->GetType() == IntelliSenseItem::ItemType::e_GlobalVar)
			{
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll)
				{
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					ListContents->Add(Itr);
					ItemCount++;
				}
			}
		}
		break;
	}

	IntelliSenseList->EndUpdate();

	if (ItemCount == 1 && !String::Compare(ListContents[0]->GetIdentifier(), Extract, true))	return;		// do not show when enumerable == extract

	if (ItemCount > 0)
	{
		Point Loc = ParentEditor->GetCaretLocation();
		Loc.X += 3; Loc.Y += OPTIONS->FetchSettingAsInt("FontSize") + 5;

		if (ItemCount > 8)
			ItemCount = 8;

		Size DisplaySize = ::Size(240, 155 - ((8 - ItemCount) * 18));
		IntelliSenseBox->SetSize(DisplaySize);
		IntelliSenseBox->ShowAtLocation(ParentEditor->GetScreenPoint(Loc), ParentEditor->GetControlBoxHandle());

		IntelliSenseList->Items[0]->Selected = true;
		ParentEditor->Focus();
	}

	LastOperation = Op;
}

VariableInfo^ IntelliSenseThingy::GetLocalVar(String^% Identifier)
{
	for each (IntelliSenseItem^% Itr in VarList)
	{
		if (!String::Compare(Itr->GetIdentifier(), Identifier, true))
		{
			return dynamic_cast<VariableInfo^>(Itr);
		}
	}
	return nullptr;
}

void IntelliSenseThingy::IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E)
{
	ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

	if (IsVisible())
	{
		if (GetSelectedIndex() == -1)
			return;

		Point Loc = ParentEditor->GetCaretLocation();
		Loc.X += 9 + 250; Loc.Y += OPTIONS->FetchSettingAsInt("FontSize") + 5 + 28;

		InfoTip->ToolTipTitle = ListContents[GetSelectedIndex()]->GetTypeIdentifier();
		InfoTip->Show(ListContents[GetSelectedIndex()]->Describe(), Control::FromHandle(ParentEditor->GetControlBoxHandle()), Loc);
	}
}

void IntelliSenseThingy::IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	if (E->KeyCode == Keys::Escape)
		Hide();
}

void IntelliSenseThingy::IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E)
{
	if (GetSelectedIndex() == -1)
		return;

	PickIdentifier();
}

void IntelliSenseThingy::IntelliSenseBox_Cancel(Object^ Sender, CancelEventArgs^ E)
{
	if (!Destroying)
		E->Cancel = true;
}

void IntelliSenseThingy::MoveIndex(IntelliSenseThingy::Direction Direction)
{
	int SelectedIndex = GetSelectedIndex();
	if (SelectedIndex == -1)		return;

	switch (Direction)
	{
	case Direction::e_Down:
		if (SelectedIndex < (IntelliSenseList->Items->Count - 1))
		{
			IntelliSenseList->Items[SelectedIndex]->Selected = false;
			IntelliSenseList->Items[SelectedIndex + 1]->Selected = true;

			if (IntelliSenseList->TopItem->Index < IntelliSenseList->Items->Count - 1)
				IntelliSenseList->TopItem = IntelliSenseList->Items[IntelliSenseList->TopItem->Index + 1];
		}
		break;
	case Direction::e_Up:
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

void IntelliSenseThingy::UpdateLocalVars()
{
	ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

	IntelliSenseDatabase::ParseScript(const_cast<String^>(ParentEditor->GetScriptText()), gcnew Boxer(this));
}

void IntelliSenseThingy::PickIdentifier()
{
	ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

	String^ Result;
	ParentEditor->Focus();

	if (GetSelectedIndex() != -1)
	{
		Result = ListContents[GetSelectedIndex()]->GetIdentifier();
		Cleanup();
		Hide();
	}

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

void IntelliSenseThingy::Hide()
{
	IntelliSenseBox->Hide();
	InfoTip->Hide(Control::FromHandle(IntelliSenseList->Parent->Handle));
}

void IntelliSenseThingy::Cleanup()
{
	Hide();
	IntelliSenseList->Items->Clear();
	ListContents->Clear();
}

int	IntelliSenseThingy::GetSelectedIndex()
{
	int Result = -1;
	for each (int SelectedIndex in IntelliSenseList->SelectedIndices)
	{
		Result = SelectedIndex;
		break;
	}
	return Result;
}

bool IntelliSenseThingy::ShowQuickInfoTip(String^ TextUnderMouse, Point TipLoc)
{
	if (OPTIONS->FetchSettingAsInt("UseQuickView") == 0)
		return false;

	ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

	IntelliSenseItem^ Item = GetLocalVar(TextUnderMouse);

	if (Item == nullptr)
	{
		for each (IntelliSenseItem^% Itr in ISDB->Enumerables)
		{
			if (!String::Compare(Itr->GetIdentifier(), TextUnderMouse, true))
			{
				Item = Itr;
				break;
			}
		}
	}
	if (Item != nullptr)
	{
		TipLoc.Y += OPTIONS->FetchSettingAsInt("FontSize") + 8;
		InfoTip->ToolTipTitle = Item->GetTypeIdentifier();
		InfoTip->Show(Item->Describe(), Control::FromHandle(ParentEditor->GetEditorBoxHandle()), TipLoc, 8000);
		return true;
	}
	else
		return false;
}

bool IntelliSenseThingy::QuickView(String^ TextUnderMouse)
{
	ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(this->ParentEditor);

	return ShowQuickInfoTip(TextUnderMouse, ParentEditor->GetCaretLocation());
}

bool IntelliSenseThingy::QuickView(String^ TextUnderMouse, Point MouseLocation)
{
	return ShowQuickInfoTip(TextUnderMouse, MouseLocation);
}