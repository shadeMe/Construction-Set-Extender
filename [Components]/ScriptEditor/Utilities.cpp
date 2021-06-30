#include "Utilities.h"

namespace cse
{


namespace scriptEditor
{


IAction::IAction(String^ Name, String^ Description)
{
	Name_ = Name;
	Description_ = Description;
}

BasicAction::BasicAction(String^ Name, String^ Description)
	: IAction(Name, Description)
{
}

BasicAction::~BasicAction()
{
	delete InvokeDelegate;
}

void BasicAction::Invoke()
{
	InvokeDelegate();
}

void KeyCombo::Validate()
{
	if (Main.HasFlag(Keys::Control) || Main.HasFlag(Keys::Shift) || Main.HasFlag(Keys::Alt))
		throw gcnew InvalidEnumArgumentException();
}

KeyCombo::KeyCombo(Keys Main, Keys Modifiers) : Main(Main), Modifiers(Modifiers)
{
	Validate();
}

bool KeyCombo::IsTriggered(KeyEventArgs^ E)
{
	if (Control && !E->Control)
		return false;
	else if (Shift && !E->Shift)
		return false;
	else if (Alt && !E->Alt)
		return false;

	return Main == E->KeyCode;
}

bool KeyCombo::Equals(Object^ obj)
{
	if (obj == nullptr)
		return false;
	else if (obj->GetType() != KeyCombo::typeid)
		return false;

	auto Other = safe_cast<KeyCombo^>(obj);
	return this->Main.Equals(Other->Main) && this->Modifiers.Equals(Other->Modifiers);
}

int KeyCombo::GetHashCode()
{
	int Hash = 7;
	Hash = 31 * Hash + static_cast<int>(Main);
	Hash = 31 * Hash + static_cast<int>(Modifiers);
	return Hash;
}


System::String^ KeyCombo::ToString()
{
	return Modifiers.ToString() + " + " + Main.ToString();
}

KeyCombo^ KeyCombo::FromKeyEvent(KeyEventArgs^ E)
{
	UInt32 Modifiers;
	if (E->Control)
		Modifiers |= safe_cast<UInt32>(Keys::Control);
	if (E->Shift)
		Modifiers |= safe_cast<UInt32>(Keys::Shift);
	if (E->Alt)
		Modifiers |= safe_cast<UInt32>(Keys::Alt);

	return gcnew KeyCombo(E->KeyCode, safe_cast<Keys>(Modifiers));
}

KeyCombo^ KeyCombo::New(Keys Modifier, Keys Key)
{
	return gcnew KeyCombo(Key, Modifier);
}

InputBox::InputBox(void)
{
	InitializeComponent();
}

InputBox::~InputBox()
{
	if (components)
		delete components;
}

void InputBox::InitializeComponent()
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

void InputBox::InputBox_Load(System::Object^ sender, System::EventArgs^ e)
{
	OutputResponse->ReturnCode = Windows::Forms::DialogResult::Ignore;
	OutputResponse->Text = String::Empty;

	txtInput->Text = _defaultValue;
	lblPrompt->Text = _formPrompt;
	Text = _formCaption;

	// Retrieve the working rectangle from the Screen class
	// using the PrimaryScreen and the WorkingArea properties->
	Rectangle workingRectangle = Screen::PrimaryScreen->WorkingArea;

	if ((_xPos >= 0 && _xPos < workingRectangle.Width - 100) && (_yPos >= 0 && _yPos < workingRectangle.Height - 100))
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
	while (PrompText->IndexOf("\n", Index) > -1)
	{
		Index = PrompText->IndexOf("\n", Index) + 1;
		n++;
	}

	if (n == 0)
		n = 1;

	Drawing::Point Txt = txtInput->Location;
	Txt.Y = Txt.Y + (n * 4);
	txtInput->Location = Txt;
	Drawing::Size form = Size;
	form.Height = form.Height + (n * 4);
	Size = form;

	txtInput->SelectionStart = 0;
	txtInput->SelectionLength = txtInput->Text->Length;
	txtInput->Focus();
}

void InputBox::btnOK_Click(Object^ sender, EventArgs^ e)
{
	OutputResponse->ReturnCode = Windows::Forms::DialogResult::OK;
	OutputResponse->Text = txtInput->Text;
	//delete frmInputDialog;
	Close();
}

void InputBox::btnCancel_Click(Object^ sender, EventArgs^ e)
{
	OutputResponse->ReturnCode = Windows::Forms::DialogResult::Cancel;
	OutputResponse->Text = String::Empty; // Clean output response
											//delete frmInputDialog;
	Close();
}

void InputBox::txtInput_KeyDown(Object^ Sender, KeyEventArgs^ E)
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

InputBoxResult^ InputBox::Show(String^ Prompt)
{
	InputBox^ dialogForm = gcnew InputBox();

	dialogForm->FormPrompt = Prompt;
	// 				dialogForm->XPosition    = -1;
	// 				dialogForm->YPosition    = -1;

	// Display the form as a modal dialog box->
	dialogForm->ShowDialog();
	return dialogForm->OutputResponse;
}

InputBoxResult^ InputBox::Show(String^ Prompt, String^ Title)
{
	InputBox^ dialogForm = gcnew InputBox();

	dialogForm->FormCaption = Title;
	dialogForm->FormPrompt = Prompt;
	// 				dialogForm->XPosition    = -1;
	// 				dialogForm->YPosition    = -1;

	// Display the form as a modal dialog box->
	dialogForm->ShowDialog();
	return dialogForm->OutputResponse;
}

InputBoxResult^ InputBox::Show(String^ Prompt, String^ Title, String^ Default)
{
	InputBox^ dialogForm = gcnew InputBox();

	dialogForm->FormCaption = Title;
	dialogForm->FormPrompt = Prompt;
	dialogForm->DefaultValue = Default;
	// 				dialogForm->XPosition    = -1;
	// 				dialogForm->YPosition    = -1;

	// Display the form as a modal dialog box->
	dialogForm->ShowDialog();
	return dialogForm->OutputResponse;
}

InputBoxResult^ InputBox::Show(String^ Prompt, String^ Title, String^ Default, int XPos, int YPos)
{
	InputBox^ dialogForm = gcnew InputBox();

	dialogForm->FormCaption = Title;
	dialogForm->FormPrompt = Prompt;
	dialogForm->DefaultValue = Default;
	dialogForm->XPosition = XPos;
	dialogForm->YPosition = YPos;

	// Display the form as a modal dialog box->
	dialogForm->ShowDialog();
	return dialogForm->OutputResponse;
}

void AnimatedForm::WndProc(Message% m)
{
	const int WM_SYSCOMMAND = 0x0112;
	const int SC_MOVE = 0xF010;
	const int WM_MOVE = 0x003;
	const int WM_MOVING = 0x0216;
	const int WM_ACTIVATE = 0x6;
	const int WM_NCACTIVATE = 0x86;
	const int WA_INACTIVE = 0;

	switch(m.Msg)
	{
	case WM_MOVE:
	case WM_MOVING:
		if (!AllowMove)
			return;
		break;
	case WM_SYSCOMMAND:
	{
		int Command = m.WParam.ToInt32() & 0xfff0;
		if (Command == SC_MOVE && !AllowMove)
			return;
		break;
	}
	/*case WM_NCACTIVATE:
	if (!PreventActivation && ActiveTransition == Transition::None)
	break;

	m.Result = IntPtr::Zero;
	return;*/
	case WM_ACTIVATE:
		if (((int)m.WParam & 0xFFFF) != WA_INACTIVE)
		{
			if (!PreventActivation && ActiveTransition == Transition::None)
				break;

			if (m.LParam != IntPtr::Zero)
				nativeWrapper::SetActiveWindow(m.LParam);
			else
				// Could not find sender, just de-activate it.
				nativeWrapper::SetActiveWindow(IntPtr::Zero);
		}
		break;
	}

	Form::WndProc(m);
}

void AnimatedForm::FadeTimer_Tick(Object^ Sender, EventArgs^ E)
{
	const auto kTransitionTime = 100;		// in ms

	auto NumTicksReqd = kTransitionTime / static_cast<double>(FadeTimer->Interval);
	auto PerTickDelta = 1.0 / NumTicksReqd;

	if (ActiveTransition == Transition::FadeIn)
		this->Opacity += PerTickDelta;
	else if (ActiveTransition == Transition::FadeOut)
		this->Opacity -= PerTickDelta;

	if (this->Opacity >= 1.0 || this->Opacity <= 0.0)
		EndTransition(nullptr);
}

void AnimatedForm::ShowFormDiscreetly(IntPtr ParentWindowHandle)
{
	this->Opacity = 0;
	if (ParentWindowHandle != IntPtr::Zero)
		Form::Show(gcnew WindowHandleWrapper(ParentWindowHandle));
	else
		Form::Show();
}

void AnimatedForm::StartTransition(StartTransitionParams^ Params)
{
	Debug::Assert(ClosingForm == false);
	Debug::Assert(ActiveTransition == Transition::None);
	Debug::Assert(ActiveTransitionEndState == TransitionFinalState::None);
	Debug::Assert(FadeTimer->Enabled == false);

	switch (Params->EndState)
	{
	case TransitionFinalState::Show:
		if (Params->UsePosition)
			SetDesktopLocation(Params->Position.X, Params->Position.Y);

		if (!Visible)
			ShowFormDiscreetly(Params->ParentWindowHandle);

		if (Params->Animate)
		{
			ActiveTransition = Transition::FadeIn;
			this->Opacity = 0;
		}

		break;
	case TransitionFinalState::Hide:
	case TransitionFinalState::Close:
		if (!Visible)
			Form::Show();

		if (Params->Animate)
		{
			ActiveTransition = Transition::FadeOut;
			this->Opacity = 1;
		}

		break;
	}

	ActiveTransitionEndState = Params->EndState;
	if (!Params->Animate)
	{
		// end the transition right away
		EndTransition(Params);
	}
	else
		FadeTimer->Start();
}

void AnimatedForm::EndTransition(StartTransitionParams^ StartParams)
{
	if (ActiveTransitionEndState == TransitionFinalState::None)
		return;
	else if (ClosingForm)
		return;

	FadeTimer->Stop();

	if (ActiveTransitionCompleteHandler)
		ActiveTransitionCompleteHandler(this);

	switch (ActiveTransitionEndState)
	{
	case TransitionFinalState::Hide:
		Form::Hide();
		this->Opacity = 1;
		break;
	case TransitionFinalState::Show:
		if (StartParams == nullptr)
			Form::BringToFront();
		this->Opacity = 1;
		break;
	case TransitionFinalState::Close:
		Form::Close();
		ClosingForm = true;
		break;
	}

	ActiveTransition = Transition::None;
	ActiveTransitionEndState = TransitionFinalState::None;
	ActiveTransitionCompleteHandler = nullptr;
}

AnimatedForm::AnimatedForm( bool ShowFormWithoutActivation )
	: System::Windows::Forms::Form()
{
	ActiveTransition = Transition::None;
	ActiveTransitionEndState = TransitionFinalState::None;
	ActiveTransitionCompleteHandler = nullptr;

	this->ShowFormWithoutActivation = ShowFormWithoutActivation;

	FadeTimerTickHandler = gcnew EventHandler(this, &AnimatedForm::FadeTimer_Tick);
	FadeTimer = gcnew Timer();
	FadeTimer->Interval = 10;
	FadeTimer->Tick += FadeTimerTickHandler;
	FadeTimer->Enabled = false;

	AllowMove = true;
	PreventActivation = false;
	ClosingForm = false;
}

AnimatedForm::~AnimatedForm()
{
	if (ClosingForm)
		return;

	ClosingForm = true;

	FadeTimer->Tick -= FadeTimerTickHandler;
	delete FadeTimer;
}

void AnimatedForm::Show()
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	if (Visible)
		return;

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = TransitionFinalState::Show;
	StartTransition(Params);
}

void AnimatedForm::Show(IntPtr ParentHandle)
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	if (Visible)
		return;

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = TransitionFinalState::Show;
	Params->ParentWindowHandle = ParentHandle;
	StartTransition(Params);
}

void AnimatedForm::Show(Drawing::Point Position, IntPtr ParentHandle, bool Animate)
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = TransitionFinalState::Show;
	Params->ParentWindowHandle = ParentHandle;
	Params->Position = Position;
	Params->UsePosition = true;
	Params->Animate = Animate;
	StartTransition(Params);
}

void AnimatedForm::Hide()
{
	Hide(true);
}

void AnimatedForm::Hide(bool Animate)
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	if (!Visible)
		return;

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = TransitionFinalState::Hide;
	Params->Animate = Animate;
	StartTransition(Params);
}

void AnimatedForm::Close()
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = TransitionFinalState::Close;
	StartTransition(Params);
}

void AnimatedForm::ForceClose()
{
	Form::Close();
}

void AnimatedForm::SetSize(Drawing::Size WindowSize)
{
	ClientSize = WindowSize;

	WindowSize.Height += 3;
	MaximumSize = WindowSize;
	MinimumSize = WindowSize;
}

void AnimatedForm::SetNextActiveTransitionCompleteHandler(TransitionCompleteHandler^ NewHandler)
{
	EndTransition(nullptr);

	ActiveTransitionCompleteHandler = NewHandler;
}


} // namespace scriptEditor


} // namespace cse