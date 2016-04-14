#include "IntelliSenseItem.h"
#include "IntelliSenseDatabase.h"
#include "IntelliSenseInterface.h"

#include "ScriptTextEditorInterface.h"
#include "SnippetManager.h"

#include "[Common]\NativeWrapper.h"

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		IntelliSenseItem::IntelliSenseItem()
		{
			this->Description = "";
			this->Type = IntelliSenseItemType::Invalid;
		}

		IntelliSenseItem::IntelliSenseItem(String^ Desc, IntelliSenseItemType Type)
		{
			this->Description = Desc;
			this->Type = Type;
		}

		IntelliSenseItem::IntelliSenseItemType IntelliSenseItem::GetItemType()
		{
			return Type;
		}

		String^ IntelliSenseItem::GetItemTypeID()
		{
			return IntelliSenseItemTypeID[(int)Type];
		}

		String^ IntelliSenseItem::Describe()
		{
			return Description;
		}

		void IntelliSenseItem::Insert(TextEditors::IScriptTextEditor^ Editor)
		{
			Editor->SetTokenAtCaretPos(GetSubstitution());
		}

		bool IntelliSenseItem::GetShouldEnumerate( String^ Token, bool SubstringSearch )
		{
			if (SubstringSearch)
				return (GetIdentifier()->IndexOf(Token, System::StringComparison::CurrentCultureIgnoreCase) != -1);
			else
				return GetIdentifier()->StartsWith(Token, System::StringComparison::CurrentCultureIgnoreCase);
		}

		bool IntelliSenseItem::GetIsQuickViewable(String^ Token)
		{
			return !String::Compare(GetIdentifier(), Token, true);
		}

		IntelliSenseItemScriptCommand::IntelliSenseItemScriptCommand(String^ Name,
																	 String^ Desc,
																	 String^ Shorthand,
																	 UInt16 NoOfParams,
																	 bool RequiresParent,
																	 UInt16 ReturnType,
																	 IntelliSenseCommandItemSourceType Source,
																	 String^ Params) :
			IntelliSenseItem(String::Format("{0}{1}\n\n{4}\n\n{2} parameter(s){6}\nReturn Type: {3}{5}",
												Name,
												(Shorthand == "None")?"":(" [ " + Shorthand + " ]"),
												NoOfParams.ToString(),
												IntelliSenseItemScriptCommand::IntelliSenseItemCommandReturnTypeID[(int)ReturnType],
												Desc, (RequiresParent)?"\n\nRequires a calling reference":"",
												Params),
								IntelliSenseItemType::Command),
				Name(Name),
				CmdDescription(Desc),
				Shorthand(Shorthand),
				ParamCount(NoOfParams),
				RequiresParent(RequiresParent),
				ReturnType(ReturnType),
				Source(Source)
		{
			;//
		}

		String^ IntelliSenseItemScriptCommand::GetIdentifier()
		{
			return Name;
		}

		bool IntelliSenseItemScriptCommand::GetRequiresParent()
		{
			return RequiresParent;
		}

		IntelliSenseItemScriptCommand::IntelliSenseCommandItemSourceType IntelliSenseItemScriptCommand::GetSource()
		{
			return Source;
		}

		String^ IntelliSenseItemScriptCommand::GetSubstitution()
		{
			return GetIdentifier();
		}

		bool IntelliSenseItemScriptCommand::GetShouldEnumerate(String^ Token, bool SubstringSearch)
		{
			bool Found = false;
			if (SubstringSearch)
				Found = Name->IndexOf(Token, System::StringComparison::CurrentCultureIgnoreCase) != -1;
			else
				Found = Name->StartsWith(Token, System::StringComparison::CurrentCultureIgnoreCase);

			if (Found == false)
			{
				if (SubstringSearch)
					Found = Shorthand->IndexOf(Token, System::StringComparison::CurrentCultureIgnoreCase) != -1;
				else
					Found = Shorthand->StartsWith(Token, System::StringComparison::CurrentCultureIgnoreCase);
			}

			return Found;
		}

		bool IntelliSenseItemScriptCommand::GetIsQuickViewable(String^ Token)
		{
			return !String::Compare(Name, Token, true) || !String::Compare(Shorthand, Token, true);
		}

		String^ IntelliSenseItemScriptCommand::GetShorthand()
		{
			return Shorthand;
		}

		IntelliSenseItemVariable::IntelliSenseItemVariable(String^ Name, String^ Comment, ObScriptParsing::Variable::DataType Type, IntelliSenseItemType Scope) :
			IntelliSenseItem(String::Format("{0} [{1}]{2}{3}",
											Name,
											ObScriptParsing::Variable::GetVariableDataTypeDescription(Type),
											(Comment != "")?"\n\n":"",
											Comment),
							Scope),
			Name(Name),
			DataType(Type),
			Comment(Comment)
		{
			;//
		}

		String^ IntelliSenseItemVariable::GetIdentifier()
		{
			return Name;
		}

		String^ IntelliSenseItemVariable::GetComment()
		{
			return Comment;
		}

		String^ IntelliSenseItemVariable::GetDataTypeID()
		{
			return ObScriptParsing::Variable::GetVariableDataTypeDescription(DataType);
		}

		ObScriptParsing::Variable::DataType IntelliSenseItemVariable::GetDataType()
		{
			return DataType;
		}

		String^ IntelliSenseItemVariable::GetSubstitution()
		{
			return GetIdentifier();
		}

		IntelliSenseItemQuest::IntelliSenseItemQuest(String^ EditorID, String^ Desc, String^ ScrName) :
					IntelliSenseItem(EditorID + ((Desc != "")?"\n":"") + Desc + ((ScrName != "")?"\n\nQuest Script: ":"") + ScrName,
									IntelliSenseItem::IntelliSenseItemType::Quest),
					Name(EditorID),
					ScriptName(ScrName)
		{
			;//
		}

		String^ IntelliSenseItemQuest::GetIdentifier()
		{
			return Name;
		}

		String^ IntelliSenseItemQuest::GetSubstitution()
		{
			return GetIdentifier();
		}

		Script::Script()
		{
			VarList = gcnew VarListT;
		}

		Script::Script(String^ ScriptText)
		{
			VarList = gcnew VarListT;

			ObScriptParsing::AnalysisData^ Data = gcnew ObScriptParsing::AnalysisData();
			Data->PerformAnalysis(ScriptText, ObScriptParsing::ScriptType::None,
								ObScriptParsing::AnalysisData::Operation::FillVariables |
								ObScriptParsing::AnalysisData::Operation::FillControlBlocks |
								ObScriptParsing::AnalysisData::Operation::FillUDFData,
								nullptr);

			for each (ObScriptParsing::Variable^ Itr in Data->Variables)
				VarList->Add(gcnew IntelliSenseItemVariable(Itr->Name, Itr->Comment, Itr->Type, IntelliSenseItem::IntelliSenseItemType::RemoteVar));

			Name = Data->Name;
			CommentDescription = Data->Description;
		}

		Script::Script(String^ ScriptText, String^ Name)
		{
			VarList = gcnew VarListT;

			ObScriptParsing::AnalysisData^ Data = gcnew ObScriptParsing::AnalysisData();
			Data->PerformAnalysis(ScriptText, ObScriptParsing::ScriptType::None,
								  ObScriptParsing::AnalysisData::Operation::FillVariables |
								  ObScriptParsing::AnalysisData::Operation::FillControlBlocks |
								  ObScriptParsing::AnalysisData::Operation::FillUDFData,
								  nullptr);

			for each (ObScriptParsing::Variable^ Itr in Data->Variables)
				VarList->Add(gcnew IntelliSenseItemVariable(Itr->Name, Itr->Comment, Itr->Type, IntelliSenseItem::IntelliSenseItemType::RemoteVar));

			this->Name = Name;
			CommentDescription = Data->Description;
		}

		String^ Script::Describe()
		{
			String^ Description;
			Description += Name + "\n\n" + CommentDescription + "\nNumber of variables: " + VarList->Count;
			return Description;
		}

		String^ Script::GetIdentifier()
		{
			return Name;
		}

		List<IntelliSenseItemVariable^>::Enumerator^ Script::GetVariableListEnumerator()
		{
			return VarList->GetEnumerator();
		}

		UserFunction::UserFunction(String^ ScriptText) : Script()
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

			ObScriptParsing::AnalysisData^ Data = gcnew ObScriptParsing::AnalysisData();
			Data->PerformAnalysis(ScriptText, ObScriptParsing::ScriptType::None,
								  ObScriptParsing::AnalysisData::Operation::FillVariables |
								  ObScriptParsing::AnalysisData::Operation::FillControlBlocks |
								  ObScriptParsing::AnalysisData::Operation::FillUDFData,
								  nullptr);

			int VarIdx = 0;
			for each (ObScriptParsing::Variable^ Itr in Data->Variables)
			{
				VarList->Add(gcnew IntelliSenseItemVariable(Itr->Name, Itr->Comment, Itr->Type, IntelliSenseItem::IntelliSenseItemType::RemoteVar));
				if (Itr->UDFParameter && Itr->ParameterIndex < 10)
					Parameters->SetValue(VarIdx, (int)Itr->ParameterIndex);

				if (Data->UDFResult == Itr)
					ReturnVar = VarIdx;
				else if (Data->UDFAmbiguousResult)
					ReturnVar = -9;

				VarIdx++;
			}

			Name = Data->Name;
			CommentDescription = Data->Description;
		}

		String^ UserFunction::Describe()
		{
			String^ Description, ^Scratch;

			int ParamIdx = 0;
			while (ParamIdx < 10)
			{
				int VarIdx = (int)Parameters->GetValue(ParamIdx);
				if (VarIdx == -1)
					break;

				String^% Comment = VarList[VarIdx]->GetComment(), ^% Name = VarList[VarIdx]->GetIdentifier();
				Scratch += "\n\t" + ((Comment == "")?Name:Comment) + " [" + (dynamic_cast<IntelliSenseItemVariable^>(VarList[VarIdx]))->GetDataTypeID() + "]";
				ParamIdx++;
			}

			if (CommentDescription->Length > 2)
				CommentDescription = "\n\nDescription: " + CommentDescription;
			else
				CommentDescription = "\n";

			Description += Name + CommentDescription + "\n" + ParamIdx + " Parameter(s)" + Scratch + "\n\n";
			if (ReturnVar == -1)			Description += "Does not return a value";
			else if (ReturnVar == -9)		Description += "Return Type: Ambiguous";
			else							Description += "Return Type: " + (dynamic_cast<IntelliSenseItemVariable^>(VarList[ReturnVar]))->GetDataTypeID();

			return Description;
		}

		IntelliSenseItemUserFunction::IntelliSenseItemUserFunction(UserFunction^ Parent) :
				IntelliSenseItem(Parent->Describe(),
								IntelliSenseItemType::UserFunction),
				Parent(Parent)
		{
			;//
		}

		String^ IntelliSenseItemUserFunction::GetIdentifier()
		{
			return Parent->GetIdentifier();
		}

		String^ IntelliSenseItemUserFunction::GetSubstitution()
		{
			return GetIdentifier();
		}

		String^ IntelliSenseItemEditorIDForm::GetFormTypeIdentifier()
		{
			return gcnew String(NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormTypeIDLongName(TypeID));
		}

		IntelliSenseItemEditorIDForm::IntelliSenseItemEditorIDForm( ComponentDLLInterface::FormData* Data ) : IntelliSenseItem()
		{
			this->Type = IntelliSenseItem::IntelliSenseItemType::Form;

			TypeID = Data->TypeID;
			FormID = Data->FormID;
			Name = gcnew String(Data->EditorID);
			Flags = Data->Flags;

			String^ FlagDescription = "" + ((Flags & (UInt32)FormFlags::FromMaster)?"   From Master File\n":"") +
				((Flags & (UInt32)FormFlags::FromActiveFile)?"   From Active File\n":"") +
				((Flags & (UInt32)FormFlags::Deleted)?"   Deleted\n":"") +
				((Flags & (UInt32)FormFlags::TurnOffFire)?"   Turn Off Fire\n":"") +
				((Flags & (UInt32)FormFlags::QuestItem)?(TypeID == 0x31?"   Persistent\n":"   Quest Item\n"):"") +
				((Flags & (UInt32)FormFlags::Disabled)?"   Initially Disabled\n":"") +
				((Flags & (UInt32)FormFlags::VisibleWhenDistant)?"   Visible When Distant\n":"");

			String^ ScriptDescription = "";
			ComponentDLLInterface::ScriptData* ScriptableData = 0;
			if (TypeID != 13 && ISDB->GetIsIdentifierScriptableForm(Name, &ScriptableData))
			{
				if (ScriptableData && ScriptableData->IsValid())
					ScriptDescription += "\nScript: " + gcnew String(ScriptableData->EditorID);

				NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(ScriptableData, false);
			}

			this->Description = Name +
								"\n\nType: " + GetFormTypeIdentifier() +
								"\nFormID: " + FormID.ToString("X8") +
								(FlagDescription->Length ? "\nFlags:\n" + FlagDescription : "") +
								ScriptDescription;
		}

		String^ IntelliSenseItemEditorIDForm::GetIdentifier()
		{
			return Name;
		}

		String^ IntelliSenseItemEditorIDForm::GetSubstitution()
		{
			return GetIdentifier();
		}

		IntelliSenseItemCodeSnippet::IntelliSenseItemCodeSnippet( CodeSnippet^ Source ) :
			IntelliSenseItem()
		{
			this->Type = IntelliSenseItem::IntelliSenseItemType::Snippet;

			Parent = Source;

			this->Description = "Name: " + Parent->Name + "\n" +
				"Shorthand: " + Parent->Shorthand +
				(Parent->Description->Length ? "\n\n" + Parent->Description : "") +
				(Parent->Variables->Count ? "\n\nVariables: " + Parent->Variables->Count.ToString() : "");
		}

		void IntelliSenseItemCodeSnippet::Insert(TextEditors::IScriptTextEditor^ Editor)
		{
			for each (CodeSnippet::VariableInfo^ Itr in Parent->Variables)
				Editor->InsertVariable(Itr->Name, Itr->Type);

			String^ Code = GetSubstitution();
			UInt32 CurrentLineIndents = Editor->GetIndentLevel(Editor->CurrentLine);
			Code = ObScriptParsing::AnalysisData::PerformLocalizedIndenting(Code, CurrentLineIndents);

			Editor->BeginUpdate();
			Editor->SetTokenAtCaretPos(Code);
			Editor->EndUpdate(false);

			Editor->ScrollToCaret();
		}

		bool IntelliSenseItemCodeSnippet::GetShouldEnumerate( String^ Token, bool SubstringSearch )
		{
			bool Found = (Parent->Name->IndexOf(Token, System::StringComparison::CurrentCultureIgnoreCase) != -1);

			if (Found == false)
				Found = (Parent->Shorthand->IndexOf(Token, System::StringComparison::CurrentCultureIgnoreCase) != -1);

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

		bool IntelliSenseItemCodeSnippet::GetIsQuickViewable(String^ Token)
		{
			return false;
		}
	}
}