#include "AvalonEditXSHD.h"
#include "IntelliSense.h"
#include "Globals.h"

using namespace System::Xml;
using namespace ICSharpCode;
using namespace IntelliSense;

namespace AvalonEditXSHD
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

	void AvalonEditXSHDManager::CreateCommentPreprocessorRuleset(Color Foreground, Color Background, bool Bold, Color PreprocessorForeground)
	{
		LinkedList<IXSHDElement^>^ Contents = gcnew LinkedList<IXSHDElement^>();

		XSHDColor^ CommentColor = gcnew XSHDColor("CommentColor", Foreground, Background, Bold);
		XSHDColor^ PreprocessorColor = gcnew XSHDColor("PreprocessorColor", PreprocessorForeground, Background, Bold);

		ColorDefinitions->AddLast(CommentColor);
		ColorDefinitions->AddLast(PreprocessorColor);

		XSHDRuleset^ CommentMarkerRuleset = gcnew XSHDRuleset("CommentPreprocessorMarker");			
		XSHDKeywords^ CommentMarkerKeywords = gcnew XSHDKeywords(PreprocessorColor, Color::GhostWhite, Color::GhostWhite, true);

		CommentMarkerKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
			[(int)CSEPreprocessorDirective::EncodingType::e_SingleLine] +
			CSEPreprocessorDirective::DirectiveIdentifier[
				(int)CSEPreprocessorDirective::DirectiveType::e_Define]));
		CommentMarkerKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
			[(int)CSEPreprocessorDirective::EncodingType::e_MultiLine] +
			CSEPreprocessorDirective::DirectiveIdentifier[
				(int)CSEPreprocessorDirective::DirectiveType::e_Define]));

		CommentMarkerKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
			[(int)CSEPreprocessorDirective::EncodingType::e_SingleLine] +
			CSEPreprocessorDirective::DirectiveIdentifier[
				(int)CSEPreprocessorDirective::DirectiveType::e_Enum]));
		CommentMarkerKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
			[(int)CSEPreprocessorDirective::EncodingType::e_MultiLine] +
			CSEPreprocessorDirective::DirectiveIdentifier[
				(int)CSEPreprocessorDirective::DirectiveType::e_Enum]));

		CommentMarkerKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
			[(int)CSEPreprocessorDirective::EncodingType::e_SingleLine] +
			CSEPreprocessorDirective::DirectiveIdentifier[
				(int)CSEPreprocessorDirective::DirectiveType::e_If]));
		CommentMarkerKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
			[(int)CSEPreprocessorDirective::EncodingType::e_MultiLine] +
			CSEPreprocessorDirective::DirectiveIdentifier[
				(int)CSEPreprocessorDirective::DirectiveType::e_If]));

		CommentMarkerKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
			[(int)CSEPreprocessorDirective::EncodingType::e_SingleLine] +
			CSEPreprocessorDirective::DirectiveIdentifier[
				(int)CSEPreprocessorDirective::DirectiveType::e_Import]));
		CommentMarkerKeywords->AddWord(gcnew XSHDWord(CSEPreprocessorDirective::EncodingIdentifier
			[(int)CSEPreprocessorDirective::EncodingType::e_MultiLine] +
			CSEPreprocessorDirective::DirectiveIdentifier[
				(int)CSEPreprocessorDirective::DirectiveType::e_Import]));

		CommentMarkerRuleset->AddChild(CommentMarkerKeywords);
		this->CommentMarkerRuleset = gcnew String(CommentMarkerRuleset->Serialize());

		XSHDSpan^ CommentSpan = gcnew XSHDSpan(CommentColor, CommentMarkerRuleset, false);
		CommentSpan->AddChild(gcnew XSHDBegin(nullptr, ";"));

		XSHDSpan^ PreprocessorBlockSpan = gcnew XSHDSpan(CommentColor, CommentMarkerRuleset, true);
		PreprocessorBlockSpan->AddChild(gcnew XSHDBegin(nullptr, ";{"));
		PreprocessorBlockSpan->AddChild(gcnew XSHDEnd(";}"));

//		Contents->AddLast(PreprocessorBlockSpan);
		Contents->AddLast(CommentSpan);

		String^ Definition = "";
		for each (IXSHDElement^ Itr in Contents)
			Definition += Itr->Serialize() + Environment::NewLine;

		Definitions->AddLast(Definition);
	}

	void AvalonEditXSHDManager::CreateRuleset(Rulesets Ruleset, Color Foreground, Color Background, bool Bold)
	{
		LinkedList<IXSHDElement^>^ Contents = gcnew LinkedList<IXSHDElement^>();

		switch (Ruleset)
		{
		case Rulesets::e_Digit:
			{
				XSHDColor^ DigitColor = gcnew XSHDColor("DigitColor", Foreground, Background, Bold);
				XSHDRule^ DigitRule = gcnew XSHDRule(DigitColor, "\\b0[xX][0-9a-fA-F]+|\t\\b(\t\\d+(\\.[0-9]+)?|\t\\.[0-9]+)([eE][+-]?[0-9]+)?");

				ColorDefinitions->AddLast(DigitColor);
				Contents->AddLast(DigitRule);
			}
			break;
		case Rulesets::e_Delimiter:
			{
				XSHDColor^ DelimiterColor = gcnew XSHDColor("DelimiterColor", Foreground, Background, Bold);
				XSHDRule^ DelimiterRule = gcnew XSHDRule(DelimiterColor, "[?,.;:>#$=()\\[\\]{}+\\-/%*&lt;&gt;^+~!|&amp;]+");

				ColorDefinitions->AddLast(DelimiterColor);
				Contents->AddLast(DelimiterRule);
			}
			break;
		case Rulesets::e_String:
			{
				XSHDColor^ StringColor = gcnew XSHDColor("StringColor", Foreground, Background, Bold);
				XSHDSpan^ StringSpan = gcnew XSHDSpan(StringColor, nullptr, false);
				StringSpan->AddChild(gcnew XSHDBegin(nullptr, "\""));
				StringSpan->AddChild(gcnew XSHDEnd("\""));

				XSHDRuleset^ StringRulesetInner = gcnew XSHDRuleset("StringInner");
				StringRulesetInner->AddChild(gcnew XSHDArbitrary("<Span begin=\"\\\\\" end=\".\"/>"));
				StringSpan->AddChild(StringRulesetInner);

				ColorDefinitions->AddLast(StringColor);
				Contents->AddLast(StringSpan);
			}
			break;
		case Rulesets::e_Keywords:
			{
				XSHDColor^ HighlightColor = gcnew XSHDColor("CommandColor", Foreground, Background, Bold);
				XSHDKeywords^ HighlightKeywords = gcnew XSHDKeywords(HighlightColor, Color::GhostWhite, Color::GhostWhite, Bold);

				HighlightKeywords->AddWord(gcnew XSHDWord("begin"));
				HighlightKeywords->AddWord(gcnew XSHDWord("end"));
				HighlightKeywords->AddWord(gcnew XSHDWord("int"));
				HighlightKeywords->AddWord(gcnew XSHDWord("short"));
				HighlightKeywords->AddWord(gcnew XSHDWord("long"));
				HighlightKeywords->AddWord(gcnew XSHDWord("ref"));
				HighlightKeywords->AddWord(gcnew XSHDWord("reference"));
				HighlightKeywords->AddWord(gcnew XSHDWord("float"));
				HighlightKeywords->AddWord(gcnew XSHDWord("return"));
				HighlightKeywords->AddWord(gcnew XSHDWord("if"));
				HighlightKeywords->AddWord(gcnew XSHDWord("else"));
				HighlightKeywords->AddWord(gcnew XSHDWord("elseif"));
				HighlightKeywords->AddWord(gcnew XSHDWord("endif"));
				HighlightKeywords->AddWord(gcnew XSHDWord("scriptname"));
				HighlightKeywords->AddWord(gcnew XSHDWord("scn"));
				HighlightKeywords->AddWord(gcnew XSHDWord("set"));
				HighlightKeywords->AddWord(gcnew XSHDWord("to"));

				HighlightKeywords->AddWord(gcnew XSHDWord("let"));
				HighlightKeywords->AddWord(gcnew XSHDWord("testexpr"));
				HighlightKeywords->AddWord(gcnew XSHDWord("typeof"));
				HighlightKeywords->AddWord(gcnew XSHDWord("eval"));

				HighlightKeywords->AddWord(gcnew XSHDWord("foreach"));
				HighlightKeywords->AddWord(gcnew XSHDWord("while"));
				HighlightKeywords->AddWord(gcnew XSHDWord("continue"));
				HighlightKeywords->AddWord(gcnew XSHDWord("break"));
				HighlightKeywords->AddWord(gcnew XSHDWord("loop"));

				HighlightKeywords->AddWord(gcnew XSHDWord("call"));
				HighlightKeywords->AddWord(gcnew XSHDWord("setfunctionvalue"));
				HighlightKeywords->AddWord(gcnew XSHDWord("getcallingscript"));

				HighlightKeywords->AddWord(gcnew XSHDWord("array_var"));
				HighlightKeywords->AddWord(gcnew XSHDWord("string_var"));

				HighlightKeywords->AddWord(gcnew XSHDWord("player"));
				HighlightKeywords->AddWord(gcnew XSHDWord("playerref"));

				ColorDefinitions->AddLast(HighlightColor);
				Contents->AddLast(HighlightKeywords);
			}
			break;
		case Rulesets::e_BlockTypes:
			{
				XSHDColor^ HighlightColor = gcnew XSHDColor("BlockColor", Foreground, Background, Bold);
				XSHDKeywords^ HighlightKeywords = gcnew XSHDKeywords(HighlightColor, Color::GhostWhite, Color::GhostWhite, Bold);

				HighlightKeywords->AddWord(gcnew XSHDWord("GameMode"));
				HighlightKeywords->AddWord(gcnew XSHDWord("MenuMode"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnActivate"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnActorEquip"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnActorUnequip"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnAdd"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnAlarm"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnAlarmVictim"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnDeath"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnDrop"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnEquip"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnHit"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnHitWith"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnKnockout"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnLoad"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnMagicEffectHit"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnMurder"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnPackageChange"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnPackageDone"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnPackageStart"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnReset"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnSell"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnTrigger"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnTriggerActor"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnTriggerMob"));
				HighlightKeywords->AddWord(gcnew XSHDWord("OnUnequip"));
				HighlightKeywords->AddWord(gcnew XSHDWord("ScriptEffectFinish"));
				HighlightKeywords->AddWord(gcnew XSHDWord("ScriptEffectStart"));
				HighlightKeywords->AddWord(gcnew XSHDWord("ScriptEffectUpdate"));
				HighlightKeywords->AddWord(gcnew XSHDWord("Function"));

				ColorDefinitions->AddLast(HighlightColor);
				Contents->AddLast(HighlightKeywords);
			}
			break;

		case Rulesets::e_VanillaCommands:
			{
				XSHDColor^ HighlightColor = gcnew XSHDColor("VanillaCmdColor", Foreground, Background, Bold);
				XSHDKeywords^ HighlightKeywords = gcnew XSHDKeywords(HighlightColor, Color::GhostWhite, Color::GhostWhite, Bold);

				for each (IntelliSenseItem^ Itr in ISDB->Enumerables)
				{
					if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd)
					{
						CommandInfo^ Command = dynamic_cast<CommandInfo^>(Itr);
						if (Command->GetSource() == CommandInfo::SourceType::e_Vanilla)
							HighlightKeywords->AddWord(gcnew XSHDWord(Itr->GetIdentifier()));
					}
				}

				ColorDefinitions->AddLast(HighlightColor);
				Contents->AddLast(HighlightKeywords);
			}
			break;
		case Rulesets::e_OBSECommands:
			{
				XSHDColor^ HighlightColor = gcnew XSHDColor("OBSECmdColor", Foreground, Background, Bold);
				XSHDKeywords^ HighlightKeywords = gcnew XSHDKeywords(HighlightColor, Color::GhostWhite, Color::GhostWhite, Bold);

				for each (IntelliSenseItem^ Itr in ISDB->Enumerables)
				{
					if (Itr->GetType() == IntelliSenseItem::ItemType::e_Cmd)
					{
						CommandInfo^ Command = dynamic_cast<CommandInfo^>(Itr);
						if (Command->GetSource() == CommandInfo::SourceType::e_OBSE)
							HighlightKeywords->AddWord(gcnew XSHDWord(Itr->GetIdentifier()));
					}
				}

				ColorDefinitions->AddLast(HighlightColor);
				Contents->AddLast(HighlightKeywords);
			}
			break;
		}

		String^ Definition = "";
		for each (IXSHDElement^ Itr in Contents)
			Definition += Itr->Serialize() + Environment::NewLine;

		Definitions->AddLast(Definition);
	}

	void AvalonEditXSHDManager::RegisterDefinitions(String^ DefinitionName)
	{
		String^ CompleteDefinition = "<?xml version=\"1.0\"?>" + Environment::NewLine + "<SyntaxDefinition name=\"" + DefinitionName + "\" xmlns=\"http://icsharpcode.net/sharpdevelop/syntaxdefinition/2008\">" + Environment::NewLine;

		for each (XSHDColor^ Itr in ColorDefinitions)
			CompleteDefinition += Itr->Serialize() + Environment::NewLine;

		CompleteDefinition += CommentMarkerRuleset + Environment::NewLine;

		CompleteDefinition += "<RuleSet ignoreCase = \"true\">" + Environment::NewLine;
		for each (String^ Itr in Definitions)
			CompleteDefinition += Itr + Environment::NewLine;
		CompleteDefinition += "</RuleSet>" + Environment::NewLine;
		CompleteDefinition += "</SyntaxDefinition>";

#if 0
		StreamWriter^ XSHDWriter = gcnew StreamWriter(Globals::AppPath + "XSHD-CSE.txt");
		XSHDWriter->Write(CompleteDefinition);
		XSHDWriter->Close();
#endif

		array<Byte>^ ByteArray = System::Text::Encoding::ASCII->GetBytes(CompleteDefinition);
		XmlTextReader^ Reader = gcnew XmlTextReader(gcnew IO::MemoryStream(ByteArray));

		AvalonEdit::Highlighting::IHighlightingDefinition^ XSHDDef = AvalonEdit::Highlighting::Xshd::HighlightingLoader::Load(Reader, AvalonEdit::Highlighting::HighlightingManager::Instance);
		AvalonEdit::Highlighting::HighlightingManager::Instance->RegisterHighlighting(DefinitionName, gcnew array<String^>(1) { ".ObScript" }, XSHDDef);

		Initialized = true;
	}
}