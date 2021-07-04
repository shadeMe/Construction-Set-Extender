#include "SnippetManager.h"
#include "IScriptEditorView.h"
#include "Utilities.h"


namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


using namespace System::IO;
using namespace System::Windows::Forms;

CodeSnippet::VariableInfo::VariableInfo(String^ Name, obScriptParsing::Variable::eDataType Type)
{
	this->Name = Name;
	this->Type = Type;
}

void CodeSnippet::Save( String^ Path )
{
	try
	{
		DataContractSerializer^ Serializer = gcnew DataContractSerializer(CodeSnippet::typeid);
		FileStream^ OutStream =  gcnew FileStream(Path + "\\" + Name + ".csesnippet", FileMode::Create);

		Serializer->WriteObject(OutStream, this);
		OutStream->Close();
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't serialize code snippet '" + Name + "'!\n\tException: " + E->Message, true);
	}
}

void CodeSnippet::CopyTo(CodeSnippet^ Destination)
{
	Destination->Name = this->Name;
	Destination->Shorthand = this->Shorthand;
	Destination->Description = this->Description;
	Destination->Code = this->Code;

	Destination->Variables = gcnew List<VariableInfo^>;
	for each (auto Itr in this->Variables)
		Destination->Variables->Add(gcnew VariableInfo(Itr->Name, Itr->Type));
}

CodeSnippet^ CodeSnippet::Load( String^ FullPath )
{
	CodeSnippet^ Result = nullptr;

	try
	{
		DataContractSerializer^ Serializer = gcnew DataContractSerializer(CodeSnippet::typeid);
		FileStream^ InStream = gcnew FileStream(FullPath, FileMode::Open);

		Result = dynamic_cast<CodeSnippet^>(Serializer->ReadObject(InStream));
		InStream->Close();
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't de-serialize code snippet at '" + FullPath + "'!\n\tException: " + E->Message, true);
	}

	return Result;
}

void CodeSnippet::AddVariable(String^ Name, obScriptParsing::Variable::eDataType Type)
{
	if (LookupVariable(Name) == nullptr)
		Variables->Add(gcnew VariableInfo(Name, Type));
}

void CodeSnippet::AddVariable( VariableInfo^ Var )
{
	if (LookupVariable(Var->Name) == nullptr)
		Variables->Add(Var);
}

void CodeSnippet::RemoveVariable( VariableInfo^ Var )
{
	if (LookupVariable(Var->Name))
		Variables->Remove(Var);
}

CodeSnippet::VariableInfo^ CodeSnippet::LookupVariable( String^ Name )
{
	for each (VariableInfo^ Itr in Variables)
	{
		if (!String::Compare(Itr->Name, Name, true))
			return Itr;
	}

	return nullptr;
}

CodeSnippet::CodeSnippet() :
	Variables(gcnew List<VariableInfo^>())
{
	Name = "";
	Shorthand = "";
	Description = "";
	Code = "";
}

CodeSnippet::~CodeSnippet()
{
	Variables->Clear();
}

CodeSnippetCollection::CodeSnippetCollection() :
	LoadedSnippets(gcnew List<CodeSnippet^>())
{
	;//
}

CodeSnippetCollection::CodeSnippetCollection( List<CodeSnippet^>^ Source ) :
	LoadedSnippets(gcnew List<CodeSnippet^>(Source))
{
	;//
}

CodeSnippetCollection::~CodeSnippetCollection()
{
	LoadedSnippets->Clear();
}

CodeSnippet^ CodeSnippetCollection::Lookup( String^ Name )
{
	for each (CodeSnippet^ Itr in LoadedSnippets)
	{
		if (!String::Compare(Itr->Name, Name, true))
			return Itr;
	}

	return nullptr;
}

void CodeSnippetCollection::Save( String^ SnippetDirectory )
{
	try
	{
		Directory::Delete(SnippetDirectory, true);
		Directory::CreateDirectory(SnippetDirectory);

		for each (CodeSnippet^ Itr in LoadedSnippets)
			Itr->Save(SnippetDirectory);
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't save code snippet collection to '" + SnippetDirectory + "'!\n\tException: " + E->Message, true);
	}
}

void CodeSnippetCollection::Load( String^ SnippetDirectory )
{
	try
	{
		if (Directory::Exists(SnippetDirectory) == false)
			Directory::CreateDirectory(SnippetDirectory);

		for each (String^ Itr in Directory::GetFiles(SnippetDirectory))
		{
			CodeSnippet^ New = CodeSnippet::Load(Itr);
			if (New != nullptr)
				Add(New);
		}
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't load code snippet collection from '" + SnippetDirectory + "'!\n\tException: " + E->Message, true);
	}
}

void CodeSnippetCollection::Add( CodeSnippet^ In )
{
	if (Lookup(In->Name) == nullptr)
		LoadedSnippets->Add(In);
}

void CodeSnippetCollection::Remove( CodeSnippet^ In )
{
	if (Lookup(In->Name) != nullptr)
		LoadedSnippets->Remove(In);
}

void CodeSnippetManagerDialog::InitializeComponent()
{
	auto resources = (gcnew System::Resources::ResourceManager("ScriptEditor.CodeSnippetsDialog", Assembly::GetExecutingAssembly()));
	this->ContainerToolbar = (gcnew DevComponents::DotNetBar::Bar());
	this->ToolbarAddSnippet = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->ToolbarRemoveSnippet = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->SnippetsList = (gcnew BrightIdeasSoftware::ObjectListView());
	this->SnippetListColumnName = (gcnew BrightIdeasSoftware::OLVColumn());
	this->SnippetListColumnShortcut = (gcnew BrightIdeasSoftware::OLVColumn());
	this->GroupSnippetData = (gcnew DevComponents::DotNetBar::Controls::GroupPanel());
	this->ButtonApply = (gcnew DevComponents::DotNetBar::ButtonX());
	this->GroupCode = (gcnew DevComponents::DotNetBar::Controls::GroupPanel());
	this->TextBoxCode = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
	this->GroupVariables = (gcnew DevComponents::DotNetBar::Controls::GroupPanel());
	this->VariablesList = (gcnew BrightIdeasSoftware::ObjectListView());
	this->VariablesListColumnName = (gcnew BrightIdeasSoftware::OLVColumn());
	this->VariablesListColumnType = (gcnew BrightIdeasSoftware::OLVColumn());
	this->VariablesToolbar = (gcnew DevComponents::DotNetBar::Bar());
	this->VariablesToolbarAddVar = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->AddVarMenuInteger = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->AddVarMenuFloat = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->AddVarMenuRef = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->AddVarMenuString = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->AddVarMenuArray = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->VariablesToolbarRemoveVar = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->TextBoxShortcut = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
	this->TextBoxDescription = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
	this->LabelDescription = (gcnew DevComponents::DotNetBar::LabelX());
	this->TextBoxName = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
	this->LabelShortcut = (gcnew DevComponents::DotNetBar::LabelX());
	this->LabelName = (gcnew DevComponents::DotNetBar::LabelX());
	this->LeftPanel = (gcnew System::Windows::Forms::Panel());
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ContainerToolbar))->BeginInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->SnippetsList))->BeginInit();
	this->GroupSnippetData->SuspendLayout();
	this->GroupCode->SuspendLayout();
	this->GroupVariables->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->VariablesList))->BeginInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->VariablesToolbar))->BeginInit();
	this->LeftPanel->SuspendLayout();
	this->SuspendLayout();
	this->ContainerToolbar->AntiAlias = true;
	this->ContainerToolbar->Dock = System::Windows::Forms::DockStyle::Top;
	this->ContainerToolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
	this->ContainerToolbar->IsMaximized = false;
	this->ContainerToolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
		this->ToolbarAddSnippet,
			this->ToolbarRemoveSnippet
	});
	this->ContainerToolbar->ItemSpacing = 5;
	this->ContainerToolbar->Location = System::Drawing::Point(0, 0);
	this->ContainerToolbar->Name = L"ContainerToolbar";
	this->ContainerToolbar->Size = System::Drawing::Size(286, 29);
	this->ContainerToolbar->Stretch = true;
	this->ContainerToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->ContainerToolbar->TabIndex = 0;
	this->ContainerToolbar->TabStop = false;
	this->ToolbarAddSnippet->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarAddSnippet.Image")));
	this->ToolbarAddSnippet->ImagePaddingHorizontal = 15;
	this->ToolbarAddSnippet->ImagePaddingVertical = 10;
	this->ToolbarAddSnippet->Name = L"ToolbarAddSnippet";
	this->ToolbarAddSnippet->Text = L"New Snippet";
	this->ToolbarAddSnippet->Tooltip = L"New Snippet";
	this->ToolbarRemoveSnippet->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarRemoveSnippet.Image")));
	this->ToolbarRemoveSnippet->ImagePaddingHorizontal = 15;
	this->ToolbarRemoveSnippet->ImagePaddingVertical = 10;
	this->ToolbarRemoveSnippet->Name = L"ToolbarRemoveSnippet";
	this->ToolbarRemoveSnippet->Text = L"Remove Snippet";
	this->ToolbarRemoveSnippet->Tooltip = L"Remove Snippet";
	this->SnippetsList->AllColumns->Add(this->SnippetListColumnName);
	this->SnippetsList->AllColumns->Add(this->SnippetListColumnShortcut);
	this->SnippetsList->CellEditUseWholeCell = false;
	this->SnippetsList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {
		this->SnippetListColumnName,
			this->SnippetListColumnShortcut
	});
	this->SnippetsList->Cursor = System::Windows::Forms::Cursors::Default;
	this->SnippetsList->Dock = System::Windows::Forms::DockStyle::Fill;
	this->SnippetsList->FullRowSelect = true;
	this->SnippetsList->HideSelection = false;
	this->SnippetsList->Location = System::Drawing::Point(0, 29);
	this->SnippetsList->MultiSelect = false;
	this->SnippetsList->ShowGroups = false;
	this->SnippetsList->Name = L"SnippetsList";
	this->SnippetsList->Size = System::Drawing::Size(286, 603);
	this->SnippetsList->TabIndex = 1;
	this->SnippetsList->UseCompatibleStateImageBehavior = false;
	this->SnippetsList->View = System::Windows::Forms::View::Details;
	this->SnippetListColumnName->Text = L"Name";
	this->SnippetListColumnName->Width = 211;
	this->SnippetListColumnShortcut->Text = L"Shortcut";
	this->SnippetListColumnShortcut->Width = 65;
	this->GroupSnippetData->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->GroupSnippetData->Controls->Add(this->ButtonApply);
	this->GroupSnippetData->Controls->Add(this->GroupCode);
	this->GroupSnippetData->Controls->Add(this->GroupVariables);
	this->GroupSnippetData->Controls->Add(this->TextBoxShortcut);
	this->GroupSnippetData->Controls->Add(this->TextBoxDescription);
	this->GroupSnippetData->Controls->Add(this->LabelDescription);
	this->GroupSnippetData->Controls->Add(this->TextBoxName);
	this->GroupSnippetData->Controls->Add(this->LabelShortcut);
	this->GroupSnippetData->Controls->Add(this->LabelName);
	this->GroupSnippetData->DisabledBackColor = System::Drawing::Color::Empty;
	this->GroupSnippetData->Location = System::Drawing::Point(311, 12);
	this->GroupSnippetData->Name = L"GroupSnippetData";
	this->GroupSnippetData->Size = System::Drawing::Size(453, 632);
	this->GroupSnippetData->Style->BackColor2SchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground2;
	this->GroupSnippetData->Style->BackColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
	this->GroupSnippetData->Style->BorderColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
	this->GroupSnippetData->Style->CornerType = DevComponents::DotNetBar::eCornerType::Rounded;
	this->GroupSnippetData->Style->TextColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
	this->GroupSnippetData->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->GroupSnippetData->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->GroupSnippetData->TabIndex = 2;
	this->GroupSnippetData->Text = L"Snippet Data";
	this->ButtonApply->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
	this->ButtonApply->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
	this->ButtonApply->Location = System::Drawing::Point(339, 576);
	this->ButtonApply->Name = L"ButtonApply";
	this->ButtonApply->Size = System::Drawing::Size(101, 28);
	this->ButtonApply->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->ButtonApply->TabIndex = 8;
	this->ButtonApply->Text = L"Apply Changes";
	this->GroupCode->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->GroupCode->Controls->Add(this->TextBoxCode);
	this->GroupCode->DisabledBackColor = System::Drawing::Color::Empty;
	this->GroupCode->Location = System::Drawing::Point(9, 323);
	this->GroupCode->Name = L"GroupCode";
	this->GroupCode->Size = System::Drawing::Size(434, 247);
	this->GroupCode->Style->BackColor2SchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground2;
	this->GroupCode->Style->BackColorGradientAngle = 90;
	this->GroupCode->Style->BackColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
	this->GroupCode->Style->BorderBottom = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupCode->Style->BorderBottomWidth = 1;
	this->GroupCode->Style->BorderColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
	this->GroupCode->Style->BorderLeft = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupCode->Style->BorderLeftWidth = 1;
	this->GroupCode->Style->BorderRight = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupCode->Style->BorderRightWidth = 1;
	this->GroupCode->Style->BorderTop = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupCode->Style->BorderTopWidth = 1;
	this->GroupCode->Style->CornerDiameter = 4;
	this->GroupCode->Style->CornerType = DevComponents::DotNetBar::eCornerType::Rounded;
	this->GroupCode->Style->TextAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Center;
	this->GroupCode->Style->TextColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
	this->GroupCode->Style->TextLineAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Near;
	this->GroupCode->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->GroupCode->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->GroupCode->TabIndex = 7;
	this->GroupCode->Text = L"Code";
	this->TextBoxCode->Border->Class = L"TextBoxBorder";
	this->TextBoxCode->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->TextBoxCode->DisabledBackColor = System::Drawing::Color::Black;
	this->TextBoxCode->Dock = System::Windows::Forms::DockStyle::Fill;
	this->TextBoxCode->Location = System::Drawing::Point(0, 0);
	this->TextBoxCode->Multiline = true;
	this->TextBoxCode->Name = L"TextBoxCode";
	this->TextBoxCode->PreventEnterBeep = true;
	this->TextBoxCode->Size = System::Drawing::Size(428, 224);
	this->TextBoxCode->TabIndex = 8;
	this->TextBoxCode->AcceptsTab = true;
	this->TextBoxCode->AcceptsReturn = true;
	this->GroupVariables->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->GroupVariables->Controls->Add(this->VariablesList);
	this->GroupVariables->Controls->Add(this->VariablesToolbar);
	this->GroupVariables->DisabledBackColor = System::Drawing::Color::Empty;
	this->GroupVariables->Location = System::Drawing::Point(9, 158);
	this->GroupVariables->Name = L"GroupVariables";
	this->GroupVariables->Size = System::Drawing::Size(434, 159);
	this->GroupVariables->Style->BackColor2SchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground2;
	this->GroupVariables->Style->BackColorGradientAngle = 90;
	this->GroupVariables->Style->BackColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
	this->GroupVariables->Style->BorderBottom = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupVariables->Style->BorderBottomWidth = 1;
	this->GroupVariables->Style->BorderColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
	this->GroupVariables->Style->BorderLeft = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupVariables->Style->BorderLeftWidth = 1;
	this->GroupVariables->Style->BorderRight = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupVariables->Style->BorderRightWidth = 1;
	this->GroupVariables->Style->BorderTop = DevComponents::DotNetBar::eStyleBorderType::Solid;
	this->GroupVariables->Style->BorderTopWidth = 1;
	this->GroupVariables->Style->CornerDiameter = 4;
	this->GroupVariables->Style->CornerType = DevComponents::DotNetBar::eCornerType::Rounded;
	this->GroupVariables->Style->TextAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Center;
	this->GroupVariables->Style->TextColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
	this->GroupVariables->Style->TextLineAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Near;
	this->GroupVariables->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->GroupVariables->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->GroupVariables->TabIndex = 6;
	this->GroupVariables->Text = L"Variables";
	this->VariablesList->AllColumns->Add(this->VariablesListColumnName);
	this->VariablesList->AllColumns->Add(this->VariablesListColumnType);
	this->VariablesList->CellEditUseWholeCell = false;
	this->VariablesList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {
		this->VariablesListColumnName,
			this->VariablesListColumnType
	});
	this->VariablesList->Cursor = System::Windows::Forms::Cursors::Default;
	this->VariablesList->Dock = System::Windows::Forms::DockStyle::Fill;
	this->VariablesList->HideSelection = false;
	this->VariablesList->Location = System::Drawing::Point(0, 29);
	this->VariablesList->MultiSelect = false;
	this->VariablesList->FullRowSelect = true;
	this->VariablesList->ShowGroups = false;
	this->VariablesList->Name = L"VariablesList";
	this->VariablesList->Size = System::Drawing::Size(428, 107);
	this->VariablesList->TabIndex = 1;
	this->VariablesList->UseCompatibleStateImageBehavior = false;
	this->VariablesList->View = System::Windows::Forms::View::Details;
	this->VariablesListColumnName->Text = L"Name";
	this->VariablesListColumnName->Width = 217;
	this->VariablesListColumnType->Text = L"Data Type";
	this->VariablesListColumnType->Width = 93;
	this->VariablesToolbar->AntiAlias = true;
	this->VariablesToolbar->Dock = System::Windows::Forms::DockStyle::Top;
	this->VariablesToolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
	this->VariablesToolbar->IsMaximized = false;
	this->VariablesToolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
		this->VariablesToolbarAddVar,
			this->VariablesToolbarRemoveVar
	});
	this->VariablesToolbar->ItemSpacing = 5;
	this->VariablesToolbar->Location = System::Drawing::Point(0, 0);
	this->VariablesToolbar->Name = L"VariablesToolbar";
	this->VariablesToolbar->Size = System::Drawing::Size(428, 29);
	this->VariablesToolbar->Stretch = true;
	this->VariablesToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->VariablesToolbar->TabIndex = 0;
	this->VariablesToolbar->TabStop = false;
	this->VariablesToolbar->Text = L"bar1";
	this->VariablesToolbarAddVar->AutoExpandOnClick = true;
	this->VariablesToolbarAddVar->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"VariablesToolbarAddVar.Image")));
	this->VariablesToolbarAddVar->ImagePaddingHorizontal = 10;
	this->VariablesToolbarAddVar->ImagePaddingVertical = 10;
	this->VariablesToolbarAddVar->Name = L"VariablesToolbarAddVar";
	this->VariablesToolbarAddVar->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(5) {
		this->AddVarMenuInteger,
			this->AddVarMenuFloat, this->AddVarMenuRef, this->AddVarMenuString, this->AddVarMenuArray
	});
	this->VariablesToolbarAddVar->Text = L"Add Variable...";
	this->VariablesToolbarAddVar->Tooltip = L"Add Variable...";
	this->AddVarMenuInteger->Name = L"AddVarMenuInteger";
	this->AddVarMenuInteger->Text = L"Integer";
	this->AddVarMenuFloat->Name = L"AddVarMenuFloat";
	this->AddVarMenuFloat->Text = L"Float";
	this->AddVarMenuRef->Name = L"AddVarMenuRef";
	this->AddVarMenuRef->Text = L"Reference";
	this->AddVarMenuString->Name = L"AddVarMenuString";
	this->AddVarMenuString->Text = L"String";
	this->AddVarMenuArray->Name = L"AddVarMenuArray";
	this->AddVarMenuArray->Text = L"Array";
	this->VariablesToolbarRemoveVar->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"VariablesToolbarRemoveVar.Image")));
	this->VariablesToolbarRemoveVar->ImagePaddingHorizontal = 10;
	this->VariablesToolbarRemoveVar->ImagePaddingVertical = 10;
	this->VariablesToolbarRemoveVar->Name = L"VariablesToolbarRemoveVar";
	this->VariablesToolbarRemoveVar->Text = L"Remove Variable";
	this->VariablesToolbarRemoveVar->Tooltip = L"Remove Variable";
	this->TextBoxShortcut->Border->Class = L"TextBoxBorder";
	this->TextBoxShortcut->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->TextBoxShortcut->DisabledBackColor = System::Drawing::Color::Black;
	this->TextBoxShortcut->Location = System::Drawing::Point(80, 50);
	this->TextBoxShortcut->Name = L"TextBoxShortcut";
	this->TextBoxShortcut->PreventEnterBeep = true;
	this->TextBoxShortcut->Size = System::Drawing::Size(366, 22);
	this->TextBoxShortcut->TabIndex = 5;
	this->TextBoxDescription->Border->Class = L"TextBoxBorder";
	this->TextBoxDescription->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->TextBoxDescription->DisabledBackColor = System::Drawing::Color::Black;
	this->TextBoxDescription->Location = System::Drawing::Point(80, 87);
	this->TextBoxDescription->Multiline = true;
	this->TextBoxDescription->Name = L"TextBoxDescription";
	this->TextBoxDescription->PreventEnterBeep = true;
	this->TextBoxDescription->Size = System::Drawing::Size(366, 65);
	this->TextBoxDescription->TabIndex = 4;
	this->LabelDescription->AutoSize = true;
	this->LabelDescription->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->LabelDescription->Location = System::Drawing::Point(9, 87);
	this->LabelDescription->Name = L"LabelDescription";
	this->LabelDescription->Size = System::Drawing::Size(58, 17);
	this->LabelDescription->TabIndex = 3;
	this->LabelDescription->Text = L"Description";
	this->TextBoxName->Border->Class = L"TextBoxBorder";
	this->TextBoxName->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->TextBoxName->DisabledBackColor = System::Drawing::Color::Black;
	this->TextBoxName->Location = System::Drawing::Point(80, 12);
	this->TextBoxName->Name = L"TextBoxName";
	this->TextBoxName->PreventEnterBeep = true;
	this->TextBoxName->Size = System::Drawing::Size(366, 22);
	this->TextBoxName->TabIndex = 2;
	this->LabelShortcut->AutoSize = true;
	this->LabelShortcut->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->LabelShortcut->Location = System::Drawing::Point(9, 50);
	this->LabelShortcut->Name = L"LabelShortcut";
	this->LabelShortcut->Size = System::Drawing::Size(44, 17);
	this->LabelShortcut->TabIndex = 1;
	this->LabelShortcut->Text = L"Shortcut";
	this->LabelName->AutoSize = true;
	this->LabelName->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->LabelName->Location = System::Drawing::Point(9, 12);
	this->LabelName->Name = L"LabelName";
	this->LabelName->Size = System::Drawing::Size(32, 17);
	this->LabelName->TabIndex = 0;
	this->LabelName->Text = L"Name";
	this->LeftPanel->Controls->Add(this->SnippetsList);
	this->LeftPanel->Controls->Add(this->ContainerToolbar);
	this->LeftPanel->Location = System::Drawing::Point(12, 12);
	this->LeftPanel->Name = L"LeftPanel";
	this->LeftPanel->Size = System::Drawing::Size(286, 632);
	this->LeftPanel->TabIndex = 3;
	this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
	this->ClientSize = System::Drawing::Size(776, 656);
	this->Controls->Add(this->LeftPanel);
	this->Controls->Add(this->GroupSnippetData);
	this->DoubleBuffered = true;
	this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
	this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
	this->MaximizeBox = false;
	this->Name = L"ScriptEditorCodeSnippets";
	this->Text = L"Code Snippets";
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ContainerToolbar))->EndInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->SnippetsList))->EndInit();
	this->GroupSnippetData->ResumeLayout(false);
	this->GroupSnippetData->PerformLayout();
	this->GroupCode->ResumeLayout(false);
	this->GroupVariables->ResumeLayout(false);
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->VariablesList))->EndInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->VariablesToolbar))->EndInit();
	this->LeftPanel->ResumeLayout(false);
	this->ResumeLayout(false);
}


void CodeSnippetManagerDialog::FinalizeComponents()
{
	SnippetListColumnName->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CodeSnippetManagerDialog::SnippetsListAspectGetterName);
	SnippetListColumnShortcut->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CodeSnippetManagerDialog::SnippetsListAspectGetterShorthand);
	VariablesListColumnName->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CodeSnippetManagerDialog::VariablesListAspectGetterName);
	VariablesListColumnType->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CodeSnippetManagerDialog::VariablesListAspectGetterType);

	AddVarMenuInteger->Tag = obScriptParsing::Variable::eDataType::Integer;
	AddVarMenuFloat->Tag = obScriptParsing::Variable::eDataType::Float;
	AddVarMenuRef->Tag = obScriptParsing::Variable::eDataType::Ref;
	AddVarMenuString->Tag = obScriptParsing::Variable::eDataType::StringVar;
	AddVarMenuArray->Tag = obScriptParsing::Variable::eDataType::ArrayVar;

	ToolbarAddSnippet->Click += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::ToolbarAddSnippet_Click);
	ToolbarRemoveSnippet->Click += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::ToolbarRemoveSnippet_Click);
	VariablesToolbarRemoveVar->Click += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::VariablesToolbarRemoveVar_Click);
	AddVarMenuInteger->Click += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::VariablesToolbarAddVar_Click);
	AddVarMenuFloat->Click += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::VariablesToolbarAddVar_Click);
	AddVarMenuRef->Click += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::VariablesToolbarAddVar_Click);
	AddVarMenuString->Click += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::VariablesToolbarAddVar_Click);
	AddVarMenuArray->Click += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::VariablesToolbarAddVar_Click);
	ButtonApply->Click += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::ButtonApply_Click);

	SnippetsList->SelectedIndexChanged += gcnew System::EventHandler(this, &CodeSnippetManagerDialog::SnippetsList_SelectedIndexChanged);

	ResetFields();
}

void CodeSnippetManagerDialog::ResetFields()
{
	ButtonApply->Enabled = false;
	TextBoxName->Enabled = false;
	TextBoxShortcut->Enabled = false;
	TextBoxDescription->Enabled = false;
	TextBoxCode->Enabled = false;
	VariablesList->Enabled = false;
	VariablesToolbarAddVar->Enabled = false;
	VariablesToolbarRemoveVar->Enabled = false;

	TextBoxName->Text = String::Empty;
	TextBoxShortcut->Text = String::Empty;
	TextBoxDescription->Text = String::Empty;
	TextBoxCode->Text = String::Empty;

	VariablesList->ClearObjects();
}

void CodeSnippetManagerDialog::SetFields(CodeSnippet^ Snippet)
{
	ButtonApply->Enabled = true;
	TextBoxName->Enabled = true;
	TextBoxShortcut->Enabled = true;
	TextBoxDescription->Enabled = true;
	TextBoxCode->Enabled = true;
	VariablesList->Enabled = true;
	VariablesToolbarAddVar->Enabled = true;
	VariablesToolbarRemoveVar->Enabled = true;

	Snippet->CopyTo(ActiveSelectionBuffer);

	TextBoxName->Text = ActiveSelectionBuffer->Name;
	TextBoxShortcut->Text = ActiveSelectionBuffer->Shorthand;
	TextBoxDescription->Text = ActiveSelectionBuffer->Description;
	TextBoxCode->Text = ActiveSelectionBuffer->Code;
	VariablesList->SetObjects(ActiveSelectionBuffer->Variables);
}


CodeSnippetManagerDialog::CodeSnippetManagerDialog( CodeSnippetCollection^ Data )
{
	if (Data == nullptr)
		Data = gcnew CodeSnippetCollection();

	InitializeComponent();
	FinalizeComponents();

	WorkingCopy = Data;
	ActiveSelectionBuffer = gcnew CodeSnippet;

	PopulateSnippetList();

	this->ShowDialog();
}

CodeSnippetManagerDialog::~CodeSnippetManagerDialog()
{
	if (components)
	{
		delete components;
	}

	WorkingCopy = nullptr;
}

void CodeSnippetManagerDialog::PopulateSnippetList()
{
	Debug::Assert(WorkingCopy != nullptr);

	SnippetsList->SetObjects(WorkingCopy->LoadedSnippets);
}

void CodeSnippetManagerDialog::ButtonApply_Click( Object^ Sender, EventArgs^ E )
{
	auto Selection = safe_cast<CodeSnippet^>(SnippetsList->SelectedObject);
	Debug::Assert(Selection != nullptr);

	String^ ReservedCharacters = "\\/:*?\"<>|";
	String^ NewName = TextBoxName->Text;

	if (NewName->Length == 0)
	{
		MessageBox::Show("Snippet must have a name",
						 view::IScriptEditorView::MainWindowDefaultTitle,
						 MessageBoxButtons::OK,
						 MessageBoxIcon::Exclamation);
		return;
	}
	else if (NewName->IndexOfAny(ReservedCharacters->ToCharArray()) != -1)
	{
		MessageBox::Show("Snippet name cannot contain any of the following characters: " + ReservedCharacters,
						 view::IScriptEditorView::MainWindowDefaultTitle,
						 MessageBoxButtons::OK,
						 MessageBoxIcon::Exclamation);
		return;
	}
	else if (String::Compare(NewName, Selection->Name, true) && WorkingCopy->Lookup(NewName))
	{
		MessageBox::Show("Snippet name must be unique.",
						 view::IScriptEditorView::MainWindowDefaultTitle,
						 MessageBoxButtons::OK,
						 MessageBoxIcon::Exclamation);
		return;
	}

	ActiveSelectionBuffer->Name = NewName;
	ActiveSelectionBuffer->Shorthand = TextBoxShortcut->Text;
	ActiveSelectionBuffer->Description = TextBoxDescription->Text;
	ActiveSelectionBuffer->Code = TextBoxCode->Text;

	ActiveSelectionBuffer->CopyTo(Selection);
}

void CodeSnippetManagerDialog::ToolbarAddSnippet_Click( Object^ Sender, EventArgs^ E )
{
	auto Result = InputBox::Show("Enter Snippet Name", "New Snippet", "", Handle);
	if (Result->ReturnCode == System::Windows::Forms::DialogResult::Cancel || Result->Text == "")
		return;
	else if (WorkingCopy->Lookup(Result->Text))
	{
		MessageBox::Show("Snippet name must be unique.",
						 view::IScriptEditorView::MainWindowDefaultTitle,
						 MessageBoxButtons::OK,
						 MessageBoxIcon::Exclamation);
		return;
	}

	auto NewSnippet = gcnew CodeSnippet;
	NewSnippet->Name = Result->Text;
	WorkingCopy->Add(NewSnippet);

	SnippetsList->SetObjects(WorkingCopy->LoadedSnippets, true);
	SnippetsList->SelectObject(NewSnippet);
}

void CodeSnippetManagerDialog::ToolbarRemoveSnippet_Click( Object^ Sender, EventArgs^ E )
{
	auto Selection = safe_cast<CodeSnippet^>(SnippetsList->SelectedObject);
	if (Selection == nullptr)
		return;

	WorkingCopy->Remove(Selection);
	SnippetsList->SetObjects(WorkingCopy->LoadedSnippets, true);
	ResetFields();
}

void CodeSnippetManagerDialog::VariablesToolbarAddVar_Click( Object^ Sender, EventArgs^ E )
{
	auto MenuItem = safe_cast<DevComponents::DotNetBar::ButtonItem^>(Sender);
	auto VarType = safe_cast<obScriptParsing::Variable::eDataType>(MenuItem->Tag);

	auto Result = InputBox::Show("Enter Variable Name", "Add Variable", "", Handle);
	if (Result->ReturnCode == System::Windows::Forms::DialogResult::Cancel || Result->Text == "")
		return;
	else if (ActiveSelectionBuffer->LookupVariable(Result->Text))
	{
		MessageBox::Show("A variable with that name already exists.",
						 view::IScriptEditorView::MainWindowDefaultTitle,
						 MessageBoxButtons::OK,
						 MessageBoxIcon::Exclamation);
		return;
	}

	ActiveSelectionBuffer->AddVariable(Result->Text, VarType);
	VariablesList->SetObjects(ActiveSelectionBuffer->Variables, true);
}

void CodeSnippetManagerDialog::VariablesToolbarRemoveVar_Click( Object^ Sender, EventArgs^ E )
{
	auto Selection = safe_cast<CodeSnippet::VariableInfo^>(VariablesList->SelectedObject);
	if (Selection == nullptr)
		return;

	ActiveSelectionBuffer->RemoveVariable(Selection);
	VariablesList->SetObjects(ActiveSelectionBuffer->Variables, true);
}

void CodeSnippetManagerDialog::SnippetsList_SelectedIndexChanged( Object^ Sender, EventArgs^ E )
{
	ResetFields();

	auto Selection = safe_cast<CodeSnippet^>(SnippetsList->SelectedObject);
	if (Selection == nullptr)
		return;

	SetFields(Selection);
}


System::Object^ CodeSnippetManagerDialog::SnippetsListAspectGetterName(Object ^ Model)
{
	auto Snippet = safe_cast<CodeSnippet^>(Model);
	if (Snippet == nullptr)
		return nullptr;

	return Snippet->Name;
}

System::Object^ CodeSnippetManagerDialog::SnippetsListAspectGetterShorthand(Object^ Model)
{
	auto Snippet = safe_cast<CodeSnippet^>(Model);
	if (Snippet == nullptr)
		return nullptr;

	return Snippet->Shorthand;
}

System::Object^ CodeSnippetManagerDialog::VariablesListAspectGetterName(Object^ Model)
{
	auto Variable = safe_cast<CodeSnippet::VariableInfo^>(Model);
	if (Variable == nullptr)
		return nullptr;

	return Variable->Name;
}

System::Object^ CodeSnippetManagerDialog::VariablesListAspectGetterType(Object^ Model)
{
	auto Variable = safe_cast<CodeSnippet::VariableInfo^>(Model);
	if (Variable == nullptr)
		return nullptr;

	return Variable->Type;
}


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse