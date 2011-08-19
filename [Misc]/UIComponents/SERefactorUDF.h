#pragma once

namespace UIComponents {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SERefactorUDF
	/// </summary>
	public ref class SERefactorUDF : public System::Windows::Forms::Form
	{
	public:
		SERefactorUDF(void)
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
		~SERefactorUDF()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ListView^  ParameterBox;
	protected:

	private: System::Windows::Forms::Button^  ImplementBox;
	private: System::Windows::Forms::ColumnHeader^  ParameterBoxCHName;
	private: System::Windows::Forms::ColumnHeader^  ParameterBoxCHType;
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
			System::Windows::Forms::ListViewItem^  listViewItem1 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			System::Windows::Forms::ListViewItem^  listViewItem2 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			System::Windows::Forms::ListViewItem^  listViewItem3 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			System::Windows::Forms::ListViewItem^  listViewItem4 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			System::Windows::Forms::ListViewItem^  listViewItem5 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			System::Windows::Forms::ListViewItem^  listViewItem6 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			System::Windows::Forms::ListViewItem^  listViewItem7 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			System::Windows::Forms::ListViewItem^  listViewItem8 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			System::Windows::Forms::ListViewItem^  listViewItem9 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			System::Windows::Forms::ListViewItem^  listViewItem10 = (gcnew System::Windows::Forms::ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"Int"}, -1));
			this->ParameterBox = (gcnew System::Windows::Forms::ListView());
			this->ImplementBox = (gcnew System::Windows::Forms::Button());
			this->ParameterBoxCHName = (gcnew System::Windows::Forms::ColumnHeader());
			this->ParameterBoxCHType = (gcnew System::Windows::Forms::ColumnHeader());
			this->SuspendLayout();
			//
			// ParameterBox
			//
			this->ParameterBox->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {this->ParameterBoxCHName,
				this->ParameterBoxCHType});
			this->ParameterBox->Items->AddRange(gcnew cli::array< System::Windows::Forms::ListViewItem^  >(10) {listViewItem1, listViewItem2,
				listViewItem3, listViewItem4, listViewItem5, listViewItem6, listViewItem7, listViewItem8, listViewItem9, listViewItem10});
			this->ParameterBox->Location = System::Drawing::Point(12, 12);
			this->ParameterBox->Name = L"ParameterBox";
			this->ParameterBox->Size = System::Drawing::Size(345, 200);
			this->ParameterBox->TabIndex = 0;
			this->ParameterBox->UseCompatibleStateImageBehavior = false;
			this->ParameterBox->View = System::Windows::Forms::View::Details;
			this->ParameterBox->SelectedIndexChanged += gcnew System::EventHandler(this, &SERefactorUDF::listView1_SelectedIndexChanged);
			//
			// ImplementBox
			//
			this->ImplementBox->Location = System::Drawing::Point(137, 218);
			this->ImplementBox->Name = L"ImplementBox";
			this->ImplementBox->Size = System::Drawing::Size(95, 23);
			this->ImplementBox->TabIndex = 1;
			this->ImplementBox->Text = L"Implement";
			this->ImplementBox->UseVisualStyleBackColor = true;
			//
			// ParameterBoxCHName
			//
			this->ParameterBoxCHName->Text = L"Parameter Name";
			this->ParameterBoxCHName->Width = 241;
			//
			// ParameterBoxCHType
			//
			this->ParameterBoxCHType->Text = L"Type";
			this->ParameterBoxCHType->Width = 100;
			//
			// SERefactorUDF
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(369, 248);
			this->Controls->Add(this->ImplementBox);
			this->Controls->Add(this->ParameterBox);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
			this->KeyPreview = true;
			this->Name = L"SERefactorUDF";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Create UDF Implementation";
			this->ResumeLayout(false);
		}
#pragma endregion
	private: System::Void listView1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	};
}