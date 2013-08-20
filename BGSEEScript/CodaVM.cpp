#include "CodaVM.h"
#include "CodaInterpreter.h"
#include "BGSEEUIManager.h"
#include "BGSEEMain.h"
#include "BGSEditorExtenderBase_Resource.h"

#include "MUP Implementation\CodaMUPArrayDataType.h"
#include "MUP Implementation\CodaMUPExpressionParser.h"
#include "MUP Implementation\CodaMUPValue.h"

#include "Commands\CodaScriptCommands-General.h"
#include "Commands\CodaScriptCommands-String.h"
#include "Commands\CodaScriptCommands-Array.h"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		void CodaScriptCommandRegistry::AppendToStream( std::fstream& Out, const char* Fmt, ... )
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

			return NULL;
		}

		bool CodaScriptCommandRegistry::RegisterCommand( const char* Category, ICodaScriptCommand* Command )
		{
			SME_ASSERT(Command->GetName());

			if (Category == NULL)
				Category = "Uncategorized";

			std::string Buffer = Category;
			Buffer += " Functions";

			if (LookupCommand(Command->GetName()) || LookupCommand(Command->GetAlias(), true))
				return false;

			Registry.insert(std::make_pair(Buffer, Command));
			return true;
		}

		void CodaScriptCommandRegistry::RegisterCommands( const CodaScriptRegistrarListT& Registrars )
		{
			for (CodaScriptRegistrarListT::const_iterator Itr = Registrars.begin(); Itr != Registrars.end(); Itr++)
			{
				CodaScriptCommandRegistrar* Current = *Itr;

				for (CodaScriptCommandRegistrar::CommandListT::const_iterator ItrEx = Current->Commands.begin();
																			ItrEx != Current->Commands.end();
																			ItrEx++)
				{
					RegisterCommand(Current->Category.c_str(), *ItrEx);
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
						ICodaScriptCommand::ParameterInfo* ParameterData = NULL;
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

			ShellExecute(NULL, "open", (LPSTR)"coda_command_doc.html", NULL, NULL, SW_SHOW);
		}

		CodaScriptCommandRegistrar::CodaScriptCommandRegistrar( const char* Category ) :
			Commands(),
			Category(Category)
		{
			;//
		}

		CodaScriptCommandRegistrar::~CodaScriptCommandRegistrar()
		{
			Commands.clear();
		}

		void CodaScriptCommandRegistrar::Add( ICodaScriptCommand* Command )
		{
			Commands.push_back(Command);
		}

		CodaScriptProfiler::CodaScriptProfiler() :
			Counters()
		{
			;//
		}

		CodaScriptProfiler::~CodaScriptProfiler()
		{
			SME_ASSERT(Counters.size() == 0);
		}

		void CodaScriptProfiler::BeginProfiling( void )
		{
			Counters.push(CodaScriptElapsedTimeCounterT());
		}

		long double CodaScriptProfiler::EndProfiling( void )
		{
			SME_ASSERT(Counters.size());

			CodaScriptElapsedTimeCounterT& Current = Counters.top();
			Current.Update();

			long double ElapsedTime = Current.GetTimePassed();
			Counters.pop();

			return ElapsedTime;
		}

		CodaScriptMessageHandler::CodaScriptMessageHandler() :
			State(true)
		{
			;//
		}

		CodaScriptMessageHandler::~CodaScriptMessageHandler()
		{
			;//
		}

		void CodaScriptMessageHandler::Suspend( void )
		{
			SME_ASSERT(State == true);

			State = false;
		}

		void CodaScriptMessageHandler::Resume( void )
		{
			SME_ASSERT(State == false);

			State = true;
		}

		void CodaScriptMessageHandler::LogMsg( const char* Format, ... )
		{
			if (State == false)
				return;

			va_list Args;
			char Buffer[0x1000] = {0};

			va_start(Args, Format);
			vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
			va_end(Args);

			BGSEECONSOLE_MESSAGE(Buffer);
		}

		const UInt32											CodaScriptExecutive::kMaxRecursionLimit = 30;

#define CODASCRIPTEXECUTIVE_INISECTION							"CodaExecutive"
		SME::INI::INISetting									CodaScriptExecutive::kINI_Profiling("Profiling", CODASCRIPTEXECUTIVE_INISECTION,
																									"Profile script execution",
																									(SInt32)0);

		CodaScriptExecutive::CodaScriptExecutive( CodaScriptMessageHandler* MsgHdlr ) :
			ExecutionStack(),
			Profiler(),
			OwnerThreadID(0),
			MessageHandler(MsgHdlr)
		{
			SME_ASSERT(MessageHandler);

			OwnerThreadID = GetCurrentThreadId();
		}

		CodaScriptExecutive::~CodaScriptExecutive()
		{
			SME_ASSERT(ExecutionStack.size() == 0);
		}

		bool CodaScriptExecutive::Execute( CodaScriptExecutionContext* Context, CodaScriptBackingStore* Result, bool& ReturnedResult )
		{
			SME_ASSERT(Context && OwnerThreadID == GetCurrentThreadId());

			if (ExecutionStack.size() >= kMaxRecursionLimit)
			{
				MessageHandler->LogMsg("Maximum script recursion depth hit");
				return false;
			}

			bool ProfilerEnabled = kINI_Profiling.GetData().i;

			if (ProfilerEnabled)
			{
				Profiler.BeginProfiling();
			}

			ExecutionStack.push(Context);
			CodaScriptSyntaxTreeExecuteVisitor Visitor(Context->VirtualMachine, Context, Context->BoundParser);
			bool ExecuteResult = Context->Execute(&Visitor, Result, ReturnedResult);
			ExecutionStack.pop();

			if (ProfilerEnabled)
			{
				double ElapsedTime = Profiler.EndProfiling() * 1.0;
				MessageHandler->LogMsg("Profiler: %s [%.4f ms]", Context->ScriptName.c_str(), ElapsedTime);
			}

			return ExecuteResult;
		}

		CodaScriptExecutionContext* CodaScriptExecutive::GetExecutingContext( void )
		{
			if (ExecutionStack.size())
				return ExecutionStack.top();
			else
				return NULL;
		}

		void CodaScriptExecutive::RegisterINISettings( INISettingDepotT& Depot )
		{
			Depot.push_back(&kINI_Profiling);
		}


		
		const std::string										CodaScriptBackgrounder::kDepotName	= "Background";

#define CODASCRIPTBACKGROUNDER_INISECTION						"CodaBackgrounder"
		SME::INI::INISetting									CodaScriptBackgrounder::kINI_Enabled("Enabled", CODASCRIPTBACKGROUNDER_INISECTION,
																										"Execute background scripts",
																										(SInt32)1);
		SME::INI::INISetting									CodaScriptBackgrounder::kINI_UpdatePeriod("UpdatePeriod", CODASCRIPTBACKGROUNDER_INISECTION,
																									"Duration, in milliseconds, between consecutive executions",
																									(SInt32)10);

		VOID CALLBACK CodaScriptBackgrounder::CallbackProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
		{
			if (CODAVM->Backgrounder && CODAVM->Backgrounder->TimerID == idEvent)
			{
				CODAVM->Backgrounder->Execute(CODAVM->Executive);
			}
		}

		void CodaScriptBackgrounder::ResetCache( bool Renew /*= false*/ )
		{
			SME_ASSERT(Backgrounding == false);

			for (CodaScriptBackgroundExecutionCacheT::iterator Itr = BackgroundCache.begin(); Itr != BackgroundCache.end(); Itr++)
				delete *Itr;

			BackgroundCache.clear();

			if (Renew)
			{
				for (IDirectoryIterator Itr(SourceDepot().c_str(), (std::string("*" + CodaScriptVM::kSourceExtension)).c_str()); !Itr.Done(); Itr.Next())
				{
					std::string FullPath = SourceDepot() + "\\" + std::string(Itr.Get()->cFileName);
					std::fstream InputStream(FullPath.c_str(), std::ios::in);

					if (InputStream.fail() == false)
					{
						BGSEECONSOLE_MESSAGE("Script: %s", Itr.Get()->cFileName);
						BGSEECONSOLE->Indent();

						CodaScriptExecutionContext* BackgroundScript = CODAVM->CreateExecutionContext(InputStream, NULL);
						if (BackgroundScript->GetIsValid())
						{
							BGSEECONSOLE_MESSAGE("Success: %s [%.6f s]", BackgroundScript->ScriptName.c_str(), BackgroundScript->PollingInterval);
							BackgroundCache.push_back(BackgroundScript);
						}
						else
						{
							BGSEECONSOLE_MESSAGE("Failure!");
							delete BackgroundScript;
						}
						BGSEECONSOLE->Exdent();
					}
				}
			}
		}

		void CodaScriptBackgrounder::ResetTimer( bool Renew /*= false*/ )
		{
			SME_ASSERT(Backgrounding == false);

			KillTimer(NULL, TimerID);
			TimerID = 0;

			if (Renew)
			{
				UInt32 UpdatePeriod = kINI_UpdatePeriod.GetData().i;
				TimerID = SetTimer(NULL, 0, UpdatePeriod, &CallbackProc);
				SME_ASSERT(TimerID);
			}
		}

		void CodaScriptBackgrounder::Execute( CodaScriptExecutive* Executive )
		{
			SME_ASSERT(Executive && Backgrounding == false);

			if (Executive->GetExecutingContext())
				return;		// the timer message will be dispatched by the message pump if a dialog box or window is spawned during the course of a script's execution

			SME::MiscGunk::ScopedSetter<bool> GuardBackgrounding(Backgrounding, true);
			PollingTimeCounter.Update();

			for (CodaScriptBackgroundExecutionCacheT::iterator Itr = BackgroundCache.begin(); Itr != BackgroundCache.end();)
			{
				CodaScriptExecutionContext* BackgroundScript = *Itr;
				BackgroundScript->PollingIntervalReminder -= PollingTimeCounter.GetTimePassed() / 1000.0f;

				if (BackgroundScript->PollingIntervalReminder <= 0.0f)
				{
					BackgroundScript->PollingIntervalReminder = BackgroundScript->PollingInterval;

					if (State == true)
					{
						bool Throwaway = false;

						BGSEECONSOLE->Indent();
						CODAVM->MsgHdlr()->Suspend();
						bool ExecuteResult = Executive->Execute(BackgroundScript, NULL, Throwaway);
						CODAVM->MsgHdlr()->Resume();
						BGSEECONSOLE->Exdent();

						if (ExecuteResult == false)
						{
							BGSEECONSOLE_MESSAGE("CodaScriptBackgrounder::Execute - Script '%s' halted!", BackgroundScript->ScriptName.c_str());

							delete BackgroundScript;
							Itr = BackgroundCache.erase(Itr);
							continue;
						}
					}
				}

				Itr++;
			}
		}

		CodaScriptBackgrounder::CodaScriptBackgrounder( BGSEEResourceLocation Source, BGSEEINIManagerGetterFunctor Getter, BGSEEINIManagerSetterFunctor Setter ) :
			SourceDepot(Source),
			BackgroundCache(),
			State(false),
			Backgrounding(false),
			TimerID(0),
			PollingTimeCounter(),
			INISettingGetter(Getter),
			INISettingSetter(Setter)
		{
			State = kINI_Enabled.GetData().i;
		}

		CodaScriptBackgrounder::~CodaScriptBackgrounder()
		{
			SME_ASSERT(Backgrounding == false);

			ResetTimer();
			ResetCache();

			kINI_Enabled.SetInt(State);
		}

		void CodaScriptBackgrounder::Suspend( void )
		{
			SME_ASSERT(Backgrounding == false);

			State = false;
		}

		void CodaScriptBackgrounder::Resume( void )
		{
			SME_ASSERT(Backgrounding == false);

			State = true;
		}

		bool CodaScriptBackgrounder::GetState( void ) const
		{
			return State;
		}

		void CodaScriptBackgrounder::Rebuild( void )
		{
			BGSEECONSOLE_MESSAGE("Backgrounder:");
			BGSEECONSOLE->Indent();
			ResetTimer(true);
			ResetCache(true);
			BGSEECONSOLE->Exdent();
		}

		void CodaScriptBackgrounder::RegisterINISettings( INISettingDepotT& Depot )
		{
			Depot.push_back(&kINI_Enabled);
			Depot.push_back(&kINI_UpdatePeriod);
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
				SetWindowText(NameBox, NULL);
				SetWindowText(ValueBox, NULL);
				break;
			case IDM_BGSEE_CODAGLOBALDATASTORE_RELOADVARLIST:
				{
					SendMessage(GlobaList, LB_RESETCONTENT, NULL, NULL);

					char Buffer[0x100] = {0};
					for (CodaScriptVariableListT::const_iterator Itr = Instance->Cache.begin(); Itr != Instance->Cache.end(); Itr++)
					{
						CodaScriptVariable* GlobalVar = *Itr;

						sprintf_s(Buffer, sizeof(Buffer), "[%c]\t\t%s",
							GlobalVar->GetStoreOwner()->GetDataStore()->GetType(),
							GlobalVar->GetName());

						int Index = SendMessage(GlobaList, LB_INSERTSTRING, -1, (LPARAM)Buffer);
						SendMessage(GlobaList, LB_SETITEMDATA, Index, (LPARAM)GlobalVar);
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

						CodaScriptVariable* GlobalVar = NULL;
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
			if (Lookup(Variable->GetName()) == NULL)
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

			if (Global == NULL)
			{
				ICodaScriptDataStoreOwner* StoreOwner = CodaScriptObjectFactory::BuildDataStoreOwner(CodaScriptObjectFactory::kFactoryType_MUP);
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
			CodaScriptVariableListT::iterator Match;
			if (Lookup(Variable, Match))
			{
				delete Variable;
				Cache.erase(Match);
			}
		}

		CodaScriptVariable* CodaScriptGlobalDataStore::Lookup( const char* Name )
		{
			for (CodaScriptVariableListT::iterator Itr = Cache.begin(); Itr != Cache.end(); Itr++)
			{
				CodaScriptVariable* Global = *Itr;
				if (!_stricmp(Global->GetName(), Name))
					return Global;
			}

			return NULL;
		}

		bool CodaScriptGlobalDataStore::Lookup( CodaScriptVariable* Variable, CodaScriptVariableListT::iterator& Match )
		{
			for (CodaScriptVariableListT::iterator Itr = Cache.begin(); Itr != Cache.end(); Itr++)
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
			for (CodaScriptVariableListT::iterator Itr = Cache.begin(); Itr != Cache.end(); Itr++)
				delete *Itr;

			Cache.clear();
		}

		void CodaScriptGlobalDataStore::INILoadState( void )
		{
			this->Clear();

			char SectionBuffer[0x8000] = {0};
			if (INISettingGetter(CODASCRIPTGLOBALDATASTORE_INISECTION, SectionBuffer, sizeof(SectionBuffer)))
			{
				BGSEECONSOLE_MESSAGE("Global Data Store:");
				BGSEECONSOLE->Indent();
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

						BGSEECONSOLE_MESSAGE("%s = %s", Name.c_str(), Value.c_str());
					}
				}
				BGSEECONSOLE->Exdent();
			}
		}

		void CodaScriptGlobalDataStore::INISaveState( void )
		{
			INISettingSetter(CODASCRIPTGLOBALDATASTORE_INISECTION, NULL);
			char Buffer[0x512] = {0};

			for (CodaScriptVariableListT::iterator Itr = Cache.begin(); Itr != Cache.end(); Itr++)
			{
				CodaScriptVariable* Global = *Itr;

				bool InvalidType = false;
				switch (Global->GetStoreOwner()->GetDataStore()->GetType())
				{
				case ICodaScriptDataStore::kDataType_Numeric:
					FORMAT_STR(Buffer, "n|%0.6f", Global->GetStoreOwner()->GetDataStore()->GetNumber() * 1.0);
					break;
				case ICodaScriptDataStore::kDataType_String:
					FORMAT_STR(Buffer, "s|%s", Global->GetStoreOwner()->GetDataStore()->GetString());
					break;
				default:
					InvalidType = true;
					break;
				}

				if (InvalidType)
				{
					BGSEECONSOLE_MESSAGE("CodaScriptGlobalDataStore::INISaveState - Blasphemy!! Unexpected data store value of type %c! Did you assign a reference/array to a global variable? Why dammit?!",
										Global->GetStoreOwner()->GetDataStore()->GetType());
				}
				else
				{
					INISettingSetter(Global->GetName(), CODASCRIPTGLOBALDATASTORE_INISECTION, Buffer);
				}
			}
		}

		CodaScriptGlobalDataStore::CodaScriptGlobalDataStore( BGSEEINIManagerGetterFunctor Getter, BGSEEINIManagerSetterFunctor Setter ) :
			Cache(),
			INISettingGetter(Getter),
			INISettingSetter(Setter)
		{
			this->INILoadState();
		}

		CodaScriptGlobalDataStore::~CodaScriptGlobalDataStore()
		{
			this->INISaveState();
			this->Clear();
		}

		void CodaScriptGlobalDataStore::ShowEditDialog( HINSTANCE ResourceInstance, HWND Parent )
		{
			BGSEEUI->ModalDialog(ResourceInstance, MAKEINTRESOURCE(IDD_BGSEE_CODAGLOBALDATASTORE), Parent, EditDlgProc, (LPARAM)this);
		}

		CodaScriptVariableListT& CodaScriptGlobalDataStore::GetCache( void )
		{
			return Cache;
		}

		const std::string										CodaScriptVM::kSourceExtension	= ".coda";
		CodaScriptVM*											CodaScriptVM::Singleton			= NULL;
		BGSEditorExtender::BGSEEConsoleCommandInfo				CodaScriptVM::kDumpCodaDocsConsoleCommandData =
		{
			"DumpCodaDocs",
			0,
			CodaScriptVM::DumpCodaDocsConsoleCommandHandler
		};

		void CodaScriptVM::DumpCodaDocsConsoleCommandHandler(UInt32 ParamCount, const char* Args)
		{
			CODAVM->CommandRegistry->Dump("coda_command_doc.html");
		}

		CodaScriptVM::CodaScriptVM() :
			BaseDirectory(),
			CommandRegistry(NULL),
			MessageHandler(NULL),
			Executive(NULL),
			Backgrounder(NULL),
			GlobalStore(NULL),
			ExpressionParser(NULL),
			Initialized(false)
		{
			;//
		}

		CodaScriptExecutionContext* CodaScriptVM::CreateExecutionContext( std::fstream& Input, CodaScriptMutableDataArrayT* Parameters /*= NULL*/ )
		{
			return new CodaScriptExecutionContext(this, Input, ExpressionParser, Parameters);
		}

		CodaScriptVM::~CodaScriptVM()
		{
			SAFEDELETE(Executive);
			SAFEDELETE(Backgrounder);
			SAFEDELETE(GlobalStore);
			SAFEDELETE(CommandRegistry);
			SAFEDELETE(ExpressionParser);
			SAFEDELETE(MessageHandler);

			bool Leakage = false;

			if (ICodaScriptExecutableCode::GIC)
			{
				BGSEECONSOLE_MESSAGE("CodaScriptVM::D'tor - Session leaked %d instances of ICodaScriptExecutableCode!", ICodaScriptExecutableCode::GIC);
				Leakage = true;
			}

			if (CodaScriptBackingStore::GIC)
			{
				BGSEECONSOLE_MESSAGE("CodaScriptVM::D'tor - Session leaked %d instances of CodaScriptBackingStore!", CodaScriptBackingStore::GIC);
				Leakage = true;
			}

			if (CodaScriptVariable::GIC)
			{
				BGSEECONSOLE_MESSAGE("CodaScriptVM::D'tor - Session leaked %d instances of CodaScriptVariable!", CodaScriptVariable::GIC);
				Leakage = true;
			}

			if (mup::CodaScriptMUPArrayDataType::GIC)
			{
				BGSEECONSOLE_MESSAGE("CodaScriptVM::D'tor - Session leaked %d instances of mup::CodaScriptMUPArrayDataType!", mup::CodaScriptMUPArrayDataType::GIC);
				Leakage = true;
			}

			if (mup::CodaScriptMUPValue::GIC)
			{
				BGSEECONSOLE_MESSAGE("CodaScriptVM::D'tor - Session leaked %d instances of mup::CodaScriptMUPValue!", mup::CodaScriptMUPValue::GIC);
				Leakage = true;
			}

			if (Leakage)
				SHOW_LEAKAGE_MESSAGE("CodaScriptVM");

			Initialized = false;

			Singleton = NULL;
		}

		CodaScriptVM* CodaScriptVM::GetSingleton()
		{
			if (Singleton == NULL)
				Singleton = new CodaScriptVM();

			return Singleton;
		}

		bool CodaScriptVM::Initialize( BGSEEResourceLocation BasePath,
										const char* WikiURL,
										BGSEEINIManagerGetterFunctor INIGetter,
										BGSEEINIManagerSetterFunctor INISetter,
										CodaScriptRegistrarListT& ScriptCommands )
		{
			if (Initialized)
				return false;

			Initialized = true;

			BaseDirectory = BasePath;
			CommandRegistry = new CodaScriptCommandRegistry(WikiURL);
			MessageHandler = new CodaScriptMessageHandler();
			Executive = new CodaScriptExecutive(MessageHandler);
			Backgrounder = new CodaScriptBackgrounder(BaseDirectory.GetRelativePath() + "\\" + CodaScriptBackgrounder::kDepotName, INIGetter, INISetter);
			GlobalStore = new CodaScriptGlobalDataStore(INIGetter, INISetter);
			ExpressionParser = CodaScriptObjectFactory::BuildExpressionParser(CodaScriptObjectFactory::kFactoryType_MUP);

			// register built-in commands first
			ScriptCommands.push_front(Commands::General::GetRegistrar());
			ScriptCommands.push_front(Commands::String::GetRegistrar());
			ScriptCommands.push_front(Commands::Array::GetRegistrar());

			CommandRegistry->RegisterCommands(ScriptCommands);
			CommandRegistry->InitializeExpressionParser(ExpressionParser);

			// register console command
			BGSEECONSOLE->RegisterConsoleCommand(&kDumpCodaDocsConsoleCommandData);

			Backgrounder->Rebuild();

			return Initialized;
		}

		bool CodaScriptVM::RunScript( std::string ScriptName,
									CodaScriptMutableDataArrayT* Parameters,
									CodaScriptBackingStore* Result,
									bool& ReturnedResult )
		{
			SME_ASSERT(Initialized);

			std::replace(ScriptName.begin(), ScriptName.end(), '\\', ' ');
			std::replace(ScriptName.begin(), ScriptName.end(), '/', ' ');

			BGSEEResourceLocation Path(BaseDirectory.GetRelativePath() + "\\" + ScriptName + CodaScriptVM::kSourceExtension);
			std::fstream InputStream(Path().c_str(), std::iostream::in);

			if (InputStream.fail() == false)
			{
				BGSEECONSOLE->Indent();
				std::auto_ptr<CodaScriptExecutionContext> ScriptContext(CreateExecutionContext(InputStream, Parameters));
				BGSEECONSOLE->Exdent();

				if (ScriptContext->GetIsValid() == false)
				{
					BGSEECONSOLE_MESSAGE("CodaScriptVM::RunScript - Script '%s' has outstanding errors!", ScriptContext->ScriptName.c_str());
				}
				else
				{
					BGSEECONSOLE->Indent();
					bool Success = Executive->Execute(ScriptContext.get(), Result, ReturnedResult);
					BGSEECONSOLE->Exdent();
					return Success;
				}
			}
			else
			{
				BGSEECONSOLE_MESSAGE("CodaScriptVM::RunScript - Script at '%s' couldn't be opened for reading!", Path().c_str());
			}

			return false;
		}

		void CodaScriptVM::ShowGlobalStoreEditDialog( HINSTANCE ResourceInstance, HWND Parent )
		{
			SME_ASSERT(Initialized);
			SME_ASSERT(Backgrounder->Backgrounding == false);

			bool BackgrounderState = GetBackgrounderState();
			Backgrounder->Suspend();

			GlobalStore->ShowEditDialog(ResourceInstance, Parent);
			BGSEECONSOLE_MESSAGE("Reinitializing Coda Virtual Machine");
			BGSEECONSOLE->Indent();
			Backgrounder->Rebuild();
			BGSEECONSOLE->Exdent();

			if (BackgrounderState)
				Backgrounder->Resume();
		}

		CodaScriptVariable* CodaScriptVM::GetGlobal( const char* Name )
		{
			SME_ASSERT(Initialized);

			return GlobalStore->Lookup(Name);
		}

		CodaScriptVariableListT& CodaScriptVM::GetGlobals( void ) const
		{
			SME_ASSERT(Initialized);

			return GlobalStore->GetCache();
		}

		CodaScriptMessageHandler* CodaScriptVM::MsgHdlr( void )
		{
			SME_ASSERT(Initialized);

			return MessageHandler;
		}

		bool CodaScriptVM::GetBackgrounderState( void ) const
		{
			SME_ASSERT(Backgrounder);

			return Backgrounder->GetState();
		}

		bool CodaScriptVM::ToggleBackgrounderState( void )
		{
			SME_ASSERT(Backgrounder);

			if (Backgrounder->GetState())
			{
				Backgrounder->Suspend();
				return false;
			}
			else
			{
				Backgrounder->Resume();
				return true;
			}
		}

		void CodaScriptVM::OpenScriptRepository( void ) const
		{
			SME_ASSERT(Initialized);

			ShellExecute(NULL, "open", BaseDirectory.GetFullPath().c_str(), NULL, NULL, SW_SHOW);
		}

		ICodaScriptExpressionParser* CodaScriptObjectFactory::BuildExpressionParser( UInt8 Type )
		{
			switch (Type)
			{
			case CodaScriptObjectFactory::kFactoryType_MUP:
				return new mup::CodaScriptMUPExpressionParser();
			default:
				return NULL;
			}
		}

		ICodaScriptDataStoreOwner* CodaScriptObjectFactory::BuildDataStoreOwner( UInt8 Type )
		{
			switch (Type)
			{
			case CodaScriptObjectFactory::kFactoryType_MUP:
				return new mup::CodaScriptMUPValue((mup::float_type)0.0);
			default:
				return NULL;
			}
		}

		CodaScriptSharedHandleArrayT CodaScriptObjectFactory::BuildArray( UInt8 Type, UInt32 InitialSize )
		{
			switch (Type)
			{
			case CodaScriptObjectFactory::kFactoryType_MUP:
				{
					CodaScriptSharedHandleArrayT Array(new mup::CodaScriptMUPArrayDataType(InitialSize));
					return Array;
				}
			default:
				return CodaScriptSharedHandleArrayT();
			}
		}
	}
}