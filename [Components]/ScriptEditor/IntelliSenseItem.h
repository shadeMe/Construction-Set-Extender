#pragma once

#include "[Common]\HandShakeStructs.h"
#include "SemanticAnalysis.h"
#include "ScriptTextEditorInterface.h"
#include "Globals.h"

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

		ref class IntelliSenseItem : public IRichTooltipContentProvider
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

			static array<Image^>^ ItemTypeImages =
			{
				Globals::ImageResources()->CreateImage("IntelliSenseItemEmpty"),
				Globals::ImageResources()->CreateImage("IntelliSenseItemCommand"),
				Globals::ImageResources()->CreateImage("IntelliSenseItemLocalVar"),
				Globals::ImageResources()->CreateImage("IntelliSenseItemQuest"),
				Globals::ImageResources()->CreateImage("IntelliSenseItemUDF"),
				Globals::ImageResources()->CreateImage("IntelliSenseItemUDF"),
				Globals::ImageResources()->CreateImage("IntelliSenseItemGMST"),
				Globals::ImageResources()->CreateImage("IntelliSenseItemGlobalVar"),
				Globals::ImageResources()->CreateImage("IntelliSenseItemForm"),
				Globals::ImageResources()->CreateImage("IntelliSenseItemSnippet"),
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

			static void		PopulateImageListWithItemTypeImages(ImageList^ Destination);
		protected:
			ItemType	Type;
			String^		HelpTextHeader;
			String^		HelpTextBody;
			String^		HelpTextFooter;

			String^		GenerateHelpTextHeader(String^ Identifier);
			String^		GenerateHelpTextFooter();
		public:
			IntelliSenseItem();
			IntelliSenseItem(ItemType Type);

			virtual void		Insert(textEditors::IScriptTextEditor^ Editor);

			virtual bool		MatchesToken(String^ Token, StringMatchType Comparison);
			virtual bool		HasInsightInfo();

			virtual String^		GetIdentifier() abstract;
			virtual String^		GetSubstitution() abstract;
			virtual String^		GetItemTypeName();
			ItemType			GetItemType();

			virtual property String^ TooltipHeaderText
			{
				String^ get() { return HelpTextHeader; }
				void set(String^ set) {}
			}
			virtual property String^ TooltipBodyText
			{
				String^ get() { return HelpTextBody; }
				void set(String^ set) {}
			}
			virtual property Image^	TooltipBodyImage
			{
				Image^ get() { return nullptr; }
				void set(Image^ set) {}
			}
			virtual property String^ TooltipFooterText
			{
				String^ get() { return HelpTextFooter; }
				void set(String^ set) {}
			}
			virtual property Image^	TooltipFooterImage
			{
				Image^ get() { return ItemTypeImages[safe_cast<int>(Type)]; }
				void set(Image^ set) {}
			}
			virtual property IRichTooltipContentProvider::BackgroundColor TooltipBgColor
			{
				IRichTooltipContentProvider::BackgroundColor get()
				{ return IRichTooltipContentProvider::BackgroundColor::Default; }
				void set(IRichTooltipContentProvider::BackgroundColor set) {}
			}
		};

		ref struct ScriptCommandParameter
		{
			String^	TypeName;
			String^ Description;
			bool	Optional;

			ScriptCommandParameter(componentDLLInterface::ObScriptCommandInfo::ParamInfo* ParamInfo);
		};

		ref class IntelliSenseItemScriptCommand : public IntelliSenseItem
		{
		protected:
			static enum class SourceType
			{
				Vanilla = 0,
				OBSE,
				OBSEPlugin
			};

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

			static String^	GetPrettyNameForObsePlugin(String^ PluginName);

			String^		Name;
			SourceType	Source;
			String^		SourceName;
			UInt32		SourceVersion;
			String^		Description;
			String^		Shorthand;
			ReturnValueType
						ResultType;
			List<ScriptCommandParameter^>^
						Parameters;
			bool		RequireCallingRef;
			String^		DocumentationUrl;
		public:
			IntelliSenseItemScriptCommand(componentDLLInterface::CommandTableData* CommandTableData,
										  const componentDLLInterface::ObScriptCommandInfo* CommandInfo,
										  String^ DeveloperUrl);

			virtual bool		MatchesToken(String^ Token, StringMatchType Comparison) override;
			virtual String^		GetIdentifier() override;
			virtual String^		GetSubstitution() override;
			bool				RequiresCallingRef();
			String^				GetShorthand();
			String^				GetDocumentationUrl();
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
			IntelliSenseItemScriptVariable(String^ Name,
										   String^ Comment,
										   obScriptParsing::Variable::DataType Type,
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

			void				SetValue(String^ Val);
			virtual String^		GetItemTypeName() override;

			property String^ TooltipBodyText
			{
				virtual String^ get() override;
			}
		};

		ref class IntelliSenseItemGameSetting : public IntelliSenseItemGlobalVariable
		{
		public:
			IntelliSenseItemGameSetting(componentDLLInterface::FormData* Data,
				obScriptParsing::Variable::DataType Type, String^ Value);

			virtual String^		GetItemTypeName() override;
		};

		ref class IntelliSenseItemQuest : public IntelliSenseItemForm
		{
		public:
			IntelliSenseItemQuest(componentDLLInterface::FormData* Data,
								componentDLLInterface::ScriptData* AttachedScript);
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

			IEnumerable<IntelliSenseItemScriptVariable^>^
								GetVariables();
			IntelliSenseItemScriptVariable^				
								LookupVariable(String^ VariableName);
			bool				IsUserDefinedFunction();
			virtual String^		GetItemTypeName() override;
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