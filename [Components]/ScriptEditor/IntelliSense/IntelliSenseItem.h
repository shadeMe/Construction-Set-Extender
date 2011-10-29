#pragma once
#include "[Common]\HandShakeStructs.h"

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		ref class IntelliSenseInterface;

		public ref class IntelliSenseItem
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
				"Object"
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
				e_Form
			};

			IntelliSenseItem();
			IntelliSenseItem(String^ Desc, IntelliSenseItemType Type);

			virtual String^										Describe();
			virtual String^										GetIdentifier() = 0;
			IntelliSenseItemType								GetIntelliSenseItemType();
			String^												GetIntelliSenseItemTypeID();
		protected:
			String^												Description;
			IntelliSenseItemType								Type;
		};

		public ref class IntelliSenseItemScriptCommand : public IntelliSenseItem
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
			bool												GetRequiresParent();
			IntelliSenseCommandItemSourceType					GetSource();
		};

		public ref class IntelliSenseItemVariable : public IntelliSenseItem
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
			static enum class									IntelliSenseItemVariableDataType
			{
				e_Int = 0,
				e_Float,
				e_Ref,
				e_String,
				e_Array
			};
		private:
			String^												Name;
			IntelliSenseItemVariableDataType					DataType;
			String^												Comment;

		public:
			IntelliSenseItemVariable(String^% Name, String^% Comment, IntelliSenseItemVariableDataType Type, IntelliSenseItemType Scope);

			virtual String^										GetIdentifier() override;
			String^												GetComment();
			IntelliSenseItemVariableDataType					GetDataType();
			String^												GetDataTypeID();
		};

		public ref class IntelliSenseItemQuest : public IntelliSenseItem
		{
		protected:
			String^												Name;
			String^												ScriptName;
		public:
			IntelliSenseItemQuest(String^% EditorID, String^% Desc, String^% ScrName);

			virtual String^										GetIdentifier() override;
		};

		public ref class Script
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

		public ref class UserFunction : public Script
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

		public ref class IntelliSenseItemUserFunction : public IntelliSenseItem
		{
			UserFunction^										Parent;
		public:
			IntelliSenseItemUserFunction(UserFunction^% Parent);

			virtual String^										GetIdentifier() override;
		};

		public ref class IntelliSenseItemEditorIDForm : public IntelliSenseItem
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
			UInt32												FormType;
			UInt32												FormID;
			UInt32												Flags;

			String^												GetFormTypeIdentifier();
		public:
			IntelliSenseItemEditorIDForm(ComponentDLLInterface::FormData* Data);

			virtual String^										GetIdentifier() override;
		};

		public ref struct IntelliSenseParseScriptData
		{
			IntelliSenseInterface^								SourceIntelliSenseInterface;
			Script^												SourceScript;

			static enum class									DataType
			{
				e_UserFunction = 0,
				e_Script,
				e_IntelliSenseInterface
			};

			DataType											Type;

			IntelliSenseParseScriptData(IntelliSenseInterface^ Obj) : SourceIntelliSenseInterface(Obj), Type(DataType::e_IntelliSenseInterface) {};
			IntelliSenseParseScriptData(Script^ Obj) : SourceScript(Obj), Type(DataType::e_Script) {};
			IntelliSenseParseScriptData(UserFunction^ Obj) : SourceScript(Obj), Type(DataType::e_UserFunction) {};
		};
	}
}