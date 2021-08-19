#pragma once

#include "SemanticAnalysis.h"
#include "IScriptEditorView.h"


namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


using namespace System::Runtime::Serialization;

[DataContract]
ref class CodeSnippet
{
public:
	[DataContract]
	ref struct VariableInfo
	{
		[DataMember]
		String^ Name;
		[DataMember]
		obScriptParsing::Variable::eDataType Type;

		VariableInfo(String^ Name, obScriptParsing::Variable::eDataType Type);
	};

	[DataMember]
	String^ Name;
	[DataMember]
	String^ Shorthand;
	[DataMember]
	String^ Description;
	[DataMember]
	List<VariableInfo^>^ Variables;
	[DataMember]
	String^ Code;

	CodeSnippet();
	~CodeSnippet();

	void AddVariable(String^ Name, obScriptParsing::Variable::eDataType Type);
	void AddVariable(VariableInfo^ Var);
	void RemoveVariable(VariableInfo^ Var);
	VariableInfo^ LookupVariable(String^ Name);
	void Save(String^ Path);
	void CopyTo(CodeSnippet^ Destination);

	static CodeSnippet^ Load(String^ FullPath);
};

ref struct CodeSnippetCollection
{
	List<CodeSnippet^>^ LoadedSnippets;

	CodeSnippetCollection();
	CodeSnippetCollection(List<CodeSnippet^>^ Source);
	~CodeSnippetCollection();

	void Add(CodeSnippet^ In);
	void Remove(CodeSnippet^ In);
	CodeSnippet^ Lookup(String^ Name);
	void Save(String^ SnippetDirectory);
	void Load(String^ SnippetDirectory);
};

ref class CodeSnippetManagerDialog : public DevComponents::DotNetBar::Metro::MetroForm
{
	System::ComponentModel::IContainer^ components;
	DevComponents::DotNetBar::Bar^ ContainerToolbar;
	DevComponents::DotNetBar::ButtonItem^ ToolbarAddSnippet;
	DevComponents::DotNetBar::ButtonItem^ ToolbarRemoveSnippet;
	BrightIdeasSoftware::ObjectListView^ SnippetsList;
	BrightIdeasSoftware::OLVColumn^ SnippetListColumnName;
	BrightIdeasSoftware::OLVColumn^ SnippetListColumnShortcut;
	DevComponents::DotNetBar::Controls::GroupPanel^ GroupSnippetData;
	DevComponents::DotNetBar::Controls::GroupPanel^ GroupVariables;
	DevComponents::DotNetBar::Bar^ VariablesToolbar;
	DevComponents::DotNetBar::ButtonItem^ VariablesToolbarAddVar;
	DevComponents::DotNetBar::ButtonItem^ VariablesToolbarRemoveVar;
	DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxShortcut;
	DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxDescription;
	DevComponents::DotNetBar::LabelX^ LabelDescription;
	DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxName;
	DevComponents::DotNetBar::LabelX^ LabelShortcut;
	DevComponents::DotNetBar::LabelX^ LabelName;
	DevComponents::DotNetBar::ButtonX^ ButtonApply;
	DevComponents::DotNetBar::Controls::GroupPanel^ GroupCode;
	DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxCode;
	BrightIdeasSoftware::ObjectListView^ VariablesList;
	BrightIdeasSoftware::OLVColumn^ VariablesListColumnName;
	BrightIdeasSoftware::OLVColumn^ VariablesListColumnType;
	DevComponents::DotNetBar::ButtonItem^ AddVarMenuInteger;
	DevComponents::DotNetBar::ButtonItem^ AddVarMenuFloat;
	DevComponents::DotNetBar::ButtonItem^ AddVarMenuRef;
	DevComponents::DotNetBar::ButtonItem^ AddVarMenuString;
	DevComponents::DotNetBar::ButtonItem^ AddVarMenuArray;
	System::Windows::Forms::Panel^ LeftPanel;
	view::IObjectListView^ SnippetsListThemeWrapper;
	view::IObjectListView^ VariablesListThemeWrapper;

	CodeSnippet^ ActiveSelectionBuffer;

	void InitializeComponent();
	void FinalizeComponents();
	void ResetFields();
	void SetFields(CodeSnippet^ Snippet);
	void PopulateSnippetList();

	void ButtonApply_Click(Object^ Sender, EventArgs^ E);
	void ToolbarAddSnippet_Click(Object^ Sender, EventArgs^ E);
	void ToolbarRemoveSnippet_Click(Object^ Sender, EventArgs^ E);
	void VariablesToolbarAddVar_Click(Object^ Sender, EventArgs^ E);
	void VariablesToolbarRemoveVar_Click(Object^ Sender, EventArgs^ E);
	void SnippetsList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);

	static Object^ SnippetsListAspectGetterName(Object^ Model);
	static Object^ SnippetsListAspectGetterShorthand(Object^ Model);
	static Object^ VariablesListAspectGetterName(Object^ Model);
	static Object^ VariablesListAspectGetterType(Object^ Model);
public:
	CodeSnippetManagerDialog(CodeSnippetCollection^ Data);
	~CodeSnippetManagerDialog();

	property CodeSnippetCollection^ WorkingCopy;
};


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse