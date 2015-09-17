#pragma once
#include "[Common]\IncludesCLR.h"

using namespace System::Runtime::Serialization;

namespace ConstructionSetExtender
{
	class ByteCodeParser
	{
	protected:
		static UInt32										Read16(Array^% Data, UInt32% CurrentOffset);
		static bool											LineHasData(String^% Line);
	public:
		static UInt32										GetOffsetForLine(String^% Line, Array^% Data, UInt32% CurrentOffset);
	};

	namespace ObScriptSemanticAnalysis
	{
		static enum	class ScriptType
		{
			None = 0,
			Object,
			Quest,
			MagicEffect
		};

		static enum	class ScriptTokenType
		{
			None = 0,
			ScriptName,
			Variable,
			Comment,
			Begin,
			End,
			While,
			ForEach,
			Loop,
			If,
			ElseIf,
			Else,
			EndIf,
			Return,
			SetFunctionValue,
			Set,
			Let,
			Call,
			Player,
			SetEventHandler,
			RemoveEventHandler,
			DispatchEvent,
		};

		ref class Tokenizer
		{
			bool					Good;
			String^					ReferenceDelimiters;
			String^					ReferenceControlChars;
		public:
			List<String^>^			Tokens;
			List<UInt32>^			Indices;					// the position of each token relative to its parent line
			List<Char>^				Delimiters;

			property UInt32			TokenCount
			{
				virtual UInt32 get() { return Tokens->Count; }
			}
			property bool			Valid
			{
				virtual bool get() { return Good; }
			}

			Tokenizer();
			Tokenizer(String^ InputDelimiters);
			Tokenizer(String^ InputDelimiters, String^ InputControlChars);

			bool						Tokenize(String^ Source, bool CollectEmptyTokens);
			void						ResetState();

			ScriptTokenType				GetFirstTokenType(void);
			int							GetCommentTokenIndex(int SearchEndIndex);		// returns the index of the token that contains the comment delimiter. argument specifies the end token index for the search (pass -1 for full search)
			int							GetTokenIndex(String^ Source);					// returns the token index corresponding to the first match

			static ScriptTokenType		GetScriptTokenType(String^ ScriptToken);
			static bool					GetIndexInsideString(String^ Source, int Index);	// returns true if the index follows a " character or if it is wrapped b'ween two
			static bool					HasIllegalChar(String^ Source, String^ Includes, String^ Excludes);

			static String^			DefaultDelimiters = "., (){}[]\t\r\n";
			static String^			DefaultControlChars = " \t";
		};

		ref struct Variable
		{
			[DataContract]
			static enum class DataType
			{
				[EnumMember]
				None = 0,
				[EnumMember]
				Integer,
				[EnumMember]
				Float,
				[EnumMember]
				Ref,
				[EnumMember]
				StringVar,
				[EnumMember]
				ArrayVar
			};

			String^		Name;
			String^		Comment;
			DataType	Type;
			bool		UDFParameter;
			UInt32		ParameterIndex;		// index of the variable in the UDF parameter list (0-9)
			UInt32		Line;
			UInt32		RefCount;

			Variable(String^ Name, DataType Type, String^ Comment, UInt32 Line);

			static Variable::DataType				GetVariableDataType(String^ TypeToken);
			static String^							GetVariableDataTypeToken(Variable::DataType Type);
			static String^							GetVariableDataTypeDescription(Variable::DataType Type);
		};

		ref struct ControlBlock
		{
			static enum	class ControlBlockType
			{
				None = 0,
				ScriptBlock,
				If,
				ElseIf,
				Else,
				While,
				ForEach
			};

			ControlBlockType		Type;
			UInt32					StartLine;
			UInt32					EndLine;				// line with the block end specifier
			UInt32					IndentLevel;			// indent count for the block's contents
			ControlBlock^			Parent;					// nullptr for script blocks
			bool					BasicBlock;				// set to true for all blocks except IFs with ELSE/ELSEIF clauses
			UInt32					OuterEndLine;			// set to the ENDIF line for IF's with ELSE/ELSEIF clauses

			ControlBlock(ControlBlockType Type, UInt32 Start, UInt32 Indents, ControlBlock^ Parent);

			bool					IsMalformed();
		};

		ref struct ScriptBlock : public ControlBlock
		{
			static enum	class ScriptBlockAttribute
			{
				None = 0,
				Universal,
				ReferenceSpecific,
				MagicEffect
			};

			static enum class ScriptBlockType
			{
				None = 0,
				GameMode,
				MenuMode,
				OnActivate,
				OnActorEquip,
				OnActorUnequip,
				OnAdd,
				OnAlarm,
				OnAlarmVictim,
				OnDeath,
				OnDrop,
				OnEquip,
				OnHit,
				OnHitWith,
				OnKnockout,
				OnLoad,
				OnMagicEffectHit,
				OnMurder,
				OnPackageChange,
				OnPackageDone,
				OnPackageStart,
				OnReset,
				OnSell,
				OnTrigger,
				OnTriggerActor,
				OnTriggerMob,
				OnUnequip,
				ScriptEffectFinish,
				ScriptEffectStart,
				ScriptEffectUpdate,
				Function,
			};

			ScriptBlockAttribute	SBAttribute;
			ScriptBlockType			SBType;
			bool					CompilerOverride;

			ScriptBlock(UInt32 Start, UInt32 Indents, ScriptBlockType Type, bool Override);

			bool							IsBlockValid(ScriptType Type);

			static ScriptBlockType			GetScriptBlockType(String^ TypeToken);
			static String^					GetScriptBlockTypeToken(ScriptBlockType Type);
			static bool						HasCompilerOverride(String^ TypeToken);
		};

		ref struct AnalysisData
		{
			ref struct UserMessage
			{
				UInt32				Line;
				String^				Message;
				bool				Critical;

				UserMessage(UInt32 Line, String^ Message, bool Critical);
			};

			String^										Name;
			String^										Description;
			List<Variable^>^							Variables;
			UInt32										NextVariableLine;			// line where the next variable can be inserted
			List<ControlBlock^>^						ControlBlocks;
			bool										MalformedStructure;
			UInt32										FirstStructuralErrorLine;
			bool										UDF;
			Variable^									UDFResult;					// nullptr if ambiguous
			List<UserMessage^>^							AnalysisMessages;
			property bool								HasCriticalMessages
			{
				virtual bool get() { return GetHasCriticalMessages(); }
			}

			[Flags]
			static enum class Operation
			{
				None = 0,
				FillVariables = 1 << 0,
				FillControlBlocks = 1 << 1,
				FillUDFData = 1 << 2,

				PerformBasicValidation = 1 << 3,
				CountVariableReferences = 1 << 4,
				SuppressQuestVariableRefCount = 1 << 5,
				CheckVariableNameCommandCollisions = 1 << 6,
				CheckVariableNameFormCollisions = 1 << 7,
			};

			AnalysisData();
			~AnalysisData();

			delegate void						CheckVariableNameCollision(String^ VarName, bool% HasCommandCollision, bool% HasFormCollision);

			void								PerformAnalysis(String^ ScriptText, ScriptType Type, Operation Operations, CheckVariableNameCollision^ Delegate);

			ControlBlock^						GetBlockStartingAt(UInt32 Line);
			ControlBlock^						GetBlockEndingAt(UInt32 Line);
			UInt32								GetLineIndentLevel(UInt32 Line);
			Variable^							LookupVariable(String^ VarName);

			static String^						PerformLocalizedIndenting(String^ Source, UInt32 DefaultIndentLevel);
		private:
			void								LogAnalysisMessage(UInt32 Line, String^ Message);
			void								LogCriticalAnalysisMessage(UInt32 Line, String^ Message);
			bool								GetHasCriticalMessages();
			bool								ParseConditionExpression(UInt32 Line, String^ Expression);
		};

		ref class Sanitizer
		{
			String^								InputText;
			AnalysisData^						Data;
			String^								SanitizedText;
		public:
			Sanitizer(String^ Source);

			[Flags]
			static enum class Operation
			{
				IndentLines = 1 << 0,
				AnnealCasing = 1 << 1,
				EvalifyIfs = 1 << 2,
				CompilerOverrideBlocks = 1 << 3,
			};

			property String^					Output
			{
				virtual String^	get() { return SanitizedText; }
			}

			delegate String^					GetSanitizedIdentifier(String^ Identifier);

			bool								SanitizeScriptText(Operation Operations, GetSanitizedIdentifier^ Delegate);		// returns false if unsuccessful
		};

		ref class Documenter
		{
			String^								InputText;
			String^								DocumentedText;

			String^								GetVariableDescription(String^ Identifier, Dictionary<String^, String^>^ Descriptions);
		public:
			Documenter(String^ Source);

			property String^					Output
			{
				virtual String^	get() { return DocumentedText; }
			}

			void								Document(String^ ScriptDescription, Dictionary<String^, String^>^ VariableDescriptions);
		};
	};

#ifdef CSE_SE
	typedef ObScriptSemanticAnalysis::Tokenizer						ScriptParser;
	typedef ObScriptSemanticAnalysis::AnalysisData::UserMessage		ScriptErrorMessage;
#endif
}