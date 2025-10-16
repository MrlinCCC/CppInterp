
#include"Parser.h"
#include "Exception.hpp"

using namespace CppInterp;

void Parser::PreprocessTokens(std::vector<Token>& tokens) {
	static const std::unordered_map<std::string, TokenType::Type> keywordMap = {
		{"function", TokenType::FUNCTION},
		{"let", TokenType::LET},
		{"const", TokenType::CONST},
		{"struct", TokenType::STRUCT},
		{"switch", TokenType::SWITCH},
		{"case", TokenType::CASE},
		{"default", TokenType::DEFAULT},
		{"if", TokenType::IF},
		{"else", TokenType::ELSE},
		{"while", TokenType::WHILE},
		{"for", TokenType::FOR},
		{"return", TokenType::RETURN},
		{"break", TokenType::BREAK},
		{"continue", TokenType::CONTINUE},
		{"true", TokenType::BOOL_LITERAL},
		{"false", TokenType::BOOL_LITERAL},
		{"NULL", TokenType::NULL_LITERAL},
		{"int", TokenType::INT},
		{"double", TokenType::DOUBLE},
		{"char", TokenType::CHAR},
		{"string", TokenType::STRING},
		{"bool", TokenType::BOOL},
		{"void", TokenType::VOID},
		{"import", TokenType::IMPORT},
		{"lambda",TokenType::LAMBDA}
	};
	for (auto& token : tokens) {
		if (token.m_type == TokenType::IDENTIFIER) {
			auto it = keywordMap.find(token.m_content);
			if (it != keywordMap.end()) {
				token.m_type = it->second;
			}
		}
	}
}


AstNode* Parser::Parse(const std::string& str) {
	m_tokens.clear();
	m_nodes.clear();
	m_tokens = Lexer::Instance().Tokenize(str);
	PreprocessTokens(m_tokens);
	m_current = 0;
	m_root = ParseProgram(nullptr);
	return m_root;
}

AstNode* Parser::Parse(const std::vector<Token>& tokens) {
	m_tokens.clear();
	m_nodes.clear();
	m_tokens = tokens;
	PreprocessTokens(m_tokens);
	m_current = 0;
	m_root = ParseProgram(nullptr);
	return m_root;
}

Parser::~Parser() {
	for (auto node : m_nodes) {
		delete node;
	}
}

AstNode* Parser::ParseProgram(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::PROGRAM, parent);
	m_nodes.push_back(node);
	int tokensSize = m_tokens.size();
	while (m_current < tokensSize) {
		const Token& token = Peek();
		AstNode* target = nullptr;
		switch (token.m_type)
		{
		case TokenType::IMPORT:
			target = ParseImportStmt(node);
			break;
		case TokenType::FUNCTION:
			target = ParseFunctionDef(node);
			break;
		default:
			target = ParseStatement(node);
			break;
		}
		node->m_children.push_back(target);
	}
	return node;
}


AstNode* Parser::ParseImportStmt(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::IMPORT_STMT, parent);
	m_nodes.push_back(node);
	//import
	node->m_token = Peek();
	Consume();
	//string literal or identifier
	{
		const Token& token = Peek();
		if (!MatchAny({ TokenType::STRING_LITERAL ,TokenType::IDENTIFIER })) {
			throw ParserException(
				"Invalid import: '" + token.m_content + "', expected string literal or identifier after 'import'",
				token.m_line,
				token.m_column
			);
		}
		NodeType::Type childType = token.m_type == TokenType::STRING_LITERAL ? NodeType::LITERAL : NodeType::IDENTIFIER;
		AstNode* target = new AstNode(childType, token, node);
		m_nodes.push_back(target);
		node->m_children.push_back(target);
	}
	//;
	if (!Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after import statement",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	return node;
}

AstNode* Parser::ParseFunctionDef(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::FUNCTION_DEF, parent);
	m_nodes.push_back(node);
	//function
	node->m_token = Peek();
	Consume();
	//type
	node->m_children.push_back(ParseType(node));
	//identifier
	{
		const Token& token = Peek();
		if (!Match(TokenType::IDENTIFIER)) {
			throw ParserException(
				"Invalid function definition: '" + token.m_content + "', expected identifier after return type",
				token.m_line,
				token.m_column
			);
		}
		AstNode* target = new AstNode(NodeType::IDENTIFIER, token, node);
		node->m_children.push_back(target);
		m_nodes.push_back(target);
	}
	//(
	if (!Match(TokenType::LEFT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid function definition: '" + token.m_content + "', expected '(' after function name",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	//parameter_list
	if (!Check(TokenType::RIGHT_PAREN))
		node->m_children.push_back(ParseParameterList(node));
	//)
	if (!Match(TokenType::RIGHT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid function definition: '" + token.m_content + "', expected ')' after parameter list",
			token.m_line,
			token.m_column
		);
	}
	//compound_stmt
	node->m_children.push_back(ParseCompoundStmt(node));
	return node;
}

AstNode* Parser::ParseParameterList(AstNode* parent) {
	// parameter {, parameter}
	return ParseSeparatedListTemplate(NodeType::PARAMETER_LIST, parent, &Parser::ParseParameter);
}

AstNode* Parser::ParseParameter(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::PARAMETER, parent);
	m_nodes.push_back(node);
	//type
	node->m_children.push_back(ParseType(node));
	//declarator
	node->m_children.push_back(ParseDeclarator(node));
	return node;
}

//statement is a logical node
AstNode* Parser::ParseStatement(AstNode* parent) {
	const Token& token = Peek();
	AstNode* node = nullptr;

	switch (token.m_type)
	{
	case TokenType::LEFT_BRACE:
		node = ParseCompoundStmt(parent);
		break;
	case TokenType::STRUCT:
		node = ParseStructDefinition(parent);
		break;
	case TokenType::IF:
		node = ParseIfStmt(parent);
		break;
	case TokenType::SWITCH:
		node = ParseSwitchStmt(parent);
		break;
	case TokenType::WHILE:
		node = ParseWhileStmt(parent);
		break;
	case TokenType::FOR:
		node = ParseForStmt(parent);
		break;
	case TokenType::RETURN:
		node = ParseReturnStmt(parent);
		break;
	case TokenType::BREAK:
		node = ParseBreakStmt(parent);
		break;
	case TokenType::CONTINUE:
		node = ParseContinueStmt(parent);
		break;
	case TokenType::LET:
	case TokenType::CONST:
		node = ParseVariableDeclaration(parent);
		break;
	default:
		node = ParseExpressionStmt(parent);
		break;
	}

	return node;
}

AstNode* Parser::ParseCompoundStmt(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::COMPOUND_STMT, parent);
	m_nodes.push_back(node);
	//{
	if (!Match(TokenType::LEFT_BRACE)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid compound statement: '" + token.m_content + "', expected '{' at the beginning of compound statement",
			token.m_line,
			token.m_column
		);
	}
	//statement
	while (!Check(TokenType::RIGHT_BRACE)) {
		node->m_children.push_back(ParseStatement(node));
	}
	//}
	if (!Match(TokenType::RIGHT_BRACE)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid compound statement: '" + token.m_content + "', expected '}' at the end of compound statement",
			token.m_line,
			token.m_column
		);
	}

	return node;
}

//ExpressionStmt is a logical node
AstNode* Parser::ParseExpressionStmt(AstNode* parent) {
	AstNode* exprNode = nullptr;
	//expression
	if (!Check(TokenType::SEMICOLON)) {
		exprNode = ParseExpression(parent);
	}
	//;
	if (!Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after expression statement",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	//empty statement
	if (exprNode == nullptr) {
		exprNode = new AstNode(NodeType::EMPTY_STMT, parent);
		m_nodes.push_back(exprNode);
	}
	return exprNode;
}

AstNode* Parser::ParseVariableDeclaration(AstNode* parent, bool consumeSemicolo) {
	AstNode* node = new AstNode(NodeType::VAR_DECL, parent);
	m_nodes.push_back(node);
	//let or const
	const Token& token = Peek();
	if (!MatchAny({ TokenType::LET ,TokenType::CONST })) {
		throw ParserException(
			"Invalid variable declaration: '" + token.m_content + "', expected 'let' or 'const' at the beginning of variable declaration",
			token.m_line,
			token.m_column
		);
	}
	node->m_token = token;
	//type
	node->m_children.push_back(ParseType(node));
	//declarator_list
	node->m_children.push_back(ParseDeclaratorList(node));
	//;
	if (consumeSemicolo && !Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after variable declaration",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	return node;
}

AstNode* Parser::ParseDeclaratorList(AstNode* parent) {
	return ParseSeparatedListTemplate(NodeType::DECLARATOR_LIST, parent, &Parser::ParseDeclarator);
}

AstNode* Parser::ParseDeclarator(AstNode* parent) {
	// identifier
	const Token& token = Peek();
	if (!Match(TokenType::IDENTIFIER)) {
		throw ParserException(
			"Invalid declarator: expected identifier",
			token.m_line,
			token.m_column
		);
	}
	AstNode* identifier = new AstNode(NodeType::IDENTIFIER, token, parent);
	m_nodes.push_back(identifier);

	bool isComplex = false;
	std::vector<AstNode*> extraChildren;
	// array
	if (Check(TokenType::LEFT_SQUARE)) {
		isComplex = true;
		auto* arrayNode = new AstNode(NodeType::ARRAY, parent);
		m_nodes.push_back(arrayNode);
		while (Match(TokenType::LEFT_SQUARE)) {
			if (!Check(TokenType::RIGHT_SQUARE)) {
				arrayNode->m_children.push_back(ParseExpression(arrayNode));
			}
			else {
				arrayNode->m_children.push_back(new AstNode(NodeType::EMPTY_STMT, arrayNode));
			}

			if (!Match(TokenType::RIGHT_SQUARE)) {
				const Token& t = Peek();
				throw ParserException(
					"Invalid array declarator: expected ']' after '" + t.m_content + "'",
					t.m_line,
					t.m_column
				);
			}
		}

		extraChildren.push_back(arrayNode);
	}
	// initializer
	if (Match(TokenType::ASSIGN)) {
		isComplex = true;
		AstNode* initializer = ParseInitializer(nullptr);
		extraChildren.push_back(initializer);
	}
	// if complex (array or initializer), wrap in DECLARATOR
	if (isComplex) {
		AstNode* declarator = new AstNode(NodeType::DECLARATOR, parent);
		m_nodes.push_back(declarator);
		declarator->m_children.push_back(identifier);
		identifier->m_parent = declarator;
		for (auto* c : extraChildren) {
			declarator->m_children.push_back(c);
			c->m_parent = declarator;
		}
		return declarator;
	}
	// simple identifier only
	return identifier;
}


AstNode* Parser::ParseStructDefinition(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::STRUCT_DEF, parent);
	m_nodes.push_back(node);
	//struct
	node->m_token = Peek();
	Consume();
	//indentifier
	{
		const Token& token = Peek();
		if (!Match(TokenType::IDENTIFIER)) {
			throw ParserException(
				"Invalid struct definition: '" + token.m_content + "', expected identifier after 'struct'",
				token.m_line,
				token.m_column
			);
		}
		AstNode* target = new AstNode(NodeType::IDENTIFIER, token, node);
		node->m_children.push_back(target);
		m_nodes.push_back(target);
	}
	//{
	if (!Match(TokenType::LEFT_BRACE)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid struct definition: '" + token.m_content + "', expected '{' after struct name",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	//struct member declaration
	while (!Check(TokenType::RIGHT_BRACE)) {
		node->m_children.push_back(ParseStructDecaratorList(node));
	}
	//}
	if (!Match(TokenType::RIGHT_BRACE)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid struct definition: '" + token.m_content + "', expected '}' at the end of struct definition",
			token.m_line,
			token.m_column
		);
	}
	//;
	if (!Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after struct definition",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	return node;
}

//struct_member_declaration { struct_member_declaration }
AstNode* Parser::ParseStructDecaratorList(AstNode* parent) {
	//element, if only one node direct return
	AstNode* first = ParseStructMemberDeclaration(parent);
	if (Check(TokenType::RIGHT_BRACE))
		return first;
	//list parent
	AstNode* node = new AstNode(NodeType::STRUCT_DECLARATOR_LIST, parent);
	m_nodes.push_back(node);
	node->m_children.push_back(first);
	first->m_parent = node;
	//{ struct_member_declaration }
	while (!Check(TokenType::RIGHT_BRACE)) {
		node->m_children.push_back(ParseStructMemberDeclaration(node));
	}
	return node;
}

AstNode* Parser::ParseStructMemberDeclaration(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::STRUCT_MEMBER_DECL, parent);
	m_nodes.push_back(node);
	//type
	node->m_children.push_back(ParseType(node));
	//declarator_list
	node->m_children.push_back(ParseDeclaratorList(node));
	//;
	if (!Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after struct member declaration",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	return node;
}

AstNode* Parser::ParseIfStmt(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::IF_STMT, parent);
	m_nodes.push_back(node);
	//if
	node->m_token = Peek();
	Consume();
	// "("
	if (!Match(TokenType::LEFT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid if statement: expected '(' after 'if'",
			token.m_line,
			token.m_column
		);
	}
	//expression
	node->m_children.push_back(ParseExpression(node));
	// ")"
	if (!Match(TokenType::RIGHT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid if statement: expected ')' after condition expression",
			token.m_line,
			token.m_column
		);
	}
	// then
	AstNode* thenBranch = ParseStatement(node);
	node->m_children.push_back(thenBranch);
	//else
	if (Match(TokenType::ELSE)) {
		AstNode* elseBranch = ParseStatement(node);
		node->m_children.push_back(elseBranch);
	}
	return node;
}

AstNode* Parser::ParseSwitchStmt(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::SWITCH_STMT, parent);
	m_nodes.push_back(node);
	//switch
	node->m_token = Peek();
	Consume();
	// (
	if (!Match(TokenType::LEFT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid switch statement: expected '(' after 'switch'",
			token.m_line,
			token.m_column
		);
	}
	// statement
	node->m_children.push_back(ParseExpression(node));
	// )
	if (!Match(TokenType::RIGHT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid switch statement: expected ')' after switch expression",
			token.m_line,
			token.m_column
		);
	}
	//{
	if (!Match(TokenType::LEFT_BRACE)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid switch statement: expected '{' at the beginning of switch body",
			token.m_line,
			token.m_column
		);
	}
	//case
	while (Check(TokenType::CASE)) {
		node->m_children.push_back(ParseCaseClause(node));
	}
	//default
	if (Check(TokenType::DEFAULT)) {
		node->m_children.push_back(ParseDefaultClause(node));
	}
	//}
	if (!Match(TokenType::RIGHT_BRACE)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid switch statement: expected '}' at the ending of switch body",
			token.m_line,
			token.m_column
		);
	}
	return node;
}

AstNode* Parser::ParseCaseClause(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::CASE_CLAUSE, parent);
	m_nodes.push_back(node);
	//case
	node->m_token = Peek();
	Consume();
	//literal
	node->m_children.push_back(ParseLiteral(node));
	//:
	if (!Match(TokenType::COLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid case clause: '" + token.m_content + "', expected ':' after case literal",
			token.m_line,
			token.m_column
		);
	}
	//statement
	{
		while (!CheckAny({ TokenType::CASE, TokenType::DEFAULT, TokenType::RIGHT_BRACE }))
		{
			node->m_children.push_back(ParseStatement(node));
		}
	}
	return node;
}

AstNode* Parser::ParseDefaultClause(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::DEFAULT_CLAUSE, parent);
	m_nodes.push_back(node);
	//default
	node->m_token = Peek();
	Consume();
	//:
	if (!Match(TokenType::COLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid case clause: '" + token.m_content + "', expected ':' after case literal",
			token.m_line,
			token.m_column
		);
	}
	//statement
	{
		const Token* pToken = &Peek();
		while (pToken->m_type != TokenType::RIGHT_BRACE)
		{
			node->m_children.push_back(ParseStatement(node));
			pToken = &Peek();
		}
	}
	return node;
}

AstNode* Parser::ParseWhileStmt(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::WHILE_STMT, parent);
	m_nodes.push_back(node);
	//while
	node->m_token = Peek();
	Consume();
	// (
	if (!Match(TokenType::LEFT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid while statement: expected '(' after 'while'",
			token.m_line,
			token.m_column
		);
	}
	//expression
	node->m_children.push_back(ParseExpression(node));
	//)
	if (!Match(TokenType::RIGHT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid while statement: expected ')' after condition expression",
			token.m_line,
			token.m_column
		);
	}
	//statement
	node->m_children.push_back(ParseStatement(node));
	return node;
}

AstNode* Parser::ParseForStmt(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::FOR_STMT, parent);
	m_nodes.push_back(node);
	//for
	node->m_token = Peek();
	Consume();
	//(
	if (!Match(TokenType::LEFT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid for statement: expected '(' after 'for'",
			token.m_line,
			token.m_column
		);
	}
	//initialization
	if (!Check(TokenType::SEMICOLON)) {
		if (Check(TokenType::LET) || Check(TokenType::CONST))
			node->m_children.push_back(ParseVariableDeclaration(node, false));
		else
			node->m_children.push_back(ParseExpression(node));
	}
	//;
	if (!Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after for-loop initialization",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	//condition
	if (!Check(TokenType::SEMICOLON)) {
		node->m_children.push_back(ParseExpression(node));
	}
	//;
	if (!Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after for-loop condition",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	//increment
	if (!Check(TokenType::RIGHT_PAREN)) {
		node->m_children.push_back(ParseExpression(node));
	}
	//)
	if (!Match(TokenType::RIGHT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid for statement: expected ')' after for-loop increment",
			token.m_line,
			token.m_column
		);
	}
	//statement
	node->m_children.push_back(ParseStatement(node));
	return node;
}

AstNode* Parser::ParseReturnStmt(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::RETURN_STMT, parent);
	m_nodes.push_back(node);
	//return
	node->m_token = Peek();
	Consume();
	//expression
	if (!Check(TokenType::SEMICOLON)) {
		node->m_children.push_back(ParseExpression(node));
	}
	//;
	if (!Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after return statement",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	return node;
}

AstNode* Parser::ParseBreakStmt(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::BREAK_STMT, parent);
	m_nodes.push_back(node);
	//break
	node->m_token = Peek();
	Consume();
	//;
	if (!Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after break statement",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	return node;
}

AstNode* Parser::ParseContinueStmt(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::CONTINUE_STMT, parent);
	m_nodes.push_back(node);
	//continue
	node->m_token = Peek();
	Consume();
	//;
	if (!Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after continue statement",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	return node;
}

AstNode* Parser::ParseExpression(AstNode* parent) {
	//assignment {, assignment}
	return ParseSeparatedListTemplate(NodeType::EXPRESSION, parent, &Parser::ParseAssignment);
}

AstNode* Parser::ParseAssignment(AstNode* parent) {
	AstNode* left = ParseConditional(parent);
	//assignment_operator assignment
	if (CheckAny({ TokenType::ASSIGN,TokenType::SELF_ADD,TokenType::SELF_SUB,
			TokenType::SELF_MUL ,TokenType::SELF_DIV,TokenType::SELF_MODULO }))
	{
		AstNode* node = new AstNode(NodeType::ASSIGN_EXPR, parent);
		m_nodes.push_back(node);
		//=, +=, -=, *=, /=, %=
		node->m_token = Peek();
		Consume();
		node->m_children.push_back(left);
		left->m_parent = node;
		//assignment
		node->m_children.push_back(ParseAssignment(node));
		return node;
	}
	return left;
}

AstNode* Parser::ParseConditional(AstNode* parent) {
	//logical_or
	AstNode* condition = ParseLogicalOr(parent);
	//? expression : conditional
	if (Check(TokenType::QUESTION)) {
		AstNode* node = new AstNode(NodeType::CONDITIONAL_EXPR, parent);
		m_nodes.push_back(node);
		//?
		node->m_token = Peek();
		Consume();
		//condition
		node->m_children.push_back(condition);
		condition->m_parent = node;
		//true
		node->m_children.push_back(ParseExpression(node));
		//:
		if (!Match(TokenType::COLON)) {
			const Token& token = Peek();
			throw ParserException(
				"Invalid conditional expression: '" + token.m_content + "', expected ':' after true expression",
				token.m_line,
				token.m_column
			);
		}
		//false
		node->m_children.push_back(ParseConditional(node));
		return node;
	}
	return condition;
}

//binary expr : lower {ops lower}  left associative
AstNode* Parser::ParseBinary(AstNode* parent, std::function<AstNode* (AstNode*)> lower, const std::vector<TokenType::Type>& ops) {
	AstNode* left = lower(parent);
	while (std::any_of(ops.begin(), ops.end(), [&](TokenType::Type t) { return Check(t); })) {
		AstNode* node = new AstNode(NodeType::BINARY_EXPR, parent);
		m_nodes.push_back(node);
		node->m_token = Peek();
		Consume();
		left->m_parent = node;
		node->m_children.push_back(left);
		node->m_children.push_back(lower(node));
		left = node;
	}
	return left;
}

AstNode* Parser::ParseLogicalOr(AstNode* parent) {
	//logical_and {|| logical_and}
	return ParseBinary(parent, std::bind(&Parser::ParseLogicalAnd, this, std::placeholders::_1), { TokenType::OR });
}

AstNode* Parser::ParseLogicalAnd(AstNode* parent) {
	//bit_or {&& bit_or}
	return ParseBinary(parent, std::bind(&Parser::ParseBitOr, this, std::placeholders::_1), { TokenType::AND });
}

AstNode* Parser::ParseBitOr(AstNode* parent) {
	//bit_xor {| bit_xor}
	return ParseBinary(parent, std::bind(&Parser::ParseBitXOr, this, std::placeholders::_1), { TokenType::BIT_OR });
}

AstNode* Parser::ParseBitXOr(AstNode* parent) {
	//bit_and {^ bit_and}
	return ParseBinary(parent, std::bind(&Parser::ParseBitAnd, this, std::placeholders::_1), { TokenType::XOR });
}

AstNode* Parser::ParseBitAnd(AstNode* parent) {
	//equality { & equality }
	return ParseBinary(parent, std::bind(&Parser::ParseEquality, this, std::placeholders::_1), { TokenType::BIT_AND });
}

AstNode* Parser::ParseEquality(AstNode* parent) {
	//relational { (== | !=) relational }
	return ParseBinary(parent, std::bind(&Parser::ParseRelational, this, std::placeholders::_1), { TokenType::EQUAL, TokenType::NOT_EQUAL });
}

AstNode* Parser::ParseRelational(AstNode* parent) {
	//shift {(< | > | <= | >=) shift}
	return ParseBinary(parent, std::bind(&Parser::ParseShift, this, std::placeholders::_1),
		{ TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQUAL, TokenType::GREATER_EQUAL });
}

AstNode* Parser::ParseShift(AstNode* parent) {
	//additive {(<< | >>) additive}
	return ParseBinary(parent, std::bind(&Parser::ParseAdditive, this, std::placeholders::_1),
		{ TokenType::LEFT_MOVE, TokenType::RIGHT_MOVE });
}

AstNode* Parser::ParseAdditive(AstNode* parent) {
	//multiplicative {(+ | -) multiplicative}
	return ParseBinary(parent, std::bind(&Parser::ParseMultiplicative, this, std::placeholders::_1),
		{ TokenType::ADD, TokenType::SUBTRACT });
}

AstNode* Parser::ParseMultiplicative(AstNode* parent) {
	//unary {(* | / | %) unary}
	return ParseBinary(parent, std::bind(&Parser::ParseUnary, this, std::placeholders::_1),
		{ TokenType::MULTIPLY, TokenType::DIVIDE, TokenType::MODULO });
}

AstNode* Parser::ParseUnary(AstNode* parent) {
	std::vector<Token> ops;
	while (CheckAny({ TokenType::ADD ,TokenType::SUBTRACT,TokenType::NOT,
		TokenType::BIT_NOT,TokenType::SELF_ADD,TokenType::SELF_SUB }))
	{
		ops.push_back(Peek());
		Consume();
	}
	//right associative
	AstNode* node = ParsePostfix(parent);
	for (auto it = ops.rbegin(); it != ops.rend(); ++it) {
		AstNode* unary = new AstNode(NodeType::UNARY_EXPR, parent);
		m_nodes.push_back(unary);
		unary->m_token = *it;
		unary->m_children.push_back(node);
		node->m_parent = unary;
		node = unary;
	}
	return node;
}


AstNode* Parser::ParsePostfix(AstNode* parent) {
	//primary
	AstNode* current = ParsePrimary(parent);

	// (++ | -- | . identifier | [ expression ] | ( [ argument_list ] ))
	while (CheckAny({ TokenType::INCREMENT,TokenType::DECREMENT,
		TokenType::DOT,TokenType::LEFT_SQUARE,TokenType::LEFT_PAREN }))
	{
		const Token& token = Peek();

		if (MatchAny({ TokenType::INCREMENT ,TokenType::DECREMENT })) {
			//++ --
			AstNode* postfix = new AstNode(NodeType::POSTFIX_EXPR, parent);
			m_nodes.push_back(postfix);
			postfix->m_token = token;
			postfix->m_children.push_back(current);
			current->m_parent = postfix;
			// left associative
			current = postfix;
		}
		else if (Match(TokenType::DOT)) {
			// . identifier
			AstNode* member = new AstNode(NodeType::MEMBER_EXPR, parent);
			m_nodes.push_back(member);
			member->m_token = token;
			member->m_children.push_back(current);
			current->m_parent = member;
			// identifier
			const Token& idToken = Peek();
			if (!Match(TokenType::IDENTIFIER)) {
				throw ParserException(
					"Invalid member access: expected identifier after '.'",
					idToken.m_line,
					idToken.m_column
				);
			}
			AstNode* idNode = new AstNode(NodeType::IDENTIFIER, idToken, member);
			member->m_children.push_back(idNode);
			m_nodes.push_back(idNode);
			// left associative
			current = member;
		}
		else if (Match(TokenType::LEFT_SQUARE)) {
			//[ expression ]
			AstNode* index = new AstNode(NodeType::INDEX_EXPR, parent);
			m_nodes.push_back(index);
			index->m_children.push_back(current);
			current->m_parent = index;
			// expression
			index->m_children.push_back(ParseExpression(index));
			// ]
			if (!Match(TokenType::RIGHT_SQUARE)) {
				const Token& rbracketToken = Peek();
				throw ParserException(
					"Invalid index expression: '" + rbracketToken.m_content + "', expected ']' after index expression",
					rbracketToken.m_line,
					rbracketToken.m_column
				);
			}
			// left associative
			current = index;
		}
		else if (Match(TokenType::LEFT_PAREN)) {
			// ( [ argument_list ] )
			AstNode* call = new AstNode(NodeType::CALL_EXPR, parent);
			m_nodes.push_back(call);
			call->m_children.push_back(current);
			current->m_parent = call;
			// argument_list
			if (!Check(TokenType::RIGHT_PAREN)) {
				call->m_children.push_back(ParseArgumentList(call));
			}
			// )
			if (!Match(TokenType::RIGHT_PAREN)) {
				const Token& token = Peek();
				throw ParserException(
					"Invalid function call: expected ')' after argument list",
					token.m_line,
					token.m_column
				);
			}
			// left associative
			current = call;
		}
	}
	return current;
}

AstNode* Parser::ParsePrimary(AstNode* parent) {
	const Token& token = Peek();
	AstNode* node = nullptr;

	switch (token.m_type) {
	case TokenType::IDENTIFIER: {
		// identifier
		node = new AstNode(NodeType::IDENTIFIER, token, parent);
		m_nodes.push_back(node);
		Consume();
		break;
	}
	case TokenType::INT_LITERAL:
	case TokenType::DOUBLE_LITERAL:
	case TokenType::CHARACTER_LITERAL:
	case TokenType::STRING_LITERAL:
	case TokenType::BOOL_LITERAL:
	case TokenType::NULL_LITERAL: {
		// literal
		node = ParseLiteral(parent);
		break;
	}
	case TokenType::LEFT_PAREN: {
		// ( expression )
		Consume(); // consume '('
		node = new AstNode(NodeType::GROUP_EXPR, parent);
		m_nodes.push_back(node);

		// expression
		node->m_children.push_back(ParseExpression(node));

		// )
		if (!Match(TokenType::RIGHT_PAREN)) {
			const Token& t = Peek();
			throw ParserException(
				"Invalid grouping expression: expected ')' after expression",
				t.m_line,
				t.m_column
			);
		}
		break;
	}
	case TokenType::LAMBDA: { // function literal
		node = ParseFunctionLiteral(parent);
		break;
	}
	default: {
		throw ParserException(
			"Invalid primary expression: '" + token.m_content +
			"', expected identifier, literal, or '('",
			token.m_line,
			token.m_column
		);
	}
	}

	return node;
}

AstNode* Parser::ParseArgumentList(AstNode* parent) {
	//expression {, expression}
	return ParseSeparatedListTemplate(NodeType::ARGUMENT_LIST, parent, &Parser::ParseAssignment);
}

AstNode* Parser::ParseInitializer(AstNode* parent) {
	// {
	if (Match(TokenType::LEFT_BRACE)) {
		//{}
		if (Match(TokenType::RIGHT_BRACE)) {
			auto node = new AstNode(NodeType::INITIALIZER, parent);
			m_nodes.push_back(node);
			return node;
		}

		// designated_initializer_list
		AstNode* node = ParseDesignatedInitializerList(parent);

		// }
		if (!Match(TokenType::RIGHT_BRACE)) {
			const Token& token = Peek();
			throw ParserException(
				"Invalid initializer: expected '}' at end of initializer list",
				token.m_line,
				token.m_column
			);
		}

		return node;
	}

	// assignment
	return ParseAssignment(parent);
}

//designated_initializer { "," designated_initializer }
AstNode* Parser::ParseDesignatedInitializerList(AstNode* parent) {
	return ParseSeparatedListTemplate(NodeType::INITIALIZER, parent, &Parser::ParseDesignatedInitializer);
}

//[ "." identifier "=" ] initializer
AstNode* Parser::ParseDesignatedInitializer(AstNode* parent) {
	if (Check(TokenType::DOT)) {
		AstNode* node = new AstNode(NodeType::DESIGNATED_INITIALIZER, parent);
		m_nodes.push_back(node);
		//.
		node->m_token = Peek();
		Consume();
		//identifier
		{
			const Token& token = Peek();
			if (!Match(TokenType::IDENTIFIER)) {
				throw ParserException(
					"Invalid designated initializer: '" + token.m_content + "', expected identifier after '.'",
					token.m_line,
					token.m_column
				);
			}
			AstNode* idNode = new AstNode(NodeType::IDENTIFIER, token, node);
			node->m_children.push_back(idNode);
			m_nodes.push_back(idNode);
		}
		//=
		if (!Match(TokenType::ASSIGN)) {
			const Token& token = Peek();
			throw ParserException(
				"Invalid designated initializer: '" + token.m_content + "', expected '=' after identifier",
				token.m_line,
				token.m_column
			);
		}
		//initializer
		node->m_children.push_back(ParseInitializer(node));
		return node;
	}
	else {
		//initializer
		return ParseInitializer(parent);
	}
}

AstNode* Parser::ParseFunctionLiteral(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::FUNCTION_LITERAL, parent);
	m_nodes.push_back(node);
	//lambda
	if (!Match(TokenType::LAMBDA)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid function literal: '" + token.m_content + "', expected 'lambda' at the beginning of function literal",
			token.m_line,
			token.m_column
		);
	}
	//(
	if (!Match(TokenType::LEFT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid function literal: '" + token.m_content + "', expected '(' after 'lambda'",
			token.m_line,
			token.m_column
		);
	}
	//parameter_list
	if (!Check(TokenType::RIGHT_PAREN)) {
		node->m_children.push_back(ParseParameterList(node));
	}
	//)
	if (!Match(TokenType::RIGHT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid function literal: '" + token.m_content + "', expected ')' after parameter list",
			token.m_line,
			token.m_column
		);
	}
	//->
	if (!Match(TokenType::POINT_TO)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid function literal: '" + token.m_content + "', expected '->' after parameter list",
			token.m_line,
			token.m_column
		);
	}
	//type
	node->m_children.push_back(ParseType(node));
	//compound_stmt
	node->m_children.push_back(ParseCompoundStmt(node));
	return node;
}

AstNode* Parser::ParseLiteral(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::LITERAL, parent);
	m_nodes.push_back(node);
	const Token& token = Peek();
	//INT_LITERAL | DOUBLE_LITERAL | CHARACTER_LITERAL | STRING_LITERAL | BOOL_LITERAL| NULL_LITERAL
	if (!MatchAny({ TokenType::INT_LITERAL,TokenType::DOUBLE_LITERAL, TokenType::CHARACTER_LITERAL,
		TokenType::STRING_LITERAL,TokenType::BOOL_LITERAL,TokenType::NULL_LITERAL })) {
		throw ParserException(
			"Invalid literal: '" + token.m_content + "', expected literal but got identifier",
			token.m_line,
			token.m_column
		);
	}
	node->m_token = token;
	return node;
}

AstNode* Parser::ParseType(AstNode* parent) {
	AstNode* node = nullptr;
	const Token& token = Peek();
	switch (token.m_type) {
	case TokenType::IDENTIFIER:
		// user-defined type
		node = new AstNode(NodeType::IDENTIFIER, token, parent);
		m_nodes.push_back(node);
		Consume();
		break;
	case TokenType::INT:
	case TokenType::DOUBLE:
	case TokenType::CHAR:
	case TokenType::STRING:
	case TokenType::BOOL:
	case TokenType::VOID:
		// builtin type
		node = ParseBuiltinType(parent);
		break;
	case TokenType::LEFT_PAREN:
		// function type
		node = ParseFunctionType(parent);
		break;
	default:
		throw ParserException(
			"Invalid type: '" + token.m_content + "', expected type name, builtin type, or '(' for function type",
			token.m_line,
			token.m_column
		);
	}
	return node;
}


AstNode* Parser::ParseBuiltinType(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::BUILTIN_TYPE, parent);
	m_nodes.push_back(node);
	//int | double | char | string | bool | void
	node->m_token = Peek();
	MatchAny({ TokenType::INT,TokenType::DOUBLE,TokenType::CHAR,
		TokenType::STRING,TokenType::BOOL,TokenType::VOID });
	return node;
}

AstNode* Parser::ParseFunctionType(AstNode* parent) {
	AstNode* node = new AstNode(NodeType::FUNCTION_TYPE, parent);
	m_nodes.push_back(node);
	//(
	if (!Match(TokenType::LEFT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid function type: '" + token.m_content + "', expected '(' at the beginning of function type",
			token.m_line,
			token.m_column
		);
	}
	//parameter_type_list
	if (!Check(TokenType::RIGHT_PAREN)) {
		node->m_children.push_back(ParseParameterTypeList(node));
	}
	//)
	if (!Match(TokenType::RIGHT_PAREN)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid function type: '" + token.m_content + "', expected ')' after parameter type list",
			token.m_line,
			token.m_column
		);
	}
	//->
	if (!Match(TokenType::POINT_TO)) {
		const Token& token = Peek();
		throw ParserException(
			"Invalid function type: '" + token.m_content + "', expected '->' after parameter type list",
			token.m_line,
			token.m_column
		);
	}
	//type
	node->m_children.push_back(ParseType(node));
	return node;
}

AstNode* Parser::ParseParameterTypeList(AstNode* parent) {
	//type,{, type}
	return ParseSeparatedListTemplate(NodeType::PARAMETER_TYPE_LIST, parent, &Parser::ParseType);
}

void Parser::PrintAstTree(AstNode* node, int depth) {
	if (!node)
		return;
	std::string indent(depth * 2, ' ');
	std::cout << indent << NodeTypeToString(node->m_type);
	if (node->m_token.m_content != "")
		std::cout << " (" << node->m_token.m_content << ")";
	std::cout << std::endl;
	for (AstNode* child : node->m_children) {
		PrintAstTree(child, depth + 1);
	}
}