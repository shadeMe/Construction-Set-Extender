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
			this->SuspendLayout();
			// 
			// PreviewBox
			// 
			this->PreviewBox->Font = (gcnew System::Drawing::Font(L"Consolas", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->PreviewBox->Location = System::Drawing::Point(462, 12);
			this->PreviewBox->Multiline = true;
			this->PreviewBox->Name = L"PreviewBox";
			this->PreviewBox->ReadOnly = true;
			this->PreviewBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->PreviewBox->Size = System::Drawing::Size(444, 520);
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
			this->ScriptList->Size = System::Drawing::Size(444, 485);
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
			this->SearchBox->Location = System::Drawing::Point(12, 503);
			this->SearchBox->MaxLength = 512;
			this->SearchBox->Name = L"SearchBox";
			this->SearchBox->Size = System::Drawing::Size(312, 30);
			this->SearchBox->TabIndex = 2;
			// 
			// button1
			// 
			this->button1->Enabled = false;
			this->button1->Location = System::Drawing::Point(330, 503);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(126, 29);
			this->button1->TabIndex = 3;
			this->button1->Text = L"Filter";
			this->button1->UseVisualStyleBackColor = true;
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(916, 541);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->SearchBox);
			this->Controls->Add(this->ScriptList);
			this->Controls->Add(this->PreviewBox);
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"Form1";
			this->Text = L"Select Script";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
};
}

