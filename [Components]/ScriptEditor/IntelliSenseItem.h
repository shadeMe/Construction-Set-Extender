#pragma once

#include "[Common]\HandShakeStructs.h"
#include "SemanticAnalysis.h"
#include "ITextEditor.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


ref class IntelliSenseInterface;
ref class CodeSnippet;

static enum class eStringMatchType
{
	StartsWith,
	Substring,
	FullMatch
};

bool DoStringMatch(String^ Source, String^ Target, eStringMatchType Comparison);

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
public:
	static enum class eItemType
	{
		None = 0,
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

	static Image^ GetItemTypeIcon(eItemType Type);
protected:
	static Dictionary<eItemType, Image^>^ ItemTypeIcons = gcnew Dictionary<eItemType, Image^>;

	eItemType ItemType;
	String^ HelpTextHeader;
	String^ HelpTextBody;
	String^ HelpTextFooter;

	String^ GenerateHelpTextHeader(String^ Identifier);
	String^ GenerateHelpTextFooter();
public:
	IntelliSenseItem();
	IntelliSenseItem(eItemType Type);

	virtual void Insert(textEditor::ITextEditor^ Editor);

	virtual bool MatchesToken(String^ Token, eStringMatchType Comparison);
	virtual bool HasInsightInfo();

	virtual String^ GetIdentifier() abstract;
	virtual String^ GetSubstitution() abstract;
	virtual String^ GetItemTypeName();
	eItemType GetItemType();

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
		Image^ get() { return GetItemTypeIcon(ItemType); }
		void set(Image^ set) {}
	}
	virtual property IRichTooltipContentProvider::eBackgroundColor TooltipBgColor
	{
		IRichTooltipContentProvider::eBackgroundColor get()
		{ return IRichTooltipContentProvider::eBackgroundColor::Default; }
		void set(IRichTooltipContentProvider::eBackgroundColor set) {}
	}
};

ref struct ScriptCommandParameter
{
	String^ TypeName;
	String^ Description;
	bool Optional;

	ScriptCommandParameter(componentDLLInterface::ObScriptCommandInfo::ParamInfo* ParamInfo);
};

ref class IntelliSenseItemScriptCommand : public IntelliSenseItem
{
protected:
	static enum class eSourceType
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

	static enum class eReturnValueType
	{
		Default = 0,
		Form,
		String,
		Array,
		ArrayIndex,
		Ambiguous
	};

	static String^ GetPrettyNameForObsePlugin(String^ PluginName);

	String^ Name;
	eSourceType Source;
	String^ SourceName;
	UInt32 SourceVersion;
	String^ Description;
	String^ Shorthand;
	eReturnValueType ResultType;
	List<ScriptCommandParameter^>^ Parameters;
	bool RequireCallingRef;
	String^ DocumentationUrl;
public:
	IntelliSenseItemScriptCommand(componentDLLInterface::CommandTableData* CommandTableData,
								  const componentDLLInterface::ObScriptCommandInfo* CommandInfo,
								  String^ DeveloperUrl);

	virtual bool MatchesToken(String^ Token, eStringMatchType Comparison) override;
	virtual String^ GetIdentifier() override;
	virtual String^ GetSubstitution() override;
	bool RequiresCallingRef();
	String^ GetShorthand();
	String^ GetDocumentationUrl();
};

ref class IntelliSenseItemScriptVariable : public IntelliSenseItem
{
protected:
	String^ Name;
	obScriptParsing::Variable::eDataType DataType;
	String^ Comment;
	String^ ParentEditorID;		// Optional editorID of the parent script; empty for ad-hoc local variables
public:
	IntelliSenseItemScriptVariable(String^ Name, String^ Comment, obScriptParsing::Variable::eDataType Type,
								   String^ ParentEditorID);

	virtual String^ GetItemTypeName() override;
	virtual String^ GetIdentifier() override;
	virtual String^ GetSubstitution() override;
	String^ GetComment();
	obScriptParsing::Variable::eDataType GetDataType();
	String^ GetDataTypeID();
};

ref class IntelliSenseItemForm : public IntelliSenseItem
{
protected:
	static enum class eFormFlags
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

	String^ EditorID;
	UInt32 TypeID;
	UInt32 FormID;
	UInt32 Flags;
	bool BaseForm;
	String^ AttachedScriptEditorID;

	String^ GetFormTypeIdentifier();

	IntelliSenseItemForm();
public:
	IntelliSenseItemForm(componentDLLInterface::FormData* Data, componentDLLInterface::ScriptData* AttachedScript);

	virtual String^ GetIdentifier() override;
	virtual String^ GetSubstitution() override;
	virtual String^ GetItemTypeName() override;
	bool IsObjectReference();
	bool HasAttachedScript();
	String^ GetAttachedScriptEditorID();
};

ref class IntelliSenseItemGlobalVariable : public IntelliSenseItemForm
{
protected:
	obScriptParsing::Variable::eDataType DataType;
	String^ Value;
public:
	IntelliSenseItemGlobalVariable(componentDLLInterface::FormData* Data, obScriptParsing::Variable::eDataType Type, String^ Value);

	virtual String^ GetItemTypeName() override;
	void SetValue(String^ Val);

	property String^ TooltipBodyText
	{
		virtual String^ get() override;
	}
};

ref class IntelliSenseItemGameSetting : public IntelliSenseItemGlobalVariable
{
public:
	IntelliSenseItemGameSetting(componentDLLInterface::FormData* Data, obScriptParsing::Variable::eDataType Type, String^ Value);

	virtual String^ GetItemTypeName() override;
};

ref class IntelliSenseItemQuest : public IntelliSenseItemForm
{
public:
	IntelliSenseItemQuest(componentDLLInterface::FormData* Data, componentDLLInterface::ScriptData* AttachedScript);
};

ref class IntelliSenseItemScript : public IntelliSenseItemForm
{
	static IntelliSenseItemScript^ Empty = gcnew IntelliSenseItemScript;
protected:
	IntelliSenseItemScript();

	List<IntelliSenseItemScriptVariable^>^ Variables;
	String^ CommentDescription;
	obScriptParsing::AnalysisData^ InitialAnalysisData;
public:
	IntelliSenseItemScript(componentDLLInterface::ScriptData* ScriptData);

	static property IntelliSenseItemScript^ Default
	{
		IntelliSenseItemScript^ get() { return Empty; }
	}

	IEnumerable<IntelliSenseItemScriptVariable^>^ GetVariables();
	IntelliSenseItemScriptVariable^ LookupVariable(String^ VariableName);
	bool IsUserDefinedFunction();
	virtual String^ GetItemTypeName() override;
};

ref class IntelliSenseItemUserFunction : public IntelliSenseItemScript
{
	static const int kReturnVarIdxNone = -1;
	static const int kReturnVarIdxAmbiguous = -9;
protected:
	List<int>^ ParameterIndices;
	int ReturnVarIndex;
public:
	IntelliSenseItemUserFunction(componentDLLInterface::ScriptData* ScriptData);
};

ref class IntelliSenseItemCodeSnippet : public IntelliSenseItem
{
protected:
	CodeSnippet^ Parent;
public:
	IntelliSenseItemCodeSnippet(CodeSnippet^ Source);

	virtual void Insert(textEditor::ITextEditor^ Editor) override;
	virtual bool MatchesToken(String^ Token, eStringMatchType Comparison) override;
	virtual bool HasInsightInfo() override;
	virtual String^ GetIdentifier() override;
	virtual String^ GetSubstitution() override;
};

ref class IntelliSenseItemSorter : public System::Collections::Generic::IComparer<IntelliSenseItem^>
{
protected:
	SortOrder Order;
public:
	IntelliSenseItemSorter(SortOrder Order) : Order(Order) {}

	virtual int Compare(IntelliSenseItem^ X, IntelliSenseItem^ Y);
};


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse