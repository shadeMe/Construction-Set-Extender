#include "AuxiliaryWindowsForm.h"
#include "NativeWrapper.h"

namespace cse
{
	void AnimatedForm::WndProc(Message% m)
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
			if (((int)m.WParam & 0xFFFF) != WA_INACTIVE)
			{
				if (!PreventActivation && FadeOperation == FadeOperationType::None)
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
		RemainingTime -= FadeTimer->Interval / 1000.0;

		if (FadeOperation == FadeOperationType::FadeIn)
			this->Opacity += FadeTimer->Interval / (RemainingTime * 1000.0);
		else
			this->Opacity -= FadeTimer->Interval / (RemainingTime * 1000.0);

		if (this->Opacity >= 1.0 || this->Opacity <= 0.0)
		{
			FadeTimer->Stop();

			if (FadeOperation == FadeOperationType::FadeOut)
			{
				if (CloseOnFadeOut)
					Form::Close();
				else
					Form::Hide();
			}
			else
				Form::BringToFront();

			FadeOperation = FadeOperationType::None;
		}
	}

	void AnimatedForm::Show()
	{
		this->Opacity = 0.0;
		RemainingTime = FadeDuration;
		Form::Show();

		FadeOperation = FadeOperationType::FadeIn;
		FadeTimer->Start();
	}

	void AnimatedForm::Show( IWin32Window^ Parent )
	{
		this->Opacity = 0.0;
		RemainingTime = FadeDuration;
		Form::Show(Parent);

		FadeOperation = FadeOperationType::FadeIn;
		FadeTimer->Start();
	}

	void AnimatedForm::Show(Drawing::Point Position, IntPtr ParentHandle, bool Animate)
	{
		SetDesktopLocation(Position.X, Position.Y);
		if (Visible)
			return;

		if (!Animate)
		{
			this->Opacity = 1.0;
			if (ParentHandle != IntPtr::Zero)
				Form::Show(gcnew WindowHandleWrapper(ParentHandle));
			else
				Form::Show();

			return;
		}

		if (ParentHandle != IntPtr::Zero)
			Show(gcnew WindowHandleWrapper(ParentHandle));
		else
			Show();
	}

	System::Windows::Forms::DialogResult AnimatedForm::ShowDialog()
	{
		this->Opacity = 0.0;
		RemainingTime = FadeDuration;

		FadeOperation = FadeOperationType::FadeIn;
		FadeTimer->Start();

		return Form::ShowDialog();
	}

	void AnimatedForm::Hide()
	{
		if (Visible)
		{
			FadeOperation = FadeOperationType::FadeOut;
			this->Opacity = 1.0;
			FadeTimer->Start();
			RemainingTime = FadeDuration;
		}
	}

	AnimatedForm::AnimatedForm( double FadeDuration ) : System::Windows::Forms::Form()
	{
		this->FadeDuration = FadeDuration;
		FadeOperation = FadeOperationType::None;
		CloseOnFadeOut = false;
		RemainingTime = 0.0;

		FadeTimer = gcnew Timer();
		FadeTimerTickHandler = gcnew EventHandler(this, &AnimatedForm::FadeTimer_Tick);
		FadeTimer->Interval = 10;
		FadeTimer->Tick += FadeTimerTickHandler;
		FadeTimer->Stop();

		AllowMove = true;
		PreventActivation = false;
	}

	void AnimatedForm::Close()
	{
		CloseOnFadeOut = true;
		FadeOperation = FadeOperationType::FadeOut;
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

	void AnimatedForm::SetSize(Drawing::Size WindowSize)
	{
		ClientSize = WindowSize;

		WindowSize.Height += 3;
		MaximumSize = WindowSize;
		MinimumSize = WindowSize;
	}
}