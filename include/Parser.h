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
	}

	namespace NodeType {
		using Type = uint8_t;

		// --- Program structure ---
		constexpr Type PROGRAM = 0;
		constexpr Type IMPORT_STMT = 1;
		constexpr Type FUNCTION_DECL = 2;
		constexpr Type PARAMETER_LIST = 3;
		constexpr Type PARAMETER = 4;

		// --- Statements ---
		constexpr Type COMPOUND_STMT = 10;
		constexpr Type EMPTY_STMT = 11;
		constexpr Type VAR_DECL = 12;
		constexpr Type DECLARATOR_LIST = 13;
		constexpr Type DECLARATOR = 14;
		constexpr Type IF_STMT = 15;
		constexpr Type SWITCH_STMT = 16;
		constexpr Type CASE_CLAUSE = 17;
		constexpr Type WHILE_STMT = 18;
		constexpr Type DEFAULT_CLAUSE = 19;
		constexpr Type FOR_STMT = 20;
		constexpr Type RETURN_STMT = 21;
		constexpr Type BREAK_STMT = 22;
		constexpr Type CONTINUE_STMT = 23;
		constexpr Type STRUCT_DECL = 24;
		constexpr Type STRUCT_DECLARATOR_LIST = 25;
		constexpr Type STRUCT_MEMBER_DECL = 26;
		constexpr Type ARRAY_SIZE = 27;

		// --- Expressions ---
		constexpr Type EXPRESSION = 30;
		constexpr Type ASSIGN_EXPR = 31;
		constexpr Type CONDITIONAL_EXPR = 32;
		constexpr Type BINARY_EXPR = 33;
		constexpr Type UNARY_EXPR = 34;
		constexpr Type POSTFIX_EXPR = 35;
		constexpr Type CALL_EXPR = 36;
		constexpr Type INDEX_EXPR = 37;
		constexpr Type MEMBER_EXPR = 38;
		constexpr Type GROUP_EXPR = 40; //( expression )
		constexpr Type INITIALIZER = 41;
		constexpr Type ARGUMENT_LIST = 42;
		constexpr Type FUNCTION_LITERAL = 43;

		// --- Primary ---
		constexpr Type PRIMARY = 50;
		constexpr Type IDENTIFIER = 51;
		constexpr Type LITERAL = 52;

		// --- Types ---
		constexpr Type BUILTIN_TYPE = 60;
		constexpr Type FUNCTION_TYPE = 61;
		constexpr Type PARAMETER_TYPE_LIST = 62;
	};


	inline std::string NodeTypeToString(NodeType::Type type) {
		std::string typeStr;
		switch (type) {
		case NodeType::PROGRAM: return "PROGRAM";
		case NodeType::IMPORT_STMT: return "IMPORT_STMT";
		case NodeType::FUNCTION_DECL: return "FUNCTION_DECL";
		case NodeType::PARAMETER_LIST: return "PARAMETER_LIST";
		case NodeType::PARAMETER: return "PARAMETER";

		case NodeType::COMPOUND_STMT: return "COMPOUND_STMT";
		case NodeType::EMPTY_STMT: return "EMPTY_STMT";
		case NodeType::VAR_DECL: return "VAR_DECL";
		case NodeType::DECLARATOR_LIST: return "DECLARATOR_LIST";
		case NodeType::DECLARATOR: return "DECLARATOR";
		case NodeType::IF_STMT: return "IF_STMT";
		case NodeType::SWITCH_STMT: return "SWITCH_STMT";
		case NodeType::CASE_CLAUSE: return "CASE_CLAUSE";
		case NodeType::WHILE_STMT: return "WHILE_STMT";
		case NodeType::DEFAULT_CLAUSE: return "DEFAULT_CLAUSE";
		case NodeType::FOR_STMT: return "FOR_STMT";
		case NodeType::RETURN_STMT: return "RETURN_STMT";
		case NodeType::BREAK_STMT: return "BREAK_STMT";
		case NodeType::CONTINUE_STMT: return "CONTINUE_STMT";
		case NodeType::STRUCT_DECL: return "STRUCT_DECL";
		case NodeType::STRUCT_DECLARATOR_LIST: return "STRUCT_DECLARATOR_LIST";
		case NodeType::STRUCT_MEMBER_DECL: return "STRUCT_MEMBER_DECL";
		case NodeType::ARRAY_SIZE: return "ARRAY_SIZE";

		case NodeType::EXPRESSION: return "EXPRESSION";
		case NodeType::ASSIGN_EXPR: return "ASSIGN_EXPR";
		case NodeType::CONDITIONAL_EXPR: return "CONDITIONAL_EXPR";
		case NodeType::BINARY_EXPR: return "BINARY_EXPR";
		case NodeType::UNARY_EXPR: return "UNARY_EXPR";
		case NodeType::POSTFIX_EXPR: return "POSTFIX_EXPR";
		case NodeType::CALL_EXPR: return "CALL_EXPR";
		case NodeType::INDEX_EXPR: return "INDEX_EXPR";
		case NodeType::MEMBER_EXPR: return "MEMBER_EXPR";
		case NodeType::GROUP_EXPR: return "GROUP_EXPR";
		case NodeType::INITIALIZER: return "INITIALIZER";
		case NodeType::ARGUMENT_LIST: return "ARGUMENT_LIST";
		case NodeType::FUNCTION_LITERAL: return "FUNCTION_LITERAL";

		case NodeType::PRIMARY: return "PRIMARY";
		case NodeType::IDENTIFIER: return "IDENTIFIER";
		case NodeType::LITERAL: return "LITERAL";

		case NodeType::BUILTIN_TYPE: return "BUILTIN_TYPE";
		case NodeType::FUNCTION_TYPE: return "FUNCTION_TYPE";
		case NodeType::PARAMETER_TYPE_LIST: return "PARAMETER_TYPE_LIST";

		default:
			return "UNKNOWN_NODE_TYPE";
		}
		return typeStr;
	}

	struct AstNode {
		NodeType::Type m_type;
		Token m_token;
		std::vector<AstNode*> m_children;
		AstNode* m_parent;

		AstNode(NodeType::Type type = NodeType::PROGRAM, AstNode* parent = nullptr)
			: m_type(type), m_parent(parent) {
		}

		AstNode(NodeType::Type type, Token token, AstNode* parent = nullptr)
			: m_type(type), m_token(token), m_parent(parent) {
		}
	};

	class Parser {
	public:
		Parser() = default;
		AstNode* Parse(const std::string& str);
		AstNode* Parse(const std::vector<Token>& tokens);

		~Parser();

		inline AstNode* GetAstRoot() const { return m_root; }
		inline const std::vector<AstNode*>& GetNodes() const { return m_nodes; }

		static void PrintAstTree(AstNode* node, int depth = 0);
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

		inline bool CheckAny(const std::initializer_list<TokenType::Type>& types) {
			return m_current < m_tokens.size() &&
				std::ranges::any_of(types, [&](auto t) { return Peek().m_type == t; });
		}

		inline bool MatchAny(const std::initializer_list<TokenType::Type>& types) {
			return std::ranges::any_of(types, [&](auto t) { return Match(t); });
		}

		AstNode* ParseProgram(AstNode* parent);
		AstNode* ParseImportStmt(AstNode* parent);
		AstNode* ParseFunctionDef(AstNode* parent);
		AstNode* ParseStatement(AstNode* parent);
		AstNode* ParseParameterList(AstNode* parent);
		AstNode* ParseParameter(AstNode* parent);

		AstNode* ParseCompoundStmt(AstNode* parent);
		AstNode* ParseExpressionStmt(AstNode* parent);
		AstNode* ParseVariableDeclaration(AstNode* parent, bool consumeSemicolo = true);
		AstNode* ParseDeclaratorList(AstNode* parent);
		AstNode* ParseDeclarator(AstNode* parent);
		AstNode* ParseStructDeclaration(AstNode* parent);
		AstNode* ParseStructDecaratorList(AstNode* parent);
		AstNode* ParseStructMemberDeclaration(AstNode* parent);
		AstNode* ParseIfStmt(AstNode* parent);
		AstNode* ParseSwitchStmt(AstNode* parent);
		AstNode* ParseCaseClause(AstNode* parent);
		AstNode* ParseDefaultClause(AstNode* parent);
		AstNode* ParseWhileStmt(AstNode* parent);
		AstNode* ParseForStmt(AstNode* parent);
		AstNode* ParseReturnStmt(AstNode* parent);
		AstNode* ParseBreakStmt(AstNode* parent);
		AstNode* ParseContinueStmt(AstNode* parent);

		AstNode* ParseExpression(AstNode* parent);
		AstNode* ParseAssignment(AstNode* parent);
		AstNode* ParseConditional(AstNode* parent);
		AstNode* ParseLogicalOr(AstNode* parent);
		AstNode* ParseLogicalAnd(AstNode* parent);
		AstNode* ParseBitOr(AstNode* parent);
		AstNode* ParseBitXOr(AstNode* parent);
		AstNode* ParseBitAnd(AstNode* parent);
		AstNode* ParseEquality(AstNode* parent);
		AstNode* ParseRelational(AstNode* parent);
		AstNode* ParseShift(AstNode* parent);
		AstNode* ParseAdditive(AstNode* parent);
		AstNode* ParseMultiplicative(AstNode* parent);
		AstNode* ParseUnary(AstNode* parent);
		AstNode* ParsePostfix(AstNode* parent);
		AstNode* ParsePrimary(AstNode* parent);
		AstNode* ParseArgumentList(AstNode* parent);
		AstNode* ParseInitializer(AstNode* parent);
		AstNode* ParseFunctionLiteral(AstNode* parent);

		AstNode* ParseLiteral(AstNode* parent);

		AstNode* ParseType(AstNode* parent);
		AstNode* ParseBuiltinType(AstNode* parent);
		AstNode* ParseFunctionType(AstNode* parent);
		AstNode* ParseParameterTypeList(AstNode* parent);

		AstNode* ParseBinary(AstNode* parent, std::function<AstNode* (AstNode*)> lower, const std::vector<TokenType::Type>& ops);

		// X {splitToken X}
		//template <typename ParseFunc>
		//inline AstNode* ParseSeparatedListTemplate(
		//	NodeType::Type type,
		//	AstNode* parent,
		//	ParseFunc parseElementFunc,
		//	TokenType::Type splitToken = TokenType::COMMA
		//) {
		//	//element, if only one node direct return
		//	AstNode* first = (this->*parseElementFunc)(parent);
		//	if (!Check(splitToken))
		//		return first;
		//	//list parent
		//	AstNode* node = new AstNode(type, parent);
		//	m_nodes.push_back(node);
		//	node->m_children.push_back(first);
		//	first->m_parent = node;
		//	//splitToken element
		//	while (Match(splitToken)) {
		//		node->m_children.push_back((this->*parseElementFunc)(node));
		//	}
		//	return node;
		//}

		template <typename ParseFunc>
		inline AstNode* ParseSeparatedListTemplate(
			NodeType::Type type,
			AstNode* parent,
			ParseFunc parseElementFunc,
			TokenType::Type splitToken = TokenType::COMMA)
		{
			AstNode* node = new AstNode(type, parent);
			m_nodes.push_back(node);
			//element 
			node->m_children.push_back((this->*parseElementFunc)(node)); // , element 
			while (Match(splitToken))
			{
				node->m_children.push_back((this->*parseElementFunc)(node));
			}
			return node;
		}

		int m_current = 0;
		std::vector<Token> m_tokens;
		AstNode* m_root;
		std::vector<AstNode*> m_nodes;
	};

};