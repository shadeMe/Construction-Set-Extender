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
	public ref class ScriptEditorScriptSync : public DevComponents::DotNetBar::Metro::MetroForm
	{
	public:
		ScriptEditorScriptSync(void)
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
		~ScriptEditorScriptSync()
		{
			if (components)
			{
				delete components;
			}
		}
	private: DevComponents::DotNetBar::ButtonX^ ButtonOpenWorkingDir;

	protected:

	private: System::Windows::Forms::ToolStripMenuItem^ OpenSyncedFileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ OpenLogToolStripMenuItem;
	private: System::Windows::Forms::ToolStripSeparator^ ToolStripSeparator1;
	private: System::Windows::Forms::ToolStripMenuItem^ SyncFromDiskToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ SyncToDiskToolStripMenuItem;
	private: System::Windows::Forms::ContextMenuStrip^ LVSyncedStripsContextMenu;
	private: BrightIdeasSoftware::OLVColumn^ ColLastSyncTime;
	private: BrightIdeasSoftware::OLVColumn^ ColScriptName;
	private: DevComponents::DotNetBar::LabelX^ LabelSelectedScriptLog;
	private: BrightIdeasSoftware::FastObjectListView^ ListViewSyncedScripts;
	private: DevComponents::DotNetBar::ButtonX^ ButtonSelectScripts;
	private: DevComponents::DotNetBar::LabelX^ LabelScriptsToSync;
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
            this->ButtonOpenWorkingDir = (gcnew DevComponents::DotNetBar::ButtonX());
            this->OpenSyncedFileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->OpenLogToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->ToolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->SyncFromDiskToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->SyncToDiskToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->LVSyncedStripsContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->ColLastSyncTime = (gcnew BrightIdeasSoftware::OLVColumn());
            this->ColScriptName = (gcnew BrightIdeasSoftware::OLVColumn());
            this->LabelSelectedScriptLog = (gcnew DevComponents::DotNetBar::LabelX());
            this->ListViewSyncedScripts = (gcnew BrightIdeasSoftware::FastObjectListView());
            this->ButtonSelectScripts = (gcnew DevComponents::DotNetBar::ButtonX());
            this->LabelScriptsToSync = (gcnew DevComponents::DotNetBar::LabelX());
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
            this->LVSyncedStripsContextMenu->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->BeginInit();
            this->GroupSyncSettings->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->BeginInit();
            this->GroupStartupFileHandling->SuspendLayout();
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
            // OpenSyncedFileToolStripMenuItem
            // 
            this->OpenSyncedFileToolStripMenuItem->Name = L"OpenSyncedFileToolStripMenuItem";
            this->OpenSyncedFileToolStripMenuItem->Size = System::Drawing::Size(140, 22);
            this->OpenSyncedFileToolStripMenuItem->Text = L"Open Synced File";
            // 
            // OpenLogToolStripMenuItem
            // 
            this->OpenLogToolStripMenuItem->Name = L"OpenLogToolStripMenuItem";
            this->OpenLogToolStripMenuItem->Size = System::Drawing::Size(140, 22);
            this->OpenLogToolStripMenuItem->Text = L"Open Log";
            // 
            // ToolStripSeparator1
            // 
            this->ToolStripSeparator1->Name = L"ToolStripSeparator1";
            this->ToolStripSeparator1->Size = System::Drawing::Size(137, 6);
            // 
            // SyncFromDiskToolStripMenuItem
            // 
            this->SyncFromDiskToolStripMenuItem->Name = L"SyncFromDiskToolStripMenuItem";
            this->SyncFromDiskToolStripMenuItem->Size = System::Drawing::Size(140, 22);
            this->SyncFromDiskToolStripMenuItem->Text = L"Sync From Disk";
            // 
            // SyncToDiskToolStripMenuItem
            // 
            this->SyncToDiskToolStripMenuItem->Name = L"SyncToDiskToolStripMenuItem";
            this->SyncToDiskToolStripMenuItem->Size = System::Drawing::Size(140, 22);
            this->SyncToDiskToolStripMenuItem->Text = L"Sync To Disk";
            // 
            // LVSyncedStripsContextMenu
            // 
            this->LVSyncedStripsContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {
                this->SyncToDiskToolStripMenuItem,
                    this->SyncFromDiskToolStripMenuItem, this->ToolStripSeparator1, this->OpenLogToolStripMenuItem, this->OpenSyncedFileToolStripMenuItem
            });
            this->LVSyncedStripsContextMenu->Name = L"LVSyncedStripsContextMenu";
            this->LVSyncedStripsContextMenu->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
            this->LVSyncedStripsContextMenu->ShowImageMargin = false;
            this->LVSyncedStripsContextMenu->Size = System::Drawing::Size(141, 98);
            // 
            // ColLastSyncTime
            // 
            this->ColLastSyncTime->Sortable = false;
            this->ColLastSyncTime->Text = L"Last Sync Attempt Time";
            this->ColLastSyncTime->Width = 160;
            // 
            // ColScriptName
            // 
            this->ColScriptName->Sortable = false;
            this->ColScriptName->Text = L"EditorID";
            this->ColScriptName->Width = 211;
            // 
            // LabelSelectedScriptLog
            // 
            this->LabelSelectedScriptLog->AutoSize = true;
            this->LabelSelectedScriptLog->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            // 
            // 
            // 
            this->LabelSelectedScriptLog->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelSelectedScriptLog->ForeColor = System::Drawing::Color::White;
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
            this->ListViewSyncedScripts->BackColor = System::Drawing::Color::Black;
            this->ListViewSyncedScripts->CellEditUseWholeCell = false;
            this->ListViewSyncedScripts->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {
                this->ColScriptName,
                    this->ColLastSyncTime
            });
            this->ListViewSyncedScripts->Cursor = System::Windows::Forms::Cursors::Default;
            this->ListViewSyncedScripts->ForeColor = System::Drawing::Color::White;
            this->ListViewSyncedScripts->FullRowSelect = true;
            this->ListViewSyncedScripts->GridLines = true;
            this->ListViewSyncedScripts->HideSelection = false;
            this->ListViewSyncedScripts->Location = System::Drawing::Point(15, 37);
            this->ListViewSyncedScripts->Name = L"ListViewSyncedScripts";
            this->ListViewSyncedScripts->ShowGroups = false;
            this->ListViewSyncedScripts->Size = System::Drawing::Size(396, 331);
            this->ListViewSyncedScripts->TabIndex = 21;
            this->ListViewSyncedScripts->UseCompatibleStateImageBehavior = false;
            this->ListViewSyncedScripts->View = System::Windows::Forms::View::Details;
            this->ListViewSyncedScripts->VirtualMode = true;
            // 
            // ButtonSelectScripts
            // 
            this->ButtonSelectScripts->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
            this->ButtonSelectScripts->Location = System::Drawing::Point(345, 6);
            this->ButtonSelectScripts->Name = L"ButtonSelectScripts";
            this->ButtonSelectScripts->Size = System::Drawing::Size(67, 21);
            this->ButtonSelectScripts->TabIndex = 20;
            this->ButtonSelectScripts->Text = L"Select...";
            // 
            // LabelScriptsToSync
            // 
            this->LabelScriptsToSync->AutoSize = true;
            this->LabelScriptsToSync->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            // 
            // 
            // 
            this->LabelScriptsToSync->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelScriptsToSync->ForeColor = System::Drawing::Color::White;
            this->LabelScriptsToSync->Location = System::Drawing::Point(12, 10);
            this->LabelScriptsToSync->Name = L"LabelScriptsToSync";
            this->LabelScriptsToSync->Size = System::Drawing::Size(72, 17);
            this->LabelScriptsToSync->TabIndex = 19;
            this->LabelScriptsToSync->Text = L"Synced Scripts";
            // 
            // ButtonStartStopSync
            // 
            this->ButtonStartStopSync->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
            this->ButtonStartStopSync->Location = System::Drawing::Point(15, 374);
            this->ButtonStartStopSync->Name = L"ButtonStartStopSync";
            this->ButtonStartStopSync->Size = System::Drawing::Size(396, 32);
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
            this->LabelWorkingDir->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(48)));
            // 
            // 
            // 
            this->LabelWorkingDir->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelWorkingDir->ForeColor = System::Drawing::Color::White;
            this->LabelWorkingDir->Location = System::Drawing::Point(435, 7);
            this->LabelWorkingDir->Name = L"LabelWorkingDir";
            this->LabelWorkingDir->Size = System::Drawing::Size(91, 17);
            this->LabelWorkingDir->TabIndex = 14;
            this->LabelWorkingDir->Text = L"Working Directory";
            // 
            // GroupSyncSettings
            // 
            this->GroupSyncSettings->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->GroupSyncSettings->CanvasColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->GroupSyncSettings->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::Office2007;
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
            this->NumericAutoSyncSeconds->BackColor = System::Drawing::Color::Black;
            // 
            // 
            // 
            this->NumericAutoSyncSeconds->BackgroundStyle->Class = L"DateTimeInputBackground";
            this->NumericAutoSyncSeconds->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->NumericAutoSyncSeconds->ButtonFreeText->Shortcut = DevComponents::DotNetBar::eShortcut::F2;
            this->NumericAutoSyncSeconds->ForeColor = System::Drawing::Color::White;
            this->NumericAutoSyncSeconds->Location = System::Drawing::Point(208, 3);
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
            this->GroupStartupFileHandling->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->GroupStartupFileHandling->CanvasColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->GroupStartupFileHandling->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::Office2007;
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
            this->RadioPromptForFileHandling->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->RadioPromptForFileHandling->ForeColor = System::Drawing::Color::White;
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
            this->RadioUseExistingFiles->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->RadioUseExistingFiles->ForeColor = System::Drawing::Color::White;
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
            this->RadioOverwriteExistingFiles->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->RadioOverwriteExistingFiles->ForeColor = System::Drawing::Color::White;
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
            this->CheckboxAutoDeleteLogs->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->CheckboxAutoDeleteLogs->ForeColor = System::Drawing::Color::White;
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
            this->LabelSeconds->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(48)));
            // 
            // 
            // 
            this->LabelSeconds->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->LabelSeconds->ForeColor = System::Drawing::Color::White;
            this->LabelSeconds->Location = System::Drawing::Point(277, 5);
            this->LabelSeconds->Name = L"LabelSeconds";
            this->LabelSeconds->Size = System::Drawing::Size(42, 17);
            this->LabelSeconds->TabIndex = 7;
            this->LabelSeconds->Text = L"seconds";
            // 
            // CheckboxAutoSync
            // 
            this->CheckboxAutoSync->AutoSize = true;
            this->CheckboxAutoSync->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
                static_cast<System::Int32>(static_cast<System::Byte>(48)));
            this->CheckboxAutoSync->ForeColor = System::Drawing::Color::White;
            this->CheckboxAutoSync->Location = System::Drawing::Point(16, 5);
            this->CheckboxAutoSync->Name = L"CheckboxAutoSync";
            this->CheckboxAutoSync->Size = System::Drawing::Size(195, 17);
            this->CheckboxAutoSync->TabIndex = 5;
            this->CheckboxAutoSync->Text = L"Automatically sync changes every";
            this->CheckboxAutoSync->UseVisualStyleBackColor = false;
            // 
            // TextBoxSelectedScriptLog
            // 
            this->TextBoxSelectedScriptLog->BackColor = System::Drawing::Color::Black;
            // 
            // 
            // 
            this->TextBoxSelectedScriptLog->Border->Class = L"TextBoxBorder";
            this->TextBoxSelectedScriptLog->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->TextBoxSelectedScriptLog->DisabledBackColor = System::Drawing::Color::Black;
            this->TextBoxSelectedScriptLog->ForeColor = System::Drawing::Color::White;
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
            this->TextBoxWorkingDir->BackColor = System::Drawing::Color::Black;
            // 
            // 
            // 
            this->TextBoxWorkingDir->Border->Class = L"TextBoxBorder";
            this->TextBoxWorkingDir->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
            this->TextBoxWorkingDir->DisabledBackColor = System::Drawing::Color::Black;
            this->TextBoxWorkingDir->ForeColor = System::Drawing::Color::White;
            this->TextBoxWorkingDir->Location = System::Drawing::Point(532, 6);
            this->TextBoxWorkingDir->Name = L"TextBoxWorkingDir";
            this->TextBoxWorkingDir->PreventEnterBeep = true;
            this->TextBoxWorkingDir->Size = System::Drawing::Size(180, 22);
            this->TextBoxWorkingDir->TabIndex = 28;
            // 
            // ScriptEditorScriptSync
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(852, 413);
            this->Controls->Add(this->TextBoxWorkingDir);
            this->Controls->Add(this->TextBoxSelectedScriptLog);
            this->Controls->Add(this->GroupSyncSettings);
            this->Controls->Add(this->ButtonOpenWorkingDir);
            this->Controls->Add(this->LabelSelectedScriptLog);
            this->Controls->Add(this->ListViewSyncedScripts);
            this->Controls->Add(this->ButtonSelectScripts);
            this->Controls->Add(this->LabelScriptsToSync);
            this->Controls->Add(this->ButtonStartStopSync);
            this->Controls->Add(this->ButtonSelectWorkingDir);
            this->Controls->Add(this->LabelWorkingDir);
            this->DoubleBuffered = true;
            this->ForeColor = System::Drawing::Color::White;
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->Name = L"ScriptEditorScriptSync";
            this->Text = L"Sync Scripts To Disk";
            this->LVSyncedStripsContextMenu->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->EndInit();
            this->GroupSyncSettings->ResumeLayout(false);
            this->GroupSyncSettings->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->EndInit();
            this->GroupStartupFileHandling->ResumeLayout(false);
            this->GroupStartupFileHandling->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
	};
}
