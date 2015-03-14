#include "AuxiliaryWindowsForm.h"
#include "NativeWrapper.h"

namespace ConstructionSetExtender
{
	void AnimatedForm::FadeTimer_Tick( Object^ Sender, EventArgs^ E )
	{
		RemainingTime -= FadeTimer->Interval / 1000.0;

		if (FadeOperation == FadeOperationType::e_FadeIn)
			this->Opacity += FadeTimer->Interval / (RemainingTime * 1000.0);
		else
			this->Opacity -= FadeTimer->Interval / (RemainingTime * 1000.0);

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
		RemainingTime = FadeDuration;
		Form::Show();

		FadeOperation = FadeOperationType::e_FadeIn;
		FadeTimer->Start();
	}

	void AnimatedForm::Show( IWin32Window^ Parent )
	{
		this->Opacity = 0.0;
		RemainingTime = FadeDuration;
		Form::Show(Parent);

		FadeOperation = FadeOperationType::e_FadeIn;
		FadeTimer->Start();
	}

	System::Windows::Forms::DialogResult AnimatedForm::ShowDialog()
	{
		this->Opacity = 0.0;
		RemainingTime = FadeDuration;

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
			RemainingTime = FadeDuration;
		}
	}

	AnimatedForm::AnimatedForm( double FadeDuration ) : System::Windows::Forms::Form()
	{
		this->FadeDuration = FadeDuration;
		FadeOperation = FadeOperationType::e_None;
		CloseOnFadeOut = false;
		RemainingTime = 0.0;

		FadeTimer = gcnew Timer();
		FadeTimerTickHandler = gcnew EventHandler(this, &AnimatedForm::FadeTimer_Tick);
		FadeTimer->Interval = 10;
		FadeTimer->Tick += FadeTimerTickHandler;
		FadeTimer->Stop();
	}

	void AnimatedForm::Close()
	{
		CloseOnFadeOut = true;
		FadeOperation = FadeOperationType::e_FadeOut;
		this->Opacity = 1.0;
		RemainingTime = FadeDuration;
		FadeTimer->Start();
	}

	void AnimatedForm::ForceClose()
	{
		Form::Close();
	}

	AnimatedForm::~AnimatedForm()
	{
		FadeTimer->Stop();
		FadeTimer->Tick -= FadeTimerTickHandler;
		delete FadeTimer;
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
			{
				Show(gcnew WindowHandleWrapper(ParentHandle));
			}
			else
			{
				Show();
			}

			BringToFront();

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
		const int WM_ACTIVATE = 0x6;
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
			if (PreventActivation == false && ((int)m.WParam & 0xFFFF) != WA_INACTIVE)
			{
				if (m.LParam != IntPtr::Zero)
				{
					NativeWrapper::SetActiveWindow(m.LParam);
				}
				else
				{
					// Could not find sender, just in-activate it.
					NativeWrapper::SetActiveWindow(IntPtr::Zero);
				}
			}
			break;
		}

		Form::WndProc(m);
	}

	NonActivatingImmovableAnimatedForm::NonActivatingImmovableAnimatedForm() : Form()
	{
		AllowMove = false;
		FadeOperation = FadeOperationType::e_None;

		FadeTimer = gcnew Timer();
		FadeTimerTickHandler = gcnew EventHandler(this, &NonActivatingImmovableAnimatedForm::FadeTimer_Tick);
		FadeTimer->Interval = 10;
		FadeTimer->Tick += FadeTimerTickHandler;
		FadeTimer->Stop();

		PreventActivation = false;
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

	NonActivatingImmovableAnimatedForm::~NonActivatingImmovableAnimatedForm()
	{
		FadeTimer->Stop();
		FadeTimer->Tick -= FadeTimerTickHandler;
		delete FadeTimer;
	}
}