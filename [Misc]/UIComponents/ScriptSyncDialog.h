#pragma once

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for ScriptEditorScriptSync
	/// </summary>
	public ref class ScriptSyncDialog : public DevComponents::DotNetBar::Metro::MetroForm
	{
	public:
		ScriptSyncDialog(void)
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
		~ScriptSyncDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: DevComponents::DotNetBar::ButtonX^ ButtonOpenWorkingDir;

	protected:







	private: BrightIdeasSoftware::OLVColumn^ ColLastSyncTime;
	private: BrightIdeasSoftware::OLVColumn^ ColScriptName;
	private: DevComponents::DotNetBar::LabelX^ LabelSelectedScriptLog;
	private: BrightIdeasSoftware::FastObjectListView^ ListViewSyncedScripts;


	private: DevComponents::DotNetBar::ButtonX^ ButtonStartStopSync;









	private: DevComponents::DotNetBar::ButtonX^ ButtonSelectWorkingDir;


	private: System::Windows::Forms::FolderBrowserDialog^ FolderDlgWorkingDir;
	private: DevComponents::DotNetBar::LabelX^ LabelWorkingDir;
	private: DevComponents::DotNetBar::Controls::GroupPanel^ GroupSyncSettings;
	private: DevComponents::DotNetBar::Controls::GroupPanel^ GroupStartupFileHandling;
	private: System::Windows::Forms::RadioButton^ RadioPromptForFileHandling;
	private: System::Windows::Forms::RadioButton^ RadioUseExistingFiles;
	private: System::Windows::Forms::RadioButton^ RadioOverwriteExistingFiles;
	private: System::Windows::Forms::CheckBox^ CheckboxAutoDeleteLogs;
	private: DevComponents::DotNetBar::LabelX^ LabelSeconds;


	private: System::Windows::Forms::CheckBox^ CheckboxAutoSync;
	private: DevComponents::Editors::IntegerInput^ NumericAutoSyncSeconds;
	private: DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxSelectedScriptLog;
	private: DevComponents::DotNetBar::Controls::TextBoxX^ TextBoxWorkingDir;
	private: DevComponents::DotNetBar::PanelEx^ LeftPanel;
	private: DevComponents::DotNetBar::Bar^ Toolbar;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarSyncToDisk;
	private: DevComponents::DotNetBar::LabelItem^ ToolbarLabelSyncedScripts;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarSelectScripts;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarSyncFromDisk;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarOpenLog;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarOpenSyncedFile;
	private: System::Windows::Forms::Timer^ DeferredSelectionUpdateTimer;
    private: DevComponents::DotNetBar::StyleManagerAmbient^ ColorManager;

	private: System::ComponentModel::IContainer^ components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(ScriptSyncDialog::typeid));
            this->ButtonOpenWorkingDir = (gcnew DevComponents::DotNetBar::ButtonX());
            this->ColLastSyncTime = (gcnew BrightIdeasSoftware::OLVColumn());
            this->ColScriptName = (gcnew BrightIdeasSoftware::OLVColumn());
            this->LabelSelectedScriptLog = (gcnew DevComponents::DotNetBar::LabelX());
            this->ListViewSyncedScripts = (gcnew BrightIdeasSoftware::FastObjectListView());
            this->ButtonStartStopSync = (gcnew DevComponents::DotNetBar::ButtonX());
            this->ButtonSelectWorkingDir = (gcnew DevComponents::DotNetBar::ButtonX());
            this->FolderDlgWorkingDir = (gcnew System::Windows::Forms::FolderBrowserDialog());
            this->LabelWorkingDir = (gcnew DevComponents::DotNetBar::LabelX());
            this->GroupSyncSettings = (gcnew DevComponents::DotNetBar::Controls::GroupPanel());
            this->NumericAutoSyncSeconds = (gcnew DevComponents::Editors::IntegerInput());
            this->GroupStartupFileHandling = (gcnew DevComponents::DotNetBar::Controls::GroupPanel());
            this->RadioPromptForFileHandling = (gcnew System::Windows::Forms::RadioButton());
            this->RadioUseExistingFiles = (gcnew System::Windows::Forms::RadioButton());
            this->RadioOverwriteExistingFiles = (gcnew System::Windows::Forms::RadioButton());
            this->CheckboxAutoDeleteLogs = (gcnew System::Windows::Forms::CheckBox());
            this->LabelSeconds = (gcnew DevComponents::DotNetBar::LabelX());
            this->CheckboxAutoSync = (gcnew System::Windows::Forms::CheckBox());
            this->TextBoxSelectedScriptLog = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
            this->TextBoxWorkingDir = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
            this->LeftPanel = (gcnew DevComponents::DotNetBar::PanelEx());
            this->Toolbar = (gcnew DevComponents::DotNetBar::Bar());
            this->ToolbarLabelSyncedScripts = (gcnew DevComponents::DotNetBar::LabelItem());
            this->ToolbarSelectScripts = (gcnew DevComponents::DotNetBar::ButtonItem());
            this->ToolbarSyncToDisk = (gcnew DevComponents::DotNetBar::ButtonItem());
            this->ToolbarSyncFromDisk = (gcnew DevComponents::DotNetBar::ButtonItem());
            this->ToolbarOpenLog = (gcnew DevComponents::DotNetBar::ButtonItem());
            this->ToolbarOpenSyncedFile = (gcnew DevComponents::DotNetBar::ButtonItem());
            this->DeferredSelectionUpdateTimer = (gcnew System::Windows::Forms::Timer(this->components));
            this->ColorManager = (gcnew DevComponents::DotNetBar::StyleManagerAmbient(this->components));
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->BeginInit();
            this->GroupSyncSettings->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->BeginInit();
            this->GroupStartupFileHandling->SuspendLayout();
            this->LeftPanel->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Toolbar))->BeginInit();
            this->SuspendLayout();
            // 
            // ButtonOpenWorkingDir
            // 
            this->ButtonOpenWorkingDir->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
            this->ButtonOpenWorkingDir->Location = System::Drawing::Point(791, 6);
            this->ButtonOpenWorkingDir->Name = L"ButtonOpenWorkingDir";
            this->ButtonOpenWorkingDir->Size = System::Drawing::Size(45, 21);
            this->ButtonOpenWorkingDir->TabIndex = 24;
            this->ButtonOpenWorkingDir->Text = L"Open";
            // 
            // ColLastSyncTime
            // 
            this->ColLastSyncTime->Sortable = false;
            this->ColLastSyncTime->Text = L"Last Sync Attempt Time";
            this->ColLastSyncTime->Width = 152;
            // 
            // ColScriptName
            // 
            this->ColScriptName->Sortable = false;
            this->ColScriptName->Text = L"EditorID";
            this->ColScriptName->Width = 195;
            // 
            // LabelSelectedScriptLog
            // 
            this->LabelSelectedScriptLog->AutoSize = true;
            this->LabelSelectedScriptLog->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            // 
            // 
            // 
            this->LabelSelectedScriptLog->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelSelectedScriptLog->ForeColor = System::Drawing::Color::Black;
            this->LabelSelectedScriptLog->Location = System::Drawing::Point(435, 36);
            this->LabelSelectedScriptLog->Name = L"LabelSelectedScriptLog";
            this->LabelSelectedScriptLog->Size = System::Drawing::Size(21, 17);
            this->LabelSelectedScriptLog->TabIndex = 23;
            this->LabelSelectedScriptLog->Text = L"Log";
            // 
            // ListViewSyncedScripts
            // 
            this->ListViewSyncedScripts->AllColumns->Add(this->ColScriptName);
            this->ListViewSyncedScripts->AllColumns->Add(this->ColLastSyncTime);
            this->ListViewSyncedScripts->CellEditUseWholeCell = false;
            this->ListViewSyncedScripts->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {
                this->ColScriptName,
                    this->ColLastSyncTime
            });
            this->ListViewSyncedScripts->Cursor = System::Windows::Forms::Cursors::Default;
            this->ListViewSyncedScripts->Dock = System::Windows::Forms::DockStyle::Fill;
            this->ListViewSyncedScripts->EmptyListMsg = L"Doesn\'t look like anything to me...";
            this->ListViewSyncedScripts->EmptyListMsgFont = (gcnew System::Drawing::Font(L"Segoe UI caps", 9.75F, System::Drawing::FontStyle::Regular,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->ColorManager->SetEnableAmbientSettings(this->ListViewSyncedScripts, DevComponents::DotNetBar::eAmbientSettings::ChildControls);
            this->ListViewSyncedScripts->FullRowSelect = true;
            this->ListViewSyncedScripts->HideSelection = false;
            this->ListViewSyncedScripts->Location = System::Drawing::Point(0, 37);
            this->ListViewSyncedScripts->Name = L"ListViewSyncedScripts";
            this->ListViewSyncedScripts->ShowGroups = false;
            this->ListViewSyncedScripts->Size = System::Drawing::Size(400, 323);
            this->ListViewSyncedScripts->TabIndex = 21;
            this->ListViewSyncedScripts->UseCompatibleStateImageBehavior = false;
            this->ListViewSyncedScripts->View = System::Windows::Forms::View::Details;
            this->ListViewSyncedScripts->VirtualMode = true;
            // 
            // ButtonStartStopSync
            // 
            this->ButtonStartStopSync->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
            this->ButtonStartStopSync->Location = System::Drawing::Point(12, 374);
            this->ButtonStartStopSync->Name = L"ButtonStartStopSync";
            this->ButtonStartStopSync->Size = System::Drawing::Size(400, 32);
            this->ButtonStartStopSync->TabIndex = 18;
            this->ButtonStartStopSync->Text = L"Start Syncing";
            // 
            // ButtonSelectWorkingDir
            // 
            this->ButtonSelectWorkingDir->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
            this->ButtonSelectWorkingDir->Location = System::Drawing::Point(718, 7);
            this->ButtonSelectWorkingDir->Name = L"ButtonSelectWorkingDir";
            this->ButtonSelectWorkingDir->Size = System::Drawing::Size(67, 20);
            this->ButtonSelectWorkingDir->TabIndex = 16;
            this->ButtonSelectWorkingDir->Text = L"Browse...";
            // 
            // LabelWorkingDir
            // 
            this->LabelWorkingDir->AutoSize = true;
            this->LabelWorkingDir->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(242)));
            // 
            // 
            // 
            this->LabelWorkingDir->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelWorkingDir->ForeColor = System::Drawing::Color::Black;
            this->LabelWorkingDir->Location = System::Drawing::Point(435, 7);
            this->LabelWorkingDir->Name = L"LabelWorkingDir";
            this->LabelWorkingDir->Size = System::Drawing::Size(91, 17);
            this->LabelWorkingDir->TabIndex = 14;
            this->LabelWorkingDir->Text = L"Working Directory";
            // 
            // GroupSyncSettings
            // 
            this->GroupSyncSettings->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->GroupSyncSettings->CanvasColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->GroupSyncSettings->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
            this->GroupSyncSettings->Controls->Add(this->NumericAutoSyncSeconds);
            this->GroupSyncSettings->Controls->Add(this->GroupStartupFileHandling);
            this->GroupSyncSettings->Controls->Add(this->CheckboxAutoDeleteLogs);
            this->GroupSyncSettings->Controls->Add(this->LabelSeconds);
            this->GroupSyncSettings->Controls->Add(this->CheckboxAutoSync);
            this->GroupSyncSettings->DisabledBackColor = System::Drawing::Color::Empty;
            this->GroupSyncSettings->Location = System::Drawing::Point(435, 243);
            this->GroupSyncSettings->Name = L"GroupSyncSettings";
            this->GroupSyncSettings->Size = System::Drawing::Size(401, 163);
            // 
            // 
            // 
            this->GroupSyncSettings->Style->BackColor2SchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground2;
            this->GroupSyncSettings->Style->BackColorGradientAngle = 90;
            this->GroupSyncSettings->Style->BackColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
            this->GroupSyncSettings->Style->BorderBottom = DevComponents::DotNetBar::eStyleBorderType::Solid;
            this->GroupSyncSettings->Style->BorderBottomWidth = 1;
            this->GroupSyncSettings->Style->BorderColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
            this->GroupSyncSettings->Style->BorderLeft = DevComponents::DotNetBar::eStyleBorderType::Solid;
            this->GroupSyncSettings->Style->BorderLeftWidth = 1;
            this->GroupSyncSettings->Style->BorderRight = DevComponents::DotNetBar::eStyleBorderType::Solid;
            this->GroupSyncSettings->Style->BorderRightWidth = 1;
            this->GroupSyncSettings->Style->BorderTop = DevComponents::DotNetBar::eStyleBorderType::Solid;
            this->GroupSyncSettings->Style->BorderTopWidth = 1;
            this->GroupSyncSettings->Style->CornerDiameter = 4;
            this->GroupSyncSettings->Style->CornerType = DevComponents::DotNetBar::eCornerType::Rounded;
            this->GroupSyncSettings->Style->TextAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Center;
            this->GroupSyncSettings->Style->TextColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
            this->GroupSyncSettings->Style->TextLineAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Near;
            // 
            // 
            // 
            this->GroupSyncSettings->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            // 
            // 
            // 
            this->GroupSyncSettings->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->GroupSyncSettings->TabIndex = 26;
            this->GroupSyncSettings->Text = L"Sync Settings";
            // 
            // NumericAutoSyncSeconds
            // 
            this->NumericAutoSyncSeconds->BackColor = System::Drawing::Color::White;
            // 
            // 
            // 
            this->NumericAutoSyncSeconds->BackgroundStyle->Class = L"DateTimeInputBackground";
            this->NumericAutoSyncSeconds->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->NumericAutoSyncSeconds->ForeColor = System::Drawing::Color::Black;
            this->NumericAutoSyncSeconds->Location = System::Drawing::Point(214, 3);
            this->NumericAutoSyncSeconds->MaxValue = 60;
            this->NumericAutoSyncSeconds->MinValue = 1;
            this->NumericAutoSyncSeconds->Name = L"NumericAutoSyncSeconds";
            this->NumericAutoSyncSeconds->ShowUpDown = true;
            this->NumericAutoSyncSeconds->Size = System::Drawing::Size(63, 22);
            this->NumericAutoSyncSeconds->TabIndex = 27;
            this->NumericAutoSyncSeconds->Value = 1;
            // 
            // GroupStartupFileHandling
            // 
            this->GroupStartupFileHandling->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->GroupStartupFileHandling->CanvasColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->GroupStartupFileHandling->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
            this->GroupStartupFileHandling->Controls->Add(this->RadioPromptForFileHandling);
            this->GroupStartupFileHandling->Controls->Add(this->RadioUseExistingFiles);
            this->GroupStartupFileHandling->Controls->Add(this->RadioOverwriteExistingFiles);
            this->GroupStartupFileHandling->DisabledBackColor = System::Drawing::Color::Empty;
            this->GroupStartupFileHandling->Location = System::Drawing::Point(3, 32);
            this->GroupStartupFileHandling->Name = L"GroupStartupFileHandling";
            this->GroupStartupFileHandling->Size = System::Drawing::Size(389, 71);
            // 
            // 
            // 
            this->GroupStartupFileHandling->Style->BackColor2SchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground2;
            this->GroupStartupFileHandling->Style->BackColorGradientAngle = 90;
            this->GroupStartupFileHandling->Style->BackColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
            this->GroupStartupFileHandling->Style->BorderBottom = DevComponents::DotNetBar::eStyleBorderType::Solid;
            this->GroupStartupFileHandling->Style->BorderBottomWidth = 1;
            this->GroupStartupFileHandling->Style->BorderColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
            this->GroupStartupFileHandling->Style->BorderLeft = DevComponents::DotNetBar::eStyleBorderType::Solid;
            this->GroupStartupFileHandling->Style->BorderLeftWidth = 1;
            this->GroupStartupFileHandling->Style->BorderRight = DevComponents::DotNetBar::eStyleBorderType::Solid;
            this->GroupStartupFileHandling->Style->BorderRightWidth = 1;
            this->GroupStartupFileHandling->Style->BorderTop = DevComponents::DotNetBar::eStyleBorderType::Solid;
            this->GroupStartupFileHandling->Style->BorderTopWidth = 1;
            this->GroupStartupFileHandling->Style->CornerDiameter = 4;
            this->GroupStartupFileHandling->Style->CornerType = DevComponents::DotNetBar::eCornerType::Rounded;
            this->GroupStartupFileHandling->Style->TextAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Center;
            this->GroupStartupFileHandling->Style->TextColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
            this->GroupStartupFileHandling->Style->TextLineAlignment = DevComponents::DotNetBar::eStyleTextAlignment::Near;
            // 
            // 
            // 
            this->GroupStartupFileHandling->StyleMouseDown->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            // 
            // 
            // 
            this->GroupStartupFileHandling->StyleMouseOver->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->GroupStartupFileHandling->TabIndex = 27;
            this->GroupStartupFileHandling->Text = L"Existing File Handling During Startup";
            // 
            // RadioPromptForFileHandling
            // 
            this->RadioPromptForFileHandling->AutoSize = true;
            this->RadioPromptForFileHandling->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->RadioPromptForFileHandling->ForeColor = System::Drawing::Color::Black;
            this->RadioPromptForFileHandling->Location = System::Drawing::Point(14, 26);
            this->RadioPromptForFileHandling->Name = L"RadioPromptForFileHandling";
            this->RadioPromptForFileHandling->Size = System::Drawing::Size(126, 17);
            this->RadioPromptForFileHandling->TabIndex = 5;
            this->RadioPromptForFileHandling->TabStop = true;
            this->RadioPromptForFileHandling->Text = L"Prompt for each file";
            this->RadioPromptForFileHandling->UseVisualStyleBackColor = false;
            // 
            // RadioUseExistingFiles
            // 
            this->RadioUseExistingFiles->AutoSize = true;
            this->RadioUseExistingFiles->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->RadioUseExistingFiles->ForeColor = System::Drawing::Color::Black;
            this->RadioUseExistingFiles->Location = System::Drawing::Point(219, 3);
            this->RadioUseExistingFiles->Name = L"RadioUseExistingFiles";
            this->RadioUseExistingFiles->Size = System::Drawing::Size(161, 17);
            this->RadioUseExistingFiles->TabIndex = 4;
            this->RadioUseExistingFiles->TabStop = true;
            this->RadioUseExistingFiles->Text = L"Use without modifications";
            this->RadioUseExistingFiles->UseVisualStyleBackColor = false;
            // 
            // RadioOverwriteExistingFiles
            // 
            this->RadioOverwriteExistingFiles->AutoSize = true;
            this->RadioOverwriteExistingFiles->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->RadioOverwriteExistingFiles->ForeColor = System::Drawing::Color::Black;
            this->RadioOverwriteExistingFiles->Location = System::Drawing::Point(14, 3);
            this->RadioOverwriteExistingFiles->Name = L"RadioOverwriteExistingFiles";
            this->RadioOverwriteExistingFiles->Size = System::Drawing::Size(191, 17);
            this->RadioOverwriteExistingFiles->TabIndex = 3;
            this->RadioOverwriteExistingFiles->TabStop = true;
            this->RadioOverwriteExistingFiles->Text = L"Overwrite with plugin script text";
            this->RadioOverwriteExistingFiles->UseVisualStyleBackColor = false;
            // 
            // CheckboxAutoDeleteLogs
            // 
            this->CheckboxAutoDeleteLogs->AutoSize = true;
            this->CheckboxAutoDeleteLogs->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->CheckboxAutoDeleteLogs->ForeColor = System::Drawing::Color::Black;
            this->CheckboxAutoDeleteLogs->Location = System::Drawing::Point(16, 114);
            this->CheckboxAutoDeleteLogs->Name = L"CheckboxAutoDeleteLogs";
            this->CheckboxAutoDeleteLogs->Size = System::Drawing::Size(276, 17);
            this->CheckboxAutoDeleteLogs->TabIndex = 9;
            this->CheckboxAutoDeleteLogs->Text = L"Automatically delete log files when syncing ends";
            this->CheckboxAutoDeleteLogs->UseVisualStyleBackColor = false;
            // 
            // LabelSeconds
            // 
            this->LabelSeconds->AutoSize = true;
            this->LabelSeconds->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(242)));
            // 
            // 
            // 
            this->LabelSeconds->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelSeconds->ForeColor = System::Drawing::Color::Black;
            this->LabelSeconds->Location = System::Drawing::Point(283, 5);
            this->LabelSeconds->Name = L"LabelSeconds";
            this->LabelSeconds->Size = System::Drawing::Size(42, 17);
            this->LabelSeconds->TabIndex = 7;
            this->LabelSeconds->Text = L"seconds";
            // 
            // CheckboxAutoSync
            // 
            this->CheckboxAutoSync->AutoSize = true;
            this->CheckboxAutoSync->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(239)),
                static_cast<System::Int32>(static_cast<System::Byte>(239)), static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->CheckboxAutoSync->ForeColor = System::Drawing::Color::Black;
            this->CheckboxAutoSync->Location = System::Drawing::Point(16, 5);
            this->CheckboxAutoSync->Name = L"CheckboxAutoSync";
            this->CheckboxAutoSync->Size = System::Drawing::Size(195, 17);
            this->CheckboxAutoSync->TabIndex = 5;
            this->CheckboxAutoSync->Text = L"Automatically sync changes every";
            this->CheckboxAutoSync->UseVisualStyleBackColor = false;
            // 
            // TextBoxSelectedScriptLog
            // 
            this->TextBoxSelectedScriptLog->BackColor = System::Drawing::Color::White;
            // 
            // 
            // 
            this->TextBoxSelectedScriptLog->Border->Class = L"TextBoxBorder";
            this->TextBoxSelectedScriptLog->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->TextBoxSelectedScriptLog->DisabledBackColor = System::Drawing::Color::Black;
            this->TextBoxSelectedScriptLog->ForeColor = System::Drawing::Color::Black;
            this->TextBoxSelectedScriptLog->Location = System::Drawing::Point(435, 59);
            this->TextBoxSelectedScriptLog->Multiline = true;
            this->TextBoxSelectedScriptLog->Name = L"TextBoxSelectedScriptLog";
            this->TextBoxSelectedScriptLog->PreventEnterBeep = true;
            this->TextBoxSelectedScriptLog->ReadOnly = true;
            this->TextBoxSelectedScriptLog->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
            this->TextBoxSelectedScriptLog->Size = System::Drawing::Size(401, 178);
            this->TextBoxSelectedScriptLog->TabIndex = 27;
            // 
            // TextBoxWorkingDir
            // 
            this->TextBoxWorkingDir->BackColor = System::Drawing::Color::White;
            // 
            // 
            // 
            this->TextBoxWorkingDir->Border->Class = L"TextBoxBorder";
            this->TextBoxWorkingDir->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->TextBoxWorkingDir->DisabledBackColor = System::Drawing::Color::White;
            this->TextBoxWorkingDir->ForeColor = System::Drawing::Color::Black;
            this->TextBoxWorkingDir->Location = System::Drawing::Point(532, 6);
            this->TextBoxWorkingDir->Name = L"TextBoxWorkingDir";
            this->TextBoxWorkingDir->PreventEnterBeep = true;
            this->TextBoxWorkingDir->Size = System::Drawing::Size(180, 22);
            this->TextBoxWorkingDir->TabIndex = 28;
            // 
            // LeftPanel
            // 
            this->LeftPanel->CanvasColor = System::Drawing::SystemColors::Control;
            this->LeftPanel->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
            this->LeftPanel->Controls->Add(this->ListViewSyncedScripts);
            this->LeftPanel->Controls->Add(this->Toolbar);
            this->LeftPanel->DisabledBackColor = System::Drawing::Color::Empty;
            this->LeftPanel->Location = System::Drawing::Point(12, 6);
            this->LeftPanel->Name = L"LeftPanel";
            this->LeftPanel->Size = System::Drawing::Size(400, 360);
            this->LeftPanel->Style->Alignment = System::Drawing::StringAlignment::Center;
            this->LeftPanel->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
            this->LeftPanel->Style->Border = DevComponents::DotNetBar::eBorderType::SingleLine;
            this->LeftPanel->Style->BorderColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
            this->LeftPanel->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
            this->LeftPanel->Style->GradientAngle = 90;
            this->LeftPanel->TabIndex = 29;
            // 
            // Toolbar
            // 
            this->Toolbar->AntiAlias = true;
            this->Toolbar->Dock = System::Windows::Forms::DockStyle::Top;
            this->Toolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
            this->Toolbar->IsMaximized = false;
            this->Toolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(6) {
                this->ToolbarLabelSyncedScripts,
                    this->ToolbarSelectScripts, this->ToolbarSyncToDisk, this->ToolbarSyncFromDisk, this->ToolbarOpenLog, this->ToolbarOpenSyncedFile
            });
            this->Toolbar->ItemSpacing = 5;
            this->Toolbar->Location = System::Drawing::Point(0, 0);
            this->Toolbar->Name = L"Toolbar";
            this->Toolbar->PaddingBottom = 5;
            this->Toolbar->PaddingLeft = 5;
            this->Toolbar->PaddingRight = 5;
            this->Toolbar->PaddingTop = 5;
            this->Toolbar->Size = System::Drawing::Size(400, 37);
            this->Toolbar->Stretch = true;
            this->Toolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
            this->Toolbar->TabIndex = 22;
            this->Toolbar->TabStop = false;
            this->Toolbar->Text = L"bar1";
            // 
            // ToolbarLabelSyncedScripts
            // 
            this->ToolbarLabelSyncedScripts->Name = L"ToolbarLabelSyncedScripts";
            this->ToolbarLabelSyncedScripts->Text = L"Synced Scripts";
            // 
            // ToolbarSelectScripts
            // 
            this->ToolbarSelectScripts->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarSelectScripts.Image")));
            this->ToolbarSelectScripts->ImagePaddingHorizontal = 10;
            this->ToolbarSelectScripts->ImagePaddingVertical = 10;
            this->ToolbarSelectScripts->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
            this->ToolbarSelectScripts->Name = L"ToolbarSelectScripts";
            this->ToolbarSelectScripts->Text = L"Select Scripts...";
            this->ToolbarSelectScripts->Tooltip = L"Select Scripts...";
            // 
            // ToolbarSyncToDisk
            // 
            this->ToolbarSyncToDisk->BeginGroup = true;
            this->ToolbarSyncToDisk->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarSyncToDisk.Image")));
            this->ToolbarSyncToDisk->ImagePaddingHorizontal = 10;
            this->ToolbarSyncToDisk->ImagePaddingVertical = 10;
            this->ToolbarSyncToDisk->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
            this->ToolbarSyncToDisk->Name = L"ToolbarSyncToDisk";
            this->ToolbarSyncToDisk->Text = L"Sync to Disk";
            this->ToolbarSyncToDisk->Tooltip = L"Sync to Disk";
            // 
            // ToolbarSyncFromDisk
            // 
            this->ToolbarSyncFromDisk->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarSyncFromDisk.Image")));
            this->ToolbarSyncFromDisk->ImagePaddingHorizontal = 10;
            this->ToolbarSyncFromDisk->ImagePaddingVertical = 10;
            this->ToolbarSyncFromDisk->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
            this->ToolbarSyncFromDisk->Name = L"ToolbarSyncFromDisk";
            this->ToolbarSyncFromDisk->Text = L"Sync from Disk";
            this->ToolbarSyncFromDisk->Tooltip = L"Sync from Disk";
            // 
            // ToolbarOpenLog
            // 
            this->ToolbarOpenLog->BeginGroup = true;
            this->ToolbarOpenLog->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarOpenLog.Image")));
            this->ToolbarOpenLog->ImagePaddingHorizontal = 10;
            this->ToolbarOpenLog->ImagePaddingVertical = 10;
            this->ToolbarOpenLog->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
            this->ToolbarOpenLog->Name = L"ToolbarOpenLog";
            this->ToolbarOpenLog->Text = L"Open Log";
            this->ToolbarOpenLog->Tooltip = L"Open Log";
            // 
            // ToolbarOpenSyncedFile
            // 
            this->ToolbarOpenSyncedFile->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarOpenSyncedFile.Image")));
            this->ToolbarOpenSyncedFile->ImagePaddingHorizontal = 10;
            this->ToolbarOpenSyncedFile->ImagePaddingVertical = 10;
            this->ToolbarOpenSyncedFile->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
            this->ToolbarOpenSyncedFile->Name = L"ToolbarOpenSyncedFile";
            this->ToolbarOpenSyncedFile->Text = L"Open Synced File";
            this->ToolbarOpenSyncedFile->Tooltip = L"Open Synced File";
            // 
            // ScriptSyncDialog
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(852, 413);
            this->Controls->Add(this->LeftPanel);
            this->Controls->Add(this->TextBoxWorkingDir);
            this->Controls->Add(this->TextBoxSelectedScriptLog);
            this->Controls->Add(this->GroupSyncSettings);
            this->Controls->Add(this->ButtonOpenWorkingDir);
            this->Controls->Add(this->LabelSelectedScriptLog);
            this->Controls->Add(this->ButtonStartStopSync);
            this->Controls->Add(this->ButtonSelectWorkingDir);
            this->Controls->Add(this->LabelWorkingDir);
            this->DoubleBuffered = true;
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
            this->MaximizeBox = false;
            this->Name = L"ScriptSyncDialog";
            this->Text = L"Sync Scripts To Disk";
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->EndInit();
            this->GroupSyncSettings->ResumeLayout(false);
            this->GroupSyncSettings->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->EndInit();
            this->GroupStartupFileHandling->ResumeLayout(false);
            this->GroupStartupFileHandling->PerformLayout();
            this->LeftPanel->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Toolbar))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
	private: System::Void labelItem1_Click(System::Object^ sender, System::EventArgs^ e) {
	}
};
}
