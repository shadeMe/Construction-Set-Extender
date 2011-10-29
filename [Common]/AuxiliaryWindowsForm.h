#pragma once

namespace ConstructionSetExtender
{
	public ref class AnimatedForm : public System::Windows::Forms::Form
	{
	protected:
		static enum class					FadeOperationType
		{
			e_None = 0,
			e_FadeIn,
			e_FadeOut
		};

		FadeOperationType					FadeOperation;
		Timer^								FadeTimer;
		double								FadeDuration;
		bool								CloseOnFadeOut;
		EventHandler^						FadeTimerTickHandler;

		static double						FadeAnimationFactor = 0.60;

		void								FadeTimer_Tick(Object^ Sender, EventArgs^ E);

		void								Destroy();
	public:
		AnimatedForm(double FadeDuration);
		virtual ~AnimatedForm()
		{
			Destroy();
		}

		void										Show();
		void										Show(IWin32Window^ Parent);
		System::Windows::Forms::DialogResult		ShowDialog();
		void										Hide();
		void										Close();
		void										ForceClose();
	};

	public ref class NonActivatingImmovableAnimatedForm : public Form
	{
	protected:
		property bool										ShowWithoutActivation
		{
			virtual bool									get() override { return true; }
		}

		virtual void										WndProc(Message% m) override;

		static enum class									FadeOperationType
		{
			e_None = 0,
			e_FadeIn,
			e_FadeOut
		};

		bool												AllowMove;
		FadeOperationType									FadeOperation;
		Timer^												FadeTimer;
		EventHandler^										FadeTimerTickHandler;

		void												FadeTimer_Tick(Object^ Sender, EventArgs^ E);

		void												Destroy();
	public:
		virtual ~NonActivatingImmovableAnimatedForm()
		{
			Destroy();
		}

		property bool										PreventActivation;

		void												SetSize(Drawing::Size WindowSize);
		void												ShowForm(Drawing::Point Position, IntPtr ParentHandle, bool Animate);
		void												HideForm(bool Animate);

		NonActivatingImmovableAnimatedForm();
	};
}