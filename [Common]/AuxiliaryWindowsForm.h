#pragma once

namespace cse
{
	ref class AnimatedForm : public System::Windows::Forms::Form
	{
	protected:
		virtual property bool DoubleBuffered
		{
			virtual bool get() override { return true; }
		}

		virtual property bool ShowWithoutActivation
		{
			virtual bool get() override { return ShowFormWithoutActivation; }
		}

		virtual void WndProc(Message% m) override;

		static enum class					FadeOperationType
		{
			None = 0,
			FadeIn,
			FadeOut
		};

		FadeOperationType					FadeOperation;
		Timer^								FadeTimer;
		double								FadeDuration;
		bool								CloseOnFadeOut;
		EventHandler^						FadeTimerTickHandler;
		double								RemainingTime;
		bool								ShowFormWithoutActivation;

		void								FadeTimer_Tick(Object^ Sender, EventArgs^ E);
	public:
		AnimatedForm(double FadeDuration, bool ShowFormWithoutActivation);
		~AnimatedForm();

		property bool AllowMove;
		property bool PreventActivation;

		property bool IsFadingIn
		{
			bool get() { return FadeOperation == FadeOperationType::FadeIn; }
		}
		property bool IsFadingOut
		{
			bool get() { return FadeOperation == FadeOperationType::FadeOut; }
		}

		void									SetSize(Drawing::Size WindowSize);
		void									Show();
		void									Show(IWin32Window^ Parent);
		void									Show(Drawing::Point Position, IntPtr ParentHandle, bool Animate);
		System::Windows::Forms::DialogResult	ShowDialog();
		void									Hide();
		void									Close();
		virtual void							ForceClose();
	};
}