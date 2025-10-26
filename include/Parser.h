#pragma once
#include"Lexer.h"
#include <iostream>
#include <algorithm>

namespace CppInterp {

	namespace TokenType {
		constexpr Type FUNCTION = 39;      // function
		constexpr Type LET = 40;           // let
		constexpr Type CONST = 41;           // const
		constexpr Type STRUCT = 42;           // struct
		constexpr Type IF = 43;            // if
		constexpr Type ELSE = 44;          // else
		constexpr Type SWITCH = 45;          // switch
		constexpr Type CASE = 46;          // case
		constexpr Type DEFAULT = 47;          // default
		constexpr Type WHILE = 48;         // while
		constexpr Type FOR = 49;           // for
		constexpr Type RETURN = 50;        // return
		constexpr Type BREAK = 51;         // break
		constexpr Type CONTINUE = 52;      // continue
		constexpr Type BOOL_LITERAL = 53;          // false, true
		constexpr Type NULL_LITERAL = 54;  // null
		constexpr Type INT = 55;   // int
		constexpr Type DOUBLE = 56;   // double
		constexpr Type CHAR = 57;   // char
		constexpr Type STRING = 58;   // string
		constexpr Type BOOL = 59;   // bool
		constexpr Type VOID = 60;   // void
		constexpr Type IMPORT = 61;   // import
		constexpr Type LAMBDA = 62;   // lambda
	};

	namespace NodeType {
		using Type = uint8_t;

		// --- Program structure ---
		constexpr Type PROGRAM = 0;
		constexpr Type IMPORT_STMT = 1;
		constexpr Type FUNCTION_DECL = 2;

		// --- Statements ---
		constexpr Type COMPOUND_STMT = 10;
		constexpr Type EXPRESSION_STMT = 11;
		constexpr Type VAR_DECL = 12;
		constexpr Type STRUCT_DECL = 13;
		constexpr Type IF_STMT = 14;
		constexpr Type SWITCH_STMT = 15;
		constexpr Type CASE_STMT = 16;
		constexpr Type DEFAULT_STMT = 17;
		constexpr Type WHILE_STMT = 18;
		constexpr Type FOR_STMT = 19;
		constexpr Type RETURN_STMT = 20;
		constexpr Type BREAK_STMT = 21;
		constexpr Type CONTINUE_STMT = 22;

		// --- Expressions ---
		constexpr Type COMMA_EXPR = 30;
		constexpr Type ASSIGN_EXPR = 31;
		constexpr Type COND_EXPR = 32;
		constexpr Type BINARY_EXPR = 33;
		constexpr Type UNARY_EXPR = 34;
		constexpr Type POSTFIX_EXPR = 35;
		constexpr Type FUNCTION_CALL = 36;
		constexpr Type ARRAY_INDEX = 37;
		constexpr Type MEMBER_ACCESS = 38;
		constexpr Type FUNCTION_LITERAL = 39;
		constexpr Type IDENTIFIER = 40;
		constexpr Type LITERAL = 41;

		//--- Auxiliary Node ---
		constexpr Type PARAMETER = 50;
		constexpr Type DECLARATOR = 51;
		constexpr Type STRUCT_MEMBER_DECL = 52;
		constexpr Type INITIALIZER = 53;

		// --- Types ---
		constexpr Type BUILTIN_TYPE = 60;
		constexpr Type NAMED_TYPE = 61;
		constexpr Type FUNCTION_TYPE = 62;
	};

	inline std::string NodeTypeToString(NodeType::Type type) {
		std::string typeStr;
		switch (type) {
		case NodeType::PROGRAM: return "PROGRAM";
		case NodeType::IMPORT_STMT: return "IMPORT_STMT";
		case NodeType::FUNCTION_DECL: return "FUNCTION_DECL";

		case NodeType::COMPOUND_STMT: return "COMPOUND_STMT";
		case NodeType::EXPRESSION_STMT: return "EXPRESSION_STMT";
		case NodeType::VAR_DECL: return "VAR_DECL";
		case NodeType::STRUCT_DECL: return "STRUCT_DECL";
		case NodeType::IF_STMT: return "IF_STMT";
		case NodeType::SWITCH_STMT: return "SWITCH_STMT";
		case NodeType::CASE_STMT: return "CASE_STMT";
		case NodeType::DEFAULT_STMT: return "DEFAULT_STMT";
		case NodeType::WHILE_STMT: return "WHILE_STMT";
		case NodeType::FOR_STMT: return "FOR_STMT";
		case NodeType::RETURN_STMT: return "RETURN_STMT";
		case NodeType::BREAK_STMT: return "BREAK_STMT";
		case NodeType::CONTINUE_STMT: return "CONTINUE_STMT";

		case NodeType::COMMA_EXPR: return "COMMA_EXPR";
		case NodeType::ASSIGN_EXPR: return "ASSIGN_EXPR";
		case NodeType::COND_EXPR: return "COND_EXPR";
		case NodeType::BINARY_EXPR: return "BINARY_EXPR";
		case NodeType::UNARY_EXPR: return "UNARY_EXPR";
		case NodeType::POSTFIX_EXPR: return "POSTFIX_EXPR";
		case NodeType::FUNCTION_CALL: return "FUNCTION_CALL";
		case NodeType::ARRAY_INDEX: return "ARRAY_INDEX";
		case NodeType::MEMBER_ACCESS: return "MEMBER_ACCESS";
		case NodeType::FUNCTION_LITERAL: return "FUNCTION_LITERAL";
		case NodeType::IDENTIFIER: return "IDENTIFIER";
		case NodeType::LITERAL: return "LITERAL";

		case NodeType::PARAMETER: return "PARAMETER";
		case NodeType::DECLARATOR: return "DECLARATOR";
		case NodeType::STRUCT_MEMBER_DECL: return "STRUCT_MEMBER_DECL";
		case NodeType::INITIALIZER: return "INITIALIZER";

		case NodeType::BUILTIN_TYPE: return "BUILTIN_TYPE";
		case NodeType::NAMED_TYPE: return "NAMED_TYPE";
		case NodeType::FUNCTION_TYPE: return "FUNCTION_TYPE";

		default:
			return "UNKNOWN_NODE_TYPE";
		}
		return typeStr;
	}

	struct ProgramNode;
	struct ImportNode;
	struct FunctionDeclNode;

	struct StatementNode;
	struct CompoundStmtNode;
	struct ExpressionStmtNode;
	struct VariableDeclNode;
	struct StructDeclNode;
	struct IfStmtNode;
	struct SwitchStmtNode;
	struct CaseNode;
	struct DefaultNode;
	struct WhileStmtNode;
	struct ForStmtNode;
	struct ReturnStmtNode;
	struct BreakStmtNode;
	struct ContinueStmtNode;

	struct ExpressionNode;
	struct CommaExprNode;
	struct AssignmentExprNode;
	struct ConditionalExprNode;
	struct BinaryExprNode;
	struct UnaryExprNode;
	struct PostfixExprNode;
	struct FunctionCallNode;
	struct ArrayIndexNode;
	struct MemberAccessNode;
	struct FunctionLiteralNode;
	struct IdentifierNode;
	struct LiteralNode;

	struct ParameterNode;
	struct DeclaratorNode;
	struct StructMemberNode;
	struct InitializerNode;

	struct TypeNode;
	struct BuiltinTypeNode;
	struct NamedTypeNode;
	struct FunctionTypeNode;

	class AstVisitor {
	public:
		virtual void Visit(ProgramNode& node) = 0;
		virtual void Visit(ImportNode& node) = 0;
		virtual void Visit(FunctionDeclNode& node) = 0;

		virtual void Visit(CompoundStmtNode& node) = 0;
		virtual void Visit(ExpressionStmtNode& node) = 0;
		virtual void Visit(VariableDeclNode& node) = 0;
		virtual void Visit(StructDeclNode& node) = 0;
		virtual void Visit(IfStmtNode& node) = 0;
		virtual void Visit(SwitchStmtNode& node) = 0;
		virtual void Visit(CaseNode& node) = 0;
		virtual void Visit(DefaultNode& node) = 0;
		virtual void Visit(WhileStmtNode& node) = 0;
		virtual void Visit(ForStmtNode& node) = 0;
		virtual void Visit(ReturnStmtNode& node) = 0;
		virtual void Visit(BreakStmtNode& node) = 0;
		virtual void Visit(ContinueStmtNode& node) = 0;

		virtual void Visit(CommaExprNode& node) = 0;
		virtual void Visit(AssignmentExprNode& node) = 0;
		virtual void Visit(ConditionalExprNode& node) = 0;
		virtual void Visit(BinaryExprNode& node) = 0;
		virtual void Visit(UnaryExprNode& node) = 0;
		virtual void Visit(PostfixExprNode& node) = 0;
		virtual void Visit(FunctionCallNode& node) = 0;
		virtual void Visit(ArrayIndexNode& node) = 0;
		virtual void Visit(MemberAccessNode& node) = 0;
		virtual void Visit(FunctionLiteralNode& node) = 0;
		virtual void Visit(IdentifierNode& node) = 0;
		virtual void Visit(LiteralNode& node) = 0;

		virtual void Visit(ParameterNode& node) = 0;
		virtual void Visit(DeclaratorNode& node) = 0;
		virtual void Visit(StructMemberNode& node) = 0;
		virtual void Visit(InitializerNode& node) = 0;

		virtual void Visit(BuiltinTypeNode& node) = 0;
		virtual void Visit(NamedTypeNode& node) = 0;
		virtual void Visit(FunctionTypeNode& node) = 0;
	};

	struct AstNode {
		AstNode* m_parent = nullptr;
		NodeType::Type m_nodeType;
		int m_line = 0;
		int m_column = 0;

		AstNode(NodeType::Type type, AstNode* parent = nullptr)
			: m_nodeType(type), m_parent(parent) {
		}

		virtual ~AstNode() = default;
		virtual void Accept(AstVisitor& visitor) = 0;
	};

	struct ProgramNode : AstNode {
		std::vector<AstNode*> m_declarations;
		ProgramNode(AstNode* parent = nullptr) : AstNode(NodeType::PROGRAM, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct ImportNode : AstNode {
		std::string m_moduleName;
		bool m_isStringLiteral = false;
		ImportNode(AstNode* parent = nullptr) : AstNode(NodeType::IMPORT_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct FunctionDeclNode : AstNode {
		TypeNode* m_returnType = nullptr;
		IdentifierNode* m_name = nullptr;
		std::vector<ParameterNode*> m_params;
		CompoundStmtNode* m_body = nullptr;
		FunctionDeclNode(AstNode* parent) : AstNode(NodeType::FUNCTION_DECL, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }

	};

	struct StatementNode : AstNode {
		using AstNode::AstNode;
	};

	struct CompoundStmtNode : StatementNode {
		std::vector<AstNode*> m_statements;
		CompoundStmtNode(AstNode* parent) : StatementNode(NodeType::COMPOUND_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }

	};

	struct ExpressionStmtNode : StatementNode {
		ExpressionNode* m_expression = nullptr;
		ExpressionStmtNode(AstNode* parent) : StatementNode(NodeType::EXPRESSION_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }

	};

	struct VariableDeclNode : StatementNode {
		bool m_isConst = false;
		TypeNode* m_type = nullptr;
		std::vector<DeclaratorNode*> m_declarators;
		VariableDeclNode(AstNode* parent) : StatementNode(NodeType::VAR_DECL, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }

	};

	struct StructDeclNode : StatementNode {
		IdentifierNode* m_name = nullptr;
		std::vector<StructMemberNode*> m_members;
		StructDeclNode(AstNode* parent) : StatementNode(NodeType::STRUCT_DECL, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }

	};

	struct IfStmtNode : StatementNode {
		ExpressionNode* m_condition = nullptr;
		StatementNode* m_thenStmt = nullptr;
		StatementNode* m_elseStmt = nullptr;
		IfStmtNode(AstNode* parent) : StatementNode(NodeType::IF_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }

	};

	struct SwitchStmtNode : StatementNode {
		ExpressionNode* m_condition = nullptr;
		std::vector<CaseNode*> m_cases;
		DefaultNode* m_default = nullptr;
		SwitchStmtNode(AstNode* parent) : StatementNode(NodeType::SWITCH_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct CaseNode : StatementNode {
		LiteralNode* m_literal = nullptr;
		std::vector<StatementNode*> m_statements;
		CaseNode(AstNode* parent) : StatementNode(NodeType::CASE_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct DefaultNode : StatementNode {
		std::vector<StatementNode*> m_statements;
		DefaultNode(AstNode* parent) : StatementNode(NodeType::DEFAULT_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct WhileStmtNode : StatementNode {
		ExpressionNode* m_condition = nullptr;
		StatementNode* m_body = nullptr;
		WhileStmtNode(AstNode* parent) : StatementNode(NodeType::WHILE_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct ForStmtNode : StatementNode {
		AstNode* m_init = nullptr;
		ExpressionNode* m_condition = nullptr;
		ExpressionNode* m_increment = nullptr;
		StatementNode* m_body = nullptr;
		ForStmtNode(AstNode* parent) : StatementNode(NodeType::FOR_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct ReturnStmtNode : StatementNode {
		ExpressionNode* m_expression = nullptr;
		ReturnStmtNode(AstNode* parent) : StatementNode(NodeType::RETURN_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct BreakStmtNode : StatementNode {
		BreakStmtNode(AstNode* parent) : StatementNode(NodeType::BREAK_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct ContinueStmtNode : StatementNode {
		ContinueStmtNode(AstNode* parent) : StatementNode(NodeType::CONTINUE_STMT, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct ExpressionNode : AstNode {
		using AstNode::AstNode;
	};

	struct CommaExprNode : ExpressionNode {
		std::vector<ExpressionNode*> m_expressions;
		CommaExprNode(AstNode* parent) : ExpressionNode(NodeType::COMMA_EXPR, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct AssignmentExprNode : ExpressionNode {
		std::string m_op;
		ExpressionNode* m_left = nullptr;
		ExpressionNode* m_right = nullptr;
		AssignmentExprNode(AstNode* parent) : ExpressionNode(NodeType::ASSIGN_EXPR, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct ConditionalExprNode : ExpressionNode {
		ExpressionNode* m_condition = nullptr;
		ExpressionNode* m_trueExpr = nullptr;
		ExpressionNode* m_falseExpr = nullptr;
		ConditionalExprNode(AstNode* parent) : ExpressionNode(NodeType::COND_EXPR, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct BinaryExprNode : ExpressionNode {
		std::string m_op;
		ExpressionNode* m_left = nullptr;
		ExpressionNode* m_right = nullptr;
		BinaryExprNode(AstNode* parent) : ExpressionNode(NodeType::BINARY_EXPR, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct UnaryExprNode : ExpressionNode {
		std::string m_op;
		ExpressionNode* m_operand = nullptr;
		UnaryExprNode(AstNode* parent) : ExpressionNode(NodeType::UNARY_EXPR, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct PostfixExprNode : ExpressionNode {
		ExpressionNode* m_primary = nullptr;
		std::string m_op;
		PostfixExprNode(AstNode* parent) : ExpressionNode(NodeType::POSTFIX_EXPR, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct FunctionCallNode : ExpressionNode {
		ExpressionNode* m_callee = nullptr;
		std::vector<ExpressionNode*> m_arguments; //expression and initilizer
		FunctionCallNode(AstNode* parent) : ExpressionNode(NodeType::FUNCTION_CALL, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct ArrayIndexNode : ExpressionNode {
		ExpressionNode* m_array = nullptr;
		ExpressionNode* m_index = nullptr;
		ArrayIndexNode(AstNode* parent) : ExpressionNode(NodeType::ARRAY_INDEX, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct MemberAccessNode : ExpressionNode {
		ExpressionNode* m_object = nullptr;
		IdentifierNode* m_memberName = nullptr;
		MemberAccessNode(AstNode* parent) : ExpressionNode(NodeType::MEMBER_ACCESS, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct FunctionLiteralNode : ExpressionNode {
		std::vector<ParameterNode*> m_params;
		TypeNode* m_returnType = nullptr;
		CompoundStmtNode* m_body = nullptr;
		FunctionLiteralNode(AstNode* parent) : ExpressionNode(NodeType::FUNCTION_LITERAL, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct IdentifierNode : ExpressionNode {
		std::string m_name;
		IdentifierNode(const Token& token, AstNode* parent) : ExpressionNode(NodeType::IDENTIFIER, parent) {
			m_name = token.m_content;
			m_line = token.m_line;
			m_column = token.m_column;
		}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct LiteralNode : ExpressionNode {
		TokenType::Type m_literalType;
		std::string m_value;
		LiteralNode(const Token& token, AstNode* parent) : ExpressionNode(NodeType::LITERAL, parent) {
			m_literalType = token.m_type;
			m_value = token.m_content;
			m_line = token.m_line;
			m_column = token.m_column;
		}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct ParameterNode : AstNode {
		TypeNode* m_type = nullptr;
		DeclaratorNode* m_declarator = nullptr;
		ParameterNode(AstNode* parent) : AstNode(NodeType::PARAMETER, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct DeclaratorNode : AstNode {
		IdentifierNode* m_name = nullptr;
		std::vector<ExpressionNode*> m_arraySizes;
		ExpressionNode* m_initializer = nullptr;
		DeclaratorNode(AstNode* parent) : AstNode(NodeType::DECLARATOR, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct StructMemberNode : AstNode {
		TypeNode* m_type = nullptr;
		std::vector<DeclaratorNode*> m_declarators;
		StructMemberNode(AstNode* parent) : AstNode(NodeType::STRUCT_MEMBER_DECL, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct InitializerNode : ExpressionNode {
		std::vector<ExpressionNode*> m_values;
		InitializerNode(AstNode* parent) : ExpressionNode(NodeType::INITIALIZER, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct TypeNode : AstNode {
		using AstNode::AstNode;
	};

	struct BuiltinTypeNode : TypeNode {
		std::string m_name;
		BuiltinTypeNode(const Token& token, AstNode* parent) : TypeNode(NodeType::BUILTIN_TYPE, parent) {
			m_name = token.m_content;
			m_line = token.m_line;
			m_column = token.m_column;
		}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct NamedTypeNode : TypeNode {
		std::string m_name;

		NamedTypeNode(const Token& token, AstNode* parent)
			: TypeNode(NodeType::NAMED_TYPE, parent) {
			m_name = token.m_content;
			m_line = token.m_line;
			m_column = token.m_column;
		}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	struct FunctionTypeNode : TypeNode {
		std::vector<TypeNode*> m_paramTypes;
		TypeNode* m_returnType = nullptr;
		FunctionTypeNode(AstNode* parent) : TypeNode(NodeType::FUNCTION_TYPE, parent) {}
		void Accept(AstVisitor& visitor) override { visitor.Visit(*this); }
	};

	class Parser {
	public:
		Parser() = default;
		AstNode* Parse(const std::string& str);
		AstNode* Parse(const std::vector<Token>& tokens);

		~Parser();

		inline AstNode* GetAstRoot() const { return m_root; }
		inline const std::vector<AstNode*>& GetNodes() const { return m_nodes; }
	private:
		inline void ClearNodes() {
			for (auto node : m_nodes)
				delete node;
			m_nodes.clear();
			m_root = nullptr;
		}

		void PreprocessTokens(std::vector<Token>& tokens); // distinguish keyword and identifier

		inline const Token& Peek() {
			if (m_current >= m_tokens.size()) {
				throw ParserException(
					"Unexpected end of input while peeking next token",
					m_tokens.empty() ? 0 : m_tokens.back().m_line,
					m_tokens.empty() ? 0 : m_tokens.back().m_column
				);
			}
			return m_tokens[m_current];
		}

		inline void Consume() {
			++m_current;
		}

		inline bool Check(TokenType::Type type) {
			return m_current < m_tokens.size() && Peek().m_type == type;
		}

		inline bool Match(TokenType::Type type) {
			if (Check(type)) {
				Consume();
				return true;
			}
			return false;
		}

		inline bool CheckAny(const auto& types) {
			return m_current < m_tokens.size() &&
				std::any_of(std::begin(types), std::end(types),
					[&](auto t) { return Peek().m_type == t; });
		}

		inline bool CheckAny(std::initializer_list<TokenType::Type> types) {
			return m_current < m_tokens.size() &&
				std::any_of(types.begin(), types.end(),
					[&](auto t) { return Peek().m_type == t; });
		}

		inline bool MatchAny(const std::initializer_list<TokenType::Type>& types) {
			return std::ranges::any_of(types, [&](auto t) { return Match(t); });
		}

		ProgramNode* ParseProgram(AstNode* parent);
		ImportNode* ParseImportStmt(AstNode* parent);
		FunctionDeclNode* ParseFunctionDecl(AstNode* parent);

		StatementNode* ParseStatement(AstNode* parent);
		CompoundStmtNode* ParseCompoundStmt(AstNode* parent);
		ExpressionStmtNode* ParseExpressionStmt(AstNode* parent);
		VariableDeclNode* ParseVariableDeclaration(AstNode* parent, bool consumeSemicol = true);
		StructDeclNode* ParseStructDeclaration(AstNode* parent);
		IfStmtNode* ParseIfStmt(AstNode* parent);
		SwitchStmtNode* ParseSwitchStmt(AstNode* parent);
		CaseNode* ParseCaseClause(AstNode* parent);
		DefaultNode* ParseDefaultClause(AstNode* parent);
		WhileStmtNode* ParseWhileStmt(AstNode* parent);
		ForStmtNode* ParseForStmt(AstNode* parent);
		ReturnStmtNode* ParseReturnStmt(AstNode* parent);
		BreakStmtNode* ParseBreakStmt(AstNode* parent);
		ContinueStmtNode* ParseContinueStmt(AstNode* parent);

		ExpressionNode* ParseCommaExpression(AstNode* parent);
		ExpressionNode* ParseAssignment(AstNode* parent);
		ExpressionNode* ParseConditional(AstNode* parent);
		ExpressionNode* ParseBinary(AstNode* parent, std::function<ExpressionNode* (AstNode*)> lower, const std::vector<TokenType::Type>& ops);
		ExpressionNode* ParseLogicalOr(AstNode* parent);
		ExpressionNode* ParseLogicalAnd(AstNode* parent);
		ExpressionNode* ParseBitOr(AstNode* parent);
		ExpressionNode* ParseBitXOr(AstNode* parent);
		ExpressionNode* ParseBitAnd(AstNode* parent);
		ExpressionNode* ParseEquality(AstNode* parent);
		ExpressionNode* ParseRelational(AstNode* parent);
		ExpressionNode* ParseShift(AstNode* parent);
		ExpressionNode* ParseAdditive(AstNode* parent);
		ExpressionNode* ParseMultiplicative(AstNode* parent);
		ExpressionNode* ParseUnary(AstNode* parent);
		ExpressionNode* ParsePostfix(AstNode* parent);
		ExpressionNode* ParsePrimary(AstNode* parent);
		FunctionLiteralNode* ParseFunctionLiteral(AstNode* parent);
		IdentifierNode* ParseIdentifier(AstNode* parent);
		LiteralNode* ParseLiteral(AstNode* parent);

		ParameterNode* ParseParameter(AstNode* parent);
		DeclaratorNode* ParseDeclarator(AstNode* parent);
		StructMemberNode* ParseStructMemberDeclaration(AstNode* parent);
		ExpressionNode* ParseInitializer(AstNode* parent);

		TypeNode* ParseType(AstNode* parent);
		TypeNode* ParseFunctionType(AstNode* parent);


		int m_current = 0;
		std::vector<Token> m_tokens;
		AstNode* m_root;
		std::vector<AstNode*> m_nodes;
	};


	class AstPrinter : public AstVisitor {
	public:
		static void PrintIndent(int depth) {
			for (int i = 0; i < depth; ++i) std::cout << "  ";
		}

		static void PrintAstTree(AstNode* node) {
			if (!node) return;
			static AstPrinter printer;
			printer.m_depth = 0;
			node->Accept(printer);
		}

		void Visit(ProgramNode& node) override;
		void Visit(ImportNode& node) override;
		void Visit(FunctionDeclNode& node) override;

		void Visit(CompoundStmtNode& node) override;
		void Visit(ExpressionStmtNode& node) override;
		void Visit(VariableDeclNode& node) override;
		void Visit(StructDeclNode& node) override;
		void Visit(IfStmtNode& node) override;
		void Visit(SwitchStmtNode& node) override;
		void Visit(CaseNode& node) override;
		void Visit(DefaultNode& node) override;
		void Visit(WhileStmtNode& node) override;
		void Visit(ForStmtNode& node) override;
		void Visit(ReturnStmtNode& node) override;
		void Visit(BreakStmtNode& node) override;
		void Visit(ContinueStmtNode& node) override;

		void Visit(CommaExprNode& node) override;
		void Visit(AssignmentExprNode& node) override;
		void Visit(ConditionalExprNode& node) override;
		void Visit(BinaryExprNode& node) override;
		void Visit(UnaryExprNode& node) override;
		void Visit(PostfixExprNode& node) override;
		void Visit(FunctionCallNode& node) override;
		void Visit(ArrayIndexNode& node) override;
		void Visit(MemberAccessNode& node) override;
		void Visit(FunctionLiteralNode& node) override;
		void Visit(IdentifierNode& node) override;
		void Visit(LiteralNode& node) override;

		void Visit(ParameterNode& node) override;
		void Visit(DeclaratorNode& node) override;
		void Visit(StructMemberNode& node) override;
		void Visit(InitializerNode& node) override;

		void Visit(BuiltinTypeNode& node) override;
		void Visit(NamedTypeNode& node) override;
		void Visit(FunctionTypeNode& node) override;

	private:
		int m_depth = 0;
	};


};