#pragma once
#include "IntelliSenseItem.h"

namespace cse
{
	namespace intellisense
	{
		ref class CodeSnippetCollection;

		[Flags]
		static enum class DatabaseLookupFilter
		{
			None = 0,

			Command = 1 << 0,
			GlobalVariable = 1 << 1,
			Quest = 1 << 2,
			Script = 1 << 3,
			UserFunction = 1 << 4,
			GameSetting = 1 << 5,
			Form = 1 << 6,
			ObjectReference = 1 << 7,
			Snippet = 1 << 8,

			All = Command | GlobalVariable | Quest | Script | UserFunction | GameSetting | Form | ObjectReference | Snippet
		};

		[Flags]
		static enum class DatabaseLookupOptions
		{
			None = 0,

			OnlyCommandsThatNeedCallingObject = 1 << 0,
		};

		ref struct FetchIntelliSenseItemsArgs
		{
			property String^					IdentifierToMatch;
			property StringMatchType			MatchType;
			property DatabaseLookupFilter		FilterBy;
			property DatabaseLookupOptions		Options;

			FetchIntelliSenseItemsArgs()
			{
				IdentifierToMatch = String::Empty;
				MatchType = StringMatchType::StartsWith;
				FilterBy = DatabaseLookupFilter::All;
				Options = DatabaseLookupOptions::None;
			}
		};

		ref struct ContextualIntelliSenseLookupArgs
		{
			property String^	CurrentToken;
			property String^	PreviousToken;
			property bool		DotOperatorInUse;
			property bool		OnlyWithInsightInfo;

			ContextualIntelliSenseLookupArgs()
			{
				CurrentToken = String::Empty;
				PreviousToken = String::Empty;
				DotOperatorInUse = false;
				OnlyWithInsightInfo = false;
			}
		};

		ref struct ContextualIntelliSenseLookupResult
		{
			property IntelliSenseItem^			CurrentToken;
			property bool						CurrentTokenIsCallableObject;	// Callable Object = Quests, References (w/t attached scripts)
			property bool						CurrentTokenIsObjectReference;

			property IntelliSenseItem^			PreviousToken;
			property bool						PreviousTokenIsCallableObject;
			property bool						PreviousTokenIsObjectReference;

			ContextualIntelliSenseLookupResult()
			{
				CurrentToken = nullptr;
				CurrentTokenIsCallableObject = false;
				CurrentTokenIsObjectReference = false;

				PreviousToken = nullptr;
				PreviousTokenIsCallableObject = false;
				PreviousTokenIsObjectReference = false;
			}
		};

		ref class IntelliSenseBackend
		{
			static IntelliSenseBackend^ Singleton = nullptr;

			Timer^													UpdateTimer;
			DateTime												LastUpdateTimestamp;

			Dictionary<String^, IntelliSenseItemScriptCommand^>^	ScriptCommands;
			Dictionary<String^, IntelliSenseItemGameSetting^>^		GameSettings;
			Dictionary<String^, IntelliSenseItemScript^>^			Scripts;
			Dictionary<String^, IntelliSenseItemGlobalVariable^>^	GlobalVariables;
			Dictionary<String^, IntelliSenseItemQuest^>^			Quests;
			Dictionary<String^, IntelliSenseItemForm^>^				Forms;
			Dictionary<String^, IntelliSenseItemCodeSnippet^>^		Snippets;
			CodeSnippetCollection^									SnippetCollection;

			void					UpdateTimer_Tick(Object^ Sender, EventArgs^ E);
			void					UpdateDatabase();
			String^					GetVariableValueString(componentDLLInterface::VariableData* Data);
			void					RefreshCodeSnippetIntelliSenseItems();

			generic <typename T>
			where T : IntelliSenseItem
			static void				DoFetch(Dictionary<String^, T>^% Source,
											FetchIntelliSenseItemsArgs^ Args,
											List<IntelliSenseItem^>^% OutFetched);

			IntelliSenseItem^		LookupIntelliSenseItem(String^ Identifier, bool OnlyWithInsightInfo);
			bool					IsCallableObject(IntelliSenseItem^ Item);
			bool					IsObjectReference(IntelliSenseItem^ Item);
			bool					TryGetAttachedScriptData(String^ Identifier,
															DisposibleDataAutoPtr<componentDLLInterface::ScriptData>* OutData);

			IntelliSenseBackend();
		public:
			~IntelliSenseBackend();


			property DateTime LastUpdateTime
			{
				DateTime get() { return LastUpdateTimestamp; }
			}

			void		InitializeScriptCommands(componentDLLInterface::CommandTableData* Data);
			void		InitializeGameSettings(componentDLLInterface::IntelliSenseUpdateData* Data);

			String^		GetScriptCommandDeveloperURL(String^ CommandName);

			bool		IsUserFunction(String^ Identifier);
			bool		IsScriptCommand(String^ Identifier, bool CheckCommandShorthand);
			bool		IsForm(String^ Identifier);

			HashSet<String^>^			CreateIndentifierSnapshot(DatabaseLookupFilter Categories);

			bool						HasAttachedScript(String^ Identifier);
			IntelliSenseItemScript^		GetAttachedScript(String^ Identifier);
			IntelliSenseItemScript^		GetAttachedScript(IntelliSenseItem^ Item);

			List<IntelliSenseItem^>^				FetchIntelliSenseItems(FetchIntelliSenseItemsArgs^ FetchArgs);
			ContextualIntelliSenseLookupResult^		ContextualIntelliSenseLookup(ContextualIntelliSenseLookupArgs^ LookupArgs);
			IntelliSenseItem^						LookupIntelliSenseItem(String^ Indentifier);

			String^		SanitizeIdentifier(String^ Identifier);

			void		Refresh(bool Force);
			void		ShowCodeSnippetManager();

			static IntelliSenseBackend^	Get();
			static void					Deinitialize();
		};
	}
}
