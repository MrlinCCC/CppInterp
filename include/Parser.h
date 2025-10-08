#pragma once
#include"Lexer.h"


namespace CppInterp {
	namespace TokenType {
		constexpr Type FUNCTION = 39;      // function
		constexpr Type LET = 40;           // let
		constexpr Type CONST = 41;           // const
		constexpr Type IF = 42;            // if
		constexpr Type ELSE = 43;          // else
		constexpr Type SWITCH = 44;          // switch
		constexpr Type CASE = 45;          // case
		constexpr Type DEFAULT = 46;          // default
		constexpr Type WHILE = 47;         // while
		constexpr Type FOR = 48;           // for
		constexpr Type RETURN = 49;        // return
		constexpr Type BREAK = 50;         // break
		constexpr Type CONTINUE = 51;      // continue
		constexpr Type TRUE = 52;          // true
		constexpr Type FALSE = 53;         // false
		constexpr Type NULL_LITERAL = 54;  // null
		constexpr Type INT = 55;   // int
		constexpr Type DOUBLE = 56;   // double
		constexpr Type CHAR = 57;   // char
		constexpr Type STRING = 58;   // string
		constexpr Type BOOL = 59;   // bool
		constexpr Type VOID = 60;   // void
	}

	namespace NodeType {
		using Type = uint8_t;

		constexpr Type PROGRAM = 0;
		constexpr Type FUNCTION_DEF = 1;
		constexpr Type PARAMETER_LIST = 2;
		constexpr Type PARAMETER = 3;
		constexpr Type COMPOUND_STMT = 4;

		constexpr Type VAR_DECL = 5;
		constexpr Type EXPR_STMT = 6;
		constexpr Type IF_STMT = 7;
		constexpr Type WHILE_STMT = 8;
		constexpr Type FOR_STMT = 9;
		constexpr Type RETURN_STMT = 10;
		constexpr Type BREAK_STMT = 11;
		constexpr Type CONTINUE_STMT = 12;

		constexpr Type ASSIGN_EXPR = 13;
		constexpr Type CONDITIONAL_EXPR = 14;
		constexpr Type BINARY_EXPR = 15;
		constexpr Type UNARY_EXPR = 16;
		constexpr Type POSTFIX_EXPR = 17;
		constexpr Type CALL_EXPR = 18;

		constexpr Type INDEX_EXPR = 19;
		constexpr Type MEMBER_EXPR = 20;
		constexpr Type LITERAL = 21;
		constexpr Type IDENTIFIER = 22;
		constexpr Type MAP_LITERAL = 23;
		constexpr Type MAP_ENTRY = 24;
	};


	struct AstNode {
		NodeType::Type m_type;
		Token m_token;
		std::vector<AstNode*> m_children;
		AstNode* m_parent;

		AstNode(NodeType::Type type = NodeType::PROGRAM, AstNode* parent = nullptr)
			: m_type(type), m_parent(parent) {
		}
	};

	class Parser {
	public:
		AstNode* Parse(const std::string& str);


	private:
		void PreprocessTokens(std::vector<Token>& tokens); //区分关键字和标识符

		std::vector<Token> m_tokens;
		AstNode* m_root;
		std::vector<AstNode*> m_nodes;
	};
};