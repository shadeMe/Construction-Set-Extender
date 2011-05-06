#pragma once

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace ICSharpCode;

	/// <summary>
	/// Summary for WPFHostTest
	/// </summary>
	public ref class WPFHostTest : public System::Windows::Forms::Form
	{
	public:
		WPFHostTest(void)
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
		~WPFHostTest()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Integration::ElementHost^  elementHost1;
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
			this->elementHost1 = (gcnew System::Windows::Forms::Integration::ElementHost());
		//	this->SuspendLayout();

			// 
			// elementHost1
			// 
			this->elementHost1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->elementHost1->Location = System::Drawing::Point(0, 0);
			this->elementHost1->Name = L"elementHost1";
			this->elementHost1->Size = System::Drawing::Size(1106, 602);
			this->elementHost1->TabIndex = 0;
			this->elementHost1->Text = L"elementHost1";
			this->elementHost1->ChildChanged += gcnew System::EventHandler<System::Windows::Forms::Integration::ChildChangedEventArgs^ >(this, &WPFHostTest::elementHost1_ChildChanged);
			this->elementHost1->Child = gcnew AvalonEdit::TextEditor();
			// 
			// WPFHostTest
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1106, 602);
			this->Controls->Add(this->elementHost1);
			this->Name = L"WPFHostTest";
			this->Text = L"WPFHostTest";
		//	this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void elementHost1_ChildChanged(System::Object^  sender, System::Windows::Forms::Integration::ChildChangedEventArgs^  e) {
			 }
	};
}
