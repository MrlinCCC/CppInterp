#pragma once
#include <vector>
#include "Parser.h"
#include <optional>
#include "Exception.hpp"

namespace CppInterp {

	class ProgramNode;
	class ImportStmt;
	class FunctionDecl;
	class CompoundStmt;
	class IfStmt;
	class WhileStmt;
	class ForStmt;
	class SwitchStmt;
	class CaseClause;
	class DefaultClause;
	class ReturnStmt;
	class BreakStmt;
	class ContinueStmt;
	class VariableDecl;
	class Declarator;
	class StructDef;
	class InitializerExpr;
	class ConditionalExpr;
	class AssignmentExpr;
	class BinaryExpr;
	class UnaryExpr;
	class LiteralExpr;
	class IdentifierExpr;
	class CallExpr;
	class MemberAccessExpr;
	class ArrayAccessExpr;
	class LambdaExpr;
	class Expression;

	class AstVisitor {
	public:
		virtual void Visit(ProgramNode& node) = 0;
		virtual void Visit(ImportStmt& node) = 0;
		virtual void Visit(FunctionDecl& node) = 0;
		virtual void Visit(CompoundStmt& node) = 0;
		virtual void Visit(IfStmt& node) = 0;
		virtual void Visit(WhileStmt& node) = 0;
		virtual void Visit(ForStmt& node) = 0;
		virtual void Visit(SwitchStmt& node) = 0;
		virtual void Visit(CaseClause& node) = 0;
		virtual void Visit(DefaultClause& node) = 0;
		virtual void Visit(ReturnStmt& node) = 0;
		virtual void Visit(BreakStmt& node) = 0;
		virtual void Visit(ContinueStmt& node) = 0;
		virtual void Visit(VariableDecl& node) = 0;
		virtual void Visit(Declarator& node) = 0;
		virtual void Visit(StructDef& node) = 0;
		virtual void Visit(InitializerExpr& node) = 0;
		virtual void Visit(ConditionalExpr& node) = 0;
		virtual void Visit(AssignmentExpr& node) = 0;
		virtual void Visit(BinaryExpr& node) = 0;
		virtual void Visit(UnaryExpr& node) = 0;
		virtual void Visit(LiteralExpr& node) = 0;
		virtual void Visit(IdentifierExpr& node) = 0;
		virtual void Visit(CallExpr& node) = 0;
		virtual void Visit(MemberAccessExpr& node) = 0;
		virtual void Visit(ArrayAccessExpr& node) = 0;
		virtual void Visit(LambdaExpr& node) = 0;
	};

	struct TypeInfo {

		enum Kind {
			BUILTIN,    // int, double, bool, char, string
			ARRAY,      // T[]
			FUNCTION,   // (paramTypes) -> returnType
			STRUCT,     // struct
			UNKNOWN
		};

		Kind m_kind;
		std::string m_name;
		TypeInfo* m_elementType;
		std::vector<TypeInfo*> m_paramTypes;
		TypeInfo* m_returnType;

		TypeInfo();
		TypeInfo(Kind k, const std::string& n = "");

		bool IsSame(const TypeInfo& other) const;
	};

	class TypeRegistry {
	public:
		TypeRegistry();
		~TypeRegistry();

		std::optional<TypeInfo*> Find(const std::string& name) const;
		TypeInfo* RegisterStruct(const std::string& name);
		TypeInfo* GetOrCreateArray(TypeInfo* elem);
		TypeInfo* GetOrCreateFunction(const std::vector<TypeInfo*>& params, TypeInfo* ret);

	private:
		void InitBuiltins();

		std::unordered_map<std::string, TypeInfo*> m_builtinTypes;
		std::unordered_map<std::string, TypeInfo*> m_structTypes;
		std::unordered_map<std::string, TypeInfo*> m_arrayTypes;
		std::unordered_map<std::string, TypeInfo*> m_functionTypes;
	};

	struct SemanticAstNode {
		virtual ~SemanticAstNode() = default;
		virtual void Accept(AstVisitor& visitor) = 0;
	};

	struct ProgramNode : SemanticAstNode {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};

		std::vector<SemanticAstNode*> m_declarations;
	};

	struct ImportStmt : SemanticAstNode {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		std::string m_moduleName;
	};

	struct FunctionDecl : SemanticAstNode {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		std::string m_name;
		TypeInfo* m_returnType;
		std::vector<VariableDecl*> m_parameters;
		CompoundStmt* m_body;
	};

	struct Statement : SemanticAstNode {};

	struct CompoundStmt : Statement {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		std::vector<Statement*> m_statements;
	};

	struct IfStmt : Statement {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Expression* m_condition;
		Statement* m_thenBranch;
		Statement* m_elseBranch;
	};

	struct WhileStmt : Statement {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Expression* m_condition;
		Statement* m_body;
	};

	struct ForStmt : Statement {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		SemanticAstNode* m_init;
		Expression* m_condition;
		Expression* m_increment;
		Statement* m_body;
	};

	struct DefaultClause : SemanticAstNode {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		std::vector<std::unique_ptr<Statement>> statements;
	};

	struct SwitchStmt : Statement {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Expression* m_condition;
		std::vector<CaseClause*> m_cases;
		DefaultClause* m_defaultClause;
	};

	struct CaseClause : SemanticAstNode {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Expression* m_value;
		std::vector<Statement*> m_statements;
	};

	struct ReturnStmt : Statement {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Expression* m_value;
	};

	struct BreakStmt : Statement {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
	};

	struct ContinueStmt : Statement {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
	};

	struct VariableDecl : Statement {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		bool m_isConst;
		TypeInfo* m_type;
		std::vector<Declarator*> m_declarators;
	};

	struct Declarator : SemanticAstNode {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		std::string m_name;
		std::vector<Expression*> m_arrayDimensions;
		Expression* m_initializer;
	};

	struct StructDef : SemanticAstNode {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		std::string m_name;
		std::vector<VariableDecl*> m_members;
	};

	struct Expression : SemanticAstNode {};

	struct InitializerExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		std::vector<std::pair<std::string, Expression*>> m_designatedInitializers;
	};

	struct ConditionalExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Expression* m_condition, * m_trueExpr, * m_falseExpr;
	};

	struct AssignmentExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Token m_op;
		Expression* m_left, * m_right;
	};

	struct BinaryExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Token m_op;
		Expression* m_left, * m_right;
	};

	struct UnaryExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Token m_op;
		Expression* m_operand;
	};

	struct LiteralExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Token m_token;
	};

	struct IdentifierExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		std::string m_name;
	};

	struct CallExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Expression* m_callee;
		std::vector<Expression*> m_arguments;
	};

	struct MemberAccessExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Expression* m_object;
		std::string m_member;
	};

	struct ArrayAccessExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		Expression* m_array;
		Expression* m_index;
	};

	struct Parameter {
		std::string m_name;
		TypeInfo* m_type;
	};

	struct LambdaExpr : Expression {
		inline void Accept(AstVisitor& visitor) override {
			visitor.Visit(*this);
		};
		std::vector<Parameter> m_parameters;
		TypeInfo* m_returnType;
		CompoundStmt* m_body;
	};

	class AstConverter {
	public:
		AstConverter() = default;
		~AstConverter();

		SemanticAstNode* ConvertToSemanticAst(AstNode* root);

	private:
		void ClearNodes();

		SemanticAstNode* ConvertNode(AstNode* node);

		SemanticAstNode* ConvertProgram(AstNode* node);
		SemanticAstNode* ConvertImportStmt(AstNode* node);
		SemanticAstNode* ConvertFunctionDecl(AstNode* node);

		SemanticAstNode* ConvertCompoundStmt(AstNode* node);
		SemanticAstNode* ConvertEmptyStmt(AstNode* node);
		SemanticAstNode* ConvertVariableDecl(AstNode* node);
		SemanticAstNode* ConvertDeclaratorList(AstNode* node);
		SemanticAstNode* ConvertIfStmt(AstNode* node);
		SemanticAstNode* ConvertCaseClause(AstNode* node);
		SemanticAstNode* ConvertWhileStmt(AstNode* node);
		SemanticAstNode* ConvertDefaultClause(AstNode* node);
		SemanticAstNode* ConvertForStmt(AstNode* node);
		SemanticAstNode* ConvertReturnStmt(AstNode* node);
		SemanticAstNode* ConvertBreakStmt(AstNode* node);
		SemanticAstNode* ConvertContinueStmt(AstNode* node);
		SemanticAstNode* ConvertStructDecl(AstNode* node);
		SemanticAstNode* ConvertStructDeclaratorList(AstNode* node);
		SemanticAstNode* ConvertMemberDecl(AstNode* node);
		SemanticAstNode* ConvertExpression(AstNode* node);
		SemanticAstNode* ConvertAssignExpr(AstNode* node);
		SemanticAstNode* ConvertConditionExpr(AstNode* node);
		SemanticAstNode* ConvertBinaryExpr(AstNode* node);
		SemanticAstNode* ConvertUnaryExpr(AstNode* node);
		SemanticAstNode* ConvertPostfixExpr(AstNode* node);
		SemanticAstNode* ConvertCallExpr(AstNode* node);
		SemanticAstNode* ConvertIndexExpr(AstNode* node);
		SemanticAstNode* ConvertMemberExpr(AstNode* node);
		SemanticAstNode* ConvertGroupExpr(AstNode* node);
		SemanticAstNode* ConvertInitializer(AstNode* node);
		SemanticAstNode* ConvertDesignedInitializer(AstNode* node);
		SemanticAstNode* ConvertArgumentList(AstNode* node);
		SemanticAstNode* ConvertFunctionLiteral(AstNode* node);
		SemanticAstNode* ConvertPrimary(AstNode* node);
		SemanticAstNode* ConvertLiteral(AstNode* node);
		SemanticAstNode* ConvertParameterTypeList(AstNode* node);

		Declarator* ConvertDeclarator(AstNode* node);
		std::vector<VariableDecl*> ConvertParameterList(AstNode* node);
		VariableDecl* ConvertParameter(AstNode* node);
		TypeInfo* ConvertType(AstNode* node);
		std::vector<Expression*> ConvertArraySize(AstNode* node);


		using NodeConvertFunc = std::function<SemanticAstNode* (AstNode*)>;

		SemanticAstNode* m_root;
		std::vector<SemanticAstNode*> m_nodes;
		TypeRegistry m_typeRegistry;
	};


	struct Variable {

	};

	struct Function {

	};

	struct Struct {
	};

	class Context {
	public:
		Context(Context* parent = nullptr) :m_parent(parent) {}
	private:
		Context* m_parent;
		std::unordered_map<std::string, Variable> m_variables;
		std::unordered_map<std::string, Function> m_functions;
		std::unordered_map<std::string, Struct> m_structs;
	};

	class SemanticAnalyzer {
	public:
		SemanticAnalyzer(AstNode* astRoot);
		void SemanticAnalyze();
	private:
		AstNode* m_astRoot;
		Context* m_globalContext;
		std::vector<Context*> m_contextStack;
		std::vector<Context*> m_contexts;
	};
}