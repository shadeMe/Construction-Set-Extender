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

	protected:

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

	private: System::Windows::Forms::CheckBox^  LoadScriptUpdateExistingScripts;
	private: System::Windows::Forms::Label^  LabelISDBUpdatePeriod;
	private: System::Windows::Forms::NumericUpDown^  DatabaseUpdateInterval;

	private: System::Windows::Forms::CheckBox^  UseQuickView;
	private: System::Windows::Forms::TabPage^  TabSanitize;
	private: System::Windows::Forms::CheckBox^  IndentLines;

	private: System::Windows::Forms::CheckBox^  AnnealCasing;
	private: System::Windows::Forms::GroupBox^  GroupBoxSyntaxHighlighting;
	private: System::Windows::Forms::Button^  CmDlgSyntaxDigitsColor;

	private: System::Windows::Forms::Button^  CmDlgSyntaxPreprocessorColor;

	private: System::Windows::Forms::Button^  CmDlgSyntaxScriptBlocksColor;

	private: System::Windows::Forms::Button^  CmDlgSyntaxDelimitersColor;

	private: System::Windows::Forms::Button^  CmDlgSyntaxStringsColor;
private: System::Windows::Forms::Button^  CmDlgSyntaxKeywordsColor;

	private: System::Windows::Forms::Label^  LabelPreprocessor;
	private: System::Windows::Forms::Label^  LabelScriptBlocks;
	private: System::Windows::Forms::Label^  LabelStrings;

	private: System::Windows::Forms::Label^  LabelDigits;
	private: System::Windows::Forms::Label^  LabelDelimiters;
	private: System::Windows::Forms::Label^  LabelKeywords;
private: System::Windows::Forms::Button^  CmDlgCurrentLineHighlightColor;
private: System::Windows::Forms::Label^  LabelCurrentLineHighlight;

private: System::Windows::Forms::Button^  CmDlgCharLimitColor;
private: System::Windows::Forms::Label^  LabelCharLimitHighlight;
private: System::Windows::Forms::Button^  CmDlgErrorHighlightColor;

private: System::Windows::Forms::Label^  LabelErrorHighlight;

private: System::Windows::Forms::Button^  CmDlgSelectionHighlightColor;
private: System::Windows::Forms::CheckBox^  WordWrap;
private: System::Windows::Forms::Label^  LabelSelectionHighlight;
private: System::Windows::Forms::Button^  CmDlgFindResultsHighlightColor;
private: System::Windows::Forms::Label^  LabelFindResultsHighlight;
private: System::Windows::Forms::Button^  CmDlgSyntaxCommentsColor;
private: System::Windows::Forms::Label^  LabelComments;
private: System::Windows::Forms::CheckBox^  CutCopyEntireLine;
private: System::Windows::Forms::CheckBox^  ShowSpaces;
private: System::Windows::Forms::CheckBox^  ShowTabs;
private: System::Windows::Forms::Label^  LabelNoOfPasses;
private: System::Windows::Forms::NumericUpDown^  NoOfPasses;
private: System::Windows::Forms::CheckBox^  CodeFolding;
private: System::Windows::Forms::CheckBox^  TabsOnTop;
	private: System::Windows::Forms::Label^  LabelMaxVisibleItems;
	private: System::Windows::Forms::NumericUpDown^  MaxVisibleItems;
private: System::Windows::Forms::CheckBox^  BoldFacedHighlighting;
private: System::Windows::Forms::CheckBox^  EvalifyIfs;
private: System::Windows::Forms::CheckBox^  CompilerOverrideBlocks;
private: System::Windows::Forms::CheckBox^  NoFocusUI;
private: System::Windows::Forms::TabPage^  TabBackup;
private: System::Windows::Forms::Label^  LabelAutoRecoveryInterval;

private: System::Windows::Forms::NumericUpDown^  AutoRecoverySavePeriod;
private: System::Windows::Forms::CheckBox^  UseAutoRecovery;
private: System::Windows::Forms::Button^  ForceDatabaseUpdate;
private: System::Windows::Forms::TabPage^  TabValidator;
private: System::Windows::Forms::CheckBox^  VarFormNameCollisions;

private: System::Windows::Forms::CheckBox^  SuppressRefCountForQuestScripts;
private: System::Windows::Forms::CheckBox^  CountVarRefs;
private: System::Windows::Forms::CheckBox^  SubstringSearch;
private: System::Windows::Forms::CheckBox^  VarCmdNameCollisions;
private: System::Windows::Forms::Button^  CmDlgBackgroundColor;
private: System::Windows::Forms::Button^  CmDlgForegroundColor;
private: System::Windows::Forms::Label^  LabelBackgroundColor;
private: System::Windows::Forms::Label^  LabelForegroundColor;
private: System::Windows::Forms::Button^  CmDlgSyntaxLocalVarsColor;
private: System::Windows::Forms::Label^  LabelLocalVars;

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
			this->CmDlgFont = (gcnew System::Windows::Forms::Button());
			this->TabContainer = (gcnew System::Windows::Forms::TabControl());
			this->TabGeneral = (gcnew System::Windows::Forms::TabPage());
			this->CutCopyEntireLine = (gcnew System::Windows::Forms::CheckBox());
			this->LoadScriptUpdateExistingScripts = (gcnew System::Windows::Forms::CheckBox());
			this->TabIntelliSense = (gcnew System::Windows::Forms::TabPage());
			this->SubstringSearch = (gcnew System::Windows::Forms::CheckBox());
			this->ForceDatabaseUpdate = (gcnew System::Windows::Forms::Button());
			this->NoFocusUI = (gcnew System::Windows::Forms::CheckBox());
			this->LabelMaxVisibleItems = (gcnew System::Windows::Forms::Label());
			this->MaxVisibleItems = (gcnew System::Windows::Forms::NumericUpDown());
			this->UseQuickView = (gcnew System::Windows::Forms::CheckBox());
			this->LabelISDBUpdatePeriod = (gcnew System::Windows::Forms::Label());
			this->DatabaseUpdateInterval = (gcnew System::Windows::Forms::NumericUpDown());
			this->TabPreprocessor = (gcnew System::Windows::Forms::TabPage());
			this->LabelNoOfPasses = (gcnew System::Windows::Forms::Label());
			this->NoOfPasses = (gcnew System::Windows::Forms::NumericUpDown());
			this->TabAppearance = (gcnew System::Windows::Forms::TabPage());
			this->CmDlgBackgroundColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgForegroundColor = (gcnew System::Windows::Forms::Button());
			this->LabelBackgroundColor = (gcnew System::Windows::Forms::Label());
			this->LabelForegroundColor = (gcnew System::Windows::Forms::Label());
			this->BoldFacedHighlighting = (gcnew System::Windows::Forms::CheckBox());
			this->TabsOnTop = (gcnew System::Windows::Forms::CheckBox());
			this->CodeFolding = (gcnew System::Windows::Forms::CheckBox());
			this->ShowSpaces = (gcnew System::Windows::Forms::CheckBox());
			this->ShowTabs = (gcnew System::Windows::Forms::CheckBox());
			this->CmDlgFindResultsHighlightColor = (gcnew System::Windows::Forms::Button());
			this->LabelFindResultsHighlight = (gcnew System::Windows::Forms::Label());
			this->CmDlgCurrentLineHighlightColor = (gcnew System::Windows::Forms::Button());
			this->LabelCurrentLineHighlight = (gcnew System::Windows::Forms::Label());
			this->CmDlgCharLimitColor = (gcnew System::Windows::Forms::Button());
			this->LabelCharLimitHighlight = (gcnew System::Windows::Forms::Label());
			this->CmDlgErrorHighlightColor = (gcnew System::Windows::Forms::Button());
			this->LabelErrorHighlight = (gcnew System::Windows::Forms::Label());
			this->CmDlgSelectionHighlightColor = (gcnew System::Windows::Forms::Button());
			this->WordWrap = (gcnew System::Windows::Forms::CheckBox());
			this->LabelSelectionHighlight = (gcnew System::Windows::Forms::Label());
			this->GroupBoxSyntaxHighlighting = (gcnew System::Windows::Forms::GroupBox());
			this->CmDlgSyntaxCommentsColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgSyntaxDigitsColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgSyntaxPreprocessorColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgSyntaxScriptBlocksColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgSyntaxDelimitersColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgSyntaxStringsColor = (gcnew System::Windows::Forms::Button());
			this->CmDlgSyntaxKeywordsColor = (gcnew System::Windows::Forms::Button());
			this->LabelPreprocessor = (gcnew System::Windows::Forms::Label());
			this->LabelScriptBlocks = (gcnew System::Windows::Forms::Label());
			this->LabelStrings = (gcnew System::Windows::Forms::Label());
			this->LabelComments = (gcnew System::Windows::Forms::Label());
			this->LabelDigits = (gcnew System::Windows::Forms::Label());
			this->LabelDelimiters = (gcnew System::Windows::Forms::Label());
			this->LabelKeywords = (gcnew System::Windows::Forms::Label());
			this->TabSanitize = (gcnew System::Windows::Forms::TabPage());
			this->EvalifyIfs = (gcnew System::Windows::Forms::CheckBox());
			this->CompilerOverrideBlocks = (gcnew System::Windows::Forms::CheckBox());
			this->IndentLines = (gcnew System::Windows::Forms::CheckBox());
			this->AnnealCasing = (gcnew System::Windows::Forms::CheckBox());
			this->TabBackup = (gcnew System::Windows::Forms::TabPage());
			this->LabelAutoRecoveryInterval = (gcnew System::Windows::Forms::Label());
			this->AutoRecoverySavePeriod = (gcnew System::Windows::Forms::NumericUpDown());
			this->UseAutoRecovery = (gcnew System::Windows::Forms::CheckBox());
			this->TabValidator = (gcnew System::Windows::Forms::TabPage());
			this->VarCmdNameCollisions = (gcnew System::Windows::Forms::CheckBox());
			this->CountVarRefs = (gcnew System::Windows::Forms::CheckBox());
			this->SuppressRefCountForQuestScripts = (gcnew System::Windows::Forms::CheckBox());
			this->VarFormNameCollisions = (gcnew System::Windows::Forms::CheckBox());
			this->LabelLocalVars = (gcnew System::Windows::Forms::Label());
			this->CmDlgSyntaxLocalVarsColor = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ThresholdLength))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->TabSize))->BeginInit();
			this->TabContainer->SuspendLayout();
			this->TabGeneral->SuspendLayout();
			this->TabIntelliSense->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->MaxVisibleItems))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->DatabaseUpdateInterval))->BeginInit();
			this->TabPreprocessor->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NoOfPasses))->BeginInit();
			this->TabAppearance->SuspendLayout();
			this->GroupBoxSyntaxHighlighting->SuspendLayout();
			this->TabSanitize->SuspendLayout();
			this->TabBackup->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->AutoRecoverySavePeriod))->BeginInit();
			this->TabValidator->SuspendLayout();
			this->SuspendLayout();
			// 
			// AllowRedefinitions
			// 
			this->AllowRedefinitions->Location = System::Drawing::Point(133, 97);
			this->AllowRedefinitions->Name = L"AllowRedefinitions";
			this->AllowRedefinitions->Size = System::Drawing::Size(154, 23);
			this->AllowRedefinitions->TabIndex = 9;
			this->AllowRedefinitions->Text = L"Allow Macro Redefinitions";
			this->AllowRedefinitions->UseVisualStyleBackColor = true;
			// 
			// LabelISThreshold
			// 
			this->LabelISThreshold->Location = System::Drawing::Point(30, 51);
			this->LabelISThreshold->Name = L"LabelISThreshold";
			this->LabelISThreshold->Size = System::Drawing::Size(148, 31);
			this->LabelISThreshold->TabIndex = 1;
			this->LabelISThreshold->Text = L"IntelliSense Interface Pop-up Threshold";
			// 
			// ThresholdLength
			// 
			this->ThresholdLength->Location = System::Drawing::Point(30, 104);
			this->ThresholdLength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
			this->ThresholdLength->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
			this->ThresholdLength->Name = L"ThresholdLength";
			this->ThresholdLength->Size = System::Drawing::Size(148, 20);
			this->ThresholdLength->TabIndex = 0;
			this->ThresholdLength->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
			// 
			// DestroyOnLastTabClose
			// 
			this->DestroyOnLastTabClose->AutoSize = true;
			this->DestroyOnLastTabClose->Location = System::Drawing::Point(22, 92);
			this->DestroyOnLastTabClose->Name = L"DestroyOnLastTabClose";
			this->DestroyOnLastTabClose->Size = System::Drawing::Size(181, 17);
			this->DestroyOnLastTabClose->TabIndex = 11;
			this->DestroyOnLastTabClose->Text = L"Close Editor With Last Open Tab";
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
			this->LabelTabSize->AutoSize = true;
			this->LabelTabSize->Location = System::Drawing::Point(325, 7);
			this->LabelTabSize->Name = L"LabelTabSize";
			this->LabelTabSize->Size = System::Drawing::Size(49, 13);
			this->LabelTabSize->TabIndex = 3;
			this->LabelTabSize->Text = L"Tab Size";
			this->LabelTabSize->Click += gcnew System::EventHandler(this, &OptionsDialog::label2_Click);
			// 
			// TabSize
			// 
			this->TabSize->Location = System::Drawing::Point(328, 28);
			this->TabSize->Name = L"TabSize";
			this->TabSize->Size = System::Drawing::Size(88, 20);
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
			// CmDlgFont
			// 
			this->CmDlgFont->Location = System::Drawing::Point(328, 62);
			this->CmDlgFont->Name = L"CmDlgFont";
			this->CmDlgFont->Size = System::Drawing::Size(88, 24);
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
			this->TabContainer->Controls->Add(this->TabBackup);
			this->TabContainer->Controls->Add(this->TabValidator);
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
			this->TabGeneral->Controls->Add(this->CutCopyEntireLine);
			this->TabGeneral->Controls->Add(this->LoadScriptUpdateExistingScripts);
			this->TabGeneral->Controls->Add(this->DestroyOnLastTabClose);
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
			// CutCopyEntireLine
			// 
			this->CutCopyEntireLine->AutoSize = true;
			this->CutCopyEntireLine->Location = System::Drawing::Point(22, 138);
			this->CutCopyEntireLine->Name = L"CutCopyEntireLine";
			this->CutCopyEntireLine->Size = System::Drawing::Size(246, 17);
			this->CutCopyEntireLine->TabIndex = 29;
			this->CutCopyEntireLine->Text = L"Copy/Cut Entire Line When Selection Is Empty";
			this->CutCopyEntireLine->UseVisualStyleBackColor = true;
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
			// TabIntelliSense
			// 
			this->TabIntelliSense->Controls->Add(this->SubstringSearch);
			this->TabIntelliSense->Controls->Add(this->ForceDatabaseUpdate);
			this->TabIntelliSense->Controls->Add(this->NoFocusUI);
			this->TabIntelliSense->Controls->Add(this->LabelMaxVisibleItems);
			this->TabIntelliSense->Controls->Add(this->MaxVisibleItems);
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
			// SubstringSearch
			// 
			this->SubstringSearch->Location = System::Drawing::Point(231, 238);
			this->SubstringSearch->Name = L"SubstringSearch";
			this->SubstringSearch->Size = System::Drawing::Size(167, 36);
			this->SubstringSearch->TabIndex = 11;
			this->SubstringSearch->Text = L"Use Substring Search While Filtering";
			this->SubstringSearch->UseVisualStyleBackColor = true;
			// 
			// ForceDatabaseUpdate
			// 
			this->ForceDatabaseUpdate->Location = System::Drawing::Point(30, 238);
			this->ForceDatabaseUpdate->Name = L"ForceDatabaseUpdate";
			this->ForceDatabaseUpdate->Size = System::Drawing::Size(148, 24);
			this->ForceDatabaseUpdate->TabIndex = 10;
			this->ForceDatabaseUpdate->Text = L"Update Database Now";
			this->ForceDatabaseUpdate->UseVisualStyleBackColor = true;
			// 
			// NoFocusUI
			// 
			this->NoFocusUI->Location = System::Drawing::Point(231, 78);
			this->NoFocusUI->Name = L"NoFocusUI";
			this->NoFocusUI->Size = System::Drawing::Size(167, 46);
			this->NoFocusUI->TabIndex = 9;
			this->NoFocusUI->Text = L"Prevent IntelliSense Interface From Acquiring Focus";
			this->NoFocusUI->UseVisualStyleBackColor = true;
			// 
			// LabelMaxVisibleItems
			// 
			this->LabelMaxVisibleItems->Location = System::Drawing::Point(228, 152);
			this->LabelMaxVisibleItems->Name = L"LabelMaxVisibleItems";
			this->LabelMaxVisibleItems->Size = System::Drawing::Size(170, 46);
			this->LabelMaxVisibleItems->TabIndex = 8;
			this->LabelMaxVisibleItems->Text = L"Maximum Number Of Items Visible In IntelliSense Interface";
			// 
			// MaxVisibleItems
			// 
			this->MaxVisibleItems->Location = System::Drawing::Point(231, 201);
			this->MaxVisibleItems->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 20, 0, 0, 0 });
			this->MaxVisibleItems->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			this->MaxVisibleItems->Name = L"MaxVisibleItems";
			this->MaxVisibleItems->Size = System::Drawing::Size(148, 20);
			this->MaxVisibleItems->TabIndex = 7;
			this->MaxVisibleItems->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			// 
			// UseQuickView
			// 
			this->UseQuickView->AutoSize = true;
			this->UseQuickView->Location = System::Drawing::Point(231, 51);
			this->UseQuickView->Name = L"UseQuickView";
			this->UseQuickView->Size = System::Drawing::Size(86, 17);
			this->UseQuickView->TabIndex = 6;
			this->UseQuickView->Text = L"QuickView™";
			this->UseQuickView->UseVisualStyleBackColor = true;
			// 
			// LabelISDBUpdatePeriod
			// 
			this->LabelISDBUpdatePeriod->Location = System::Drawing::Point(30, 152);
			this->LabelISDBUpdatePeriod->Name = L"LabelISDBUpdatePeriod";
			this->LabelISDBUpdatePeriod->Size = System::Drawing::Size(148, 46);
			this->LabelISDBUpdatePeriod->TabIndex = 3;
			this->LabelISDBUpdatePeriod->Text = L"IntelliSense Database Update Period (In Earth Minutes)";
			// 
			// DatabaseUpdateInterval
			// 
			this->DatabaseUpdateInterval->Location = System::Drawing::Point(30, 201);
			this->DatabaseUpdateInterval->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 15, 0, 0, 0 });
			this->DatabaseUpdateInterval->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			this->DatabaseUpdateInterval->Name = L"DatabaseUpdateInterval";
			this->DatabaseUpdateInterval->Size = System::Drawing::Size(148, 20);
			this->DatabaseUpdateInterval->TabIndex = 2;
			this->DatabaseUpdateInterval->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			this->DatabaseUpdateInterval->ValueChanged += gcnew System::EventHandler(this, &OptionsDialog::ISDBUpdateInterval_ValueChanged);
			// 
			// TabPreprocessor
			// 
			this->TabPreprocessor->Controls->Add(this->LabelNoOfPasses);
			this->TabPreprocessor->Controls->Add(this->NoOfPasses);
			this->TabPreprocessor->Controls->Add(this->AllowRedefinitions);
			this->TabPreprocessor->Location = System::Drawing::Point(4, 22);
			this->TabPreprocessor->Name = L"TabPreprocessor";
			this->TabPreprocessor->Padding = System::Windows::Forms::Padding(3);
			this->TabPreprocessor->Size = System::Drawing::Size(423, 290);
			this->TabPreprocessor->TabIndex = 2;
			this->TabPreprocessor->Text = L"Preprocessor";
			this->TabPreprocessor->UseVisualStyleBackColor = true;
			// 
			// LabelNoOfPasses
			// 
			this->LabelNoOfPasses->Location = System::Drawing::Point(133, 151);
			this->LabelNoOfPasses->Name = L"LabelNoOfPasses";
			this->LabelNoOfPasses->Size = System::Drawing::Size(157, 20);
			this->LabelNoOfPasses->TabIndex = 11;
			this->LabelNoOfPasses->Text = L"Number of Passes";
			// 
			// NoOfPasses
			// 
			this->NoOfPasses->Location = System::Drawing::Point(133, 174);
			this->NoOfPasses->Name = L"NoOfPasses";
			this->NoOfPasses->Size = System::Drawing::Size(154, 20);
			this->NoOfPasses->TabIndex = 10;
			// 
			// TabAppearance
			// 
			this->TabAppearance->Controls->Add(this->CmDlgBackgroundColor);
			this->TabAppearance->Controls->Add(this->CmDlgForegroundColor);
			this->TabAppearance->Controls->Add(this->LabelBackgroundColor);
			this->TabAppearance->Controls->Add(this->LabelForegroundColor);
			this->TabAppearance->Controls->Add(this->BoldFacedHighlighting);
			this->TabAppearance->Controls->Add(this->TabsOnTop);
			this->TabAppearance->Controls->Add(this->CodeFolding);
			this->TabAppearance->Controls->Add(this->ShowSpaces);
			this->TabAppearance->Controls->Add(this->ShowTabs);
			this->TabAppearance->Controls->Add(this->CmDlgFindResultsHighlightColor);
			this->TabAppearance->Controls->Add(this->LabelFindResultsHighlight);
			this->TabAppearance->Controls->Add(this->CmDlgCurrentLineHighlightColor);
			this->TabAppearance->Controls->Add(this->LabelCurrentLineHighlight);
			this->TabAppearance->Controls->Add(this->CmDlgCharLimitColor);
			this->TabAppearance->Controls->Add(this->LabelCharLimitHighlight);
			this->TabAppearance->Controls->Add(this->CmDlgErrorHighlightColor);
			this->TabAppearance->Controls->Add(this->LabelErrorHighlight);
			this->TabAppearance->Controls->Add(this->CmDlgSelectionHighlightColor);
			this->TabAppearance->Controls->Add(this->WordWrap);
			this->TabAppearance->Controls->Add(this->LabelSelectionHighlight);
			this->TabAppearance->Controls->Add(this->GroupBoxSyntaxHighlighting);
			this->TabAppearance->Controls->Add(this->CmDlgFont);
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
			// CmDlgBackgroundColor
			// 
			this->CmDlgBackgroundColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgBackgroundColor->Location = System::Drawing::Point(282, 259);
			this->CmDlgBackgroundColor->Name = L"CmDlgBackgroundColor";
			this->CmDlgBackgroundColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgBackgroundColor->TabIndex = 36;
			this->CmDlgBackgroundColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgForegroundColor
			// 
			this->CmDlgForegroundColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgForegroundColor->Location = System::Drawing::Point(282, 229);
			this->CmDlgForegroundColor->Name = L"CmDlgForegroundColor";
			this->CmDlgForegroundColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgForegroundColor->TabIndex = 35;
			this->CmDlgForegroundColor->UseVisualStyleBackColor = true;
			// 
			// LabelBackgroundColor
			// 
			this->LabelBackgroundColor->AutoSize = true;
			this->LabelBackgroundColor->Location = System::Drawing::Point(171, 263);
			this->LabelBackgroundColor->Name = L"LabelBackgroundColor";
			this->LabelBackgroundColor->Size = System::Drawing::Size(92, 13);
			this->LabelBackgroundColor->TabIndex = 34;
			this->LabelBackgroundColor->Text = L"Background Color";
			// 
			// LabelForegroundColor
			// 
			this->LabelForegroundColor->AutoSize = true;
			this->LabelForegroundColor->Location = System::Drawing::Point(171, 233);
			this->LabelForegroundColor->Name = L"LabelForegroundColor";
			this->LabelForegroundColor->Size = System::Drawing::Size(88, 13);
			this->LabelForegroundColor->TabIndex = 33;
			this->LabelForegroundColor->Text = L"Foreground Color";
			// 
			// BoldFacedHighlighting
			// 
			this->BoldFacedHighlighting->Location = System::Drawing::Point(328, 139);
			this->BoldFacedHighlighting->Name = L"BoldFacedHighlighting";
			this->BoldFacedHighlighting->Size = System::Drawing::Size(89, 34);
			this->BoldFacedHighlighting->TabIndex = 32;
			this->BoldFacedHighlighting->Text = L"Bold-Faced Highlighting";
			this->BoldFacedHighlighting->UseVisualStyleBackColor = true;
			// 
			// TabsOnTop
			// 
			this->TabsOnTop->AutoSize = true;
			this->TabsOnTop->Location = System::Drawing::Point(328, 262);
			this->TabsOnTop->Name = L"TabsOnTop";
			this->TabsOnTop->Size = System::Drawing::Size(89, 17);
			this->TabsOnTop->TabIndex = 31;
			this->TabsOnTop->Text = L"Tabs On Top";
			this->TabsOnTop->UseVisualStyleBackColor = true;
			// 
			// CodeFolding
			// 
			this->CodeFolding->AutoSize = true;
			this->CodeFolding->Location = System::Drawing::Point(328, 240);
			this->CodeFolding->Name = L"CodeFolding";
			this->CodeFolding->Size = System::Drawing::Size(88, 17);
			this->CodeFolding->TabIndex = 30;
			this->CodeFolding->Text = L"Code Folding";
			this->CodeFolding->UseVisualStyleBackColor = true;
			// 
			// ShowSpaces
			// 
			this->ShowSpaces->AutoSize = true;
			this->ShowSpaces->Location = System::Drawing::Point(328, 218);
			this->ShowSpaces->Name = L"ShowSpaces";
			this->ShowSpaces->Size = System::Drawing::Size(92, 17);
			this->ShowSpaces->TabIndex = 29;
			this->ShowSpaces->Text = L"Show Spaces";
			this->ShowSpaces->UseVisualStyleBackColor = true;
			// 
			// ShowTabs
			// 
			this->ShowTabs->AutoSize = true;
			this->ShowTabs->Location = System::Drawing::Point(328, 196);
			this->ShowTabs->Name = L"ShowTabs";
			this->ShowTabs->Size = System::Drawing::Size(80, 17);
			this->ShowTabs->TabIndex = 28;
			this->ShowTabs->Text = L"Show Tabs";
			this->ShowTabs->UseVisualStyleBackColor = true;
			// 
			// CmDlgFindResultsHighlightColor
			// 
			this->CmDlgFindResultsHighlightColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgFindResultsHighlightColor->Location = System::Drawing::Point(282, 136);
			this->CmDlgFindResultsHighlightColor->Name = L"CmDlgFindResultsHighlightColor";
			this->CmDlgFindResultsHighlightColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgFindResultsHighlightColor->TabIndex = 27;
			this->CmDlgFindResultsHighlightColor->UseVisualStyleBackColor = true;
			// 
			// LabelFindResultsHighlight
			// 
			this->LabelFindResultsHighlight->Location = System::Drawing::Point(171, 135);
			this->LabelFindResultsHighlight->Name = L"LabelFindResultsHighlight";
			this->LabelFindResultsHighlight->Size = System::Drawing::Size(108, 33);
			this->LabelFindResultsHighlight->TabIndex = 26;
			this->LabelFindResultsHighlight->Text = L"Find Results Highlight";
			// 
			// CmDlgCurrentLineHighlightColor
			// 
			this->CmDlgCurrentLineHighlightColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgCurrentLineHighlightColor->Location = System::Drawing::Point(282, 39);
			this->CmDlgCurrentLineHighlightColor->Name = L"CmDlgCurrentLineHighlightColor";
			this->CmDlgCurrentLineHighlightColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgCurrentLineHighlightColor->TabIndex = 25;
			this->CmDlgCurrentLineHighlightColor->UseVisualStyleBackColor = true;
			// 
			// LabelCurrentLineHighlight
			// 
			this->LabelCurrentLineHighlight->Location = System::Drawing::Point(171, 39);
			this->LabelCurrentLineHighlight->Name = L"LabelCurrentLineHighlight";
			this->LabelCurrentLineHighlight->Size = System::Drawing::Size(108, 20);
			this->LabelCurrentLineHighlight->TabIndex = 24;
			this->LabelCurrentLineHighlight->Text = L"Current Line Highlight";
			// 
			// CmDlgCharLimitColor
			// 
			this->CmDlgCharLimitColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgCharLimitColor->Location = System::Drawing::Point(282, 72);
			this->CmDlgCharLimitColor->Name = L"CmDlgCharLimitColor";
			this->CmDlgCharLimitColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgCharLimitColor->TabIndex = 23;
			this->CmDlgCharLimitColor->UseVisualStyleBackColor = true;
			// 
			// LabelCharLimitHighlight
			// 
			this->LabelCharLimitHighlight->Location = System::Drawing::Point(171, 68);
			this->LabelCharLimitHighlight->Name = L"LabelCharLimitHighlight";
			this->LabelCharLimitHighlight->Size = System::Drawing::Size(108, 30);
			this->LabelCharLimitHighlight->TabIndex = 22;
			this->LabelCharLimitHighlight->Text = L"Character Limit Highlight";
			// 
			// CmDlgErrorHighlightColor
			// 
			this->CmDlgErrorHighlightColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgErrorHighlightColor->Location = System::Drawing::Point(282, 104);
			this->CmDlgErrorHighlightColor->Name = L"CmDlgErrorHighlightColor";
			this->CmDlgErrorHighlightColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgErrorHighlightColor->TabIndex = 21;
			this->CmDlgErrorHighlightColor->UseVisualStyleBackColor = true;
			this->CmDlgErrorHighlightColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgErrorHighlightColor_Click);
			// 
			// LabelErrorHighlight
			// 
			this->LabelErrorHighlight->Location = System::Drawing::Point(171, 107);
			this->LabelErrorHighlight->Name = L"LabelErrorHighlight";
			this->LabelErrorHighlight->Size = System::Drawing::Size(108, 20);
			this->LabelErrorHighlight->TabIndex = 20;
			this->LabelErrorHighlight->Text = L"Error Highlight";
			// 
			// CmDlgSelectionHighlightColor
			// 
			this->CmDlgSelectionHighlightColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgSelectionHighlightColor->Location = System::Drawing::Point(282, 7);
			this->CmDlgSelectionHighlightColor->Name = L"CmDlgSelectionHighlightColor";
			this->CmDlgSelectionHighlightColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgSelectionHighlightColor->TabIndex = 19;
			this->CmDlgSelectionHighlightColor->UseVisualStyleBackColor = true;
			// 
			// WordWrap
			// 
			this->WordWrap->AutoSize = true;
			this->WordWrap->Location = System::Drawing::Point(328, 174);
			this->WordWrap->Name = L"WordWrap";
			this->WordWrap->Size = System::Drawing::Size(81, 17);
			this->WordWrap->TabIndex = 5;
			this->WordWrap->Text = L"Word-Wrap";
			this->WordWrap->UseVisualStyleBackColor = true;
			// 
			// LabelSelectionHighlight
			// 
			this->LabelSelectionHighlight->Location = System::Drawing::Point(171, 10);
			this->LabelSelectionHighlight->Name = L"LabelSelectionHighlight";
			this->LabelSelectionHighlight->Size = System::Drawing::Size(108, 20);
			this->LabelSelectionHighlight->TabIndex = 18;
			this->LabelSelectionHighlight->Text = L"Selection Highlight";
			// 
			// GroupBoxSyntaxHighlighting
			// 
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->CmDlgSyntaxLocalVarsColor);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->LabelLocalVars);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->CmDlgSyntaxCommentsColor);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->CmDlgSyntaxDigitsColor);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->CmDlgSyntaxPreprocessorColor);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->CmDlgSyntaxScriptBlocksColor);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->CmDlgSyntaxDelimitersColor);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->CmDlgSyntaxStringsColor);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->CmDlgSyntaxKeywordsColor);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->LabelPreprocessor);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->LabelScriptBlocks);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->LabelStrings);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->LabelComments);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->LabelDigits);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->LabelDelimiters);
			this->GroupBoxSyntaxHighlighting->Controls->Add(this->LabelKeywords);
			this->GroupBoxSyntaxHighlighting->Location = System::Drawing::Point(6, 3);
			this->GroupBoxSyntaxHighlighting->Name = L"GroupBoxSyntaxHighlighting";
			this->GroupBoxSyntaxHighlighting->Size = System::Drawing::Size(155, 281);
			this->GroupBoxSyntaxHighlighting->TabIndex = 4;
			this->GroupBoxSyntaxHighlighting->TabStop = false;
			this->GroupBoxSyntaxHighlighting->Text = L"Syntax Highlighting";
			// 
			// CmDlgSyntaxCommentsColor
			// 
			this->CmDlgSyntaxCommentsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgSyntaxCommentsColor->Location = System::Drawing::Point(103, 211);
			this->CmDlgSyntaxCommentsColor->Name = L"CmDlgSyntaxCommentsColor";
			this->CmDlgSyntaxCommentsColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgSyntaxCommentsColor->TabIndex = 18;
			this->CmDlgSyntaxCommentsColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgSyntaxDigitsColor
			// 
			this->CmDlgSyntaxDigitsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgSyntaxDigitsColor->Location = System::Drawing::Point(103, 50);
			this->CmDlgSyntaxDigitsColor->Name = L"CmDlgSyntaxDigitsColor";
			this->CmDlgSyntaxDigitsColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgSyntaxDigitsColor->TabIndex = 17;
			this->CmDlgSyntaxDigitsColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgSyntaxPreprocessorColor
			// 
			this->CmDlgSyntaxPreprocessorColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgSyntaxPreprocessorColor->Location = System::Drawing::Point(103, 82);
			this->CmDlgSyntaxPreprocessorColor->Name = L"CmDlgSyntaxPreprocessorColor";
			this->CmDlgSyntaxPreprocessorColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgSyntaxPreprocessorColor->TabIndex = 16;
			this->CmDlgSyntaxPreprocessorColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgSyntaxScriptBlocksColor
			// 
			this->CmDlgSyntaxScriptBlocksColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgSyntaxScriptBlocksColor->Location = System::Drawing::Point(103, 113);
			this->CmDlgSyntaxScriptBlocksColor->Name = L"CmDlgSyntaxScriptBlocksColor";
			this->CmDlgSyntaxScriptBlocksColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgSyntaxScriptBlocksColor->TabIndex = 15;
			this->CmDlgSyntaxScriptBlocksColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgSyntaxDelimitersColor
			// 
			this->CmDlgSyntaxDelimitersColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgSyntaxDelimitersColor->Location = System::Drawing::Point(103, 146);
			this->CmDlgSyntaxDelimitersColor->Name = L"CmDlgSyntaxDelimitersColor";
			this->CmDlgSyntaxDelimitersColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgSyntaxDelimitersColor->TabIndex = 14;
			this->CmDlgSyntaxDelimitersColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgSyntaxStringsColor
			// 
			this->CmDlgSyntaxStringsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgSyntaxStringsColor->Location = System::Drawing::Point(103, 178);
			this->CmDlgSyntaxStringsColor->Name = L"CmDlgSyntaxStringsColor";
			this->CmDlgSyntaxStringsColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgSyntaxStringsColor->TabIndex = 13;
			this->CmDlgSyntaxStringsColor->UseVisualStyleBackColor = true;
			// 
			// CmDlgSyntaxKeywordsColor
			// 
			this->CmDlgSyntaxKeywordsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgSyntaxKeywordsColor->Location = System::Drawing::Point(103, 18);
			this->CmDlgSyntaxKeywordsColor->Name = L"CmDlgSyntaxKeywordsColor";
			this->CmDlgSyntaxKeywordsColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgSyntaxKeywordsColor->TabIndex = 12;
			this->CmDlgSyntaxKeywordsColor->UseVisualStyleBackColor = true;
			// 
			// LabelPreprocessor
			// 
			this->LabelPreprocessor->Location = System::Drawing::Point(6, 86);
			this->LabelPreprocessor->Name = L"LabelPreprocessor";
			this->LabelPreprocessor->Size = System::Drawing::Size(78, 20);
			this->LabelPreprocessor->TabIndex = 11;
			this->LabelPreprocessor->Text = L"Preprocessor";
			// 
			// LabelScriptBlocks
			// 
			this->LabelScriptBlocks->Location = System::Drawing::Point(6, 119);
			this->LabelScriptBlocks->Name = L"LabelScriptBlocks";
			this->LabelScriptBlocks->Size = System::Drawing::Size(78, 20);
			this->LabelScriptBlocks->TabIndex = 10;
			this->LabelScriptBlocks->Text = L"Script Blocks";
			// 
			// LabelStrings
			// 
			this->LabelStrings->Location = System::Drawing::Point(6, 182);
			this->LabelStrings->Name = L"LabelStrings";
			this->LabelStrings->Size = System::Drawing::Size(78, 20);
			this->LabelStrings->TabIndex = 9;
			this->LabelStrings->Text = L"String Literals";
			// 
			// LabelComments
			// 
			this->LabelComments->Location = System::Drawing::Point(6, 215);
			this->LabelComments->Name = L"LabelComments";
			this->LabelComments->Size = System::Drawing::Size(78, 20);
			this->LabelComments->TabIndex = 8;
			this->LabelComments->Text = L"Comments";
			// 
			// LabelDigits
			// 
			this->LabelDigits->Location = System::Drawing::Point(6, 54);
			this->LabelDigits->Name = L"LabelDigits";
			this->LabelDigits->Size = System::Drawing::Size(78, 20);
			this->LabelDigits->TabIndex = 7;
			this->LabelDigits->Text = L"Digits";
			// 
			// LabelDelimiters
			// 
			this->LabelDelimiters->Location = System::Drawing::Point(6, 150);
			this->LabelDelimiters->Name = L"LabelDelimiters";
			this->LabelDelimiters->Size = System::Drawing::Size(78, 20);
			this->LabelDelimiters->TabIndex = 6;
			this->LabelDelimiters->Text = L"Delimiters";
			// 
			// LabelKeywords
			// 
			this->LabelKeywords->Location = System::Drawing::Point(6, 22);
			this->LabelKeywords->Name = L"LabelKeywords";
			this->LabelKeywords->Size = System::Drawing::Size(78, 20);
			this->LabelKeywords->TabIndex = 5;
			this->LabelKeywords->Text = L"Keywords";
			// 
			// TabSanitize
			// 
			this->TabSanitize->Controls->Add(this->EvalifyIfs);
			this->TabSanitize->Controls->Add(this->CompilerOverrideBlocks);
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
			// EvalifyIfs
			// 
			this->EvalifyIfs->AutoSize = true;
			this->EvalifyIfs->Location = System::Drawing::Point(109, 156);
			this->EvalifyIfs->Name = L"EvalifyIfs";
			this->EvalifyIfs->Size = System::Drawing::Size(155, 17);
			this->EvalifyIfs->TabIndex = 14;
			this->EvalifyIfs->Text = L"Eval\'ify If/ElseIf Statements";
			this->EvalifyIfs->UseVisualStyleBackColor = true;
			// 
			// CompilerOverrideBlocks
			// 
			this->CompilerOverrideBlocks->AutoSize = true;
			this->CompilerOverrideBlocks->Location = System::Drawing::Point(109, 195);
			this->CompilerOverrideBlocks->Name = L"CompilerOverrideBlocks";
			this->CompilerOverrideBlocks->Size = System::Drawing::Size(219, 17);
			this->CompilerOverrideBlocks->TabIndex = 13;
			this->CompilerOverrideBlocks->Text = L"Apply Compiler Override To Script Blocks";
			this->CompilerOverrideBlocks->UseVisualStyleBackColor = true;
			// 
			// IndentLines
			// 
			this->IndentLines->AutoSize = true;
			this->IndentLines->Location = System::Drawing::Point(109, 117);
			this->IndentLines->Name = L"IndentLines";
			this->IndentLines->Size = System::Drawing::Size(114, 17);
			this->IndentLines->TabIndex = 12;
			this->IndentLines->Text = L"Indent Script Lines";
			this->IndentLines->UseVisualStyleBackColor = true;
			// 
			// AnnealCasing
			// 
			this->AnnealCasing->AutoSize = true;
			this->AnnealCasing->Location = System::Drawing::Point(109, 78);
			this->AnnealCasing->Name = L"AnnealCasing";
			this->AnnealCasing->Size = System::Drawing::Size(205, 17);
			this->AnnealCasing->TabIndex = 11;
			this->AnnealCasing->Text = L"Anneal Script Command Name Casing";
			this->AnnealCasing->UseVisualStyleBackColor = true;
			// 
			// TabBackup
			// 
			this->TabBackup->Controls->Add(this->LabelAutoRecoveryInterval);
			this->TabBackup->Controls->Add(this->AutoRecoverySavePeriod);
			this->TabBackup->Controls->Add(this->UseAutoRecovery);
			this->TabBackup->Location = System::Drawing::Point(4, 22);
			this->TabBackup->Name = L"TabBackup";
			this->TabBackup->Padding = System::Windows::Forms::Padding(3);
			this->TabBackup->Size = System::Drawing::Size(423, 290);
			this->TabBackup->TabIndex = 5;
			this->TabBackup->Text = L"Backup";
			this->TabBackup->UseVisualStyleBackColor = true;
			// 
			// LabelAutoRecoveryInterval
			// 
			this->LabelAutoRecoveryInterval->Location = System::Drawing::Point(133, 139);
			this->LabelAutoRecoveryInterval->Name = L"LabelAutoRecoveryInterval";
			this->LabelAutoRecoveryInterval->Size = System::Drawing::Size(157, 34);
			this->LabelAutoRecoveryInterval->TabIndex = 14;
			this->LabelAutoRecoveryInterval->Text = L"Auto-Recovery Save Period (In Earth Minutes)";
			// 
			// AutoRecoverySavePeriod
			// 
			this->AutoRecoverySavePeriod->Location = System::Drawing::Point(133, 176);
			this->AutoRecoverySavePeriod->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 60, 0, 0, 0 });
			this->AutoRecoverySavePeriod->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->AutoRecoverySavePeriod->Name = L"AutoRecoverySavePeriod";
			this->AutoRecoverySavePeriod->Size = System::Drawing::Size(154, 20);
			this->AutoRecoverySavePeriod->TabIndex = 13;
			this->AutoRecoverySavePeriod->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			// 
			// UseAutoRecovery
			// 
			this->UseAutoRecovery->Location = System::Drawing::Point(133, 95);
			this->UseAutoRecovery->Name = L"UseAutoRecovery";
			this->UseAutoRecovery->Size = System::Drawing::Size(154, 23);
			this->UseAutoRecovery->TabIndex = 12;
			this->UseAutoRecovery->Text = L"Use Auto-Recovery";
			this->UseAutoRecovery->UseVisualStyleBackColor = true;
			// 
			// TabValidator
			// 
			this->TabValidator->Controls->Add(this->VarCmdNameCollisions);
			this->TabValidator->Controls->Add(this->CountVarRefs);
			this->TabValidator->Controls->Add(this->SuppressRefCountForQuestScripts);
			this->TabValidator->Controls->Add(this->VarFormNameCollisions);
			this->TabValidator->Location = System::Drawing::Point(4, 22);
			this->TabValidator->Name = L"TabValidator";
			this->TabValidator->Padding = System::Windows::Forms::Padding(3);
			this->TabValidator->Size = System::Drawing::Size(423, 290);
			this->TabValidator->TabIndex = 6;
			this->TabValidator->Text = L"Validator";
			this->TabValidator->UseVisualStyleBackColor = true;
			// 
			// VarCmdNameCollisions
			// 
			this->VarCmdNameCollisions->AutoSize = true;
			this->VarCmdNameCollisions->Location = System::Drawing::Point(24, 51);
			this->VarCmdNameCollisions->Name = L"VarCmdNameCollisions";
			this->VarCmdNameCollisions->Size = System::Drawing::Size(324, 17);
			this->VarCmdNameCollisions->TabIndex = 15;
			this->VarCmdNameCollisions->Text = L"Check For Variable-Command Name Collisions (Recommended)";
			this->VarCmdNameCollisions->UseVisualStyleBackColor = true;
			// 
			// CountVarRefs
			// 
			this->CountVarRefs->AutoSize = true;
			this->CountVarRefs->Location = System::Drawing::Point(24, 74);
			this->CountVarRefs->Name = L"CountVarRefs";
			this->CountVarRefs->Size = System::Drawing::Size(175, 17);
			this->CountVarRefs->TabIndex = 14;
			this->CountVarRefs->Text = L"Count Variable Use References";
			this->CountVarRefs->UseVisualStyleBackColor = true;
			// 
			// SuppressRefCountForQuestScripts
			// 
			this->SuppressRefCountForQuestScripts->AutoSize = true;
			this->SuppressRefCountForQuestScripts->Location = System::Drawing::Point(24, 255);
			this->SuppressRefCountForQuestScripts->Name = L"SuppressRefCountForQuestScripts";
			this->SuppressRefCountForQuestScripts->Size = System::Drawing::Size(293, 17);
			this->SuppressRefCountForQuestScripts->TabIndex = 13;
			this->SuppressRefCountForQuestScripts->Text = L"Suppress Variable Reference Counting For Quest Scripts";
			this->SuppressRefCountForQuestScripts->UseVisualStyleBackColor = true;
			// 
			// VarFormNameCollisions
			// 
			this->VarFormNameCollisions->AutoSize = true;
			this->VarFormNameCollisions->Location = System::Drawing::Point(24, 28);
			this->VarFormNameCollisions->Name = L"VarFormNameCollisions";
			this->VarFormNameCollisions->Size = System::Drawing::Size(300, 17);
			this->VarFormNameCollisions->TabIndex = 0;
			this->VarFormNameCollisions->Text = L"Check For Variable-Form Name Collisions (Recommended)";
			this->VarFormNameCollisions->UseVisualStyleBackColor = true;
			// 
			// LabelLocalVars
			// 
			this->LabelLocalVars->Location = System::Drawing::Point(6, 247);
			this->LabelLocalVars->Name = L"LabelLocalVars";
			this->LabelLocalVars->Size = System::Drawing::Size(91, 25);
			this->LabelLocalVars->TabIndex = 19;
			this->LabelLocalVars->Text = L"Local Variables";
			// 
			// CmDlgSyntaxLocalVarsColor
			// 
			this->CmDlgSyntaxLocalVarsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->CmDlgSyntaxLocalVarsColor->Location = System::Drawing::Point(103, 243);
			this->CmDlgSyntaxLocalVarsColor->Name = L"CmDlgSyntaxLocalVarsColor";
			this->CmDlgSyntaxLocalVarsColor->Size = System::Drawing::Size(34, 21);
			this->CmDlgSyntaxLocalVarsColor->TabIndex = 20;
			this->CmDlgSyntaxLocalVarsColor->UseVisualStyleBackColor = true;
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
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ThresholdLength))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->TabSize))->EndInit();
			this->TabContainer->ResumeLayout(false);
			this->TabGeneral->ResumeLayout(false);
			this->TabGeneral->PerformLayout();
			this->TabIntelliSense->ResumeLayout(false);
			this->TabIntelliSense->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->MaxVisibleItems))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->DatabaseUpdateInterval))->EndInit();
			this->TabPreprocessor->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->NoOfPasses))->EndInit();
			this->TabAppearance->ResumeLayout(false);
			this->TabAppearance->PerformLayout();
			this->GroupBoxSyntaxHighlighting->ResumeLayout(false);
			this->TabSanitize->ResumeLayout(false);
			this->TabSanitize->PerformLayout();
			this->TabBackup->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->AutoRecoverySavePeriod))->EndInit();
			this->TabValidator->ResumeLayout(false);
			this->TabValidator->PerformLayout();
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
private: System::Void CmDlgErrorHighlightColor_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
};
}