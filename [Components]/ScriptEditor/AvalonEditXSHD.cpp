#include "AvalonEditXSHD.h"
#include "Globals.h"
#include "ScriptEditorPreferences.h"

namespace ConstructionSetExtender
{
	using namespace System::Xml;
	using namespace ICSharpCode;

	namespace TextEditors
	{
		namespace AvalonEditor
		{
			String^ IXSHDElement::SerializeNameProperty(String^ Name)
			{
				String^ Result = " ";

				if (Name != "")
					Result += "name=\"" + Name + "\"";

				return Result;
			}

			String^ IXSHDElement::SerializeFontWeightProperty(bool Bold)
			{
				String^ Result = " ";

				if (Bold)
					Result += "fontWeight=\"bold\"";

				return Result;
			}

			String^ IXSHDElement::SerializeColorProperty(Color Foreground, Color Background)
			{
				String^ Result = " ";

				if (Foreground != Color::GhostWhite)
					Result += "foreground=\"#" + Foreground.R.ToString("X2") +
					Foreground.G.ToString("X2") +
					Foreground.B.ToString("X2") + "\"";

				if (Background != Color::GhostWhite)
					Result += " background=\"#" + Background.R.ToString("X2") +
					Background.G.ToString("X2") +
					Background.B.ToString("X2") + "\"";

				return Result;
			}

			String^ IXSHDElement::SerializeNamedColorProperty(XSHDColor^ NamedColor)
			{
				if (NamedColor != nullptr)
					return " color=\"" + NamedColor->Name + "\"";
				else
					return "";
			}

			String^ XSHDColor::Serialize()
			{
				String^ Result = "<Color";

				Result += SerializeNameProperty(Name);
				Result += SerializeColorProperty(Foreground, Background);
				Result += SerializeFontWeightProperty(Bold);

				Result += " />";

				return Result;
			}

			String^ XSHDWord::Serialize()
			{
				return "<Word>" + Value + "</Word>";
			}

			String^ XSHDKeywords::Serialize()
			{
				String^ Result = "<Keywords";

				Result += SerializeColorProperty(Foreground, Background);
				Result += SerializeNamedColorProperty(NamedColor);
				Result += ">" + Environment::NewLine;

				for each (IXSHDElement^ Itr in Words)
					Result += Itr->Serialize() + Environment::NewLine;

				Result += "</Keywords>";

				return Result;
			}

			void XSHDKeywords::AddWord(XSHDWord^ Word)
			{
				Words->AddLast(Word);
			}

			String^ XSHDBegin::Serialize()
			{
				String^ Result = "<Begin";

				Result += SerializeNamedColorProperty(NamedColor);
				Result += ">" + Value + "</Begin>";

				return Result;
			}

			String^ XSHDEnd::Serialize()
			{
				return "<End>" + Value + "</End>";
			}

			String^ XSHDSpan::Serialize()
			{
				String^ Result = "<Span";

				Result += SerializeNamedColorProperty(NamedColor);
				if (MultiLine)
					Result += " multiline = \"true\"";
				if (Ruleset != nullptr)
					Result += " ruleSet = \"" + Ruleset->Name + "\"";
				Result += ">" + Environment::NewLine;

				for each (IXSHDElement^ Itr in Children)
					Result += Itr->Serialize() + Environment::NewLine;

				Result += "</Span>";

				return Result;
			}

			void XSHDSpan::AddChild(IXSHDElement^ Child)
			{
				Children->AddLast(Child);
			}

			String^ XSHDRuleset::Serialize()
			{
				String^ Result = "<RuleSet";
				Result += SerializeNameProperty(Name);
				Result += " ignoreCase = \"true\">" + Environment::NewLine;

				for each (IXSHDElement^ Itr in Children)
					Result += Itr->Serialize() + Environment::NewLine;

				Result += "</RuleSet>";

				return Result;
			}

			void XSHDRuleset::AddChild(IXSHDElement^ Child)
			{
				Children->AddLast(Child);
			}

			String^ XSHDRule::Serialize()
			{
				String^ Result = "<Rule";

				Result += SerializeNamedColorProperty(NamedColor);
				Result += ">" + Environment::NewLine;

				Result += Value + Environment::NewLine;

				Result += "</Rule>";

				return Result;
			}

			String^ XSHDArbitrary::Serialize()
			{
				return Value;
			}

			List<String^>^ AvalonEditXSHDManager::GetKeywordList( void )
			{
				List<String^>^ KeywordList = gcnew List<String^>();

				KeywordList->Add("int");
				KeywordList->Add("short");
				KeywordList->Add("long");
				KeywordList->Add("ref");
				KeywordList->Add("reference");
				KeywordList->Add("float");
				KeywordList->Add("return");
				KeywordList->Add("if");
				KeywordList->Add("else");
				KeywordList->Add("elseif");
				KeywordList->Add("endif");
				KeywordList->Add("scriptname");
				KeywordList->Add("scn");
				KeywordList->Add("set");
				KeywordList->Add("to");

				KeywordList->Add("let");
				KeywordList->Add("testexpr");
				KeywordList->Add("typeof");
				KeywordList->Add("eval");

				KeywordList->Add("foreach");
				KeywordList->Add("while");
				KeywordList->Add("continue");
				KeywordList->Add("break");
				KeywordList->Add("loop");

				KeywordList->Add("call");
				KeywordList->Add("setfunctionvalue");

				KeywordList->Add("seteventhandler");
				KeywordList->Add("removeeventhandler");
				KeywordList->Add("dispatchevent");

				KeywordList->Add("array_var");
				KeywordList->Add("string_var");

				KeywordList->Add("player");
				KeywordList->Add("playerref");

				return KeywordList;
			}

			List<String^>^ AvalonEditXSHDManager::GetBlockTypeList( void )
			{
				List<String^>^ BlockTypeList = gcnew List<String^>();

				BlockTypeList->Add("begin");
				BlockTypeList->Add("end");
				BlockTypeList->Add("GameMode");
				BlockTypeList->Add("MenuMode");
				BlockTypeList->Add("OnActivate");
				BlockTypeList->Add("OnActorEquip");
				BlockTypeList->Add("OnActorUnequip");
				BlockTypeList->Add("OnAdd");
				BlockTypeList->Add("OnAlarm");
				BlockTypeList->Add("OnAlarmVictim");
				BlockTypeList->Add("OnDeath");
				BlockTypeList->Add("OnDrop");
				BlockTypeList->Add("OnEquip");
				BlockTypeList->Add("OnHit");
				BlockTypeList->Add("OnHitWith");
				BlockTypeList->Add("OnKnockout");
				BlockTypeList->Add("OnLoad");
				BlockTypeList->Add("OnMagicEffectHit");
				BlockTypeList->Add("OnMurder");
				BlockTypeList->Add("OnPackageChange");
				BlockTypeList->Add("OnPackageDone");
				BlockTypeList->Add("OnPackageStart");
				BlockTypeList->Add("OnReset");
				BlockTypeList->Add("OnSell");
				BlockTypeList->Add("OnTrigger");
				BlockTypeList->Add("OnTriggerActor");
				BlockTypeList->Add("OnTriggerMob");
				BlockTypeList->Add("OnUnequip");
				BlockTypeList->Add("ScriptEffectFinish");
				BlockTypeList->Add("ScriptEffectStart");
				BlockTypeList->Add("ScriptEffectUpdate");
				BlockTypeList->Add("Function");

				// OBSE compiler overridden blocks
				BlockTypeList->Add("_begin");
				BlockTypeList->Add("_end");
				BlockTypeList->Add("_GameMode");
				BlockTypeList->Add("_MenuMode");
				BlockTypeList->Add("_OnActivate");
				BlockTypeList->Add("_OnActorEquip");
				BlockTypeList->Add("_OnActorUnequip");
				BlockTypeList->Add("_OnAdd");
				BlockTypeList->Add("_OnAlarm");
				BlockTypeList->Add("_OnAlarmVictim");
				BlockTypeList->Add("_OnDeath");
				BlockTypeList->Add("_OnDrop");
				BlockTypeList->Add("_OnEquip");
				BlockTypeList->Add("_OnHit");
				BlockTypeList->Add("_OnHitWith");
				BlockTypeList->Add("_OnKnockout");
				BlockTypeList->Add("_OnLoad");
				BlockTypeList->Add("_OnMagicEffectHit");
				BlockTypeList->Add("_OnMurder");
				BlockTypeList->Add("_OnPackageChange");
				BlockTypeList->Add("_OnPackageDone");
				BlockTypeList->Add("_OnPackageStart");
				BlockTypeList->Add("_OnReset");
				BlockTypeList->Add("_OnSell");
				BlockTypeList->Add("_OnTrigger");
				BlockTypeList->Add("_OnTriggerActor");
				BlockTypeList->Add("_OnTriggerMob");
				BlockTypeList->Add("_OnUnequip");
				BlockTypeList->Add("_ScriptEffectFinish");
				BlockTypeList->Add("_ScriptEffectStart");
				BlockTypeList->Add("_ScriptEffectUpdate");
				BlockTypeList->Add("_Function");

				return BlockTypeList;
			}

			void AvalonEditXSHDManager::UpdateBaseDefinitions(void)
			{
				bool Bold = PREFERENCES->FetchSettingAsInt("BoldFacedHighlighting", "Appearance");
				Color Comments = PREFERENCES->LookupColorByKey("SyntaxCommentsColor");
				Color Preprocessor = PREFERENCES->LookupColorByKey("SyntaxPreprocessorColor");
				Color Keywords = PREFERENCES->LookupColorByKey("SyntaxKeywordsColor");
				Color Blocks = PREFERENCES->LookupColorByKey("SyntaxScriptBlocksColor");
				Color Delimiters = PREFERENCES->LookupColorByKey("SyntaxDelimitersColor");
				Color Digits = PREFERENCES->LookupColorByKey("SyntaxDigitsColor");
				Color Strings = PREFERENCES->LookupColorByKey("SyntaxStringsColor");
				Color Vars = PREFERENCES->LookupColorByKey("SyntaxLocalVarsColor");

				LinkedList<IXSHDElement^>^ Contents = gcnew LinkedList<IXSHDElement^>();
				LinkedList<XSHDColor^>^ SerializedColors = gcnew LinkedList<XSHDColor^>();
				LinkedList<String^>^ SerializedRulesets = gcnew LinkedList<String^>();

				delete LocalVarsColor;
				LocalVarsColor = gcnew XSHDColor("VarColor", Vars, Color::GhostWhite, Bold);
				SerializedColors->AddLast(LocalVarsColor);

				// digits
				XSHDColor^ DigitColor = gcnew XSHDColor("DigitColor", Digits, Color::GhostWhite, Bold);
				XSHDRule^ DigitRule = gcnew XSHDRule(DigitColor, "\\b0[xX][0-9a-fA-F]+\r\n|\\b\r\n(\\d+(\\.[0-9]+)?\r\n|\\.[0-9]+\r\n)\r\n([eE][+-]?[0-9]+)?");

				SerializedColors->AddLast(DigitColor);
				Contents->AddLast(DigitRule);

				// delimiters
				XSHDColor^ DelimiterColor = gcnew XSHDColor("DelimiterColor", Delimiters, Color::GhostWhite, Bold);
				XSHDRule^ DelimiterRule = gcnew XSHDRule(DelimiterColor, "[?,.;:>#$=()\\[\\]{}+\\-/%*&lt;&gt;^+~!|&amp;]+");

				SerializedColors->AddLast(DelimiterColor);
				Contents->AddLast(DelimiterRule);

				// strings
				XSHDColor^ StringColor = gcnew XSHDColor("StringColor", Strings, Color::GhostWhite, Bold);
				XSHDSpan^ StringSpan = gcnew XSHDSpan(StringColor, nullptr, false);
				StringSpan->AddChild(gcnew XSHDBegin(nullptr, "\""));
				StringSpan->AddChild(gcnew XSHDEnd("\""));

				XSHDRuleset^ StringRulesetInner = gcnew XSHDRuleset("StringInner");
				StringRulesetInner->AddChild(gcnew XSHDArbitrary("<Span begin=\"\\\\\" end=\".\"/>"));
				StringSpan->AddChild(StringRulesetInner);

				SerializedColors->AddLast(StringColor);
				Contents->AddLast(StringSpan);

				// keywords
				XSHDColor^ KWColor = gcnew XSHDColor("KeywordColor", Keywords, Color::GhostWhite, Bold);
				XSHDKeywords^ KWKeywords = gcnew XSHDKeywords(KWColor, Color::GhostWhite, Color::GhostWhite, Bold);

				for each (String^ Itr in GetKeywordList())
					KWKeywords->AddWord(gcnew XSHDWord(Itr));

				SerializedColors->AddLast(KWColor);
				Contents->AddLast(KWKeywords);

				// script blocks
				XSHDColor^ HighlightColor = gcnew XSHDColor("BlockColor", Blocks, Color::GhostWhite, Bold);
				XSHDKeywords^ HighlightKeywords = gcnew XSHDKeywords(HighlightColor, Color::GhostWhite, Color::GhostWhite, Bold);

				for each (String^ Itr in GetBlockTypeList())
					HighlightKeywords->AddWord(gcnew XSHDWord(Itr));

				SerializedColors->AddLast(HighlightColor);
				Contents->AddLast(HighlightKeywords);

				// comments and preprocessor
				XSHDColor^ CommentColor = gcnew XSHDColor("CommentColor", Comments, Color::GhostWhite, Bold);
				XSHDColor^ PreprocessorColor = gcnew XSHDColor("PreprocessorColor", Preprocessor, Color::GhostWhite, Bold);
				XSHDColor^ ReminderColor = gcnew XSHDColor("ReminderColor", Color::Red, Color::GhostWhite, Bold);

				XSHDRuleset^ CommentMarkerRuleset = gcnew XSHDRuleset("CommentPreprocessorMarker");
				XSHDKeywords^ CommentMarkerPreprocessorKeywords = gcnew XSHDKeywords(PreprocessorColor, Color::GhostWhite, Color::GhostWhite, true);
				XSHDKeywords^ CommentMarkerReminderKeywords = gcnew XSHDKeywords(ReminderColor, Color::GhostWhite, Color::GhostWhite, true);

				CommentMarkerPreprocessorKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
					[(int)CSEPreprocessorDirective::EncodingType::SingleLine] +
					CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::DirectiveType::Define]));
				CommentMarkerPreprocessorKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
					[(int)CSEPreprocessorDirective::EncodingType::MultiLine] +
					CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::DirectiveType::Define]));

				CommentMarkerPreprocessorKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
					[(int)CSEPreprocessorDirective::EncodingType::SingleLine] +
					CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::DirectiveType::Enum]));
				CommentMarkerPreprocessorKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
					[(int)CSEPreprocessorDirective::EncodingType::MultiLine] +
					CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::DirectiveType::Enum]));

				CommentMarkerPreprocessorKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
					[(int)CSEPreprocessorDirective::EncodingType::SingleLine] +
					CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::DirectiveType::If]));
				CommentMarkerPreprocessorKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
					[(int)CSEPreprocessorDirective::EncodingType::MultiLine] +
					CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::DirectiveType::If]));

				CommentMarkerPreprocessorKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
					[(int)CSEPreprocessorDirective::EncodingType::SingleLine] +
					CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::DirectiveType::Import]));
				CommentMarkerPreprocessorKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
					[(int)CSEPreprocessorDirective::EncodingType::MultiLine] +
					CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::DirectiveType::Import]));

				CommentMarkerReminderKeywords->AddWord(gcnew XSHDWord("TODO"));
				CommentMarkerReminderKeywords->AddWord(gcnew XSHDWord("HACK"));

				CommentMarkerRuleset->AddChild(CommentMarkerPreprocessorKeywords);
				CommentMarkerRuleset->AddChild(CommentMarkerReminderKeywords);

				this->CommentMarkerRuleset = gcnew String(CommentMarkerRuleset->Serialize());

				XSHDSpan^ CommentSpan = gcnew XSHDSpan(CommentColor, CommentMarkerRuleset, false);
				CommentSpan->AddChild(gcnew XSHDBegin(nullptr, ";"));

				XSHDSpan^ PreprocessorBlockSpan = gcnew XSHDSpan(PreprocessorColor, CommentMarkerRuleset, true);
				PreprocessorBlockSpan->AddChild(gcnew XSHDBegin(nullptr, ";{"));
				PreprocessorBlockSpan->AddChild(gcnew XSHDEnd(";}"));

				SerializedColors->AddLast(CommentColor);
				SerializedColors->AddLast(PreprocessorColor);
				SerializedColors->AddLast(ReminderColor);
				Contents->AddLast(PreprocessorBlockSpan);
				Contents->AddLast(CommentSpan);

				String^ Definition = "";
				for each (IXSHDElement^ Itr in Contents)
					Definition += Itr->Serialize() + Environment::NewLine;

				SerializedRulesets->AddLast(Definition);

				// generate XML
				StableDefinitions = "<?xml version=\"1.0\"?>" + Environment::NewLine + "<SyntaxDefinition name=\"ObScript\" xmlns=\"http://icsharpcode.net/sharpdevelop/syntaxdefinition/2008\">" + Environment::NewLine;

				for each (XSHDColor^ Itr in SerializedColors)
					StableDefinitions += Itr->Serialize() + Environment::NewLine;

				StableDefinitions += CommentMarkerRuleset->Serialize() + Environment::NewLine;

				StableDefinitions += "<RuleSet ignoreCase = \"true\">" + Environment::NewLine;
				for each (String^ Itr in SerializedRulesets)
					StableDefinitions += Itr + Environment::NewLine;

				// leave the closing tags for later
			}

			AvalonEditXSHDManager::AvalonEditXSHDManager() :
				CommentMarkerRuleset(""),
				StableDefinitions(""),
				LocalVarsColor(nullptr)
			{
				;//
			}

			AvalonEditXSHDManager::~AvalonEditXSHDManager()
			{
				SAFEDELETE_CLR(LocalVarsColor);
			}

			AvalonEditHighlightingDefinition^ AvalonEditXSHDManager::GenerateHighlightingDefinition(List<String^>^ LocalVariables)
			{
				String^ OutDefs = StableDefinitions;

				if (LocalVariables && LocalVariables->Count)
				{
					bool Bold = PREFERENCES->FetchSettingAsInt("BoldFacedHighlighting", "Appearance");
					XSHDKeywords^ HighlightKeywords = gcnew XSHDKeywords(LocalVarsColor, Color::GhostWhite, Color::GhostWhite, Bold);

					for each (String^ Itr in LocalVariables)
						HighlightKeywords->AddWord(gcnew XSHDWord(Itr));
					OutDefs += HighlightKeywords->Serialize() + Environment::NewLine;
				}

				OutDefs += "</RuleSet>" + Environment::NewLine;
				OutDefs += "</SyntaxDefinition>";

#if 0
				StreamWriter^ XSHDWriter = gcnew StreamWriter(Globals::AppPath + "XSHD-CSE.txt");
				XSHDWriter->Write(OutDefs);
				XSHDWriter->Close();
#endif

				array<Byte>^ ByteArray = System::Text::Encoding::ASCII->GetBytes(OutDefs);
				XmlTextReader^ Reader = gcnew XmlTextReader(gcnew IO::MemoryStream(ByteArray));

				return AvalonEdit::Highlighting::Xshd::HighlightingLoader::Load(Reader, AvalonEdit::Highlighting::HighlightingManager::Instance);
			}
}
	}
}