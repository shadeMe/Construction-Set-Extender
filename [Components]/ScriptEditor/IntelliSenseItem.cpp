#include "IntelliSenseItem.h"
#include "IntelliSenseDatabase.h"
#include "ScriptTextEditorInterface.h"
#include "SnippetManager.h"
#include "[Common]\NativeWrapper.h"

namespace cse
{
	namespace intellisense
	{
		bool DoStringMatch(String^ Source, String^ Target, StringMatchType Comparison)
		{
			switch (Comparison)
			{
			case StringMatchType::StartsWith:
				return Source->StartsWith(Target, System::StringComparison::CurrentCultureIgnoreCase);
			case StringMatchType::Substring:
				return Source->IndexOf(Target, System::StringComparison::CurrentCultureIgnoreCase) != -1;
			case StringMatchType::FullMatch:
				return Source->Equals(Target, System::StringComparison::CurrentCultureIgnoreCase);
			default:
				return false;
			}
		}

		void IntelliSenseItem::PopulateImageListWithItemTypeImages(ImageList^ Destination)
		{
			Destination->Images->Clear();
			Destination->Images->AddRange(ItemTypeImages);
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
			this->Type = ItemType::Invalid;
		}

		IntelliSenseItem::IntelliSenseItem(ItemType Type)
		{
			this->HelpTextHeader = String::Empty;
			this->HelpTextBody = String::Empty;
			this->HelpTextFooter = String::Empty;
			this->Type = Type;
		}

		System::String^ IntelliSenseItem::GetItemTypeName()
		{
			return ItemTypeID[safe_cast<int>(Type)];
		}

		IntelliSenseItem::ItemType IntelliSenseItem::GetItemType()
		{
			return Type;
		}

		void IntelliSenseItem::Insert(textEditors::IScriptTextEditor^ Editor)
		{
			Editor->SetTokenAtCaretPos(GetSubstitution());
		}

		bool IntelliSenseItem::MatchesToken( String^ Token, StringMatchType Comparison )
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
			: IntelliSenseItem(ItemType::ScriptCommand)
		{
			const UInt16 kVanillaCommandStartOpcode = 0x1000, kVanillaCommandEndOpcode = 0x1170;
			auto Locale = System::Globalization::CultureInfo::CurrentCulture->TextInfo;

			SourceType CommandSource = SourceType::Vanilla;
			String^ CommandSourceName = "Vanilla";
			UInt32 CommandSourceVersion = 0;

			if (CommandInfo->opcode >= kVanillaCommandStartOpcode && CommandInfo->opcode <= kVanillaCommandEndOpcode)
				;//
			else
			{
				auto SourceObsePlugin = CommandTableData->GetParentPlugin(CommandInfo);
				if (SourceObsePlugin)
				{
					CommandSource = SourceType::OBSEPlugin;
					CommandSourceName = GetPrettyNameForObsePlugin(gcnew String(SourceObsePlugin->name));
					CommandSourceVersion = SourceObsePlugin->version;
				}
				else
				{
					CommandSource = SourceType::OBSE;
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

			ReturnValueType CommandReturnType = safe_cast<ReturnValueType>(CommandTableData->GetCommandReturnType(CommandInfo));

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

		bool IntelliSenseItemScriptCommand::MatchesToken(String^ Token, StringMatchType Comparison)
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
														obScriptParsing::Variable::DataType Type, String^ ParentEditorID) :
			IntelliSenseItem(ItemType::ScriptVariable),
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

		obScriptParsing::Variable::DataType IntelliSenseItemScriptVariable::GetDataType()
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
			IntelliSenseItem(ItemType::Form)
		{
			TypeID = Data->TypeID;
			FormID = Data->FormID;
			EditorID = gcnew String(Data->EditorID);
			Flags = Data->Flags;
			BaseForm = Data->ObjectReference == false;
			AttachedScriptEditorID = AttachedScript && AttachedScript->IsValid() ? gcnew String(AttachedScript->EditorID) : String::Empty;

			HelpTextHeader = GenerateHelpTextHeader(EditorID);

			HelpTextBody = "FormID: " + FormID.ToString("X8") + "<br/>";
			if (Data->NameComponent)
				HelpTextBody += "Name: " + gcnew String(Data->NameComponent) + "<br/>";

			String^ FlagsDesc = "";
			auto CheckedFlags = gcnew Dictionary<FormFlags, String^>;
			CheckedFlags->Add(FormFlags::FromMaster, "From master file");
			CheckedFlags->Add(FormFlags::FromActiveFile, "From active file");
			CheckedFlags->Add(FormFlags::Deleted, "Deleted");
			CheckedFlags->Add(FormFlags::TurnOffFire, "Turn off fire");
			CheckedFlags->Add(FormFlags::QuestItem, "Quest item");
			CheckedFlags->Add(FormFlags::VisibleWhenDistant, "Visible when distant");

			for each (auto% CheckedFlag in CheckedFlags)
			{
				if (Flags & safe_cast<UInt32>(CheckedFlag.Key))
				{
					auto FlagString = CheckedFlag.Value;
					if (TypeID == 0x31 && CheckedFlag.Key == FormFlags::QuestItem)
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
				HelpTextBody += "<br/>Base form: " + gcnew String(Data->BaseFormEditorID);

			if (AttachedScriptEditorID->Length)
				HelpTextBody += "<br/>Attached script: " + AttachedScriptEditorID;

			HelpTextFooter = GenerateHelpTextFooter();
		}

		IntelliSenseItemForm::IntelliSenseItemForm() :
			IntelliSenseItem(ItemType::Form)
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
			obScriptParsing::Variable::DataType Type, String^ Value) :
			IntelliSenseItemForm(Data, nullptr)

		{
			this->Type = IntelliSenseItem::ItemType::GlobalVariable;
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
			obScriptParsing::Variable::DataType Type, String^ Value) :
			IntelliSenseItemGlobalVariable(Data, Type, Value)
		{
			this->Type = IntelliSenseItem::ItemType::GameSetting;

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
			this->Type = IntelliSenseItem::ItemType::Quest;

			// regenerate after changing type
			HelpTextFooter = GenerateHelpTextFooter();
		}


		IntelliSenseItemScript::IntelliSenseItemScript() :
			IntelliSenseItemForm()
		{
			this->Type = IntelliSenseItem::ItemType::Script;

			VarList = gcnew List<IntelliSenseItemScriptVariable^>;
			EditorID = "EmptyScript";
			CommentDescription = String::Empty;
		}

		IntelliSenseItemScript::IntelliSenseItemScript(componentDLLInterface::ScriptData* ScriptData) :
			IntelliSenseItemForm(ScriptData, nullptr)
		{
			this->Type = IntelliSenseItem::ItemType::Script;

			VarList = gcnew List<IntelliSenseItemScriptVariable^>;
			InitialAnalysisData = gcnew obScriptParsing::AnalysisData();
			auto AnalysisParams = gcnew obScriptParsing::AnalysisData::Params;

			AnalysisParams->ScriptText = gcnew String(ScriptData->Text);
			AnalysisParams->Ops = obScriptParsing::AnalysisData::Operation::FillVariables |
								obScriptParsing::AnalysisData::Operation::FillControlBlocks |
								obScriptParsing::AnalysisData::Operation::FillUDFData;


			InitialAnalysisData->PerformAnalysis(AnalysisParams);

			for each (obScriptParsing::Variable ^ Itr in InitialAnalysisData->Variables)
				VarList->Add(gcnew IntelliSenseItemScriptVariable(Itr->Name, Itr->Comment, Itr->Type, EditorID));

			CommentDescription = InitialAnalysisData->Description;

			if (CommentDescription->Length)
				HelpTextBody += "<p width=\"250\" padding=\"0,0,5,10\">Comment: <i>" + CommentDescription->Replace("\n", "<br/>") + "</i></p>";
			HelpTextBody += "Variables: " + VarList->Count;

			// regenerate after changing type
			HelpTextFooter = GenerateHelpTextFooter();
		}

		IntelliSenseItemScriptVariable ^ IntelliSenseItemScript::LookupVariable(String ^ VariableName)
		{
			for each (auto Itr in VarList)
			{
				if (Itr->MatchesToken(VariableName, StringMatchType::FullMatch))
					return Itr;
			}

			return nullptr;
		}

		bool IntelliSenseItemScript::IsUserDefinedFunction()
		{
			return Type == IntelliSenseItem::ItemType::UserFunction;
		}

		System::String^ IntelliSenseItemScript::GetItemTypeName()
		{
			// to differentiate between regular scripts and UDFs
			return IntelliSenseItem::GetItemTypeName();
		}

		IEnumerable<IntelliSenseItemScriptVariable^>^ IntelliSenseItemScript::GetVariables()
		{
			return VarList;
		}

		IntelliSenseItemUserFunction::IntelliSenseItemUserFunction(componentDLLInterface::ScriptData* ScriptData) :
			IntelliSenseItemScript(ScriptData)
		{
			this->Type = ItemType::UserFunction;

			ParameterIndices = gcnew List<int>;
			for (int i = 0; i < 10; ++i)
				ParameterIndices->Add(-1);
			ReturnVarIndex = kReturnVarIdxNone;

			Debug::Assert(InitialAnalysisData->UDF == true);

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

				HelpTextBody += "<br/>" + NumParams + " UDF parameter(s)<br/>";
				HelpTextBody += "<div padding=\"15,0,2,10\">";
				HelpTextBody += "<div width=\"250\" padding=\"0,0,0,4\">" +
								kParamCellStart + "<u>Name</u>" + kParamCellEnd +
								kParamCellStart + "<u>Type</u>" + kParamCellEnd +
								kParamRowEnd;

				for each (auto UdfParamIndex in ParameterIndices)
				{
					if (UdfParamIndex == -1)
						break;

					auto ScriptVar = safe_cast<IntelliSenseItemScriptVariable^>(VarList[UdfParamIndex]);

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
				HelpTextBody += "Return type: " + safe_cast<IntelliSenseItemScriptVariable^>(VarList[ReturnVarIndex])->GetDataTypeID();

			// regenerate after changing type
			HelpTextFooter = GenerateHelpTextFooter();
		}

		IntelliSenseItemCodeSnippet::IntelliSenseItemCodeSnippet(CodeSnippet^ Source) :
			IntelliSenseItem()
		{
			this->Type = IntelliSenseItem::ItemType::Snippet;
			Parent = Source;

			HelpTextHeader = GenerateHelpTextHeader(Parent->Name + " (" + Parent->Shorthand + ")");

			if (Parent->Description->Length)
				HelpTextBody += "<p width=\"250\" padding=\"0,0,0,10\">Description: <i>" + Parent->Description + "</i></p>";

			if (Parent->Variables->Count)
				HelpTextBody += "New variables: " + Parent->Variables->Count;

			HelpTextFooter = GenerateHelpTextFooter();
		}

		void IntelliSenseItemCodeSnippet::Insert(textEditors::IScriptTextEditor^ Editor)
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

		bool IntelliSenseItemCodeSnippet::MatchesToken( String^ Token, StringMatchType Comparison )
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
	}
}