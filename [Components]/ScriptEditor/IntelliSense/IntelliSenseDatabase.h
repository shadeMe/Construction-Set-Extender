#pragma once

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		ref class Script;
		ref class IntelliSenseItem;
		ref class UserFunction;
		ref struct IntelliSenseParseScriptData;

		public ref class IntelliSenseDatabase
		{
		protected:
			static IntelliSenseDatabase^						Singleton = nullptr;

			IntelliSenseDatabase();

			ref struct ParsedUpdateData
			{
				LinkedList<UserFunction^>^						UDFList;
				LinkedList<IntelliSenseItem^>^					Enumerables;

				ParsedUpdateData() : UDFList(gcnew LinkedList<UserFunction^>()), Enumerables(gcnew LinkedList<IntelliSenseItem^>()) {}
			};

			virtual ParsedUpdateData^							InitializeDatabaseUpdate();
			virtual void										FinalizeDatabaseUpdate(ParsedUpdateData^ Data);

			virtual void										DatabaseUpdateTimer_OnTimed(Object^ Sender, Timers::ElapsedEventArgs^ E);
			virtual void										DatabaseUpdateThread_DoWork(Object^ Sender, DoWorkEventArgs^ E);
			virtual void										DatabaseUpdateThread_RunWorkerCompleted(Object^ Sender, RunWorkerCompletedEventArgs^ E);

			Timers::Timer^										DatabaseUpdateTimer;
			BackgroundWorker^									DatabaseUpdateThread;

			LinkedList<UserFunction^>^							UserFunctionList;
			Dictionary<String^, String^>^						DeveloperURLMap;
			Dictionary<String^, Script^>^						RemoteScripts;				// key = baseEditorID

			bool												ForceUpdateFlag;
			UInt32												UpdateThreadTimerInterval;	// in minutes

			void												UpdateDatabase();
		public:
			LinkedList<IntelliSenseItem^>^						Enumerables;

			virtual void										InitializeCommandTableDatabase(ComponentDLLInterface::CommandTableData* Data);
			virtual void										InitializeGMSTDatabase(ComponentDLLInterface::IntelliSenseUpdateData* GMSTCollection);

			static IntelliSenseDatabase^%						GetSingleton();
			virtual void										ParseScript(String^% SourceText, IntelliSenseParseScriptData^ Box);

			void												RegisterDeveloperURL(String^% CmdName, String^% URL);
			String^												LookupDeveloperURLByCommand(String^% CmdName);

			String^												SanitizeCommandIdentifier(String^% CmdName);

			Script^												CacheRemoteScript(String^ BaseEditorID, String^ ScriptText);	// returns the cached script
			IntelliSenseItem^									LookupRemoteScriptVariable(String^ BaseEditorID, String^ Variable);

			bool												GetIsIdentifierUserFunction(String^% Name);
			bool												GetIsIdentifierScriptCommand(String^% Name);

			void												ForceUpdateDatabase();
			void												InitializeDatabaseUpdateThread();
		};

#define ISDB											IntelliSenseDatabase::GetSingleton()
	}
}