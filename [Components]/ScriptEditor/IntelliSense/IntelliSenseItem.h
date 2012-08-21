#pragma once

#include "..\ScriptParser.h"
#include "[Common]\HandShakeStructs.h"

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		ref class IntelliSenseInterface;
		ref class CodeSnippet;

		ref class IntelliSenseItem
		{
			static array<String^>^								IntelliSenseItemTypeID =
			{
				"Unknown Object",
				"Command",
				"Local Variable",
				"Remote Variable",
				"User Defined Function",
				"Quest",
				"Global Variable",
				"Game Setting",
				"Object",
				"Code Snippet"
			};
		public:
			static enum class									IntelliSenseItemType
			{
				e_Invalid = 0,
				e_Cmd,
				e_LocalVar,
				e_RemoteVar,
				e_UserFunct,
				e_Quest,
				e_GlobalVar,
				e_GMST,
				e_Form,
				e_Snippet,
			};

			IntelliSenseItem();
			IntelliSenseItem(String^ Desc, IntelliSenseItemType Type);

			virtual String^										Describe();
			virtual void										Insert(Object^ Workspace, IntelliSenseInterface^ Interface); // argument's a ScriptEditor::Workspace^
			virtual bool										GetShouldEnumerate(String^ Token);		// returns true if the item can be enumerated in the interface
			virtual bool										GetIsQuickViewable();					// returns true if the item allows a quick view tooltip
			virtual String^										GetIdentifier() = 0;					// identifier for display in the interface
			virtual String^										GetSubstitution() = 0;					// string to be inserted into the code

			IntelliSenseItemType								GetItemType();
			String^												GetItemTypeID();
		protected:
			String^												Description;
			IntelliSenseItemType								Type;
		};

		ref class IntelliSenseItemScriptCommand : public IntelliSenseItem
		{
		public:
			static enum class									IntelliSenseCommandItemSourceType
			{
				e_Vanilla = 0,
				e_OBSE
			};
		private:
			static array<String^>^								IntelliSenseItemCommandReturnTypeID =
			{
				"Numeric",
				"Form",
				"String",
				"Array",
				"Array [Reference]",
				"Ambiguous"
			};
			static enum class									IntelliSenseItemCommandReturnType
			{
				e_Default = 0,
				e_Form,
				e_String,
				e_Array,
				e_ArrayIndex,
				e_Ambiguous
			};

			String^												Name;
			String^												CmdDescription;
			String^												Shorthand;
			UInt16												ParamCount;
			bool												RequiresParent;
			UInt16												ReturnType;
			IntelliSenseCommandItemSourceType					Source;
		public:
			IntelliSenseItemScriptCommand(String^% Name, String^% Desc, String^% Shorthand, UInt16 NoOfParams, bool RequiresParent, UInt16 ReturnType, IntelliSenseCommandItemSourceType Source);

			virtual String^										GetIdentifier() override;
			virtual String^										GetSubstitution() override;
			bool												GetRequiresParent();
			IntelliSenseCommandItemSourceType					GetSource();
		};

		ref class IntelliSenseItemVariable : public IntelliSenseItem
		{
		public:
			static array<String^>^								IntelliSenseItemVariableDataTypeID =
			{
				"Integer",
				"Float",
				"Reference",
				"String",
				"Array"
			};
		private:
			String^												Name;
			ScriptParser::VariableType							DataType;
			String^												Comment;

		public:
			IntelliSenseItemVariable(String^% Name, String^% Comment, ScriptParser::VariableType Type, IntelliSenseItemType Scope);

			virtual String^										GetIdentifier() override;
			virtual String^										GetSubstitution() override;
			String^												GetComment();
			ScriptParser::VariableType							GetDataType();
			String^												GetDataTypeID();
		};

		ref class IntelliSenseItemQuest : public IntelliSenseItem
		{
		protected:
			String^												Name;
			String^												ScriptName;
		public:
			IntelliSenseItemQuest(String^% EditorID, String^% Desc, String^% ScrName);

			virtual String^										GetIdentifier() override;
			virtual String^										GetSubstitution() override;
		};

		ref class Script
		{
		public:
			typedef List<IntelliSenseItemVariable^>				VarListT;
		protected:
			Script();
			Script(String^% ScriptText, String^% Name);

			VarListT^											VarList;
			String^												Name;
			String^												CommentDescription;
		public:
			static Script^										NullScript = gcnew Script(gcnew String("scn nullscript"));

			Script(String^% ScriptText);

			virtual String^										Describe();
			virtual String^										GetIdentifier();

			void												SetName(String^ Name);
			void												SetCommentDescription(String^ Description);

			void												AddVariable(IntelliSenseItemVariable^ Variable);
			void												ClearVariableList();
			List<IntelliSenseItemVariable^>::Enumerator^		GetVariableListEnumerator();
		};

		ref class UserFunction : public Script
		{
		protected:
			Array^												Parameters;			// indices of the parameters in VarList
			int													ReturnVar;			// index of the return var. -9 for ambiguous retn values
		public:
			UserFunction(String^% ScriptText);

			virtual String^										Describe() override;

			void												AddParameter(int VariableIndex, int ParameterIndex);
			void												SetReturnVariable(int VariableIndex);
		};

		ref class IntelliSenseItemUserFunction : public IntelliSenseItem
		{
			UserFunction^										Parent;
		public:
			IntelliSenseItemUserFunction(UserFunction^% Parent);

			virtual String^										GetIdentifier() override;
			virtual String^										GetSubstitution() override;
		};

		ref class IntelliSenseItemEditorIDForm : public IntelliSenseItem
		{
		protected:
			static enum class									FormFlags
			{
				e_FromMaster           = /*00*/ 0x00000001,
				e_FromActiveFile       = /*01*/ 0x00000002,
				e_Deleted              = /*05*/ 0x00000020,
				e_TurnOffFire          = /*07*/ 0x00000080,
				e_QuestItem            = /*0A*/ 0x00000400,
				e_Disabled             = /*0B*/ 0x00000800,
				e_Ignored              = /*0C*/ 0x00001000,
				e_Temporary            = /*0E*/ 0x00004000,
				e_VisibleWhenDistant   = /*0F*/ 0x00008000,
			};

			String^												Name;
			UInt32												TypeID;
			UInt32												FormID;
			UInt32												Flags;

			String^												GetFormTypeIdentifier();
		public:
			IntelliSenseItemEditorIDForm(ComponentDLLInterface::FormData* Data);

			virtual String^										GetIdentifier() override;
			virtual String^										GetSubstitution() override;
		};

		ref class IntelliSenseItemCodeSnippet : public IntelliSenseItem
		{
		protected:
			CodeSnippet^										Parent;
		public:
			IntelliSenseItemCodeSnippet(CodeSnippet^ Source);

			virtual void										Insert(Object^ Workspace, IntelliSenseInterface^ Interface) override;
			virtual bool										GetShouldEnumerate(String^ Token) override;
			virtual bool										GetIsQuickViewable() override;
			virtual String^										GetIdentifier() override;
			virtual String^										GetSubstitution() override;
		};
	}
}