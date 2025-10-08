#include"Parser.h"

using namespace CppInterp;

void Parser::PreprocessTokens(std::vector<Token>& tokens) {
	static const std::unordered_map<std::string, TokenType::Type> keywordMap = {
		{"function", TokenType::FUNCTION},
		{"let", TokenType::LET},
		{"const", TokenType::CONST},
		{"if", TokenType::IF},
		{"else", TokenType::ELSE},
		{"while", TokenType::WHILE},
		{"for", TokenType::FOR},
		{"return", TokenType::RETURN},
		{"break", TokenType::BREAK},
		{"continue", TokenType::CONTINUE},
		{"true", TokenType::TRUE},
		{"false", TokenType::FALSE},
		{"TRUE", TokenType::TRUE},
		{"FALSE", TokenType::FALSE},
		{"null", TokenType::NULL_LITERAL},
		{"NULL", TokenType::NULL_LITERAL},
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
	m_tokens = Lexer::Instance().Tokenize(str);
	PreprocessTokens(m_tokens);
	m_root = new AstNode(NodeType::PROGRAM);

}