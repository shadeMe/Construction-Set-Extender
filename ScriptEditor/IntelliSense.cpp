#include "Intellisense.h"
#include "Common\NativeWrapper.h"
#include "Globals.h"
#include "ScriptParser.h"
#include "ScriptEditor.h"
#include "PluginParser.h"
#include "Common\HandShakeStructs.h"


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
	while (ParamIdx < 10) {
		int VarIdx = (int)Parameters->GetValue(ParamIdx);
		if (VarIdx == -1)	break;

		String^% Comment = VarList[VarIdx]->Comment, ^% Name = VarList[VarIdx]->Name;
		Scratch += "\n\t" + ((Comment == "")?Name:Comment) + " [" + VariableInfo::TypeIdentifier[(int)VarList[VarIdx]->Type] + "]";
		ParamIdx++;
	}

	Description += Name + "\n\nDescription: " + this->Description + "\n" + ParamIdx + " Arguments" + Scratch + "\n\n";
	if (ReturnVar == -1)			Description += "Does not return a value";
	else if (ReturnVar == -9)		Description += "Return Type: Ambiguous";
	else							Description += "Return Type: " + VariableInfo::TypeIdentifier[(int)VarList[ReturnVar]->Type];

	return Description;
}

void UserFunction::DumpData()
{
	DebugPrint("\n");
	DebugPrint(Name + "\n" + this->Description + "\n");
}

IntelliSenseDatabase^% IntelliSenseDatabase::GetSingleton()
{
	if (Singleton == nullptr) {
		Singleton = gcnew IntelliSenseDatabase();
	}
	return Singleton;
}

IntelliSenseDatabase::IntelliSenseDatabase()
{
	Enumerables = gcnew LinkedList<IntelliSenseItem^>();
	UserFunctionList = gcnew LinkedList<UserFunction^>();						
	QuestList = nullptr;
	URLMap = gcnew Dictionary<String^, String^>();
	RemoteScripts = gcnew Dictionary<String^, Script^>();
	ActiveScriptRecords = gcnew LinkedList<String^>();

	PluginParserThread = gcnew BackgroundWorker();
	PluginParserThread->DoWork += gcnew DoWorkEventHandler(this, &IntelliSenseDatabase::PluginParserThread_DoWork);
	PluginParserThread->RunWorkerCompleted += gcnew RunWorkerCompletedEventHandler(this, &IntelliSenseDatabase::PluginParserThread_RunWorkerCompleted);

	DebugPrint("Initialized IntelliSense");
}

IntelliSenseDatabase::ParsedPluginData^ IntelliSenseDatabase::DoUpdateDatabase(String^ PluginName)
{
	ParsedPluginData^ Data = nullptr;
	List<PluginParser^>^ ParsedPlugins = gcnew List<PluginParser^>();

	PluginParser^ ActivePlugin = gcnew PluginParser(PluginName);

	if (!ActivePlugin->IsUsable())		return Data;

	for each (String^% Itr in ActivePlugin->PluginHeader->MasterList) {
		if (!String::Compare(Itr, "Oblivion.esm"))	continue;
		ParsedPlugins->Add(gcnew PluginParser(Itr));
	}
	ParsedPlugins->Add(ActivePlugin);

	Data = gcnew ParsedPluginData();
	Dictionary<UInt32, SCPTRecord^>^ ProcessedScriptRecords = gcnew Dictionary<UInt32, SCPTRecord^>();
	Dictionary<UInt32, QUSTRecord^>^ ProcessedQuestRecords = gcnew Dictionary<UInt32, QUSTRecord^>();

	for each (PluginParser^% Itr in ParsedPlugins) {
		if (!Itr->IsUsable())			continue;
																						// create a list of overridden records
		for each (SCPTRecord^% ScriptRecord in Itr->ScriptRecords) {
			if (ProcessedScriptRecords->ContainsKey(ScriptRecord->GetFormID()))				ProcessedScriptRecords[ScriptRecord->GetFormID()] = ScriptRecord;
			else																			ProcessedScriptRecords->Add(ScriptRecord->GetFormID(), ScriptRecord);
		}

		for each (QUSTRecord^% QuestRecord in Itr->QuestRecords) {
			if (ProcessedQuestRecords->ContainsKey(QuestRecord->GetFormID()))				ProcessedQuestRecords[QuestRecord->GetFormID()] = QuestRecord;
			else																			ProcessedQuestRecords->Add(QuestRecord->GetFormID(), QuestRecord);
		}
	}

	for each (SCPTRecord^% ScriptRecord in ActivePlugin->ScriptRecords) {
		Data->ActiveScriptRecords->AddLast(ScriptRecord->GetEditorID());
	}

																						// parse overriden list
	for each (KeyValuePair<UInt32, SCPTRecord^>% Itr in ProcessedScriptRecords) {
		if (Itr.Value->Type == SCPTRecord::ScriptType::e_Object && Itr.Value->UDF)
			Data->UDFList->AddLast(gcnew UserFunction(Itr.Value->ScriptText));
	}

	for each (KeyValuePair<UInt32, QUSTRecord^>% Itr in ProcessedQuestRecords) {
		for each (KeyValuePair<UInt32, SCPTRecord^>% ItrEx in ProcessedScriptRecords) {
			if (ItrEx.Value->Type == SCPTRecord::ScriptType::e_Quest) {
				String^ QFID = Itr.Value->QuestScript.ToString("x8")->Substring(2),
						^SFID = ItrEx.Value->GetFormID().ToString("x8")->Substring(2);
				if (!String::Compare(QFID, SFID)) {
					Data->QuestList->AddLast(gcnew Quest(ItrEx.Value->ScriptText, Itr.Value->GetEditorID()));
					break;
				}
			}
		}
	}

	DebugPrint("PluginParser thread ended successfully!", false);
	return Data;
}

void IntelliSenseDatabase::PostUpdateDatabase(ParsedPluginData^ Data)
{
	if (UserFunctionList != nullptr)	UserFunctionList->Clear();
										UserFunctionList = Data->UDFList;
	if (QuestList != nullptr)			QuestList->Clear();
										QuestList = Data->QuestList;
	ActiveScriptRecords = Data->ActiveScriptRecords;

	LinkedList<IntelliSenseItem^>^ EnumerablesClone = gcnew LinkedList<IntelliSenseItem^>();
	for each (IntelliSenseItem^ Itr in Enumerables) {
		if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd)
			EnumerablesClone->AddLast(Itr);
	}
	Enumerables->Clear(), Enumerables = EnumerablesClone;

	for each (UserFunction^% Itr in UserFunctionList) {
		Enumerables->AddLast(gcnew UserFunctionDelegate(Itr));
	}
	for each (Quest^% Itr in QuestList) {
		Enumerables->AddLast(gcnew QuestDelegate(Itr));
	}

	RemoteScripts->Clear();

	NativeWrapper::PrintToCSStatusBar(2, "IntelliSense database updated");
	DebugPrint("IntelliSense database updated");
}

void IntelliSenseDatabase::PluginParserThread_DoWork(Object^ Sender, DoWorkEventArgs^ E)
{
	E->Result = DoUpdateDatabase(dynamic_cast<String^>(E->Argument));
}

void IntelliSenseDatabase::PluginParserThread_RunWorkerCompleted(Object^ Sender, RunWorkerCompletedEventArgs^ E)
{
	if (E->Error != nullptr)
		DebugPrint("The PluginParser thread raised an exception!\n\tException: " + E->Error->Message, true);
	else if (E->Cancelled)
		DebugPrint("Huh?! PluginParser thread was cancelled", true);
	else if (E->Result == nullptr)
		DebugPrint("Something seriously went wrong when parsing the loaded plugins!", true);
	else
		PostUpdateDatabase(dynamic_cast<ParsedPluginData^>(E->Result));

	if (E->Result == nullptr)
		NativeWrapper::PrintToCSStatusBar(2, "Error encountered while updating IntelliSense database !");
}

void IntelliSenseDatabase::UpdateDatabase(String^ PluginName)
{
	if (PluginName == "") {
		String^ Message = "Couldn't extract plugin name from window title - No active plugin?";
		DebugPrint(Message);
		NativeWrapper::PrintToCSStatusBar(2, Message);
		return;
	}

	if (!PluginParserThread->IsBusy) {
		DebugPrint("PluginParser thread started");
		PluginParserThread->RunWorkerAsync(PluginName);
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
	case Boxer::BoxType::e_SyntaxBox:
		Box->ISBox->VarList->Clear();
		LocalVars = true;
		break;
	case Boxer::BoxType::e_UserFunct:
		Box->FunctBox->VarList->Clear();
		break;
	}

	while (ReadLine != nullptr) {
		ScriptTextParser->Tokenize(ReadLine, false);
		if (!ScriptTextParser->Valid) {
			ReadLine = TextParser->ReadLine();
			continue;
		}
		
		if (ScriptTextParser->Tokens[0][0] == ';' && ReadLine->IndexOf(";<CSE") != 0 && ReadLine->IndexOf(";</CSE") != 0 && GrabDef)		Description += ReadLine->Substring(ScriptTextParser->Indices[0] + 1) + "\n";

		FirstToken = ScriptTextParser->Tokens[0], 
		SecondToken = (ScriptTextParser->Tokens->Count > 1)?ScriptTextParser->Tokens[1]:"";

		ScriptParser::TokenType Type = ScriptTextParser->GetTokenType(FirstToken);

		switch (Type) 
		{
		case ScriptParser::TokenType::e_Variable:
			GrabDef = false;
			if (ScriptTextParser->FindVariable(SecondToken)->VarName == "@nullptr" && SecondToken != "") {
				Comment = "";
				if (ScriptTextParser->Tokens->Count > 2 && ScriptTextParser->IsComment(ScriptTextParser->Tokens->Count) == 2) {
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
				case Boxer::BoxType::e_SyntaxBox:
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
			if (Box->Type == Boxer::BoxType::e_UserFunct) {
				if (!String::Compare(SecondToken, "function", true)) {
					String^ ParamList = ReadLine->Substring(ReadLine->IndexOf("{"), ReadLine->IndexOf("}") - ReadLine->IndexOf("{"));
					ScriptTextParser->Tokenize(ParamList, false);
					int ParamIdx = 0;
					for each (String^% Itr in ScriptTextParser->Tokens) {
						int VarIdx = 0;
						for each (ScriptParser::VariableInfo^% ItrEx in ScriptTextParser->Variables) {
							if (!String::Compare(ItrEx->VarName, Itr, true) && ParamIdx < 10) {
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
			if (Box->Type == Boxer::BoxType::e_UserFunct) {
				Box->FunctBox->ReturnVar = -9;						// ambiguous
				int VarIdx = 0;
				for each (ScriptParser::VariableInfo^% Itr in ScriptTextParser->Variables) {
					if (!String::Compare(SecondToken, Itr->VarName, true)) {
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
	try {
	String^ Name, ^Desc, ^SH, ^PluginName;
	int Count = 0, ReturnType = 0, CSCount = 0;

	for (const CommandInfoCLI* Itr = Data->CommandTableStart; Itr != Data->CommandTableEnd; ++Itr) {		
		Name = gcnew String(Itr->longName);
		if (!String::Compare(Name, "", true))	continue;

		const CommandTableData::PluginInfo* Info = Data->GetParentPlugin(Itr);

		if (CSCount < 370)													Desc = "[CS] ";				// 369 vanilla commands
		else if (Info) {
			PluginName = gcnew String(Info->name);
			if (!String::Compare(PluginName, "OBSE_Kyoma_MenuQue", true))			PluginName = "MenuQue";
			else if (!String::Compare(PluginName, "OBSE_Elys_Pluggy", true))		PluginName = "Pluggy";

																			Desc = "[" + PluginName + " v" + Info->version + "] ";
		}
		else																Desc = "[OBSE] ";

		if (!String::Compare(gcnew String(Itr->helpText), "", true))		Desc += "No description";
		else																Desc += gcnew String(Itr->helpText);

		if (!String::Compare(gcnew String(Itr->shortName), "", true))		SH = "None";
		else																SH = gcnew String(Itr->shortName);

		ReturnType = Data->GetCommandReturnType(Itr);
		if (ReturnType == 6)												ReturnType = 0;

#ifdef SHADECASE
	CmdName = String::Format("{0}", Char::ToLower(CmdName[0])) + CmdName->Substring(1);
#endif

		Enumerables->AddLast(gcnew CommandInfo(Name, Desc, SH, Itr->numParams, Itr->needsParent, ReturnType));

		CSCount++;
		Count++;
	}

	DebugPrint(String::Format("\tSuccessfully parsed {0} commands!", Count));
	} catch (Exception^ E) {
		DebugPrint("Exception raised!\n\tMessage: " + E->Message, true);
	}
}

void IntelliSenseDatabase::AddToURLMap(String^% CmdName, String^% URL)
{
	for each (KeyValuePair<String^, String^>% Itr in URLMap) {
		if (!String::Compare(CmdName, Itr.Key, true)) {
			return;
		}
	}

	URLMap->Add(CmdName, URL);
	DebugPrint("Added " + CmdName + " ; " + URL + " to URLMap");
}


String^	IntelliSenseDatabase::GetCommandURL(String^% CmdName)
{
	String^ Result = nullptr;
	for each (KeyValuePair<String^, String^>% Itr in URLMap) {
		if (!String::Compare(CmdName, Itr.Key, true)) {
			Result = Itr.Value;
			break;
		}
	}	
	return Result;
}

Script^ IntelliSenseDatabase::GetRemoteScript(String^ BaseEditorID, String^ ScriptText)
{
	for each (KeyValuePair<String^, Script^>% Itr in RemoteScripts) {
		if (!String::Compare(BaseEditorID, Itr.Key, true)) {
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
	
	for each (UserFunction^% Itr in UserFunctionList) {
		if (!String::Compare(Name, Itr->Name, true)) {
			Result = true;
			break;
		}
	}

	return Result;
}

bool IntelliSenseDatabase::IsActiveScriptRecord(String^% EditorID)
{
	bool Result = false;
	
	for each (String^% Itr in ActiveScriptRecords) {
		if (!String::Compare(EditorID, Itr, true)) {
			Result = true;
			break;
		}
	}

	return Result;	
}

bool IntelliSenseDatabase::IsCommand(String^ Name)
{
	bool Result = false;
	
	for each (IntelliSenseItem^% Itr in Enumerables) {
		if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd) {
			if (!String::Compare(Itr->GetIdentifier(), Name, true)) {
				Result = true;
				break;
			}
		}
	}	

	return Result;
}



// SYNTAX BOX

SyntaxBox::SyntaxBox(ScriptEditor::Workspace^%	Parent)
{
	VarList = gcnew List<IntelliSenseItem^>();
	ListContents = gcnew List<IntelliSenseItem^>();

	if (Icons->Images->Count == 0) {
		Icons->TransparentColor = Color::White;
		Icons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(Globals::ImageResources->GetObject("ISEmpty"))));
		Icons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(Globals::ImageResources->GetObject("ISCommand"))));
		Icons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(Globals::ImageResources->GetObject("ISLocalVar"))));
		Icons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(Globals::ImageResources->GetObject("ISRemoteVar"))));
		Icons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(Globals::ImageResources->GetObject("ISUserF"))));
		Icons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(Globals::ImageResources->GetObject("ISQuest"))));
	}

	IntelliSenseList = gcnew ListView();
	IntelliSenseList->View = View::Details;
	IntelliSenseList->BorderStyle = BorderStyle::FixedSingle;
//	IntelliSenseList->BackColor = Color::GhostWhite;
	IntelliSenseList->AutoSize = false;
	IntelliSenseList->MultiSelect = false;
	IntelliSenseList->SmallImageList = Icons;
	IntelliSenseList->SelectedIndexChanged += gcnew EventHandler(this, &SyntaxBox::IntelliSenseList_SelectedIndexChanged);
	IntelliSenseList->KeyDown += gcnew KeyEventHandler(this, &SyntaxBox::IntelliSenseList_KeyDown);
	IntelliSenseList->MouseDoubleClick += gcnew MouseEventHandler(this, &SyntaxBox::IntelliSenseList_MouseDoubleClick);
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
	IntelliSenseList->Size = ::Size(220, 105);
	IntelliSenseList->HideSelection = false;

	InfoTip->AutoPopDelay = 5000;
	InfoTip->InitialDelay = 1000;
	InfoTip->ReshowDelay = 500;
	InfoTip->ToolTipIcon = ToolTipIcon::Info;

	CanShow = true;
	ParentEditor = Parent;
	LastOperation = Operation::e_Default;
}

void SyntaxBox::Initialize(SyntaxBox::Operation Op, bool Force, bool InitAll)
{
	if (!CanShow && !Force)		return;

	UInt32 ItemCount = 0;
	Cleanup();

	IntelliSenseList->Size = ::Size(220, 108);
	IntelliSenseList->BeginUpdate();

	String^ Extract = ParentEditor->GetTextAtLoc(Globals::MouseLocation, false, false, ParentEditor->EditorBox->SelectionStart - 1, true);

	switch (Op)
	{
	case Operation::e_Default:
		if (Extract->Length >= OptionsDialog::GetSingleton()->ThresholdLength->Value || Force) {
			for each (IntelliSenseItem^% Itr in VarList) {
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr)) {
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					ListContents->Add(Itr);
					ItemCount++;
				}
			}
			for each (IntelliSenseItem^% Itr in ISDB->Enumerables) {
				if ((Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd && !dynamic_cast<CommandInfo^>(Itr)->RequiresParent) || Itr->GetType() == IntelliSenseItem::ItemType::e_Quest) {
					if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr)) {
						IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
						ListContents->Add(Itr);
						ItemCount++;
					}
				}
			}
		}
		break;
	case Operation::e_Call:
		for each (IntelliSenseItem^% Itr in ISDB->Enumerables) {
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_UserFunct) {
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll) {
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					ListContents->Add(Itr);
					ItemCount++;
				}
			}
		}
		break;
	case Operation::e_Dot:
		if (InitAll) {
			RemoteScript = Script::NullScript;
			VariableInfo^ RefVar = GetLocalVar(Extract);
			if (RefVar != nullptr && RefVar->Type == VariableInfo::VariableType::e_Ref) {									
				IsObjRefr = true;
			} else if (!String::Compare(Extract, "player", true)) {
				IsObjRefr = true;
			}
			else {
				CStringWrapper^ CStr = gcnew CStringWrapper(Extract);														// extract = calling ref
				ScriptData* Data = NativeWrapper::FetchScriptFromForm(CStr->String());

				if (!Data->IsValid()) {
					LastOperation = Operation::e_Default;
					break;
				} else {
					RemoteScript = ISDB->GetRemoteScript(gcnew String(Data->ParentID), gcnew String(Data->Text));			// cache form data for subsequent calls
					IsObjRefr = NativeWrapper::IsFormAnObjRefr(CStr->String());
				}
			}
		}

		for each (IntelliSenseItem^% Itr in RemoteScript->VarList) {
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_RemoteVar) {
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll) {
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					ListContents->Add(Itr);
					ItemCount++;
				}
			}
		}

		for each (IntelliSenseItem^% Itr in ISDB->Enumerables) {
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd && IsObjRefr) {
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll) {
					IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
					ListContents->Add(Itr);
					ItemCount++;
				}
			}
		}		
		break;
	case Operation::e_Assign:
		for each (IntelliSenseItem^% Itr in VarList) {
			if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll) {
				IntelliSenseList->Items->Add(gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetType()));
				ListContents->Add(Itr);
				ItemCount++;
			}
		}

		for each (IntelliSenseItem^% Itr in ISDB->Enumerables) {
			if (Itr->GetType() == IntelliSenseItem::ItemType::e_Quest) {
				if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || InitAll) {
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

	if (ItemCount > 0) {
		IntelliSenseList->Items[0]->Selected = true;
		Point Loc = ParentEditor->EditorBox->GetPositionFromCharIndex(ParentEditor->EditorBox->SelectionStart);
		IntelliSenseList->Location = Point(Loc.X + 3, Loc.Y + (ParentEditor->EditorBox->Font->Size + 10));

		if (ItemCount < 6)			IntelliSenseList->Size = ::Size(220, 108 - ((6 - ItemCount) * 18));			

		IntelliSenseList->Show();
		IntelliSenseList->BringToFront();
	}
	LastOperation = Op;
}

VariableInfo^ SyntaxBox::GetLocalVar(String^% Identifier)
{
	for each (IntelliSenseItem^% Itr in VarList) {
		if (!String::Compare(Itr->GetIdentifier(), Identifier, true)) {
			return dynamic_cast<VariableInfo^>(Itr);
		}
	}
	return nullptr;
}

void SyntaxBox::IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E)
{
	if (IsVisible()) {
		if (GetSelectedIndex() == -1)
			return;

		Point TipLoc = Point(IntelliSenseList->Location.X + 275, IntelliSenseList->Location.Y + 25);
		InfoTip->ToolTipTitle = ListContents[GetSelectedIndex()]->GetTypeIdentifier();
		InfoTip->Show(ListContents[GetSelectedIndex()]->Describe(), Control::FromHandle(ParentEditor->EditorControlBox->Handle), TipLoc);
	}
}


void SyntaxBox::IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	if (E->KeyCode == Keys::Escape)
		Hide();
}

void SyntaxBox::IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E)
{
	if (GetSelectedIndex() == -1)
		return;

	PickIdentifier();
}

void SyntaxBox::MoveIndex(SyntaxBox::Direction Direction)
{
	int SelectedIndex = GetSelectedIndex();
	if (SelectedIndex == -1)		return;

	switch (Direction)
	{
	case Direction::e_Down:
		if (SelectedIndex < (IntelliSenseList->Items->Count - 1))  {
			IntelliSenseList->Items[SelectedIndex]->Selected = false;
			IntelliSenseList->Items[SelectedIndex + 1]->Selected = true;

			if (IntelliSenseList->TopItem->Index < IntelliSenseList->Items->Count - 1)
				IntelliSenseList->TopItem = IntelliSenseList->Items[IntelliSenseList->TopItem->Index + 1];
		}
		break;
	case Direction::e_Up:
		if (SelectedIndex > 0) {
			IntelliSenseList->Items[SelectedIndex]->Selected = false;
			IntelliSenseList->Items[SelectedIndex - 1]->Selected = true;

			if (IntelliSenseList->TopItem->Index > 0 )
				IntelliSenseList->TopItem = IntelliSenseList->Items[IntelliSenseList->TopItem->Index - 1];
		}
		break;
	}
}

void SyntaxBox::UpdateLocalVars()
{
	IntelliSenseDatabase::ParseScript(ParentEditor->EditorBox->Text, gcnew Boxer(this));
}

void SyntaxBox::PickIdentifier()
{
	String^ Result;
	if (GetSelectedIndex() != -1) {
		Result = ListContents[GetSelectedIndex()]->GetIdentifier();
		Cleanup();
		Hide();
	}

	try {
		NativeWrapper::LockWindowUpdate(ParentEditor->EditorBox->Handle);
		ParentEditor->HandleTextChanged = false;
		ParentEditor->GetTextAtLoc(ParentEditor->EditorBox->GetPositionFromCharIndex(ParentEditor->EditorBox->SelectionStart), false, true, ParentEditor->EditorBox->SelectionStart - 1, true);
		ParentEditor->EditorBox->SelectedText = Result;
		ParentEditor->EditorBox->Focus();
	} finally {
		NativeWrapper::LockWindowUpdate(IntPtr::Zero);
	}
}

void SyntaxBox::Hide()
{
	IntelliSenseList->Hide();
	InfoTip->Hide(Control::FromHandle(IntelliSenseList->Parent->Handle));
}

void SyntaxBox::Cleanup()
{
	Hide();
	IntelliSenseList->Items->Clear();
	ListContents->Clear();
}

int	SyntaxBox::GetSelectedIndex()
{
	int Result = -1;
	for each (int SelectedIndex in IntelliSenseList->SelectedIndices) {
		Result = SelectedIndex;
		break;
	}	
	return Result;
}

bool SyntaxBox::QuickView(String^ TextUnderMouse)
{
	IntelliSenseItem^ Item = GetLocalVar(TextUnderMouse);

	if (Item == nullptr) {
		for each (IntelliSenseItem^% Itr in ISDB->Enumerables) {
			if (!String::Compare(Itr->GetIdentifier(), TextUnderMouse, true)) {
				Item = Itr;
				break;
			}
		}
	}
	if (Item != nullptr) {
		Point TipLoc = ParentEditor->EditorBox->GetPositionFromCharIndex(ParentEditor->EditorBox->SelectionStart);
		TipLoc.X += 55, TipLoc.Y += ParentEditor->EditorBox->Font->Height + 25;
		InfoTip->ToolTipTitle = Item->GetTypeIdentifier();
		InfoTip->Show(Item->Describe(), Control::FromHandle(ParentEditor->EditorControlBox->Handle), TipLoc, 8000);
		return true;
	} else
		return false;
}