#pragma once
#include <string>
#include <vector>
#include<unordered_map>
#include<functional>
#include "Singleton.h"
#include <stdexcept>
#include<optional>
#include "Exception.hpp"

namespace CppInterp {

	namespace TokenType {
		using Type = uint8_t;

		// identifier and literal
		constexpr Type UNKNOWN = 0;
		constexpr Type IDENTIFIER = 1;
		constexpr Type INT_LITERAL = 2;
		constexpr Type DOUBLE_LITERAL = 3;
		constexpr Type CHARACTER_LITERAL = 4;
		constexpr Type STRING_LITERAL = 5;

		// delimiter
		constexpr Type LEFT_PAREN = 6;       // (
		constexpr Type RIGHT_PAREN = 7;      // )
		constexpr Type LEFT_BRACE = 8;      // {
		constexpr Type RIGHT_BRACE = 9;     // }
		constexpr Type LEFT_SQUARE = 10;     // [
		constexpr Type RIGHT_SQUARE = 11;    // ]
		constexpr Type SEMICOLON = 12;       // ;
		constexpr Type COMMA = 13;           // ,

		// single symbol
		constexpr Type ADD = 14;             // +
		constexpr Type SUBTRACT = 15;        // -
		constexpr Type MULTIPLY = 16;        // *
		constexpr Type DIVIDE = 17;          // /
		constexpr Type MODULO = 18;          // %
		constexpr Type GREATER = 19;         // >
		constexpr Type LESS = 20;            // <
		constexpr Type NOT = 21;             // !
		constexpr Type ASSIGN = 22;          // =
		constexpr Type BIT_AND = 23;         // &
		constexpr Type BIT_OR = 24;          // |
		constexpr Type XOR = 25;             // ^
		constexpr Type BIT_NOT = 26;         // ~
		constexpr Type COLON = 27;           // :
		constexpr Type QUESTION = 28;        // ?
		constexpr Type DOT = 29;             // .
		//constexpr Type BACKSLASH = 30;       // \	

		// multiple symbol
		constexpr Type SELF_ADD = 31;        // +=
		constexpr Type SELF_SUB = 32;        // -=
		constexpr Type SELF_MUL = 33;        // *=
		constexpr Type SELF_DIV = 34;        // /=
		constexpr Type SELF_MODULO = 35;     // %=
		constexpr Type GREATER_EQUAL = 36;   // >=
		constexpr Type LESS_EQUAL = 37;      // <=
		constexpr Type NOT_EQUAL = 38;       // !=
		constexpr Type EQUAL = 39;           // ==
		constexpr Type SELF_BIT_AND = 40;    // &=
		constexpr Type SELF_BIT_OR = 41;     // |=
		constexpr Type SELF_XOR = 42;        // ^=
		constexpr Type AND = 43;             // &&
		constexpr Type OR = 44;              // ||
		constexpr Type INCREMENT = 45;       // ++
		constexpr Type DECREMENT = 46;       // --
		constexpr Type POINT_TO = 47;        // ->
		constexpr Type BELONG_TO = 48;       // ::
		constexpr Type RIGHT_MOVE = 48;       // >>
		constexpr Type LEFT_MOVE = 49;        // <<
		constexpr Type SELF_RIGHT_MOVE = 50;  // >>=
		constexpr Type SELF_LEFT_MOVE = 51;   // <<=
	}


	namespace Character {
		using Type = uint8_t;

		constexpr Type UNKNOWN = 0;
		constexpr Type LETTER = 1;			//a-z A-Z
		constexpr Type NUMBER = 2;			//0-9

		constexpr Type LEFT_PAREN = 3;       // (
		constexpr Type RIGHT_PAREN = 4;      // )
		constexpr Type LEFT_BRACE = 5;      // {
		constexpr Type RIGHT_BRACE = 6;     // }
		constexpr Type LEFT_SQUARE = 7;     // [
		constexpr Type RIGHT_SQUARE = 8;    // ]
		constexpr Type SEMICOLON = 9;       // ;
		constexpr Type COMMA = 10;           // ,

		constexpr Type ADD = 11;             // +
		constexpr Type SUBTRACT = 12;        // -
		constexpr Type MULTIPLY = 13;        // *
		constexpr Type DIVIDE = 14;          // /
		constexpr Type MODULO = 15;          // %
		constexpr Type GREATER = 16;         // >
		constexpr Type LESS = 17;            // <
		constexpr Type NOT = 18;             // !
		constexpr Type ASSIGN = 19;          // =
		constexpr Type BIT_AND = 20;         // &
		constexpr Type BIT_OR = 21;          // |
		constexpr Type XOR = 22;             // ^
		constexpr Type BIT_NOT = 23;         // ~
		constexpr Type COLON = 24;           // :
		constexpr Type QUESTION = 25;        // ?
		constexpr Type DOT = 26;             // .
		constexpr Type BACKSLASH = 27;       // \	
		constexpr Type UNDERSCORE = 28;       // _
		constexpr Type SINGLE_QUOTE = 29;    // '
		constexpr Type DOUBLE_QUOTE = 30;    // "
		constexpr Type WORDSPLIT = 31;       // ' ' '\t'
		constexpr Type LINESPLIT = 32;         // '\r' '\n' 
	}

	class CharacterSet :public Singleton<CharacterSet> {
		friend class ::Singleton<CharacterSet>;
	public:
		inline std::optional<Character::Type> GetCharacterType(char ch) const {
			auto it = m_characterSet.find(ch);
			if (it == m_characterSet.end()) {
				return std::nullopt;
			}
			return it->second;
		}
	private:
		CharacterSet();

		std::unordered_map<char, Character::Type> m_characterSet;
	};

	class EscapeCharacterSet : public Singleton<EscapeCharacterSet> {
		friend class ::Singleton<EscapeCharacterSet>;
	public:
		inline bool IsEscapeCharacter(char ch) const {
			return m_escapeCharacterSet.find(ch) != m_escapeCharacterSet.end();
		}

		inline std::optional<char> Transform(char ch) const {
			auto it = m_escapeCharacterSet.find(ch);
			if (it == m_escapeCharacterSet.end()) {
				return std::nullopt;
			}
			return it->second;
		}

		inline const std::unordered_map<char, char>& GetEscapeCharacterSet() const {
			return m_escapeCharacterSet;
		}

	private:
		EscapeCharacterSet();

		std::unordered_map<char, char> m_escapeCharacterSet;
	};

	struct Token {
		TokenType::Type m_type;
		std::string m_content;
		int m_line;
		int m_column;

		Token() :m_type(TokenType::UNKNOWN), m_content(""), m_line(0), m_column(0) {}
		Token(TokenType::Type type, const std::string& content, int line, int column)
			:m_type(type), m_content(content), m_line(line), m_column(column) {
		}
	};

	namespace State {
		using Type = uint8_t;
		constexpr Type START = 0;
		constexpr Type IDENTIFIER = 1;
		constexpr Type INT = 2;
		constexpr Type DOUBLE = 3;
		constexpr Type CHAR_BEGIN = 4;
		constexpr Type CHAR_END = 5;
		constexpr Type STRING = 6;
		constexpr Type CHAR_ESCAPE = 7;
		constexpr Type STRING_ESCAPE = 8;
		constexpr Type ADD = 9;
		constexpr Type SUBTRACT = 10;
		constexpr Type MULTIPLY = 11;
		constexpr Type DIVIDE = 12;
		constexpr Type MODULO = 13;
		constexpr Type GREATER = 14;
		constexpr Type LESS = 15;
		constexpr Type NOT = 16;
		constexpr Type ASSIGN = 17;
		constexpr Type BIT_AND = 18;
		constexpr Type BIT_OR = 19;
		constexpr Type XOR = 20;
		constexpr Type COLON = 21;
		constexpr Type RIGHT_MOVE = 22;
		constexpr Type LEFT_MOVE = 23;
		constexpr Type COMMENT = 24;
	};

	constexpr int StateSize = 25;

	enum class Action {
		FORWARD,
		RETRACT,
		APPEND, //FORWARD + RETRACT
		JUMP,
		CLEAR,
		NEWLINE,
		ESCAPE
	};

	struct Transition {
		State::Type m_next;
		TokenType::Type m_tokenType;
		Action m_action;

		Transition()
			:m_next(State::START), m_tokenType(TokenType::UNKNOWN), m_action(Action::FORWARD) {
		}

		Transition(State::Type next, TokenType::Type tokenType, Action action)
			:m_next(next), m_tokenType(tokenType), m_action(action) {
		}
	};

	class Lexer :public Singleton<Lexer> {
		friend class ::Singleton<Lexer>;
	public:
		std::vector<Token> Tokenize(const std::string& str);

	private:
		Lexer();

		std::unordered_map<Character::Type, Transition> m_stateTransitionTable[StateSize];
		std::unordered_map<State::Type, Transition> m_stateOtherTransitionTable;
	};
};
