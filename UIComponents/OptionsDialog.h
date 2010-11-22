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
	private: System::Windows::Forms::GroupBox^  PreProcGrup;
	private: System::Windows::Forms::GroupBox^  IntelliSenseGrup;
	private: System::Windows::Forms::GroupBox^  generalGrup;
	private: System::Windows::Forms::Button^  highlightcolor;
	protected: 





	private: System::Windows::Forms::Button^  backcolor;

	private: System::Windows::Forms::Button^  forecolor;

	private: System::Windows::Forms::Button^  font;
	private: System::Windows::Forms::Button^  bookmarkcolor;
	private: System::Windows::Forms::CheckBox^  autoindent;
	private: System::Windows::Forms::CheckBox^  caretpos;




	private: System::Windows::Forms::CheckBox^  painteditor;
	private: System::Windows::Forms::CheckBox^  regex;


	private: System::Windows::Forms::CheckBox^  Redefs;

	private: System::Windows::Forms::CheckBox^  ImprotSeg;
	private: System::Windows::Forms::Label^  thresholdLabel;
	private: System::Windows::Forms::NumericUpDown^  ThresholdBox;
	private: System::Windows::Forms::Label^  tabsizelabel;
	private: System::Windows::Forms::NumericUpDown^  tabsizebox;
	private: System::Windows::Forms::CheckBox^  RecompileVarIdx;
	private: System::Windows::Forms::CheckBox^  PreprocessorWarnings;
	private: System::Windows::Forms::CheckBox^  UseCSParent;
	private: System::Windows::Forms::CheckBox^  DestroyOnLastTabClose;










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
			this->PreProcGrup = (gcnew System::Windows::Forms::GroupBox());
			this->PreprocessorWarnings = (gcnew System::Windows::Forms::CheckBox());
			this->Redefs = (gcnew System::Windows::Forms::CheckBox());
			this->ImprotSeg = (gcnew System::Windows::Forms::CheckBox());
			this->IntelliSenseGrup = (gcnew System::Windows::Forms::GroupBox());
			this->thresholdLabel = (gcnew System::Windows::Forms::Label());
			this->ThresholdBox = (gcnew System::Windows::Forms::NumericUpDown());
			this->generalGrup = (gcnew System::Windows::Forms::GroupBox());
			this->UseCSParent = (gcnew System::Windows::Forms::CheckBox());
			this->RecompileVarIdx = (gcnew System::Windows::Forms::CheckBox());
			this->tabsizelabel = (gcnew System::Windows::Forms::Label());
			this->tabsizebox = (gcnew System::Windows::Forms::NumericUpDown());
			this->caretpos = (gcnew System::Windows::Forms::CheckBox());
			this->painteditor = (gcnew System::Windows::Forms::CheckBox());
			this->regex = (gcnew System::Windows::Forms::CheckBox());
			this->autoindent = (gcnew System::Windows::Forms::CheckBox());
			this->bookmarkcolor = (gcnew System::Windows::Forms::Button());
			this->highlightcolor = (gcnew System::Windows::Forms::Button());
			this->backcolor = (gcnew System::Windows::Forms::Button());
			this->forecolor = (gcnew System::Windows::Forms::Button());
			this->font = (gcnew System::Windows::Forms::Button());
			this->DestroyOnLastTabClose = (gcnew System::Windows::Forms::CheckBox());
			this->PreProcGrup->SuspendLayout();
			this->IntelliSenseGrup->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ThresholdBox))->BeginInit();
			this->generalGrup->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->tabsizebox))->BeginInit();
			this->SuspendLayout();
			// 
			// PreProcGrup
			// 
			this->PreProcGrup->Controls->Add(this->PreprocessorWarnings);
			this->PreProcGrup->Controls->Add(this->Redefs);
			this->PreProcGrup->Controls->Add(this->ImprotSeg);
			this->PreProcGrup->Location = System::Drawing::Point(12, 16);
			this->PreProcGrup->Name = L"PreProcGrup";
			this->PreProcGrup->Size = System::Drawing::Size(240, 162);
			this->PreProcGrup->TabIndex = 0;
			this->PreProcGrup->TabStop = false;
			this->PreProcGrup->Text = L"Preprocessor";
			this->PreProcGrup->Enter += gcnew System::EventHandler(this, &OptionsDialog::groupBox1_Enter);
			// 
			// PreprocessorWarnings
			// 
			this->PreprocessorWarnings->AutoSize = true;
			this->PreprocessorWarnings->Location = System::Drawing::Point(13, 111);
			this->PreprocessorWarnings->Name = L"PreprocessorWarnings";
			this->PreprocessorWarnings->Size = System::Drawing::Size(130, 17);
			this->PreprocessorWarnings->TabIndex = 10;
			this->PreprocessorWarnings->Text = L"Show Script Warnings";
			this->PreprocessorWarnings->UseVisualStyleBackColor = true;
			// 
			// Redefs
			// 
			this->Redefs->AutoSize = true;
			this->Redefs->Location = System::Drawing::Point(13, 79);
			this->Redefs->Name = L"Redefs";
			this->Redefs->Size = System::Drawing::Size(141, 17);
			this->Redefs->TabIndex = 9;
			this->Redefs->Text = L"Allow macro redefinitions";
			this->Redefs->UseVisualStyleBackColor = true;
			// 
			// ImprotSeg
			// 
			this->ImprotSeg->Location = System::Drawing::Point(14, 30);
			this->ImprotSeg->Name = L"ImprotSeg";
			this->ImprotSeg->Size = System::Drawing::Size(167, 43);
			this->ImprotSeg->TabIndex = 10;
			this->ImprotSeg->Text = L"Create missing import segments from script code";
			this->ImprotSeg->UseVisualStyleBackColor = true;
			// 
			// IntelliSenseGrup
			// 
			this->IntelliSenseGrup->Controls->Add(this->thresholdLabel);
			this->IntelliSenseGrup->Controls->Add(this->ThresholdBox);
			this->IntelliSenseGrup->Location = System::Drawing::Point(258, 16);
			this->IntelliSenseGrup->Name = L"IntelliSenseGrup";
			this->IntelliSenseGrup->Size = System::Drawing::Size(243, 162);
			this->IntelliSenseGrup->TabIndex = 1;
			this->IntelliSenseGrup->TabStop = false;
			this->IntelliSenseGrup->Text = L"IntelliSense";
			// 
			// thresholdLabel
			// 
			this->thresholdLabel->AutoSize = true;
			this->thresholdLabel->Location = System::Drawing::Point(18, 30);
			this->thresholdLabel->Name = L"thresholdLabel";
			this->thresholdLabel->Size = System::Drawing::Size(148, 13);
			this->thresholdLabel->TabIndex = 1;
			this->thresholdLabel->Text = L"IntelliSense Pop-up Threshold";
			// 
			// ThresholdBox
			// 
			this->ThresholdBox->Location = System::Drawing::Point(21, 53);
			this->ThresholdBox->Name = L"ThresholdBox";
			this->ThresholdBox->Size = System::Drawing::Size(201, 20);
			this->ThresholdBox->TabIndex = 0;
			// 
			// generalGrup
			// 
			this->generalGrup->Controls->Add(this->DestroyOnLastTabClose);
			this->generalGrup->Controls->Add(this->UseCSParent);
			this->generalGrup->Controls->Add(this->RecompileVarIdx);
			this->generalGrup->Controls->Add(this->tabsizelabel);
			this->generalGrup->Controls->Add(this->tabsizebox);
			this->generalGrup->Controls->Add(this->caretpos);
			this->generalGrup->Controls->Add(this->painteditor);
			this->generalGrup->Controls->Add(this->regex);
			this->generalGrup->Controls->Add(this->autoindent);
			this->generalGrup->Controls->Add(this->bookmarkcolor);
			this->generalGrup->Controls->Add(this->highlightcolor);
			this->generalGrup->Controls->Add(this->backcolor);
			this->generalGrup->Controls->Add(this->forecolor);
			this->generalGrup->Controls->Add(this->font);
			this->generalGrup->Location = System::Drawing::Point(12, 184);
			this->generalGrup->Name = L"generalGrup";
			this->generalGrup->Size = System::Drawing::Size(488, 263);
			this->generalGrup->TabIndex = 2;
			this->generalGrup->TabStop = false;
			this->generalGrup->Text = L"General";
			// 
			// UseCSParent
			// 
			this->UseCSParent->AutoSize = true;
			this->UseCSParent->Location = System::Drawing::Point(324, 114);
			this->UseCSParent->Name = L"UseCSParent";
			this->UseCSParent->Size = System::Drawing::Size(109, 17);
			this->UseCSParent->TabIndex = 10;
			this->UseCSParent->Text = L"Use CS as Parent";
			this->UseCSParent->UseVisualStyleBackColor = true;
			// 
			// RecompileVarIdx
			// 
			this->RecompileVarIdx->AutoSize = true;
			this->RecompileVarIdx->Location = System::Drawing::Point(15, 114);
			this->RecompileVarIdx->Name = L"RecompileVarIdx";
			this->RecompileVarIdx->Size = System::Drawing::Size(272, 17);
			this->RecompileVarIdx->TabIndex = 9;
			this->RecompileVarIdx->Text = L"Recompile dependencies post variable index update";
			this->RecompileVarIdx->UseVisualStyleBackColor = true;
			// 
			// tabsizelabel
			// 
			this->tabsizelabel->AutoSize = true;
			this->tabsizelabel->Location = System::Drawing::Point(11, 202);
			this->tabsizelabel->Name = L"tabsizelabel";
			this->tabsizelabel->Size = System::Drawing::Size(101, 13);
			this->tabsizelabel->TabIndex = 3;
			this->tabsizelabel->Text = L"Tab Size ( in pixels )";
			this->tabsizelabel->Click += gcnew System::EventHandler(this, &OptionsDialog::label2_Click);
			// 
			// tabsizebox
			// 
			this->tabsizebox->Location = System::Drawing::Point(14, 225);
			this->tabsizebox->Name = L"tabsizebox";
			this->tabsizebox->Size = System::Drawing::Size(98, 20);
			this->tabsizebox->TabIndex = 2;
			// 
			// caretpos
			// 
			this->caretpos->AutoSize = true;
			this->caretpos->Location = System::Drawing::Point(14, 91);
			this->caretpos->Name = L"caretpos";
			this->caretpos->Size = System::Drawing::Size(166, 17);
			this->caretpos->TabIndex = 8;
			this->caretpos->Text = L"Save caret position with script";
			this->caretpos->UseVisualStyleBackColor = true;
			// 
			// painteditor
			// 
			this->painteditor->AutoSize = true;
			this->painteditor->Location = System::Drawing::Point(324, 68);
			this->painteditor->Name = L"painteditor";
			this->painteditor->Size = System::Drawing::Size(121, 17);
			this->painteditor->TabIndex = 7;
			this->painteditor->Text = L"Paint Editor Window";
			this->painteditor->UseVisualStyleBackColor = true;
			// 
			// regex
			// 
			this->regex->AutoSize = true;
			this->regex->Location = System::Drawing::Point(324, 91);
			this->regex->Name = L"regex";
			this->regex->Size = System::Drawing::Size(143, 17);
			this->regex->TabIndex = 6;
			this->regex->Text = L"Use Regular Expressions";
			this->regex->UseVisualStyleBackColor = true;
			// 
			// autoindent
			// 
			this->autoindent->AutoSize = true;
			this->autoindent->Location = System::Drawing::Point(14, 68);
			this->autoindent->Name = L"autoindent";
			this->autoindent->Size = System::Drawing::Size(80, 17);
			this->autoindent->TabIndex = 5;
			this->autoindent->Text = L"Auto-Indent";
			this->autoindent->UseVisualStyleBackColor = true;
			// 
			// bookmarkcolor
			// 
			this->bookmarkcolor->Location = System::Drawing::Point(273, 19);
			this->bookmarkcolor->Name = L"bookmarkcolor";
			this->bookmarkcolor->Size = System::Drawing::Size(103, 29);
			this->bookmarkcolor->TabIndex = 4;
			this->bookmarkcolor->Text = L"Bookmark Color";
			this->bookmarkcolor->UseVisualStyleBackColor = true;
			// 
			// highlightcolor
			// 
			this->highlightcolor->Location = System::Drawing::Point(379, 19);
			this->highlightcolor->Name = L"highlightcolor";
			this->highlightcolor->Size = System::Drawing::Size(103, 29);
			this->highlightcolor->TabIndex = 3;
			this->highlightcolor->Text = L"Highlight Color";
			this->highlightcolor->UseVisualStyleBackColor = true;
			// 
			// backcolor
			// 
			this->backcolor->Location = System::Drawing::Point(184, 19);
			this->backcolor->Name = L"backcolor";
			this->backcolor->Size = System::Drawing::Size(83, 29);
			this->backcolor->TabIndex = 2;
			this->backcolor->Text = L"Back Color";
			this->backcolor->UseVisualStyleBackColor = true;
			// 
			// forecolor
			// 
			this->forecolor->Location = System::Drawing::Point(95, 19);
			this->forecolor->Name = L"forecolor";
			this->forecolor->Size = System::Drawing::Size(83, 29);
			this->forecolor->TabIndex = 1;
			this->forecolor->Text = L"Fore Color";
			this->forecolor->UseVisualStyleBackColor = true;
			// 
			// font
			// 
			this->font->Location = System::Drawing::Point(6, 19);
			this->font->Name = L"font";
			this->font->Size = System::Drawing::Size(83, 29);
			this->font->TabIndex = 0;
			this->font->Text = L"Font";
			this->font->UseVisualStyleBackColor = true;
			this->font->Click += gcnew System::EventHandler(this, &OptionsDialog::button1_Click);
			// 
			// DestroyOnLastTabClose
			// 
			this->DestroyOnLastTabClose->AutoSize = true;
			this->DestroyOnLastTabClose->Location = System::Drawing::Point(15, 137);
			this->DestroyOnLastTabClose->Name = L"DestroyOnLastTabClose";
			this->DestroyOnLastTabClose->Size = System::Drawing::Size(224, 17);
			this->DestroyOnLastTabClose->TabIndex = 11;
			this->DestroyOnLastTabClose->Text = L"Close script window on closing the last tab";
			this->DestroyOnLastTabClose->UseVisualStyleBackColor = true;
			// 
			// OptionsDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(513, 459);
			this->Controls->Add(this->generalGrup);
			this->Controls->Add(this->IntelliSenseGrup);
			this->Controls->Add(this->PreProcGrup);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = L"OptionsDialog";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"OptionsDialog";
			this->PreProcGrup->ResumeLayout(false);
			this->PreProcGrup->PerformLayout();
			this->IntelliSenseGrup->ResumeLayout(false);
			this->IntelliSenseGrup->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ThresholdBox))->EndInit();
			this->generalGrup->ResumeLayout(false);
			this->generalGrup->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->tabsizebox))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void groupBox1_Enter(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
private: System::Void label2_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
};
}
