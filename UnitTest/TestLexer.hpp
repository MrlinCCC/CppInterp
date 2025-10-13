#include "gtest/gtest.h"
#include <Lexer.h>

using namespace CppInterp;

static Lexer& g_lexer = Lexer::Instance();

struct TokenCase {
	std::string input;
	TokenType::Type expected;
	std::string expectedContent;
};


static const std::vector<TokenCase> tokenCases = {
	{"abc", TokenType::IDENTIFIER, "abc"},
	{"_abc", TokenType::IDENTIFIER, "_abc"},
	{"123", TokenType::INT_LITERAL, "123"},
	{"3.14", TokenType::DOUBLE_LITERAL, "3.14"},
	{"'a'", TokenType::CHARACTER_LITERAL, "'a'"},
	{"'\\n'", TokenType::CHARACTER_LITERAL, "'\n'"},
	{"'\\t'", TokenType::CHARACTER_LITERAL, "'\t'"},
	{"'\\0'", TokenType::CHARACTER_LITERAL, std::string("'\0'", 3)},
	{"'\\r'", TokenType::CHARACTER_LITERAL, "'\r'"},
	{"'\\\\'", TokenType::CHARACTER_LITERAL, "'\\'"},
	{"'\\''", TokenType::CHARACTER_LITERAL, "'\''"},
	{"'\\\"'", TokenType::CHARACTER_LITERAL, "'\"'"},
	{"\"str\"", TokenType::STRING_LITERAL, "\"str\""},
	{"\"hello\\nworld\"", TokenType::STRING_LITERAL, "\"hello\nworld\""},
	{"\"tab\\tcharacter\"", TokenType::STRING_LITERAL, "\"tab\tcharacter\""},
	{"\"quote\\\"inside\"", TokenType::STRING_LITERAL, "\"quote\"inside\""},
	{"\"backslash\\\\inside\"", TokenType::STRING_LITERAL, "\"backslash\\inside\""},
	{"\"null\\0char\"", TokenType::STRING_LITERAL, std::string("\"null\0char\"", 11)},
	{"\"carriage\\rreturn\"", TokenType::STRING_LITERAL, "\"carriage\rreturn\""},
	{"(", TokenType::LEFT_PAREN, "("},
	{")", TokenType::RIGHT_PAREN, ")"},
	{"{", TokenType::LEFT_BRACE, "{"},
	{"}", TokenType::RIGHT_BRACE, "}"},
	{"[", TokenType::LEFT_SQUARE, "["},
	{"]", TokenType::RIGHT_SQUARE, "]"},
	{";", TokenType::SEMICOLON, ";"},
	{",", TokenType::COMMA, ","},
	{"+", TokenType::ADD, "+"},
	{"-", TokenType::SUBTRACT, "-"},
	{"*", TokenType::MULTIPLY, "*"},
	{"/", TokenType::DIVIDE, "/"},
	{"%", TokenType::MODULO, "%"},
	{">", TokenType::GREATER, ">"},
	{"<", TokenType::LESS, "<"},
	{"!", TokenType::NOT, "!"},
	{"=", TokenType::ASSIGN, "="},
	{"&", TokenType::BIT_AND, "&"},
	{"|", TokenType::BIT_OR, "|"},
	{"^", TokenType::XOR, "^"},
	{"~", TokenType::BIT_NOT, "~"},
	{":", TokenType::COLON, ":"},
	{"?", TokenType::QUESTION, "?"},
	{".", TokenType::DOT, "."},
	//{"\\", TokenType::BACKSLASH, "\\"},
	{"+=", TokenType::SELF_ADD, "+="},
	{"-=", TokenType::SELF_SUB, "-="},
	{"*=", TokenType::SELF_MUL, "*="},
	{"/=", TokenType::SELF_DIV, "/="},
	{"%=", TokenType::SELF_MODULO, "%="},
	{">=", TokenType::GREATER_EQUAL, ">="},
	{"<=", TokenType::LESS_EQUAL, "<="},
	{"!=", TokenType::NOT_EQUAL, "!="},
	{"==", TokenType::EQUAL, "=="},
	{"&=", TokenType::SELF_BIT_AND, "&="},
	{"|=", TokenType::SELF_BIT_OR, "|="},
	{"^=", TokenType::SELF_XOR, "^="},
	{"&&", TokenType::AND, "&&"},
	{"||", TokenType::OR, "||"},
	{"++", TokenType::INCREMENT, "++"},
	{"--", TokenType::DECREMENT, "--"},
	{"->", TokenType::POINT_TO, "->"},
	{"::", TokenType::BELONG_TO, "::"},
	{">>", TokenType::RIGHT_MOVE, ">>"},
	{">>=", TokenType::SELF_RIGHT_MOVE, ">>="},
	{"<<", TokenType::LEFT_MOVE, "<<"},
	{"<<=", TokenType::SELF_LEFT_MOVE, "<<="}
};

class LexerTokenTest : public ::testing::TestWithParam<TokenCase> {};

TEST_P(LexerTokenTest, HandlesSingleToken) {
	const auto& param = GetParam();
	auto tokens = g_lexer.Tokenize(param.input);
	ASSERT_EQ(tokens.size(), 1) << "Input: " << param.input;
	EXPECT_EQ(tokens[0].m_type, param.expected) << "Input: " << param.input;
	EXPECT_EQ(tokens[0].m_content, param.expectedContent) << "Input: " << param.input;
}

INSTANTIATE_TEST_SUITE_P(AllSingleTokens, LexerTokenTest, ::testing::ValuesIn(tokenCases));


struct MultiTokenCase {
	std::string input;
	std::vector<TokenType::Type> expectedTypes;
	std::vector<std::string> expectedContent;
};

class LexerMultiTokenTest : public ::testing::TestWithParam<MultiTokenCase> {};

TEST_P(LexerMultiTokenTest, HandlesMultipleTokens) {
	const auto& param = GetParam();
	auto tokens = g_lexer.Tokenize(param.input);

	ASSERT_EQ(tokens.size(), param.expectedTypes.size())
		<< "Token count mismatch for input: " << param.input;

	for (size_t i = 0; i < tokens.size(); ++i) {
		EXPECT_EQ(tokens[i].m_type, param.expectedTypes[i])
			<< "Mismatch at token " << i << " for input: " << tokens[i].m_content
			<< "at line " << tokens[i].m_line << ", col " << tokens[i].m_column;
		EXPECT_EQ(tokens[i].m_content, param.expectedContent[i])
			<< "Mismatch at token " << i << " for input: " << tokens[i].m_content
			<< "at line " << tokens[i].m_line << ", col " << tokens[i].m_column;
	}
}

std::vector<MultiTokenCase> multiTokenCases = {
	{
		"123;abc",
		{TokenType::INT_LITERAL, TokenType::SEMICOLON, TokenType::IDENTIFIER},
		{"123", ";", "abc"}
	},
	{
		"char c='\\n'; string s=\"hello\\tworld\";",
		{TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::CHARACTER_LITERAL, TokenType::SEMICOLON,
		 TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::STRING_LITERAL, TokenType::SEMICOLON},
		{"char", "c", "=", "'\n'", ";", "string", "s", "=", "\"hello\tworld\"", ";"}
	},
	{
		"(x+1)",
		{TokenType::LEFT_PAREN, TokenType::IDENTIFIER, TokenType::ADD, TokenType::INT_LITERAL, TokenType::RIGHT_PAREN},
		{"(", "x", "+", "1", ")"}
	},
	{
		"a=10;b",
		{TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INT_LITERAL, TokenType::SEMICOLON, TokenType::IDENTIFIER},
		{"a", "=", "10", ";", "b"}
	},
	{
		"x+=42;y==z;flag!=0;",
		{TokenType::IDENTIFIER, TokenType::SELF_ADD, TokenType::INT_LITERAL, TokenType::SEMICOLON,
		 TokenType::IDENTIFIER, TokenType::EQUAL, TokenType::IDENTIFIER, TokenType::SEMICOLON,
		 TokenType::IDENTIFIER, TokenType::NOT_EQUAL, TokenType::INT_LITERAL, TokenType::SEMICOLON},
		{"x", "+=", "42", ";", "y", "==", "z", ";", "flag", "!=", "0", ";"}
	},
	{
		"char c='a'; string s=\"hello\";",
		{TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::CHARACTER_LITERAL, TokenType::SEMICOLON,
		 TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::STRING_LITERAL, TokenType::SEMICOLON},
		{"char", "c", "=", "'a'", ";", "string", "s", "=", "\"hello\"", ";"}
	},
	{
		"pi=3.14159;",
		{TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::DOUBLE_LITERAL, TokenType::SEMICOLON},
		{"pi", "=", "3.14159", ";"}
	},
	{
		"if(a&&b||c){return;}",
		{TokenType::IDENTIFIER, TokenType::LEFT_PAREN, TokenType::IDENTIFIER, TokenType::AND,
		 TokenType::IDENTIFIER, TokenType::OR, TokenType::IDENTIFIER, TokenType::RIGHT_PAREN,
		 TokenType::LEFT_BRACE, TokenType::IDENTIFIER, TokenType::SEMICOLON, TokenType::RIGHT_BRACE},
		{"if", "(", "a", "&&", "b", "||", "c", ")", "{", "return", ";", "}"}
	},
	{
		"a=1; // this is a comment\n b=2;",
		{TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INT_LITERAL, TokenType::SEMICOLON,
		 TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INT_LITERAL, TokenType::SEMICOLON},
		{"a", "=", "1", ";", "b", "=", "2", ";"}
	},
	{
		"a=1+\\\n2;",
		{TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INT_LITERAL, TokenType::ADD, TokenType::INT_LITERAL, TokenType::SEMICOLON},
		{"a", "=", "1", "+", "2", ";"}
	},
	{
		"",
		{},
		{}
	},
	{
		"   ",
		{},
		{}
	},
	{
		"\n\t",
		{},
		{}
	}
};

INSTANTIATE_TEST_SUITE_P(
	CompoundTokens,
	LexerMultiTokenTest,
	::testing::ValuesIn(multiTokenCases)
);

std::vector<MultiTokenCase> complexTokenCases = {
	{
		R"(int main() {
				int a = 10;
				double b = 3.14;
				char c = '\n';
				string s = "Hello\tWorld!";
				bool flag = true;

				a = a + 5 * (b - 2) / 4;
				b += 1.5;
				c = '\\';

				if (a >= 10 && flag || b < 5.0) {
					s = "Result:\n" + s;
				}

				for (int i = 0; i < 3; i++) {
					while (flag) {
						flag = false;
					}
				}

				print(s, a, b, c);

				return 0;
			})",
		{
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::LEFT_PAREN, TokenType::RIGHT_PAREN, TokenType::LEFT_BRACE,

			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INT_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::DOUBLE_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::CHARACTER_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::STRING_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::IDENTIFIER, TokenType::SEMICOLON,

			TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::IDENTIFIER, TokenType::ADD,
			TokenType::INT_LITERAL, TokenType::MULTIPLY, TokenType::LEFT_PAREN,
			TokenType::IDENTIFIER, TokenType::SUBTRACT, TokenType::INT_LITERAL, TokenType::RIGHT_PAREN,
			TokenType::DIVIDE, TokenType::INT_LITERAL, TokenType::SEMICOLON,

			TokenType::IDENTIFIER, TokenType::SELF_ADD, TokenType::DOUBLE_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::CHARACTER_LITERAL, TokenType::SEMICOLON,

			TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
			TokenType::IDENTIFIER, TokenType::GREATER_EQUAL, TokenType::INT_LITERAL,
			TokenType::AND, TokenType::IDENTIFIER, TokenType::OR,
			TokenType::IDENTIFIER, TokenType::LESS, TokenType::DOUBLE_LITERAL,
			TokenType::RIGHT_PAREN, TokenType::LEFT_BRACE,
			TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::STRING_LITERAL, TokenType::ADD, TokenType::IDENTIFIER, TokenType::SEMICOLON,
			TokenType::RIGHT_BRACE,

			TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INT_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::LESS, TokenType::INT_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::INCREMENT,
			TokenType::RIGHT_PAREN, TokenType::LEFT_BRACE,

			TokenType::IDENTIFIER, TokenType::LEFT_PAREN, TokenType::IDENTIFIER, TokenType::RIGHT_PAREN,
			TokenType::LEFT_BRACE,
			TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::IDENTIFIER, TokenType::SEMICOLON,
			TokenType::RIGHT_BRACE,
			TokenType::RIGHT_BRACE,

			TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
			TokenType::IDENTIFIER, TokenType::COMMA, TokenType::IDENTIFIER, TokenType::COMMA,
			TokenType::IDENTIFIER, TokenType::COMMA, TokenType::IDENTIFIER,
			TokenType::RIGHT_PAREN, TokenType::SEMICOLON,

			TokenType::IDENTIFIER, TokenType::INT_LITERAL, TokenType::SEMICOLON,

			TokenType::RIGHT_BRACE
		},
		{
			"int", "main", "(", ")", "{",

			"int", "a", "=", "10", ";",
			"double", "b", "=", "3.14", ";",
			"char", "c", "=", "'\n'", ";",
			"string", "s", "=", "\"Hello\tWorld!\"", ";",
			"bool", "flag", "=", "true", ";",

			"a", "=", "a", "+", "5", "*", "(", "b", "-", "2", ")", "/", "4", ";",
			"b", "+=", "1.5", ";",
			"c", "=", "'\\'", ";",

			"if", "(", "a", ">=", "10", "&&", "flag", "||", "b", "<", "5.0", ")", "{",
			"s", "=", "\"Result:\n\"", "+", "s", ";",
			"}",

			"for", "(", "int", "i", "=", "0", ";", "i", "<", "3", ";", "i", "++", ")", "{",
			"while", "(", "flag", ")", "{",
			"flag", "=", "false", ";",
			"}",
			"}",

			"print", "(", "s", ",", "a", ",", "b", ",", "c", ")", ";",

			"return", "0", ";",

			"}"
		}
	},
	{
	R"(struct Point {
            int x;
            int y;
        };

        int add(int a, int b) {
            return a + b;
        }

        int main() {
            Point p = {1, 2};
            int arr[3] = {10, 20, 30};
            int sum = add(arr[0], p.x + p.y);

            if (sum != 0) {
                for (int i = 0; i < 3; i++) {
                    sum -= arr[i];
                }
            }

            return sum;
        })",
	{
		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::LEFT_BRACE,
		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::SEMICOLON,
		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::SEMICOLON,
		TokenType::RIGHT_BRACE, TokenType::SEMICOLON,

		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::COMMA,
		TokenType::IDENTIFIER, TokenType::IDENTIFIER,
		TokenType::RIGHT_PAREN, TokenType::LEFT_BRACE,
		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ADD, TokenType::IDENTIFIER, TokenType::SEMICOLON,
		TokenType::RIGHT_BRACE,

		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::LEFT_PAREN, TokenType::RIGHT_PAREN,
		TokenType::LEFT_BRACE,

		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN,
		TokenType::LEFT_BRACE, TokenType::INT_LITERAL, TokenType::COMMA, TokenType::INT_LITERAL, TokenType::RIGHT_BRACE, TokenType::SEMICOLON,

		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::LEFT_SQUARE, TokenType::INT_LITERAL, TokenType::RIGHT_SQUARE,
		TokenType::ASSIGN,
		TokenType::LEFT_BRACE, TokenType::INT_LITERAL, TokenType::COMMA, TokenType::INT_LITERAL, TokenType::COMMA, TokenType::INT_LITERAL, TokenType::RIGHT_BRACE, TokenType::SEMICOLON,

		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN,
		TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
		TokenType::IDENTIFIER, TokenType::LEFT_SQUARE, TokenType::INT_LITERAL, TokenType::RIGHT_SQUARE, TokenType::COMMA,
		TokenType::IDENTIFIER, TokenType::DOT, TokenType::IDENTIFIER, TokenType::ADD, TokenType::IDENTIFIER, TokenType::DOT, TokenType::IDENTIFIER,
		TokenType::RIGHT_PAREN, TokenType::SEMICOLON,

		TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
		TokenType::IDENTIFIER, TokenType::NOT_EQUAL, TokenType::INT_LITERAL,
		TokenType::RIGHT_PAREN, TokenType::LEFT_BRACE,

		TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INT_LITERAL, TokenType::SEMICOLON,
		TokenType::IDENTIFIER, TokenType::LESS, TokenType::INT_LITERAL, TokenType::SEMICOLON,
		TokenType::IDENTIFIER, TokenType::INCREMENT,
		TokenType::RIGHT_PAREN, TokenType::LEFT_BRACE,

		TokenType::IDENTIFIER, TokenType::SELF_SUB, TokenType::IDENTIFIER, TokenType::LEFT_SQUARE, TokenType::IDENTIFIER, TokenType::RIGHT_SQUARE, TokenType::SEMICOLON,

		TokenType::RIGHT_BRACE,
		TokenType::RIGHT_BRACE,

		TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::SEMICOLON,

		TokenType::RIGHT_BRACE
	},
	{
		"struct", "Point", "{",
		"int", "x", ";",
		"int", "y", ";",
		"}", ";",

		"int", "add", "(", "int", "a", ",", "int", "b", ")", "{",
		"return", "a", "+", "b", ";",
		"}",

		"int", "main", "(", ")", "{",

		"Point", "p", "=", "{", "1", ",", "2", "}", ";",
		"int", "arr", "[", "3", "]", "=", "{", "10", ",", "20", ",", "30", "}", ";",
		"int", "sum", "=", "add", "(", "arr", "[", "0", "]", ",", "p", ".", "x", "+", "p", ".", "y", ")", ";",

		"if", "(", "sum", "!=", "0", ")", "{",
		"for", "(", "int", "i", "=", "0", ";", "i", "<", "3", ";", "i", "++", ")", "{",
		"sum", "-=", "arr", "[", "i", "]", ";",
		"}",
		"}",

		"return", "sum", ";",

		"}"
	}
	}

};

INSTANTIATE_TEST_SUITE_P(
	ComplexTokens,
	LexerMultiTokenTest,
	::testing::ValuesIn(complexTokenCases)
);

struct PositionTokenCase {
	std::string input;
	std::vector<TokenType::Type> expectedTypes;
	std::vector<std::string> expectedContent;
	std::vector<int> expectedLines;
	std::vector<int> expectedColumns;
};

std::vector<PositionTokenCase> positionTokenCases = {
	{
		R"(int main() {
    int a = 10;
    a = a + 1;
})",
		{
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::LEFT_PAREN, TokenType::RIGHT_PAREN, TokenType::LEFT_BRACE,
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INT_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::IDENTIFIER, TokenType::ADD, TokenType::INT_LITERAL, TokenType::SEMICOLON,
			TokenType::RIGHT_BRACE
		},
		{
			"int", "main", "(", ")", "{",
			"int", "a", "=", "10", ";",
			"a", "=", "a", "+", "1", ";",
			"}"
		},
		{
			1,1,1,1,1,
			2,2,2,2,2,
			3,3,3,3,3,3,
			4
		},
		{
			1,5,9,10,12,
			5,9,11,13,15,
			5,7,9,11,13,14,
			1
		}
	},
	{
		R"(int x=5;
    float y = 3.14;
		x = x + y;)",
		{
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INT_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::DOUBLE_LITERAL, TokenType::SEMICOLON,
			TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::IDENTIFIER, TokenType::ADD, TokenType::IDENTIFIER, TokenType::SEMICOLON
		},
		{
			"int", "x", "=", "5", ";",
			"float", "y", "=", "3.14", ";",
			"x", "=", "x", "+", "y", ";"
		},
		{
			1,1,1,1,1,
			2,2,2,2,2,
			3,3,3,3,3,3
		},
		{
			1,5,6,7,8,
			5,11,13,15,19,
			3,5,7,9,11,12
		}
	}
};

class LexerPositionTest : public ::testing::TestWithParam<PositionTokenCase> {};

TEST_P(LexerPositionTest, HandlesTokenPositions) {
	const auto& param = GetParam();
	std::vector<Token> tokens = g_lexer.Tokenize(param.input);

	ASSERT_EQ(tokens.size(), param.expectedTypes.size())
		<< "Token count mismatch for input: " << param.input;

	for (size_t i = 0; i < tokens.size(); ++i) {
		EXPECT_EQ(tokens[i].m_type, param.expectedTypes[i])
			<< "Type mismatch at token " << i << " content: " << tokens[i].m_content;
		EXPECT_EQ(tokens[i].m_content, param.expectedContent[i])
			<< "Content mismatch at token " << i;
		EXPECT_EQ(tokens[i].m_line, param.expectedLines[i])
			<< "Line mismatch at token " << i << " content: " << tokens[i].m_content;
		EXPECT_EQ(tokens[i].m_column, param.expectedColumns[i])
			<< "Column mismatch at token " << i << " content: " << tokens[i].m_content;
	}
}

INSTANTIATE_TEST_SUITE_P(TokenPositions, LexerPositionTest, ::testing::ValuesIn(positionTokenCases));


TEST(LexerExceptionTest, InvalidCharacter) {
	std::string input = "int main() { return 1@; }";

	try {
		g_lexer.Tokenize(input);
		FAIL() << "Expected LexerException due to invalid character '@'";
	}
	catch (const LexerException& ex) {
		EXPECT_EQ(ex.GetChar(), '@');
		EXPECT_EQ(ex.GetRow(), 1);
		EXPECT_EQ(ex.GetCol(), 22);
		EXPECT_NE(std::string(ex.what()).find("Unknown character exception"), std::string::npos);
	}
	catch (...) {
		FAIL() << "Expected LexerException, but caught unknown exception";
	}
}

TEST(LexerExceptionTest, InvalidEscape) {
	std::string input = "char c = '\\q';";

	try {
		g_lexer.Tokenize(input);
		FAIL() << "Expected LexerException due to invalid escape sequence";
	}
	catch (const LexerException& ex) {
		EXPECT_EQ(ex.GetChar(), 'q');
		EXPECT_EQ(ex.GetRow(), 1);
		EXPECT_EQ(ex.GetCol(), 12);
		EXPECT_NE(std::string(ex.what()).find("Unknown escape character exception"), std::string::npos);
	}
	catch (...) {
		FAIL() << "Expected LexerException, but caught unknown exception";
	}
}
