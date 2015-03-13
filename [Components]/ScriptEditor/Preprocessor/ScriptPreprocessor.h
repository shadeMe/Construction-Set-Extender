#pragma once

namespace ConstructionSetExtender
{
	ref class ScriptParser;
	ref class Preprocessor;

	namespace ScriptPreprocessor
	{
		public delegate void StandardOutputError(String^ Message);

		void DummyStandardErrorOutput(String^ Message);
	}

	using namespace ScriptPreprocessor;

	public ref struct ScriptEditorPreprocessorData
	{
		String^											DepotPath;
		String^											StandardDirectivePath;
		bool											AllowMacroRedefinitions;
		UInt8											NoOfPasses;
		bool											ContainsDirectives;				// set to true if the source script contained at least one directive

		ScriptEditorPreprocessorData(String^ DepotPath,
									String^ StandardDirectivePath,
									bool AllowMacroRedefinitions,
									UInt8 NoOfPasses) :
			DepotPath(DepotPath),
			StandardDirectivePath(StandardDirectivePath),
			AllowMacroRedefinitions(AllowMacroRedefinitions),
			NoOfPasses(NoOfPasses),
			ContainsDirectives(false)
		{
			;//
		}
	};

	// represents a line of the script text/directive
	public ref class CSEPreprocessorToken
	{
	protected:
		String^												Token;
	public:
		virtual String^										GetToken() { return Token; }			// returns the preprocessed token
		virtual bool										GetValid() { return true; }

		CSEPreprocessorToken(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
	};

	public ref class CSEPreprocessorDirective abstract : public CSEPreprocessorToken
	{
	public:
		static enum class									EncodingType
		{
			Invalid = 0,
			SingleLine,
			MultiLine
		};
		static array<Char>^									EncodingIdentifier =
		{
			'~',
			'#',
			'@'
		};

		static enum class									DirectiveType
		{
			Invalid = 0,
			Define,
			Import,
			Enum,
			If
		};
		static array<String^>^								DirectiveIdentifier =
		{
			"Invalid",
			"Define",
			"Import",
			"Enum",
			"If"
		};
	protected:
		DirectiveType										Type;
		EncodingType										Encoding;
		bool												ErrorFlag;

		String^												SliceStart;
		String^												SliceEnd;

		String^												GetMultilineValue(CSEStringReader^% TextReader, String^% SliceStart, String^% SliceEnd);
		String^												ObfuscateToCompiler(String^ Token);
	public:
		CSEPreprocessorDirective() : CSEPreprocessorToken(nullptr, nullptr, nullptr), Type(DirectiveType::Invalid), Encoding(EncodingType::Invalid), ErrorFlag(false), SliceStart(""), SliceEnd("") {}

		virtual bool										GetValid() override { return ErrorFlag == false; }

		EncodingType										GetEncodingType() { return Encoding; }
		DirectiveType										GetDirectiveType()	{ return Type; }
		bool												GetErrorFlag() { return ErrorFlag; }
	};

	public ref class DefineDirective : public CSEPreprocessorDirective
	{
		String^												Name;
		String^												Value;

		bool												IsNameValid(String^ Name);
	public:
		static enum class									AccessoryOperatorType
		{
			None = 0,
			Stringize
		};
		static array<String^>^								AccessoryOperatorIdentifier =
		{
			"",
			"#"
		};

		DefineDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance, UInt32 LineNumber);						// used for single line definitions
		DefineDirective(String^ Token, CSEStringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);			// used for multi line definitions

		String^												GetName() { return Name; }
		String^												GetValue(String^ Prefix, AccessoryOperatorType ActiveOperator);
		void												SetValue(String^ Value) { this->Value = Value; }
		DefineDirective^									CreateCopy();

		static AccessoryOperatorType						GetAccessoryOperatorFromToken(String^ Token);

		virtual	String^										GetToken() override;
	};

	public ref class ImportDirective : public CSEPreprocessorDirective
	{
		String^												Filename;
		String^												ImportSegment;
	public:
		ImportDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance, UInt32 LineNumber);

		virtual	String^										GetToken() override;
		String^												GetFilename() { return Filename; }
	};

	public ref class EnumDirective : public CSEPreprocessorDirective
	{
		String^												Name;
		String^												Value;
		LinkedList<DefineDirective^>^						ComponentDefineDirectives;

		void												ParseComponentDefineDirectives(String^ Source, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance, UInt32 LineNumber);
	public:
		EnumDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance, UInt32 LineNumber);						// used for single line definitions
		EnumDirective(String^ Token, CSEStringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);			// used for multi line definitions

		virtual	String^										GetToken() override;
	};

	public ref class IfDirective : public CSEPreprocessorDirective
	{
		ref struct Operator
		{
			delegate bool									Handler(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);

			static enum class								BuiltInOperators
			{
				Equal,
				LessThanOrEqual,
				GreaterThanOrEqual,
				LessThan,
				GreaterThan,
				NotEqual,

				LogicalAND,
				LogicalOR
			};
			static array<String^>^							BuiltInOperatorsIdentifier =
			{
				"==",
				"<=",
				">=",
				"<",
				">",
				"!=",

				"&&",
				"||"
			};

			static bool										Evaluator(BuiltInOperators Type, String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
		private:
			String^											Identifier;
			Handler^										EvaluationHandler;
			UInt32											Precedence;
			UInt32											OperandCount;
		public:
			Operator(String^ Identifier, Handler^ EvaluationHandler, UInt32 Precedence, UInt32 OperandCount) :
			  Identifier(Identifier), EvaluationHandler(EvaluationHandler), Precedence(Precedence), OperandCount(OperandCount) {}

			  String^										GetIdentifier() { return Identifier; }
			  UInt32										GetPrecedence() { return Precedence; }
			  UInt32										GetOperandCount() { return OperandCount; }
			  bool											Evaluate(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance) { return EvaluationHandler(LHS, RHS, ErrorOutput, PreprocessorInstance); }
		};

		static bool											ParseAsInt(String^ Source, int% Result);
		static void											ProcessOperands(String^ LHSSource, String^ RHSSource, String^% LHSResult, String^% RHSResult, Preprocessor^ PreprocessorInstance);

		static bool											EqualityOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
		static bool											LessThanOrEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
		static bool											GreaterThanOrEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
		static bool											LessThanOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
		static bool											GreaterThanOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
		static bool											NotEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);

		static bool											LogicalAndOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
		static bool											LogicalOrOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);

		static array<Operator^>^							OperatorList =
		{
			gcnew Operator(Operator::BuiltInOperatorsIdentifier[(int)Operator::BuiltInOperators::Equal],
							gcnew Operator::Handler(&IfDirective::EqualityOperatorEvaluator), 2, 2),
			gcnew Operator(Operator::BuiltInOperatorsIdentifier[(int)Operator::BuiltInOperators::LessThanOrEqual],
							gcnew Operator::Handler(&IfDirective::LessThanOrEqualOperatorEvaluator), 0, 2),
			gcnew Operator(Operator::BuiltInOperatorsIdentifier[(int)Operator::BuiltInOperators::GreaterThanOrEqual],
							gcnew Operator::Handler(&IfDirective::GreaterThanOrEqualOperatorEvaluator), 1, 2),
			gcnew Operator(Operator::BuiltInOperatorsIdentifier[(int)Operator::BuiltInOperators::LessThan],
							gcnew Operator::Handler(&IfDirective::LessThanOperatorEvaluator), 0, 2),
			gcnew Operator(Operator::BuiltInOperatorsIdentifier[(int)Operator::BuiltInOperators::GreaterThan],
							gcnew Operator::Handler(&IfDirective::GreaterThanOperatorEvaluator), 1, 2),
			gcnew Operator(Operator::BuiltInOperatorsIdentifier[(int)Operator::BuiltInOperators::NotEqual],
							gcnew Operator::Handler(&IfDirective::NotEqualOperatorEvaluator), 2, 2),

			gcnew Operator(Operator::BuiltInOperatorsIdentifier[(int)Operator::BuiltInOperators::LogicalAND],
							gcnew Operator::Handler(&IfDirective::LogicalAndOperatorEvaluator), 3, 2),
			gcnew Operator(Operator::BuiltInOperatorsIdentifier[(int)Operator::BuiltInOperators::LogicalOR],
							gcnew Operator::Handler(&IfDirective::LogicalOrOperatorEvaluator), 4, 2)
		};

		bool												ConvertInfixExpressionToPostFix(String^ Source, String^% Result, StandardOutputError^ ErrorOutput);
		Operator^											LookupOperatorByIdentifier(String^ Identifier);

		String^												BaseCondition;
		String^												Block;
		bool												ValidationResult;

		bool												CheckBaseCondition(String^ Base, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
	public:
		IfDirective(String^ Token, CSEStringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);

		virtual	String^										GetToken() override;
	};

	public ref class Preprocessor
	{
		static Preprocessor^								Singleton = nullptr;
		Preprocessor();

		LinkedList<DefineDirective^>^						RegisteredDefineDirectives;
		ScriptEditorPreprocessorData^						DataBuffer;
		bool												Busy;

		void												ProcessStandardDirectives(String^ Path, StandardOutputError^ ErrorOutput);
		CSEPreprocessorToken^								CreateDirectiveFromIdentifier(CSEPreprocessorDirective::EncodingType Encoding, String^ Identifier, String^ Token, CSEStringReader^ TextReader, StandardOutputError^ ErrorOutput);
	public:
		static Preprocessor^								GetSingleton();

		static String^										kPreprocessorSigil = ";<CSE_PREPROCESSOR_SIGIL>;";

		ScriptEditorPreprocessorData^%						GetInstanceData() { return DataBuffer; }
		void												RegisterDefineDirective(DefineDirective^ Directive);
		DefineDirective^									LookupDefineDirectiveByName(String^ Name);
		bool												Preprocess(String^ Source, String^% Result, StandardOutputError^ ErrorOutput);		// INTERNAL

		// public API
		bool												PreprocessScript(String^ Source, String^% Result, StandardOutputError^ ErrorOutput, ScriptEditorPreprocessorData^ Data);
		bool												GetImportFilePath(String^ Source, String^% Result, ScriptEditorPreprocessorData^ Data);
	};

#define PREPROC											Preprocessor::GetSingleton()
}