#pragma once


namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
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
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::TextBox^  PreviewBox;
	private: System::Windows::Forms::ListView^  ScriptList;
	private: System::Windows::Forms::ColumnHeader^  ScriptListCScriptName;
	private: System::Windows::Forms::ColumnHeader^  ScriptListCFormID;
	private: System::Windows::Forms::ColumnHeader^  ScriptListCScriptType;
	private: System::Windows::Forms::TextBox^  SearchBox;
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::TabControl^  tabControl1;
	private: System::Windows::Forms::TabPage^  tabPage1;
	private: System::Windows::Forms::TabPage^  tabPage2;
	private: System::Windows::Forms::TabPage^  tabPage3;
	private: System::Windows::Forms::TabPage^  tabPage4;
	private: System::Windows::Forms::TabPage^  tabPage5;
	private: System::Windows::Forms::TabPage^  tabPage6;
	private: System::Windows::Forms::TabPage^  tabPage7;
	private: System::Windows::Forms::TabPage^  tabPage8;
	private: System::Windows::Forms::TabPage^  tabPage9;
	private: System::Windows::Forms::TabPage^  tabPage10;
	private: System::Windows::Forms::TabPage^  tabPage11;
	private: System::Windows::Forms::TabPage^  tabPage12;
	private: System::Windows::Forms::TabPage^  tabPage13;

























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
			this->PreviewBox = (gcnew System::Windows::Forms::TextBox());
			this->ScriptList = (gcnew System::Windows::Forms::ListView());
			this->ScriptListCScriptName = (gcnew System::Windows::Forms::ColumnHeader());
			this->ScriptListCFormID = (gcnew System::Windows::Forms::ColumnHeader());
			this->ScriptListCScriptType = (gcnew System::Windows::Forms::ColumnHeader());
			this->SearchBox = (gcnew System::Windows::Forms::TextBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage1 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage2 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage3 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage4 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage5 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage6 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage7 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage8 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage9 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage10 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage11 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage12 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage13 = (gcnew System::Windows::Forms::TabPage());
			this->tabControl1->SuspendLayout();
			this->SuspendLayout();
			// 
			// PreviewBox
			// 
			this->PreviewBox->Font = (gcnew System::Drawing::Font(L"Consolas", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->PreviewBox->Location = System::Drawing::Point(375, 12);
			this->PreviewBox->Multiline = true;
			this->PreviewBox->Name = L"PreviewBox";
			this->PreviewBox->ReadOnly = true;
			this->PreviewBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->PreviewBox->Size = System::Drawing::Size(357, 427);
			this->PreviewBox->TabIndex = 0;
			// 
			// ScriptList
			// 
			this->ScriptList->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->ScriptList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {this->ScriptListCScriptName, 
				this->ScriptListCFormID, this->ScriptListCScriptType});
			this->ScriptList->Font = (gcnew System::Drawing::Font(L"Consolas", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->ScriptList->Location = System::Drawing::Point(12, 12);
			this->ScriptList->Name = L"ScriptList";
			this->ScriptList->Size = System::Drawing::Size(357, 391);
			this->ScriptList->TabIndex = 1;
			this->ScriptList->UseCompatibleStateImageBehavior = false;
			this->ScriptList->View = System::Windows::Forms::View::Details;
			// 
			// ScriptListCScriptName
			// 
			this->ScriptListCScriptName->Text = L"Name";
			this->ScriptListCScriptName->Width = 196;
			// 
			// ScriptListCFormID
			// 
			this->ScriptListCFormID->Text = L"FormID";
			this->ScriptListCFormID->Width = 73;
			// 
			// ScriptListCScriptType
			// 
			this->ScriptListCScriptType->Text = L"Type";
			this->ScriptListCScriptType->Width = 87;
			// 
			// SearchBox
			// 
			this->SearchBox->Font = (gcnew System::Drawing::Font(L"Consolas", 14.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->SearchBox->Location = System::Drawing::Point(13, 409);
			this->SearchBox->MaxLength = 512;
			this->SearchBox->Name = L"SearchBox";
			this->SearchBox->Size = System::Drawing::Size(242, 30);
			this->SearchBox->TabIndex = 2;
			// 
			// button1
			// 
			this->button1->Enabled = false;
			this->button1->Location = System::Drawing::Point(261, 409);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(107, 29);
			this->button1->TabIndex = 3;
			this->button1->Text = L"Filter";
			this->button1->UseVisualStyleBackColor = true;
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->tabPage2);
			this->tabControl1->Controls->Add(this->tabPage3);
			this->tabControl1->Controls->Add(this->tabPage4);
			this->tabControl1->Controls->Add(this->tabPage1);
			this->tabControl1->Controls->Add(this->tabPage5);
			this->tabControl1->Controls->Add(this->tabPage6);
			this->tabControl1->Controls->Add(this->tabPage7);
			this->tabControl1->Controls->Add(this->tabPage8);
			this->tabControl1->Controls->Add(this->tabPage9);
			this->tabControl1->Controls->Add(this->tabPage10);
			this->tabControl1->Controls->Add(this->tabPage11);
			this->tabControl1->Controls->Add(this->tabPage12);
			this->tabControl1->Controls->Add(this->tabPage13);
			this->tabControl1->HotTrack = true;
			this->tabControl1->Location = System::Drawing::Point(13, 454);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->Size = System::Drawing::Size(718, 56);
			this->tabControl1->TabIndex = 4;
			// 
			// tabPage1
			// 
			this->tabPage1->Location = System::Drawing::Point(4, 22);
			this->tabPage1->Name = L"tabPage1";
			this->tabPage1->Padding = System::Windows::Forms::Padding(3);
			this->tabPage1->Size = System::Drawing::Size(710, 30);
			this->tabPage1->TabIndex = 0;
			this->tabPage1->Text = L"tabPage1";
			this->tabPage1->UseVisualStyleBackColor = true;
			// 
			// tabPage2
			// 
			this->tabPage2->Location = System::Drawing::Point(4, 22);
			this->tabPage2->Name = L"tabPage2";
			this->tabPage2->Padding = System::Windows::Forms::Padding(3);
			this->tabPage2->Size = System::Drawing::Size(710, 30);
			this->tabPage2->TabIndex = 1;
			this->tabPage2->Text = L"tabPage2";
			this->tabPage2->UseVisualStyleBackColor = true;
			// 
			// tabPage3
			// 
			this->tabPage3->Location = System::Drawing::Point(4, 22);
			this->tabPage3->Name = L"tabPage3";
			this->tabPage3->Padding = System::Windows::Forms::Padding(3);
			this->tabPage3->Size = System::Drawing::Size(710, 30);
			this->tabPage3->TabIndex = 2;
			this->tabPage3->Text = L"tabPage3";
			this->tabPage3->UseVisualStyleBackColor = true;
			// 
			// tabPage4
			// 
			this->tabPage4->Location = System::Drawing::Point(4, 22);
			this->tabPage4->Name = L"tabPage4";
			this->tabPage4->Padding = System::Windows::Forms::Padding(3);
			this->tabPage4->Size = System::Drawing::Size(710, 30);
			this->tabPage4->TabIndex = 3;
			this->tabPage4->Text = L"tabPage4";
			this->tabPage4->UseVisualStyleBackColor = true;
			// 
			// tabPage5
			// 
			this->tabPage5->Location = System::Drawing::Point(4, 22);
			this->tabPage5->Name = L"tabPage5";
			this->tabPage5->Padding = System::Windows::Forms::Padding(3);
			this->tabPage5->Size = System::Drawing::Size(710, 30);
			this->tabPage5->TabIndex = 4;
			this->tabPage5->Text = L"tabPage5";
			this->tabPage5->UseVisualStyleBackColor = true;
			// 
			// tabPage6
			// 
			this->tabPage6->Location = System::Drawing::Point(4, 22);
			this->tabPage6->Name = L"tabPage6";
			this->tabPage6->Padding = System::Windows::Forms::Padding(3);
			this->tabPage6->Size = System::Drawing::Size(710, 30);
			this->tabPage6->TabIndex = 5;
			this->tabPage6->Text = L"tabPage6";
			this->tabPage6->UseVisualStyleBackColor = true;
			// 
			// tabPage7
			// 
			this->tabPage7->Location = System::Drawing::Point(4, 22);
			this->tabPage7->Name = L"tabPage7";
			this->tabPage7->Padding = System::Windows::Forms::Padding(3);
			this->tabPage7->Size = System::Drawing::Size(710, 30);
			this->tabPage7->TabIndex = 6;
			this->tabPage7->Text = L"tabPage7";
			this->tabPage7->UseVisualStyleBackColor = true;
			// 
			// tabPage8
			// 
			this->tabPage8->Location = System::Drawing::Point(4, 22);
			this->tabPage8->Name = L"tabPage8";
			this->tabPage8->Padding = System::Windows::Forms::Padding(3);
			this->tabPage8->Size = System::Drawing::Size(710, 30);
			this->tabPage8->TabIndex = 7;
			this->tabPage8->Text = L"tabPage8";
			this->tabPage8->UseVisualStyleBackColor = true;
			// 
			// tabPage9
			// 
			this->tabPage9->Location = System::Drawing::Point(4, 22);
			this->tabPage9->Name = L"tabPage9";
			this->tabPage9->Padding = System::Windows::Forms::Padding(3);
			this->tabPage9->Size = System::Drawing::Size(710, 30);
			this->tabPage9->TabIndex = 8;
			this->tabPage9->Text = L"tabPage9";
			this->tabPage9->UseVisualStyleBackColor = true;
			// 
			// tabPage10
			// 
			this->tabPage10->Location = System::Drawing::Point(4, 22);
			this->tabPage10->Name = L"tabPage10";
			this->tabPage10->Padding = System::Windows::Forms::Padding(3);
			this->tabPage10->Size = System::Drawing::Size(710, 30);
			this->tabPage10->TabIndex = 9;
			this->tabPage10->Text = L"tabPage10";
			this->tabPage10->UseVisualStyleBackColor = true;
			// 
			// tabPage11
			// 
			this->tabPage11->Location = System::Drawing::Point(4, 22);
			this->tabPage11->Name = L"tabPage11";
			this->tabPage11->Padding = System::Windows::Forms::Padding(3);
			this->tabPage11->Size = System::Drawing::Size(710, 30);
			this->tabPage11->TabIndex = 10;
			this->tabPage11->Text = L"tabPage11";
			this->tabPage11->UseVisualStyleBackColor = true;
			// 
			// tabPage12
			// 
			this->tabPage12->Location = System::Drawing::Point(4, 22);
			this->tabPage12->Name = L"tabPage12";
			this->tabPage12->Padding = System::Windows::Forms::Padding(3);
			this->tabPage12->Size = System::Drawing::Size(710, 30);
			this->tabPage12->TabIndex = 11;
			this->tabPage12->Text = L"tabPage12";
			this->tabPage12->UseVisualStyleBackColor = true;
			// 
			// tabPage13
			// 
			this->tabPage13->Location = System::Drawing::Point(4, 22);
			this->tabPage13->Name = L"tabPage13";
			this->tabPage13->Padding = System::Windows::Forms::Padding(3);
			this->tabPage13->Size = System::Drawing::Size(710, 30);
			this->tabPage13->TabIndex = 12;
			this->tabPage13->Text = L"tabPage13";
			this->tabPage13->UseVisualStyleBackColor = true;
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(742, 514);
			this->Controls->Add(this->tabControl1);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->SearchBox);
			this->Controls->Add(this->ScriptList);
			this->Controls->Add(this->PreviewBox);
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"Form1";
			this->Text = L"Select Script";
			this->tabControl1->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
};
}

