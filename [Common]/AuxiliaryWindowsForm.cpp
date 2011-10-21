#include "AuxiliaryWindowsForm.h"

void AnimatedForm::FadeTimer_Tick( Object^ Sender, EventArgs^ E )
{
	if (FadeOperation == FadeOperationType::e_FadeIn)
		this->Opacity += FadeTimer->Interval / (FadeAnimationFactor * FadeDuration * 1000);
	else
		this->Opacity -= FadeTimer->Interval / (FadeAnimationFactor * FadeDuration * 1000);

	if (this->Opacity >= 1.0 || this->Opacity <= 0.0)
	{
		FadeTimer->Stop();

		if (FadeOperation == FadeOperationType::e_FadeOut)
		{
			if (CloseOnFadeOut)
				Form::Close();
			else
				Form::Hide();
		}
	}
}

void AnimatedForm::Show()
{
	this->Opacity = 0.0;
	Form::Show();

	FadeOperation = FadeOperationType::e_FadeIn;
	FadeTimer->Start();
}

void AnimatedForm::Show( IWin32Window^ Parent )
{
	this->Opacity = 0.0;
	Form::Show(Parent);

	FadeOperation = FadeOperationType::e_FadeIn;
	FadeTimer->Start();
}

System::Windows::Forms::DialogResult AnimatedForm::ShowDialog()
{
	this->Opacity = 0.0;

	FadeOperation = FadeOperationType::e_FadeIn;
	FadeTimer->Start();

	return Form::ShowDialog();
}

void AnimatedForm::Hide()
{
	if (this->Visible)
	{
		FadeOperation = FadeOperationType::e_FadeOut;
		this->Opacity = 1.0;
		FadeTimer->Start();
	}
}

AnimatedForm::AnimatedForm( double FadeDuration ) : System::Windows::Forms::Form()
{
	this->FadeDuration = FadeDuration;
	FadeOperation = FadeOperationType::e_None;
	CloseOnFadeOut = false;

	FadeTimer = gcnew Timer();
	FadeTimer->Interval = 10;
	FadeTimer->Tick += gcnew EventHandler(this, &AnimatedForm::FadeTimer_Tick);
	FadeTimer->Stop();
}

void AnimatedForm::Close()
{
	CloseOnFadeOut = true;
	FadeOperation = FadeOperationType::e_FadeOut;
	this->Opacity = 1.0;
	FadeTimer->Start();
}

void AnimatedForm::Destroy()
{
	FadeTimer->Stop();
}

void NonActivatingImmovableAnimatedForm::FadeTimer_Tick( Object^ Sender, EventArgs^ E )
{
	if (FadeOperation == FadeOperationType::e_FadeIn)
		this->Opacity += FadeTimer->Interval / (0.6 * 0.15 * 1000);
	else
		this->Opacity -= FadeTimer->Interval / (0.6 * 0.15 * 1000);

	if (this->Opacity >= 1.0 || this->Opacity <= 0.0)
	{
		FadeTimer->Stop();

		if (FadeOperation == FadeOperationType::e_FadeOut)
			this->Hide();
	}
}

void NonActivatingImmovableAnimatedForm::SetSize(Drawing::Size WindowSize)
{
	ClientSize = WindowSize;

	WindowSize.Height += 3;
	MaximumSize = WindowSize;
	MinimumSize = WindowSize;
}

void NonActivatingImmovableAnimatedForm::ShowForm(Drawing::Point Position, IntPtr ParentHandle, bool Animate)
{
	AllowMove = true;

	SetDesktopLocation(Position.X, Position.Y);
	if (this->Visible == false)
	{
		if (Animate)
			this->Opacity = 0.0;

		if (ParentHandle != IntPtr::Zero)
			Show(gcnew WindowHandleWrapper(ParentHandle));
		else
			Show();

		if (Animate)
		{
			FadeOperation = FadeOperationType::e_FadeIn;
			FadeTimer->Start();
		}
	}

	AllowMove = false;
}

void NonActivatingImmovableAnimatedForm::WndProc(Message% m)
{
	const int WM_SYSCOMMAND = 0x0112;
	const int SC_MOVE = 0xF010;
	const int WM_MOVE = 0x003;
	const int WM_MOVING = 0x0216;

	switch(m.Msg)
	{
	case WM_MOVE:
	case WM_MOVING:
		if (!AllowMove)
			return;
		break;
	case WM_SYSCOMMAND:
		int Command = m.WParam.ToInt32() & 0xfff0;
		if (Command == SC_MOVE && !AllowMove)
			return;
		break;
	}

	Form::WndProc(m);
}

NonActivatingImmovableAnimatedForm::NonActivatingImmovableAnimatedForm() : Form()
{
	AllowMove = false;
	FadeOperation = FadeOperationType::e_None;

	FadeTimer = gcnew Timer();
	FadeTimer->Interval = 10;
	FadeTimer->Tick += gcnew EventHandler(this, &NonActivatingImmovableAnimatedForm::FadeTimer_Tick);
	FadeTimer->Stop();
}

void NonActivatingImmovableAnimatedForm::HideForm(bool Animate)
{
	if (this->Visible)
	{
		if (Animate)
		{
			FadeOperation = FadeOperationType::e_FadeOut;
			this->Opacity = 1.0;
			FadeTimer->Start();
		}
		else
			this->Hide();
	}
}

void NonActivatingImmovableAnimatedForm::Destroy()
{
	FadeTimer->Stop();
}