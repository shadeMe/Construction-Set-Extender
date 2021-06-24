#pragma once

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for ScriptEditorRefactorForm
	/// </summary>
	public ref class ScriptEditorRefactorForm : public DevComponents::DotNetBar::Metro::MetroForm
	{
	public:
		ScriptEditorRefactorForm(void)
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
		~ScriptEditorRefactorForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: BrightIdeasSoftware::ObjectListView^ ListView;
	protected:

	protected:
	private: DevComponents::DotNetBar::ButtonX^ ButtonOk;
	private: DevComponents::DotNetBar::ButtonX^ ButtonCancel;
	private: DevComponents::DotNetBar::LabelX^ LabelDescription;

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
			this->ListView = (gcnew BrightIdeasSoftware::ObjectListView());
			this->ButtonOk = (gcnew DevComponents::DotNetBar::ButtonX());
			this->ButtonCancel = (gcnew DevComponents::DotNetBar::ButtonX());
			this->LabelDescription = (gcnew DevComponents::DotNetBar::LabelX());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListView))->BeginInit();
			this->SuspendLayout();
			// 
			// ListView
			// 
			this->ListView->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->ListView->CellEditUseWholeCell = false;
			this->ListView->HideSelection = false;
			this->ListView->Location = System::Drawing::Point(12, 41);
			this->ListView->MultiSelect = false;
			this->ListView->Name = L"ListView";
			this->ListView->Size = System::Drawing::Size(386, 342);
			this->ListView->TabIndex = 0;
			this->ListView->UseCompatibleStateImageBehavior = false;
			this->ListView->View = System::Windows::Forms::View::Details;
			// 
			// ButtonOk
			// 
			this->ButtonOk->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->ButtonOk->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->ButtonOk->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->ButtonOk->Location = System::Drawing::Point(226, 389);
			this->ButtonOk->Name = L"ButtonOk";
			this->ButtonOk->Size = System::Drawing::Size(83, 27);
			this->ButtonOk->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->ButtonOk->TabIndex = 1;
			this->ButtonOk->Text = L"OK";
			// 
			// ButtonCancel
			// 
			this->ButtonCancel->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->ButtonCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->ButtonCancel->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->ButtonCancel->Location = System::Drawing::Point(315, 389);
			this->ButtonCancel->Name = L"ButtonCancel";
			this->ButtonCancel->Size = System::Drawing::Size(83, 27);
			this->ButtonCancel->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->ButtonCancel->TabIndex = 2;
			this->ButtonCancel->Text = L"Cancel";
			// 
			// LabelDescription
			// 
			this->LabelDescription->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			// 
			// 
			// 
			this->LabelDescription->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->LabelDescription->Location = System::Drawing::Point(12, 3);
			this->LabelDescription->Name = L"LabelDescription";
			this->LabelDescription->Size = System::Drawing::Size(386, 32);
			this->LabelDescription->TabIndex = 3;
			this->LabelDescription->Text = L"Description";
			// 
			// ScriptEditorRefactorForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(410, 428);
			this->Controls->Add(this->LabelDescription);
			this->Controls->Add(this->ButtonCancel);
			this->Controls->Add(this->ButtonOk);
			this->Controls->Add(this->ListView);
			this->DoubleBuffered = true;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::SizableToolWindow;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"ScriptEditorRefactorForm";
			this->ShowIcon = false;
			this->Text = L"Refactor Script";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListView))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
