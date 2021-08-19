#pragma once

#include "[Common]\HandShakeStructs.h"
#include "SemanticAnalysis.h"
#include "ITextEditor.h"
#include "Utilities.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


ref class IntelliSenseInterface;
ref class CodeSnippet;


ref class IntelliSenseItem : public utilities::IRichTooltipContentProvider
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
		"Code Snippet",
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
	String^ WrapMarkupInDefaultFontSize(String^ Markup);
public:
	IntelliSenseItem();
	IntelliSenseItem(eItemType Type);

	virtual void Insert(textEditor::ITextEditor^ Editor);

	virtual String^ GetIdentifier() abstract;
	virtual String^ GetAlternateIdentifier();
	virtual bool HasAlternateIdentifier();
	virtual String^ GetSubstitution();
	virtual String^ GetItemTypeName();
	eItemType GetItemType();
	virtual bool HasInsightInfo();

	virtual property String^ TooltipHeaderText
	{
		String^ get() { return HelpTextHeader; }
	}
	virtual property String^ TooltipBodyText
	{
		String^ get() { return WrapMarkupInDefaultFontSize(HelpTextBody); }
	}
	virtual property Image^	TooltipBodyImage
	{
		Image^ get() { return nullptr; }
	}
	virtual property String^ TooltipFooterText
	{
		String^ get() { return WrapMarkupInDefaultFontSize(HelpTextFooter); }
	}
	virtual property Image^	TooltipFooterImage
	{
		Image^ get() { return GetItemTypeIcon(ItemType); }
	}
	virtual property IRichTooltipContentProvider::eBackgroundColor TooltipBgColor
	{
		IRichTooltipContentProvider::eBackgroundColor get()
		{ return IRichTooltipContentProvider::eBackgroundColor::Default; }
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

	virtual String^ GetIdentifier() override;
	virtual String^ GetAlternateIdentifier() override;
	virtual bool HasAlternateIdentifier() override;
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
	String^ GetComment();
	obScriptParsing::Variable::eDataType GetDataType();
	String^ GetDataTypeID();
};

ref class IntelliSenseItemForm : public IntelliSenseItem
{
	static enum class eFormType
	{
		None          = 0x00,
		TES4,
		Group,
		GMST,
		Global,
		Class,
		Faction,
		Hair,
		Eyes          = 0x08,
		Race,
		Sound,
		Skill,
		EffectSetting,
		Script,
		LandTexture,
		Enchantment,
		Spell         = 0x10,
		BirthSign,
		Activator     = 0x12,
		Apparatus     = 0x13,
		Armor         = 0x14,
		Book          = 0x15,
		Clothing      = 0x16,
		Container     = 0x17,
		Door          = 0x18,
		Ingredient    = 0x19,
		Light         = 0x1A,
		Misc          = 0x1B,
		Static        = 0x1C,
		Grass         = 0x1D,
		Tree          = 0x1E,
		Flora         = 0x1F,
		Furniture     = 0x20,
		Weapon        = 0x21,
		Ammo          = 0x22,
		NPC           = 0x23,
		Creature      = 0x24,
		LeveledCreature,
		SoulGem       = 0x26,
		Key           = 0x27,
		AlchemyItem   = 0x28,
		SubSpace,
		SigilStone,
		LeveledItem,
		SNDG,
		Weather,
		Climate,
		Region,
		Cell          = 0x30,
		REFR,
		ACHR,
		ACRE,
		PathGrid,
		WorldSpace,
		Land,
		TLOD,
		Road          = 0x38,
		Topic,
		TopicInfo,
		Quest,
		Idle,
		Package,
		CombatStyle,
		LoadScreen,
		LeveledSpell  = 0x40,
		AnimObject,
		WaterForm,
		EffectShader,
		TOFT          = 0x44,
	};

	static Image^ GetFormTypeIcon(eFormType Type);
	static Dictionary<eFormType, Image^>^ FormTypeIcons = gcnew Dictionary<eFormType, Image^>;
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
	eFormType TypeID;
	UInt32 FormID;
	UInt32 Flags;
	bool BaseForm;
	String^ AttachedScriptEditorID;

	String^ GetFormTypeIdentifier();

	IntelliSenseItemForm();
public:
	IntelliSenseItemForm(nativeWrapper::MarshalledFormData^ Data);

	virtual String^ GetIdentifier() override;
	virtual String^ GetItemTypeName() override;
	bool IsObjectReference();
	bool HasAttachedScript();
	String^ GetAttachedScriptEditorID();

	property Image^	TooltipFooterImage
	{
		virtual Image^ get() override;
	}
};

ref class IntelliSenseItemGlobalVariable : public IntelliSenseItemForm
{
protected:
	obScriptParsing::Variable::eDataType DataType;
	String^ Value;
public:
	IntelliSenseItemGlobalVariable(nativeWrapper::MarshalledVariableData^ Data);

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
	IntelliSenseItemGameSetting(nativeWrapper::MarshalledVariableData^ Data);

	virtual String^ GetItemTypeName() override;
};

ref class IntelliSenseItemQuest : public IntelliSenseItemForm
{
public:
	IntelliSenseItemQuest(nativeWrapper::MarshalledFormData^ Data);
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
	IntelliSenseItemScript(nativeWrapper::MarshalledScriptData^ ScriptData);

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
	IntelliSenseItemUserFunction(nativeWrapper::MarshalledScriptData^ ScriptData);
};

ref class IntelliSenseItemCodeSnippet : public IntelliSenseItem
{
protected:
	CodeSnippet^ Parent;
public:
	IntelliSenseItemCodeSnippet(CodeSnippet^ Source);

	virtual void Insert(textEditor::ITextEditor^ Editor) override;
	virtual bool HasInsightInfo() override;
	virtual String^ GetIdentifier() override;
	virtual String^ GetAlternateIdentifier() override;
	virtual bool HasAlternateIdentifier() override;
	virtual String^ GetSubstitution() override;
};


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse