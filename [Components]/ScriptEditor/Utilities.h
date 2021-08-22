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
	ref struct FuzzyMatchResult : public IEquatable<FuzzyMatchResult^>
	{
		TValue Value;
		int Cost;

		FuzzyMatchResult(TValue Value, int Cost);

		virtual bool Equals(FuzzyMatchResult^ obj);
		virtual int GetHashCode() override;
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


// For use with DotNetBar::SuperToolTip
// Text can include text-markup
interface class IRichTooltipContentProvider
{
	property String^ TooltipHeaderText
	{
		String^ get();
	}
	property String^ TooltipBodyText
	{
		String^ get();
	}
	property Image^ TooltipBodyImage
	{
		Image^ get();
	}
	property String^ TooltipFooterText
	{
		String^ get();
	}
	property Image^ TooltipFooterImage
	{
		Image^ get();
	}
	property Color TooltipBgColor
	{
		Color get();
	}
	property Color TooltipTextColor
	{
		Color get();
	}
};


// wraps a call to SuperToolTip::Show to override the text and background colors
ref class SuperTooltipColorSwapper
{
	ref class ScopedSwap
	{
		DevComponents::DotNetBar::Rendering::Office2007Renderer^ Renderer;
		Color OldTextColor;
		Color OldBackColorStart;
		Color OldBackColorEnd;
	public:
		ScopedSwap(SuperTooltipColorSwapper^ Parent);
		~ScopedSwap();
	};
public:
	property Color TextColor;
	property Color BackColor;

	SuperTooltipColorSwapper();
	SuperTooltipColorSwapper(Color Text, Color Background);

	void ShowTooltip(DevComponents::DotNetBar::SuperTooltip^ Tooltip, Object^ Sender, Point ScreenPosition);
	void UpdateWithSuperTooltipInfo(DevComponents::DotNetBar::SuperTooltip^ Tooltip, DevComponents::DotNetBar::SuperTooltipInfo^ Info, bool UpdateBounds);
};


Color ShadeColor(Color Input, float NormalizedFactor);
Color TintColor(Color Input, float NormalizedFactor);


// DotNetBar Text-markup - https://www.devcomponents.com/kb2/?p=515
ref class TextMarkupBuilder
{
	static String^ ItalicWhitespaceReplacement = "&nbsp;&nbsp;&nbsp;&nbsp;";

	ref struct FontParams
	{
		property String^ Name;
		property String^ Size;
		property String^ Color;

		FontParams();
	};

	ref struct HyperlinkParams
	{
		property String^ Name;
		property String^ Href;

		HyperlinkParams(String^ Name, String^ Href);
	};

	ref struct TagContext
	{
		static enum class eTag
		{
			None,
			Bold,
			Italic,
			Underline,
			Font,
			Header,
			Hyperlink,
			Paragraph,
			Div,
			Span,
			BreakLine
		};

		property eTag Tag;
		property int HeaderLevel;
		property DevComponents::DotNetBar::eHorizontalItemsAlignment Halign;
		property int Width;
		property Windows::Forms::Padding Padding;
		property FontParams^ ParamsFont;
		property HyperlinkParams^ ParamsHyperlink;

		TagContext(eTag Tag);
	};

	ref struct TableContext
	{
		property int ColumnCount;
		property int RowWidth;

		property int CurrentColumn;
		property int CurrentRow;
		property int CellWidth;

		TableContext(int Columns, int Width);
	};

	System::Text::StringBuilder^ Buffer;
	Stack<TableContext^>^ ActiveTables;
	Stack<TagContext^>^ ActiveTags;

	static void GenerateAlignWidthPaddingAttributes(TagContext^ Context, System::Text::StringBuilder^ Sb);

	void PushTagContext(TagContext^ Tag);
	void PopTagContext();
	void EmitClosingTag(TagContext^ Tag);

	void PrepareAndPushTag(TagContext::eTag Tag, int Width);
	void PrepareAndPushTag(TagContext::eTag Tag, DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	void PrepareAndPushTag(TagContext::eTag Tag, Windows::Forms::Padding Padding);
	void PrepareAndPushTag(TagContext::eTag Tag, int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	void PrepareAndPushTag(TagContext::eTag Tag, int Width, Windows::Forms::Padding Padding);
	void PrepareAndPushTag(TagContext::eTag Tag, int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align, Windows::Forms::Padding Padding);

	bool IsTagActive(TagContext::eTag Tag);
public:
	TextMarkupBuilder();

	property bool HasContent
	{
		bool get() { return Buffer->Length > 0; }
	}

	TextMarkupBuilder^ Bold();
	TextMarkupBuilder^ Italic();
	TextMarkupBuilder^ Underline();
	TextMarkupBuilder^ Header(int Level);
	TextMarkupBuilder^ Hyperlink(String^ Href);
	TextMarkupBuilder^ Hyperlink(String^ Name, String^ Href);

	TextMarkupBuilder^ Font(String^ Name);
	TextMarkupBuilder^ Font(int Size, bool Relative);
	TextMarkupBuilder^ Font(String^ Name, int Size, bool Relative);
	TextMarkupBuilder^ Font(String^ Name, int Size, bool Relative, Drawing::Color Color);

	TextMarkupBuilder^ Paragraph(int Width);
	TextMarkupBuilder^ Paragraph(DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	TextMarkupBuilder^ Paragraph(Windows::Forms::Padding Padding);
	TextMarkupBuilder^ Paragraph(int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	TextMarkupBuilder^ Paragraph(int Width, Windows::Forms::Padding Padding);
	TextMarkupBuilder^ Paragraph(int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align, Windows::Forms::Padding Padding);

	TextMarkupBuilder^ Span(int Width);
	TextMarkupBuilder^ Span(DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	TextMarkupBuilder^ Span(Windows::Forms::Padding Padding);
	TextMarkupBuilder^ Span(int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	TextMarkupBuilder^ Span(int Width, Windows::Forms::Padding Padding);
	TextMarkupBuilder^ Span(int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align, Windows::Forms::Padding Padding);

	TextMarkupBuilder^ Div(int Width);
	TextMarkupBuilder^ Div(DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	TextMarkupBuilder^ Div(Windows::Forms::Padding Padding);
	TextMarkupBuilder^ Div(int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	TextMarkupBuilder^ Div(int Width, Windows::Forms::Padding Padding);
	TextMarkupBuilder^ Div(int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align, Windows::Forms::Padding Padding);

	TextMarkupBuilder^ Table(int Columns, int Width);
	TextMarkupBuilder^ TableNextRow();
	TextMarkupBuilder^ TableNextRow(Windows::Forms::Padding Padding);
	TextMarkupBuilder^ TableEmptyRow();
	TextMarkupBuilder^ TableNextColumn();
	TextMarkupBuilder^ TableNextColumn(int Width);
	TextMarkupBuilder^ TableNextColumn(Windows::Forms::Padding Padding);
	TextMarkupBuilder^ TableNextColumn(int Width, Windows::Forms::Padding Padding);
	TextMarkupBuilder^ TableNextColumn(DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	TextMarkupBuilder^ TableNextColumn(int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align);
	TextMarkupBuilder^ TableNextColumn(DevComponents::DotNetBar::eHorizontalItemsAlignment Align, Windows::Forms::Padding Padding);
	TextMarkupBuilder^ TableNextColumn(int Width, DevComponents::DotNetBar::eHorizontalItemsAlignment Align, Windows::Forms::Padding Padding);

	TextMarkupBuilder^ PopTag();
	TextMarkupBuilder^ PopTag(int Count);
	TextMarkupBuilder^ PopTable();

	TextMarkupBuilder^ LineBreak();
	TextMarkupBuilder^ LineBreak(int Count);
	TextMarkupBuilder^ NonBreakingSpace();
	TextMarkupBuilder^ NonBreakingSpace(int Count);

	TextMarkupBuilder^ Text(String^ Text);
	TextMarkupBuilder^ Markup(String^ MarkupText);
	String^ ToMarkup();
	TextMarkupBuilder^ Reset();
};


} // namespace utilities


} // namespace scriptEditor


} // namespace cse