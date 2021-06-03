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

		IntelliSenseItem::IntelliSenseItem()
		{
			this->Description = String::Empty;
			this->Type = ItemType::Invalid;
		}

		IntelliSenseItem::IntelliSenseItem(ItemType Type)
		{
			this->Description = String::Empty;
			this->Type = Type;
		}

		IntelliSenseItem::ItemType IntelliSenseItem::GetItemType()
		{
			return Type;
		}

		String^ IntelliSenseItem::GetItemTypeName()
		{
			return ItemTypeID[(int)Type];
		}

		String^ IntelliSenseItem::Describe()
		{
			return Description;
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

		IntelliSenseItemScriptCommand::IntelliSenseItemScriptCommand(String^ Name,
																	 String^ Desc,
																	 String^ Shorthand,
																	 UInt16 NoOfParams,
																	 bool RequiresParent,
																	 UInt16 ReturnType,
																	 SourceType Source,
																	 String^ Params,
																	 String^ DeveloperURL) :
			IntelliSenseItem(ItemType::ScriptCommand),
			Name(Name),
			CmdDescription(Desc),
			Shorthand(Shorthand),
			ParamCount(NoOfParams),
			RequiresParent(RequiresParent),
			ResultType((ReturnValueType)ReturnType),
			Source(Source),
			DeveloperURL(DeveloperURL)
		{
			Description = String::Format("{0}{1}\n\n{4}\n\n{2} parameter(s){6}\nReturn Type: {3}{5}",
										Name,
										(Shorthand == "None") ? "" : (" [ " + Shorthand + " ]"),
										NoOfParams.ToString(),
										IntelliSenseItemScriptCommand::ReturnValueTypeID[(int)ReturnType],
										Desc, (RequiresParent) ? "\n\nRequires a calling reference" : "",
										Params);
		}

		String^ IntelliSenseItemScriptCommand::GetIdentifier()
		{
			return Name;
		}

		bool IntelliSenseItemScriptCommand::GetRequiresParent()
		{
			return RequiresParent;
		}

		IntelliSenseItemScriptCommand::SourceType IntelliSenseItemScriptCommand::GetSource()
		{
			return Source;
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

		System::String^ IntelliSenseItemScriptCommand::GetDeveloperURL()
		{
			return DeveloperURL;
		}

		void IntelliSenseItemScriptCommand::SetDeveloperURL(String^ URL)
		{
			DeveloperURL = URL;
		}

		IntelliSenseItemScriptVariable::IntelliSenseItemScriptVariable(String^ Name, String^ Comment,
														obScriptParsing::Variable::DataType Type, String^ ParentEditorID) :
			IntelliSenseItem(ItemType::ScriptVariable),
			Name(Name),
			DataType(Type),
			Comment(Comment),
			ParentEditorID(ParentEditorID)
		{
			Description = String::Format("{0} [{1}]{2}{3}",
										Name,
										obScriptParsing::Variable::GetVariableDataTypeDescription(DataType),
										Comment->Length > 0 ? "\n\n" + Comment : "",
										ParentEditorID->Length > 0 ? "\n\nParent Script: " + ParentEditorID : "");
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

			String^ FlagDescription = "" + ((Flags & (UInt32)FormFlags::FromMaster) ? gcnew String(' ', 7) + "From Master File\n" : "") +
				((Flags & (UInt32)FormFlags::FromActiveFile) ? gcnew String(' ', 7) + "From Active File\n" : "") +
				((Flags & (UInt32)FormFlags::Deleted) ? gcnew String(' ', 7) + "Deleted\n" : "") +
				((Flags & (UInt32)FormFlags::TurnOffFire) ? gcnew String(' ', 7) + "Turn Off Fire\n" : "") +
				((Flags & (UInt32)FormFlags::QuestItem) ? gcnew String(' ', 7) + (TypeID == 0x31 ? "Persistent\n" : "Quest Item\n") : "") +
				((Flags & (UInt32)FormFlags::Disabled) ? gcnew String(' ', 7) + "Initially Disabled\n" : "") +
				((Flags & (UInt32)FormFlags::VisibleWhenDistant) ? gcnew String(' ', 7) + "Visible When Distant\n" : "");

			String^ RefBaseFormDescription = "";
			if (Data->ObjectReference && Data->BaseFormEditorID)
				RefBaseFormDescription = "\nBase Form: " + gcnew String(Data->BaseFormEditorID);

			String^ ScriptDescription = "";
			if (AttachedScriptEditorID->Length > 0)
				ScriptDescription += "\nAttached Script: " + gcnew String(AttachedScript->EditorID);

			this->Description = "EditorID: " + EditorID +
								"\nFormID: " + FormID.ToString("X8") +
								(FlagDescription->Length ? "\nFlags:\n" + FlagDescription : "") +
								RefBaseFormDescription + ScriptDescription;
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
			Description = EditorID;
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
		}

		System::String^ IntelliSenseItemGlobalVariable::Describe()
		{
			const UInt32 kMaxValueStringLength = 30;

			String^ TruncatedValue = gcnew String(Value);
			if (TruncatedValue->Length > kMaxValueStringLength)
				TruncatedValue = TruncatedValue->Substring(0, kMaxValueStringLength) + "...";

			return Description + "\n\nType: " + obScriptParsing::Variable::GetVariableDataTypeDescription(DataType)
					+ (TruncatedValue->Length > 0 ? "\nValue: " + TruncatedValue : "");
		}

		System::String^ IntelliSenseItemGlobalVariable::GetItemTypeName()
		{
			return IntelliSenseItem::GetItemTypeName();
		}

		void IntelliSenseItemGlobalVariable::SetValue(String^ Val)
		{
			Value = Val;
		}

		IntelliSenseItemGameSetting::IntelliSenseItemGameSetting(componentDLLInterface::FormData* Data,
			obScriptParsing::Variable::DataType Type, String^ Value) :
			IntelliSenseItemGlobalVariable(Data, Type, Value)
		{
			this->Type = IntelliSenseItem::ItemType::GameSetting;
		}

		IntelliSenseItemQuest::IntelliSenseItemQuest(componentDLLInterface::FormData* Data,
													componentDLLInterface::ScriptData* AttachedScript,
													String^ FullName) :
			IntelliSenseItemForm(Data, AttachedScript)
		{
			this->Type = IntelliSenseItem::ItemType::Quest;
			FullName = FullName;

			Description += (FullName->Length > 0 ? "\n\n" + FullName : "");
		}

		System::String^ IntelliSenseItemQuest::GetFullName()
		{
			return FullName;
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

			Description += (CommentDescription ->Length > 0 ? "\nDescription: " + CommentDescription : "")
						+ "\n\nNumber of variables: " + VarList->Count;
		}

		String^ IntelliSenseItemScript::GetItemTypeName()
		{
			return IntelliSenseItem::GetItemTypeName();
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

			int VarIdx = 0;
			for each (obScriptParsing::Variable ^ Itr in InitialAnalysisData->Variables)
			{
				if (Itr->UDFParameter && Itr->ParameterIndex < 10)
					ParameterIndices[Itr->ParameterIndex] = VarIdx;

				if (InitialAnalysisData->UDFResult == Itr)
					ReturnVarIndex = VarIdx;
				else if (InitialAnalysisData->UDFAmbiguousResult)
					ReturnVarIndex = kReturnVarIdxAmbiguous;

				++VarIdx;
			}


			String^ Scratch = "";
			int ParamIdx = 0;
			while (ParamIdx < 10)
			{
				int VarIdx = ParameterIndices[ParamIdx];
				if (VarIdx == -1)
					break;

				String^ Comment = VarList[VarIdx]->GetComment();
				String^ Name = VarList[VarIdx]->GetIdentifier();
				Scratch += "\n" + gcnew String(' ', 7) + ((Comment == "") ? Name : Comment) + " [" + (safe_cast<IntelliSenseItemScriptVariable^>(VarList[VarIdx]))->GetDataTypeID() + "]";
				ParamIdx++;
			}

			Description += "\n" + ParamIdx + " Parameter(s)" + Scratch + "\n\n";
			if (ReturnVarIndex == kReturnVarIdxNone)
				Description += "Does not return a value";
			else if (ReturnVarIndex == kReturnVarIdxAmbiguous)
				Description += "Return Type: Ambiguous";
			else
				Description += "Return Type: " + (safe_cast<IntelliSenseItemScriptVariable^>(VarList[ReturnVarIndex]))->GetDataTypeID();
		}

		IntelliSenseItemCodeSnippet::IntelliSenseItemCodeSnippet(CodeSnippet^ Source) :
			IntelliSenseItem()
		{
			this->Type = IntelliSenseItem::ItemType::Snippet;

			Parent = Source;

			this->Description = "Name: " + Parent->Name + "\n" +
				"Shorthand: " + Parent->Shorthand +
				(Parent->Description->Length ? "\n\n" + Parent->Description : "") +
				(Parent->Variables->Count ? "\n\nVariables: " + Parent->Variables->Count.ToString() : "");
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