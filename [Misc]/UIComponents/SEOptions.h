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
	private: System::Windows::Forms::PropertyGrid^ PropertyGrid;
	private: System::Windows::Forms::ToolStrip^ ToolStripSettingCategories;
	private: System::Windows::Forms::ToolStripLabel^ ToolStripLabelCategories;

	private: System::Windows::Forms::Label^ LabelCurrentCategory;










	protected:

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
			this->PropertyGrid = (gcnew System::Windows::Forms::PropertyGrid());
			this->ToolStripSettingCategories = (gcnew System::Windows::Forms::ToolStrip());
			this->ToolStripLabelCategories = (gcnew System::Windows::Forms::ToolStripLabel());
			this->LabelCurrentCategory = (gcnew System::Windows::Forms::Label());
			this->ToolStripSettingCategories->SuspendLayout();
			this->SuspendLayout();
			// 
			// PropertyGrid
			// 
			this->PropertyGrid->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->PropertyGrid->Location = System::Drawing::Point(67, 12);
			this->PropertyGrid->Name = L"PropertyGrid";
			this->PropertyGrid->Size = System::Drawing::Size(355, 437);
			this->PropertyGrid->TabIndex = 1;
			// 
			// ToolStripSettingCategories
			// 
			this->ToolStripSettingCategories->AutoSize = false;
			this->ToolStripSettingCategories->Dock = System::Windows::Forms::DockStyle::Left;
			this->ToolStripSettingCategories->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->ToolStripSettingCategories->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->ToolStripLabelCategories });
			this->ToolStripSettingCategories->Location = System::Drawing::Point(0, 0);
			this->ToolStripSettingCategories->Name = L"ToolStripSettingCategories";
			this->ToolStripSettingCategories->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
			this->ToolStripSettingCategories->Size = System::Drawing::Size(64, 461);
			this->ToolStripSettingCategories->TabIndex = 2;
			// 
			// ToolStripLabelCategories
			// 
			this->ToolStripLabelCategories->Name = L"ToolStripLabelCategories";
			this->ToolStripLabelCategories->Padding = System::Windows::Forms::Padding(5);
			this->ToolStripLabelCategories->Size = System::Drawing::Size(62, 25);
			this->ToolStripLabelCategories->Text = L"Categories";
			// 
			// LabelCurrentCategory
			// 
			this->LabelCurrentCategory->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->LabelCurrentCategory->AutoSize = true;
			this->LabelCurrentCategory->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->LabelCurrentCategory->Location = System::Drawing::Point(292, 9);
			this->LabelCurrentCategory->Name = L"LabelCurrentCategory";
			this->LabelCurrentCategory->Size = System::Drawing::Size(130, 21);
			this->LabelCurrentCategory->TabIndex = 3;
			this->LabelCurrentCategory->Text = L"Current Category";
			this->LabelCurrentCategory->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// OptionsDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(434, 461);
			this->Controls->Add(this->LabelCurrentCategory);
			this->Controls->Add(this->ToolStripSettingCategories);
			this->Controls->Add(this->PropertyGrid);
			this->MinimizeBox = false;
			this->MinimumSize = System::Drawing::Size(450, 500);
			this->Name = L"OptionsDialog";
			this->ShowIcon = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Script Editor Preferences";
			this->ToolStripSettingCategories->ResumeLayout(false);
			this->ToolStripSettingCategories->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

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