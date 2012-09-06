#pragma once

namespace UIComponents {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SEVariableIndex
	/// </summary>
	public ref class SEVariableIndex : public System::Windows::Forms::Form
	{
	public:
		SEVariableIndex(void)
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
		~SEVariableIndex()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ListView^  VarIndexList;
	private: System::Windows::Forms::ColumnHeader^  VarIndexListCHName;
	private: System::Windows::Forms::ColumnHeader^  VarIndexListCHType;
	private: System::Windows::Forms::ColumnHeader^  VarIndexListCHIndex;
	private: System::Windows::Forms::Button^  UpdateIndicesButton;

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
			this->VarIndexList = (gcnew System::Windows::Forms::ListView());
			this->VarIndexListCHName = (gcnew System::Windows::Forms::ColumnHeader());
			this->VarIndexListCHType = (gcnew System::Windows::Forms::ColumnHeader());
			this->VarIndexListCHIndex = (gcnew System::Windows::Forms::ColumnHeader());
			this->UpdateIndicesButton = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			//
			// VarIndexList
			//
			this->VarIndexList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {this->VarIndexListCHName,
				this->VarIndexListCHType, this->VarIndexListCHIndex});
			this->VarIndexList->Location = System::Drawing::Point(12, 12);
			this->VarIndexList->Name = L"VarIndexList";
			this->VarIndexList->Size = System::Drawing::Size(484, 314);
			this->VarIndexList->TabIndex = 0;
			this->VarIndexList->UseCompatibleStateImageBehavior = false;
			this->VarIndexList->View = System::Windows::Forms::View::Details;
			//
			// VarIndexListCHName
			//
			this->VarIndexListCHName->Text = L"Name";
			this->VarIndexListCHName->Width = 250;
			//
			// VarIndexListCHType
			//
			this->VarIndexListCHType->Text = L"Type";
			this->VarIndexListCHType->Width = 124;
			//
			// VarIndexListCHIndex
			//
			this->VarIndexListCHIndex->Text = L"Index";
			this->VarIndexListCHIndex->Width = 100;
			//
			// UpdateIndicesButton
			//
			this->UpdateIndicesButton->Location = System::Drawing::Point(199, 332);
			this->UpdateIndicesButton->Name = L"UpdateIndicesButton";
			this->UpdateIndicesButton->Size = System::Drawing::Size(113, 25);
			this->UpdateIndicesButton->TabIndex = 1;
			this->UpdateIndicesButton->Text = L"Update Indices";
			this->UpdateIndicesButton->UseVisualStyleBackColor = true;
			//
			// SEVariableIndex
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(510, 369);
			this->Controls->Add(this->UpdateIndicesButton);
			this->Controls->Add(this->VarIndexList);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
			this->Name = L"SEVariableIndex";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Modify Variable Indices";
			this->ResumeLayout(false);
		}
#pragma endregion
	};
}
