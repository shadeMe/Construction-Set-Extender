#include "ScriptCommands.h"
#include "Array.h"
#include "ScriptRunner.h"

#include "Commands\Commands_General.h"
#include "Commands\Commands_Form.h"
#include "Commands\Commands_Reference.h"
#include "Commands\Commands_String.h"

namespace CSAutomationScript
{
	CommandTable				g_CSASCommandTable;

	// register commmand infos to the command table (for documentation purposes)
	BEGIN_CSASCOMMAND_PARAMINFO(Call, 1)
	{
		{ "Script Name", CSASDataElement::kParamType_String }
	};

	CSASCommandInfo kCSASCommandInfo_Return("Return", NULL,	"Stops the execution of the calling script and optionally returns a value.", "Example:<p><code class=\"s\">Return(45 + 12.4 * 0.1)</code></p>", 0, NULL, CSASDataElement::kParamType_Invalid, NULL);
	CSASCommandInfo kCSASCommandInfo_Call("Call", NULL, "Invokes a function call. Should be followed by a list of arguments matching the types expected by the function, if any. Returns whatever value is returned by the function, zero if the call fails for any reason or doesn't return a value. Return values must be explicitly cast to the expected type before use/assignment.",	"Example:<p><code class=\"s\">refVar = (ref)Call(\"UDFScript\", \"arg1\", 23.43, refVar)</code></p>", 1, kParams_Call,	CSASDataElement::kParamType_Multi, NULL);
	CSASCommandInfo kCSASCommandInfo_Break("Break", NULL, "Causes the loop to exit immediately, forcing execution to jump to the instruction immediately following the next Loop command.",	NULL, 0, NULL, CSASDataElement::kParamType_Invalid, NULL);
	CSASCommandInfo kCSASCommandInfo_Continue("Continue", NULL, "Skips to the rest of the body of a loop, returning execution to the top of the loop and evaluating the loop condition. If the condition passes, execution enters the loop body, otherwise the loop terminates and continues from the instruction following the corresponding Loop command. Only valid inside a loop context.", NULL, 0, NULL, CSASDataElement::kParamType_Invalid, NULL);
	CSASCommandInfo kCSASCommandInfo_GetSecondsPassed("GetSecondsPassed", NULL, "Returns the amount of time passed since the last execution of the calling script. Can only be called in global scripts.", NULL, 0, NULL, CSASDataElement::kParamType_Numeric, NULL);

	std::string CommandTable::SanitizeLinkableString(const char* String)
	{
		std::string SanitizedString(String);
		MakeLower(SanitizedString);
		std::replace(SanitizedString.begin(), SanitizedString.end(), ' ', '_');
		return SanitizedString;
	}

	bool CommandTable::GetParamInfoHasDetails(CSASParamInfo* ParamInfo)
	{
		const char* ParamName = ParamInfo->ParamName;

		if (!_stricmp(ParamName, "Format String") ||
			!_stricmp(ParamName, "Variable Value Type") ||
			!_stricmp(ParamName, "Attribute ID") ||
			!_stricmp(ParamName, "NPC Flags") ||
			!_stricmp(ParamName, "Creature Flags") ||
			!_stricmp(ParamName, "AI Stat ID") ||
			!_stricmp(ParamName, "AI Service Flags") ||
			!_stricmp(ParamName, "Skill ID") ||
			!_stricmp(ParamName, "Biped Model Slot Mask") ||
			!_stricmp(ParamName, "Biped Model Flags") ||
			!_stricmp(ParamName, "Harvest Season ID")
			)
		{
			return true;
		}
		else
			return false;
	}

	void CommandTable::AppendToStream(std::fstream& Out, const char* Fmt, ...)
	{
        va_list Args;
        char Buffer[0x1000] = {0};

        va_start(Args, Fmt);
        vsprintf_s(Buffer, sizeof(Buffer), Fmt, Args);
		va_end(Args);

        Out << Buffer;
	}

	const char*	CommandTable::GetSimpleStringForParamType(UInt8 ParamType)
	{
		switch (ParamType)
		{
		case CSASDataElement::kParamType_Invalid:
			return "nothing";
		case CSASDataElement::kParamType_Array:
			return "array";
		case CSASDataElement::kParamType_Numeric:
			return "numeric";
		case CSASDataElement::kParamType_Reference:
			return "ref";
		case CSASDataElement::kParamType_String:
			return "string";
		case CSASDataElement::kParamType_Multi:
			return "multi";
		default:
			return "<unknown>";
		}
	}

	void CommandTable::DumpCommandParamInfo(std::fstream& Out, CSASCommandInfo* Command)
	{
		for (int i = 0; i < Command->NoOfParams; i++)
		{
			CSASParamInfo* ParamInfo = &Command->Parameters[i];
			std::string ParamName(SanitizeLinkableString(ParamInfo->ParamName));

			if (GetParamInfoHasDetails(ParamInfo))
				AppendToStream(Out, " <a href=\"#ParamInfo_%s\">%s</a>:%s", ParamName.c_str(), ParamName.c_str(), GetSimpleStringForParamType(ParamInfo->ParamType));
			else
				AppendToStream(Out, " %s:%s", ParamName.c_str(), GetSimpleStringForParamType(ParamInfo->ParamType));
		}
	}

	void CommandTable::DumpCommandDefinition(std::fstream& Out, CSASCommandInfo* Command)
	{
		const char* LongName = Command->Name;
		const char*	ShortName = Command->Alias;

		AppendToStream(Out, "<p><a id=\"%s\" class=\"f\" href=\"http://cs.elderscrolls.com/constwiki/index.php/%s_(CSAS)\">%s</a>", LongName, LongName, LongName);
		if (ShortName)
			AppendToStream(Out, " (<i>%s</i>)", ShortName);

        AppendToStream(Out, " - %s<br />\n", Command->ShortDescription);
        AppendToStream(Out, "<code class=\"s\">(%s) ", GetSimpleStringForParamType(Command->ReturnType));
        AppendToStream(Out, LongName);
        DumpCommandParamInfo(Out, Command);
        AppendToStream(Out, "</code></p>\n\n");

		if (Command->LongDescription)
			AppendToStream(Out, "\n\n<p>%s</p>", Command->LongDescription);
	}

	void CommandTable::DumpParamInfoDetails(std::fstream& Out, const char* ParamName, const char* Details)
	{
		std::string SanitizedName(SanitizeLinkableString(ParamName));

		AppendToStream(Out, "<h3><a id=\"ParamInfo_%s\">%s</a></h3>\n", SanitizedName.c_str(), ParamName);
		AppendToStream(Out, "%s\n\n", Details);
	}

	void CommandTable::DumpFunctionSyntaxInfo(std::fstream& Out)
	{
		static const char* s_SyntaxInfo = "<h2><a id=\"Function_Syntax_Format\">Function Syntax Format</a></h2><pre class=\"code\">(returnValueType)    FunctionName    parameter1:type  parameter2:type</pre><h2><a id=\"Function_CallingSyntax_Format\">Function Calling Syntax Format</a></h2><pre class=\"code\">FunctionName(parameter1, parameter2)</pre><table><caption>Parameter Types: designated after colon</caption><tr><th>numeric</th><td>a positive or negative decimal number or integer</td><td>3.1415/-56</td></tr><tr class=\"alt\"><th>ref</th><td>a formID literal or ref returned by <a href=\"#GetFormByEditorID\">GetFormByEditorID</a>/<a href=\"#GetFormByFormID\">GetFormByFormID</a></td><td>0x00A12EF2/GetFormByEditorID(\"apple\")</td></tr><tr><th>string</th><td>a set of characters in quotes</td><td>\"Uriel Septim\"</td></tr><tr class=\"alt\"><th>array</th><td>(return type only) an array that can be used in ForEach expressions</td><td>--</td></tr><tr><th>multi</th><td>a value of any type numeric, string, ref or array</td><td>--</td></tr></table>";

		AppendToStream(Out, s_SyntaxInfo);
	}

	void CommandTable::DumpTOC(std::fstream& Out)
	{
		AppendToStream(Out, "<h2>Table of Contents</h2>\n\n");
		AppendToStream(Out, "<ul>");

		CommandInfoList::iterator MainItr, SubItr;

		for (MainItr = CommandList.begin(); MainItr != CommandList.end(); MainItr = SubItr)
		{
			const std::string& Category = MainItr->first;
			std::string SanitizedName(SanitizeLinkableString(Category.c_str()));
			AppendToStream(Out, "<li><a href=\"#%s\">%s</a></li>\n", SanitizedName.c_str(), Category.c_str());

			std::pair<CommandInfoList::iterator, CommandInfoList::iterator> KeyRange = CommandList.equal_range(Category);
			for (SubItr = KeyRange.first; SubItr != KeyRange.second; SubItr++)
				;//
		}
		AppendToStream(Out, "</ul>");
	}

	void CommandTable::DumpProlog(std::fstream& Out)
	{
		AppendToStream(Out, "<h1>CSE CSAS Command Documentation</h1><p><a href=\"http://cs.elderscrolls.com/constwiki/index.php/Category:Construction_Set_Extender\">Construction Set Extender @ Oblivion Construction Set Wiki</a> - follow the links throughout this documentation to gain access to additional information on the official Oblivion Construction Set Wiki. Users are encouraged to add to the wiki.<br />");
	}

	CSASCommandInfo* CommandTable::LookupCommandByIdentifier(const char* ID, bool AsAlias)
	{
		if (!ID)
			return NULL;

		for (CommandInfoList::iterator Itr = CommandList.begin(); Itr != CommandList.end(); Itr++)
		{
			if ((!AsAlias && Itr->second->Name && !_stricmp(ID, Itr->second->Name)) ||
				(AsAlias && Itr->second->Alias && !_stricmp(ID, Itr->second->Alias)))
			{
				return Itr->second;
			}
		}

		return NULL;
	}

	bool CommandTable::RegisterCommand(CSASCommandInfo* Command, const char* CategoryName)
	{
		if (!Command->Name)
			return false;

		if (LookupCommandByIdentifier(Command->Name) ||
			LookupCommandByIdentifier(Command->Alias, true))
		{
			return false;
		}

		CommandList.insert(std::make_pair<std::string, CSASCommandInfo*>(CategoryName, Command));
		return true;
	}

	void CommandTable::DumpDocumentation()
	{
		const char* s_Header = "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n\n<head>\n\n\t<meta content=\"text/html; charset=UTF-8\" http-equiv=\"content-type\" />\n\n\t<style type=\"text/css\">\n\n\t/* body */ \n\n\tbody {\n\n\t\n\n\tpadding: 10%%;\n\n\t}\n\n\t/* headings */\n\n\th1 {\n\n\t\n\n\tfont-size: 150%%;\n\n\t\n\n\tfont-weight: normal;\n\n\t}\n\n\th2 {\n\n\t\n\n\tfont-size: 130%%;\n\n\t\n\n\tfont-weight: normal;\n\n\t\n\n\tmargin-top: 40px;\n\n\t}\n\n\th3 {\n\n\t\n\n\tfont-size: 110%%;\n\n\t\n\n\tfont-weight: normal;\n\n\t\n\n\tmargin-top: 30px;\n\n\t\n\n\tcolor: #000099;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\th4 {\n\n\t\n\n\tfont-size: 100%%;\n\n\t\n\n\tfont-weight: bold;\n\n\t\n\n\tmargin-bottom: 0px;\n\n\t}\n\n\th5 {\n\n\t\n\n\tfont-size: 100%%;\n\n\t\n\n\tfont-weight: bold;\n\n\t\n\n\tmargin-bottom: 0px;\n\n\t\n\n\tcolor: #000099;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* tables */\n\n\ttable {\n\n\t\n\n\tborder: 1px solid black;\n\n\t\n\n\tmargin: 20px 0px 20px 0px;\n\n\t}\n\n\tcaption {\n\n\t\n\n\ttext-align: left;\n\n\t\n\n\tmargin-bottom: 10px;\n\n\t}\n\n\ttd, th {\n\n\t\n\n\tpadding: 5px;\n\n\t}\n\n\ttr.alt {\n\n\t\n\n\tcolor: black;\n\n\t\n\n\tbackground-color: #dedede;\n\n\t}\n\n\tcode.alt {\n\n\t\n\n\tcolor: 770000;\n\n\t\n\n\tbackground-color: #dedede;\n\n\t}\n\n\ttable.functionIndex td{\n\n\t\n\n\tpadding: 0px 30px 0px 5px;\n\n\t}\n\n\t/* lists */\n\n\tul {\n\n\t\n\n\tlist-style-type: none;\n\n\t}\n\n\t/* box highlighting */\n\n\t.boxhl {\n\n\t\n\n\tborder: 1px dotted black;\n\n\t\n\n\tpadding: 8px;\n\n\t\n\n\tcolor: #000000;\n\n\t\n\n\tbackground-color: #eeeeff;\n\n\t}\n\n\t/* box hl for calling conventions */\n\n\t.code {\n\n\t\n\n\tborder: 1px solid black;\n\n\t\n\n\tpadding: 20px;\n\n\t\n\n\tfont-family: monospace;\n\n\t\n\n\tcolor: #770000;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* code box */\n\n\tpre {\n\n\t\n\n\tcolor: #770000;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* code syntax */\n\n\tcode.s {\n\n\t\n\n\tmargin-left: 40px;\n\n\t\n\n\tfont-weight: normal;\n\n\t\n\n\tcolor: #770000;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* code syntax in descriptive text */\n\n\tcode {\n\n\t\n\n\tcolor: #000000;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* optional parameters */\n\n\tspan.op {\n\n\t\n\n\tfont-style: italic;\n\n\t\n\n\tcolor: #666666;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* message box examples */\n\n\tpre.msgbox {\n\n\t\n\n\tcolor: #777700;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t\n\n\tfont-family: serif;\n\n\t}\n\n\t/* indentation and text alignment */\n\n\t.ind {\n\n\t\n\n\tmargin-left: 40px;\n\n\t}\n\n\t.c {\n\n\t\n\n\ttext-align: center;\n\n\t}\n\n\t.l {\n\n\t\n\n\ttext-align: left;\n\n\t}\n\n\t/* links */\n\n\t.f {\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:link.f{\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:visited.f {\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:focus.f {\n\n\t\n\n\ttext-decoration: underline;\n\n\t\n\n\tcolor: #0000ff;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:hover.f {\n\n\t\n\n\ttext-decoration: underline;\n\n\t\n\n\tcolor: #0000ff;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:active.f {\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #ff0000;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:link.cf{\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:visited.cf {\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:focus.cf {\n\n\t\n\n\ttext-decoration: underline;\n\n\t\n\n\tcolor: #0000ff;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:hover.cf {\n\n\t\n\n\ttext-decoration: underline;\n\n\t\n\n\tcolor: #0000ff;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:active.cf {\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #ff0000;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:link img {\n\n\t\n\n\tborder-style: none;\n\n\t}\n\n\ta:visited img {\n\n\t\n\n\tborder-style: none;\n\n\t}\n\n\t</style>\n\n\n\n\t<title>CSE Construction Set Automation Script Command Documentation</title>\n\n\n\n\t<meta name=\"keywords\" content=\"CSE, CSAS, construction, set, extender, automation, script, oblivion, mod, modding\" />\n\n\n\n</head>\n<body>\n\n";
		const char* s_Footer = "<p><a href=\"http://validator.w3.org/check?uri=referer\"><img src=\"http://www.w3.org/Icons/valid-xhtml10\" alt=\"Valid XHTML 1.0 Strict\" height=\"31\" width=\"88\" /></a></p></body>";

		std::fstream DocStream("csas_command_doc.html", std::ios::out);
		if (!DocStream.fail())
		{
			AppendToStream(DocStream, s_Header);
			DumpProlog(DocStream);
			AppendToStream(DocStream, "\n\n\n\n");
			DumpTOC(DocStream);
			AppendToStream(DocStream, "\n\n\n\n");
			DumpFunctionSyntaxInfo(DocStream);
			AppendToStream(DocStream, "\n\n\n\n<br/><br/><br/>");

			CommandInfoList::iterator MainItr, SubItr;

			for (MainItr = CommandList.begin(); MainItr != CommandList.end(); MainItr = SubItr)
			{
				const std::string& Category = MainItr->first;
				std::string SanitizedName(SanitizeLinkableString(Category.c_str()));
				AppendToStream(DocStream, "<h3><a id=\"%s\">%s</a></h3>\n\n", SanitizedName.c_str(), Category.c_str());

				std::pair<CommandInfoList::iterator, CommandInfoList::iterator> KeyRange = CommandList.equal_range(Category);
				for (SubItr = KeyRange.first; SubItr != KeyRange.second; SubItr++)
				{
					CSASCommandInfo* Command = SubItr->second;
					DumpCommandDefinition(DocStream, Command);
					AppendToStream(DocStream, "\n\n");
				}
			}

			AppendToStream(DocStream, "<h2>Parameter Details / Type Codes</h2>\n\n");
			DumpParamInfoDetails(DocStream, "Format String", "<p>Refer to <a href=\"http://msdn.microsoft.com/en-us/library/56e442dc(v=VS.100).aspx\">this MSDN article</a> for format specification info.");
			DumpParamInfoDetails(DocStream, "Variable Value Type", "<ul><li>0: Invalid</li><li>1: Numeric</li><li>2: Reference</li><li>3: String</li><li>4: Array</li></ul>");
			DumpParamInfoDetails(DocStream, "Variable Value Type", "<ul><li>0: Invalid</li><li>1: Numeric</li><li>2: Reference</li><li>3: String</li><li>4: Array</li></ul>");
			DumpParamInfoDetails(DocStream, "Attribute ID", "<ul><li>0: Strength</li><li>1: Intelligence</li><li>2: Willpower</li><li>3: Agility</li><li>4: Speed</li><li>5: Endurance</li><li>6: Personality</li><li>7: Luck</li></ul>");
// 			DumpParamInfoDetails("AI Stat ID");
// 			DumpParamInfoDetails("Skill ID");
// 			DumpParamInfoDetails("Biped Model Slot Mask");
// 			DumpParamInfoDetails("Harvest Season ID");

			AppendToStream(DocStream, "\n\n\n\n");
			AppendToStream(DocStream, s_Footer);
			DocStream.close();
		}

		ShellExecute(NULL, "open", (LPSTR)"csas_command_doc.html", NULL, NULL, SW_SHOW);
	}

	void CommandTable::RegisterWithParser(mup::ParserX* Parser)
	{
		for (CommandInfoList::iterator Itr = CommandList.begin(); Itr != CommandList.end(); Itr++)
		{
			CSASCommandInfo* Command = Itr->second;
			if (!Command->Handler)
				continue;

			Parser->DefineFun(new CSASCommand(Command, false));		// register with name

			if (Command->Alias)
				Parser->DefineFun(new CSASCommand(Command, true));	// register with alias
		}

		// register special commands/operators
		Parser->DefineFun(new ReturnCommand());
		Parser->DefineFun(new CallCommand());
		Parser->DefineFun(new BreakCommand());
		Parser->DefineFun(new ContinueCommand());
		Parser->DefineFun(new GetSecondsPassedCommand());

		Parser->DefineInfixOprt(new OprtCastToRef());
		Parser->DefineInfixOprt(new OprtCastToStr("(str)"));
		Parser->DefineInfixOprt(new OprtCastToStr("$"));
		Parser->DefineOprt(new OprtBAndSymb());
		Parser->DefineOprt(new OprtBOrSymb());
	}

	void CommandTable::InitializeCommandTable()
	{
		REGISTER_CSASCOMMAND(Return, "General Functions");
		REGISTER_CSASCOMMAND(Call, "General Functions");
		REGISTER_CSASCOMMAND(Break, "General Functions");
		REGISTER_CSASCOMMAND(Continue, "General Functions");
		REGISTER_CSASCOMMAND(GetSecondsPassed, "General Functions");

		RegisterGeneralCommands();
		RegisterFormCommands();
		RegisterReferenceCommands();
		RegisterStringCommands();
	}

	void CSASCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		CSASDataElement Result;
		CSASDataElement* Arguments = NULL;

		try
		{
			if (a_iArgc)
			{
				Arguments = new CSASDataElement[a_iArgc]();
				for (int i = 0; i < a_iArgc; i++)
				{
					CSASParamInfo* CurrentParam = &CommandData->Parameters[i];

					switch (CurrentParam->ParamType)
					{
					case CSASDataElement::kParamType_Numeric:
						if (a_pArg[i]->IsScalar() == false)
							throw std::exception(PrintToBuffer("Non-scalar argument %d passed to command %s", i, CommandData->Name));

						Arguments[i].SetNumber(a_pArg[i]->GetFloat());
						break;
					case CSASDataElement::kParamType_String:
						if (a_pArg[i]->IsString() == false)
							throw std::exception(PrintToBuffer("Non-string argument %d passed to command %s", i, CommandData->Name));

						Arguments[i].SetString(a_pArg[i]->GetString().c_str());
						break;
					case CSASDataElement::kParamType_Reference:
						if (a_pArg[i]->IsScalar() == false)
							throw std::exception(PrintToBuffer("Non-reference type argument %d passed to command %s", i, CommandData->Name));

						Arguments[i].SetForm(TESForm::LookupByFormID((UInt32)((int)a_pArg[i]->GetFloat())));
						break;
					}
				}
			}

			if (CommandData->Handler(Arguments, &Result, CommandData->Parameters, CommandData->NoOfParams))
			{
				switch (CommandData->ReturnType)
				{
				case CSASDataElement::kParamType_Numeric:
					*ret = Result.GetNumber();
					break;
				case CSASDataElement::kParamType_String:
					*ret = std::string(Result.GetString());
					break;
				case CSASDataElement::kParamType_Reference:
					*ret = (int)Result.GetForm()->formID;
					break;
				case CSASDataElement::kParamType_Array:
					Result.GetArray()->ConvertToMUPArray(*ret);
					break;
				default:
					*ret = 0;
					break;
				}
			}
			else
				*ret = 0;
		}
		catch (...)													// catch any exceptions thrown by the command handler and rethrow
		{
			if (Arguments)
				delete [] Arguments;

			throw;
		}

		if (Arguments)
			delete [] Arguments;
	}

	const mup::char_type* CSASCommand::GetDesc() const
	{
		return CommandData->ShortDescription;
	}

	mup::IToken* CSASCommand::Clone() const
	{
		return new CSASCommand(*this);
	}

	bool CSASCommand::ExtractCommandArgs(CSASDataElement* ArgArray, CSASParamInfo* Parameters, UInt32 NoOfParams, ...)
	{
		bool Result = true;

		va_list Args;
		va_start(Args, NoOfParams);

		for (int i = 0; i < NoOfParams; i++)
		{
			CSASParamInfo* CurrentParam = &Parameters[i];
			CSASDataElement* CurrentArg = &ArgArray[i];

			assert(CurrentArg->GetType() == CurrentParam->ParamType);

			switch (CurrentParam->ParamType)
			{
			case CSASDataElement::kParamType_Numeric:
				{
					double* Out = va_arg(Args, double*);
					*Out = CurrentArg->GetNumber();
					break;
				}
			case CSASDataElement::kParamType_String:
				{
					char* Out = va_arg(Args, char*);
					sprintf_s(Out, strlen(CurrentArg->GetString()) + 1, "%s", CurrentArg->GetString());
					break;
				}
			case CSASDataElement::kParamType_Reference:
				{
					TESForm** Out = va_arg(Args, TESForm**);
					*Out = CurrentArg->GetForm();
					break;
				}
			default:
				assert(0);		// Unexpected param type
				break;
			}
		}

		va_end(Args);

		return Result;
	}

	void ReturnCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		ScriptContext* ExecutingScript = SCRIPTRUNNER->GetExecutingContext();
		assert(ExecutingScript && ExecutingScript->GetExecutionState() == ScriptContext::kExecutionState_Default);

		if (ExecutingScript->GetIsLoopExecuting())
			throw std::exception("Return command called inside a loop context");

		if (a_iArgc == 1)
		{
			if (a_pArg[0]->IsArray())
				throw std::exception("Array passed to Return command");

			mup::IValue* ArgVal = &(*a_pArg[0]);

			switch (ArgVal->GetCode())
			{
			case mup::cmVAR:
				ExecutingScript->SetReturnValue(*((dynamic_cast<mup::Variable*>(ArgVal))->GetPtr()));
				break;
			case mup::cmVAL:
				ExecutingScript->SetReturnValue(*dynamic_cast<mup::Value*>(ArgVal));
				break;
			}
		}
		else if (a_iArgc)
			throw std::exception("Too many arguments passed to Return command");

		ExecutingScript->SetExecutionState(ScriptContext::kExecutionState_Break);
		*ret = 0;
	}

	const mup::char_type* ReturnCommand::GetDesc() const
	{
		return "Stops the execution of the calling script and optionally returns a value.";
	}

	mup::IToken* ReturnCommand::Clone() const
	{
		return new ReturnCommand(*this);
	}

	void CallCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		int ArgCount = a_iArgc;

		if (ArgCount < 1)
			throw std::exception("Too few arguments passed to Call command");
		else if (ArgCount > 1 + MAX_BEGIN_BLOCK_PARAMS)		// arg1 = script name
			throw std::exception("Too many arguments passed to Call command");

		std::string ScriptName = a_pArg[0]->GetString();

		ParameterList Parameters;
		mup::Value ReturnValueBuffer;
		bool HasReturnedValue = false;

		for (int i = 1; i < ArgCount; i++)
		{
			mup::IValue* ArgVal = &(*a_pArg[i]);
			mup::Value Param;

			switch (ArgVal->GetCode())
			{
			case mup::cmVAR:
				Param = *((dynamic_cast<mup::Variable*>(ArgVal))->GetPtr());
				break;
			case mup::cmVAL:
				Param = *dynamic_cast<mup::Value*>(ArgVal);
				break;
			}

			Parameters.push_back(Param);
		}

		bool Result = SCRIPTRUNNER->RunScript(ScriptName, (Parameters.size() ? &Parameters : NULL), ReturnValueBuffer, &HasReturnedValue);

		if (HasReturnedValue)
			*ret = ReturnValueBuffer;
		else
			*ret = 0;
	}

	const mup::char_type* CallCommand::GetDesc() const
	{
		return "Invokes a function call.";
	}

	mup::IToken* CallCommand::Clone() const
	{
		return new CallCommand(*this);
	}

	void OprtCastToRef::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int /*a_iArgc*/)
	{
		switch(a_pArg[0]->GetType())
		{
		case 'f':
		case 'i':
		case 'b':
			*ret = (mup::int_type)a_pArg[0]->GetFloat();
			break;
		default:
			{
				mup::ErrorContext err;
				err.Errc = mup::ecINVALID_TYPECAST;
				err.Type1 = a_pArg[0]->GetType();
				err.Type2 = 'i';
				throw mup::ParserError(err);
			}
		}
	}

	const mup::char_type* OprtCastToRef::GetDesc() const
	{
		return _T("Cast a value into a reference pointer");
	}

	mup::IToken* OprtCastToRef::Clone() const
	{
		return new OprtCastToRef(*this);
	}

	void OprtCastToStr::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int /*a_iArgc*/)
	{
		char Buffer[0x32] = {0};

		switch(a_pArg[0]->GetType())
		{
		case 'f':
		case 'i':
		case 'b':
			sprintf_s(Buffer, sizeof(Buffer), "%0.0f", a_pArg[0]->GetFloat());
			*ret = std::string(Buffer);
			break;
		case 's':
			*ret = a_pArg[0]->GetString();
			break;
		default:
			{
				mup::ErrorContext err;
				err.Errc = mup::ecINVALID_TYPECAST;
				err.Type1 = a_pArg[0]->GetType();
				err.Type2 = 's';
				throw mup::ParserError(err);
			}
		}
	}

	const mup::char_type* OprtCastToStr::GetDesc() const
	{
		return _T("Cast a value into a string");
	}

	mup::IToken* OprtCastToStr::Clone() const
	{
		return new OprtCastToStr(*this);
	}

	void BreakCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		ScriptContext* ExecutingScript = SCRIPTRUNNER->GetExecutingContext();
		assert(ExecutingScript);

		ExecutingScript->SetExecutingLoopState(LoopBlock::kState_Break);
		ExecutingScript->SetExecutionState(ScriptContext::kExecutionState_Break);

		*ret = 0;
	}

	const mup::char_type* BreakCommand::GetDesc() const
	{
		return "Breaks the execution of the currently executing loop.";
	}

	mup::IToken* BreakCommand::Clone() const
	{
		return new BreakCommand(*this);
	}

	void ContinueCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		ScriptContext* ExecutingScript = SCRIPTRUNNER->GetExecutingContext();
		assert(ExecutingScript);

		ExecutingScript->SetExecutingLoopState(LoopBlock::kState_Continue);
		ExecutingScript->SetExecutionState(ScriptContext::kExecutionState_Break);

		*ret = 0;
	}

	const mup::char_type* ContinueCommand::GetDesc() const
	{
		return "Skips to the end of the currently executing loop.";
	}

	mup::IToken* ContinueCommand::Clone() const
	{
		return new ContinueCommand(*this);
	}

	void GetSecondsPassedCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		if (!GLOBALSCRIPTMANAGER->GetInExecutionLoop())
			throw std::exception("GetSecondsPassed command called in a non-global script");

		*ret = (float)GLOBALSCRIPTMANAGER->GetSecondsPassed();
	}

	const mup::char_type* GetSecondsPassedCommand::GetDesc() const
	{
		return "Returns the amount of time passed since the last execution of the calling script. Only applicable to global scripts.";
	}

	mup::IToken* GetSecondsPassedCommand::Clone() const
	{
		return new GetSecondsPassedCommand(*this);
	}

	CSASParamInfo kParams_OneForm[1] =
	{
		{ "Form", CSASDataElement::kParamType_Reference }
	};
	CSASParamInfo kParams_OneNumber[1] =
	{
		{ "Value", CSASDataElement::kParamType_Numeric }
	};
	CSASParamInfo kParams_OneString[1] =
	{
		{ "Value", CSASDataElement::kParamType_String }
	};
	CSASParamInfo kParams_FormNumber[2] =
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "Value", CSASDataElement::kParamType_Numeric }
	};
	CSASParamInfo kParams_FormString[2] =
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "Value", CSASDataElement::kParamType_String }
	};
	CSASParamInfo kParams_TwoForms[2] =
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "Value", CSASDataElement::kParamType_Reference }
	};
}