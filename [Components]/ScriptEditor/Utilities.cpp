#include "Utilities.h"

namespace cse
{


namespace scriptEditor
{


namespace utilities
{


IAction::IAction(String^ Name, String^ Description)
{
	Name_ = Name;
	Description_ = Description;
}

BasicAction::BasicAction(String^ Name, String^ Description)
	: IAction(Name, Description)
{
}

BasicAction::~BasicAction()
{
	delete InvokeDelegate;
}

void BasicAction::Invoke()
{
	InvokeDelegate();
}

void KeyCombo::Validate()
{
	if (Main.HasFlag(Keys::Control) || Main.HasFlag(Keys::Shift) || Main.HasFlag(Keys::Alt))
		throw gcnew InvalidEnumArgumentException();
}

KeyCombo::KeyCombo(Keys Main, Keys Modifiers) : Main(Main), Modifiers(Modifiers)
{
	Validate();
}

bool KeyCombo::IsTriggered(KeyEventArgs^ E)
{
	if (Control && !E->Control)
		return false;
	else if (Shift && !E->Shift)
		return false;
	else if (Alt && !E->Alt)
		return false;

	return Main == E->KeyCode;
}

bool KeyCombo::Equals(Object^ obj)
{
	if (obj == nullptr)
		return false;
	else if (obj->GetType() != KeyCombo::typeid)
		return false;

	auto Other = safe_cast<KeyCombo^>(obj);
	return this->Main.Equals(Other->Main) && this->Modifiers.Equals(Other->Modifiers);
}

int KeyCombo::GetHashCode()
{
	int Hash = 7;
	Hash = 31 * Hash + static_cast<int>(Main);
	Hash = 31 * Hash + static_cast<int>(Modifiers);
	return Hash;
}


System::String^ KeyCombo::ToString()
{
	String^ ModifierString = "";
	if (Control)
		ModifierString += "Ctrl";
	if (Shift)
		ModifierString += (Control ? " + " : "") + "Shift";
	if (Alt)
		ModifierString += (Control || Shift ? " + " : "") + "Alt";

	return ModifierString->Trim() + " + " + Main.ToString();
}

KeyCombo^ KeyCombo::FromKeyEvent(KeyEventArgs^ E)
{
	UInt32 Modifiers;
	if (E->Control)
		Modifiers |= safe_cast<UInt32>(Keys::Control);
	if (E->Shift)
		Modifiers |= safe_cast<UInt32>(Keys::Shift);
	if (E->Alt)
		Modifiers |= safe_cast<UInt32>(Keys::Alt);

	return gcnew KeyCombo(E->KeyCode, safe_cast<Keys>(Modifiers));
}

KeyCombo^ KeyCombo::New(Keys Modifier, Keys Key)
{
	return gcnew KeyCombo(Key, Modifier);
}

InputBox::InputBox(void)
{
	InitializeComponent();
	Result = gcnew InputBoxResult;
	Result->ReturnCode = Windows::Forms::DialogResult::Ignore;
	Result->Text = String::Empty;
}

InputBox::~InputBox()
{
}

void InputBox::InitializeComponent()
{
	this->lblPrompt = (gcnew DevComponents::DotNetBar::LabelX());
	this->btnOK = (gcnew DevComponents::DotNetBar::ButtonX());
	this->btnCancel = (gcnew DevComponents::DotNetBar::ButtonX());
	this->txtInput = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
	this->SuspendLayout();
	//
	// lblPrompt
	//
	//
	//
	//
	this->lblPrompt->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->lblPrompt->Location = System::Drawing::Point(12, 12);
	this->lblPrompt->Name = L"lblPrompt";
	this->lblPrompt->Size = System::Drawing::Size(313, 52);
	this->lblPrompt->TabIndex = 0;
	this->lblPrompt->Text = L"Prompt Text";
	this->lblPrompt->TextLineAlignment = System::Drawing::StringAlignment::Near;
	//
	// btnOK
	//
	this->btnOK->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
	this->btnOK->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
	this->btnOK->Location = System::Drawing::Point(345, 12);
	this->btnOK->Name = L"btnOK";
	this->btnOK->Size = System::Drawing::Size(75, 23);
	this->btnOK->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->btnOK->TabIndex = 2;
	this->btnOK->Text = L"OK";
	this->btnOK->Click += gcnew System::EventHandler(this, &InputBox::btnOK_Click);
	//
	// btnCancel
	//
	this->btnCancel->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
	this->btnCancel->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
	this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
	this->btnCancel->Location = System::Drawing::Point(345, 41);
	this->btnCancel->Name = L"btnCancel";
	this->btnCancel->Size = System::Drawing::Size(75, 23);
	this->btnCancel->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->btnCancel->TabIndex = 3;
	this->btnCancel->Text = L"Cancel";
	this->btnCancel->Click += gcnew System::EventHandler(this, &InputBox::btnCancel_Click);
	//
	// txtInput
	//
	this->txtInput->BackColor = System::Drawing::Color::Black;
	//
	//
	//
	this->txtInput->Border->Class = L"TextBoxBorder";
	this->txtInput->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->txtInput->DisabledBackColor = System::Drawing::Color::Black;
	this->txtInput->ForeColor = System::Drawing::Color::White;
	this->txtInput->Location = System::Drawing::Point(12, 74);
	this->txtInput->Name = L"txtInput";
	this->txtInput->PreventEnterBeep = true;
	this->txtInput->Size = System::Drawing::Size(313, 22);
	this->txtInput->TabIndex = 1;
	this->txtInput->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &InputBox::txtInput_KeyDown);
	//
	// InputPromptDialog
	//
	this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
	this->CancelButton = this->btnCancel;
	this->ClientSize = System::Drawing::Size(432, 108);
	this->ControlBox = false;
	this->Controls->Add(this->txtInput);
	this->Controls->Add(this->btnCancel);
	this->Controls->Add(this->btnOK);
	this->Controls->Add(this->lblPrompt);
	this->DoubleBuffered = true;
	this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
	this->Name = L"InputPromptDialog";
	this->ShowIcon = false;
	this->ShowInTaskbar = false;
	this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
	this->Text = L"Prompt Dialog";
	this->Load += gcnew System::EventHandler(this, &InputBox::InputBox_Load);
	this->ResumeLayout(false);
}

void InputBox::InputBox_Load(System::Object^ sender, System::EventArgs^ e)
{
	txtInput->SelectionStart = 0;
	txtInput->SelectionLength = txtInput->Text->Length;
	txtInput->Focus();
}

void InputBox::btnOK_Click(Object^ sender, EventArgs^ e)
{
	Result->ReturnCode = Windows::Forms::DialogResult::OK;
	Result->Text = txtInput->Text;
	Close();
}

void InputBox::btnCancel_Click(Object^ sender, EventArgs^ e)
{
	Result->ReturnCode = Windows::Forms::DialogResult::Cancel;
	Result->Text = String::Empty;
	Close();
}

void InputBox::txtInput_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	switch (E->KeyCode)
	{
	case Keys::Enter:
		btnOK->PerformClick();
		break;
	case Keys::Escape:
		btnCancel->PerformClick();
		break;
	}
}

InputBoxResult^ InputBox::Show(String^ Prompt, String^ Title, String^ Default, IntPtr ParentWindowHandle)
{
	auto Form = gcnew InputBox();

	Form->txtInput->Text = Default;
	Form->lblPrompt->Text = Prompt;
	Form->Text = Title;

	Form->ShowDialog(gcnew WindowHandleWrapper(ParentWindowHandle));
	return Form->Result;
}

Windows::Forms::CreateParams^ AnimatedForm::CreateParams::get()
{
	auto Params = MetroForm::CreateParams;
	if (ShowFormWithoutActivation)
	{
		const int WS_EX_NOACTIVATE = 0x08000000;
		const int WS_EX_TOOLWINDOW = 0x00000080;
		Params->ExStyle |= WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
	}

	return Params;
}

void AnimatedForm::WndProc(Message% m)
{
	const int WM_SYSCOMMAND = 0x0112;
	const int SC_MOVE = 0xF010;
	const int WM_MOVE = 0x003;
	const int WM_MOVING = 0x0216;
	const int WM_ACTIVATE = 0x6;
	const int WM_NCACTIVATE = 0x86;
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
			if (!PreventActivation && ActiveTransition == eTransition::None)
				break;

			if (m.LParam != IntPtr::Zero)
				nativeWrapper::SetActiveWindow(m.LParam);

			m.Result = IntPtr::Zero;
			return;
		}

		break;
	}

	MetroForm::WndProc(m);
}

AnimatedForm::StartTransitionParams::StartTransitionParams()
{
	EndState = eTransitionFinalState::None;
	ParentWindowHandle = IntPtr::Zero;
	Position = Point(0, 0);
	UsePosition = false;
	Animate = true;
}

void AnimatedForm::FadeTimer_Tick(Object^ Sender, EventArgs^ E)
{
	const auto kTransitionTime = 32;		// in ms

	auto NumTicksReqd = kTransitionTime / static_cast<double>(FadeTimer->Interval);
	auto PerTickDelta = 1.0 / NumTicksReqd;

	if (ActiveTransition == eTransition::FadeIn)
		this->Opacity += PerTickDelta;
	else if (ActiveTransition == eTransition::FadeOut)
		this->Opacity -= PerTickDelta;

	if (this->Opacity >= 1.0 || this->Opacity <= 0.0)
		EndTransition(nullptr);
}

void AnimatedForm::ShowFormDiscreetly(IntPtr ParentWindowHandle)
{
	this->Opacity = 0;
	if (ParentWindowHandle != IntPtr::Zero)
		MetroForm::Show(gcnew WindowHandleWrapper(ParentWindowHandle));
	else
		MetroForm::Show();
}

void AnimatedForm::StartTransition(StartTransitionParams^ Params)
{
	Debug::Assert(ClosingForm == false);
	Debug::Assert(ActiveTransition == eTransition::None);
	Debug::Assert(ActiveTransitionEndState == eTransitionFinalState::None);
	Debug::Assert(FadeTimer->Enabled == false);

	switch (Params->EndState)
	{
	case eTransitionFinalState::Show:
		if (Params->UsePosition)
			SetDesktopLocation(Params->Position.X, Params->Position.Y);

		if (!Visible)
			ShowFormDiscreetly(Params->ParentWindowHandle);

		if (Params->Animate)
		{
			ActiveTransition = eTransition::FadeIn;
			this->Opacity = 0;
		}

		break;
	case eTransitionFinalState::Hide:
	case eTransitionFinalState::Close:
		if (!Visible)
			MetroForm::Show();

		if (Params->Animate)
		{
			ActiveTransition = eTransition::FadeOut;
			this->Opacity = 1;
		}

		break;
	}

	ActiveTransitionEndState = Params->EndState;
	if (!Params->Animate)
	{
		// end the transition right away
		EndTransition(Params);
	}
	else
		FadeTimer->Start();
}

void AnimatedForm::EndTransition(StartTransitionParams^ StartParams)
{
	if (ActiveTransitionEndState == eTransitionFinalState::None)
		return;
	else if (ClosingForm)
		return;

	FadeTimer->Stop();

	if (ActiveTransitionCompleteHandler)
		ActiveTransitionCompleteHandler(this);

	switch (ActiveTransitionEndState)
	{
	case eTransitionFinalState::Hide:
		MetroForm::Hide();
		this->Opacity = 1;
		break;
	case eTransitionFinalState::Show:
		if (StartParams == nullptr)
			MetroForm::BringToFront();
		this->Opacity = 1;
		break;
	case eTransitionFinalState::Close:
		MetroForm::Close();
		ClosingForm = true;
		break;
	}

	ActiveTransition = eTransition::None;
	ActiveTransitionEndState = eTransitionFinalState::None;
	ActiveTransitionCompleteHandler = nullptr;
}

AnimatedForm::AnimatedForm( bool ShowFormWithoutActivation )
{
	ActiveTransition = eTransition::None;
	ActiveTransitionEndState = eTransitionFinalState::None;
	ActiveTransitionCompleteHandler = nullptr;

	this->ShowFormWithoutActivation = ShowFormWithoutActivation;

	FadeTimerTickHandler = gcnew EventHandler(this, &AnimatedForm::FadeTimer_Tick);
	FadeTimer = gcnew Timer();
	FadeTimer->Interval = 8;
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

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = eTransitionFinalState::Show;
	StartTransition(Params);
}

void AnimatedForm::Show(IntPtr ParentHandle)
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	if (Visible)
		return;

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = eTransitionFinalState::Show;
	Params->ParentWindowHandle = ParentHandle;
	StartTransition(Params);
}

void AnimatedForm::Show(Drawing::Point Position, IntPtr ParentHandle, bool Animate)
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = eTransitionFinalState::Show;
	Params->ParentWindowHandle = ParentHandle;
	Params->Position = Position;
	Params->UsePosition = true;
	Params->Animate = Animate;
	StartTransition(Params);
}

void AnimatedForm::Hide()
{
	Hide(true);
}

void AnimatedForm::Hide(bool Animate)
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	if (!Visible)
		return;

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = eTransitionFinalState::Hide;
	Params->Animate = Animate;
	StartTransition(Params);
}

void AnimatedForm::Close()
{
	if (ClosingForm)
		throw gcnew System::InvalidOperationException("Form is being disposed or has already been disposed");

	EndTransition(nullptr);

	auto Params = gcnew StartTransitionParams;
	Params->EndState = eTransitionFinalState::Close;
	StartTransition(Params);
}

void AnimatedForm::ForceClose()
{
	MetroForm::Close();
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
	EndTransition(nullptr);

	ActiveTransitionCompleteHandler = NewHandler;
}

generic <typename TValue>
CaselessFuzzyTrie<TValue>::FuzzyMatchResult^ CaselessFuzzyTrie<TValue>::MapToFuzzyMatchResultSelector(TValue Value)
{
	return gcnew FuzzyMatchResult(Value, 0);
}

generic <typename TValue>
Gma::DataStructures::StringSearch::TraversalResult CaselessFuzzyTrie<TValue>::LevenshteinMatcher::ForEachTrieNode(Gma::DataStructures::StringSearch::ITrieTraversalNode<TValue>^ Node)
{
	// http://stevehanov.ca/blog/?id=114
	auto NewPrefix = Node->Prefix + Node->Key;

	int PartitionIndex = -1;

	for (int i = 0; i < CurrentPrefix->Length; ++i)
	{
		if (i >= NewPrefix->Length)
		{
			PartitionIndex = i;
			break;
		}
		else if (NewPrefix[i] != CurrentPrefix[i])
		{
			PartitionIndex = i;
			break;
		}
	}

	List<List<int>^>^ ReusableRows = nullptr;
	if (PartitionIndex == -1)
		ReusableRows = MatchTable;
	else if (PartitionIndex <= MatchTable->Count)
		ReusableRows = MatchTable->GetRange(0, PartitionIndex + 1);		// +1 as the first row does not correspond to a character in the prefix
	else
	{
		ReusableRows = gcnew List<List<int>^>;
		auto NewRow = gcnew List<int>(Query->Length + 1);
		for (int k = 0; k < Query->Length + 1; ++k)
			NewRow->Insert(k, k);
		ReusableRows->Add(NewRow);
	}

	Debug::Assert(ReusableRows->Count > 0);

	// calculate costs for each new character in the new prefix
	bool MaxCostExceeded = false;
	for (int i = ReusableRows->Count, j = NewPrefix->Length + 1; i < j; ++i)
	{
		auto PreviousRow = ReusableRows[i - 1];
		auto NewRow = gcnew List<int>(PreviousRow->Count);

		NewRow->Insert(0, PreviousRow[0] + 1);

		int InsertCost, DeleteCost, ReplaceCost;
		for (int k = 1; k < PreviousRow->Count; ++k)
		{
			InsertCost = NewRow[k - 1] + 1;
			DeleteCost = PreviousRow[k] + 1;

			ReplaceCost = PreviousRow[k - 1];
			if (Query[k - 1] != NewPrefix[i - 1])
				++ReplaceCost;

			NewRow->Insert(k, Math::Min(InsertCost, Math::Min(DeleteCost, ReplaceCost)));
		}

		auto LowestCost = System::Linq::Enumerable::Min(NewRow);
		if (LowestCost > MaxEditDistance)
		{
			// exit early
			MaxCostExceeded = true;
			NewPrefix = NewPrefix->Substring(0, ReusableRows->Count - 1);
			break;
		}

		ReusableRows->Add(NewRow);
	}

	auto Cost = ReusableRows[ReusableRows->Count - 1][Query->Length];
	if (Cost <= MaxEditDistance && Node->ValueCount > 0)
	{
		//Debug::Assert(!MaxCostExceeded);

		for each (auto Value in Node->Values)
		{
			if (Predicate == nullptr || Predicate(Value))
				MatchedValues->Add(gcnew FuzzyMatchResult(Value, Cost));
		}
	}

	MatchTable = ReusableRows;
	CurrentPrefix = NewPrefix;

	return MaxCostExceeded ? Gma::DataStructures::StringSearch::TraversalResult::IgnoreChildren : Gma::DataStructures::StringSearch::TraversalResult::TraverseChildren;
}

generic <typename TValue>
CaselessFuzzyTrie<TValue>::LevenshteinMatcher::LevenshteinMatcher(String^ Query, int MaxEditDistance, System::Func<TValue, bool>^ Predicate)
{
	this->Query = Query;
	this->MaxEditDistance = MaxEditDistance;
	this->Predicate = Predicate;

	CurrentPrefix = "";
	MatchedValues = gcnew List<FuzzyMatchResult^>;

	MatchTable = gcnew List<List<int>^>;
	auto NewRow = gcnew List<int>(Query->Length + 1);
	for (int k = 0; k < Query->Length + 1; ++k)
		NewRow->Insert(k, k);
	MatchTable->Add(NewRow);
}

generic <typename TValue>
System::Collections::Generic::IEnumerable<CaselessFuzzyTrie<TValue>::FuzzyMatchResult^>^ CaselessFuzzyTrie<TValue>::LevenshteinMatcher::Match(Gma::DataStructures::StringSearch::ITrie<TValue>^ Trie)
{
	Trie->Traverse(gcnew Gma::DataStructures::StringSearch::TraversalHandler<TValue>(this, &LevenshteinMatcher::ForEachTrieNode));
	return MatchedValues;
}

generic <typename TValue>
CaselessFuzzyTrie<TValue>::FuzzyMatchResult::FuzzyMatchResult(TValue Value, int Cost)
	: Value(Value), Cost(Cost)
{
}

generic <typename TValue>
void CaselessFuzzyTrie<TValue>::Add(String^ Key, TValue Value)
{
	PatriciaTrie::Add(Key->ToLower(), Value);
}

generic <typename TValue>
System::Collections::Generic::IEnumerable<TValue>^ CaselessFuzzyTrie<TValue>::Retrieve(String^ Query)
{
	return Retrieve(Query, nullptr);
}

generic <typename TValue>
System::Collections::Generic::IEnumerable<TValue>^ CaselessFuzzyTrie<TValue>::Retrieve(String^ Query, System::Func<TValue, bool>^ Predicate)
{
	return PatriciaTrie::Retrieve(Query->ToLower(), Predicate);
}

generic <typename TValue>
System::Collections::Generic::IEnumerable<CaselessFuzzyTrie<TValue>::FuzzyMatchResult^>^ CaselessFuzzyTrie<TValue>::LevenshteinMatch(String^ Query, UInt32 MaxEditDistanceCost)
{
	return LevenshteinMatch(Query, MaxEditDistanceCost, nullptr);
}

generic <typename TValue>
System::Collections::Generic::IEnumerable<CaselessFuzzyTrie<TValue>::FuzzyMatchResult^>^ CaselessFuzzyTrie<TValue>::LevenshteinMatch(String^ Query, UInt32 MaxEditDistanceCost, System::Func<TValue, bool>^ Predicate)
{
	if (Query->Length == 0)
		return System::Linq::Enumerable::Select(Retrieve(String::Empty), gcnew Func<TValue, CaselessFuzzyTrie<TValue>::FuzzyMatchResult^>(MapToFuzzyMatchResultSelector));

	auto Matcher = gcnew LevenshteinMatcher(Query->ToLower(), MaxEditDistanceCost, Predicate);
	return Matcher->Match(this);
}


} // namespace utilities


} // namespace scriptEditor


} // namespace cse