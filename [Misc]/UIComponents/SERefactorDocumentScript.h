#pragma once

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SERefactorDocumentScript
	/// </summary>
	public ref class SERefactorDocumentScript : public System::Windows::Forms::Form
	{
	public:
		SERefactorDocumentScript(void)
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
		~SERefactorDocumentScript()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ListView^  DocumentationList;
	protected: 
	private: System::Windows::Forms::ColumnHeader^  DocumentationListCHDocumentableElement;
	private: System::Windows::Forms::ColumnHeader^  DocumentationListCHComment;
	private: System::Windows::Forms::Button^  ImplementBox;

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
			this->DocumentationList = (gcnew System::Windows::Forms::ListView());
			this->DocumentationListCHDocumentableElement = (gcnew System::Windows::Forms::ColumnHeader());
			this->DocumentationListCHComment = (gcnew System::Windows::Forms::ColumnHeader());
			this->ImplementBox = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// DocumentationList
			// 
			this->DocumentationList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {this->DocumentationListCHDocumentableElement, 
				this->DocumentationListCHComment});
			this->DocumentationList->Location = System::Drawing::Point(13, 12);
			this->DocumentationList->Name = L"DocumentationList";
			this->DocumentationList->Size = System::Drawing::Size(484, 314);
			this->DocumentationList->TabIndex = 0;
			this->DocumentationList->UseCompatibleStateImageBehavior = false;
			this->DocumentationList->View = System::Windows::Forms::View::Details;
			// 
			// DocumentationListCHDocumentableElement
			// 
			this->DocumentationListCHDocumentableElement->Text = L"Documentable Element";
			this->DocumentationListCHDocumentableElement->Width = 151;
			// 
			// DocumentationListCHComment
			// 
			this->DocumentationListCHComment->Text = L"Comment";
			this->DocumentationListCHComment->Width = 328;
			// 
			// ImplementBox
			// 
			this->ImplementBox->Location = System::Drawing::Point(199, 332);
			this->ImplementBox->Name = L"ImplementBox";
			this->ImplementBox->Size = System::Drawing::Size(113, 25);
			this->ImplementBox->TabIndex = 1;
			this->ImplementBox->Text = L"Implement";
			this->ImplementBox->UseVisualStyleBackColor = true;
			// 
			// SERefactorDocumentScript
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(510, 369);
			this->Controls->Add(this->ImplementBox);
			this->Controls->Add(this->DocumentationList);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
			this->Name = L"SERefactorDocumentScript";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Document Script";
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
