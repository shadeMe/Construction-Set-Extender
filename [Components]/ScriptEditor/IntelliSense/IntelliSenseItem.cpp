#include "IntelliSenseItem.h"
#include "IntelliSenseDatabase.h"
#include "IntelliSenseInterface.h"

#include "..\ScriptEditor.h"
#include "..\ScriptTextEditorInterface.h"
#include "..\SnippetManager.h"

#include "[Common]\NativeWrapper.h"

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		IntelliSenseItem::IntelliSenseItem()
		{
			this->Description = "";
			this->Type = IntelliSenseItemType::e_Invalid;
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

		void IntelliSenseItem::Insert( Object^ Workspace, IntelliSenseInterface^ Interface )
		{
			ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(Workspace);

			ParentEditor->SetCurrentToken(GetSubstitution());
		}

		bool IntelliSenseItem::GetShouldEnumerate( String^ Token, bool SubstringSearch )
		{
			if (SubstringSearch)
				return (GetIdentifier()->IndexOf(Token, System::StringComparison::CurrentCultureIgnoreCase) != -1);
			else
				return GetIdentifier()->StartsWith(Token, System::StringComparison::CurrentCultureIgnoreCase);
		}

		bool IntelliSenseItem::GetIsQuickViewable()
		{
			return true;
		}

		IntelliSenseItemScriptCommand::IntelliSenseItemScriptCommand(String^% Name, String^% Desc, String^% Shorthand, UInt16 NoOfParams, bool RequiresParent, UInt16 ReturnType, IntelliSenseCommandItemSourceType Source) :
				IntelliSenseItem(String::Format("{0}{1}\n{2} parameter(s)\nReturn Type: {3}\n\n{4}{5}",
												Name,
												(Shorthand == "None")?"":("\t[ " + Shorthand + " ]"),
												NoOfParams.ToString(),
												IntelliSenseItemScriptCommand::IntelliSenseItemCommandReturnTypeID[(int)ReturnType],
												Desc, (RequiresParent)?"\n\nRequires a calling reference":""),
								IntelliSenseItemType::e_Cmd),
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

		IntelliSenseItemVariable::IntelliSenseItemVariable(String^% Name, String^% Comment, ScriptParser::VariableType Type, IntelliSenseItemType Scope) :
			IntelliSenseItem(String::Format("{0} [{1}]{2}{3}",
											Name,
											ScriptParser::GetVariableID(Type),
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
			return ScriptParser::GetVariableID(DataType);
		}

		ScriptParser::VariableType IntelliSenseItemVariable::GetDataType()
		{
			return DataType;
		}

		String^ IntelliSenseItemVariable::GetSubstitution()
		{
			return GetIdentifier();
		}

		IntelliSenseItemQuest::IntelliSenseItemQuest(String^% EditorID, String^% Desc, String^% ScrName) :
					IntelliSenseItem(EditorID + ((Desc != "")?"\n":"") + Desc + ((ScrName != "")?"\n\nQuest Script: ":"") + ScrName,
									IntelliSenseItem::IntelliSenseItemType::e_Quest),
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
			VarList = gcnew List<IntelliSenseItemVariable^>();
		}

		Script::Script(String^% ScriptText)
		{
			VarList = gcnew List<IntelliSenseItemVariable^>();
			ISDB->ParseScript(ScriptText, gcnew IntelliSenseParseScriptData(this));
		}

		Script::Script(String^% ScriptText, String^% Name)
		{
			VarList = gcnew VarListT();
			ISDB->ParseScript(ScriptText, gcnew IntelliSenseParseScriptData(this));
			this->Name = Name;
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

		void Script::SetName( String^ Name )
		{
			this->Name = Name;
		}

		void Script::SetCommentDescription( String^ Description )
		{
			this->CommentDescription = Description;
		}

		void Script::AddVariable( IntelliSenseItemVariable^ Variable )
		{
			VarList->Add(Variable);
		}

		void Script::ClearVariableList()
		{
			VarList->Clear();
		}

		List<IntelliSenseItemVariable^>::Enumerator^ Script::GetVariableListEnumerator()
		{
			return VarList->GetEnumerator();
		}

		UserFunction::UserFunction(String^% ScriptText) : Script()
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

			ISDB->ParseScript(ScriptText, gcnew IntelliSenseParseScriptData(this));
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

			Description += Name + CommentDescription + "\n" + ParamIdx + " Parameters" + Scratch + "\n\n";
			if (ReturnVar == -1)			Description += "Does not return a value";
			else if (ReturnVar == -9)		Description += "Return Type: Ambiguous";
			else							Description += "Return Type: " + (dynamic_cast<IntelliSenseItemVariable^>(VarList[ReturnVar]))->GetDataTypeID();

			return Description;
		}

		void UserFunction::AddParameter( int VariableIndex, int ParameterIndex )
		{
			Parameters->SetValue(VariableIndex, ParameterIndex);
		}

		void UserFunction::SetReturnVariable( int VariableIndex )
		{
			ReturnVar = VariableIndex;
		}

		IntelliSenseItemUserFunction::IntelliSenseItemUserFunction(UserFunction^% Parent) :
				IntelliSenseItem(Parent->Describe(),
								IntelliSenseItemType::e_UserFunct),
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
			this->Type = IntelliSenseItem::IntelliSenseItemType::e_Form;

			TypeID = Data->TypeID;
			FormID = Data->FormID;
			Name = gcnew String(Data->EditorID);
			Flags = Data->Flags;

			String^ FlagDescription = "" + ((Flags & (UInt32)FormFlags::e_FromMaster)?"   From Master File\n":"") +
				((Flags & (UInt32)FormFlags::e_FromActiveFile)?"   From Active File\n":"") +
				((Flags & (UInt32)FormFlags::e_Deleted)?"   Deleted\n":"") +
				((Flags & (UInt32)FormFlags::e_TurnOffFire)?"   Turn Off Fire\n":"") +
				((Flags & (UInt32)FormFlags::e_QuestItem)?(TypeID == 0x31?"   Persistent\n":"   Quest Item\n"):"") +
				((Flags & (UInt32)FormFlags::e_Disabled)?"   Initially Disabled\n":"") +
				((Flags & (UInt32)FormFlags::e_VisibleWhenDistant)?"   Visible When Distant\n":"");

			String^ ScriptDescription = "";
			ComponentDLLInterface::ScriptData* ScriptableData = 0;
			if (TypeID != 13 && ISDB->GetIsIdentifierScriptableForm(Name, &ScriptableData))
			{
				if (ScriptableData && ScriptableData->IsValid())
					ScriptDescription += "\nScript: " + gcnew String(ScriptableData->EditorID);

				NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(ScriptableData, false);
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
			this->Type = IntelliSenseItem::IntelliSenseItemType::e_Snippet;

			Parent = Source;

			this->Description = "Name: " + Parent->Name + "\n" +
				"Shorthand: " + Parent->Shorthand +
				(Parent->Description->Length ? "\n\n" + Parent->Description : "") +
				(Parent->Variables->Count ? "\n\nVariables: " + Parent->Variables->Count.ToString() : "");
		}

		void IntelliSenseItemCodeSnippet::Insert( Object^ Workspace, IntelliSenseInterface^ Interface )
		{
			ScriptEditor::Workspace^ ParentEditor = dynamic_cast<ScriptEditor::Workspace^>(Workspace);

			for each (CodeSnippet::VariableInfo^ Itr in Parent->Variables)
				ParentEditor->InsertVariable(Itr->Name, Itr->Type);

			String^ Code = GetSubstitution();
			UInt32 NewLines = Code->Split('\n')->Length;
			UInt32 CurrentLine = ParentEditor->GetTextEditor()->GetCurrentLineNumber();

			Interface->Enabled = false;			// don't want it popping up when indenting the snippet
			ParentEditor->GetTextEditor()->BeginUpdate();
			ParentEditor->SetCurrentToken(Code);
			ParentEditor->GetTextEditor()->IndentLines(CurrentLine, CurrentLine + NewLines);
			ParentEditor->GetTextEditor()->EndUpdate(false);
			Interface->Enabled = true;

			ParentEditor->GetTextEditor()->ScrollToCaret();
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

		bool IntelliSenseItemCodeSnippet::GetIsQuickViewable()
		{
			return false;
		}
	}
}