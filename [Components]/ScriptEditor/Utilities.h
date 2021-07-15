#pragma once


namespace cse
{


namespace scriptEditor
{


namespace utilities
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
	Windows::Forms::DialogResult ReturnCode;
	String^ Text;
};

ref class InputBox : public DevComponents::DotNetBar::Metro::MetroForm
{
	DevComponents::DotNetBar::LabelX^ lblPrompt;
	DevComponents::DotNetBar::ButtonX^ btnOK;
	DevComponents::DotNetBar::ButtonX^ btnCancel;
	DevComponents::DotNetBar::Controls::TextBoxX^ txtInput;

	void txtInput_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void InitializeComponent(void);
	void InputBox_Load(System::Object^  sender, System::EventArgs^  e);

	void btnOK_Click(Object^ sender, EventArgs^ e);
	void btnCancel_Click(Object^ sender, EventArgs^ e);

	InputBox(void);
public:
	~InputBox();

	property InputBoxResult^ Result;

	static InputBoxResult^ Show(String^ Prompt, String^ Title, String^ Default, IntPtr ParentWindowHandle);
};


ref class AnimatedForm : public DevComponents::DotNetBar::Metro::MetroForm
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

	virtual property Windows::Forms::CreateParams^ CreateParams
	{
		Windows::Forms::CreateParams^ get() override;
	}

	virtual void WndProc(Message% m) override;

	static enum class eTransition
	{
		None = 0,
		FadeIn,
		FadeOut
	};

	static enum class eTransitionFinalState
	{
		None = 0,
		Show,
		Hide,
		Close
	};

	ref struct StartTransitionParams
	{
		eTransitionFinalState EndState;
		IntPtr ParentWindowHandle;
		Point Position;
		bool UsePosition;
		bool Animate;

		StartTransitionParams();
	};
;
	eTransition ActiveTransition;
	eTransitionFinalState ActiveTransitionEndState;
	TransitionCompleteHandler^ ActiveTransitionCompleteHandler;
	bool ShowFormWithoutActivation;

	Timer^ FadeTimer;
	EventHandler^ FadeTimerTickHandler;
	bool ClosingForm;

	void FadeTimer_Tick(Object^ Sender, EventArgs^ E);

	void ShowFormDiscreetly(IntPtr ParentWindowHandle);
	void StartTransition(StartTransitionParams^ Params);
	void EndTransition(StartTransitionParams^ StartParams);
public:
	AnimatedForm(bool ShowFormWithoutActivation);
	virtual ~AnimatedForm();

	property bool AllowMove;
	property bool PreventActivation;
	property bool IsFadingIn
	{
		bool get() { return ActiveTransition == eTransition::FadeIn; }
	}
	property bool IsFadingOut
	{
		bool get() { return ActiveTransition == eTransition::FadeOut; }
	}

	void Show();
	void Show(IntPtr ParentHandle);
	void Show(Drawing::Point Position, IntPtr ParentHandle, bool Animate);
	void Hide();
	void Hide(bool Animate);
	void Close();
	void ForceClose();

	void SetSize(Drawing::Size WindowSize);
	void SetNextActiveTransitionCompleteHandler(TransitionCompleteHandler^ NewHandler);
};


generic <typename TValue>
ref class CaselessFuzzyTrie : public Gma::DataStructures::StringSearch::PatriciaTrie<TValue>
{
public:
	ref struct FuzzyMatchResult
	{
		TValue Value;
		int Cost;

		FuzzyMatchResult(TValue Value, int Cost);
	};
private:
	static FuzzyMatchResult^ MapToFuzzyMatchResultSelector(TValue Value);

	ref struct LevenshteinMatcher
	{
		String^ Query;
		int MaxEditDistance;
		List<List<int>^>^ MatchTable;
		String^ CurrentPrefix;
		System::Func<TValue, bool>^ Predicate;
		List<FuzzyMatchResult^>^ MatchedValues;

		Gma::DataStructures::StringSearch::TraversalResult ForEachTrieNode(Gma::DataStructures::StringSearch::ITrieTraversalNode<TValue>^ Node);
	public:
		LevenshteinMatcher(String^ Query, int MaxEditDistance, System::Func<TValue, bool>^ Predicate);

		property IEnumerable<FuzzyMatchResult^>^ Matches
		{
			IEnumerable<FuzzyMatchResult^>^ get() { return MatchedValues; }
		}

		IEnumerable<FuzzyMatchResult^>^ Match(Gma::DataStructures::StringSearch::ITrie<TValue>^ Trie);
	};
public:
	virtual void Add(String^ Key, TValue Value) override;
	virtual IEnumerable<TValue>^ Retrieve(String^ Query) override; // pass an empty string to retrieve all items
	virtual IEnumerable<TValue>^ Retrieve(String^ Query, System::Func<TValue, bool>^ Predicate) override;
	virtual IEnumerable<FuzzyMatchResult^>^ LevenshteinMatch(String^ Query, UInt32 MaxEditDistanceCost); // pass an empty string to retrieve all items
	virtual IEnumerable<FuzzyMatchResult^>^ LevenshteinMatch(String^ Query, UInt32 MaxEditDistanceCost, System::Func<TValue, bool>^ Predicate);
};


} // namespace utilities


} // namespace scriptEditor


} // namespace cse