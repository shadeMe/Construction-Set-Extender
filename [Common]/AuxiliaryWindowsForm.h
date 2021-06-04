#pragma once

namespace cse
{
	ref class AnimatedForm : public System::Windows::Forms::Form
	{
	public:
		delegate void TransitionCompleteHandler(AnimatedForm^ Sender);
	protected:
		virtual property bool DoubleBuffered
		{
			bool get() override { return true; }
		}

		virtual property bool ShowWithoutActivation
		{
			bool get() override { return ShowFormWithoutActivation; }
		}

		virtual void WndProc(Message% m) override;

		static enum class Transition
		{
			None = 0,
			FadeIn,
			FadeOut
		};

		static enum class TransitionFinalState
		{
			None = 0,
			Show,
			Hide,
			Close
		};

		ref struct StartTransitionParams
		{
			TransitionFinalState	EndState;
			IntPtr					ParentWindowHandle;
			Point					Position;
			bool					UsePosition;
			bool					Animate;

			StartTransitionParams()
			{
				EndState = TransitionFinalState::None;
				ParentWindowHandle = IntPtr::Zero;
				Position = Point(0, 0);
				UsePosition = false;
				Animate = true;
			}
		};

		Transition
				ActiveTransition;
		TransitionFinalState
				ActiveTransitionEndState;
		TransitionCompleteHandler^
				ActiveTransitionCompleteHandler;

		bool	ShowFormWithoutActivation;

		Timer^	FadeTimer;
		EventHandler^
				FadeTimerTickHandler;
		bool	ClosingForm;

		void	FadeTimer_Tick(Object^ Sender, EventArgs^ E);

		void	ShowFormDiscreetly(IntPtr ParentWindowHandle);
		void	StartTransition(StartTransitionParams^ Params);
		void	EndTransition(StartTransitionParams^ StartParams);
	public:
		AnimatedForm(bool ShowFormWithoutActivation);
		virtual ~AnimatedForm();

		property bool AllowMove;
		property bool PreventActivation;
		property bool IsFadingIn
		{
			bool get() { return ActiveTransition == Transition::FadeIn; }
		}
		property bool IsFadingOut
		{
			bool get() { return ActiveTransition == Transition::FadeOut; }
		}

		void	Show();
		void	Show(IntPtr ParentHandle);
		void	Show(Drawing::Point Position, IntPtr ParentHandle, bool Animate);
		void	Hide();
		void	Hide(bool Animate);
		void	Close();
		void	ForceClose();

		void	SetSize(Drawing::Size WindowSize);
		void	SetNextActiveTransitionCompleteHandler(TransitionCompleteHandler^ NewHandler);
	};
}