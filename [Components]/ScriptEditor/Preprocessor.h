#pragma once

ref class ScriptParser;
ref class Preprocessor;

namespace ScriptPreprocessor
{
	public delegate void StandardOutputError(String^ Message);
}

using namespace ScriptPreprocessor;

// represents a line of the script text/directive
public ref class CSEPreprocessorToken
{
protected:
	String^												Token;
public:
	virtual String^										GetToken() { return Token; }			// returns the preprocessed token

	CSEPreprocessorToken(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);
};

public ref class CSEPreprocessorDirective abstract : public CSEPreprocessorToken
{
public:
	static enum class									EncodingType
														{
															e_Invalid = 0,
															e_SingleLine,
															e_MultiLine
														};
	static array<Char>^									EncodingIdentifier = 
														{
															'~',
															'#',
															'@'
														};

	static enum class									DirectiveType
														{
															e_Invalid = 0,
															e_Define,
															e_Import,
															e_Enum,
															e_If
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

	String^												GetMultilineValue(StringReader^% TextReader, String^% SliceStart, String^% SliceEnd);
	String^												ObfuscateToCompiler(String^% Token);
public:
	CSEPreprocessorDirective() : CSEPreprocessorToken(nullptr, nullptr, nullptr), Type(DirectiveType::e_Invalid), Encoding(EncodingType::e_Invalid), ErrorFlag(false) {}
	
	EncodingType										GetEncodingType() { return Encoding; }
	DirectiveType										GetDirectiveType()	{ return Type; }
	bool												GetErrorFlag() { return ErrorFlag; }
};


public ref class DefineDirective : public CSEPreprocessorDirective
{
	String^												Name;
	String^												Value;

	bool												IsNameValid(String^% Name);
public:
	DefineDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);						// used for single line definitions
	DefineDirective(String^ Token, StringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);			// used for multi line definitions

	String^												GetName() { return Name; }
	String^												GetValue() { return Value; }
	void												SetValue(String^% Value) { this->Value = Value; }
	DefineDirective^									CreateCopy();

	virtual	String^										GetToken() override;
};

public ref class ImportDirective : public CSEPreprocessorDirective
{
	String^												Filename;
	String^												ImportSegment;
public:
	ImportDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);

	virtual	String^										GetToken() override;
};

public ref class EnumDirective : public CSEPreprocessorDirective
{
	String^												Name;
	String^												Value;
	LinkedList<DefineDirective^>^						ComponentDefineDirectives;

	void												ParseComponentDefineDirectives(String^% Source, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);
public:
	EnumDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);						// used for single line definitions
	EnumDirective(String^ Token, StringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);			// used for multi line definitions

	virtual	String^										GetToken() override;
};

public ref class IfDirective : public CSEPreprocessorDirective
{
	ref struct Operator
	{
		delegate bool									Handler(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);
	private:
		String^											Identifier;
		Handler^										EvaluationHandler;
		UInt32											Precedence;
		UInt32											OperandCount;
	public:
		Operator(String^ Identifier, Handler^ EvaluationHandler, UInt32 Precedence, UInt32 OperandCount) :
					Identifier(Identifier), EvaluationHandler(EvaluationHandler), Precedence(Precedence), OperandCount(OperandCount) {}

		String^%										GetIdentifier() { return Identifier; }
		UInt32											GetPrecedence() { return Precedence; }
		UInt32											GetOperandCount() { return OperandCount; }
		bool											Evaluate(String^% LHS, String^% RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance) { return EvaluationHandler(LHS, RHS, ErrorOutput, PreprocessorInstance); }
	};

	static bool											ParseAsInt(String^% Source, int% Result);	
	static void											ProcessOperands(String^% LHSSource, String^% RHSSource, String^% LHSResult, String^% RHSResult, Preprocessor^% PreprocessorInstance);

	static bool											EqualityOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);
	static bool											LessThanOrEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);
	static bool											GreaterThanOrEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);
	static bool											LessThanOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);
	static bool											GreaterThanOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);
	static bool											NotEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);

	static bool											LogicalAndOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);
	static bool											LogicalOrOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);

	static array<Operator^>^							OperatorList =
														{
															gcnew Operator("==", gcnew Operator::Handler(&IfDirective::EqualityOperatorEvaluator), 2, 2),
															gcnew Operator("<=", gcnew Operator::Handler(&IfDirective::LessThanOrEqualOperatorEvaluator), 0, 2),
															gcnew Operator(">=", gcnew Operator::Handler(&IfDirective::GreaterThanOrEqualOperatorEvaluator), 1, 2),
															gcnew Operator("<", gcnew Operator::Handler(&IfDirective::LessThanOperatorEvaluator), 0, 2),
															gcnew Operator(">", gcnew Operator::Handler(&IfDirective::GreaterThanOperatorEvaluator), 1, 2),
															gcnew Operator("!=", gcnew Operator::Handler(&IfDirective::NotEqualOperatorEvaluator), 2, 2),

															gcnew Operator("&&", gcnew Operator::Handler(&IfDirective::LogicalAndOperatorEvaluator), 3, 2),
															gcnew Operator("||", gcnew Operator::Handler(&IfDirective::LogicalOrOperatorEvaluator), 4, 2)
														};

	bool												ConvertInfixExpressionToPostFix(String^% Source, String^% Result, StandardOutputError^ ErrorOutput);
	Operator^											LookupOperatorByIdentifier(String^% Identifier);

	String^												BaseCondition;
	String^												Block;
	bool												ValidationResult;
	String^												SliceStart;
	String^												SliceEnd;


	bool												CheckBaseCondition(String^% Base, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance);
public:
	IfDirective(String^ Token, StringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance);

	virtual	String^										GetToken() override;
};

public ref class Preprocessor
{
	static Preprocessor^								Singleton = nullptr;
	Preprocessor();
	
	LinkedList<DefineDirective^>^						RegisteredDefineDirectives;

	void												ProcessStandardDefineDirectives(StandardOutputError^ ErrorOutput);
	CSEPreprocessorToken^								CreateDirectiveFromIdentifier(CSEPreprocessorDirective::EncodingType Encoding, String^ Identifier, String^ Token, StringReader^ TextReader, StandardOutputError^ ErrorOutput);
public:
	static Preprocessor^%								GetSingleton();

	void												RegisterDefineDirective(DefineDirective^ Directive); 
	DefineDirective^									LookupDefineDirectiveByName(String^% Name);
	bool												Preprocess(String^% Source, String^% Result, StandardOutputError^ ErrorOutput);

	bool												ScriptEditorDelegate(String^% Source, String^% Result, StandardOutputError^ ErrorOutput);
};

#define PREPROC											Preprocessor::GetSingleton()