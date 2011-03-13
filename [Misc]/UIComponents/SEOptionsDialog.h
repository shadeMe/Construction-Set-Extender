#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace UIComponents {

	/// <summary>
	/// Summary for OptionsDialog
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class OptionsDialog : public System::Windows::Forms::Form
	{
	public:
		OptionsDialog(void)
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
		~OptionsDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  CmDlgHighlightColor;
	protected: 




	private: System::Windows::Forms::Button^  CmDlgBackColor;
	private: System::Windows::Forms::Button^  CmDlgForeColor;
	protected: 








	private: System::Windows::Forms::Button^  CmDlgFont;
	private: System::Windows::Forms::CheckBox^  AutoIndent;
	private: System::Windows::Forms::CheckBox^  SaveLastKnownPos;





	private: System::Windows::Forms::CheckBox^  AllowRedefinitions;
	private: System::Windows::Forms::Label^  LabelISThreshold;
	private: System::Windows::Forms::NumericUpDown^  ThresholdLength;













	private: System::Windows::Forms::Label^  LabelTabSize;
	private: System::Windows::Forms::NumericUpDown^  TabSize;


	private: System::Windows::Forms::CheckBox^  RecompileVarIdx;

	private: System::Windows::Forms::CheckBox^  UseCSParent;
	private: System::Windows::Forms::CheckBox^  DestroyOnLastTabClose;
	private: System::Windows::Forms::TabControl^  TabContainer;







	private: System::Windows::Forms::TabPage^  TabGeneral;
	private: System::Windows::Forms::TabPage^  TabIntelliSense;
	private: System::Windows::Forms::TabPage^  TabPreprocessor;
	private: System::Windows::Forms::TabPage^  TabAppearance;
	private: System::Windows::Forms::CheckBox^  SuppressRefCountForQuestScripts;
	private: System::Windows::Forms::CheckBox^  LoadScriptUpdateExistingScripts;
	private: System::Windows::Forms::Label^  LabelISDBUpdatePeriod;
	private: System::Windows::Forms::NumericUpDown^  DatabaseUpdateInterval;


	private: System::Windows::Forms::Label^  LabelLinesToScroll;
	private: System::Windows::Forms::NumericUpDown^  LinesToScroll;
	private: System::Windows::Forms::CheckBox^  UseQuickView;
	private: System::Windows::Forms::TabPage^  TabSanitize;
	private: System::Windows::Forms::CheckBox^  IndentLines;

	private: System::Windows::Forms::CheckBox^  AnnealCasing;





















	protected: 

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
			this->AllowRedefinitions = (gcnew System::Windows::Forms::CheckBox());
			this->LabelISThreshold = (gcnew System::Windows::Forms::Label());
			this->ThresholdLength = (gcnew System::Windows::Forms::NumericUpDown());
			this->DestroyOnLastTabClose = (gcnew System::Windows::Forms::CheckBox());
			this->UseCSParent = (gcnew System::Windows::Forms::CheckBox());
			this->RecompileVarIdx = (gcnew System::Windows::Forms::CheckBox());
			this->LabelTabSize = (gcnew System::Windows::Forms::Label());
			this->TabSize = (gcnew System::Windows::Forms::NumericUpDown());
			this->SaveLastKnownPos = (gcnew System::Windows::Forms::CheckBox());
			this->AutoIndent = (gcnew System::Windows::Forms::CheckBox());
			this->CmDlgHighlightColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgBackColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgForeColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgFont = (gcnew System::Windows::Forms::Button());
			this->TabContainer = (gcnew System::Windows::Forms::TabControl());
			this->TabGeneral = (gcnew System::Windows::Forms::TabPage());
			this->TabIntelliSense = (gcnew System::Windows::Forms::TabPage());
			this->TabPreprocessor = (gcnew System::Windows::Forms::TabPage());
			this->TabAppearance = (gcnew System::Windows::Forms::TabPage());
			this->SuppressRefCountForQuestScripts = (gcnew System::Windows::Forms::CheckBox());
			this->LoadScriptUpdateExistingScripts = (gcnew System::Windows::Forms::CheckBox());
			this->LabelISDBUpdatePeriod = (gcnew System::Windows::Forms::Label());
			this->DatabaseUpdateInterval = (gcnew System::Windows::Forms::NumericUpDown());
			this->LabelLinesToScroll = (gcnew System::Windows::Forms::Label());
			this->LinesToScroll = (gcnew System::Windows::Forms::NumericUpDown());
			this->UseQuickView = (gcnew System::Windows::Forms::CheckBox());
			this->TabSanitize = (gcnew System::Windows::Forms::TabPage());
			this->AnnealCasing = (gcnew System::Windows::Forms::CheckBox());
			this->IndentLines = (gcnew System::Windows::Forms::CheckBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ThresholdLength))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->TabSize))->BeginInit();
			this->TabContainer->SuspendLayout();
			this->TabGeneral->SuspendLayout();
			this->TabIntelliSense->SuspendLayout();
			this->TabPreprocessor->SuspendLayout();
			this->TabAppearance->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->DatabaseUpdateInterval))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->LinesToScroll))->BeginInit();
			this->TabSanitize->SuspendLayout();
			this->SuspendLayout();
			// 
			// AllowRedefinitions
			// 
			this->AllowRedefinitions->Location = System::Drawing::Point(147, 120);
			this->AllowRedefinitions->Name = L"AllowRedefinitions";
			this->AllowRedefinitions->Size = System::Drawing::Size(168, 51);
			this->AllowRedefinitions->TabIndex = 9;
			this->AllowRedefinitions->Text = L"Allow Macro Redefinitions";
			this->AllowRedefinitions->UseVisualStyleBackColor = true;
			// 
			// LabelISThreshold
			// 
			this->LabelISThreshold->AutoSize = true;
			this->LabelISThreshold->Location = System::Drawing::Point(30, 69);
			this->LabelISThreshold->Name = L"LabelISThreshold";
			this->LabelISThreshold->Size = System::Drawing::Size(148, 13);
			this->LabelISThreshold->TabIndex = 1;
			this->LabelISThreshold->Text = L"IntelliSense Pop-up Threshold";
			// 
			// ThresholdLength
			// 
			this->ThresholdLength->Location = System::Drawing::Point(30, 104);
			this->ThresholdLength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, 0});
			this->ThresholdLength->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {3, 0, 0, 0});
			this->ThresholdLength->Name = L"ThresholdLength";
			this->ThresholdLength->Size = System::Drawing::Size(148, 20);
			this->ThresholdLength->TabIndex = 0;
			this->ThresholdLength->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {3, 0, 0, 0});
			// 
			// DestroyOnLastTabClose
			// 
			this->DestroyOnLastTabClose->AutoSize = true;
			this->DestroyOnLastTabClose->Location = System::Drawing::Point(22, 92);
			this->DestroyOnLastTabClose->Name = L"DestroyOnLastTabClose";
			this->DestroyOnLastTabClose->Size = System::Drawing::Size(152, 17);
			this->DestroyOnLastTabClose->TabIndex = 11;
			this->DestroyOnLastTabClose->Text = L"Close Editor With Last Tab";
			this->DestroyOnLastTabClose->UseVisualStyleBackColor = true;
			// 
			// UseCSParent
			// 
			this->UseCSParent->AutoSize = true;
			this->UseCSParent->Location = System::Drawing::Point(22, 46);
			this->UseCSParent->Name = L"UseCSParent";
			this->UseCSParent->Size = System::Drawing::Size(166, 17);
			this->UseCSParent->TabIndex = 10;
			this->UseCSParent->Text = L"Show Editor As Child Window";
			this->UseCSParent->UseVisualStyleBackColor = true;
			// 
			// RecompileVarIdx
			// 
			this->RecompileVarIdx->AutoSize = true;
			this->RecompileVarIdx->Location = System::Drawing::Point(22, 250);
			this->RecompileVarIdx->Name = L"RecompileVarIdx";
			this->RecompileVarIdx->Size = System::Drawing::Size(303, 17);
			this->RecompileVarIdx->TabIndex = 9;
			this->RecompileVarIdx->Text = L"Recompile Dependencies After Variable Index Modification";
			this->RecompileVarIdx->UseVisualStyleBackColor = true;
			// 
			// LabelTabSize
			// 
			this->LabelTabSize->Location = System::Drawing::Point(160, 55);
			this->LabelTabSize->Name = L"LabelTabSize";
			this->LabelTabSize->Size = System::Drawing::Size(103, 18);
			this->LabelTabSize->TabIndex = 3;
			this->LabelTabSize->Text = L"Tab Size ( In Pixels )";
			this->LabelTabSize->Click += gcnew System::EventHandler(this, &OptionsDialog::label2_Click);
			// 
			// TabSize
			// 
			this->TabSize->Location = System::Drawing::Point(269, 53);
			this->TabSize->Name = L"TabSize";
			this->TabSize->Size = System::Drawing::Size(103, 20);
			this->TabSize->TabIndex = 2;
			// 
			// SaveLastKnownPos
			// 
			this->SaveLastKnownPos->AutoSize = true;
			this->SaveLastKnownPos->Location = System::Drawing::Point(22, 69);
			this->SaveLastKnownPos->Name = L"SaveLastKnownPos";
			this->SaveLastKnownPos->Size = System::Drawing::Size(174, 17);
			this->SaveLastKnownPos->TabIndex = 8;
			this->SaveLastKnownPos->Text = L"Save Caret Position With Script";
			this->SaveLastKnownPos->UseVisualStyleBackColor = true;
			this->SaveLastKnownPos->CheckedChanged += gcnew System::EventHandler(this, &OptionsDialog::caretpos_CheckedChanged);
			// 
			// AutoIndent
			// 
			this->AutoIndent->AutoSize = true;
			this->AutoIndent->Location = System::Drawing::Point(22, 23);
			this->AutoIndent->Name = L"AutoIndent";
			this->AutoIndent->Size = System::Drawing::Size(139, 17);
			this->AutoIndent->TabIndex = 5;
			this->AutoIndent->Text = L"Auto-Indent Script Lines";
			this->AutoIndent->UseVisualStyleBackColor = true;
			// 
			// CmDlgHighlightColor
			// 
			this->CmDlgHighlightColor->Location = System::Drawing::Point(21, 153);
			this->CmDlgHighlightColor->Name = L"CmDlgHighlightColor";
			this->CmDlgHighlightColor->Size = System::Drawing::Size(111, 34);
			this->CmDlgHighlightColor->TabIndex = 3;
			this->CmDlgHighlightColor->Text = L"Line Highlight Color";
			this->CmDlgHighlightColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgBackColor
			// 
			this->CmDlgBackColor->Location = System::Drawing::Point(21, 103);
			this->CmDlgBackColor->Name = L"CmDlgBackColor";
			this->CmDlgBackColor->Size = System::Drawing::Size(111, 34);
			this->CmDlgBackColor->TabIndex = 2;
			this->CmDlgBackColor->Text = L"Background Color";
			this->CmDlgBackColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgForeColor
			// 
			this->CmDlgForeColor->Location = System::Drawing::Point(21, 203);
			this->CmDlgForeColor->Name = L"CmDlgForeColor";
			this->CmDlgForeColor->Size = System::Drawing::Size(111, 34);
			this->CmDlgForeColor->TabIndex = 1;
			this->CmDlgForeColor->Text = L"Foreground Color";
			this->CmDlgForeColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgFont
			// 
			this->CmDlgFont->Location = System::Drawing::Point(21, 53);
			this->CmDlgFont->Name = L"CmDlgFont";
			this->CmDlgFont->Size = System::Drawing::Size(111, 34);
			this->CmDlgFont->TabIndex = 0;
			this->CmDlgFont->Text = L"Editor Font";
			this->CmDlgFont->UseVisualStyleBackColor = true;
			this->CmDlgFont->Click += gcnew System::EventHandler(this, &OptionsDialog::button1_Click);
			// 
			// TabContainer
			// 
			this->TabContainer->Controls->Add(this->TabGeneral);
			this->TabContainer->Controls->Add(this->TabIntelliSense);
			this->TabContainer->Controls->Add(this->TabPreprocessor);
			this->TabContainer->Controls->Add(this->TabAppearance);
			this->TabContainer->Controls->Add(this->TabSanitize);
			this->TabContainer->HotTrack = true;
			this->TabContainer->Location = System::Drawing::Point(12, 12);
			this->TabContainer->Multiline = true;
			this->TabContainer->Name = L"TabContainer";
			this->TabContainer->SelectedIndex = 0;
			this->TabContainer->Size = System::Drawing::Size(431, 316);
			this->TabContainer->TabIndex = 0;
			// 
			// TabGeneral
			// 
			this->TabGeneral->Controls->Add(this->LoadScriptUpdateExistingScripts);
			this->TabGeneral->Controls->Add(this->DestroyOnLastTabClose);
			this->TabGeneral->Controls->Add(this->SuppressRefCountForQuestScripts);
			this->TabGeneral->Controls->Add(this->SaveLastKnownPos);
			this->TabGeneral->Controls->Add(this->RecompileVarIdx);
			this->TabGeneral->Controls->Add(this->AutoIndent);
			this->TabGeneral->Controls->Add(this->UseCSParent);
			this->TabGeneral->Location = System::Drawing::Point(4, 22);
			this->TabGeneral->Name = L"TabGeneral";
			this->TabGeneral->Padding = System::Windows::Forms::Padding(3);
			this->TabGeneral->Size = System::Drawing::Size(423, 290);
			this->TabGeneral->TabIndex = 0;
			this->TabGeneral->Text = L"General";
			this->TabGeneral->UseVisualStyleBackColor = true;
			// 
			// TabIntelliSense
			// 
			this->TabIntelliSense->Controls->Add(this->UseQuickView);
			this->TabIntelliSense->Controls->Add(this->LabelISDBUpdatePeriod);
			this->TabIntelliSense->Controls->Add(this->DatabaseUpdateInterval);
			this->TabIntelliSense->Controls->Add(this->LabelISThreshold);
			this->TabIntelliSense->Controls->Add(this->ThresholdLength);
			this->TabIntelliSense->Location = System::Drawing::Point(4, 22);
			this->TabIntelliSense->Name = L"TabIntelliSense";
			this->TabIntelliSense->Padding = System::Windows::Forms::Padding(3);
			this->TabIntelliSense->Size = System::Drawing::Size(423, 290);
			this->TabIntelliSense->TabIndex = 1;
			this->TabIntelliSense->Text = L"IntelliSense";
			this->TabIntelliSense->UseVisualStyleBackColor = true;
			// 
			// TabPreprocessor
			// 
			this->TabPreprocessor->Controls->Add(this->AllowRedefinitions);
			this->TabPreprocessor->Location = System::Drawing::Point(4, 22);
			this->TabPreprocessor->Name = L"TabPreprocessor";
			this->TabPreprocessor->Padding = System::Windows::Forms::Padding(3);
			this->TabPreprocessor->Size = System::Drawing::Size(423, 290);
			this->TabPreprocessor->TabIndex = 2;
			this->TabPreprocessor->Text = L"Preprocessor";
			this->TabPreprocessor->UseVisualStyleBackColor = true;
			// 
			// TabAppearance
			// 
			this->TabAppearance->Controls->Add(this->LabelLinesToScroll);
			this->TabAppearance->Controls->Add(this->LinesToScroll);
			this->TabAppearance->Controls->Add(this->CmDlgFont);
			this->TabAppearance->Controls->Add(this->CmDlgBackColor);
			this->TabAppearance->Controls->Add(this->CmDlgHighlightColor);
			this->TabAppearance->Controls->Add(this->CmDlgForeColor);
			this->TabAppearance->Controls->Add(this->LabelTabSize);
			this->TabAppearance->Controls->Add(this->TabSize);
			this->TabAppearance->Location = System::Drawing::Point(4, 22);
			this->TabAppearance->Name = L"TabAppearance";
			this->TabAppearance->Padding = System::Windows::Forms::Padding(3);
			this->TabAppearance->Size = System::Drawing::Size(423, 290);
			this->TabAppearance->TabIndex = 3;
			this->TabAppearance->Text = L"Appearance";
			this->TabAppearance->UseVisualStyleBackColor = true;
			this->TabAppearance->Click += gcnew System::EventHandler(this, &OptionsDialog::TabAppearance_Click);
			// 
			// SuppressRefCountForQuestScripts
			// 
			this->SuppressRefCountForQuestScripts->AutoSize = true;
			this->SuppressRefCountForQuestScripts->Location = System::Drawing::Point(22, 227);
			this->SuppressRefCountForQuestScripts->Name = L"SuppressRefCountForQuestScripts";
			this->SuppressRefCountForQuestScripts->Size = System::Drawing::Size(293, 17);
			this->SuppressRefCountForQuestScripts->TabIndex = 12;
			this->SuppressRefCountForQuestScripts->Text = L"Suppress Variable Reference Counting For Quest Scripts";
			this->SuppressRefCountForQuestScripts->UseVisualStyleBackColor = true;
			// 
			// LoadScriptUpdateExistingScripts
			// 
			this->LoadScriptUpdateExistingScripts->AutoSize = true;
			this->LoadScriptUpdateExistingScripts->Location = System::Drawing::Point(22, 115);
			this->LoadScriptUpdateExistingScripts->Name = L"LoadScriptUpdateExistingScripts";
			this->LoadScriptUpdateExistingScripts->Size = System::Drawing::Size(236, 17);
			this->LoadScriptUpdateExistingScripts->TabIndex = 13;
			this->LoadScriptUpdateExistingScripts->Text = L"\'Load Script(s)\' Tool Updates Existing Scripts";
			this->LoadScriptUpdateExistingScripts->UseVisualStyleBackColor = true;
			// 
			// LabelISDBUpdatePeriod
			// 
			this->LabelISDBUpdatePeriod->AutoSize = true;
			this->LabelISDBUpdatePeriod->Location = System::Drawing::Point(30, 152);
			this->LabelISDBUpdatePeriod->Name = L"LabelISDBUpdatePeriod";
			this->LabelISDBUpdatePeriod->Size = System::Drawing::Size(181, 26);
			this->LabelISDBUpdatePeriod->TabIndex = 3;
			this->LabelISDBUpdatePeriod->Text = L"IntelliSense Database Update Period\r\n(In Earth Minutes)";
			// 
			// DatabaseUpdateInterval
			// 
			this->DatabaseUpdateInterval->Location = System::Drawing::Point(30, 201);
			this->DatabaseUpdateInterval->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {15, 0, 0, 0});
			this->DatabaseUpdateInterval->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
			this->DatabaseUpdateInterval->Name = L"DatabaseUpdateInterval";
			this->DatabaseUpdateInterval->Size = System::Drawing::Size(148, 20);
			this->DatabaseUpdateInterval->TabIndex = 2;
			this->DatabaseUpdateInterval->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
			this->DatabaseUpdateInterval->ValueChanged += gcnew System::EventHandler(this, &OptionsDialog::ISDBUpdateInterval_ValueChanged);
			// 
			// LabelLinesToScroll
			// 
			this->LabelLinesToScroll->AutoSize = true;
			this->LabelLinesToScroll->Location = System::Drawing::Point(160, 94);
			this->LabelLinesToScroll->Name = L"LabelLinesToScroll";
			this->LabelLinesToScroll->Size = System::Drawing::Size(98, 26);
			this->LabelLinesToScroll->TabIndex = 5;
			this->LabelLinesToScroll->Text = L"Lines To Scroll\r\nWith Mouse Wheel";
			// 
			// LinesToScroll
			// 
			this->LinesToScroll->Location = System::Drawing::Point(269, 100);
			this->LinesToScroll->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {15, 0, 0, 0});
			this->LinesToScroll->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
			this->LinesToScroll->Name = L"LinesToScroll";
			this->LinesToScroll->Size = System::Drawing::Size(103, 20);
			this->LinesToScroll->TabIndex = 4;
			this->LinesToScroll->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
			// 
			// UseQuickView
			// 
			this->UseQuickView->AutoSize = true;
			this->UseQuickView->Location = System::Drawing::Point(231, 69);
			this->UseQuickView->Name = L"UseQuickView";
			this->UseQuickView->Size = System::Drawing::Size(89, 17);
			this->UseQuickView->TabIndex = 6;
			this->UseQuickView->Text = L"Quick-View™";
			this->UseQuickView->UseVisualStyleBackColor = true;
			// 
			// TabSanitize
			// 
			this->TabSanitize->Controls->Add(this->IndentLines);
			this->TabSanitize->Controls->Add(this->AnnealCasing);
			this->TabSanitize->Location = System::Drawing::Point(4, 22);
			this->TabSanitize->Name = L"TabSanitize";
			this->TabSanitize->Padding = System::Windows::Forms::Padding(3);
			this->TabSanitize->Size = System::Drawing::Size(423, 290);
			this->TabSanitize->TabIndex = 4;
			this->TabSanitize->Text = L"Sanitize";
			this->TabSanitize->UseVisualStyleBackColor = true;
			// 
			// AnnealCasing
			// 
			this->AnnealCasing->AutoSize = true;
			this->AnnealCasing->Location = System::Drawing::Point(128, 109);
			this->AnnealCasing->Name = L"AnnealCasing";
			this->AnnealCasing->Size = System::Drawing::Size(205, 17);
			this->AnnealCasing->TabIndex = 11;
			this->AnnealCasing->Text = L"Anneal Script Command Name Casing";
			this->AnnealCasing->UseVisualStyleBackColor = true;
			// 
			// IndentLines
			// 
			this->IndentLines->AutoSize = true;
			this->IndentLines->Location = System::Drawing::Point(128, 164);
			this->IndentLines->Name = L"IndentLines";
			this->IndentLines->Size = System::Drawing::Size(114, 17);
			this->IndentLines->TabIndex = 12;
			this->IndentLines->Text = L"Indent Script Lines";
			this->IndentLines->UseVisualStyleBackColor = true;
			// 
			// OptionsDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(456, 337);
			this->Controls->Add(this->TabContainer);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = L"OptionsDialog";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Preferences";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ThresholdLength))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->TabSize))->EndInit();
			this->TabContainer->ResumeLayout(false);
			this->TabGeneral->ResumeLayout(false);
			this->TabGeneral->PerformLayout();
			this->TabIntelliSense->ResumeLayout(false);
			this->TabIntelliSense->PerformLayout();
			this->TabPreprocessor->ResumeLayout(false);
			this->TabAppearance->ResumeLayout(false);
			this->TabAppearance->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->DatabaseUpdateInterval))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->LinesToScroll))->EndInit();
			this->TabSanitize->ResumeLayout(false);
			this->TabSanitize->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void groupBox1_Enter(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
private: System::Void label2_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void TabAppearance_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void caretpos_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void ISDBUpdateInterval_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
		 }
};
}
