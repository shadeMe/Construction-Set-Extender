#pragma once


namespace cse
{


namespace scriptEditor
{


ref class IAction abstract
{
protected:
	String^ Name_;
	String^ Description_;
public:
	IAction(String^ Name, String^ Description);

	property String^ Name
	{
		String^ get() { return Name_; }
	}
	property String^ Description
	{
		String^ get() { return Description_; }
	}

	virtual void Invoke() = 0;
};

ref class BasicAction : public IAction
{
public:
	delegate void InvokationDelegate();

	BasicAction(String^ Name, String^ Description);
	virtual ~BasicAction();

	property InvokationDelegate^ InvokeDelegate;

	virtual void Invoke() override;
};


ref struct KeyCombo
{
	Keys Main;
	Keys Modifiers;

	void Validate();

	KeyCombo(Keys Main, Keys Modifiers);
public:
	property Keys Key
	{
		Keys get() { return Main; }
	}
	property bool Control
	{
		bool get() { return Modifiers.HasFlag(Keys::Control); }
	}
	property bool Shift
	{
		bool get() { return Modifiers.HasFlag(Keys::Shift); }
	}
	property bool Alt
	{
		bool get() { return Modifiers.HasFlag(Keys::Alt); }
	}

	bool IsTriggered(KeyEventArgs^ E);
	virtual bool Equals(Object^ obj) override;
	virtual int GetHashCode() override;
	virtual String^ ToString() override;

	static KeyCombo^ FromKeyEvent(KeyEventArgs^ E);
	static KeyCombo^ New(Keys Modifier, Keys Key);
};


ref struct InputBoxResult
{
	DialogResult ReturnCode;
	String^ Text;
};

ref class InputBox : public DevComponents::DotNetBar::Metro::MetroForm
{
	static String^ _formCaption = String::Empty;
	static String^ _formPrompt = String::Empty;
	static InputBoxResult^ _outputResponse = gcnew InputBoxResult();
	static String^ _defaultValue = String::Empty;
	static int _xPos = -1;
	static int _yPos = -1;

	static property String^ FormCaption
	{
		void set(String^ value)
		{
			_formCaption = value;
		}
	}
	static property String^ FormPrompt
	{
		void set(String^ value)
		{
			_formPrompt = value;
		}
	}
	static property InputBoxResult^ OutputResponse
	{
		InputBoxResult^ get()
		{
			return _outputResponse;
		}
		void set(InputBoxResult^ value)
		{
			_outputResponse = value;
		}
	}
	static property String^ DefaultValue
	{
		void set(String^ value)
		{
			_defaultValue = value;
		}
	}
	static property int XPosition
	{
		void set(int value)
		{
			//	if (value >= 0)
			_xPos = value;
		}
	}
	static property int YPosition
	{
		void set(int value)
		{
			//	if (value >= 0)
			_yPos = value;
		}
	}

	System::Windows::Forms::Label^ lblPrompt;
	System::Windows::Forms::Button^ btnOK;
	System::Windows::Forms::Button^ btnCancel;
	System::Windows::Forms::TextBox^ txtInput;
	System::ComponentModel::Container^ components;

	void txtInput_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void InitializeComponent(void);
	void InputBox_Load(System::Object^  sender, System::EventArgs^  e);

	void btnOK_Click(Object^ sender, EventArgs^ e);
	void btnCancel_Click(Object^ sender, EventArgs^ e);
public:
	InputBox(void);
	~InputBox();

	static InputBoxResult^ Show(String^ Prompt);
	static InputBoxResult^ Show(String^ Prompt,String^ Title);
	static InputBoxResult^ Show(String^ Prompt,String^ Title,String^ Default);
	static InputBoxResult^ Show(String^ Prompt,String^ Title,String^ Default,int XPos,int YPos);
};


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


} // namespace scriptEditor


} // namespace cse