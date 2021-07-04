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
	String^ ModifierString = "";
	if (Control)
		ModifierString += "Ctrl";
	if (Shift)
		ModifierString += (Control ? " + " : "") + "Shift";
	if (Alt)
		ModifierString += (Control || Shift ? " + " : "") + "Alt";

	return ModifierString->Trim() + " + " + Main.ToString();
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
	Result = gcnew InputBoxResult;
	Result->ReturnCode = Windows::Forms::DialogResult::Ignore;
	Result->Text = String::Empty;
}

InputBox::~InputBox()
{
}

void InputBox::InitializeComponent()
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
	this->btnOK->TabIndex = 2;
	this->btnOK->Text = L"OK";
	this->btnOK->Click += gcnew System::EventHandler(this, &InputBox::btnOK_Click);
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
	this->btnCancel->TabIndex = 3;
	this->btnCancel->Text = L"Cancel";
	this->btnCancel->Click += gcnew System::EventHandler(this, &InputBox::btnCancel_Click);
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
	this->txtInput->TabIndex = 1;
	this->txtInput->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &InputBox::txtInput_KeyDown);
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
	this->Load += gcnew System::EventHandler(this, &InputBox::InputBox_Load);
	this->ResumeLayout(false);
}

void InputBox::InputBox_Load(System::Object^ sender, System::EventArgs^ e)
{
	txtInput->SelectionStart = 0;
	txtInput->SelectionLength = txtInput->Text->Length;
	txtInput->Focus();
}

void InputBox::btnOK_Click(Object^ sender, EventArgs^ e)
{
	Result->ReturnCode = Windows::Forms::DialogResult::OK;
	Result->Text = txtInput->Text;
	Close();
}

void InputBox::btnCancel_Click(Object^ sender, EventArgs^ e)
{
	Result->ReturnCode = Windows::Forms::DialogResult::Cancel;
	Result->Text = String::Empty;
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

InputBoxResult^ InputBox::Show(String^ Prompt, String^ Title, String^ Default, IntPtr ParentWindowHandle)
{
	auto Form = gcnew InputBox();

	Form->txtInput->Text = Default;
	Form->lblPrompt->Text = Prompt;
	Form->Text = Title;

	Form->ShowDialog(gcnew WindowHandleWrapper(ParentWindowHandle));
	return Form->Result;
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
	case WM_ACTIVATE:
		if (((int)m.WParam & 0xFFFF) != WA_INACTIVE)
		{
			if (!PreventActivation && ActiveTransition == eTransition::None)
				break;

			if (m.LParam != IntPtr::Zero)
				nativeWrapper::SetActiveWindow(m.LParam);
			else
				nativeWrapper::SetActiveWindow(IntPtr::Zero);	// Could not find sender, just de-activate it.

			m.Result = IntPtr::Zero;
			return;
		}

		break;
	}

	MetroForm::WndProc(m);
}

AnimatedForm::StartTransitionParams::StartTransitionParams()
{
	EndState = eTransitionFinalState::None;
	ParentWindowHandle = IntPtr::Zero;
	Position = Point(0, 0);
	UsePosition = false;
	Animate = true;
}

void AnimatedForm::FadeTimer_Tick(Object^ Sender, EventArgs^ E)
{
	const auto kTransitionTime = 100;		// in ms

	auto NumTicksReqd = kTransitionTime / static_cast<double>(FadeTimer->Interval);
	auto PerTickDelta = 1.0 / NumTicksReqd;

	if (ActiveTransition == eTransition::FadeIn)
		this->Opacity += PerTickDelta;
	else if (ActiveTransition == eTransition::FadeOut)
		this->Opacity -= PerTickDelta;

	if (this->Opacity >= 1.0 || this->Opacity <= 0.0)
		EndTransition(nullptr);
}

void AnimatedForm::ShowFormDiscreetly(IntPtr ParentWindowHandle)
{
	this->Opacity = 0;
	if (ParentWindowHandle != IntPtr::Zero)
		MetroForm::Show(gcnew WindowHandleWrapper(ParentWindowHandle));
	else
		MetroForm::Show();
}

void AnimatedForm::StartTransition(StartTransitionParams^ Params)
{
	Debug::Assert(ClosingForm == false);
	Debug::Assert(ActiveTransition == eTransition::None);
	Debug::Assert(ActiveTransitionEndState == eTransitionFinalState::None);
	Debug::Assert(FadeTimer->Enabled == false);

	switch (Params->EndState)
	{
	case eTransitionFinalState::Show:
		if (Params->UsePosition)
			SetDesktopLocation(Params->Position.X, Params->Position.Y);

		if (!Visible)
			ShowFormDiscreetly(Params->ParentWindowHandle);

		if (Params->Animate)
		{
			ActiveTransition = eTransition::FadeIn;
			this->Opacity = 0;
		}

		break;
	case eTransitionFinalState::Hide:
	case eTransitionFinalState::Close:
		if (!Visible)
			MetroForm::Show();

		if (Params->Animate)
		{
			ActiveTransition = eTransition::FadeOut;
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
	if (ActiveTransitionEndState == eTransitionFinalState::None)
		return;
	else if (ClosingForm)
		return;

	FadeTimer->Stop();

	if (ActiveTransitionCompleteHandler)
		ActiveTransitionCompleteHandler(this);

	switch (ActiveTransitionEndState)
	{
	case eTransitionFinalState::Hide:
		MetroForm::Hide();
		this->Opacity = 1;
		break;
	case eTransitionFinalState::Show:
		if (StartParams == nullptr)
			MetroForm::BringToFront();
		this->Opacity = 1;
		break;
	case eTransitionFinalState::Close:
		MetroForm::Close();
		ClosingForm = true;
		break;
	}

	ActiveTransition = eTransition::None;
	ActiveTransitionEndState = eTransitionFinalState::None;
	ActiveTransitionCompleteHandler = nullptr;
}

AnimatedForm::AnimatedForm( bool ShowFormWithoutActivation )
{
	ActiveTransition = eTransition::None;
	ActiveTransitionEndState = eTransitionFinalState::None;
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
	Params->EndState = eTransitionFinalState::Show;
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
	Params->EndState = eTransitionFinalState::Show;
	Params->ParentWindowHandle = ParentHandle;
	StartTransition(Params);
}

void AnimatedForm::Show(Drawing::Point Position, IntPtr ParentHandle, bool Animate)
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = eTransitionFinalState::Show;
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
	Params->EndState = eTransitionFinalState::Hide;
	Params->Animate = Animate;
	StartTransition(Params);
}

void AnimatedForm::Close()
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = eTransitionFinalState::Close;
	StartTransition(Params);
}

void AnimatedForm::ForceClose()
{
	MetroForm::Close();
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