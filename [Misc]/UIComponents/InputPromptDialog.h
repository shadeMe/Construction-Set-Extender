#pragma once

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for InputPromptDialog
	/// </summary>
	public ref class InputPromptDialog : public DevComponents::DotNetBar::Metro::MetroForm
	{
	public:
		InputPromptDialog(void)
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
		~InputPromptDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: DevComponents::DotNetBar::LabelX^ lblPrompt;
	private: DevComponents::DotNetBar::ButtonX^ btnOK;
	private: DevComponents::DotNetBar::ButtonX^ btnCancel;
	private: DevComponents::DotNetBar::Controls::TextBoxX^ txtInput;
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
			this->lblPrompt = (gcnew DevComponents::DotNetBar::LabelX());
			this->btnOK = (gcnew DevComponents::DotNetBar::ButtonX());
			this->btnCancel = (gcnew DevComponents::DotNetBar::ButtonX());
			this->txtInput = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
			this->SuspendLayout();
			//
			// lblPrompt
			//
			//
			//
			//
			this->lblPrompt->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->lblPrompt->Location = System::Drawing::Point(12, 12);
			this->lblPrompt->Name = L"lblPrompt";
			this->lblPrompt->Size = System::Drawing::Size(313, 52);
			this->lblPrompt->TabIndex = 0;
			this->lblPrompt->Text = L"Prompt Text";
			this->lblPrompt->TextLineAlignment = System::Drawing::StringAlignment::Near;
			//
			// btnOK
			//
			this->btnOK->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->btnOK->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->btnOK->Location = System::Drawing::Point(345, 12);
			this->btnOK->Name = L"btnOK";
			this->btnOK->Size = System::Drawing::Size(75, 23);
			this->btnOK->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->btnOK->TabIndex = 1;
			this->btnOK->Text = L"OK";
			//
			// btnCancel
			//
			this->btnCancel->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->btnCancel->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->btnCancel->Location = System::Drawing::Point(345, 41);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->btnCancel->TabIndex = 2;
			this->btnCancel->Text = L"Cancel";
			//
			// txtInput
			//
			this->txtInput->BackColor = System::Drawing::Color::Black;
			//
			//
			//
			this->txtInput->Border->Class = L"TextBoxBorder";
			this->txtInput->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->txtInput->DisabledBackColor = System::Drawing::Color::Black;
			this->txtInput->ForeColor = System::Drawing::Color::White;
			this->txtInput->Location = System::Drawing::Point(12, 74);
			this->txtInput->Name = L"txtInput";
			this->txtInput->PreventEnterBeep = true;
			this->txtInput->Size = System::Drawing::Size(313, 22);
			this->txtInput->TabIndex = 3;
			//
			// InputPromptDialog
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->btnCancel;
			this->ClientSize = System::Drawing::Size(432, 108);
			this->ControlBox = false;
			this->Controls->Add(this->txtInput);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->btnOK);
			this->Controls->Add(this->lblPrompt);
			this->DoubleBuffered = true;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
			this->Name = L"InputPromptDialog";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Prompt Dialog";
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
