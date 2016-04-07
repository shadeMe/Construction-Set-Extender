#pragma once

namespace ConstructionSetExtender
{
	ref class AnimatedForm : public System::Windows::Forms::Form
	{
	protected:
		virtual property bool DoubleBuffered
		{
			bool get() override { return true; }
		}

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

		void								FadeTimer_Tick(Object^ Sender, EventArgs^ E);
	public:
		AnimatedForm(double FadeDuration);
		~AnimatedForm();

		void												Show();
		void												Show(IWin32Window^ Parent);
		System::Windows::Forms::DialogResult				ShowDialog();
		void												Hide();
		void												Close();
		virtual void										ForceClose();
	};

	ref class NonActivatingImmovableAnimatedForm : public Form
	{
	protected:
		virtual property bool DoubleBuffered
		{
			bool get() override { return true; }
		}

		property bool										ShowWithoutActivation
		{
			virtual bool									get() override { return true; }
		}

		virtual void										WndProc(Message% m) override;

		static enum class									FadeOperationType
		{
			None = 0,
			FadeIn,
			FadeOut
		};

		bool												AllowMove;
		FadeOperationType									FadeOperation;
		Timer^												FadeTimer;
		EventHandler^										FadeTimerTickHandler;

		void												FadeTimer_Tick(Object^ Sender, EventArgs^ E);
	public:
		NonActivatingImmovableAnimatedForm();
		~NonActivatingImmovableAnimatedForm();

		property bool										PreventActivation;
		property bool										FadingIn
		{
			bool get() { return FadeOperation == FadeOperationType::FadeIn;  }
		}

		void												SetSize(Drawing::Size WindowSize);
		void												ShowForm(Drawing::Point Position, IntPtr ParentHandle, bool Animate);
		void												HideForm(bool Animate);
	};
}