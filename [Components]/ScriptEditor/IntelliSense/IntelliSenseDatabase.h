#pragma once

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		ref class Script;
		ref class IntelliSenseItem;
		ref class UserFunction;
		ref struct IntelliSenseParseScriptData;

		ref class IntelliSenseDatabase
		{
		protected:
			static IntelliSenseDatabase^						Singleton = nullptr;

			IntelliSenseDatabase();

			virtual void										DatabaseUpdateTimer_Tick(Object^ Sender, EventArgs^ E);

			Timer^												DatabaseUpdateTimer;
			LinkedList<UserFunction^>^							UserFunctionList;
			Dictionary<String^, String^>^						DeveloperURLMap;
			Dictionary<String^, Script^>^						RemoteScripts;				// key = baseEditorID
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

			String^												SanitizeIdentifier(String^% Name);

			Script^												CacheRemoteScript(String^ BaseEditorID, String^ ScriptText);	// returns the cached script
			IntelliSenseItem^									LookupRemoteScriptVariable(String^ BaseEditorID, String^ Variable);

			bool												GetIsIdentifierUserFunction(String^% Name);
			bool												GetIsIdentifierScriptCommand(String^% Name);
			bool												GetIsIdentifierScriptableForm(String^% Name);
			bool												GetIsIdentifierScriptableForm(String^% Name, ComponentDLLInterface::ScriptData** OutScriptData);

			void												ForceUpdateDatabase();
		};

#define ISDB											IntelliSenseDatabase::GetSingleton()
	}
}