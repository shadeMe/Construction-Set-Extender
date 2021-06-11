#pragma once

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for ScriptEditorCodeSnippets
	/// </summary>
	public ref class ScriptEditorCodeSnippets : public DevComponents::DotNetBar::Metro::MetroForm
	{
	public:
		ScriptEditorCodeSnippets(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~ScriptEditorCodeSnippets()
		{
			if (components)
			{
				delete components;
			}
		}
    private: DevComponents::DotNetBar::Bar^ ContainerToolbar;
    protected:
    private: DevComponents::DotNetBar::ButtonItem^ ToolbarAddSnippet;
    private: DevComponents::DotNetBar::ButtonItem^ ToolbarRemoveSnippet;
    private: BrightIdeasSoftware::ObjectListView^ SnippetsList;
    private: BrightIdeasSoftware::OLVColumn^ SnippetListColumnName;
    private: BrightIdeasSoftware::OLVColumn^ SnippetListColumnShortcut;

    private: DevComponents::DotNetBar::Controls::GroupPanel^ GroupSnippetData;
    private: DevComponents::DotNetBar::Controls::GroupPanel^ GroupVariables;
    private: DevComponents::DotNetBar::Bar^ VariablesToolbar;
    private: DevComponents::DotNetBar::ButtonItem^ VariablesToolbarAddVar;
    private: DevComponents::DotNetBar::ButtonItem^ VariablesToolbarRemoveVar;
    private: DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxShortcut;

    private: DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxDescription;

    private: DevComponents::DotNetBar::LabelX^ LabelDescription;
    private: DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxName;


    private: DevComponents::DotNetBar::LabelX^ LabelShortcut;

    private: DevComponents::DotNetBar::LabelX^ LabelName;
    private: DevComponents::DotNetBar::ButtonX^ ButtonApply;

    private: DevComponents::DotNetBar::Controls::GroupPanel^ GroupCode;
    private: DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxCode;

    private: BrightIdeasSoftware::ObjectListView^ VariablesList;
    private: BrightIdeasSoftware::OLVColumn^ VariablesListColumnName;
    private: BrightIdeasSoftware::OLVColumn^ VariablesListColumnType;
    private: DevComponents::DotNetBar::ButtonItem^ AddVarMenuInteger;
    private: DevComponents::DotNetBar::ButtonItem^ AddVarMenuFloat;
    private: DevComponents::DotNetBar::ButtonItem^ AddVarMenuRef;
    private: DevComponents::DotNetBar::ButtonItem^ AddVarMenuString;
    private: DevComponents::DotNetBar::ButtonItem^ AddVarMenuArray;
    private: System::Windows::Forms::Panel^ LeftPanel;



	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(ScriptEditorCodeSnippets::typeid));
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
            // 
            // ContainerToolbar
            // 
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
            // 
            // ToolbarAddSnippet
            // 
            this->ToolbarAddSnippet->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarAddSnippet.Image")));
            this->ToolbarAddSnippet->ImagePaddingHorizontal = 15;
            this->ToolbarAddSnippet->ImagePaddingVertical = 10;
            this->ToolbarAddSnippet->Name = L"ToolbarAddSnippet";
            this->ToolbarAddSnippet->Text = L"New Snippet";
            this->ToolbarAddSnippet->Tooltip = L"New Snippet";
            // 
            // ToolbarRemoveSnippet
            // 
            this->ToolbarRemoveSnippet->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarRemoveSnippet.Image")));
            this->ToolbarRemoveSnippet->ImagePaddingHorizontal = 15;
            this->ToolbarRemoveSnippet->ImagePaddingVertical = 10;
            this->ToolbarRemoveSnippet->Name = L"ToolbarRemoveSnippet";
            this->ToolbarRemoveSnippet->Text = L"Remove Snippet";
            this->ToolbarRemoveSnippet->Tooltip = L"Remove Snippet";
            // 
            // SnippetsList
            // 
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
            this->SnippetsList->GridLines = true;
            this->SnippetsList->HideSelection = false;
            this->SnippetsList->Location = System::Drawing::Point(0, 29);
            this->SnippetsList->MultiSelect = false;
            this->SnippetsList->Name = L"SnippetsList";
            this->SnippetsList->Size = System::Drawing::Size(286, 603);
            this->SnippetsList->TabIndex = 1;
            this->SnippetsList->UseCompatibleStateImageBehavior = false;
            this->SnippetsList->View = System::Windows::Forms::View::Details;
            // 
            // SnippetListColumnName
            // 
            this->SnippetListColumnName->Text = L"Name";
            this->SnippetListColumnName->Width = 211;
            // 
            // SnippetListColumnShortcut
            // 
            this->SnippetListColumnShortcut->Text = L"Shortcut";
            this->SnippetListColumnShortcut->Width = 65;
            // 
            // GroupSnippetData
            // 
            this->GroupSnippetData->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->GroupSnippetData->CanvasColor = System::Drawing::SystemColors::Control;
            this->GroupSnippetData->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::Office2007;
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
            // 
            // 
            // 
            this->GroupSnippetData->Style->BackColor2SchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground2;
            this->GroupSnippetData->Style->BackColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
            this->GroupSnippetData->Style->BorderColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
            this->GroupSnippetData->Style->CornerType = DevComponents::DotNetBar::eCornerType::Rounded;
            this->GroupSnippetData->Style->TextColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
            // 
            // 
            // 
            this->GroupSnippetData->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            // 
            // 
            // 
            this->GroupSnippetData->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->GroupSnippetData->TabIndex = 2;
            this->GroupSnippetData->Text = L"Snippet Data";
            // 
            // ButtonApply
            // 
            this->ButtonApply->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
            this->ButtonApply->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
            this->ButtonApply->Location = System::Drawing::Point(339, 576);
            this->ButtonApply->Name = L"ButtonApply";
            this->ButtonApply->Size = System::Drawing::Size(101, 28);
            this->ButtonApply->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
            this->ButtonApply->TabIndex = 8;
            this->ButtonApply->Text = L"Apply Changes";
            // 
            // GroupCode
            // 
            this->GroupCode->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->GroupCode->CanvasColor = System::Drawing::SystemColors::Control;
            this->GroupCode->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::Office2007;
            this->GroupCode->Controls->Add(this->TextBoxCode);
            this->GroupCode->DisabledBackColor = System::Drawing::Color::Empty;
            this->GroupCode->Location = System::Drawing::Point(9, 323);
            this->GroupCode->Name = L"GroupCode";
            this->GroupCode->Size = System::Drawing::Size(434, 247);
            // 
            // 
            // 
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
            // 
            // 
            // 
            this->GroupCode->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            // 
            // 
            // 
            this->GroupCode->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->GroupCode->TabIndex = 7;
            this->GroupCode->Text = L"Code";
            // 
            // TextBoxCode
            // 
            this->TextBoxCode->BackColor = System::Drawing::Color::Black;
            // 
            // 
            // 
            this->TextBoxCode->Border->Class = L"TextBoxBorder";
            this->TextBoxCode->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->TextBoxCode->DisabledBackColor = System::Drawing::Color::Black;
            this->TextBoxCode->Dock = System::Windows::Forms::DockStyle::Fill;
            this->TextBoxCode->ForeColor = System::Drawing::Color::White;
            this->TextBoxCode->Location = System::Drawing::Point(0, 0);
            this->TextBoxCode->Multiline = true;
            this->TextBoxCode->Name = L"TextBoxCode";
            this->TextBoxCode->PreventEnterBeep = true;
            this->TextBoxCode->Size = System::Drawing::Size(428, 224);
            this->TextBoxCode->TabIndex = 8;
            // 
            // GroupVariables
            // 
            this->GroupVariables->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->GroupVariables->CanvasColor = System::Drawing::SystemColors::Control;
            this->GroupVariables->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::Office2007;
            this->GroupVariables->Controls->Add(this->VariablesList);
            this->GroupVariables->Controls->Add(this->VariablesToolbar);
            this->GroupVariables->DisabledBackColor = System::Drawing::Color::Empty;
            this->GroupVariables->Location = System::Drawing::Point(9, 158);
            this->GroupVariables->Name = L"GroupVariables";
            this->GroupVariables->Size = System::Drawing::Size(434, 159);
            // 
            // 
            // 
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
            // 
            // 
            // 
            this->GroupVariables->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            // 
            // 
            // 
            this->GroupVariables->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->GroupVariables->TabIndex = 6;
            this->GroupVariables->Text = L"Variables";
            // 
            // VariablesList
            // 
            this->VariablesList->AllColumns->Add(this->VariablesListColumnName);
            this->VariablesList->AllColumns->Add(this->VariablesListColumnType);
            this->VariablesList->CellEditUseWholeCell = false;
            this->VariablesList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {
                this->VariablesListColumnName,
                    this->VariablesListColumnType
            });
            this->VariablesList->Cursor = System::Windows::Forms::Cursors::Default;
            this->VariablesList->Dock = System::Windows::Forms::DockStyle::Fill;
            this->VariablesList->GridLines = true;
            this->VariablesList->HideSelection = false;
            this->VariablesList->Location = System::Drawing::Point(0, 29);
            this->VariablesList->MultiSelect = false;
            this->VariablesList->Name = L"VariablesList";
            this->VariablesList->Size = System::Drawing::Size(428, 107);
            this->VariablesList->TabIndex = 1;
            this->VariablesList->UseCompatibleStateImageBehavior = false;
            this->VariablesList->View = System::Windows::Forms::View::Details;
            // 
            // VariablesListColumnName
            // 
            this->VariablesListColumnName->Text = L"Name";
            this->VariablesListColumnName->Width = 217;
            // 
            // VariablesListColumnType
            // 
            this->VariablesListColumnType->Text = L"Data Type";
            this->VariablesListColumnType->Width = 93;
            // 
            // VariablesToolbar
            // 
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
            // 
            // VariablesToolbarAddVar
            // 
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
            // 
            // AddVarMenuInteger
            // 
            this->AddVarMenuInteger->Name = L"AddVarMenuInteger";
            this->AddVarMenuInteger->Text = L"Integer";
            // 
            // AddVarMenuFloat
            // 
            this->AddVarMenuFloat->Name = L"AddVarMenuFloat";
            this->AddVarMenuFloat->Text = L"Float";
            // 
            // AddVarMenuRef
            // 
            this->AddVarMenuRef->Name = L"AddVarMenuRef";
            this->AddVarMenuRef->Text = L"Reference";
            // 
            // AddVarMenuString
            // 
            this->AddVarMenuString->Name = L"AddVarMenuString";
            this->AddVarMenuString->Text = L"String";
            // 
            // AddVarMenuArray
            // 
            this->AddVarMenuArray->Name = L"AddVarMenuArray";
            this->AddVarMenuArray->Text = L"Array";
            // 
            // VariablesToolbarRemoveVar
            // 
            this->VariablesToolbarRemoveVar->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"VariablesToolbarRemoveVar.Image")));
            this->VariablesToolbarRemoveVar->ImagePaddingHorizontal = 10;
            this->VariablesToolbarRemoveVar->ImagePaddingVertical = 10;
            this->VariablesToolbarRemoveVar->Name = L"VariablesToolbarRemoveVar";
            this->VariablesToolbarRemoveVar->Text = L"Remove Variable";
            this->VariablesToolbarRemoveVar->Tooltip = L"Remove Variable";
            // 
            // TextBoxShortcut
            // 
            this->TextBoxShortcut->BackColor = System::Drawing::Color::Black;
            // 
            // 
            // 
            this->TextBoxShortcut->Border->Class = L"TextBoxBorder";
            this->TextBoxShortcut->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->TextBoxShortcut->DisabledBackColor = System::Drawing::Color::Black;
            this->TextBoxShortcut->ForeColor = System::Drawing::Color::White;
            this->TextBoxShortcut->Location = System::Drawing::Point(80, 50);
            this->TextBoxShortcut->Name = L"TextBoxShortcut";
            this->TextBoxShortcut->PreventEnterBeep = true;
            this->TextBoxShortcut->Size = System::Drawing::Size(366, 22);
            this->TextBoxShortcut->TabIndex = 5;
            // 
            // TextBoxDescription
            // 
            this->TextBoxDescription->BackColor = System::Drawing::Color::Black;
            // 
            // 
            // 
            this->TextBoxDescription->Border->Class = L"TextBoxBorder";
            this->TextBoxDescription->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->TextBoxDescription->DisabledBackColor = System::Drawing::Color::Black;
            this->TextBoxDescription->ForeColor = System::Drawing::Color::White;
            this->TextBoxDescription->Location = System::Drawing::Point(80, 87);
            this->TextBoxDescription->Multiline = true;
            this->TextBoxDescription->Name = L"TextBoxDescription";
            this->TextBoxDescription->PreventEnterBeep = true;
            this->TextBoxDescription->Size = System::Drawing::Size(366, 65);
            this->TextBoxDescription->TabIndex = 4;
            // 
            // LabelDescription
            // 
            this->LabelDescription->AutoSize = true;
            // 
            // 
            // 
            this->LabelDescription->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelDescription->Location = System::Drawing::Point(9, 87);
            this->LabelDescription->Name = L"LabelDescription";
            this->LabelDescription->Size = System::Drawing::Size(58, 17);
            this->LabelDescription->TabIndex = 3;
            this->LabelDescription->Text = L"Description";
            // 
            // TextBoxName
            // 
            this->TextBoxName->BackColor = System::Drawing::Color::Black;
            // 
            // 
            // 
            this->TextBoxName->Border->Class = L"TextBoxBorder";
            this->TextBoxName->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->TextBoxName->DisabledBackColor = System::Drawing::Color::Black;
            this->TextBoxName->ForeColor = System::Drawing::Color::White;
            this->TextBoxName->Location = System::Drawing::Point(80, 12);
            this->TextBoxName->Name = L"TextBoxName";
            this->TextBoxName->PreventEnterBeep = true;
            this->TextBoxName->Size = System::Drawing::Size(366, 22);
            this->TextBoxName->TabIndex = 2;
            // 
            // LabelShortcut
            // 
            this->LabelShortcut->AutoSize = true;
            // 
            // 
            // 
            this->LabelShortcut->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelShortcut->Location = System::Drawing::Point(9, 50);
            this->LabelShortcut->Name = L"LabelShortcut";
            this->LabelShortcut->Size = System::Drawing::Size(44, 17);
            this->LabelShortcut->TabIndex = 1;
            this->LabelShortcut->Text = L"Shortcut";
            // 
            // LabelName
            // 
            this->LabelName->AutoSize = true;
            // 
            // 
            // 
            this->LabelName->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelName->Location = System::Drawing::Point(9, 12);
            this->LabelName->Name = L"LabelName";
            this->LabelName->Size = System::Drawing::Size(32, 17);
            this->LabelName->TabIndex = 0;
            this->LabelName->Text = L"Name";
            // 
            // LeftPanel
            // 
            this->LeftPanel->Controls->Add(this->SnippetsList);
            this->LeftPanel->Controls->Add(this->ContainerToolbar);
            this->LeftPanel->Location = System::Drawing::Point(12, 12);
            this->LeftPanel->Name = L"LeftPanel";
            this->LeftPanel->Size = System::Drawing::Size(286, 632);
            this->LeftPanel->TabIndex = 3;
            // 
            // ScriptEditorCodeSnippets
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(776, 656);
            this->Controls->Add(this->LeftPanel);
            this->Controls->Add(this->GroupSnippetData);
            this->DoubleBuffered = true;
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
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
#pragma endregion
	};
}
