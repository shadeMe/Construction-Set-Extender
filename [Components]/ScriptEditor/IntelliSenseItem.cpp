#include "IntelliSenseItem.h"
#include "IntelliSenseBackend.h"
#include "ITextEditor.h"
#include "SnippetManager.h"
#include "[Common]\NativeWrapper.h"
#include "IScriptEditorView.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


Image^ IntelliSenseItem::GetItemTypeIcon(eItemType Type)
{
	if (ItemTypeIcons->Count == 0)
	{
		auto ImageResources = view::components::CommonIcons::Get()->ResourceManager;
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

bool IntelliSenseItem::HasInsightInfo()
{
	return true;
}

bool IntelliSenseItem::HasAlternateIdentifier()
{
	return false;
}

System::String^ IntelliSenseItem::GetSubstitution()
{
	return GetIdentifier();
}

System::String^ IntelliSenseItem::GetAlternateIdentifier()
{
	return String::Empty;
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

System::String^ IntelliSenseItemScriptCommand::GetAlternateIdentifier()
{
	return Shorthand;
}

bool IntelliSenseItemScriptCommand::RequiresCallingRef()
{
	return RequireCallingRef;
}

String^ IntelliSenseItemScriptCommand::GetShorthand()
{
	return Shorthand;
}

System::String^ IntelliSenseItemScriptCommand::GetDocumentationUrl()
{
	return DocumentationUrl;
}

bool IntelliSenseItemScriptCommand::HasAlternateIdentifier()
{
	return true;
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

String^ IntelliSenseItemForm::GetFormTypeIdentifier()
{
	return gcnew String(nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormTypeIDLongName(TypeID));
}

IntelliSenseItemForm::IntelliSenseItemForm(nativeWrapper::MarshalledFormData^ Data) :
	IntelliSenseItem(eItemType::Form)
{
	TypeID = Data->FormType;
	FormID = Data->FormId;
	EditorID = Data->EditorId;
	Flags = Data->FormFlags;
	BaseForm = !Data->IsObjectRef;
	AttachedScriptEditorID = Data->AttachedScriptEditorId;

	HelpTextHeader = GenerateHelpTextHeader(EditorID);

	String^ kRowStart = "<div width=\"150\">", ^kRowEnd = "</div>";
	String^ kCellStart = "<span width=\"75\">", ^kCellEnd = "</span>";

	HelpTextBody = "<span>FormID: " + FormID.ToString("X8") + "</span><br/>";
	if (Data->SourcePluginName)
		HelpTextBody = "<span>Source file: " + Data->SourcePluginName + "</span><br/>";
	if (Data->NameComponent)
		HelpTextBody += "<span>Name: " + Data->NameComponent + "</span><br/>";

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
		HelpTextBody += "<p width=\"250\">Description: " + Data->DescriptionComponent + "</p>";

	if (Data->IsObjectRef && Data->BaseFormEditorId->Length != 0)
		HelpTextBody += "<br/><span>Base form: " + Data->BaseFormEditorId + "</span>";

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


IntelliSenseItemGlobalVariable::IntelliSenseItemGlobalVariable(nativeWrapper::MarshalledVariableData^ Data) :
	IntelliSenseItemForm(Data)

{
	ItemType = IntelliSenseItem::eItemType::GlobalVariable;
	switch (Data->VariableType)
	{
	case nativeWrapper::MarshalledVariableData::eVariableType::Integer:
		DataType = obScriptParsing::Variable::eDataType::Integer;
		break;
	case nativeWrapper::MarshalledVariableData::eVariableType::Float:
		DataType = obScriptParsing::Variable::eDataType::Float;
		break;
	case nativeWrapper::MarshalledVariableData::eVariableType::String:
		DataType = obScriptParsing::Variable::eDataType::String;
		break;
	default:
		DataType = obScriptParsing::Variable::eDataType::None;
	}

	Value = Data->ValueAsString;

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

IntelliSenseItemGameSetting::IntelliSenseItemGameSetting(nativeWrapper::MarshalledVariableData^ Data) :
	IntelliSenseItemGlobalVariable(Data)
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

IntelliSenseItemQuest::IntelliSenseItemQuest(nativeWrapper::MarshalledFormData^ Data)
	: IntelliSenseItemForm(Data)
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

IntelliSenseItemScript::IntelliSenseItemScript(nativeWrapper::MarshalledScriptData^ ScriptData) :
	IntelliSenseItemForm(ScriptData)
{
	this->ItemType = IntelliSenseItem::eItemType::Script;

	Variables = gcnew List<IntelliSenseItemScriptVariable^>;
	InitialAnalysisData = gcnew obScriptParsing::AnalysisData();
	auto AnalysisParams = gcnew obScriptParsing::AnalysisData::Params;

	AnalysisParams->ScriptText = gcnew String(ScriptData->ScriptText);
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

	// release the analysis data if it won't be used by the UDF subclass
	if (!InitialAnalysisData->IsUDF)
		InitialAnalysisData = nullptr;
}

IntelliSenseItemScriptVariable ^ IntelliSenseItemScript::LookupVariable(String ^ VariableName)
{
	for each (auto Itr in Variables)
	{
		if (String::Compare(Itr->GetIdentifier(), VariableName, true) == 0)
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

IntelliSenseItemUserFunction::IntelliSenseItemUserFunction(nativeWrapper::MarshalledScriptData^ ScriptData) :
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

	// no longer useful, release it
	InitialAnalysisData = nullptr;
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

String^ IntelliSenseItemCodeSnippet::GetIdentifier()
{
	return "[" + Parent->Shorthand + "] " + Parent->Name;
}

System::String^ IntelliSenseItemCodeSnippet::GetAlternateIdentifier()
{
	return Parent->Shorthand;
}

String^ IntelliSenseItemCodeSnippet::GetSubstitution()
{
	return Parent->Code;
}

bool IntelliSenseItemCodeSnippet::HasInsightInfo()
{
	return false;
}

bool IntelliSenseItemCodeSnippet::HasAlternateIdentifier()
{
	return true;
}


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse