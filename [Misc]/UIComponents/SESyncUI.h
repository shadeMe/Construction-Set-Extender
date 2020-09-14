#pragma once

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SESyncUI
	/// </summary>
	public ref class SESyncUI : public System::Windows::Forms::Form
	{
	public:
		SESyncUI(void)
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
		~SESyncUI()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Label^ LabelWorkingDir;
	private: System::Windows::Forms::FolderBrowserDialog^ FolderDlgWorkingDir;
	private: System::Windows::Forms::TextBox^ TextBoxWorkingDir;
	private: System::Windows::Forms::Button^ ButtonSelectWorkingDir;
	private: System::Windows::Forms::GroupBox^ GroupSyncSettings;
	private: System::Windows::Forms::CheckBox^ CheckboxAutoSync;
	private: System::Windows::Forms::Label^ LabelSeconds;






	private: System::Windows::Forms::NumericUpDown^ NumericAutoSyncSeconds;
	private: System::Windows::Forms::GroupBox^ GroupStartupFileHandling;
	private: System::Windows::Forms::RadioButton^ RadioPromptForFileHandling;



	private: System::Windows::Forms::RadioButton^ RadioUseExistingFiles;

	private: System::Windows::Forms::RadioButton^ RadioOverwriteExistingFiles;
	private: System::Windows::Forms::Button^ ButtonStartStopSync;





	private: System::Windows::Forms::Label^ LabelScriptsToSync;
	private: System::Windows::Forms::Button^ ButtonSelectScripts;
	private: System::Windows::Forms::CheckBox^ CheckboxAutoDeleteLogs;
	private: BrightIdeasSoftware::FastObjectListView^ ListViewSyncedScripts;

	private: BrightIdeasSoftware::OLVColumn^ ColScriptName;
	private: BrightIdeasSoftware::OLVColumn^ ColLastSyncTime;
	private: System::Windows::Forms::ContextMenuStrip^ LVSyncedStripsContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^ SyncToDiskToolStripMenuItem;

	private: System::Windows::Forms::ToolStripMenuItem^ SyncFromDiskToolStripMenuItem;
	private: System::Windows::Forms::ToolStripSeparator^ ToolStripSeparator1;
	private: System::Windows::Forms::ToolStripMenuItem^ ShowLogToolStripMenuItem;
	private: System::Windows::Forms::TextBox^ TextBoxSelectedScriptLog;
	private: System::Windows::Forms::Label^ LabelSelectedScriptLog;

	private: System::ComponentModel::IContainer^ components;



	protected:

	protected:

	protected:

	protected:

	protected:

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
			this->LabelWorkingDir = (gcnew System::Windows::Forms::Label());
			this->FolderDlgWorkingDir = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->TextBoxWorkingDir = (gcnew System::Windows::Forms::TextBox());
			this->ButtonSelectWorkingDir = (gcnew System::Windows::Forms::Button());
			this->GroupSyncSettings = (gcnew System::Windows::Forms::GroupBox());
			this->CheckboxAutoDeleteLogs = (gcnew System::Windows::Forms::CheckBox());
			this->GroupStartupFileHandling = (gcnew System::Windows::Forms::GroupBox());
			this->RadioPromptForFileHandling = (gcnew System::Windows::Forms::RadioButton());
			this->RadioUseExistingFiles = (gcnew System::Windows::Forms::RadioButton());
			this->RadioOverwriteExistingFiles = (gcnew System::Windows::Forms::RadioButton());
			this->LabelSeconds = (gcnew System::Windows::Forms::Label());
			this->NumericAutoSyncSeconds = (gcnew System::Windows::Forms::NumericUpDown());
			this->CheckboxAutoSync = (gcnew System::Windows::Forms::CheckBox());
			this->ButtonStartStopSync = (gcnew System::Windows::Forms::Button());
			this->LabelScriptsToSync = (gcnew System::Windows::Forms::Label());
			this->ButtonSelectScripts = (gcnew System::Windows::Forms::Button());
			this->ListViewSyncedScripts = (gcnew BrightIdeasSoftware::FastObjectListView());
			this->ColScriptName = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ColLastSyncTime = (gcnew BrightIdeasSoftware::OLVColumn());
			this->LVSyncedStripsContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->SyncToDiskToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->SyncFromDiskToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ToolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->ShowLogToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->TextBoxSelectedScriptLog = (gcnew System::Windows::Forms::TextBox());
			this->LabelSelectedScriptLog = (gcnew System::Windows::Forms::Label());
			this->GroupSyncSettings->SuspendLayout();
			this->GroupStartupFileHandling->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->BeginInit();
			this->LVSyncedStripsContextMenu->SuspendLayout();
			this->SuspendLayout();
			this->LabelWorkingDir->AutoSize = true;
			this->LabelWorkingDir->Location = System::Drawing::Point(418, 9);
			this->LabelWorkingDir->Name = L"LabelWorkingDir";
			this->LabelWorkingDir->Size = System::Drawing::Size(92, 13);
			this->LabelWorkingDir->TabIndex = 0;
			this->LabelWorkingDir->Text = L"Working Directory";
			this->TextBoxWorkingDir->Location = System::Drawing::Point(516, 6);
			this->TextBoxWorkingDir->Name = L"TextBoxWorkingDir";
			this->TextBoxWorkingDir->ReadOnly = true;
			this->TextBoxWorkingDir->Size = System::Drawing::Size(229, 20);
			this->TextBoxWorkingDir->TabIndex = 1;
			this->ButtonSelectWorkingDir->Location = System::Drawing::Point(752, 6);
			this->ButtonSelectWorkingDir->Name = L"ButtonSelectWorkingDir";
			this->ButtonSelectWorkingDir->Size = System::Drawing::Size(67, 20);
			this->ButtonSelectWorkingDir->TabIndex = 2;
			this->ButtonSelectWorkingDir->Text = L"Browse...";
			this->ButtonSelectWorkingDir->UseVisualStyleBackColor = true;
			this->GroupSyncSettings->Controls->Add(this->CheckboxAutoDeleteLogs);
			this->GroupSyncSettings->Controls->Add(this->GroupStartupFileHandling);
			this->GroupSyncSettings->Controls->Add(this->LabelSeconds);
			this->GroupSyncSettings->Controls->Add(this->NumericAutoSyncSeconds);
			this->GroupSyncSettings->Controls->Add(this->CheckboxAutoSync);
			this->GroupSyncSettings->Location = System::Drawing::Point(418, 242);
			this->GroupSyncSettings->Name = L"GroupSyncSettings";
			this->GroupSyncSettings->Size = System::Drawing::Size(401, 163);
			this->GroupSyncSettings->TabIndex = 3;
			this->GroupSyncSettings->TabStop = false;
			this->GroupSyncSettings->Text = L"Sync Settings";
			this->CheckboxAutoDeleteLogs->AutoSize = true;
			this->CheckboxAutoDeleteLogs->Location = System::Drawing::Point(16, 134);
			this->CheckboxAutoDeleteLogs->Name = L"CheckboxAutoDeleteLogs";
			this->CheckboxAutoDeleteLogs->Size = System::Drawing::Size(252, 17);
			this->CheckboxAutoDeleteLogs->TabIndex = 4;
			this->CheckboxAutoDeleteLogs->Text = L"Automatically delete log files when syncing ends";
			this->CheckboxAutoDeleteLogs->UseVisualStyleBackColor = true;
			this->GroupStartupFileHandling->Controls->Add(this->RadioPromptForFileHandling);
			this->GroupStartupFileHandling->Controls->Add(this->RadioUseExistingFiles);
			this->GroupStartupFileHandling->Controls->Add(this->RadioOverwriteExistingFiles);
			this->GroupStartupFileHandling->Location = System::Drawing::Point(16, 53);
			this->GroupStartupFileHandling->Name = L"GroupStartupFileHandling";
			this->GroupStartupFileHandling->Size = System::Drawing::Size(362, 66);
			this->GroupStartupFileHandling->TabIndex = 3;
			this->GroupStartupFileHandling->TabStop = false;
			this->GroupStartupFileHandling->Text = L"Existing File Handling During Startup";
			this->RadioPromptForFileHandling->AutoSize = true;
			this->RadioPromptForFileHandling->Location = System::Drawing::Point(6, 42);
			this->RadioPromptForFileHandling->Name = L"RadioPromptForFileHandling";
			this->RadioPromptForFileHandling->Size = System::Drawing::Size(116, 17);
			this->RadioPromptForFileHandling->TabIndex = 2;
			this->RadioPromptForFileHandling->TabStop = true;
			this->RadioPromptForFileHandling->Text = L"Prompt for each file";
			this->RadioPromptForFileHandling->UseVisualStyleBackColor = true;
			this->RadioUseExistingFiles->AutoSize = true;
			this->RadioUseExistingFiles->Location = System::Drawing::Point(211, 19);
			this->RadioUseExistingFiles->Name = L"RadioUseExistingFiles";
			this->RadioUseExistingFiles->Size = System::Drawing::Size(145, 17);
			this->RadioUseExistingFiles->TabIndex = 1;
			this->RadioUseExistingFiles->TabStop = true;
			this->RadioUseExistingFiles->Text = L"Use without modifications";
			this->RadioUseExistingFiles->UseVisualStyleBackColor = true;
			this->RadioOverwriteExistingFiles->AutoSize = true;
			this->RadioOverwriteExistingFiles->Location = System::Drawing::Point(6, 19);
			this->RadioOverwriteExistingFiles->Name = L"RadioOverwriteExistingFiles";
			this->RadioOverwriteExistingFiles->Size = System::Drawing::Size(171, 17);
			this->RadioOverwriteExistingFiles->TabIndex = 0;
			this->RadioOverwriteExistingFiles->TabStop = true;
			this->RadioOverwriteExistingFiles->Text = L"Overwrite with plugin script text";
			this->RadioOverwriteExistingFiles->UseVisualStyleBackColor = true;
			this->LabelSeconds->AutoSize = true;
			this->LabelSeconds->Location = System::Drawing::Point(271, 20);
			this->LabelSeconds->Name = L"LabelSeconds";
			this->LabelSeconds->Size = System::Drawing::Size(47, 13);
			this->LabelSeconds->TabIndex = 2;
			this->LabelSeconds->Text = L"seconds";
			this->NumericAutoSyncSeconds->Location = System::Drawing::Point(208, 18);
			this->NumericAutoSyncSeconds->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 60, 0, 0, 0 });
			this->NumericAutoSyncSeconds->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->NumericAutoSyncSeconds->Name = L"NumericAutoSyncSeconds";
			this->NumericAutoSyncSeconds->Size = System::Drawing::Size(57, 20);
			this->NumericAutoSyncSeconds->TabIndex = 1;
			this->NumericAutoSyncSeconds->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->CheckboxAutoSync->AutoSize = true;
			this->CheckboxAutoSync->Location = System::Drawing::Point(16, 19);
			this->CheckboxAutoSync->Name = L"CheckboxAutoSync";
			this->CheckboxAutoSync->Size = System::Drawing::Size(186, 17);
			this->CheckboxAutoSync->TabIndex = 0;
			this->CheckboxAutoSync->Text = L"Automatically sync changes every";
			this->CheckboxAutoSync->UseVisualStyleBackColor = true;
			this->ButtonStartStopSync->Location = System::Drawing::Point(15, 373);
			this->ButtonStartStopSync->Name = L"ButtonStartStopSync";
			this->ButtonStartStopSync->Size = System::Drawing::Size(396, 32);
			this->ButtonStartStopSync->TabIndex = 4;
			this->ButtonStartStopSync->Text = L"Start Syncing";
			this->ButtonStartStopSync->UseVisualStyleBackColor = true;
			this->LabelScriptsToSync->AutoSize = true;
			this->LabelScriptsToSync->Location = System::Drawing::Point(12, 9);
			this->LabelScriptsToSync->Name = L"LabelScriptsToSync";
			this->LabelScriptsToSync->Size = System::Drawing::Size(78, 13);
			this->LabelScriptsToSync->TabIndex = 6;
			this->LabelScriptsToSync->Text = L"Synced Scripts";
			this->ButtonSelectScripts->Location = System::Drawing::Point(345, 5);
			this->ButtonSelectScripts->Name = L"ButtonSelectScripts";
			this->ButtonSelectScripts->Size = System::Drawing::Size(67, 21);
			this->ButtonSelectScripts->TabIndex = 7;
			this->ButtonSelectScripts->Text = L"Select...";
			this->ButtonSelectScripts->UseVisualStyleBackColor = true;
			this->ListViewSyncedScripts->AllColumns->Add(this->ColScriptName);
			this->ListViewSyncedScripts->AllColumns->Add(this->ColLastSyncTime);
			this->ListViewSyncedScripts->CellEditUseWholeCell = false;
			this->ListViewSyncedScripts->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {
				this->ColScriptName,
					this->ColLastSyncTime
			});
			this->ListViewSyncedScripts->ContextMenuStrip = this->LVSyncedStripsContextMenu;
			this->ListViewSyncedScripts->Cursor = System::Windows::Forms::Cursors::Default;
			this->ListViewSyncedScripts->HideSelection = false;
			this->ListViewSyncedScripts->Location = System::Drawing::Point(15, 36);
			this->ListViewSyncedScripts->Name = L"ListViewSyncedScripts";
			this->ListViewSyncedScripts->ShowGroups = false;
			this->ListViewSyncedScripts->Size = System::Drawing::Size(396, 331);
			this->ListViewSyncedScripts->TabIndex = 8;
			this->ListViewSyncedScripts->UseCompatibleStateImageBehavior = false;
			this->ListViewSyncedScripts->View = System::Windows::Forms::View::Details;
			this->ListViewSyncedScripts->VirtualMode = true;
			this->ColScriptName->Sortable = false;
			this->ColScriptName->Text = L"EditorID";
			this->ColScriptName->Width = 211;
			this->ColLastSyncTime->Sortable = false;
			this->ColLastSyncTime->Text = L"Last Sync Attempt Time";
			this->ColLastSyncTime->Width = 169;
			this->LVSyncedStripsContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
				this->SyncToDiskToolStripMenuItem,
					this->SyncFromDiskToolStripMenuItem, this->ToolStripSeparator1, this->ShowLogToolStripMenuItem
			});
			this->LVSyncedStripsContextMenu->Name = L"LVSyncedStripsContextMenu";
			this->LVSyncedStripsContextMenu->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
			this->LVSyncedStripsContextMenu->ShowImageMargin = false;
			this->LVSyncedStripsContextMenu->Size = System::Drawing::Size(131, 76);
			this->SyncToDiskToolStripMenuItem->Name = L"SyncToDiskToolStripMenuItem";
			this->SyncToDiskToolStripMenuItem->Size = System::Drawing::Size(130, 22);
			this->SyncToDiskToolStripMenuItem->Text = L"Sync To Disk";
			this->SyncFromDiskToolStripMenuItem->Name = L"SyncFromDiskToolStripMenuItem";
			this->SyncFromDiskToolStripMenuItem->Size = System::Drawing::Size(130, 22);
			this->SyncFromDiskToolStripMenuItem->Text = L"Sync From Disk";
			this->ToolStripSeparator1->Name = L"ToolStripSeparator1";
			this->ToolStripSeparator1->Size = System::Drawing::Size(127, 6);
			this->ShowLogToolStripMenuItem->Name = L"ShowLogToolStripMenuItem";
			this->ShowLogToolStripMenuItem->Size = System::Drawing::Size(130, 22);
			this->ShowLogToolStripMenuItem->Text = L"Show Log";
			this->TextBoxSelectedScriptLog->Location = System::Drawing::Point(418, 52);
			this->TextBoxSelectedScriptLog->Multiline = true;
			this->TextBoxSelectedScriptLog->Name = L"TextBoxSelectedScriptLog";
			this->TextBoxSelectedScriptLog->ReadOnly = true;
			this->TextBoxSelectedScriptLog->Size = System::Drawing::Size(401, 184);
			this->TextBoxSelectedScriptLog->TabIndex = 11;
			this->LabelSelectedScriptLog->AutoSize = true;
			this->LabelSelectedScriptLog->Location = System::Drawing::Point(418, 36);
			this->LabelSelectedScriptLog->Name = L"LabelSelectedScriptLog";
			this->LabelSelectedScriptLog->Size = System::Drawing::Size(25, 13);
			this->LabelSelectedScriptLog->TabIndex = 12;
			this->LabelSelectedScriptLog->Text = L"Log";
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(830, 413);
			this->Controls->Add(this->LabelSelectedScriptLog);
			this->Controls->Add(this->TextBoxSelectedScriptLog);
			this->Controls->Add(this->ListViewSyncedScripts);
			this->Controls->Add(this->ButtonSelectScripts);
			this->Controls->Add(this->LabelScriptsToSync);
			this->Controls->Add(this->ButtonStartStopSync);
			this->Controls->Add(this->GroupSyncSettings);
			this->Controls->Add(this->ButtonSelectWorkingDir);
			this->Controls->Add(this->TextBoxWorkingDir);
			this->Controls->Add(this->LabelWorkingDir);
			this->DoubleBuffered = true;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
			this->Name = L"SESyncUI";
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Sync Scripts to Disk";
			this->GroupSyncSettings->ResumeLayout(false);
			this->GroupSyncSettings->PerformLayout();
			this->GroupStartupFileHandling->ResumeLayout(false);
			this->GroupStartupFileHandling->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NumericAutoSyncSeconds))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewSyncedScripts))->EndInit();
			this->LVSyncedStripsContextMenu->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	};
}
