

#include "Parser.h"
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

Parser::~Parser() {
	ClearNodes();
}

AstNode* Parser::Parse(const std::string& str) {
	m_tokens.clear();
	ClearNodes();
	m_tokens = Lexer::Instance().Tokenize(str);
	PreprocessTokens(m_tokens);
	m_current = 0;
	m_root = ParseProgram(nullptr);
	return m_root;
}

AstNode* Parser::Parse(const std::vector<Token>& tokens) {
	m_tokens.clear();
	ClearNodes();
	m_tokens = tokens;
	PreprocessTokens(m_tokens);
	m_current = 0;
	m_root = ParseProgram(nullptr);
	return m_root;
}

ProgramNode* Parser::ParseProgram(AstNode* parent) {
	ProgramNode* node = new ProgramNode(parent);
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
			target = ParseFunctionDecl(node);
			break;
		default:
			target = ParseStatement(node);
			break;
		}
		node->m_declarations.push_back(target);
	}
	return node;
}


ImportNode* Parser::ParseImportStmt(AstNode* parent) {
	ImportNode* node = new ImportNode(parent);
	m_nodes.push_back(node);
	//import
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
		node->m_isStringLiteral = token.m_type == TokenType::STRING_LITERAL ? true : false;
		node->m_moduleName = token.m_content;
		node->m_column = token.m_column;
		node->m_line = token.m_line;
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

FunctionDeclNode* Parser::ParseFunctionDecl(AstNode* parent) {
	FunctionDeclNode* node = new FunctionDeclNode(parent);
	m_nodes.push_back(node);
	//function
	Consume();
	//type
	node->m_returnType = ParseType(node);
	//identifier
	{
		const Token& token = Peek();
		if (!Check(TokenType::IDENTIFIER)) {
			throw ParserException(
				"Invalid function definition: '" + token.m_content + "', expected identifier after return type",
				token.m_line,
				token.m_column
			);
		}
		node->m_name = ParseIdentifier(node);
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
	while (!Check(TokenType::RIGHT_PAREN)) {
		node->m_params.push_back(ParseParameter(node));
		if (Match(TokenType::COMMA))
			continue;
		else
			break;
	}
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
	node->m_body = ParseCompoundStmt(node);
	return node;
}

StatementNode* Parser::ParseStatement(AstNode* parent) {
	const Token& token = Peek();
	StatementNode* node = nullptr;

	switch (token.m_type)
	{
	case TokenType::LEFT_BRACE:
		node = ParseCompoundStmt(parent);
		break;
	case TokenType::STRUCT:
		node = ParseStructDeclaration(parent);
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

CompoundStmtNode* Parser::ParseCompoundStmt(AstNode* parent) {
	CompoundStmtNode* node = new CompoundStmtNode(parent);
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
		node->m_statements.push_back(ParseStatement(node));
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

ExpressionStmtNode* Parser::ParseExpressionStmt(AstNode* parent) {
	ExpressionStmtNode* exprNode = new ExpressionStmtNode(parent);
	m_nodes.push_back(exprNode);
	//expression
	if (!Check(TokenType::SEMICOLON)) {
		exprNode->m_expression = ParseCommaExpression(exprNode);
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
	return exprNode;
}

VariableDeclNode* Parser::ParseVariableDeclaration(AstNode* parent, bool consumeSemicol) {
	VariableDeclNode* node = new VariableDeclNode(parent);
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
	node->m_isConst = token.m_type == TokenType::CONST ? true : false;
	node->m_column = token.m_column;
	node->m_line = token.m_line;
	//type
	node->m_type = ParseType(node);
	//declarator_list
	while (true) {
		node->m_declarators.push_back(ParseDeclarator(node));
		if (Match(TokenType::COMMA))
			continue;
		else
			break;
	}
	//;
	if (consumeSemicol && !Match(TokenType::SEMICOLON)) {
		const Token& token = Peek();
		throw ParserException(
			"Missing semicolon ';' after variable declaration",
			token.m_line,
			token.m_column + token.m_content.size()
		);
	}
	return node;
}

StructDeclNode* Parser::ParseStructDeclaration(AstNode* parent) {
	StructDeclNode* node = new StructDeclNode(parent);
	m_nodes.push_back(node);
	//struct
	Consume();
	//indentifier
	{
		const Token& token = Peek();
		if (!Check(TokenType::IDENTIFIER)) {
			throw ParserException(
				"Invalid struct definition: '" + token.m_content + "', expected identifier after 'struct'",
				token.m_line,
				token.m_column
			);
		}
		node->m_name = ParseIdentifier(node);
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
		node->m_members.push_back(ParseStructMemberDeclaration(node));
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

IfStmtNode* Parser::ParseIfStmt(AstNode* parent) {
	IfStmtNode* node = new IfStmtNode(parent);
	m_nodes.push_back(node);
	//if
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
	node->m_condition = ParseCommaExpression(node);
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
	node->m_thenStmt = ParseStatement(node);
	//else
	if (Match(TokenType::ELSE)) {
		node->m_elseStmt = ParseStatement(node);
	}
	return node;
}

SwitchStmtNode* Parser::ParseSwitchStmt(AstNode* parent) {
	SwitchStmtNode* node = new SwitchStmtNode(parent);
	m_nodes.push_back(node);
	//switch
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
	node->m_condition = ParseCommaExpression(node);
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
		node->m_cases.push_back(ParseCaseClause(node));
	}
	//default
	if (Check(TokenType::DEFAULT)) {
		node->m_default = ParseDefaultClause(node);
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

CaseNode* Parser::ParseCaseClause(AstNode* parent) {
	CaseNode* node = new CaseNode(parent);
	m_nodes.push_back(node);
	//case
	Consume();
	//literal
	node->m_literal = ParseLiteral(node);
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
		while (!CheckAny({ TokenType::CASE, TokenType::DEFAULT,TokenType::RIGHT_BRACE }))
		{
			node->m_statements.push_back(ParseStatement(node));
		}
	}
	return node;
}

DefaultNode* Parser::ParseDefaultClause(AstNode* parent) {
	DefaultNode* node = new DefaultNode(parent);
	m_nodes.push_back(node);
	//default
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
			node->m_statements.push_back(ParseStatement(node));
			pToken = &Peek();
		}
	}
	return node;
}

WhileStmtNode* Parser::ParseWhileStmt(AstNode* parent) {
	WhileStmtNode* node = new WhileStmtNode(parent);
	m_nodes.push_back(node);
	//while
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
	node->m_condition = ParseCommaExpression(node);
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
	node->m_body = ParseStatement(node);
	return node;
}

ForStmtNode* Parser::ParseForStmt(AstNode* parent) {
	ForStmtNode* node = new ForStmtNode(parent);
	m_nodes.push_back(node);
	//for
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
		if (CheckAny({ TokenType::LET,TokenType::CONST })) {
			node->m_init = ParseVariableDeclaration(node, false);
		}
		else
			node->m_init = ParseCommaExpression(node);
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
		node->m_condition = ParseCommaExpression(node);
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
		node->m_increment = ParseCommaExpression(node);
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
	node->m_body = ParseStatement(node);
	return node;
}

ReturnStmtNode* Parser::ParseReturnStmt(AstNode* parent) {
	ReturnStmtNode* node = new ReturnStmtNode(parent);
	m_nodes.push_back(node);
	//return
	Consume();
	//expression
	if (!Check(TokenType::SEMICOLON)) {
		node->m_expression = ParseCommaExpression(node);
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

BreakStmtNode* Parser::ParseBreakStmt(AstNode* parent) {
	BreakStmtNode* node = new BreakStmtNode(parent);
	m_nodes.push_back(node);
	//break
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

ContinueStmtNode* Parser::ParseContinueStmt(AstNode* parent) {
	ContinueStmtNode* node = new ContinueStmtNode(parent);
	m_nodes.push_back(node);
	//continue
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

ExpressionNode* Parser::ParseCommaExpression(AstNode* parent) {
	//assignment {, assignment}
	ExpressionNode* first = ParseAssignment(parent);
	if (!Check(TokenType::COMMA))
		return first;
	//list parent
	CommaExprNode* node = new CommaExprNode(parent);
	m_nodes.push_back(node);
	node->m_expressions.push_back(first);
	first->m_parent = node;
	//splitToken element
	while (Match(TokenType::COMMA)) {
		node->m_expressions.push_back(ParseAssignment(node));
	}
	return node;
}

ExpressionNode* Parser::ParseAssignment(AstNode* parent) {
	ExpressionNode* left = ParseConditional(parent);
	//assignment_operator assignment
	if (CheckAny({ TokenType::ASSIGN,TokenType::SELF_ADD,TokenType::SELF_SUB,
			TokenType::SELF_MUL ,TokenType::SELF_DIV,TokenType::SELF_MODULO }))
	{
		AssignmentExprNode* node = new AssignmentExprNode(parent);
		m_nodes.push_back(node);
		//=, +=, -=, *=, /=, %=
		const Token& token = Peek();
		node->m_op = token.m_content;
		node->m_column = token.m_column;
		node->m_line = token.m_line;
		Consume();
		//left
		node->m_left = left;
		left->m_parent = node;
		//assignment
		node->m_right = ParseAssignment(node);
		return node;
	}
	return left;
}

ExpressionNode* Parser::ParseConditional(AstNode* parent) {
	//logical_or
	ExpressionNode* condition = ParseLogicalOr(parent);
	//? expression : conditional
	if (Check(TokenType::QUESTION)) {
		ConditionalExprNode* node = new ConditionalExprNode(parent);
		m_nodes.push_back(node);
		//?
		Consume();
		//condition
		node->m_condition = condition;
		condition->m_parent = node;
		//true
		node->m_trueExpr = ParseCommaExpression(node);
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
		node->m_falseExpr = ParseCommaExpression(node);
		return node;
	}
	return condition;
}

ExpressionNode* Parser::ParseBinary(AstNode* parent, std::function<ExpressionNode* (AstNode*)> lower, const std::vector<TokenType::Type>& ops)
{
	ExpressionNode* left = lower(parent);

	while (CheckAny(ops))
	{
		Token op = Peek();
		Consume();
		ExpressionNode* right = lower(parent);

		BinaryExprNode* node = new BinaryExprNode(parent);
		m_nodes.push_back(node);
		node->m_left = left;
		left->m_parent = node;
		node->m_op = op.m_content;
		node->m_column = op.m_column;
		node->m_line = op.m_line;
		node->m_right = right;
		right->m_parent = node;
		left = node;
	}
	return left;
}

ExpressionNode* Parser::ParseLogicalOr(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseLogicalAnd, this, std::placeholders::_1), { TokenType::OR });
}

ExpressionNode* Parser::ParseLogicalAnd(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseBitOr, this, std::placeholders::_1), { TokenType::AND });
}

ExpressionNode* Parser::ParseBitOr(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseBitXOr, this, std::placeholders::_1), { TokenType::BIT_OR });
}

ExpressionNode* Parser::ParseBitXOr(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseBitAnd, this, std::placeholders::_1), { TokenType::XOR });
}

ExpressionNode* Parser::ParseBitAnd(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseEquality, this, std::placeholders::_1), { TokenType::BIT_AND });
}

ExpressionNode* Parser::ParseEquality(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseRelational, this, std::placeholders::_1), { TokenType::EQUAL, TokenType::NOT_EQUAL });
}

ExpressionNode* Parser::ParseRelational(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseShift, this, std::placeholders::_1),
		{ TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQUAL, TokenType::GREATER_EQUAL });
}

ExpressionNode* Parser::ParseShift(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseAdditive, this, std::placeholders::_1),
		{ TokenType::LEFT_MOVE, TokenType::RIGHT_MOVE });
}

ExpressionNode* Parser::ParseAdditive(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseMultiplicative, this, std::placeholders::_1),
		{ TokenType::ADD, TokenType::SUBTRACT });
}

ExpressionNode* Parser::ParseMultiplicative(AstNode* parent) {
	return ParseBinary(parent, std::bind(&Parser::ParseUnary, this, std::placeholders::_1),
		{ TokenType::MULTIPLY, TokenType::DIVIDE, TokenType::MODULO });
}

ExpressionNode* Parser::ParseUnary(AstNode* parent) {
	std::vector<Token> ops;
	while (CheckAny({ TokenType::ADD ,TokenType::SUBTRACT,TokenType::NOT,
		TokenType::BIT_NOT,TokenType::SELF_ADD,TokenType::SELF_SUB }))
	{
		ops.push_back(Peek());
		Consume();
	}
	//right associative
	ExpressionNode* node = ParsePostfix(parent);
	for (auto it = ops.rbegin(); it != ops.rend(); ++it) {
		UnaryExprNode* unary = new UnaryExprNode(parent);
		m_nodes.push_back(unary);
		unary->m_op = it->m_content;
		unary->m_column = it->m_column;
		unary->m_line = it->m_line;
		unary->m_operand = node;
		node->m_parent = unary;
		node = unary;
	}
	return node;
}

ExpressionNode* Parser::ParsePostfix(AstNode* parent) {
	//primary
	ExpressionNode* current = ParsePrimary(parent);

	// (++ | -- | . identifier | [ expression ] | ( [ argument_list ] ))
	while (CheckAny({ TokenType::INCREMENT,TokenType::DECREMENT,
		TokenType::DOT,TokenType::LEFT_SQUARE,TokenType::LEFT_PAREN }))
	{
		const Token& token = Peek();

		if (MatchAny({ TokenType::INCREMENT ,TokenType::DECREMENT })) {
			//++ --
			PostfixExprNode* postfix = new PostfixExprNode(parent);
			m_nodes.push_back(postfix);
			postfix->m_op = token.m_content;
			postfix->m_column = token.m_column;
			postfix->m_line = token.m_line;
			postfix->m_primary = current;
			current->m_parent = postfix;
			// left associative
			current = postfix;
		}
		else if (Match(TokenType::DOT)) {
			// . identifier
			MemberAccessNode* member = new MemberAccessNode(parent);
			m_nodes.push_back(member);
			member->m_object = current;
			current->m_parent = member;
			// identifier
			const Token& idToken = Peek();
			if (!Check(TokenType::IDENTIFIER)) {
				throw ParserException(
					"Invalid member access: expected identifier after '.'",
					idToken.m_line,
					idToken.m_column
				);
			}
			member->m_memberName = ParseIdentifier(member);
			// left associative
			current = member;
		}
		else if (Match(TokenType::LEFT_SQUARE)) {
			//[ expression ]
			ArrayIndexNode* index = new ArrayIndexNode(parent);
			m_nodes.push_back(index);
			index->m_array = current;
			current->m_parent = index;
			// expression
			index->m_index = ParseCommaExpression(index);
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
			FunctionCallNode* call = new FunctionCallNode(parent);
			m_nodes.push_back(call);
			call->m_callee = current;
			current->m_parent = call;
			// argument_list
			while (!Check(TokenType::RIGHT_PAREN)) {
				call->m_arguments.push_back(ParseAssignment(call));
				if (Match(TokenType::COMMA))
					continue;
				else
					break;
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

ExpressionNode* Parser::ParsePrimary(AstNode* parent) {
	const Token& token = Peek();
	ExpressionNode* node = nullptr;

	switch (token.m_type) {
	case TokenType::IDENTIFIER: {
		// identifier
		node = new IdentifierNode(token, parent);
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

		// expression
		node = ParseCommaExpression(node);

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

FunctionLiteralNode* Parser::ParseFunctionLiteral(AstNode* parent) {
	FunctionLiteralNode* node = new FunctionLiteralNode(parent);
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
	while (!Check(TokenType::RIGHT_PAREN)) {
		node->m_params.push_back(ParseParameter(node));
		if (Match(TokenType::COMMA))
			continue;
		else
			break;
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
	node->m_returnType = ParseType(node);
	//compound_stmt
	node->m_body = ParseCompoundStmt(node);
	return node;
}

IdentifierNode* Parser::ParseIdentifier(AstNode* parent) {
	const Token& token = Peek();
	if (!Match(TokenType::IDENTIFIER)) {
		throw ParserException(
			"Invalid identifier: expected identifier",
			token.m_line,
			token.m_column
		);
	}
	IdentifierNode* node = new IdentifierNode(token, parent);
	m_nodes.push_back(node);
	return node;
}

LiteralNode* Parser::ParseLiteral(AstNode* parent) {
	const Token& token = Peek();
	if (!MatchAny({ TokenType::INT_LITERAL,TokenType::DOUBLE_LITERAL, TokenType::CHARACTER_LITERAL,
	TokenType::STRING_LITERAL,TokenType::BOOL_LITERAL,TokenType::NULL_LITERAL })) {
		throw ParserException(
			"Invalid literal: '" + token.m_content + "', expected literal but got identifier",
			token.m_line,
			token.m_column
		);
	}
	LiteralNode* node = new LiteralNode(token, parent);
	m_nodes.push_back(node);
	return node;
}

ParameterNode* Parser::ParseParameter(AstNode* parent) {
	ParameterNode* node = new ParameterNode(parent);
	m_nodes.push_back(node);
	//type
	node->m_type = ParseType(node);
	//declarator
	node->m_declarator = ParseDeclarator(node);
	return node;
}

DeclaratorNode* Parser::ParseDeclarator(AstNode* parent) {
	// identifier
	DeclaratorNode* declarator = new DeclaratorNode(parent);
	m_nodes.push_back(declarator);
	const Token& token = Peek();
	if (!Check(TokenType::IDENTIFIER)) {
		throw ParserException(
			"Invalid declarator: expected identifier",
			token.m_line,
			token.m_column
		);
	}
	declarator->m_name = ParseIdentifier(declarator);

	// array
	while (Match(TokenType::LEFT_SQUARE)) {
		declarator->m_arraySizes.push_back(ParseCommaExpression(declarator));
		if (!Match(TokenType::RIGHT_SQUARE)) {
			const Token& t = Peek();
			throw ParserException(
				"Invalid array declarator: expected ']' after '" + t.m_content + "'",
				t.m_line,
				t.m_column
			);
		}
	}
	// initializer
	if (Match(TokenType::ASSIGN)) {
		declarator->m_initializer = ParseInitializer(declarator);
	}
	return declarator;
}

StructMemberNode* Parser::ParseStructMemberDeclaration(AstNode* parent) {
	StructMemberNode* node = new StructMemberNode(parent);
	m_nodes.push_back(node);
	//type
	node->m_type = ParseType(node);
	//declarator_list
	while (true) {
		node->m_declarators.push_back(ParseDeclarator(node));
		if (Match(TokenType::COMMA))
			continue;
		else
			break;
	}
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

ExpressionNode* Parser::ParseInitializer(AstNode* parent) {
	if (!Check(TokenType::LEFT_BRACE)) {
		return ParseAssignment(parent);
	}

	InitializerNode* node = new InitializerNode(parent);
	m_nodes.push_back(node);
	//{
	Match(TokenType::LEFT_BRACE);
	//{}
	if (Match(TokenType::RIGHT_BRACE)) {
		return node;
	}

	while (true) {
		node->m_values.push_back(ParseInitializer(node));
		if (Match(TokenType::COMMA)) {
			continue;
		}
		if (Match(TokenType::RIGHT_BRACE)) {
			break;
		}
		const Token& token = Peek();
		throw ParserException(
			"Invalid initializer: expected ',' or '}'",
			token.m_line,
			token.m_column
		);
	}
	return node;
}

TypeNode* Parser::ParseType(AstNode* parent) {
	TypeNode* node = nullptr;
	const Token& token = Peek();
	switch (token.m_type) {
	case TokenType::IDENTIFIER:
		// user-defined type
		node = new NamedTypeNode(token, parent);
		m_nodes.push_back(node);
		Consume();
		break;
	case TokenType::INT:
	case TokenType::DOUBLE:
	case TokenType::CHAR:
	case TokenType::STRING:
	case TokenType::BOOL:
	case TokenType::VOID:
		// bultin type
		node = new BuiltinTypeNode(token, parent);
		m_nodes.push_back(node);
		Consume();
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

TypeNode* Parser::ParseFunctionType(AstNode* parent) {
	FunctionTypeNode* node = new FunctionTypeNode(parent);
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
	while (!Check(TokenType::RIGHT_PAREN)) {
		node->m_paramTypes.push_back(ParseType(node));
		if (Match(TokenType::COMMA))
			continue;
		else
			break;
	}
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
	node->m_returnType = ParseType(node);
	return node;
}

void AstPrinter::Visit(ProgramNode& node) {
	PrintIndent(m_depth);
	std::cout << "Program\n";
	++m_depth;
	for (auto* decl : node.m_declarations)
		if (decl) decl->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(ImportNode& node) {
	PrintIndent(m_depth);
	++m_depth;
	std::cout << "Import: " << node.m_moduleName << "\n";
}

void AstPrinter::Visit(FunctionDeclNode& node) {
	PrintIndent(m_depth);
	std::cout << "FunctionDecl: " << node.m_name->m_name << "\n";
	++m_depth;
	if (node.m_returnType) node.m_returnType->Accept(*this);
	for (auto* param : node.m_params)
		if (param) param->Accept(*this);
	--m_depth;
	++m_depth;
	if (node.m_body)
		node.m_body->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(CompoundStmtNode& node) {
	PrintIndent(m_depth);
	std::cout << "CompoundStmt\n";
	++m_depth;
	for (auto* stmt : node.m_statements)
		if (stmt) stmt->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(ExpressionStmtNode& node) {
	PrintIndent(m_depth);
	std::cout << "ExpressionStmt\n";
	++m_depth;
	if (node.m_expression)
		node.m_expression->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(VariableDeclNode& node) {
	PrintIndent(m_depth);
	std::cout << "VariableDecl: " << (node.m_isConst ? "const" : "let") << "\n";
	++m_depth;
	if (node.m_type)
		node.m_type->Accept(*this);
	for (auto* declarator : node.m_declarators)
		if (declarator) declarator->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(StructDeclNode& node) {
	PrintIndent(m_depth);
	std::cout << "StructDecl: " << node.m_name->m_name << "\n";
	++m_depth;
	for (auto* member : node.m_members)
		if (member) member->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(IfStmtNode& node) {
	PrintIndent(m_depth);
	std::cout << "IfStmt\n";
	++m_depth;
	if (node.m_condition) {
		node.m_condition->Accept(*this);
	}
	if (node.m_thenStmt) {
		node.m_thenStmt->Accept(*this);
	}
	if (node.m_elseStmt) {
		node.m_elseStmt->Accept(*this);
	}
	--m_depth;
}

void AstPrinter::Visit(SwitchStmtNode& node) {
	PrintIndent(m_depth);
	std::cout << "SwitchStmt\n";
	++m_depth;
	if (node.m_condition) {
		node.m_condition->Accept(*this);
	}
	for (auto* caseClause : node.m_cases)
		if (caseClause) caseClause->Accept(*this);
	if (node.m_default)
		node.m_default->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(CaseNode& node) {
	PrintIndent(m_depth);
	std::cout << "Case\n";
	++m_depth;
	if (node.m_literal)
		node.m_literal->Accept(*this);
	for (auto* stmt : node.m_statements)
		if (stmt) stmt->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(DefaultNode& node) {
	PrintIndent(m_depth);
	std::cout << "Default\n";
	++m_depth;
	for (auto* stmt : node.m_statements)
		if (stmt) stmt->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(WhileStmtNode& node) {
	PrintIndent(m_depth);
	std::cout << "WhileStmt\n";
	++m_depth;
	if (node.m_condition) {
		node.m_condition->Accept(*this);
	}
	if (node.m_body) {
		node.m_body->Accept(*this);
	}
	--m_depth;
}

void AstPrinter::Visit(ForStmtNode& node) {
	PrintIndent(m_depth);
	std::cout << "ForStmt\n";
	++m_depth;
	if (node.m_init) {
		node.m_init->Accept(*this);
	}
	if (node.m_condition) {
		node.m_condition->Accept(*this);
	}
	if (node.m_increment) {
		node.m_increment->Accept(*this);
	}
	if (node.m_body) {
		node.m_body->Accept(*this);
	}
	--m_depth;
}

void AstPrinter::Visit(ReturnStmtNode& node) {
	PrintIndent(m_depth);
	std::cout << "ReturnStmt\n";
	++m_depth;
	if (node.m_expression)
		node.m_expression->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(BreakStmtNode& node) {
	PrintIndent(m_depth);
	std::cout << "BreakStmt\n";
}

void AstPrinter::Visit(ContinueStmtNode& node) {
	PrintIndent(m_depth);
	std::cout << "ContinueStmt\n";
}

void AstPrinter::Visit(CommaExprNode& node) {
	PrintIndent(m_depth);
	std::cout << "CommaExpr(Expression)\n";
	++m_depth;
	for (auto* expr : node.m_expressions)
		if (expr) expr->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(AssignmentExprNode& node) {
	PrintIndent(m_depth);
	std::cout << "AssignmentExpr: " << node.m_op << "\n";
	++m_depth;
	if (node.m_left)
		node.m_left->Accept(*this);
	if (node.m_right)
		node.m_right->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(ConditionalExprNode& node) {
	PrintIndent(m_depth);
	std::cout << "ConditionalExpr\n";
	++m_depth;
	if (node.m_condition)
		node.m_condition->Accept(*this);
	if (node.m_trueExpr)
		node.m_trueExpr->Accept(*this);
	if (node.m_falseExpr)
		node.m_falseExpr->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(BinaryExprNode& node) {
	PrintIndent(m_depth);
	std::cout << "BinaryExpr: " << node.m_op << "\n";
	++m_depth;
	if (node.m_left)
		node.m_left->Accept(*this);
	if (node.m_right)
		node.m_right->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(UnaryExprNode& node) {
	PrintIndent(m_depth);
	std::cout << "UnaryExpr: " << node.m_op << "\n";
	++m_depth;
	if (node.m_operand)
		node.m_operand->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(PostfixExprNode& node) {
	PrintIndent(m_depth);
	std::cout << "PostfixExpr: " << node.m_op << "\n";
	++m_depth;
	if (node.m_primary)
		node.m_primary->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(FunctionCallNode& node) {
	PrintIndent(m_depth);
	std::cout << "FunctionCall\n";
	++m_depth;
	if (node.m_callee)
		node.m_callee->Accept(*this);
	for (auto* arg : node.m_arguments)
		if (arg) arg->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(ArrayIndexNode& node) {
	PrintIndent(m_depth);
	std::cout << "ArrayIndex\n";
	++m_depth;
	if (node.m_array)
		node.m_array->Accept(*this);
	if (node.m_index)
		node.m_index->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(MemberAccessNode& node) {
	PrintIndent(m_depth);
	std::cout << "MemberAccess\n";
	++m_depth;
	if (node.m_object)
		node.m_object->Accept(*this);
	PrintIndent(m_depth);
	std::cout << "MemberName: " << node.m_memberName->m_name << "\n";
	--m_depth;
}

void AstPrinter::Visit(FunctionLiteralNode& node) {
	PrintIndent(m_depth);
	std::cout << "FunctionLiteral\n";
	++m_depth;
	for (auto* param : node.m_params)
		if (param) param->Accept(*this);
	if (node.m_returnType)
		node.m_returnType->Accept(*this);
	if (node.m_body)
		node.m_body->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(IdentifierNode& node) {
	PrintIndent(m_depth);
	std::cout << "Identifier: " << node.m_name << "\n";
}

void AstPrinter::Visit(LiteralNode& node) {
	PrintIndent(m_depth);
	std::cout << "Literal: " << node.m_value << "\n";
}

void AstPrinter::Visit(ParameterNode& node) {
	PrintIndent(m_depth);
	std::cout << "Parameter\n";
	++m_depth;
	if (node.m_type)
		node.m_type->Accept(*this);
	if (node.m_declarator)
		node.m_declarator->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(DeclaratorNode& node) {
	PrintIndent(m_depth);
	std::cout << "Declarator: " << node.m_name->m_name << "\n";
	++m_depth;
	for (auto* sizeExpr : node.m_arraySizes)
		if (sizeExpr) sizeExpr->Accept(*this);
	if (node.m_initializer)
		node.m_initializer->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(StructMemberNode& node) {
	PrintIndent(m_depth);
	std::cout << "StructMember\n";
	++m_depth;
	if (node.m_type)
		node.m_type->Accept(*this);
	for (auto* declarator : node.m_declarators)
		if (declarator) declarator->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(InitializerNode& node) {
	PrintIndent(m_depth);
	std::cout << "Initializer\n";
	++m_depth;
	for (auto* value : node.m_values)
		if (value) value->Accept(*this);
	--m_depth;
}

void AstPrinter::Visit(BuiltinTypeNode& node) {
	PrintIndent(m_depth);
	std::cout << "BuiltinType: " << node.m_name << "\n";
}

void AstPrinter::Visit(NamedTypeNode& node) {
	PrintIndent(m_depth);
	std::cout << "NamedType: " << node.m_name << "\n";
}

void AstPrinter::Visit(FunctionTypeNode& node) {
	PrintIndent(m_depth);
	std::cout << "FunctionType\n";
	++m_depth;
	for (auto* paramType : node.m_paramTypes)
		if (paramType) paramType->Accept(*this);
	if (node.m_returnType)
		node.m_returnType->Accept(*this);
	--m_depth;
}