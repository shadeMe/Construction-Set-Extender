#pragma once

namespace UIComponents {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SEFindReplace
	/// </summary>
	public ref class SEFindReplace : public System::Windows::Forms::Form
	{
	public:
		SEFindReplace(void)
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
		~SEFindReplace()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Label^  LabelFind;
	private: System::Windows::Forms::Label^  LabelReplace;
	private: System::Windows::Forms::ComboBox^  QueryBox;
	private: System::Windows::Forms::ComboBox^  ReplacementBox;
	private: System::Windows::Forms::Button^  FindButton;
	private: System::Windows::Forms::Button^  ReplaceButton;
	private: System::Windows::Forms::Button^  FindInTabsButton;
	private: System::Windows::Forms::Button^  ReplaceInTabsButton;
	private: System::Windows::Forms::Button^  CountMatchesButton;
	private: System::Windows::Forms::CheckBox^  MatchWholeWord;
	private: System::Windows::Forms::CheckBox^  CaseInsensitiveSearch;
	private: System::Windows::Forms::CheckBox^  InSelection;
	private: System::Windows::Forms::CheckBox^  UseRegEx;
	protected:

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
			this->LabelFind = (gcnew System::Windows::Forms::Label());
			this->LabelReplace = (gcnew System::Windows::Forms::Label());
			this->QueryBox = (gcnew System::Windows::Forms::ComboBox());
			this->ReplacementBox = (gcnew System::Windows::Forms::ComboBox());
			this->FindButton = (gcnew System::Windows::Forms::Button());
			this->ReplaceButton = (gcnew System::Windows::Forms::Button());
			this->FindInTabsButton = (gcnew System::Windows::Forms::Button());
			this->ReplaceInTabsButton = (gcnew System::Windows::Forms::Button());
			this->CountMatchesButton = (gcnew System::Windows::Forms::Button());
			this->MatchWholeWord = (gcnew System::Windows::Forms::CheckBox());
			this->CaseInsensitiveSearch = (gcnew System::Windows::Forms::CheckBox());
			this->InSelection = (gcnew System::Windows::Forms::CheckBox());
			this->UseRegEx = (gcnew System::Windows::Forms::CheckBox());
			this->SuspendLayout();
			// 
			// LabelFind
			// 
			this->LabelFind->Location = System::Drawing::Point(12, 21);
			this->LabelFind->Name = L"LabelFind";
			this->LabelFind->Size = System::Drawing::Size(78, 13);
			this->LabelFind->TabIndex = 0;
			this->LabelFind->Text = L"Find What :";
			this->LabelFind->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// LabelReplace
			// 
			this->LabelReplace->Location = System::Drawing::Point(12, 48);
			this->LabelReplace->Name = L"LabelReplace";
			this->LabelReplace->Size = System::Drawing::Size(78, 13);
			this->LabelReplace->TabIndex = 1;
			this->LabelReplace->Text = L"Replace With :";
			this->LabelReplace->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// QueryBox
			// 
			this->QueryBox->DropDownHeight = 200;
			this->QueryBox->FormattingEnabled = true;
			this->QueryBox->IntegralHeight = false;
			this->QueryBox->Location = System::Drawing::Point(96, 18);
			this->QueryBox->Name = L"QueryBox";
			this->QueryBox->Size = System::Drawing::Size(307, 21);
			this->QueryBox->TabIndex = 2;
			// 
			// ReplacementBox
			// 
			this->ReplacementBox->DropDownHeight = 200;
			this->ReplacementBox->FormattingEnabled = true;
			this->ReplacementBox->IntegralHeight = false;
			this->ReplacementBox->Location = System::Drawing::Point(96, 45);
			this->ReplacementBox->Name = L"ReplacementBox";
			this->ReplacementBox->Size = System::Drawing::Size(307, 21);
			this->ReplacementBox->TabIndex = 3;
			// 
			// FindButton
			// 
			this->FindButton->Location = System::Drawing::Point(409, 19);
			this->FindButton->Name = L"FindButton";
			this->FindButton->Size = System::Drawing::Size(72, 21);
			this->FindButton->TabIndex = 4;
			this->FindButton->Text = L"Find All";
			this->FindButton->UseVisualStyleBackColor = true;
			// 
			// ReplaceButton
			// 
			this->ReplaceButton->Location = System::Drawing::Point(409, 46);
			this->ReplaceButton->Name = L"ReplaceButton";
			this->ReplaceButton->Size = System::Drawing::Size(72, 21);
			this->ReplaceButton->TabIndex = 5;
			this->ReplaceButton->Text = L"Replace All";
			this->ReplaceButton->UseVisualStyleBackColor = true;
			// 
			// FindInTabsButton
			// 
			this->FindInTabsButton->Location = System::Drawing::Point(487, 19);
			this->FindInTabsButton->Name = L"FindInTabsButton";
			this->FindInTabsButton->Size = System::Drawing::Size(102, 21);
			this->FindInTabsButton->TabIndex = 6;
			this->FindInTabsButton->Text = L"In All Open Scripts";
			this->FindInTabsButton->UseVisualStyleBackColor = true;
			// 
			// ReplaceInTabsButton
			// 
			this->ReplaceInTabsButton->Location = System::Drawing::Point(487, 46);
			this->ReplaceInTabsButton->Name = L"ReplaceInTabsButton";
			this->ReplaceInTabsButton->Size = System::Drawing::Size(102, 21);
			this->ReplaceInTabsButton->TabIndex = 7;
			this->ReplaceInTabsButton->Text = L"In All Open Scripts";
			this->ReplaceInTabsButton->UseVisualStyleBackColor = true;
			// 
			// CountMatchesButton
			// 
			this->CountMatchesButton->Location = System::Drawing::Point(487, 73);
			this->CountMatchesButton->Name = L"CountMatchesButton";
			this->CountMatchesButton->Size = System::Drawing::Size(102, 26);
			this->CountMatchesButton->TabIndex = 8;
			this->CountMatchesButton->Text = L"Count Matches";
			this->CountMatchesButton->UseVisualStyleBackColor = true;
			// 
			// MatchWholeWord
			// 
			this->MatchWholeWord->AutoSize = true;
			this->MatchWholeWord->Location = System::Drawing::Point(15, 73);
			this->MatchWholeWord->Name = L"MatchWholeWord";
			this->MatchWholeWord->Size = System::Drawing::Size(143, 17);
			this->MatchWholeWord->TabIndex = 9;
			this->MatchWholeWord->Text = L"Match Whole Word Only";
			this->MatchWholeWord->UseVisualStyleBackColor = true;
			// 
			// CaseInsensitiveSearch
			// 
			this->CaseInsensitiveSearch->AutoSize = true;
			this->CaseInsensitiveSearch->Location = System::Drawing::Point(15, 96);
			this->CaseInsensitiveSearch->Name = L"CaseInsensitiveSearch";
			this->CaseInsensitiveSearch->Size = System::Drawing::Size(140, 17);
			this->CaseInsensitiveSearch->TabIndex = 10;
			this->CaseInsensitiveSearch->Text = L"Case Insensitive Search";
			this->CaseInsensitiveSearch->UseVisualStyleBackColor = true;
			// 
			// InSelection
			// 
			this->InSelection->AutoSize = true;
			this->InSelection->Location = System::Drawing::Point(321, 73);
			this->InSelection->Name = L"InSelection";
			this->InSelection->Size = System::Drawing::Size(82, 17);
			this->InSelection->TabIndex = 11;
			this->InSelection->Text = L"In Selection";
			this->InSelection->UseVisualStyleBackColor = true;
			// 
			// UseRegEx
			// 
			this->UseRegEx->AutoSize = true;
			this->UseRegEx->Location = System::Drawing::Point(161, 96);
			this->UseRegEx->Name = L"UseRegEx";
			this->UseRegEx->Size = System::Drawing::Size(144, 17);
			this->UseRegEx->TabIndex = 12;
			this->UseRegEx->Text = L"Use Regular Expressions";
			this->UseRegEx->UseVisualStyleBackColor = true;
			// 
			// SEFindReplace
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(601, 126);
			this->Controls->Add(this->UseRegEx);
			this->Controls->Add(this->InSelection);
			this->Controls->Add(this->CaseInsensitiveSearch);
			this->Controls->Add(this->MatchWholeWord);
			this->Controls->Add(this->CountMatchesButton);
			this->Controls->Add(this->ReplaceInTabsButton);
			this->Controls->Add(this->FindInTabsButton);
			this->Controls->Add(this->ReplaceButton);
			this->Controls->Add(this->FindButton);
			this->Controls->Add(this->ReplacementBox);
			this->Controls->Add(this->QueryBox);
			this->Controls->Add(this->LabelReplace);
			this->Controls->Add(this->LabelFind);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
			this->Name = L"SEFindReplace";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Find/Replace";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	};
}