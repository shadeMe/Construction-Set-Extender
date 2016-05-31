#pragma once

// InputBox.h
//
// This input box module was designed to support 342 characters in text field and 74 in input field
// It's a suiting for Form Designers in C/C++ from C#
// It was based on 'hestol' code: http://www.codeproject.com/KB/edit/InputBox.aspx
//
// To edit it in Window Form Designer you must withdraw the first class (InputBoxResult) to another
// location of the code and after replace it again where it was. Or you can just comment it.

namespace inputBoxes
{
	using namespace System;
	using namespace System::Drawing;
	using namespace System::Collections;
	using namespace System::ComponentModel;
	using namespace System::Windows::Forms;

	/* START THE COMMENT HERE TO DESIGN THE FORM */
	#pragma region InputBox return result
		ref class InputBoxResult
		{
			public: DialogResult ReturnCode;
			public: String^ Text;
		};
	#pragma endregion
	/* STOP THE COMMENT HERE TO DESIGN THE FORM */

	ref class InputBox : public System::Windows::Forms::Form
	{
		#pragma region Private Variables
			private:
				static String^ _formCaption = String::Empty;
				static String^ _formPrompt = String::Empty;
				static InputBoxResult^ _outputResponse = gcnew InputBoxResult();
				static String^ _defaultValue = String::Empty;
				static int _xPos = -1;
				static int _yPos = -1;
		#pragma endregion
		#pragma region Private Windows Contols and Constructor
			public:
				InputBox(void)
				{
					InitializeComponent();
				}
			protected:
				~InputBox()
				{
					if (components) delete components;
				}
			private:
				System::Windows::Forms::Label^      lblPrompt;
				System::Windows::Forms::Button^     btnOK;
				System::Windows::Forms::Button^     btnCancel;
				System::Windows::Forms::TextBox^    txtInput;
				System::ComponentModel::Container^  components;

				void								txtInput_KeyDown(Object^ Sender, KeyEventArgs^ E)
				{
					switch (E->KeyCode)
					{
					case Keys::Enter:
						btnOK->PerformClick();
						break;
					case Keys::Escape:
						btnCancel->PerformClick();
						break;
					}
				}
		#pragma endregion
		#pragma region Windows Form Designer generated code
			void InitializeComponent(void)
			{
				this->lblPrompt = (gcnew System::Windows::Forms::Label());
				this->btnOK = (gcnew System::Windows::Forms::Button());
				this->btnCancel = (gcnew System::Windows::Forms::Button());
				this->txtInput = (gcnew System::Windows::Forms::TextBox());
				this->SuspendLayout();
				//
				// lblPrompt
				//
				this->lblPrompt->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
					| System::Windows::Forms::AnchorStyles::Left)
					| System::Windows::Forms::AnchorStyles::Right));
				this->lblPrompt->Location = System::Drawing::Point(13, 9);
				this->lblPrompt->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
				this->lblPrompt->Name = L"lblPrompt";
				this->lblPrompt->Size = System::Drawing::Size(362, 84);
				this->lblPrompt->TabIndex = 0;
				//
				// btnOK
				//
				this->btnOK->DialogResult = System::Windows::Forms::DialogResult::OK;
				this->btnOK->Location = System::Drawing::Point(388, 12);
				this->btnOK->Margin = System::Windows::Forms::Padding(4, 3, 4, 3);
				this->btnOK->Name = L"btnOK";
				this->btnOK->Size = System::Drawing::Size(77, 24);
				this->btnOK->TabIndex = 1;
				this->btnOK->Text = L"OK";
				this->btnOK->UseVisualStyleBackColor = true;
				this->btnOK->Click += gcnew System::EventHandler(this, &InputBox::btnOK_Click);
				//
				// btnCancel
				//
				this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
				this->btnCancel->Location = System::Drawing::Point(388, 42);
				this->btnCancel->Margin = System::Windows::Forms::Padding(4, 3, 4, 3);
				this->btnCancel->Name = L"btnCancel";
				this->btnCancel->Size = System::Drawing::Size(77, 24);
				this->btnCancel->TabIndex = 2;
				this->btnCancel->Text = L"Cancel";
				this->btnCancel->UseVisualStyleBackColor = true;
				this->btnCancel->Click += gcnew System::EventHandler(this, &InputBox::btnCancel_Click);
				//
				// txtInput
				//
				this->txtInput->Location = System::Drawing::Point(13, 96);
				this->txtInput->Margin = System::Windows::Forms::Padding(4, 3, 4, 3);
				this->txtInput->Name = L"txtInput";
				this->txtInput->Size = System::Drawing::Size(452, 20);
				this->txtInput->TabIndex = 3;
				this->txtInput->AcceptsReturn = true;
				this->txtInput->Multiline = true;
				this->txtInput->KeyDown += gcnew KeyEventHandler(this, &InputBox::txtInput_KeyDown);
				//
				// InputBox
				//
				this->ControlBox = false;
				this->ShowInTaskbar = false;
				this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
				this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
				this->ClientSize = System::Drawing::Size(478, 128);
				this->Controls->Add(this->txtInput);
				this->Controls->Add(this->btnCancel);
				this->Controls->Add(this->btnOK);
				this->Controls->Add(this->lblPrompt);
				this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
				this->Visible = false;
				this->Margin = System::Windows::Forms::Padding(4, 3, 4, 3);
				this->Name = L"InputBox";
				this->Load += gcnew System::EventHandler(this, &InputBox::InputBox_Load);
				this->ResumeLayout(false);
				this->PerformLayout();
			}
		#pragma endregion
		#pragma region Private function, InputBox Form move and change size
			private: Void InputBox_Load(System::Object^  sender, System::EventArgs^  e)
			{
				OutputResponse->ReturnCode = Windows::Forms::DialogResult::Ignore;
				OutputResponse->Text = String::Empty;

				txtInput->Text = _defaultValue;
				lblPrompt->Text = _formPrompt;
				Text = _formCaption;

				// Retrieve the working rectangle from the Screen class
				// using the PrimaryScreen and the WorkingArea properties->
				Rectangle workingRectangle = Screen::PrimaryScreen->WorkingArea;

				if ((_xPos >= 0 && _xPos < workingRectangle.Width-100) && (_yPos >= 0 && _yPos < workingRectangle.Height-100))
				{
					StartPosition = FormStartPosition::Manual;
					Location = Point(_xPos, _yPos);
				}
				else
				{
					StartPosition = FormStartPosition::CenterScreen;
					this->CenterToScreen();
				}

				String^ PrompText = lblPrompt->Text;

				int n = 0;
				int Index = 0;
				while (PrompText->IndexOf("\n",Index) > -1)
				{
					Index = PrompText->IndexOf("\n",Index)+1;
					n++;
				}

				if (n == 0)
					n = 1;

				Drawing::Point Txt = txtInput->Location;
				Txt.Y = Txt.Y + (n*4);
				txtInput->Location = Txt;
				Drawing::Size form = Size;
				form.Height = form.Height + (n*4);
				Size = form;

				txtInput->SelectionStart = 0;
				txtInput->SelectionLength = txtInput->Text->Length;
				txtInput->Focus();
			}
		#pragma endregion
		#pragma region Button control click event
			private:
				Void btnOK_Click(Object^ sender, EventArgs^ e)
				{
					OutputResponse->ReturnCode = Windows::Forms::DialogResult::OK;
					OutputResponse->Text = txtInput->Text;
					//delete frmInputDialog;
					Close();
				}
				Void btnCancel_Click(Object^ sender, EventArgs^ e)
				{
					OutputResponse->ReturnCode = Windows::Forms::DialogResult::Cancel;
					OutputResponse->Text = String::Empty; // Clean output response
					//delete frmInputDialog;
					Close();
				}
		#pragma endregion
		#pragma region Public Static Show functions
			public: static InputBoxResult^ Show(String^ Prompt)
			{
				InputBox^ dialogForm = gcnew InputBox();

				dialogForm->FormPrompt = Prompt;
// 				dialogForm->XPosition    = -1;
// 				dialogForm->YPosition    = -1;

				// Display the form as a modal dialog box->
				dialogForm->ShowDialog();
				return dialogForm->OutputResponse;
			}
			public: static InputBoxResult^ Show(String^ Prompt,String^ Title)
			{
				InputBox^ dialogForm = gcnew InputBox();

				dialogForm->FormCaption = Title;
				dialogForm->FormPrompt  = Prompt;
// 				dialogForm->XPosition    = -1;
// 				dialogForm->YPosition    = -1;

				// Display the form as a modal dialog box->
				dialogForm->ShowDialog();
				return dialogForm->OutputResponse;
			}
			public: static InputBoxResult^ Show(String^ Prompt,String^ Title,String^ Default)
			{
				InputBox^ dialogForm = gcnew InputBox();

				dialogForm->FormCaption  = Title;
				dialogForm->FormPrompt   = Prompt;
				dialogForm->DefaultValue = Default;
// 				dialogForm->XPosition    = -1;
// 				dialogForm->YPosition    = -1;

				// Display the form as a modal dialog box->
				dialogForm->ShowDialog();
				return dialogForm->OutputResponse;
			}
			public: static InputBoxResult^ Show(String^ Prompt,String^ Title,String^ Default,int XPos,int YPos)
			{
				InputBox^ dialogForm = gcnew InputBox();

				dialogForm->FormCaption  = Title;
				dialogForm->FormPrompt   = Prompt;
				dialogForm->DefaultValue = Default;
				dialogForm->XPosition    = XPos;
				dialogForm->YPosition    = YPos;

				// Display the form as a modal dialog box->
				dialogForm->ShowDialog();
				return dialogForm->OutputResponse;
			}
		#pragma endregion
		#pragma region Private Properties
			private: static property String^ FormCaption
			{
				void set(String^ value)
				{
					_formCaption = value;
				}
			}
			private: static property String^ FormPrompt
			{
				void set(String^ value)
				{
					_formPrompt = value;
				}
			}
			private: static property InputBoxResult^ OutputResponse
			{
				InputBoxResult^ get()
				{
					return _outputResponse;
				}
				void set(InputBoxResult^ value)
				{
					_outputResponse = value;
				}
			}
			private: static property String^ DefaultValue
			{
				void set(String^ value)
				{
					_defaultValue = value;
				}
			}
			private: static property int XPosition
			{
				void set(int value)
				{
				//	if (value >= 0)
						_xPos = value;
				}
			}
			private: static property int YPosition
			{
				void set(int value)
				{
				//	if (value >= 0)
						_yPos = value;
				}
			}
		#pragma endregion
	};
}