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
		const auto kTransitionTime = 1000;		// in ms

		double NumTicksReqd = kTransitionTime / static_cast<double>(FadeTimer->Interval);
		double PerTickDelta = 1.0 / NumTicksReqd;

		if (ActiveTransition == Transition::FadeIn)
			this->Opacity += PerTickDelta;
		else if (ActiveTransition == Transition::FadeOut)
			this->Opacity -= PerTickDelta;

		if (this->Opacity >= 1.0 || this->Opacity <= 0.0)
			EndTransition();
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

			if (Params->ParentWindowHandle != IntPtr::Zero)
				Form::Show(gcnew WindowHandleWrapper(Params->ParentWindowHandle));
			else
				Form::Show();

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
			EndTransition();
		}
		else
			FadeTimer->Start();
	}

	void AnimatedForm::EndTransition()
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
		FadeTimer->Interval = 4;
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

		EndTransition();
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

		EndTransition();

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

		EndTransition();

		auto Params = gcnew StartTransitionParams;
		Params->EndState = TransitionFinalState::Show;
		Params->ParentWindowHandle = ParentHandle;
		StartTransition(Params);
	}

	void AnimatedForm::Show(Drawing::Point Position, IntPtr ParentHandle, bool Animate)
	{
		if (ClosingForm)
			throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

		if (Visible)
			return;

		EndTransition();

		auto Params = gcnew StartTransitionParams;
		Params->EndState = TransitionFinalState::Show;
		Params->ParentWindowHandle = ParentHandle;
		Params->Position = Position;
		Params->Animate = Animate;
		StartTransition(Params);
	}

	void AnimatedForm::Hide()
	{
		if (ClosingForm)
			throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

		if (!Visible)
			return;

		EndTransition();

		auto Params = gcnew StartTransitionParams;
		Params->EndState = TransitionFinalState::Hide;
		StartTransition(Params);
	}

	void AnimatedForm::Close()
	{
		if (ClosingForm)
			throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

		EndTransition();

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
		EndTransition();

		ActiveTransitionCompleteHandler = NewHandler;
	}
}