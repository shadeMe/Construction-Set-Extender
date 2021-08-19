#include "AvalonEditSyntaxHighlighting.h"
#include "Preferences.h"
#include "ScriptPreprocessor.h"

namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


namespace avalonEdit
{


using namespace preprocessor;


XshdPropertyName::XshdPropertyName()
{
	Name = "";
}

XshdPropertyName::XshdPropertyName(String^ Name)
{
	this->Name = Name;
}

System::String^ XshdPropertyName::Serialize()
{
	return Name != "" ? ("name=\"" + Name + "\"") : "";
}

XshdPropertyColor::XshdPropertyColor()
{
	Foreground = Color::Empty;
	Background = Color::Empty;
}

XshdPropertyColor::XshdPropertyColor(Drawing::Color ForeColor, Drawing::Color BackColor)
{
	Foreground = ForeColor;
	Background = BackColor;
}

System::String^ XshdPropertyColor::Serialize()
{
	String^ Out = "";

	if (Foreground != Color::Empty)
	{
		Out += "foreground=\"#" + Foreground.R.ToString("X2") +
			   Foreground.G.ToString("X2") +
			   Foreground.B.ToString("X2") + "\" ";
	}

	if (Background != Color::Empty)
	{
		Out += "background=\"#" + Background.R.ToString("X2") +
			   Background.G.ToString("X2") +
			   Background.B.ToString("X2") + "\" ";
	}

	return Out->Trim();
}

XshdPropertyFont::XshdPropertyFont()
{
	Weight = Windows::FontWeights::Normal;
	Style = Windows::FontStyles::Normal;
}

XshdPropertyFont::XshdPropertyFont(bool Bold, bool Italic)
	: XshdPropertyFont()
{
	if (Bold)
		Weight = Windows::FontWeights::Bold;

	if (Italic)
		Style = Windows::FontStyles::Italic;
}

System::String^ XshdPropertyFont::Serialize()
{
	String^ Out = "";

	if (Weight != Windows::FontWeights::Normal)
		Out += "fontWeight=\"" + Weight.ToString()->ToLower() + "\" ";

	if (Style != Windows::FontStyles::Normal)
		Out += "fontStyle=\"" + Style.ToString()->ToLower() + "\" ";

	return Out->Trim();
}

XshdColor::XshdColor(String^ Name, Drawing::Color ForeColor, Drawing::Color BackColor, bool Bold, bool Italic)
{
	this->Name = gcnew XshdPropertyName(Name);
	this->Color = gcnew XshdPropertyColor(ForeColor, BackColor);
	this->Font = gcnew XshdPropertyFont;

	if (Bold)
		Font->Weight = Windows::FontWeights::Bold;
	if (Italic)
		Font->Style = Windows::FontStyles::Italic;
}

XshdColor::XshdColor(String^ Name)
	: XshdColor(Name, Drawing::Color::Empty, Drawing::Color::Empty, false, false)
{
}

void XshdColor::Update(Drawing::Color ForeColor, Drawing::Color BackColor, bool Bold, bool Italic)
{
	this->Color = gcnew XshdPropertyColor(ForeColor, BackColor);
	this->Font = gcnew XshdPropertyFont(Bold, Italic);
}

System::String^ XshdColor::Serialize()
{
	return "<Color "
		   + Name->Serialize() + " "
		   + Color->Serialize() + " "
		   + Font->Serialize()
		   + "/>\n";
}

System::String^ XshdColor::SerializeAsProperty()
{
	return "color=\"" + Name->Name + "\" ";
}

XshdKeywords::XshdKeywords(XshdColor^ NamedColor)
{
	Words = gcnew HashSet<String^>(StringComparer::CurrentCultureIgnoreCase);
	Color = NamedColor;
}

void XshdKeywords::AddKeyword(String^ Keyword)
{
	Words->Add(Keyword);
}

void XshdKeywords::ClearKeywords()
{
	Words->Clear();
}

System::String^ XshdKeywords::Serialize()
{
	if (Words->Count == 0)
		return "";

	auto Sb = gcnew System::Text::StringBuilder;

	Sb->Append("<Keywords ")->Append(Color->SerializeAsProperty() + ">\n");
	for each (auto Itr in Words)
		Sb->Append("\t<Word>")->Append(Itr)->Append("</Word>\n");
	Sb->Append("</Keywords>\n");

	return Sb->ToString();
}

XshdRuleSet::XshdRuleSet()
	: XshdRuleSet("")
{
}

XshdRuleSet::XshdRuleSet(String^ Name)
{
	this->Name = gcnew XshdPropertyName(Name);

	IgnoreCase = true;
	Children = gcnew List<IXshdElement^>;
}

void XshdRuleSet::AddChild(IXshdElement^ Child)
{
	Children->Add(Child);
}

XshdRuleSet^ XshdRuleSet::ShallowCopy()
{
	auto Copy = gcnew XshdRuleSet;
	Copy->Name = this->Name;
	Copy->IgnoreCase = this->IgnoreCase;

	Copy->Children->AddRange(this->Children);
	return Copy;
}

System::String^ XshdRuleSet::Serialize()
{
	auto Sb = gcnew System::Text::StringBuilder;

	Sb->Append("<RuleSet ")->Append(Name->Serialize());
	Sb->Append(" ignoreCase=\"" + (IgnoreCase ? "true" : "false") + "\">\n");
	for each (auto Itr in Children)
		Sb->Append("\t")->Append(Itr->Serialize())->Append("\n");
	Sb->Append("</RuleSet>\n");

	return Sb->ToString();
}

System::String^ XshdRuleSet::SerializeAsProperty()
{
	return "ruleSet=\"" + Name->Name + "\" ";
}

XshdRule::XshdRule(XshdColor^ NamedColor)
{
	Color = NamedColor;
	Value = "";
}

System::String^ XshdRule::Serialize()
{
	return "<Rule " + Color->SerializeAsProperty() + ">\n\t" + Value + "\n</Rule>\n";
}

XshdSpan::XshdSpan(XshdColor^ NamedColor, XshdRuleSet^ NamedRuleSet)
{
	Color = NamedColor;
	Begin = "";
	End = "";
	MultiLine = false;
	RuleSet = NamedRuleSet;
}

XshdSpan::XshdSpan(XshdColor^ NamedColor)
	: XshdSpan(NamedColor, nullptr)
{
}

System::String^ XshdSpan::Serialize()
{
	auto Sb = gcnew System::Text::StringBuilder;

	Sb->Append("<Span ")->Append(Color->SerializeAsProperty());
	if (RuleSet)
		Sb->Append(RuleSet->SerializeAsProperty());

	Sb->Append(" multiline=\"" + (MultiLine ? "true" : "false") + "\">\n");

	if (Begin != "")
		Sb->Append("<Begin>")->Append(Begin)->Append("</Begin>\n");
	if (End != "")
		Sb->Append("<End>")->Append(End)->Append("</End>\n");

	Sb->Append("</Span>\n");

	return Sb->ToString();
}


SyntaxHighlightingManager::Colors::Colors()
{
	ColorComment = gcnew XshdColor("Comment");
	ColorPreprocessor = gcnew XshdColor("Preprocessor");
	ColorKeyword = gcnew XshdColor("Keyword");
	ColorScriptBlock = gcnew XshdColor("ScriptBlock");
	ColorDigit = gcnew XshdColor("Digit");
	ColorString = gcnew XshdColor("String");
	ColorLocalVariable = gcnew XshdColor("LocalVariable");
	ColorTaskComment = gcnew XshdColor("TaskComment");
}

void SyntaxHighlightingManager::Colors::UpdateFromPreferences()
{
	bool Bold = preferences::SettingsHolder::Get()->Appearance->BoldFaceHighlightedText;
	bool KeywordBlocksItalic = preferences::SettingsHolder::Get()->Appearance->KeywordsAndBlocksInItalic;
	Color BaseColorComment = preferences::SettingsHolder::Get()->Appearance->ForeColorComments;
	Color BaseColorPreprocessor = preferences::SettingsHolder::Get()->Appearance->ForeColorPreprocessor;
	Color BaseColorKeyword = preferences::SettingsHolder::Get()->Appearance->ForeColorKeywords;
	Color BaseColorScriptBlock = preferences::SettingsHolder::Get()->Appearance->ForeColorScriptBlocks;
	Color BaseColorDigit = preferences::SettingsHolder::Get()->Appearance->ForeColorDigits;
	Color BaseColorString = preferences::SettingsHolder::Get()->Appearance->ForeColorStringLiterals;
	Color BaseColorLocalVariable = preferences::SettingsHolder::Get()->Appearance->ForeColorLocalVariables;
	Color BaseColorTaskComment = preferences::SettingsHolder::Get()->Appearance->BackColorTaskComment;

	ColorComment->Update(BaseColorComment, Color::Empty, Bold, false);
	ColorPreprocessor->Update(BaseColorPreprocessor, Color::Empty, Bold, false);
	ColorKeyword->Update(BaseColorKeyword, Color::Empty, Bold, KeywordBlocksItalic);
	ColorScriptBlock->Update(BaseColorScriptBlock, Color::Empty, Bold, KeywordBlocksItalic);
	ColorDigit->Update(BaseColorDigit, Color::Empty, Bold, false);
	ColorString->Update(BaseColorString, Color::Empty, Bold, false);
	ColorLocalVariable->Update(BaseColorLocalVariable, Color::Empty, Bold, false);
	ColorTaskComment->Update(BaseColorComment, BaseColorTaskComment, Bold, false);
}


System::String^ SyntaxHighlightingManager::Colors::Serialize()
{
	auto Sb = gcnew System::Text::StringBuilder;

	Sb->AppendLine(ColorComment->Serialize());
	Sb->AppendLine(ColorPreprocessor->Serialize());
	Sb->AppendLine(ColorKeyword->Serialize());
	Sb->AppendLine(ColorScriptBlock->Serialize());
	Sb->AppendLine(ColorDigit->Serialize());
	Sb->AppendLine(ColorString->Serialize());
	Sb->AppendLine(ColorLocalVariable->Serialize());
	Sb->AppendLine(ColorTaskComment->Serialize());

	return Sb->ToString();
}

void SyntaxHighlightingManager::Preferences_Changed(Object^ Sender, EventArgs^ E)
{
	HighlightingColors->UpdateFromPreferences();

	for each (auto% Itr in RegisteredEditors)
	{
		auto NewHighlightingDefinition = GenerateHighlightingDefinition(Itr.Key->LastAnalysisResult);
		Itr.Value->SyntaxHighlighting = NewHighlightingDefinition;
	}
}

void SyntaxHighlightingManager::BgAnalysis_Complete(Object^ Sender, BgAnalyserT::AnalysisCompleteEventArgs^ E)
{
	auto BgAnalyser = safe_cast<BgAnalyserT^>(Sender);
	auto TextEditor = RegisteredEditors[BgAnalyser];

	auto HighlightingDefinition = GenerateHighlightingDefinition(E->Result);
	TextEditor->SyntaxHighlighting = HighlightingDefinition;
}

XshdRuleSet^ SyntaxHighlightingManager::GenerateMainRuleSet(Colors^ XshdColors, XshdRuleSet^ CommentSpanRules)
{
	auto XshdRuleDigit = gcnew XshdRule(XshdColors->ColorDigit);
	XshdRuleDigit->Value = "\\b0[xX][0-9a-fA-F]+\r\n|\\b\r\n(\\d+(\\.[0-9]+)?\r\n|\\.[0-9]+\r\n)\r\n([eE][+-]?[0-9]+)?";

	auto XshdRuleString = gcnew XshdSpan(XshdColors->ColorString);
	XshdRuleString->Begin = "\"";
	XshdRuleString->End = "\"";

	auto XshdRuleKeywords = gcnew XshdKeywords(XshdColors->ColorKeyword);
	for each (auto Itr in ScriptIdentifiers->ScriptKeywords)
		XshdRuleKeywords->AddKeyword(Itr);

	auto XshdRuleScriptBlocks = gcnew XshdKeywords(XshdColors->ColorScriptBlock);
	for each (auto Itr in ScriptIdentifiers->ScriptBlocks)
		XshdRuleScriptBlocks->AddKeyword(Itr);


	auto XshdRuleCommentSpan = gcnew XshdSpan(XshdColors->ColorComment, CommentSpanRules);
	XshdRuleCommentSpan->Begin = ";";

	auto XshdRulePreprocessorSpan = gcnew XshdSpan(XshdColors->ColorPreprocessor, CommentSpanRules);
	XshdRulePreprocessorSpan->Begin = ";{";
	XshdRulePreprocessorSpan->End = ";}";
	XshdRulePreprocessorSpan->MultiLine = true;

	auto MainRuleSet = gcnew XshdRuleSet;
	MainRuleSet->IgnoreCase = true;

	MainRuleSet->AddChild(XshdRuleDigit);
	MainRuleSet->AddChild(XshdRuleString);
	MainRuleSet->AddChild(XshdRuleKeywords);
	MainRuleSet->AddChild(XshdRuleScriptBlocks);
	MainRuleSet->AddChild(XshdRulePreprocessorSpan);
	MainRuleSet->AddChild(XshdRuleCommentSpan);

	return MainRuleSet;
}


XshdRuleSet^ SyntaxHighlightingManager::GenerateCommentSpanRuleSet(Colors^ XshdColors)
{
	auto XshdRuleSetNestedCommentRules = gcnew XshdRuleSet("CommentNestedRuleSet");
	//XshdRuleSetNestedCommentSpans->IgnoreCase = false;

	auto XshdRuleTaskKeywords = gcnew XshdKeywords(XshdColors->ColorTaskComment);
	XshdRuleTaskKeywords->AddKeyword("TODO");
	XshdRuleTaskKeywords->AddKeyword("HACK");
	XshdRuleTaskKeywords->AddKeyword("FIX");
	XshdRuleTaskKeywords->AddKeyword("FIXME");

	auto XshdRulePreprocessorKeywords = gcnew XshdKeywords(XshdColors->ColorPreprocessor);
	XshdRulePreprocessorKeywords->AddKeyword(CSEPreprocessorDirective::GetDirectiveKeyword(CSEPreprocessorDirective::eDirectiveType::Define, CSEPreprocessorDirective::eEncodingType::SingleLine));
	XshdRulePreprocessorKeywords->AddKeyword(CSEPreprocessorDirective::GetDirectiveKeyword(CSEPreprocessorDirective::eDirectiveType::Define, CSEPreprocessorDirective::eEncodingType::MultiLine));

	XshdRulePreprocessorKeywords->AddKeyword(CSEPreprocessorDirective::GetDirectiveKeyword(CSEPreprocessorDirective::eDirectiveType::Enum, CSEPreprocessorDirective::eEncodingType::SingleLine));
	XshdRulePreprocessorKeywords->AddKeyword(CSEPreprocessorDirective::GetDirectiveKeyword(CSEPreprocessorDirective::eDirectiveType::Enum, CSEPreprocessorDirective::eEncodingType::MultiLine));

	XshdRulePreprocessorKeywords->AddKeyword(CSEPreprocessorDirective::GetDirectiveKeyword(CSEPreprocessorDirective::eDirectiveType::If, CSEPreprocessorDirective::eEncodingType::SingleLine));
	XshdRulePreprocessorKeywords->AddKeyword(CSEPreprocessorDirective::GetDirectiveKeyword(CSEPreprocessorDirective::eDirectiveType::If, CSEPreprocessorDirective::eEncodingType::MultiLine));

	XshdRulePreprocessorKeywords->AddKeyword(CSEPreprocessorDirective::GetDirectiveKeyword(CSEPreprocessorDirective::eDirectiveType::Import, CSEPreprocessorDirective::eEncodingType::SingleLine));
	XshdRulePreprocessorKeywords->AddKeyword(CSEPreprocessorDirective::GetDirectiveKeyword(CSEPreprocessorDirective::eDirectiveType::Import, CSEPreprocessorDirective::eEncodingType::MultiLine));

	XshdRuleSetNestedCommentRules->AddChild(XshdRuleTaskKeywords);
	XshdRuleSetNestedCommentRules->AddChild(XshdRulePreprocessorKeywords);

	return XshdRuleSetNestedCommentRules;
}

ICSharpCode::AvalonEdit::Highlighting::IHighlightingDefinition^ SyntaxHighlightingManager::GenerateHighlightingDefinition(obScriptParsing::AnalysisData^ SemanticAnalysisData)
{
	auto PrimaryRuleSet = MainRuleSet->ShallowCopy();
	auto RuleLocalVariables = gcnew XshdKeywords(HighlightingColors->ColorLocalVariable);

	if (SemanticAnalysisData)
	{
		for each (auto Itr in SemanticAnalysisData->Variables)
			RuleLocalVariables->AddKeyword(Itr->Name);

		PrimaryRuleSet->AddChild(RuleLocalVariables);
	}

	auto XmlSb = gcnew System::Text::StringBuilder;
	XmlSb->AppendLine("<?xml version=\"1.0\"?>");
	XmlSb->AppendLine("<SyntaxDefinition name=\"ObScript\" xmlns=\"http://icsharpcode.net/sharpdevelop/syntaxdefinition/2008\">");
	XmlSb->AppendLine(HighlightingColors->Serialize());
	XmlSb->AppendLine(CommentSpanRuleSet->Serialize());
	XmlSb->AppendLine(PrimaryRuleSet->Serialize());
	XmlSb->AppendLine("</SyntaxDefinition>");

	auto XmlString = XmlSb->ToString();
	//Debug::WriteLine(XmlString + "\n\n\n");

	auto ByteArray = System::Text::Encoding::UTF8->GetBytes(XmlString);
	auto XmlReader = gcnew System::Xml::XmlTextReader(gcnew IO::MemoryStream(ByteArray));

	return AvalonEdit::Highlighting::Xshd::HighlightingLoader::Load(XmlReader, AvalonEdit::Highlighting::HighlightingManager::Instance);
}

SyntaxHighlightingManager::SyntaxHighlightingManager(obScriptParsing::IObScriptIdentifiers^ ScriptIdentifiers)
{
	this->ScriptIdentifiers = ScriptIdentifiers;

	HighlightingColors = gcnew Colors;
	HighlightingColors->UpdateFromPreferences();
	CommentSpanRuleSet = GenerateCommentSpanRuleSet(HighlightingColors);
	MainRuleSet = GenerateMainRuleSet(HighlightingColors, CommentSpanRuleSet);
	RegisteredEditors = gcnew Dictionary<BgAnalyserT^, AvalonEdit::TextEditor^>;

	DelegatePreferencesChanged = gcnew System::EventHandler(this, &SyntaxHighlightingManager::Preferences_Changed);
	DelegateBgAnalysisComplete = gcnew model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler(this, &SyntaxHighlightingManager::BgAnalysis_Complete);

	preferences::SettingsHolder::Get()->PreferencesChanged += DelegatePreferencesChanged;
}

SyntaxHighlightingManager::~SyntaxHighlightingManager()
{
	RegisteredEditors->Clear();

	preferences::SettingsHolder::Get()->PreferencesChanged -= DelegatePreferencesChanged;

	SAFEDELETE_CLR(DelegatePreferencesChanged);
	SAFEDELETE_CLR(DelegateBgAnalysisComplete);
}

void SyntaxHighlightingManager::RegisterScriptDocument(model::IScriptDocument^ Document, AvalonEdit::TextEditor^ TextEditor)
{
	if (RegisteredEditors->ContainsKey(Document->BackgroundAnalyzer))
		throw gcnew ArgumentException("Document already registered");

	RegisteredEditors->Add(Document->BackgroundAnalyzer, TextEditor);
	Document->BackgroundAnalyzer->SemanticAnalysisComplete += DelegateBgAnalysisComplete;
}

void SyntaxHighlightingManager::DeregisterScriptDocument(model::IScriptDocument^ Document)
{
	if (!RegisteredEditors->ContainsKey(Document->BackgroundAnalyzer))
		throw gcnew ArgumentException("Document was not previously registered");

	RegisteredEditors->Remove(Document->BackgroundAnalyzer);
	Document->BackgroundAnalyzer->SemanticAnalysisComplete -= DelegateBgAnalysisComplete;
}

SyntaxHighlightingManager^ SyntaxHighlightingManager::Get()
{
	if (Singleton == nullptr)
		Singleton = gcnew SyntaxHighlightingManager(gcnew obScriptParsing::ObseIdentifiers);

	return Singleton;
}


} // namespace avalonEdit


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse