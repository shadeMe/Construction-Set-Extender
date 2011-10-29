#include "IntelliSenseItem.h"
#include "IntelliSenseDatabase.h"

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

		IntelliSenseItem::IntelliSenseItemType IntelliSenseItem::GetIntelliSenseItemType()
		{
			return Type;
		}

		String^ IntelliSenseItem::GetIntelliSenseItemTypeID()
		{
			return IntelliSenseItemTypeID[(int)Type];
		}

		String^ IntelliSenseItem::Describe()
		{
			return Description;
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

		IntelliSenseItemVariable::IntelliSenseItemVariable(String^% Name, String^% Comment, IntelliSenseItemVariableDataType Type, IntelliSenseItemType Scope) :
			IntelliSenseItem(String::Format("{0} [{1}]{2}{3}",
											Name,
											IntelliSenseItemVariable::IntelliSenseItemVariableDataTypeID[(int)Type],
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
			return IntelliSenseItemVariableDataTypeID[(int)DataType];
		}

		IntelliSenseItemVariable::IntelliSenseItemVariableDataType IntelliSenseItemVariable::GetDataType()
		{
			return DataType;
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

			Description += Name + "\n\nDescription: " + CommentDescription + "\n" + ParamIdx + " Parameters" + Scratch + "\n\n";
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

		String^ IntelliSenseItemEditorIDForm::GetFormTypeIdentifier()
		{
			static const char* s_FormTypeIdentifier[] =
			{
				"None",
				"TES4",
				"Group",
				"GMST",
				"Global",
				"Class",
				"Faction",
				"Hair",
				"Eyes",
				"Race",
				"Sound",
				"Skill",
				"Effect",
				"Script",
				"Land Texture",
				"Enchantment",
				"Spell",
				"BirthSign",
				"Activator",
				"Apparatus",
				"Armor",
				"Book",
				"Clothing",
				"Container",
				"Door",
				"Ingredient",
				"Light",
				"Misc Item",
				"Static",
				"Grass",
				"Tree",
				"Flora",
				"Furniture",
				"Weapon",
				"Ammo",
				"NPC",
				"Creature",
				"Leveled Creature",
				"SoulGem",
				"Key",
				"Alchemy Item",
				"SubSpace",
				"Sigil Stone",
				"Leveled Item",
				"SNDG",
				"Weather",
				"Climate",
				"Region",
				"Cell",
				"Reference",
				"Reference",			// ACHR
				"Reference",			// ACRE
				"PathGrid",
				"World Space",
				"Land",
				"TLOD",
				"Road",
				"Dialog",
				"Dialog Info",
				"Quest",
				"Idle",
				"AI Package",
				"Combat Style",
				"Load Screen",
				"Leveled Spell",
				"Anim Object",
				"Water Type",
				"Effect Shader",
				"TOFT"
			};

			if (FormType >= 0x45)
				return "Unknown";
			else
				return gcnew String(s_FormTypeIdentifier[FormType]);
		}

		IntelliSenseItemEditorIDForm::IntelliSenseItemEditorIDForm( ComponentDLLInterface::FormData* Data ) : IntelliSenseItem()
		{
			this->Type = IntelliSenseItem::IntelliSenseItemType::e_Form;

			FormType = Data->TypeID;
			FormID = Data->FormID;
			Name = gcnew String(Data->EditorID);
			Flags = Data->Flags;

			String^ FlagDescription = "" + ((Flags & (UInt32)FormFlags::e_FromMaster)?"   From Master File\n":"") +
				((Flags & (UInt32)FormFlags::e_FromActiveFile)?"   From Active File\n":"") +
				((Flags & (UInt32)FormFlags::e_Deleted)?"   Deleted\n":"") +
				((Flags & (UInt32)FormFlags::e_TurnOffFire)?"   Turn Off Fire\n":"") +
				((Flags & (UInt32)FormFlags::e_QuestItem)?(FormType == 0x31?"   Persistent\n":"   Quest Item\n"):"") +
				((Flags & (UInt32)FormFlags::e_Disabled)?"   Initially Disabled\n":"") +
				((Flags & (UInt32)FormFlags::e_VisibleWhenDistant)?"   Visible When Distant\n":"");

			this->Description = Name + "\n\nType: " + GetFormTypeIdentifier() + "\nFormID: " + FormID.ToString("X8") +
				(FlagDescription->Length?"\nFlags:\n" + FlagDescription:"");
		}

		String^ IntelliSenseItemEditorIDForm::GetIdentifier()
		{
			return Name;
		}
	}
}