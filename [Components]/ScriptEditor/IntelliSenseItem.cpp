#include "IntelliSenseItem.h"
#include "IntelliSenseBackend.h"
#include "ITextEditor.h"
#include "SnippetManager.h"
#include "[Common]\NativeWrapper.h"
#include "IScriptEditorView.h"
#include "Preferences.h"

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
		ItemTypeIcons->Add(eItemType::Script, ImageResources->CreateImage("Script"));
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
	auto Builder = gcnew utilities::TextMarkupBuilder;

	return Builder
		->Font(preferences::SettingsHolder::Get()->Appearance->TextFont->FontFamily->Name)
			->Header(4)
				->Text(Identifier)
			->PopTag()
		->PopTag()
		->ToMarkup();
}

System::String^ IntelliSenseItem::GenerateHelpTextFooter()
{
	auto Builder = gcnew utilities::TextMarkupBuilder;

	return Builder
		->Span(DevComponents::DotNetBar::eHorizontalItemsAlignment::Left)
			->Text(GetItemTypeName())
			->NonBreakingSpace(3)
		->PopTag()
		->ToMarkup();
}

System::String^ IntelliSenseItem::WrapMarkupInDefaultFontSize(String^ Markup)
{
	auto Builder = gcnew utilities::TextMarkupBuilder;

	return Builder->Font(1, true)->Markup(Markup)->PopTag()->ToMarkup();
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
	{
		CommandShorthand = gcnew String(CommandInfo->shortName);
		if (!String::Compare(CommandShorthand, CommandName, true))
			CommandShorthand = "";
	}

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

	auto Sb = gcnew utilities::TextMarkupBuilder;

	const int kTableWidth = 400, kFirstColumnWidth = 100, kSecondColumnWidth = 300;
	Sb->Table(2, kTableWidth);
	{
		Sb->TableNextRow();
		{
			Sb->TableNextColumn(kTableWidth);
			{
				Sb->Text(SourceName);
				if (SourceVersion)
				{
					Sb->Text(" v");
					if (SourceVersion > 0xFF)
					{
						// it's likely that the version number is packed,
						// so let's display it in base 16
						Sb->Text(SourceVersion.ToString("X8"));
					}
					else
						Sb->Text(SourceVersion.ToString());
				}
				Sb->Text(" Command");
			}
			Sb->TableNextColumn(0)->TableNextColumn();
		}

		Sb->TableEmptyRow();

		if (Description->Length)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Description :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(Description);
					//Sb->NonBreakingSpace(1)->Text(":")->NonBreakingSpace(1)->Text(Description);
				}
				Sb->TableNextColumn();
			}
		}

		Sb->TableNextRow();
		{
			Sb->TableNextColumn(kFirstColumnWidth);
			{
				Sb->Bold()->Text("Parameters :")->PopTag();
			}
			Sb->TableNextColumn(kSecondColumnWidth);
			{
				Sb->Text(Parameters->Count.ToString());
				//Sb->NonBreakingSpace(1)->Text(":")->NonBreakingSpace(1)->Text(Parameters->Count.ToString());
			}
			Sb->TableNextColumn();
		}

		if (Parameters->Count)
		{
			auto kRowPadding = Padding(10, 5, 0, 3);
			const int kParamTableWidth = 310, kFirstColumnWidth = 150, kSecondColumnWidth = 150;

			Sb->Table(2, kParamTableWidth);
			{
				Sb->TableNextRow(kRowPadding);
				{
					Sb->TableNextColumn(kFirstColumnWidth);
					{
						Sb->Underline()->Text("Name")->PopTag();
					}
					Sb->TableNextColumn(kSecondColumnWidth);
					{
						Sb->Underline()->Text("Type (O = Optional)")->PopTag();
					}
					Sb->TableNextColumn();
				}

				for each (auto Param in Parameters)
				{
					Sb->TableNextRow(Padding(kRowPadding.Left, 0, 0, 0));
					{
						Sb->TableNextColumn(kFirstColumnWidth);
						{
							Sb->Text(Param->Description);
						}
						Sb->TableNextColumn(kSecondColumnWidth);
						{
							Sb->Text(Param->TypeName + (Param->Optional ? " (O)" : ""));
						}
						Sb->TableNextColumn();
					}
				}
			}
			Sb->PopTable();
		}

		Sb->TableEmptyRow();
		Sb->TableNextRow();
		{
			Sb->TableNextColumn(kFirstColumnWidth);
			{
				Sb->Bold()->Text("Return Type :")->PopTag();
			}
			Sb->TableNextColumn(kSecondColumnWidth);
			{
				Sb->Text(ReturnValueTypeID[safe_cast<int>(ResultType)]);
				//Sb->NonBreakingSpace(1)->Text(":")->NonBreakingSpace(1)->Text(ReturnValueTypeID[safe_cast<int>(ResultType)]);
			}
			Sb->TableNextColumn();
		}
	}
	Sb->PopTable();

	if (RequireCallingRef)
		Sb->LineBreak()->Text("Requires a calling reference");

	HelpTextBody = Sb->ToMarkup();

	Sb->Reset();

	String^ WikiUrl = "https://cs.uesp.net/wiki/" + Name;
	Sb->Markup(GenerateHelpTextFooter())
		->Span(DevComponents::DotNetBar::eHorizontalItemsAlignment::Right)
			->Text("Links:")->NonBreakingSpace(2)
				->Hyperlink(WikiUrl)->Text("Wiki")->PopTag();

	if (Source == eSourceType::OBSE)
	{
		String^ ObseCommandDocUrl = "https://htmlpreview.github.io/?https://github.com/llde/xOBSE/blob/master/obse_command_doc.html#" + Name;
		Sb->Text(" | ")->Hyperlink(ObseCommandDocUrl)->Text("OBSE")->PopTag();
	}

	if (DocumentationUrl->Length &&
		!WikiUrl->Equals(DocumentationUrl, StringComparison::CurrentCultureIgnoreCase))
	{
		Sb->Text(" | ")->Hyperlink(DocumentationUrl)->Text("Docs")->PopTag();
	}
	Sb->PopTag();

	HelpTextFooter = Sb->ToMarkup();
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
	return Shorthand->Length > 0;
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

	auto Sb = gcnew utilities::TextMarkupBuilder;

	const int kTableWidth = 300, kFirstColumnWidth = 120, kSecondColumnWidth = 180;
	Sb->Table(2, kTableWidth);
	{
		Sb->TableNextRow();
		{
			Sb->TableNextColumn(kFirstColumnWidth);
			{
				Sb->Bold()->Text("Type :")->PopTag();
			}
			Sb->TableNextColumn(kSecondColumnWidth);
			{
				Sb->Text(obScriptParsing::Variable::GetVariableDataTypeDescription(DataType));
			}
			Sb->TableNextColumn();
		}

		if (Comment->Length)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Comment :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Italic()->Text(Comment)->PopTag();
				}
				Sb->TableNextColumn();
			}
		}

		if (ParentEditorID->Length)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Parent Script :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(ParentEditorID);
				}
				Sb->TableNextColumn();
			}
		}
	}
	Sb->PopTable();

	HelpTextBody = Sb->ToMarkup();

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

Image^ IntelliSenseItemForm::GetFormTypeIcon(eFormType Type)
{
	if (FormTypeIcons->Count == 0)
	{
		auto ImageResources = view::components::CommonIcons::Get()->ResourceManager;
		FormTypeIcons->Add(eFormType::REFR, ImageResources->CreateImage("ObjectReference"));
		FormTypeIcons->Add(eFormType::ACHR, ImageResources->CreateImage("ObjectReference"));
		FormTypeIcons->Add(eFormType::ACRE, ImageResources->CreateImage("ObjectReference"));
	}

	Image^ Out = nullptr;
	FormTypeIcons->TryGetValue(Type, Out);
	return Out;
}

String^ IntelliSenseItemForm::GetFormTypeIdentifier()
{
	return gcnew String(nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormTypeIDLongName(safe_cast<UInt32>(TypeID)));
}

IntelliSenseItemForm::IntelliSenseItemForm(nativeWrapper::MarshalledFormData^ Data) :
	IntelliSenseItem(eItemType::Form)
{
	TypeID = safe_cast<eFormType>(Data->FormType);
	FormID = Data->FormId;
	EditorID = Data->EditorId;
	Flags = Data->FormFlags;
	BaseForm = !Data->IsObjectRef;
	AttachedScriptEditorID = Data->AttachedScriptEditorId;

	HelpTextHeader = GenerateHelpTextHeader(EditorID);

	auto Sb = gcnew utilities::TextMarkupBuilder;

	const int kTableWidth = 370, kFirstColumnWidth = 120, kSecondColumnWidth = 250;
	Sb->Table(2, kTableWidth);
	{
		Sb->TableNextRow();
		{
			Sb->TableNextColumn(kFirstColumnWidth);
			{
				Sb->Bold()->Text("FormID :")->PopTag();
			}
			Sb->TableNextColumn(kSecondColumnWidth);
			{
				Sb->Text(FormID.ToString("X8"));
			}
			Sb->TableNextColumn();
		}

		if (Data->SourcePluginName->Length > 0)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Source File :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(Data->SourcePluginName);
				}
				Sb->TableNextColumn();
			}
		}

		if (Data->NameComponent->Length > 0)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Name :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(Data->NameComponent);
				}
				Sb->TableNextColumn();
			}
		}

		auto CheckedFlags = gcnew Dictionary<eFormFlags, String^>;
		CheckedFlags->Add(eFormFlags::FromMaster, "From Master File");
		CheckedFlags->Add(eFormFlags::FromActiveFile, "From Active File");
		CheckedFlags->Add(eFormFlags::Deleted, "Deleted");
		CheckedFlags->Add(eFormFlags::TurnOffFire, "Turn Off Fire");
		CheckedFlags->Add(eFormFlags::QuestItem, "Quest Item");
		CheckedFlags->Add(eFormFlags::VisibleWhenDistant, "Visible When Distant");
		auto FlagStrings = gcnew List<String^>;

		for each (auto% CheckedFlag in CheckedFlags)
		{
			if (Flags & safe_cast<UInt32>(CheckedFlag.Key))
			{
				auto FlagString = CheckedFlag.Value;
				if (TypeID == eFormType::REFR && CheckedFlag.Key == eFormFlags::QuestItem)
					FlagString = "Persistent";

				FlagStrings->Add(FlagString);
			}
		}

		if (FlagStrings->Count > 0)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Flags :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(FlagStrings[0]);
				}
				Sb->TableNextColumn();
			}

			for (int i = 1; i < FlagStrings->Count; ++i)
			{
				Sb->TableNextRow();
				{
					Sb->TableNextColumn(kFirstColumnWidth);
					{
						Sb->NonBreakingSpace(1);
					}
					Sb->TableNextColumn(kSecondColumnWidth);
					{
						Sb->Text(FlagStrings[i]);
					}
					Sb->TableNextColumn();
				}
			}
		}

		if (Data->DescriptionComponent->Length > 0)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Description :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Bold()->Text(Data->DescriptionComponent)->PopTag();
				}
				Sb->TableNextColumn();
			}
		}

		if (Data->IsObjectRef && Data->BaseFormEditorId->Length != 0)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Base Form :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(Data->BaseFormEditorId);
				}
				Sb->TableNextColumn();
			}
		}

		if (AttachedScriptEditorID->Length)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Attached Script :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(AttachedScriptEditorID);
				}
				Sb->TableNextColumn();
			}
		}
	}
	Sb->PopTable();

	HelpTextBody = Sb->ToMarkup();

	HelpTextFooter = GenerateHelpTextFooter();
}

IntelliSenseItemForm::IntelliSenseItemForm() :
	IntelliSenseItem(eItemType::Form)
{
	TypeID = eFormType::None;
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

Image^ IntelliSenseItemForm::TooltipFooterImage::get()
{
	auto FormIcon = GetFormTypeIcon(TypeID);
	return FormIcon ? FormIcon : GetItemTypeIcon(ItemType);
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
	const UInt32 kMaxValueStringLength = 200;

	String^ TruncatedValue = gcnew String(Value);
	if (TruncatedValue->Length > kMaxValueStringLength)
		TruncatedValue = TruncatedValue->Substring(0, kMaxValueStringLength) + "...";

	auto Sb = gcnew utilities::TextMarkupBuilder;
	const int kTableWidth = 300, kFirstColumnWidth = 120, kSecondColumnWidth = 180;
	Sb->Table(2, kTableWidth);
	{
		Sb->TableNextRow();
		{
			Sb->TableNextColumn(kFirstColumnWidth);
			{
				Sb->Bold()->Text("Type :")->PopTag();
			}
			Sb->TableNextColumn(kSecondColumnWidth);
			{
				Sb->Text(obScriptParsing::Variable::GetVariableDataTypeDescription(DataType));
			}
			Sb->TableNextColumn();
		}

		if (TruncatedValue->Length)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Value :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(TruncatedValue);
				}
				Sb->TableNextColumn();
			}
		}
	}
	Sb->PopTable();

	HelpTextBody = Sb->ToMarkup();

	return WrapMarkupInDefaultFontSize(HelpTextBody);
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

	auto Sb = gcnew utilities::TextMarkupBuilder;

	const int kTableWidth = 370, kFirstColumnWidth = 120, kSecondColumnWidth = 250;
	Sb->Table(2, kTableWidth);
	{
		if (CommentDescription->Length)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Comment :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Italic()->Text(CommentDescription)->PopTag();
				}
				Sb->TableNextColumn();
			}
		}

		Sb->TableNextRow();
		{
			Sb->TableNextColumn(kFirstColumnWidth);
			{
				Sb->Bold()->Text("Variables :")->PopTag();
			}
			Sb->TableNextColumn(kSecondColumnWidth);
			{
				Sb->Text(Variables->Count.ToString());
			}
			Sb->TableNextColumn();
		}
	}
	Sb->PopTable();

	HelpTextBody += Sb->ToMarkup();

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

	auto Sb = gcnew utilities::TextMarkupBuilder;

	const int kTableWidth = 370, kFirstColumnWidth = 120, kSecondColumnWidth = 250;
	Sb->Table(2, kTableWidth);
	{
		Sb->TableNextRow();
		{
			Sb->TableNextColumn(kFirstColumnWidth);
			{
				Sb->Bold()->Text("Parameters :")->PopTag();
			}
			Sb->TableNextColumn(kSecondColumnWidth);
			{
				Sb->Text(NumParams.ToString());
			}
			Sb->TableNextColumn();
		}

		if (NumParams)
		{
			auto kRowPadding = Padding(10, 5, 0, 3);
			const int kParamTableWidth = 310, kFirstColumnWidth = 150, kSecondColumnWidth = 150;

			Sb->Table(2, kParamTableWidth);
			{
				Sb->TableNextRow(kRowPadding);
				{
					Sb->TableNextColumn(kFirstColumnWidth);
					{
						Sb->Underline()->Text("Name")->PopTag();
					}
					Sb->TableNextColumn(kSecondColumnWidth);
					{
						Sb->Underline()->Text("Type")->PopTag();
					}
					Sb->TableNextColumn();
				}

				for each (auto UdfParamIndex in ParameterIndices)
				{
					if (UdfParamIndex == -1)
						break;

					auto ScriptVar = safe_cast<IntelliSenseItemScriptVariable^>(Variables[UdfParamIndex]);
					Sb->TableNextRow(Padding(kRowPadding.Left, 0, 0, 0));
					{
						Sb->TableNextColumn(kFirstColumnWidth);
						{
							Sb->Text(ScriptVar->GetIdentifier());
						}
						Sb->TableNextColumn(kSecondColumnWidth);
						{
							Sb->Text(ScriptVar->GetDataTypeID());
						}
						Sb->TableNextColumn();
					}
				}
			}
			Sb->PopTable();
		}

		Sb->TableEmptyRow();

		Sb->TableNextRow();
		{
			Sb->TableNextColumn(kFirstColumnWidth);
			{
				Sb->Bold()->Text("Return Value :")->PopTag();
			}
			Sb->TableNextColumn(kSecondColumnWidth);
			{
				if (ReturnVarIndex == kReturnVarIdxNone)
					Sb->Text("None");
				else if (ReturnVarIndex == kReturnVarIdxAmbiguous)
					Sb->Text("Ambiguous");
				else
					Sb->Text(safe_cast<IntelliSenseItemScriptVariable^>(Variables[ReturnVarIndex])->GetDataTypeID());
			}
			Sb->TableNextColumn();
		}
	}
	Sb->PopTable();

	HelpTextBody += Sb->ToMarkup();

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

	auto Sb = gcnew utilities::TextMarkupBuilder;
	const int kTableWidth = 300, kFirstColumnWidth = 120, kSecondColumnWidth = 180;
	Sb->Table(2, kTableWidth);
	{
		if (Parent->Description->Length)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("Description :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(Parent->Description);
				}
				Sb->TableNextColumn();
			}
		}

		if (Parent->Variables->Count)
		{
			Sb->TableNextRow();
			{
				Sb->TableNextColumn(kFirstColumnWidth);
				{
					Sb->Bold()->Text("New Variables :")->PopTag();
				}
				Sb->TableNextColumn(kSecondColumnWidth);
				{
					Sb->Text(Parent->Variables->Count.ToString());
				}
				Sb->TableNextColumn();
			}
		}
	}
	Sb->PopTable();

	HelpTextBody = Sb->ToMarkup();
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