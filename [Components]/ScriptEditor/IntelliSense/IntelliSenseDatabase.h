#pragma once

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		ref class Script;
		ref class IntelliSenseItem;
		ref class UserFunction;
		ref class IntelliSenseInterface;
		ref struct IntelliSenseParseScriptData;
		ref class CodeSnippetCollection;
		ref class IntelliSenseItemScriptCommand;
		ref class IntelliSenseItemVariable;

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
			CodeSnippetCollection^								CodeSnippets;
			UInt32												UpdateTimerInterval;		// in minutes
			LinkedList<IntelliSenseItemScriptCommand^>^			ScriptCommands;
			LinkedList<IntelliSenseItemVariable^>^				GameSettings;
			LinkedList<IntelliSenseItem^>^						Enumerables;

			void												UpdateDatabase();
		public:
			property LinkedList<IntelliSenseItem^>^		ItemRegistry
			{
				virtual LinkedList<IntelliSenseItem^>^ get() { return Enumerables; }
			}

			virtual ~IntelliSenseDatabase();

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
			bool												GetIsIdentifierScriptableForm(String^% Name, ComponentDLLInterface::ScriptData** OutScriptData);	// caller takes ownership of pointer
			bool												GetIsIdentifierForm(String^% Name);

			void												ForceUpdateDatabase();
			void												ShowCodeSnippetManager();
		};

#define ISDB											IntelliSenseDatabase::GetSingleton()

		ref struct IntelliSenseParseScriptData
		{
			IntelliSenseInterface^								SourceIntelliSenseInterface;
			Script^												SourceScript;

			static enum class									DataType
			{
				e_UserFunction = 0,
				e_Script,
				e_IntelliSenseInterface
			};

			DataType											Type;

			IntelliSenseParseScriptData(IntelliSenseInterface^ Obj);
			IntelliSenseParseScriptData(Script^ Obj);
			IntelliSenseParseScriptData(UserFunction^ Obj);
		};
	}
}