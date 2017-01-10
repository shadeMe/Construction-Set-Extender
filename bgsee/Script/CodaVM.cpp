#include "CodaVM.h"
#include "CodaUtilities.h"
#include "..\UIManager.h"
#include "..\BGSEditorExtenderBase_Resource.h"

#include "MUP Base\mpIToken.h"
#include "MUP Implementation\CodaMUPArrayDataType.h"
#include "MUP Implementation\CodaMUPExpressionParser.h"
#include "MUP Implementation\CodaMUPValue.h"

#include "Commands\CodaScriptCommands-General.h"
#include "Commands\CodaScriptCommands-String.h"
#include "Commands\CodaScriptCommands-Array.h"

namespace bgsee
{
	namespace script
	{
		void CodaScriptCommandRegistry::AppendToStream(std::fstream& Out, const char* Fmt, ...)
		{
			va_list Args;
			char Buffer[0x1000] = {0};

			va_start(Args, Fmt);
			vsprintf_s(Buffer, sizeof(Buffer), Fmt, Args);
			va_end(Args);

			Out << Buffer;
		}

		std::string CodaScriptCommandRegistry::SanitizeLinkableString( const char* String )
		{
			std::string SanitizedString(String);

			SME::StringHelpers::MakeLower(SanitizedString);
			std::replace(SanitizedString.begin(), SanitizedString.end(), ' ', '_');

			return SanitizedString;
		}

		const char* CodaScriptCommandRegistry::StringifyParameterType( UInt8 ParamType )
		{
			switch (ParamType)
			{
			case ICodaScriptDataStore::kDataType_Invalid:
				return "nothing";
			case ICodaScriptDataStore::kDataType_Array:
				return "array";
			case ICodaScriptDataStore::kDataType_Numeric:
				return "numeric";
			case ICodaScriptDataStore::kDataType_Reference:
				return "ref";
			case ICodaScriptDataStore::kDataType_String:
				return "string";
			case ICodaScriptCommand::ParameterInfo::kType_Multi:
				return "multi";
			default:
				return "<unknown>";
			}
		}

		void CodaScriptCommandRegistry::InitializeExpressionParser( ICodaScriptExpressionParser* Parser )
		{
			for (CommandTableMapT::iterator Itr = Registry.begin(); Itr != Registry.end(); Itr++)
			{
				ICodaScriptCommand* Command = Itr->second;
				Parser->RegisterCommand(Command);
			}

			// register constants
			Parser->RegisterConstant(commands::general::kConstant_ScriptSelf.c_str(),
									 CodaScriptBackingStore(commands::general::kConstant_ScriptSelf.c_str()));
		}

		CodaScriptCommandRegistry::CodaScriptCommandRegistry(const char* WikiURL) :
			Registry(),
			BaseWikiURL(WikiURL)
		{
			;//
		}

		CodaScriptCommandRegistry::~CodaScriptCommandRegistry()
		{
			Registry.clear();
		}

		ICodaScriptCommand* CodaScriptCommandRegistry::LookupCommand( const char* Name, bool UseAlias /*= false*/ )
		{
			if (Name)
			{
				for (CommandTableMapT::iterator Itr = Registry.begin(); Itr != Registry.end(); Itr++)
				{
					if ((UseAlias == false && Itr->second->GetName() && !_stricmp(Name, Itr->second->GetName())) ||
						(UseAlias && Itr->second->GetAlias() && !_stricmp(Name, Itr->second->GetAlias())))
					{
						return Itr->second;
					}
				}
			}

			return nullptr;
		}

		bool CodaScriptCommandRegistry::RegisterCommand( const char* Category, ICodaScriptCommand* Command )
		{
			SME_ASSERT(Command->GetName());

			if (Category == nullptr)
				Category = "Uncategorized";

			std::string Buffer = Category;
			Buffer += " Functions";

			if (LookupCommand(Command->GetName()) || LookupCommand(Command->GetAlias(), true))
				return false;

			Registry.insert(std::make_pair(Buffer, Command));
			return true;
		}

		void CodaScriptCommandRegistry::RegisterCommands( const CodaScriptCommandRegistrar::ListT& Registrars )
		{
			for (auto& Itr : Registrars)
			{
				for (auto& Command : Itr->GetCommands())
				{
					RegisterCommand(Itr->GetCategory(), Command);
				}
			}
		}

		void CodaScriptCommandRegistry::Dump( std::string OutPath )
		{
			const char* Header = "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n\n<head>\n\n\t<meta content=\"text/html; charset=UTF-8\" http-equiv=\"content-type\" />\n\n\t<style type=\"text/css\">\n\n\t/* body */ \n\n\tbody {\n\n\t\n\n\tpadding: 10%%;\n\n\t}\n\n\t/* headings */\n\n\th1 {\n\n\t\n\n\tfont-size: 150%%;\n\n\t\n\n\tfont-weight: normal;\n\n\t}\n\n\th2 {\n\n\t\n\n\tfont-size: 130%%;\n\n\t\n\n\tfont-weight: normal;\n\n\t\n\n\tmargin-top: 40px;\n\n\t}\n\n\th3 {\n\n\t\n\n\tfont-size: 110%%;\n\n\t\n\n\tfont-weight: normal;\n\n\t\n\n\tmargin-top: 30px;\n\n\t\n\n\tcolor: #000099;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\th4 {\n\n\t\n\n\tfont-size: 100%%;\n\n\t\n\n\tfont-weight: bold;\n\n\t\n\n\tmargin-bottom: 0px;\n\n\t}\n\n\th5 {\n\n\t\n\n\tfont-size: 100%%;\n\n\t\n\n\tfont-weight: bold;\n\n\t\n\n\tmargin-bottom: 0px;\n\n\t\n\n\tcolor: #000099;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* tables */\n\n\ttable {\n\n\t\n\n\tborder: 1px solid black;\n\n\t\n\n\tmargin: 20px 0px 20px 0px;\n\n\t}\n\n\tcaption {\n\n\t\n\n\ttext-align: left;\n\n\t\n\n\tmargin-bottom: 10px;\n\n\t}\n\n\ttd, th {\n\n\t\n\n\tpadding: 5px;\n\n\t}\n\n\ttr.alt {\n\n\t\n\n\tcolor: black;\n\n\t\n\n\tbackground-color: #dedede;\n\n\t}\n\n\tcode.alt {\n\n\t\n\n\tcolor: 770000;\n\n\t\n\n\tbackground-color: #dedede;\n\n\t}\n\n\ttable.functionIndex td{\n\n\t\n\n\tpadding: 0px 30px 0px 5px;\n\n\t}\n\n\t/* lists */\n\n\tul {\n\n\t\n\n\tlist-style-type: none;\n\n\t}\n\n\t/* box highlighting */\n\n\t.boxhl {\n\n\t\n\n\tborder: 1px dotted black;\n\n\t\n\n\tpadding: 8px;\n\n\t\n\n\tcolor: #000000;\n\n\t\n\n\tbackground-color: #eeeeff;\n\n\t}\n\n\t/* box hl for calling conventions */\n\n\t.code {\n\n\t\n\n\tborder: 1px solid black;\n\n\t\n\n\tpadding: 20px;\n\n\t\n\n\tfont-family: monospace;\n\n\t\n\n\tcolor: #770000;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* code box */\n\n\tpre {\n\n\t\n\n\tcolor: #770000;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* code syntax */\n\n\tcode.s {\n\n\t\n\n\tmargin-left: 40px;\n\n\t\n\n\tfont-weight: normal;\n\n\t\n\n\tcolor: #770000;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* code syntax in descriptive text */\n\n\tcode {\n\n\t\n\n\tcolor: #000000;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* optional parameters */\n\n\tspan.op {\n\n\t\n\n\tfont-style: italic;\n\n\t\n\n\tcolor: #666666;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t}\n\n\t/* message box examples */\n\n\tpre.msgbox {\n\n\t\n\n\tcolor: #777700;\n\n\t\n\n\tbackground-color: #ffffff;\n\n\t\n\n\tfont-family: serif;\n\n\t}\n\n\t/* indentation and text alignment */\n\n\t.ind {\n\n\t\n\n\tmargin-left: 40px;\n\n\t}\n\n\t.c {\n\n\t\n\n\ttext-align: center;\n\n\t}\n\n\t.l {\n\n\t\n\n\ttext-align: left;\n\n\t}\n\n\t/* links */\n\n\t.f {\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:link.f{\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:visited.f {\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:focus.f {\n\n\t\n\n\ttext-decoration: underline;\n\n\t\n\n\tcolor: #0000ff;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:hover.f {\n\n\t\n\n\ttext-decoration: underline;\n\n\t\n\n\tcolor: #0000ff;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:active.f {\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #ff0000;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:link.cf{\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:visited.cf {\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #3333cc;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:focus.cf {\n\n\t\n\n\ttext-decoration: underline;\n\n\t\n\n\tcolor: #0000ff;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:hover.cf {\n\n\t\n\n\ttext-decoration: underline;\n\n\t\n\n\tcolor: #0000ff;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:active.cf {\n\n\t\n\n\ttext-decoration: none;\n\n\t\n\n\tcolor: #ff0000;\n\n\t\n\n\tbackground-color: white;\n\n\t}\n\n\ta:link img {\n\n\t\n\n\tborder-style: none;\n\n\t}\n\n\ta:visited img {\n\n\t\n\n\tborder-style: none;\n\n\t}\n\n\t</style>\n\n\n\n\t<title>Coda Script Command Documentation</title>\n\n\n\n\t<meta name=\"keywords\" content=\"BGSEE, BGSEditorExtender, bgs, bethesda, editor, Coda, Script, CodaScript, construction, set, creation, kit, extender, tes, fallout, oblivion, skyrim, mod, modding, plugin, script extender, OBSE, SKSE\" />\n\n\n\n</head>\n<body>\n\n";

			std::fstream DocStream("coda_command_doc.html", std::ios::out);

			if (DocStream.fail() == false)
			{
				AppendToStream(DocStream, Header);

				AppendToStream(DocStream,
							"<h1>%s Coda Command Documentation</h1><p><a href=\"%sCategory:%s\">%s</a> - Follow the links throughout this documentation to gain access to additional information on the official Wiki. Users are encouraged to add to the wiki.<br />",
							BGSEEMAIN->ExtenderGetShortName(),
							BaseWikiURL.c_str(),
							BGSEEMAIN->ExtenderGetLongName(),
							BGSEEMAIN->ExtenderGetLongName());
				AppendToStream(DocStream, "\n\n\n\n");

				CommandTableMapT::iterator MainItr, SubItr;

				AppendToStream(DocStream, "<h2>Table of Contents</h2>\n\n");
				AppendToStream(DocStream, "<ul>");
				for (MainItr = Registry.begin(); MainItr != Registry.end(); MainItr = SubItr)
				{
					const std::string& Category = MainItr->first;
					std::string SanitizedName = SanitizeLinkableString(Category.c_str());

					AppendToStream(DocStream, "<li><a href=\"#%s\">%s</a></li>\n", SanitizedName.c_str(), Category.c_str());

					std::pair<CommandTableMapT::iterator, CommandTableMapT::iterator> KeyRange = Registry.equal_range(Category);
					for (SubItr = KeyRange.first; SubItr != KeyRange.second; SubItr++)
						;//
				}
				AppendToStream(DocStream, "</ul>");

				AppendToStream(DocStream, "\n\n\n\n");
				AppendToStream(DocStream, "<h2><a id=\"Function_Syntax_Format\">Function Syntax Format</a></h2><pre class=\"code\">(returnValueType)    FunctionName    parameter1:type  parameter2:type</pre><h2><a id=\"Function_CallingSyntax_Format\">Function Calling Syntax Format</a></h2><pre class=\"code\">FunctionName(parameter1, parameter2)</pre><table><caption>Parameter Types: designated after colon</caption><tr><th>numeric</th><td>a positive or negative decimal number or integer</td><td>3.1415/-56</td></tr><tr class=\"alt\"><th>ref</th><td>a formID literal or ref returned by <a href=\"#GetFormByEditorID\">GetFormByEditorID</a>/<a href=\"#GetFormByFormID\">GetFormByFormID</a></td><td>0x00A12EF2/GetFormByEditorID(\"apple\")</td></tr><tr><th>string</th><td>a set of characters in quotes</td><td>\"Uriel Septim\"</td></tr><tr class=\"alt\"><th>array</th><td>an array of elements that can be used in ForEach expressions, etc</td><td>--</td></tr><tr><th>multi</th><td>a value of any type numeric, string, ref or array</td><td>--</td></tr></table>");
				AppendToStream(DocStream, "\n\n\n\n<br/><br/><br/>");

				for (MainItr = SubItr = Registry.begin(); MainItr != Registry.end(); MainItr = SubItr)
				{
					const std::string& Category = MainItr->first;
					std::string SanitizedName = SanitizeLinkableString(Category.c_str());

					AppendToStream(DocStream, "<h3><a id=\"%s\">%s</a></h3>\n\n", SanitizedName.c_str(), Category.c_str());

					std::pair<CommandTableMapT::iterator, CommandTableMapT::iterator> KeyRange = Registry.equal_range(Category);
					for (SubItr = KeyRange.first; SubItr != KeyRange.second; SubItr++)
					{
						ICodaScriptCommand* Command = SubItr->second;

						const char* Name = Command->GetName();
						const char*	Alias = Command->GetAlias();

						int ParameterCount = 0;
						ICodaScriptCommand::ParameterInfo* ParameterData = nullptr;
						UInt8 ResultType = ICodaScriptDataStore::kDataType_Invalid;
						Command->GetParameterData(&ParameterCount, &ParameterData, &ResultType);

						AppendToStream(DocStream,
							"<p><a id=\"%s\" class=\"f\" href=\"%s%s_(Coda)\">%s</a>",
							Name,
							BaseWikiURL.c_str(),
							Name,
							Name);

						if (Alias)
						{
							AppendToStream(DocStream, " (<i>%s</i>)", Alias);
						}

						AppendToStream(DocStream, " - %s<br />\n", Command->GetDescription());
						AppendToStream(DocStream, "<code class=\"s\">(%s) ", StringifyParameterType(ResultType));
						AppendToStream(DocStream, Name);

						for (int i = 0; i < ParameterCount; i++)
						{
							ICodaScriptCommand::ParameterInfo* Current = &ParameterData[i];
							std::string Name = SanitizeLinkableString(Current->Name);

							AppendToStream(DocStream, " %s:%s",
								Name.c_str(),
								StringifyParameterType(Current->Type));
						}

						AppendToStream(DocStream, "</code></p>\n\n");

						const char* Documentation = Command->GetDocumentation();

						if (Documentation)
						{
							AppendToStream(DocStream, "\n\n<p>%s</p>", Documentation);
						}

						AppendToStream(DocStream, "\n\n");
					}
				}

				DocStream.close();
			}

			ShellExecute(nullptr, "open", (LPSTR)"coda_command_doc.html", nullptr, nullptr, SW_SHOW);
		}

		ICodaScriptProgram* CodaScriptProgramCache::Lookup(const std::string& Filepath) const
		{
			if (Store.count(Filepath) == 0)
				return nullptr;
			else
				return Store.at(Filepath).get();
		}

		void CodaScriptProgramCache::Remove(const std::string& Filepath)
		{
			Store.erase(Filepath);
		}

		void CodaScriptProgramCache::Add(const std::string& Filepath, ICodaScriptProgram::PtrT& Program)
		{
			SME_ASSERT(Store.count(Filepath) == 0);

			Store.insert(std::make_pair(Filepath, std::move(Program)));
		}

		CodaScriptProgramCache::CodaScriptProgramCache(ICodaScriptVirtualMachine* VM) :
			VM(VM),
			Store()
		{
			SME_ASSERT(VM);
		}

		CodaScriptProgramCache::~CodaScriptProgramCache()
		{
			for (auto& Itr : Store)
			{
				if (VM->IsProgramExecuting(Itr.second.get()))
					VM->GetMessageHandler()->Log("Coda script program '%s' is still executing during disposal", Itr.second->GetName().c_str());
			}
		}

		ICodaScriptProgram* CodaScriptProgramCache::Get(const ResourceLocation& Filepath, bool Recompile /*= false*/)
		{
			ICodaScriptProgram* OutProgram(Lookup(Filepath()));

			if (Recompile && OutProgram)
			{
				if (VM->IsProgramExecuting(OutProgram))
					VM->GetMessageHandler()->Log("Ignored request to recompile executing Coda script program @ %s", Filepath().c_str());
				else
				{
					Remove(Filepath());
					OutProgram = nullptr;
				}
			}

			// recompile if invalid
			if (OutProgram && OutProgram->IsValid() == false)
			{
				SME_ASSERT(VM->IsProgramExecuting(OutProgram) == false);
				Remove(Filepath());
				OutProgram = nullptr;
			}

			if (OutProgram == nullptr)
			{
				// create the program context from disk
				ICodaScriptProgram::PtrT Program(CodaScriptCompiler::Instance.Compile(VM, Filepath));

				if (Program->IsValid() == false)
					VM->GetMessageHandler()->Log("Couldn't compile Coda script @ %s", Filepath().c_str());
				else
				{
					OutProgram = Program.get();
					Add(Filepath(), Program);
				}
			}

			return OutProgram;
		}

		void CodaScriptProgramCache::Invalidate()
		{
			for (auto& Itr : Store)
			{
				SME_ASSERT(CODAVM->IsProgramExecuting(Itr.second.get()) == false);
				Itr.second->InvalidateBytecode();
			}
		}


#define CODASCRIPTEXECUTIVE_INISECTION							"CodaExecutive"
		SME::INI::INISetting									CodaScriptExecutive::kINI_Profiling("Profiling", CODASCRIPTEXECUTIVE_INISECTION,
																									"Profile script execution",
																									(SInt32)0);

		SME::INI::INISetting									CodaScriptExecutive::kINI_RecursionLimit("RecursionLimit", CODASCRIPTEXECUTIVE_INISECTION,
																									"Maximum number of times scripts can recursively call themselves or other scripts. Large values may cause instability",
																									(SInt32)50);

		void CodaScriptExecutive::Push(ICodaScriptExecutionContext* Context)
		{
			ICodaScriptProgram* Program = Context->GetProgram();
			if (ExecutionCounter.count(Program))
				ExecutionCounter[Program]++;
			else
				ExecutionCounter[Program] = 1;

			ExecutingContexts.push(Context);
		}

		void CodaScriptExecutive::Pop(ICodaScriptExecutionContext* Context)
		{
			ICodaScriptProgram* Program = Context->GetProgram();
			SME_ASSERT(ExecutionCounter.count(Program));
			SME_ASSERT(ExecutingContexts.size() && ExecutingContexts.top() == Context);

			int Count = ExecutionCounter[Program]--;
			if (Count == 1)
				ExecutionCounter.erase(Program);
			else
				SME_ASSERT(Count);

			ExecutingContexts.pop();
		}

		CodaScriptExecutive::CodaScriptExecutive(ICodaScriptVirtualMachine* VM) :
			ExecutionCounter(),
			ExecutingContexts(),
			Profiler(),
			OwnerThreadID(0),
			VM(VM)
		{
			SME_ASSERT(VM);

			OwnerThreadID = GetCurrentThreadId();
		}

		CodaScriptExecutive::~CodaScriptExecutive()
		{
			SME_ASSERT(ExecutingContexts.empty());
			SME_ASSERT(ExecutionCounter.empty());
		}

		void CodaScriptExecutive::Execute(ICodaScriptExecutionContext* Context,
										  ICodaScriptVirtualMachine::ExecuteResult& Out)
		{
			SME_ASSERT(OwnerThreadID == GetCurrentThreadId());
			SME_ASSERT(Context);
			SME_ASSERT(Context->CanExecute());
			SME_ASSERT(Out.HasResult() == false && Out.Success == false);

			ICodaScriptProgram* Program = Context->GetProgram();
			SME_ASSERT(Program->IsValid());

			if (ExecutingContexts.size() > kINI_RecursionLimit().i)
			{
				VM->GetMessageHandler()->Log("Maximum script recursion depth hit");
				Out.Success = false;
				return;
			}

			bool ProfilerEnabled = kINI_Profiling().i;
			if (ProfilerEnabled)
				Profiler.BeginProfiling();

			Push(Context);
			{
				ICodaScriptExpressionParser::EvaluateData EvaluatorInput(Context, VM->GetGlobals());
				try
				{
					CodaScriptSyntaxTreeExecuteVisitor Visitor(VM, Context);
					VM->GetParser()->BeginEvaluation(Program, EvaluatorInput);
					Program->Accept(&Visitor);		// doesn't throw any exceptions, so the next statement will always be executed
					VM->GetParser()->EndEvaluation(Program);

					Out.Success = Context->HasError() == false;
					if (Context->HasResult())
						Out.Result = std::move(ICodaScriptDataStore::PtrT(new CodaScriptBackingStore(Context->GetResult())));
				}
				catch (CodaScriptException& E)
				{
					VM->GetMessageHandler()->Log("Runtime Error [Script: %s] - %s", Program->GetName().c_str(), E.Get());
				}
				catch (...)
				{
					VM->GetMessageHandler()->Log("Unknown Runtime Error!");
				}
			}
			Pop(Context);

			if (ProfilerEnabled)
			{
				double ElapsedTime = Profiler.EndProfiling() * 1.0;
				VM->GetMessageHandler()->Log("Profiler: %s [%.4f ms]", Context->GetProgram()->GetName().c_str(), ElapsedTime);
			}
		}

		bool CodaScriptExecutive::IsBusy() const
		{
			return ExecutingContexts.empty() == false;
		}

		bool CodaScriptExecutive::IsProgramExecuting(ICodaScriptProgram* Program) const
		{
			return ExecutionCounter.count(Program);
		}

		void CodaScriptExecutive::RegisterINISettings( INISettingDepotT& Depot )
		{
			Depot.push_back(&kINI_Profiling);
			Depot.push_back(&kINI_RecursionLimit);
		}



#define CODASCRIPTBACKGROUNDER_INISECTION						"CodaBackgrounder"
		SME::INI::INISetting									CodaScriptBackgrounder::kINI_Enabled("Enabled", CODASCRIPTBACKGROUNDER_INISECTION,
																										"Execute background scripts",
																										(SInt32)1);
		SME::INI::INISetting									CodaScriptBackgrounder::kINI_UpdatePeriod("UpdatePeriod", CODASCRIPTBACKGROUNDER_INISECTION,
																									"Duration, in milliseconds, between consecutive executions",
																									(SInt32)10);
		SME::INI::INISetting									CodaScriptBackgrounder::kINI_LogToDefaultConsoleContext("LogToDefaultConsoleContext", CODASCRIPTBACKGROUNDER_INISECTION,
																									 "Print console output to the default context (in addition to the Coda Script context)",
																									 (SInt32)1);

		VOID CALLBACK CodaScriptBackgrounder::CallbackProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
		{
			CodaScriptBackgrounder* Daemon = (CodaScriptBackgrounder*)idEvent;
			Daemon->Tick();
		}

		void CodaScriptBackgrounder::ResetDepotCache(bool Renew /*= false*/ )
		{
			SME_ASSERT(Backgrounding == false);

			DepotCache.clear();

			if (Renew)
			{
				for (IDirectoryIterator Itr(SourceDepot().c_str(),
					(std::string("*" + VM->GetScriptFileExtension())).c_str()); !Itr.Done(); Itr.Next())
				{
					std::string FullPath = SourceDepot.GetRelativePath() + "\\" + std::string(Itr.Get()->cFileName);
					VM->GetMessageHandler()->Log("Script: %s", Itr.Get()->cFileName);
					VM->GetMessageHandler()->Indent();
					{
						ICodaScriptProgram* BackgroundScript = VM->GetProgramCache()->Get(FullPath, true);
						if (BackgroundScript)
						{
							VM->GetMessageHandler()->Log("Success: %s [%.4f s]", BackgroundScript->GetName().c_str(), BackgroundScript->GetPollingInteval());

							ICodaScriptExecutionContext::PtrT Context(new CodaScriptExecutionContext(VM, BackgroundScript));
							DepotCache.push_back(std::move(Context));
						}
					}
					VM->GetMessageHandler()->Outdent();
				}
			}
		}

		void CodaScriptBackgrounder::ResetTimer( bool Renew /*= false*/ )
		{
			SME_ASSERT(Backgrounding == false);

			if (TimerDummyWindow)
				KillTimer(TimerDummyWindow, (UINT_PTR)this);

			if (Renew)
			{
				SME_ASSERT(TimerDummyWindow);

				UInt32 UpdatePeriod = kINI_UpdatePeriod.GetData().i;
				UINT_PTR Result = SetTimer(TimerDummyWindow, (UINT_PTR)this, UpdatePeriod, &CallbackProc);
				SME_ASSERT(Result);
			}
		}

		void CodaScriptBackgrounder::Execute( ContextArrayT& Cache,  double TimePassed )
		{
			for (auto Itr = Cache.begin(); Itr != Cache.end();)
			{
				ICodaScriptExecutionContext* BackgroundScript = (*Itr).get();
				if (BackgroundScript->GetProgram()->IsValid() == false)
				{
					VM->GetMessageHandler()->Log("Background script '%s' halted - Invalid program", BackgroundScript->GetProgram()->GetName().c_str());
					Itr = Cache.erase(Itr);
					continue;
				}

				if (BackgroundScript->TickPollingInterval(TimePassed))
				{
					if (IsEnabled())
					{
						if (kINI_LogToDefaultConsoleContext().i == 0)
							VM->GetMessageHandler()->SuspendDefaultContextLogging();
						else
							VM->GetMessageHandler()->Indent();

						ICodaScriptVirtualMachine::ExecuteResult Result;
						VM->GetExecutor()->Execute(BackgroundScript, Result);

						if (kINI_LogToDefaultConsoleContext().i == 0)
							VM->GetMessageHandler()->ResumeDefaultContextLogging();
						else
							VM->GetMessageHandler()->Outdent();

						if (BackgroundScript->CanExecute() == false)
						{
							if (BackgroundScript->HasError())
								VM->GetMessageHandler()->Log("Background script '%s' halted - Unhandled exception", BackgroundScript->GetProgram()->GetName().c_str());

							Itr = Cache.erase(Itr);
							continue;
						}
					}
				}

				Itr++;
			}
		}

		void CodaScriptBackgrounder::Tick()
		{
			// prevent nested calls that might crop in the event of message pump congestions
			if (VM->GetExecutor()->IsBusy() || Backgrounding)
				return;

			SME::MiscGunk::ScopedSetter<bool> GuardBackgrounding(Backgrounding, true);

			PollingTimeCounter.Update();
			double TimePassed = PollingTimeCounter.GetTimePassed() / 1000.0f;

			Execute(DepotCache, TimePassed);
			Execute(RuntimeCache, TimePassed);
		}

		CodaScriptBackgrounder::CodaScriptBackgrounder(ICodaScriptVirtualMachine* VM,
													   ResourceLocation Source,
													   INIManagerGetterFunctor Getter,
													   INIManagerSetterFunctor Setter ) :
			SourceDepot(Source),
			DepotCache(),
			RuntimeCache(),
			State(false),
			Backgrounding(false),
			TimerDummyWindow(NULL),
			PollingTimeCounter(),
			VM(VM),
			INISettingGetter(Getter),
			INISettingSetter(Setter)
		{
			SME_ASSERT(VM);
			State = kINI_Enabled.GetData().i;

			const char* ClassName = "BACKGROUNDER_TIMER_WINDOW";
			WNDCLASSEX wx = {};
			wx.cbSize = sizeof(WNDCLASSEX);
			wx.lpfnWndProc = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hWnd, uMsg, wParam, lParam); };
			wx.hInstance = BGSEEMAIN->GetExtenderHandle();
			wx.lpszClassName = ClassName;
			SME_ASSERT(RegisterClassEx(&wx));

			TimerDummyWindow = CreateWindowEx(0, ClassName, ClassName, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
			ResetTimer(true);
		}

		CodaScriptBackgrounder::~CodaScriptBackgrounder()
		{
			SME_ASSERT(Backgrounding == false);

			ResetTimer();
			ResetDepotCache();
			RuntimeCache.clear();

			DestroyWindow(TimerDummyWindow);
			kINI_Enabled.SetInt(State);
		}

		void CodaScriptBackgrounder::RegisterINISettings( INISettingDepotT& Depot )
		{
			Depot.push_back(&kINI_Enabled);
			Depot.push_back(&kINI_UpdatePeriod);
			Depot.push_back(&kINI_LogToDefaultConsoleContext);
		}

		const ResourceLocation& CodaScriptBackgrounder::GetBackgroundScriptRepository() const
		{
			return SourceDepot;
		}

		void CodaScriptBackgrounder::Suspend()
		{
			SME_ASSERT(Backgrounding == false);
			State = false;
		}

		void CodaScriptBackgrounder::Resume()
		{
			SME_ASSERT(Backgrounding == false);
			State = true;
		}

		bool CodaScriptBackgrounder::IsEnabled() const
		{
			return State;
		}

		bool CodaScriptBackgrounder::IsBackgrounding() const
		{
			return Backgrounding;
		}

		void CodaScriptBackgrounder::Rebuild()
		{
			VM->GetMessageHandler()->Log("Coda Script Backgrounder:");
			VM->GetMessageHandler()->Indent();
			ResetTimer(true);
			ResetDepotCache(true);
			VM->GetMessageHandler()->Outdent();
		}

		void CodaScriptBackgrounder::Queue(ICodaScriptExecutionContext* Context)
		{
			SME_ASSERT(Backgrounding == false);
			SME_ASSERT(Context->CanExecute());

			ICodaScriptExecutionContext::PtrT NewScript(Context);
			RuntimeCache.push_back(std::move(NewScript));
		}

#define CODASCRIPTGLOBALDATASTORE_INISECTION					"CodaGlobalDataStore"

#define IDM_BGSEE_CODAGLOBALDATASTORE_CLEAREDITFIELD			(WM_USER + 5003)
#define IDM_BGSEE_CODAGLOBALDATASTORE_RELOADVARLIST				(WM_USER + 5004)

		BOOL CALLBACK CodaScriptGlobalDataStore::EditDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			CodaScriptGlobalDataStore* Instance = (CodaScriptGlobalDataStore*)GetWindowLongPtr(hWnd, GWL_USERDATA);

			HWND GlobaList = GetDlgItem(hWnd, IDC_BGSEE_CODAGLOBALDATASTORE_VARLIST);
			HWND NameBox =  GetDlgItem(hWnd, IDC_BGSEE_CODAGLOBALDATASTORE_NAME);
			HWND ValueBox =  GetDlgItem(hWnd, IDC_BGSEE_CODAGLOBALDATASTORE_VALUE);

			switch (uMsg)
			{
			case IDM_BGSEE_CODAGLOBALDATASTORE_CLEAREDITFIELD:
				SetWindowText(NameBox, nullptr);
				SetWindowText(ValueBox, nullptr);
				break;
			case IDM_BGSEE_CODAGLOBALDATASTORE_RELOADVARLIST:
				{
					SendMessage(GlobaList, LB_RESETCONTENT, NULL, NULL);

					char Buffer[0x100] = {0};
					for (auto Itr : Instance->Cache)
					{
						sprintf_s(Buffer, sizeof(Buffer), "[%c]\t\t%s",
								  Itr->GetStoreOwner()->GetDataStore()->GetType(),
								  Itr->GetName());

						int Index = SendMessage(GlobaList, LB_INSERTSTRING, -1, (LPARAM)Buffer);
						SendMessage(GlobaList, LB_SETITEMDATA, Index, (LPARAM)Itr);
					}

					SendMessage(GlobaList, LB_SETCURSEL, -1, NULL);
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_BGSEE_CLOSE:
					EndDialog(hWnd, 0);
					return TRUE;
				case IDC_BGSEE_APPLY:
					if (IsWindowEnabled(ValueBox) == TRUE)
					{
						char NameBuffer[0x200] = {0}, ValueBuffer[0x200] = {0};
						GetWindowText(NameBox, (LPSTR)NameBuffer, 0x200);
						GetWindowText(ValueBox, (LPSTR)ValueBuffer, 0x200);

						if ((strlen(NameBuffer) < 1 || strlen(ValueBuffer) < 1) ||
							(strstr(NameBuffer, "=") || strstr(ValueBuffer, "=")) ||
							(strstr(NameBuffer, "|") || strstr(ValueBuffer, "|")))
						{
							BGSEEUI->MsgBoxW(hWnd, NULL, "Invalid input. Make sure the strings are non-null and don't contain a '=' or a '|'.");
							break;
						}

						CodaScriptVariable* GlobalVar = nullptr;
						bool ExistingVar = false;

						if (IsDlgButtonChecked(hWnd, IDC_BGSEE_CODAGLOBALDATASTORE_ADDSTRING) == BST_CHECKED)
							GlobalVar = Instance->Add(NameBuffer, ValueBuffer, ExistingVar);
						else
							GlobalVar = Instance->Add(NameBuffer, atof(ValueBuffer), ExistingVar);

						SME_ASSERT(GlobalVar);

						if (ExistingVar == false)
						{
							sprintf_s(NameBuffer, sizeof(NameBuffer), "[%c]\t\t%s",
								GlobalVar->GetStoreOwner()->GetDataStore()->GetType(),
								GlobalVar->GetName());

							int Index = SendMessage(GlobaList, LB_INSERTSTRING, -1, (LPARAM)NameBuffer);
							SendMessage(GlobaList, LB_SETITEMDATA, Index, (LPARAM)GlobalVar);
						}

						SendMessage(hWnd, IDM_BGSEE_CODAGLOBALDATASTORE_RELOADVARLIST, NULL, NULL);
					}

					break;
				case IDC_BGSEE_REMOVE:
					{
						int Index = SendMessage(GlobaList, LB_GETCURSEL, NULL, NULL);
						if (Index != LB_ERR)
						{
							CodaScriptVariable* Global = (CodaScriptVariable*)SendMessage(GlobaList, LB_GETITEMDATA, Index, NULL);
							Instance->Remove(Global);

							SendMessage(GlobaList, LB_DELETESTRING, Index, NULL);
							SendMessage(hWnd, IDM_BGSEE_CODAGLOBALDATASTORE_CLEAREDITFIELD, NULL, NULL);
							SendMessage(GlobaList, LB_SETSEL, FALSE, -1);
						}

						SendMessage(hWnd, IDM_BGSEE_CODAGLOBALDATASTORE_RELOADVARLIST, NULL, NULL);
					}

					break;
				case IDC_BGSEE_CODAGLOBALDATASTORE_VARLIST:
					{
						switch (HIWORD(wParam))
						{
						case LBN_SELCHANGE:
							EnableWindow(ValueBox, TRUE);

							int Index = SendMessage(GlobaList, LB_GETCURSEL, NULL, NULL);
							if (Index != LB_ERR)
							{
								CodaScriptVariable* Global = (CodaScriptVariable*)SendMessage(GlobaList, LB_GETITEMDATA, Index, NULL);
								SetWindowText(NameBox, (LPSTR)Global->GetName());
								char Buffer[0x200] = {0};

								switch (Global->GetStoreOwner()->GetDataStore()->GetType())
								{
								case ICodaScriptDataStore::kDataType_Numeric:
									FORMAT_STR(Buffer, "%0.6f", Global->GetStoreOwner()->GetDataStore()->GetNumber() * 1.0);
									SetWindowText(ValueBox, (LPSTR)Buffer);
									CheckRadioButton(hWnd,
													IDC_BGSEE_CODAGLOBALDATASTORE_ADDNUMBER,
													IDC_BGSEE_CODAGLOBALDATASTORE_ADDSTRING,
													IDC_BGSEE_CODAGLOBALDATASTORE_ADDNUMBER);

									break;
								case ICodaScriptDataStore::kDataType_String:
									SetWindowText(ValueBox, (LPSTR)Global->GetStoreOwner()->GetDataStore()->GetString());
									CheckRadioButton(hWnd,
										IDC_BGSEE_CODAGLOBALDATASTORE_ADDNUMBER,
										IDC_BGSEE_CODAGLOBALDATASTORE_ADDSTRING,
										IDC_BGSEE_CODAGLOBALDATASTORE_ADDSTRING);

									break;
								default:
									EnableWindow(ValueBox, FALSE);
									break;
								}
							}
							else
								SendMessage(hWnd, IDM_BGSEE_CODAGLOBALDATASTORE_CLEAREDITFIELD, NULL, NULL);

							break;
						}
					}

					break;
				}

				break;
			case WM_INITDIALOG:
				{
					SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);
					Instance = (CodaScriptGlobalDataStore*)lParam;

					SendMessage(hWnd, IDM_BGSEE_CODAGLOBALDATASTORE_RELOADVARLIST, NULL, NULL);
				}

				break;
			}

			return FALSE;
		}

		bool CodaScriptGlobalDataStore::Add( CodaScriptVariable* Variable )
		{
			if (Lookup(Variable->GetName()) == nullptr)
			{
				Cache.push_back(Variable);
				return true;
			}

			return false;
		}

		template<typename T>
		CodaScriptVariable* CodaScriptGlobalDataStore::Add( const char* Name, T Value, bool& ExistingVar )
		{
			CodaScriptVariable* Global = Lookup(Name);

			if (Global == nullptr)
			{
				ICodaScriptDataStoreOwner* StoreOwner = VM->BuildDataStoreOwner();
				Global = new CodaScriptVariable(std::string(Name), StoreOwner);
				Cache.push_back(Global);

				ExistingVar = false;
			}
			else
				ExistingVar = true;

			*Global->GetStoreOwner()->GetDataStore() = Value;
			return Global;
		}

		CodaScriptVariable* CodaScriptGlobalDataStore::Add( const char* Name, CodaScriptStringParameterTypeT Value, bool& ExistingVar )
		{
			return Add<CodaScriptStringParameterTypeT>(Name, Value, ExistingVar);
		}

		CodaScriptVariable* CodaScriptGlobalDataStore::Add( const char* Name, CodaScriptNumericDataTypeT Value, bool& ExistingVar )
		{
			return Add<CodaScriptNumericDataTypeT>(Name, Value, ExistingVar);
		}

		CodaScriptVariable* CodaScriptGlobalDataStore::Add( const char* Name, CodaScriptReferenceDataTypeT Value, bool& ExistingVar )
		{
			return Add<CodaScriptReferenceDataTypeT>(Name, Value, ExistingVar);
		}

		void CodaScriptGlobalDataStore::Remove( CodaScriptVariable* Variable )
		{
			CodaScriptVariable::ArrayT::iterator Match;
			if (Lookup(Variable, Match))
			{
				delete Variable;
				Cache.erase(Match);
			}
		}

		CodaScriptVariable* CodaScriptGlobalDataStore::Lookup( const char* Name )
		{
			for (auto Itr = Cache.begin(); Itr != Cache.end(); Itr++)
			{
				CodaScriptVariable* Global = *Itr;
				if (!_stricmp(Global->GetName(), Name))
					return Global;
			}

			return nullptr;
		}

		bool CodaScriptGlobalDataStore::Lookup( CodaScriptVariable* Variable, CodaScriptVariable::ArrayT::iterator& Match )
		{
			for (auto Itr = Cache.begin(); Itr != Cache.end(); Itr++)
			{
				CodaScriptVariable* Global = *Itr;
				if (Global == Variable)
				{
					Match = Itr;
					return true;
				}
			}

			return false;
		}

		void CodaScriptGlobalDataStore::Clear( void )
		{
			for (auto Itr : Cache)
				delete Itr;

			Cache.clear();
		}

		void CodaScriptGlobalDataStore::INILoadState( void )
		{
			this->Clear();

			char SectionBuffer[0x8000] = {0};
			if (INISettingGetter(CODASCRIPTGLOBALDATASTORE_INISECTION, SectionBuffer, sizeof(SectionBuffer)))
			{
				VM->GetMessageHandler()->Log("Global Data Store:");
				VM->GetMessageHandler()->Indent();
				for (const char* Itr = SectionBuffer; *Itr != '\0'; Itr += strlen(Itr) + 1)
				{
					std::string StrBuffer(Itr);
					size_t IndexA = StrBuffer.find("="), IndexB = StrBuffer.find("|");
					if (IndexA != std::string::npos && IndexB != std::string::npos)
					{
						std::string Name(StrBuffer.substr(0, IndexA));
						std::string Type(StrBuffer.substr(IndexA + 1, IndexB - IndexA - 1));
						std::string Value(StrBuffer.substr(IndexB + 1));
						bool Throwaway = false;

						SME_ASSERT(Type == "s" || Type == "S" || Type == "n" || Type == "N");

						if (Type == "s" || Type == "S")
							Add(Name.c_str(), Value.c_str(), Throwaway);
						else
							Add(Name.c_str(), atof(Value.c_str()), Throwaway);

						VM->GetMessageHandler()->Log("%s = %s", Name.c_str(), Value.c_str());
					}
				}
				VM->GetMessageHandler()->Outdent();
			}
		}

		void CodaScriptGlobalDataStore::INISaveState( void )
		{
			INISettingSetter(CODASCRIPTGLOBALDATASTORE_INISECTION, nullptr);
			char Buffer[0x512] = {0};

			for (auto Itr : Cache)
			{
				bool InvalidType = false;
				switch (Itr->GetStoreOwner()->GetDataStore()->GetType())
				{
				case ICodaScriptDataStore::kDataType_Numeric:
					FORMAT_STR(Buffer, "n|%0.6f", Itr->GetStoreOwner()->GetDataStore()->GetNumber() * 1.0);
					break;
				case ICodaScriptDataStore::kDataType_String:
					FORMAT_STR(Buffer, "s|%s", Itr->GetStoreOwner()->GetDataStore()->GetString());
					break;
				default:
					InvalidType = true;
					break;
				}

				if (InvalidType)
				{
					VM->GetMessageHandler()->Log("Couldn't save Coda script global variable '%s' - Unexpected type %c!",
												 Itr->GetName(), Itr->GetStoreOwner()->GetDataStore()->GetType());
				}
				else
					INISettingSetter(Itr->GetName(), CODASCRIPTGLOBALDATASTORE_INISECTION, Buffer);
			}
		}

		CodaScriptGlobalDataStore::CodaScriptGlobalDataStore(ICodaScriptVirtualMachine* VM,
															 INIManagerGetterFunctor Getter,
															 INIManagerSetterFunctor Setter ) :
			Cache(),
			INISettingGetter(Getter),
			INISettingSetter(Setter),
			VM(VM)
		{
			SME_ASSERT(VM);
			INILoadState();
		}

		CodaScriptGlobalDataStore::~CodaScriptGlobalDataStore()
		{
			INISaveState();
			Clear();
		}

		void CodaScriptGlobalDataStore::ShowEditDialog( HINSTANCE ResourceInstance, HWND Parent )
		{
			BGSEEUI->ModalDialog(ResourceInstance, MAKEINTRESOURCE(IDD_BGSEE_CODAGLOBALDATASTORE), Parent, EditDlgProc, (LPARAM)this);
		}

		CodaScriptVariable::ArrayT& CodaScriptGlobalDataStore::GetCache( void )
		{
			return Cache;
		}

		const std::string				CodaScriptVM::kBackgroundDepotName	= "Background";
		const std::string				CodaScriptVM::kSourceExtension		= ".coda";
		CodaScriptVM*					CodaScriptVM::Singleton				= nullptr;


		bgsee::ConsoleCommandInfo		CodaScriptVM::kDumpCodaDocsConsoleCommandData =
		{
			"DumpCodaDocs",
			0,
			CodaScriptVM::DumpCodaDocsConsoleCommandHandler
		};

		void CodaScriptVM::DumpCodaDocsConsoleCommandHandler(UInt32 ParamCount, const char* Args)
		{
			CODAVM->CommandRegistry->Dump("coda_command_doc.html");
		}

		CodaScriptVM::CodaScriptVM(ResourceLocation BasePath,
								   const char* WikiURL,
								   INIManagerGetterFunctor INIGetter,
								   INIManagerSetterFunctor INISetter,
								   CodaScriptCommandRegistrar::ListT& ScriptCommands) :
			ICodaScriptVirtualMachine(ObjectFactoryType::MUP),
			BaseDirectory(BasePath),
			CommandRegistry(new CodaScriptCommandRegistry(WikiURL)),
			MessageHandler(new CodaScriptMessageHandler("Coda Script")),
			ProgramCache(new CodaScriptProgramCache(this)),
			Executive(new CodaScriptExecutive(this)),
			Backgrounder(new CodaScriptBackgrounder(this,
													BaseDirectory.GetRelativePath() + "\\" + kBackgroundDepotName, INIGetter, INISetter)),
			GlobalStore(new CodaScriptGlobalDataStore(this, INIGetter, INISetter)),
			ExpressionParser(BuildExpressionParser()),
			Initialized(false)
		{
			SME_ASSERT(Singleton == nullptr);
			Singleton = this;

			Initialized = true;

			// register built-in commands first
			ScriptCommands.push_front(commands::general::GetRegistrar());
			ScriptCommands.push_front(commands::string::GetRegistrar());
			ScriptCommands.push_front(commands::array::GetRegistrar());

			CommandRegistry->RegisterCommands(ScriptCommands);
			CommandRegistry->InitializeExpressionParser(GetParser());

			// register console command
			BGSEECONSOLE->RegisterConsoleCommand(&kDumpCodaDocsConsoleCommandData);

			Backgrounder->Rebuild();
		}

		CodaScriptVM::~CodaScriptVM()
		{
			// we need to release the following pointers in an order-dependent fashion
			GlobalStore.reset(nullptr);
			Backgrounder.reset(nullptr);
			ProgramCache.reset(nullptr);
			Executive.reset(nullptr);
			ExpressionParser.reset(nullptr);

			bool Leakage = false;

			if (ICodaScriptExecutableCode::GetGIC())
			{
				MessageHandler->Log("CodaScriptVM::D'tor - Session leaked %d instances of ICodaScriptExecutableCode!", ICodaScriptExecutableCode::GetGIC());
				Leakage = true;
			}

			if (CodaScriptBackingStore::GetGIC())
			{
				MessageHandler->Log("CodaScriptVM::D'tor - Session leaked %d instances of CodaScriptBackingStore!", CodaScriptBackingStore::GetGIC());
				Leakage = true;
			}

			if (CodaScriptVariable::GetGIC())
			{
				MessageHandler->Log("CodaScriptVM::D'tor - Session leaked %d instances of CodaScriptVariable!", CodaScriptVariable::GetGIC());
				Leakage = true;
			}

			if (mup::CodaScriptMUPArrayDataType::GetGIC())
			{
				MessageHandler->Log("CodaScriptVM::D'tor - Session leaked %d instances of mup::CodaScriptMUPArrayDataType!", mup::CodaScriptMUPArrayDataType::GetGIC());
				Leakage = true;
			}

			if (mup::CodaScriptMUPValue::GetGIC())
			{
				MessageHandler->Log("CodaScriptVM::D'tor - Session leaked %d instances of mup::CodaScriptMUPValue!", mup::CodaScriptMUPValue::GetGIC());
				Leakage = true;
			}

			if (mup::CodaScriptMUPVariable::GetGIC())
			{
				MessageHandler->Log("CodaScriptVM::D'tor - Session leaked %d instances of mup::CodaScriptMUPVariable!", mup::CodaScriptMUPVariable::GetGIC());
				Leakage = true;
			}

			std::string MUPLeakage;
			if (mup::IToken::LeakageReport(MUPLeakage))
			{
				MessageHandler->Log("CodaScriptVM::D'tor - Session leaked mup::IToken!");
				MessageHandler->Log("%s", MUPLeakage.c_str());
				Leakage = true;
			}

			if (Leakage)
				SHOW_LEAKAGE_MESSAGE("CodaScriptVM");

			Initialized = false;

			Singleton = nullptr;
		}

		CodaScriptVM* CodaScriptVM::Get()
		{
			return Singleton;
		}

		bool CodaScriptVM::Initialize( ResourceLocation BasePath,
										const char* WikiURL,
										INIManagerGetterFunctor INIGetter,
										INIManagerSetterFunctor INISetter,
										CodaScriptCommandRegistrar::ListT& ScriptCommands )
		{
			if (Singleton)
				return false;

			CodaScriptVM* Buffer = new CodaScriptVM(BasePath, WikiURL, INIGetter, INISetter, ScriptCommands);
			return Buffer->Initialized;
		}

		void CodaScriptVM::Deinitialize()
		{
			SME_ASSERT(Singleton);
			delete Singleton;
		}

		void CodaScriptVM::ShowGlobalStoreEditDialog(HINSTANCE ResourceInstance, HWND Parent)
		{
			SME_ASSERT(Backgrounder->IsBackgrounding() == false);

			bool BackgrounderState = Backgrounder->IsEnabled();
			Backgrounder->Suspend();

			GlobalStore->ShowEditDialog(ResourceInstance, Parent);

			if (BackgrounderState)
				Backgrounder->Resume();
		}

		void CodaScriptVM::OpenScriptRepository( void ) const
		{
			ShellExecute(nullptr, "open", BaseDirectory.GetFullPath().c_str(), nullptr, nullptr, SW_SHOW);
		}

		ICodaScriptExpressionParser* CodaScriptVM::BuildExpressionParser()
		{
			switch (FactoryType)
			{
			case ObjectFactoryType::MUP:
				return new mup::CodaScriptMUPExpressionParser();
			default:
				throw CodaScriptException("Invalid ObjectFactoryType");
			}
		}

		ICodaScriptDataStoreOwner* CodaScriptVM::BuildDataStoreOwner()
		{
			switch (FactoryType)
			{
			case ObjectFactoryType::MUP:
				return new mup::CodaScriptMUPValue((mup::float_type)0.0);
			default:
				throw CodaScriptException("Invalid ObjectFactoryType");
			}
		}

		ICodaScriptArrayDataType::SharedPtrT CodaScriptVM::BuildArray(UInt32 InitialSize /*= 0*/)
		{
			switch (FactoryType)
			{
			case ObjectFactoryType::MUP:
				{
					ICodaScriptArrayDataType::SharedPtrT Array(new mup::CodaScriptMUPArrayDataType(InitialSize));
					return Array;
				}
			default:
				throw CodaScriptException("Invalid ObjectFactoryType");
			}
		}

		const ResourceLocation& CodaScriptVM::GetScriptRepository() const
		{
			return BaseDirectory;
		}

		const std::string& CodaScriptVM::GetScriptFileExtension() const
		{
			return kSourceExtension;
		}

		CodaScriptVariable* CodaScriptVM::GetGlobal(const char* Name) const
		{
			return GlobalStore->Lookup(Name);
		}

		const CodaScriptVariable::ArrayT& CodaScriptVM::GetGlobals() const
		{
			return GlobalStore->GetCache();
		}

		CodaScriptMessageHandler* CodaScriptVM::GetMessageHandler() const
		{
			return MessageHandler.get();
		}

		ICodaScriptExpressionParser* CodaScriptVM::GetParser() const
		{
			return ExpressionParser.get();
		}

		ICodaScriptExecutor* CodaScriptVM::GetExecutor() const
		{
			return Executive.get();
		}

		ICodaScriptBackgroundDaemon* CodaScriptVM::GetBackgroundDaemon() const
		{
			return Backgrounder.get();
		}

		ICodaScriptProgramCache* CodaScriptVM::GetProgramCache() const
		{
			return ProgramCache.get();
		}

		bool CodaScriptVM::IsProgramExecuting(ICodaScriptProgram* Program) const
		{
			return Executive->IsProgramExecuting(Program);
		}

		void CodaScriptVM::RunScript(ExecuteParams& Input, ExecuteResult& Output)
		{
			ResourceLocation Path(Input.Program ? Input.Program->GetFilepath().GetRelativePath() :
								BaseDirectory.GetRelativePath() + "\\" + Input.Filepath + CodaScriptVM::kSourceExtension);
			if (ResourceLocation::IsRelativeTo(Path, Backgrounder->GetBackgroundScriptRepository()))
			{
				MessageHandler->Log("Cannot execute background script '%s' manually", Input.Filepath.c_str());
				return;
			}

			ICodaScriptProgram* Program = ProgramCache->Get(Path, Input.Recompile);
			if (Program && Program->IsValid())
			{
				try
				{
					ICodaScriptExecutionContext::PtrT Context(new CodaScriptExecutionContext(this, Program));
					if (Input.Parameters.empty() == false)
						Context->SetParameters(Input.Parameters);

					if (Input.RunInBackground)
					{
						Backgrounder->Queue(Context.release());
						Output.Success = true;
					}
					else
						Executive->Execute(Context.get(), Output);
				}
				catch (CodaScriptException& ex)
				{
					MessageHandler->Log("Couldn't execute script '%s' - %s", Program->GetName().c_str(), ex.Get());
				}
			}
		}
	}
}