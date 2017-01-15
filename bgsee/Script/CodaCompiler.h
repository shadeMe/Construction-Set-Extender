#pragma once
#include "CodaAST.h"
#include "WorkspaceManager.h"

namespace bgsee
{
	namespace script
	{
		class ICodaScriptProgram
		{
		public:
			virtual ~ICodaScriptProgram() = 0 {}

			virtual const CodaScriptSourceCodeT&			GetName() const = 0;
			virtual const CodaScriptVariableNameArrayT&		GetVariables(CodaScriptVariableNameArrayT& OutNames) const = 0;
			virtual UInt32									GetVariableCount() const = 0;
			virtual const CodaScriptVariableNameArrayT&		GetParameters(CodaScriptVariableNameArrayT& OutNames) const = 0;		// returns the ordered list of parameter variables
			virtual double									GetPollingInteval() const = 0;
			virtual ICodaScriptExpressionParser*			GetBoundParser() const = 0;
			virtual ICodaScriptCompilerMetadata*			GetCompilerMetadata() const = 0;
			virtual bool									IsValid() const = 0;
			virtual void									InvalidateBytecode() = 0;
			virtual void									Accept(ICodaScriptSyntaxTreeEvaluator* Visitor) noexcept = 0;
			virtual const ResourceLocation&					GetFilepath() const = 0;

			typedef std::unique_ptr<ICodaScriptProgram>		PtrT;
		};


		class ICodaScriptCompilerMetadata
		{
		public:
			virtual ~ICodaScriptCompilerMetadata() = 0 {}

			virtual ICodaScriptExpressionParser*			GetParentParser() const = 0;
			virtual ICodaScriptProgram*						GetSourceProgram() const = 0;
		};

		class ICodaScriptExpressionByteCode
		{
		protected:
			ICodaScriptExecutableCode*						Source;
		public:
			ICodaScriptExpressionByteCode(ICodaScriptExecutableCode* SourceCode);
			virtual ~ICodaScriptExpressionByteCode() = 0 {}

			ICodaScriptExecutableCode*						GetSource(void) const;
		};

		// since the language syntax and feature set is predominantly based on the muParserX expression parser's,
		// the interface expects all implementations to support at least as much as the former
		class ICodaScriptExpressionParser
		{
		public:
			virtual ~ICodaScriptExpressionParser() = 0 {}

			struct CompileData
			{
				const CodaScriptVariable::ArrayT&		GlobalVariables;

				CompileData(const CodaScriptVariable::ArrayT& Globals)
					: GlobalVariables(Globals) {}
			};

			struct EvaluateData
			{
				ICodaScriptExecutionContext*		Context;
				const CodaScriptVariable::ArrayT&	GlobalVariables;

				EvaluateData(ICodaScriptExecutionContext* Context, const CodaScriptVariable::ArrayT& Globals)
					: Context(Context), GlobalVariables(Globals) {}

				CodaScriptVariable*					GetGlobal(const CodaScriptSourceCodeT& Name) const;
			};

			// commands and constants are registered once after the parser is instantiated
			virtual void									RegisterCommand(ICodaScriptCommand* Command) = 0;
			virtual void									RegisterConstant(const char* Name, CodaScriptBackingStore& Value) = 0;

			virtual void									RegisterProgram(ICodaScriptProgram* Program) = 0;
			virtual void									DeregisterProgram(ICodaScriptProgram* Program) = 0;

			virtual void									BeginCompilation(ICodaScriptProgram* Program, CompileData Data) = 0;
			virtual void									Compile(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
																	ICodaScriptExecutableCode* SourceCode,
																	ICodaScriptExpressionByteCode** OutByteCode) = 0;
			virtual void									EndCompilation(ICodaScriptProgram* Program, ICodaScriptCompilerMetadata** OutMetadata) = 0;		// caller takes ownership of the metadata

			virtual void									BeginEvaluation(ICodaScriptProgram* Program, EvaluateData Data) = 0;
			virtual void									Evaluate(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
																	 ICodaScriptExpressionByteCode* ByteCode,
																	 CodaScriptBackingStore* Result = nullptr) = 0;
			virtual void									EndEvaluation(ICodaScriptProgram* Program) = 0;

			typedef std::unique_ptr<ICodaScriptExpressionParser>		PtrT;
		};

		class CodaScriptProgram : public ICodaScriptProgram
		{
			friend class CodaScriptCompiler;
			friend class CodaScriptSyntaxTreeCompileVisitor;

			enum
			{
				kFlag_Uncompiled			= 1 << 0,
				kFlag_CompileError			= 1 << 1,
				kFlag_InvalidBytecode		= 1 << 2,
			};

			struct VariableInfo
			{
				CodaScriptSourceCodeT			Name;
				CodaScriptSourceCodeT			Initalizer;
				UInt32							Line;

				VariableInfo(CodaScriptSourceCodeT& Name, CodaScriptSourceCodeT& Initalizer, UInt32 Line) : Name(Name), Initalizer(Initalizer), Line(Line) {}
			};

			struct ParameterInfo
			{
				const VariableInfo*				BoundVariable;

				ParameterInfo(const VariableInfo* Bound) : BoundVariable(Bound) {}
			};

			typedef std::vector<VariableInfo>						VariableInfoArrayT;
			typedef std::vector<ParameterInfo>						ParameterInfoArrayT;
			typedef std::unique_ptr<CodaScriptAbstractSyntaxTree>	ScopedASTPointerT;
			typedef std::unique_ptr<ICodaScriptCompilerMetadata>	ScopedMetadataPointerT;

			ResourceLocation					Filepath;
			CodaScriptSourceCodeT				Name;
			VariableInfoArrayT					Variables;
			ParameterInfoArrayT					Parameters;
			double								PollingInterval;
			ScopedASTPointerT					AST;
			UInt8								Flags;
			ICodaScriptVirtualMachine*			VM;
			ICodaScriptExpressionParser*		Parser;
			ScopedMetadataPointerT				Metadata;

			void								AddVariable(CodaScriptSourceCodeT& Name, CodaScriptSourceCodeT& Initalizer, UInt32 Line);
			const VariableInfo*					GetVariable(CodaScriptSourceCodeT& Name) const;
			const VariableInfo*					GetVariable(const char* Name) const;
			void								AddParameter(const VariableInfo* BoundVar);
			bool								IsParameter(const VariableInfo* Var) const;

			CodaScriptProgram(ICodaScriptVirtualMachine* VM, const ResourceLocation& Filepath);
		public:
			virtual ~CodaScriptProgram();

			virtual const CodaScriptSourceCodeT&		GetName() const override;
			virtual const CodaScriptVariableNameArrayT&	GetVariables(CodaScriptVariableNameArrayT& OutNames) const override;
			virtual UInt32								GetVariableCount() const override;
			virtual const CodaScriptVariableNameArrayT&	GetParameters(CodaScriptVariableNameArrayT& OutNames) const override;
			virtual double								GetPollingInteval() const override;
			virtual ICodaScriptExpressionParser*		GetBoundParser() const override;
			virtual ICodaScriptCompilerMetadata*		GetCompilerMetadata() const override;
			virtual bool								IsValid() const override;
			virtual void								InvalidateBytecode() override;
			virtual void								Accept(ICodaScriptSyntaxTreeEvaluator* Visitor) noexcept override;
			virtual const ResourceLocation&				GetFilepath() const override;
		};


		class CodaScriptSyntaxTreeCompileVisitor : public ICodaScriptSyntaxTreeEvaluator
		{
		protected:
			bool											Failed;		// set to true when the compile operation fails

			void											GenericCompile(ICodaScriptExecutableCode* Node);
		public:
			CodaScriptSyntaxTreeCompileVisitor(ICodaScriptVirtualMachine* VM, ICodaScriptProgram* Program);
			inline virtual ~CodaScriptSyntaxTreeCompileVisitor() = default;

			virtual void									Visit(CodaScriptExpression* Node) override;
			virtual void									Visit(CodaScriptBEGINBlock* Node) override;
			virtual void									Visit(CodaScriptIFBlock* Node) override;
			virtual void									Visit(CodaScriptELSEIFBlock* Node) override;
			virtual void									Visit(CodaScriptELSEBlock* Node) override;
			virtual void									Visit(CodaScriptWHILEBlock* Node) override;
			virtual void									Visit(CodaScriptFOREACHBlock* Node) override;

			bool											HasFailed(void) const;
		};

		class CodaScriptCompiler
		{
			typedef std::stack<CodaScriptKeywordT>	CodaScriptKeywordStackT;

			bool								GetKeywordInStack(CodaScriptKeywordStackT& Stack, CodaScriptKeywordT Keyword) const;
			bool								GetKeywordOnStackTop(CodaScriptKeywordStackT& Stack, CodaScriptKeywordT Keyword) const;
			bool								CheckVariableName(const CodaScriptSourceCodeT& Name) const;

			struct SourceData
			{
				std::map<int, CodaScriptSourceCodeT>		Lines;		// key = line no
			};

			void								Preprocess(std::fstream& SourceCode, SourceData& OutPreprocessedCode);
			CodaScriptProgram*					GenerateProgram(ICodaScriptVirtualMachine* VirtualMachine,
																CodaScriptProgram* Instance,
																SourceData& SourceCode);
			CodaScriptProgram*					GenerateByteCode(ICodaScriptVirtualMachine* VirtualMachine,
																 CodaScriptProgram* In);
		public:
			CodaScriptProgram*					Compile(ICodaScriptVirtualMachine* VirtualMachine,
														const ResourceLocation& Filepath);

			static CodaScriptCompiler			Instance;

			static const UInt32					kMaxParameters = 10;
		};
	}
}