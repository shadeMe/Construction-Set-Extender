#include "IntelliSenseItem.h"
#include "IntelliSenseBackend.h"
#include "ITextEditor.h"
#include "SnippetManager.h"
#include "[Common]\NativeWrapper.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


bool DoStringMatch(String^ Source, String^ Target, eStringMatchType Comparison)
{
	switch (Comparison)
	{
	case eStringMatchType::StartsWith:
		return Source->StartsWith(Target, System::StringComparison::CurrentCultureIgnoreCase);
	case eStringMatchType::Substring:
		return Source->IndexOf(Target, System::StringComparison::CurrentCultureIgnoreCase) != -1;
	case eStringMatchType::FullMatch:
		return Source->Equals(Target, System::StringComparison::CurrentCultureIgnoreCase);
	default:
		return false;
	}
}

Image^ IntelliSenseItem::GetItemTypeIcon(eItemType Type)
{
	if (ItemTypeIcons->Count == 0)
	{
		auto ImageResources =  gcnew ImageResourceManager("ScriptEditor.Icons");
		ItemTypeIcons->Add(eItemType::None, ImageResources->CreateImage("Transparent"));
		ItemTypeIcons->Add(eItemType::ScriptCommand, ImageResources->CreateImage("ScriptCommand"));
		ItemTypeIcons->Add(eItemType::ScriptVariable, ImageResources->CreateImage("LocalVariable"));
		ItemTypeIcons->Add(eItemType::Quest, ImageResources->CreateImage("Quest"));
		ItemTypeIcons->Add(eItemType::Script, ImageResources->CreateImage("UserDefinedFunction"));
		ItemTypeIcons->Add(eItemType::UserFunction, ImageResources->CreateImage("UserDefinedFunction"));
		ItemTypeIcons->Add(eItemType::GameSetting, ImageResources->CreateImage("GameSetting"));
		ItemTypeIcons->Add(eItemType::GlobalVariable, ImageResources->CreateImage("GlobalVariable"));
		ItemTypeIcons->Add(eItemType::Form, ImageResources->CreateImage("Form"));
		ItemTypeIcons->Add(eItemType::Snippet, ImageResources->CreateImage("Snippet"));
	}

	return ItemTypeIcons[Type];
}

System::String^ IntelliSenseItem::GenerateHelpTextHeader(String^ Identifier)
{
	return "<font size=\"+2\"><b>" + Identifier + "</b></font>";
}

System::String^ IntelliSenseItem::GenerateHelpTextFooter()
{
	return "<span align=\"left\">" + GetItemTypeName() + "</span>";
}

IntelliSenseItem::IntelliSenseItem()
{
	this->HelpTextHeader = String::Empty;
	this->HelpTextBody = String::Empty;
	this->HelpTextFooter = String::Empty;
	this->ItemType = eItemType::None;
}

IntelliSenseItem::IntelliSenseItem(eItemType Type)
{
	this->HelpTextHeader = String::Empty;
	this->HelpTextBody = String::Empty;
	this->HelpTextFooter = String::Empty;
	this->ItemType = Type;
}

System::String^ IntelliSenseItem::GetItemTypeName()
{
	return ItemTypeID[safe_cast<int>(ItemType)];
}

IntelliSenseItem::eItemType IntelliSenseItem::GetItemType()
{
	return ItemType;
}

void IntelliSenseItem::Insert(textEditor::ITextEditor^ Editor)
{
	Editor->SetTokenAtCaretPos(GetSubstitution());
}

bool IntelliSenseItem::MatchesToken( String^ Token, eStringMatchType Comparison )
{
	return DoStringMatch(GetIdentifier(), Token, Comparison);
}

bool IntelliSenseItem::HasInsightInfo()
{
	return true;
}

ScriptCommandParameter::ScriptCommandParameter(componentDLLInterface::ObScriptCommandInfo::ParamInfo* ParamInfo)
{

	TypeName = gcnew String(ParamInfo->TypeIDString());
	Description = ParamInfo->typeStr ? gcnew String(ParamInfo->typeStr) : "";
	Optional = ParamInfo->isOptional;
}

System::String^ IntelliSenseItemScriptCommand::GetPrettyNameForObsePlugin(String^ PluginName)
{
	if (!String::Compare(PluginName, "OBSE_Kyoma_MenuQue", true))
		return "MenuQue";
	else if (!String::Compare(PluginName, "OBSE_Elys_Pluggy", true))
		return "Pluggy";

	return PluginName;
}

IntelliSenseItemScriptCommand::IntelliSenseItemScriptCommand(componentDLLInterface::CommandTableData* CommandTableData,
																const componentDLLInterface::ObScriptCommandInfo* CommandInfo,
																String^ DeveloperUrl)
	: IntelliSenseItem(eItemType::ScriptCommand)
{
	const UInt16 kVanillaCommandStartOpcode = 0x1000, kVanillaCommandEndOpcode = 0x1170;
	auto Locale = System::Globalization::CultureInfo::CurrentCulture->TextInfo;

	eSourceType CommandSource = eSourceType::Vanilla;
	String^ CommandSourceName = "Vanilla";
	UInt32 CommandSourceVersion = 0;

	if (CommandInfo->opcode >= kVanillaCommandStartOpcode && CommandInfo->opcode <= kVanillaCommandEndOpcode)
		;//
	else
	{
		auto SourceObsePlugin = CommandTableData->GetParentPlugin(CommandInfo);
		if (SourceObsePlugin)
		{
			CommandSource = eSourceType::OBSEPlugin;
			CommandSourceName = GetPrettyNameForObsePlugin(gcnew String(SourceObsePlugin->name));
			CommandSourceVersion = SourceObsePlugin->version;
		}
		else
		{
			CommandSource = eSourceType::OBSE;
			CommandSourceName = "OBSE";
			CommandSourceVersion = CommandTableData->GetRequiredOBSEVersion(CommandInfo);
		}
	}

	String^ CommandName = gcnew String(CommandInfo->longName);

	String^ CommandHelpText = "";
	if (CommandInfo->helpText && CommandInfo->helpText[0] != '\0')
		CommandHelpText = gcnew String(CommandInfo->helpText);

	String^ CommandShorthand = "";
	if (CommandInfo->shortName && CommandInfo->shortName[0] != '\0')
		CommandShorthand = gcnew String(CommandInfo->shortName);

	eReturnValueType CommandReturnType = safe_cast<eReturnValueType>(CommandTableData->GetCommandReturnType(CommandInfo));

	List<ScriptCommandParameter^>^ CommandParameters = gcnew List<ScriptCommandParameter^>;
	for (int i = 0; i < CommandInfo->numParams; ++i)
	{
		auto Param = &CommandInfo->params[i];
		if (Param)
		{
			auto NewParam = gcnew ScriptCommandParameter(Param);
			if (NewParam->Description->Length != 0)
				NewParam->Description = Locale->ToTitleCase(NewParam->Description);

			CommandParameters->Add(NewParam);
		}
	}

	Name = CommandName;
	Source = CommandSource;
	SourceName = CommandSourceName;
	SourceVersion = CommandSourceVersion;
	Description = CommandHelpText;
	Shorthand = CommandShorthand;
	ResultType = CommandReturnType;
	Parameters = CommandParameters;
	RequireCallingRef = CommandInfo->needsParent;
	DocumentationUrl = DeveloperUrl;

	HelpTextHeader = GenerateHelpTextHeader(Name + (Shorthand->Length ? " (" + Shorthand + ")" : ""));

	HelpTextBody += SourceName + (SourceVersion ? " v" + SourceVersion : "") + " command<br/>";
	if (Description->Length)
		HelpTextBody += "<p width=\"300\" padding=\"0,0,5,5\">Description: " + Description + "</p>";

	if (Parameters->Count)
	{
		String^ kParamRowStart = "<div width=\"250\">", ^kParamRowEnd = "</div>";
		String^ kParamCellStart = "<span width=\"125\">", ^kParamCellEnd = "</span>";

		HelpTextBody += Parameters->Count + " parameter(s):<br/>";
		HelpTextBody += "<div padding=\"15,0,2,10\">";
		HelpTextBody += "<div width=\"250\" padding=\"0,0,0,4\">" +
						kParamCellStart + "<u>Name</u>" + kParamCellEnd +
						kParamCellStart + "<u>Type (O = Optional)</u>" + kParamCellEnd +
						kParamRowEnd;

		for each (auto Param in Parameters)
		{
			HelpTextBody += kParamRowStart;
			HelpTextBody += kParamCellStart + Param->Description + kParamCellEnd;
			HelpTextBody += kParamCellStart + Param->TypeName + (Param->Optional ? " (O)" : "") + kParamCellEnd;
			HelpTextBody += kParamRowEnd;
		}
		HelpTextBody += "</div>";
	}
	else
		HelpTextBody += "No parameters<br/>";

	HelpTextBody += "Return type: " + ReturnValueTypeID[safe_cast<int>(ResultType)];

	if (RequireCallingRef)
		HelpTextBody += "<br/>Requires a calling reference";

	String^ WikiUrl = "https://cs.elderscrolls.com/index.php?title=" + Name;
	HelpTextFooter = GenerateHelpTextFooter();
	HelpTextFooter += "<span align=\"right\">Links:&nbsp;&nbsp;<a href=\"" + WikiUrl + "\">Wiki</a>";

	if (DocumentationUrl->Length &&
		!WikiUrl->Equals(DocumentationUrl, StringComparison::CurrentCultureIgnoreCase))
	{
		HelpTextFooter += " | <a href=\"" + DocumentationUrl + "\">Docs</a>";
	}
	HelpTextFooter += "</span>";
}

String^ IntelliSenseItemScriptCommand::GetIdentifier()
{
	return Name;
}

bool IntelliSenseItemScriptCommand::RequiresCallingRef()
{
	return RequireCallingRef;
}

String^ IntelliSenseItemScriptCommand::GetSubstitution()
{
	return GetIdentifier();
}

bool IntelliSenseItemScriptCommand::MatchesToken(String^ Token, eStringMatchType Comparison)
{
	bool Found = DoStringMatch(Name, Token, Comparison);
	if (!Found)
		Found = DoStringMatch(Shorthand, Token, Comparison);

	return Found;
}

String^ IntelliSenseItemScriptCommand::GetShorthand()
{
	return Shorthand;
}

System::String^ IntelliSenseItemScriptCommand::GetDocumentationUrl()
{
	return DocumentationUrl;
}

IntelliSenseItemScriptVariable::IntelliSenseItemScriptVariable(String^ Name, String^ Comment,
												obScriptParsing::Variable::eDataType Type, String^ ParentEditorID) :
	IntelliSenseItem(eItemType::ScriptVariable),
	Name(Name),
	DataType(Type),
	Comment(Comment),
	ParentEditorID(ParentEditorID)
{
	HelpTextHeader = GenerateHelpTextHeader(Name);

	HelpTextBody = "Type: " + obScriptParsing::Variable::GetVariableDataTypeDescription(DataType) + "<br/>";
	if (Comment->Length)
		HelpTextBody += "<p width=\"200\" padding=\"0,0,5,0\">Comment: <i>" + Comment + "</i></p>";

	if (ParentEditorID->Length)
		HelpTextBody += "Parent script: " + ParentEditorID;

	HelpTextFooter = GenerateHelpTextFooter();
}


System::String^ IntelliSenseItemScriptVariable::GetItemTypeName()
{
	if (ParentEditorID->Length == 0)
		return "Local Variable";
	else
		return "Script Variable";
}

String^ IntelliSenseItemScriptVariable::GetIdentifier()
{
	return Name;
}

String^ IntelliSenseItemScriptVariable::GetComment()
{
	return Comment;
}

String^ IntelliSenseItemScriptVariable::GetDataTypeID()
{
	return obScriptParsing::Variable::GetVariableDataTypeDescription(DataType);
}

obScriptParsing::Variable::eDataType IntelliSenseItemScriptVariable::GetDataType()
{
	return DataType;
}

String^ IntelliSenseItemScriptVariable::GetSubstitution()
{
	return GetIdentifier();
}

String^ IntelliSenseItemForm::GetFormTypeIdentifier()
{
	return gcnew String(nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormTypeIDLongName(TypeID));
}

IntelliSenseItemForm::IntelliSenseItemForm( componentDLLInterface::FormData* Data,
											componentDLLInterface::ScriptData* AttachedScript) :
	IntelliSenseItem(eItemType::Form)
{
	TypeID = Data->TypeID;
	FormID = Data->FormID;
	EditorID = gcnew String(Data->EditorID);
	Flags = Data->Flags;
	BaseForm = Data->ObjectReference == false;
	AttachedScriptEditorID = AttachedScript && AttachedScript->IsValid() ? gcnew String(AttachedScript->EditorID) : String::Empty;

	HelpTextHeader = GenerateHelpTextHeader(EditorID);

	String^ kRowStart = "<div width=\"150\">", ^kRowEnd = "</div>";
	String^ kCellStart = "<span width=\"75\">", ^kCellEnd = "</span>";

	HelpTextBody = "<span>FormID: " + FormID.ToString("X8") + "</span><br/>";
	if (Data->ParentPluginName)
		HelpTextBody = "<span>Source file: " + gcnew String(Data->ParentPluginName) + "</span><br/>";
	if (Data->NameComponent)
		HelpTextBody += "<span>Name: " + gcnew String(Data->NameComponent) + "</span><br/>";

	String^ FlagsDesc = "";
	auto CheckedFlags = gcnew Dictionary<eFormFlags, String^>;
	CheckedFlags->Add(eFormFlags::FromMaster, "From master file");
	CheckedFlags->Add(eFormFlags::FromActiveFile, "From active file");
	CheckedFlags->Add(eFormFlags::Deleted, "Deleted");
	CheckedFlags->Add(eFormFlags::TurnOffFire, "Turn off fire");
	CheckedFlags->Add(eFormFlags::QuestItem, "Quest item");
	CheckedFlags->Add(eFormFlags::VisibleWhenDistant, "Visible when distant");

	for each (auto% CheckedFlag in CheckedFlags)
	{
		if (Flags & safe_cast<UInt32>(CheckedFlag.Key))
		{
			auto FlagString = CheckedFlag.Value;
			if (TypeID == 0x31 && CheckedFlag.Key == eFormFlags::QuestItem)
				FlagString = "Persistent";

			FlagsDesc += "<div width=\"100\"><span width=\"100\">" + FlagString + "</span></div>";
		}
	}

	if (FlagsDesc->Length)
	{
		HelpTextBody += "Flags:<br/>";
		HelpTextBody += "<div padding=\"15,0,2,10\" width=\"150\">" + FlagsDesc + "</div>";
	}

	if (Data->DescriptionComponent)
		HelpTextBody += "<p width=\"250\">Description: " + gcnew String(Data->DescriptionComponent) + "</p>";

	if (Data->ObjectReference && Data->BaseFormEditorID)
		HelpTextBody += "<br/><span>Base form: " + gcnew String(Data->BaseFormEditorID) + "</span>";

	if (AttachedScriptEditorID->Length)
		HelpTextBody += "<br/><span>Attached script: " + AttachedScriptEditorID + "</span>";

	HelpTextFooter = GenerateHelpTextFooter();
}

IntelliSenseItemForm::IntelliSenseItemForm() :
	IntelliSenseItem(eItemType::Form)
{
	TypeID = 0;
	FormID = 0;
	EditorID = "<InvalidIntelliSenseItemForm>";
	Flags = 0;
	BaseForm = false;
	AttachedScriptEditorID = String::Empty;
}

String^ IntelliSenseItemForm::GetIdentifier()
{
	return EditorID;
}

String^ IntelliSenseItemForm::GetSubstitution()
{
	return GetIdentifier();
}

System::String^ IntelliSenseItemForm::GetItemTypeName()
{
	return GetFormTypeIdentifier();
}

bool IntelliSenseItemForm::IsObjectReference()
{
	return BaseForm == false;
}

bool IntelliSenseItemForm::HasAttachedScript()
{
	return AttachedScriptEditorID->Length > 0;
}

System::String^ IntelliSenseItemForm::GetAttachedScriptEditorID()
{
	return AttachedScriptEditorID;
}


IntelliSenseItemGlobalVariable::IntelliSenseItemGlobalVariable(componentDLLInterface::FormData* Data,
	obScriptParsing::Variable::eDataType Type, String^ Value) :
	IntelliSenseItemForm(Data, nullptr)

{
	this->ItemType = IntelliSenseItem::eItemType::GlobalVariable;
	this->DataType = Type;
	this->Value = Value;

	// the body text is generated on demand to reflect any changes to the variable's in-editor value
	HelpTextHeader = GenerateHelpTextHeader(EditorID);
	HelpTextFooter = GenerateHelpTextFooter();
}

void IntelliSenseItemGlobalVariable::SetValue(String^ Val)
{
	Value = Val;
}

System::String^ IntelliSenseItemGlobalVariable::GetItemTypeName()
{
	// override IntelliSenseItemForm::GetItemTypeName to use the locally defined type name
	return IntelliSenseItem::GetItemTypeName();
}

System::String^ IntelliSenseItemGlobalVariable::TooltipBodyText::get()
{
	const UInt32 kMaxValueStringLength = 100;

	String^ TruncatedValue = gcnew String(Value);
	if (TruncatedValue->Length > kMaxValueStringLength)
		TruncatedValue = TruncatedValue->Substring(0, kMaxValueStringLength) + "...";

	HelpTextBody = "Type: " + obScriptParsing::Variable::GetVariableDataTypeDescription(DataType) + "<br/>";
	if (TruncatedValue->Length)
		HelpTextBody += "<p width=\"150\" padding=\"0,0,5,0\">Value: " + Value + "</p>";

	return HelpTextBody;
}

IntelliSenseItemGameSetting::IntelliSenseItemGameSetting(componentDLLInterface::FormData* Data,
	obScriptParsing::Variable::eDataType Type, String^ Value) :
	IntelliSenseItemGlobalVariable(Data, Type, Value)
{
	this->ItemType = IntelliSenseItem::eItemType::GameSetting;

	// regenerate after changing type
	HelpTextFooter = GenerateHelpTextFooter();
}

System::String^ IntelliSenseItemGameSetting::GetItemTypeName()
{
	// we need to distinguish ourselves from global vars
	return IntelliSenseItem::GetItemTypeName();
}

IntelliSenseItemQuest::IntelliSenseItemQuest(componentDLLInterface::FormData* Data,
											componentDLLInterface::ScriptData* AttachedScript)
	: IntelliSenseItemForm(Data, AttachedScript)
{
	this->ItemType = IntelliSenseItem::eItemType::Quest;

	// regenerate after changing type
	HelpTextFooter = GenerateHelpTextFooter();
}


IntelliSenseItemScript::IntelliSenseItemScript() :
	IntelliSenseItemForm()
{
	this->ItemType = IntelliSenseItem::eItemType::Script;

	Variables = gcnew List<IntelliSenseItemScriptVariable^>;
	EditorID = "EmptyScript";
	CommentDescription = String::Empty;
}

IntelliSenseItemScript::IntelliSenseItemScript(componentDLLInterface::ScriptData* ScriptData) :
	IntelliSenseItemForm(ScriptData, nullptr)
{
	this->ItemType = IntelliSenseItem::eItemType::Script;

	Variables = gcnew List<IntelliSenseItemScriptVariable^>;
	InitialAnalysisData = gcnew obScriptParsing::AnalysisData();
	auto AnalysisParams = gcnew obScriptParsing::AnalysisData::Params;

	AnalysisParams->ScriptText = gcnew String(ScriptData->Text);
	AnalysisParams->Ops = obScriptParsing::AnalysisData::eOperation::FillVariables |
						obScriptParsing::AnalysisData::eOperation::FillControlBlocks |
						obScriptParsing::AnalysisData::eOperation::FillUDFData;


	InitialAnalysisData->PerformAnalysis(AnalysisParams);

	for each (obScriptParsing::Variable ^ Itr in InitialAnalysisData->Variables)
		Variables->Add(gcnew IntelliSenseItemScriptVariable(Itr->Name, Itr->Comment, Itr->Type, EditorID));

	CommentDescription = InitialAnalysisData->Description;

	if (CommentDescription->Length)
		HelpTextBody += "<p width=\"250\" padding=\"0,0,5,10\">Comment: <i>" + CommentDescription->Replace("\n", "<br/>") + "</i></p>";
	HelpTextBody += "Variables: " + Variables->Count;

	// regenerate after changing type
	HelpTextFooter = GenerateHelpTextFooter();
}

IntelliSenseItemScriptVariable ^ IntelliSenseItemScript::LookupVariable(String ^ VariableName)
{
	for each (auto Itr in Variables)
	{
		if (Itr->MatchesToken(VariableName, eStringMatchType::FullMatch))
			return Itr;
	}

	return nullptr;
}

bool IntelliSenseItemScript::IsUserDefinedFunction()
{
	return ItemType == IntelliSenseItem::eItemType::UserFunction;
}

System::String^ IntelliSenseItemScript::GetItemTypeName()
{
	// to differentiate between regular scripts and UDFs
	return IntelliSenseItem::GetItemTypeName();
}

IEnumerable<IntelliSenseItemScriptVariable^>^ IntelliSenseItemScript::GetVariables()
{
	return Variables;
}

IntelliSenseItemUserFunction::IntelliSenseItemUserFunction(componentDLLInterface::ScriptData* ScriptData) :
	IntelliSenseItemScript(ScriptData)
{
	this->ItemType = eItemType::UserFunction;

	ParameterIndices = gcnew List<int>;
	for (int i = 0; i < 10; ++i)
		ParameterIndices->Add(-1);
	ReturnVarIndex = kReturnVarIdxNone;

	Debug::Assert(InitialAnalysisData->IsUDF == true);

	int VarIdx = 0, NumParams = 0;
	for each (obScriptParsing::Variable ^ Itr in InitialAnalysisData->Variables)
	{
		if (Itr->UDFParameter && Itr->ParameterIndex < 10)
		{
			ParameterIndices[Itr->ParameterIndex] = VarIdx;
			++NumParams;
		}

		if (InitialAnalysisData->UDFResult == Itr)
			ReturnVarIndex = VarIdx;
		else if (InitialAnalysisData->UDFAmbiguousResult)
			ReturnVarIndex = kReturnVarIdxAmbiguous;

		++VarIdx;
	}

	if (NumParams)
	{
		String^ kParamRowStart = "<div width=\"250\">", ^kParamRowEnd = "</div>";
		String^ kParamCellStart = "<span width=\"125\">", ^kParamCellEnd = "</span>";

		HelpTextBody += "<br/>" + NumParams + " UDF parameter(s):<br/>";
		HelpTextBody += "<div padding=\"15,0,2,10\">";
		HelpTextBody += "<div width=\"250\" padding=\"0,0,0,4\">" +
						kParamCellStart + "<u>Name</u>" + kParamCellEnd +
						kParamCellStart + "<u>Type</u>" + kParamCellEnd +
						kParamRowEnd;

		for each (auto UdfParamIndex in ParameterIndices)
		{
			if (UdfParamIndex == -1)
				break;

			auto ScriptVar = safe_cast<IntelliSenseItemScriptVariable^>(Variables[UdfParamIndex]);

			HelpTextBody += kParamRowStart;
			HelpTextBody += kParamCellStart + ScriptVar->GetIdentifier() + kParamCellEnd;
			HelpTextBody += kParamCellStart + ScriptVar->GetDataTypeID() + kParamCellEnd;
			HelpTextBody += kParamRowEnd;
		}
		HelpTextBody += "</div>";
	}
	else
		HelpTextBody += "<br/>No UDF parameters<br/>";

	if (ReturnVarIndex == kReturnVarIdxNone)
		HelpTextBody += "Does not return a value";
	else if (ReturnVarIndex == kReturnVarIdxAmbiguous)
		HelpTextBody += "Return type: Ambiguous";
	else
		HelpTextBody += "Return type: " + safe_cast<IntelliSenseItemScriptVariable^>(Variables[ReturnVarIndex])->GetDataTypeID();

	// regenerate after changing type
	HelpTextFooter = GenerateHelpTextFooter();
}

IntelliSenseItemCodeSnippet::IntelliSenseItemCodeSnippet(CodeSnippet^ Source) :
	IntelliSenseItem()
{
	this->ItemType = IntelliSenseItem::eItemType::Snippet;
	Parent = Source;

	HelpTextHeader = GenerateHelpTextHeader(Parent->Name + " (" + Parent->Shorthand + ")");

	if (Parent->Description->Length)
		HelpTextBody += "<p width=\"250\" padding=\"0,0,0,10\">Description: <i>" + Parent->Description + "</i></p>";

	if (Parent->Variables->Count)
		HelpTextBody += "New variables: " + Parent->Variables->Count;

	HelpTextFooter = GenerateHelpTextFooter();
}

void IntelliSenseItemCodeSnippet::Insert(textEditor::ITextEditor^ Editor)
{
	for each (CodeSnippet::VariableInfo^ Itr in Parent->Variables)
		Editor->InsertVariable(Itr->Name, Itr->Type);

	String^ Code = GetSubstitution();
	UInt32 CurrentLineIndents = Editor->GetIndentLevel(Editor->CurrentLine);
	Code = obScriptParsing::Sanitizer::PerformLocalizedIndenting(Code, CurrentLineIndents);

	Editor->BeginUpdate();
	Editor->SetTokenAtCaretPos(Code);
	Editor->EndUpdate(false);

	Editor->ScrollToCaret();
}

bool IntelliSenseItemCodeSnippet::MatchesToken( String^ Token, eStringMatchType Comparison )
{
	bool Found = DoStringMatch(Parent->Name, Token, Comparison);
	if (!Found)
		Found = DoStringMatch(Parent->Shorthand, Token, Comparison);

	return Found;
}

String^ IntelliSenseItemCodeSnippet::GetIdentifier()
{
	return "[" + Parent->Shorthand + "] " + Parent->Name;
}

String^ IntelliSenseItemCodeSnippet::GetSubstitution()
{
	return Parent->Code;
}

bool IntelliSenseItemCodeSnippet::HasInsightInfo()
{
	return false;
}

int IntelliSenseItemSorter::Compare(IntelliSenseItem^ X, IntelliSenseItem^ Y)
{
	int Result = X->GetItemType() < Y->GetItemType();
	if (X->GetItemType() == Y->GetItemType());
	Result = String::Compare(X->GetIdentifier(), Y->GetIdentifier(), true);

	if (Order == SortOrder::Descending)
		Result *= -1;

	return Result;
}


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse