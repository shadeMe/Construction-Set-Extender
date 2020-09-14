#pragma once

#include "[Common]\HandShakeStructs.h"
#include "SemanticAnalysis.h"
#include "ScriptTextEditorInterface.h"

namespace cse
{
	namespace intellisense
	{
		ref class IntelliSenseInterface;
		ref class CodeSnippet;

		static enum class StringMatchType
		{
			StartsWith,
			Substring,
			FullMatch
		};

		bool DoStringMatch(String^ Source, String^ Target, StringMatchType Comparison);


		ref class IntelliSenseItem
		{
			static array<String^>^ ItemTypeID =
			{
				"Unknown Object",
				"Script Command",
				"Script Variable",
				"Quest",
				"Script",
				"User Defined Function",
				"Game Setting",
				"Global Variable",
				"Form",
				"Code Snippet"
			};
		public:
			static enum class ItemType
			{
				Invalid = 0,
				ScriptCommand,
				ScriptVariable,
				Quest,
				Script,
				UserFunction,
				GameSetting,
				GlobalVariable,
				Form,
				Snippet,
			};

			IntelliSenseItem();
			IntelliSenseItem(ItemType Type);

			virtual String^		Describe();
			virtual void		Insert(textEditors::IScriptTextEditor^ Editor);

			virtual bool		MatchesToken(String^ Token, StringMatchType Comparison);
			virtual bool		HasInsightInfo();

			virtual String^		GetIdentifier() abstract;
			virtual String^		GetSubstitution() abstract;

			ItemType			GetItemType();
			virtual String^		GetItemTypeName();
		protected:
			String^				Description;
			ItemType			Type;
		};

		ref class IntelliSenseItemScriptCommand : public IntelliSenseItem
		{
		public:
			static enum class SourceType
			{
				Vanilla = 0,
				OBSE
			};
		private:
			static array<String^>^ ReturnValueTypeID =
			{
				"Numeric",
				"Form",
				"String",
				"Array",
				"Array [Reference]",
				"Ambiguous"
			};
			static enum class ReturnValueType
			{
				Default = 0,
				Form,
				String,
				Array,
				ArrayIndex,
				Ambiguous
			};

			String^						Name;
			String^						CmdDescription;
			String^						Shorthand;
			UInt16						ParamCount;
			bool						RequiresParent;
			ReturnValueType				ResultType;
			SourceType					Source;
			String^						DeveloperURL;
		public:
			IntelliSenseItemScriptCommand(String^ Name,
										  String^ Desc,
										  String^ Shorthand,
										  UInt16 NoOfParams,
										  bool RequiresParent,
										  UInt16 ReturnType,
										  SourceType Source,
										  String^ Params,
										  String^ DeveloperURL);

			virtual bool				MatchesToken(String^ Token, StringMatchType Comparison) override;
			virtual String^				GetIdentifier() override;
			virtual String^				GetSubstitution() override;
			bool						GetRequiresParent();
			SourceType					GetSource();
			String^						GetShorthand();
			String^						GetDeveloperURL();
			void						SetDeveloperURL(String^ URL);
		};

		ref class IntelliSenseItemScriptVariable : public IntelliSenseItem
		{
		protected:
			String^								Name;
			obScriptParsing::Variable::DataType	DataType;
			String^								Comment;
			String^								ParentEditorID;		// Optional editorID of the parent script
																	// Empty for ad-hoc local variables.
		public:
			IntelliSenseItemScriptVariable(String^ Name, String^ Comment, obScriptParsing::Variable::DataType Type,
										String^ ParentEditorID);

			virtual String^							GetItemTypeName() override;
			virtual String^							GetIdentifier() override;
			virtual String^							GetSubstitution() override;
			String^									GetComment();
			obScriptParsing::Variable::DataType		GetDataType();
			String^									GetDataTypeID();
		};

		ref class IntelliSenseItemForm : public IntelliSenseItem
		{
		protected:
			static enum class FormFlags
			{
				FromMaster           = /*00*/ 0x00000001,
				FromActiveFile       = /*01*/ 0x00000002,
				Deleted              = /*05*/ 0x00000020,
				TurnOffFire          = /*07*/ 0x00000080,
				QuestItem            = /*0A*/ 0x00000400,
				Disabled             = /*0B*/ 0x00000800,
				Ignored              = /*0C*/ 0x00001000,
				Temporary            = /*0E*/ 0x00004000,
				VisibleWhenDistant   = /*0F*/ 0x00008000,
			};

			String^		EditorID;
			UInt32		TypeID;
			UInt32		FormID;
			UInt32		Flags;
			bool		BaseForm;
			String^		AttachedScriptEditorID;

			String^		GetFormTypeIdentifier();

			IntelliSenseItemForm();
		public:
			IntelliSenseItemForm(componentDLLInterface::FormData* Data, componentDLLInterface::ScriptData* AttachedScript);

			virtual String^ GetIdentifier() override;
			virtual String^ GetSubstitution() override;
			virtual String^ GetItemTypeName() override;
			bool			IsObjectReference();
			bool			HasAttachedScript();
			String^			GetAttachedScriptEditorID();
		};

		ref class IntelliSenseItemGlobalVariable : public IntelliSenseItemForm
		{
		protected:
			obScriptParsing::Variable::DataType	DataType;
			String^								Value;
		public:
			IntelliSenseItemGlobalVariable(componentDLLInterface::FormData* Data,
								obScriptParsing::Variable::DataType Type, String^ Value);

			virtual String^ Describe() override;
			virtual String^ GetItemTypeName() override;
			void			SetValue(String^ Val);
		};

		ref class IntelliSenseItemGameSetting : public IntelliSenseItemGlobalVariable
		{
		public:
			IntelliSenseItemGameSetting(componentDLLInterface::FormData* Data,
				obScriptParsing::Variable::DataType Type, String^ Value);
		};

		ref class IntelliSenseItemQuest : public IntelliSenseItemForm
		{
		protected:
			String^		FullName;
		public:
			IntelliSenseItemQuest(componentDLLInterface::FormData* Data,
								componentDLLInterface::ScriptData* AttachedScript,
								String^ FullName);

			String^		GetFullName();
		};

		ref class IntelliSenseItemScript : public IntelliSenseItemForm
		{
			static IntelliSenseItemScript^ Empty = gcnew IntelliSenseItemScript;
		protected:
			IntelliSenseItemScript();

			List<IntelliSenseItemScriptVariable^>^	VarList;
			String^									CommentDescription;
			obScriptParsing::AnalysisData^			InitialAnalysisData;
		public:
			IntelliSenseItemScript(componentDLLInterface::ScriptData* ScriptData);

			static property IntelliSenseItemScript^ Default
			{
				IntelliSenseItemScript^ get() { return Empty; }
			}

			virtual String^								GetItemTypeName() override;
			IEnumerable<IntelliSenseItemScriptVariable^>^
														GetVariables();
			IntelliSenseItemScriptVariable^				LookupVariable(String^ VariableName);
			bool										IsUserDefinedFunction();
		};

		ref class IntelliSenseItemUserFunction : public IntelliSenseItemScript
		{
			static const int	kReturnVarIdxNone = -1;
			static const int	kReturnVarIdxAmbiguous = -9;
		protected:
			List<int>^			ParameterIndices;
			int					ReturnVarIndex;
		public:
			IntelliSenseItemUserFunction(componentDLLInterface::ScriptData* ScriptData);
		};

		ref class IntelliSenseItemCodeSnippet : public IntelliSenseItem
		{
		protected:
			CodeSnippet^	Parent;
		public:
			IntelliSenseItemCodeSnippet(CodeSnippet^ Source);

			virtual void	Insert(textEditors::IScriptTextEditor^ Editor) override;
			virtual bool	MatchesToken(String^ Token, StringMatchType Comparison) override;
			virtual bool	HasInsightInfo() override;
			virtual String^ GetIdentifier() override;
			virtual String^ GetSubstitution() override;
		};

		ref class IntelliSenseItemSorter : public System::Collections::Generic::IComparer<IntelliSenseItem^>
		{
		protected:
			SortOrder	Order;
		public:
			IntelliSenseItemSorter(SortOrder Order) : Order(Order) {}

			virtual int Compare(IntelliSenseItem^ X, IntelliSenseItem^ Y);
		};
	}
}