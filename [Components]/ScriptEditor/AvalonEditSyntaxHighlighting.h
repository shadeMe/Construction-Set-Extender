#pragma once

#include "IScriptEditorModel.h"
#include "SemanticAnalysis.h"


namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


namespace avalonEdit
{


using namespace ICSharpCode;


interface class IXshdElement
{
	virtual String^ Serialize();
};

ref struct XshdPropertyName : public IXshdElement
{
	property String^ Name;

	XshdPropertyName();
	XshdPropertyName(String^ Name);

	virtual String^ Serialize();
};

ref struct XshdPropertyColor : public IXshdElement
{
	property Color Foreground;
	property Color Background;

	XshdPropertyColor();
	XshdPropertyColor(Drawing::Color ForeColor, Drawing::Color BackColor);

	virtual String^ Serialize();
};

ref struct XshdPropertyFont : public IXshdElement
{
	property Windows::FontWeight Weight;
	property Windows::FontStyle Style;

	XshdPropertyFont();
	XshdPropertyFont(bool Bold, bool Italic);

	virtual String^ Serialize();
};


ref struct XshdColor : public IXshdElement
{
	property XshdPropertyName^ Name;
	property XshdPropertyColor^ Color;
	property XshdPropertyFont^ Font;

	XshdColor(String^ Name);
	XshdColor(String^ Name, Drawing::Color ForeColor, Drawing::Color BackColor, bool Bold, bool Italic);

	void Update(Drawing::Color ForeColor, Drawing::Color BackColor, bool Bold, bool Italic);
	virtual String^ Serialize();
	String^ SerializeAsProperty();
};

ref struct XshdKeywords : public IXshdElement
{
private:
	property HashSet<String^>^ Words;
public:
	property XshdColor^ Color;

	XshdKeywords(XshdColor^ NamedColor);

	void AddKeyword(String^ Keyword);
	void ClearKeywords();
	virtual String^ Serialize();
};

ref struct XshdRuleSet : public IXshdElement
{
private:
	property List<IXshdElement^>^ Children;
public:
	property XshdPropertyName^ Name;
	property bool IgnoreCase;

	XshdRuleSet();
	XshdRuleSet(String^ Name);

	void AddChild(IXshdElement^ Child);
	XshdRuleSet^ ShallowCopy();
	virtual String^ Serialize();
	String^ SerializeAsProperty();
};

ref struct XshdRule : public IXshdElement
{
	property XshdColor^ Color;
	property String^ Value;

	XshdRule(XshdColor^ NamedColor);

	virtual String^ Serialize();
};

ref struct XshdSpan : public IXshdElement
{
public:
	property XshdColor^ Color;
	property String^ Begin;
	property String^ End;
	property bool MultiLine;
	property XshdRuleSet^ RuleSet;

	XshdSpan(XshdColor^ NamedColor);
	XshdSpan(XshdColor^ NamedColor, XshdRuleSet^ NamedRuleSet);

	virtual String^ Serialize();
};


ref class SyntaxHighlightingManager
{
	static SyntaxHighlightingManager^ Singleton = nullptr;

	ref struct Colors
	{
		property XshdColor^ ColorComment;
		property XshdColor^ ColorPreprocessor;
		property XshdColor^ ColorKeyword;
		property XshdColor^ ColorScriptBlock;
		property XshdColor^ ColorDigit;
		property XshdColor^ ColorString;
		property XshdColor^ ColorLocalVariable;
		property XshdColor^ ColorTaskComment;

		Colors();

		void UpdateFromPreferences();
		String^ Serialize();
	};

	using BgAnalyserT = model::components::IBackgroundSemanticAnalyzer;

	Colors^ HighlightingColors;
	XshdRuleSet^ MainRuleSet;
	XshdRuleSet^ CommentSpanRuleSet;
	obScriptParsing::IObScriptIdentifiers^ ScriptIdentifiers;
	Dictionary<BgAnalyserT^, AvalonEdit::TextEditor^>^ RegisteredEditors;

	EventHandler^ DelegatePreferencesChanged;
	model::components::IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler^ DelegateBgAnalysisComplete;

	void Preferences_Changed(Object^ Sender, EventArgs^ E);
	void BgAnalysis_Complete(Object^ Sender, BgAnalyserT::AnalysisCompleteEventArgs^ E);

	XshdRuleSet^ GenerateMainRuleSet(Colors^ XshdColors, XshdRuleSet^ CommentSpanRules);
	XshdRuleSet^ GenerateCommentSpanRuleSet(Colors^ XshdColors);

	SyntaxHighlightingManager(obScriptParsing::IObScriptIdentifiers^ ScriptIdentifiers);
public:
	~SyntaxHighlightingManager();

	void RegisterScriptDocument(model::IScriptDocument^ Document, AvalonEdit::TextEditor^ TextEditor);
	void DeregisterScriptDocument(model::IScriptDocument^ Document);
	AvalonEdit::Highlighting::IHighlightingDefinition^ GenerateHighlightingDefinition(obScriptParsing::AnalysisData^ SemanticAnalysisData);

	static SyntaxHighlightingManager^ Get();
};


} // namespace avalonEdit


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse