#include "Lexer.h"

namespace CppInterp {

	CharacterSet::CharacterSet() {
		for (int i = 0; i < 26; i++) {
			char ch = 'a' + i;
			m_characterSet[ch] = Character::LETTER;
			ch = 'A' + i;
			m_characterSet[ch] = Character::LETTER;
		}
		for (int i = 0; i < 10; i++) {
			char ch = '0' + i;
			m_characterSet[ch] = Character::NUMBER;
		}
		m_characterSet['('] = Character::LEFT_PAREN;
		m_characterSet[')'] = Character::RIGHT_PAREN;
		m_characterSet['{'] = Character::LEFT_BRACE;
		m_characterSet['}'] = Character::RIGHT_BRACE;
		m_characterSet['['] = Character::LEFT_SQUARE;
		m_characterSet[']'] = Character::RIGHT_SQUARE;
		m_characterSet[';'] = Character::SEMICOLON;
		m_characterSet[','] = Character::COMMA;
		m_characterSet['+'] = Character::ADD;
		m_characterSet['-'] = Character::SUBTRACT;
		m_characterSet['*'] = Character::MULTIPLY;
		m_characterSet['/'] = Character::DIVIDE;
		m_characterSet['%'] = Character::MODULO;
		m_characterSet['>'] = Character::GREATER;
		m_characterSet['<'] = Character::LESS;
		m_characterSet['!'] = Character::NOT;
		m_characterSet['='] = Character::ASSIGN;
		m_characterSet['&'] = Character::BIT_AND;
		m_characterSet['|'] = Character::BIT_OR;
		m_characterSet['^'] = Character::XOR;
		m_characterSet['~'] = Character::BIT_NOT;
		m_characterSet[':'] = Character::COLON;
		m_characterSet['?'] = Character::QUESTION;
		m_characterSet['.'] = Character::DOT;
		m_characterSet['\\'] = Character::BACKSLASH;
		m_characterSet['_'] = Character::UNDERSCORE;
		m_characterSet['\''] = Character::SINGLE_QUOTE;
		m_characterSet['"'] = Character::DOUBLE_QUOTE;
		m_characterSet[' '] = Character::WORDSPLIT;
		m_characterSet['\t'] = Character::WORDSPLIT;
		m_characterSet['\n'] = Character::LINESPLIT;
		m_characterSet['\r'] = Character::LINESPLIT;
	}

	EscapeCharacterSet::EscapeCharacterSet() {
		m_escapeCharacterSet['n'] = '\n';
		m_escapeCharacterSet['t'] = '\t';
		m_escapeCharacterSet['r'] = '\r';
		m_escapeCharacterSet['\''] = '\'';
		m_escapeCharacterSet['"'] = '\"';
		m_escapeCharacterSet['\\'] = '\\';
		m_escapeCharacterSet['0'] = '\0';
	}

	Lexer::Lexer() {
		//identifier
		m_stateTransitionTable[State::START].emplace(
			Character::LETTER, Transition(State::IDENTIFIER, TokenType::IDENTIFIER, Action::FORWARD)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::UNDERSCORE, Transition(State::IDENTIFIER, TokenType::IDENTIFIER, Action::FORWARD)
		);
		m_stateTransitionTable[State::IDENTIFIER].emplace(
			Character::LETTER, Transition(State::IDENTIFIER, TokenType::IDENTIFIER, Action::FORWARD)
		);
		m_stateTransitionTable[State::IDENTIFIER].emplace(
			Character::NUMBER, Transition(State::IDENTIFIER, TokenType::IDENTIFIER, Action::FORWARD)
		);
		m_stateTransitionTable[State::IDENTIFIER].emplace(
			Character::UNDERSCORE, Transition(State::IDENTIFIER, TokenType::IDENTIFIER, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::IDENTIFIER, Transition(State::START, TokenType::IDENTIFIER, Action::RETRACT)
		);
		//literal
		// int
		m_stateTransitionTable[State::START].emplace(
			Character::NUMBER, Transition(State::INT, TokenType::INT_LITERAL, Action::FORWARD)
		);
		m_stateTransitionTable[State::INT].emplace(
			Character::NUMBER, Transition(State::INT, TokenType::INT_LITERAL, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(State::INT,
			Transition(State::START, TokenType::INT_LITERAL, Action::RETRACT)
		);
		// double
		m_stateTransitionTable[State::INT].emplace(
			Character::DOT, Transition(State::DOUBLE, TokenType::DOUBLE_LITERAL, Action::FORWARD)
		);
		m_stateTransitionTable[State::DOUBLE].emplace(
			Character::NUMBER, Transition(State::DOUBLE, TokenType::DOUBLE_LITERAL, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::DOUBLE, Transition(State::START, TokenType::DOUBLE_LITERAL, Action::RETRACT)
		);
		// char
		m_stateTransitionTable[State::START].emplace(
			Character::SINGLE_QUOTE, Transition(State::CHAR_BEGIN, TokenType::CHARACTER_LITERAL, Action::FORWARD)
		);
		m_stateTransitionTable[State::CHAR_BEGIN].emplace(
			Character::BACKSLASH, Transition(State::CHAR_ESCAPE, TokenType::CHARACTER_LITERAL, Action::FORWARD)
		);
		const auto& escapeCharacterSet = EscapeCharacterSet::Instance().GetEscapeCharacterSet();
		auto& characterSet = CharacterSet::Instance();
		for (auto [ch, escapeCh] : escapeCharacterSet) {
			auto escapeOpt = characterSet.GetCharacterType(ch);
			if (escapeOpt)
				m_stateTransitionTable[State::CHAR_ESCAPE].emplace(
					*escapeOpt, Transition(State::CHAR_END, TokenType::CHARACTER_LITERAL, Action::ESCAPE)
				);
		}
		m_stateOtherTransitionTable.emplace(
			State::CHAR_BEGIN, Transition(State::CHAR_END, TokenType::CHARACTER_LITERAL, Action::FORWARD)
		);
		m_stateTransitionTable[State::CHAR_END].emplace(
			Character::SINGLE_QUOTE, Transition(State::START, TokenType::CHARACTER_LITERAL, Action::APPEND)
		);
		// string
		m_stateTransitionTable[State::START].emplace(
			Character::DOUBLE_QUOTE, Transition(State::STRING, TokenType::STRING_LITERAL, Action::FORWARD)
		);
		m_stateTransitionTable[State::STRING].emplace(
			Character::LETTER, Transition(State::STRING, TokenType::STRING_LITERAL, Action::FORWARD)
		);
		m_stateTransitionTable[State::STRING].emplace(
			Character::BACKSLASH, Transition(State::STRING_ESCAPE, TokenType::STRING_LITERAL, Action::FORWARD)
		);
		for (auto [ch, escapeCh] : escapeCharacterSet) {
			auto escapeOpt = characterSet.GetCharacterType(ch);
			if (escapeOpt)
				m_stateTransitionTable[State::STRING_ESCAPE].emplace(
					*escapeOpt, Transition(State::STRING, TokenType::STRING_LITERAL, Action::ESCAPE)
				);
		}
		m_stateOtherTransitionTable.emplace(
			State::STRING, Transition(State::STRING, TokenType::STRING_LITERAL, Action::FORWARD)
		);
		m_stateTransitionTable[State::STRING].emplace(
			Character::DOUBLE_QUOTE, Transition(State::START, TokenType::STRING_LITERAL, Action::APPEND)
		);
		//split
		m_stateTransitionTable[State::START].emplace(
			Character::WORDSPLIT, Transition(State::START, TokenType::UNKNOWN, Action::JUMP)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::LINESPLIT, Transition(State::START, TokenType::UNKNOWN, Action::NEWLINE)
		);
		//operator
		// +
		m_stateTransitionTable[State::START].emplace(
			Character::ADD, Transition(State::ADD, TokenType::ADD, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::ADD, Transition(State::START, TokenType::ADD, Action::RETRACT)
		);
		m_stateTransitionTable[State::ADD].emplace(
			Character::ADD, Transition(State::START, TokenType::INCREMENT, Action::APPEND)
		);
		m_stateTransitionTable[State::ADD].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_ADD, Action::APPEND)
		);
		// -
		m_stateTransitionTable[State::START].emplace(
			Character::SUBTRACT, Transition(State::SUBTRACT, TokenType::SUBTRACT, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::SUBTRACT, Transition(State::START, TokenType::SUBTRACT, Action::RETRACT)
		);
		m_stateTransitionTable[State::SUBTRACT].emplace(
			Character::SUBTRACT, Transition(State::START, TokenType::DECREMENT, Action::APPEND)
		);
		m_stateTransitionTable[State::SUBTRACT].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_SUB, Action::APPEND)
		);
		// *
		m_stateTransitionTable[State::START].emplace(
			Character::MULTIPLY, Transition(State::MULTIPLY, TokenType::MULTIPLY, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::MULTIPLY, Transition(State::START, TokenType::MULTIPLY, Action::RETRACT)
		);
		m_stateTransitionTable[State::MULTIPLY].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_MUL, Action::APPEND)
		);
		// /
		m_stateTransitionTable[State::START].emplace(
			Character::DIVIDE, Transition(State::DIVIDE, TokenType::DIVIDE, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::DIVIDE, Transition(State::START, TokenType::DIVIDE, Action::RETRACT)
		);
		m_stateTransitionTable[State::DIVIDE].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_DIV, Action::APPEND)
		);
		// %
		m_stateTransitionTable[State::START].emplace(
			Character::MODULO, Transition(State::MODULO, TokenType::MODULO, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::MODULO, Transition(State::START, TokenType::MODULO, Action::RETRACT)
		);
		m_stateTransitionTable[State::MODULO].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_MODULO, Action::APPEND)
		);
		// >
		m_stateTransitionTable[State::START].emplace(
			Character::GREATER, Transition(State::GREATER, TokenType::GREATER, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::GREATER, Transition(State::START, TokenType::GREATER, Action::RETRACT)
		);
		m_stateTransitionTable[State::GREATER].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::GREATER_EQUAL, Action::APPEND)
		);
		m_stateTransitionTable[State::GREATER].emplace(
			Character::GREATER, Transition(State::RIGHT_MOVE, TokenType::RIGHT_MOVE, Action::FORWARD)
		);
		m_stateTransitionTable[State::RIGHT_MOVE].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_RIGHT_MOVE, Action::APPEND)
		);
		m_stateOtherTransitionTable.emplace(
			State::RIGHT_MOVE, Transition(State::START, TokenType::RIGHT_MOVE, Action::RETRACT)
		);
		// <
		m_stateTransitionTable[State::START].emplace(
			Character::LESS, Transition(State::LESS, TokenType::LESS, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::LESS, Transition(State::START, TokenType::LESS, Action::RETRACT)
		);
		m_stateTransitionTable[State::LESS].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::LESS_EQUAL, Action::APPEND)
		);
		m_stateTransitionTable[State::LESS].emplace(
			Character::LESS, Transition(State::LEFT_MOVE, TokenType::LEFT_MOVE, Action::FORWARD)
		);
		m_stateTransitionTable[State::LEFT_MOVE].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_LEFT_MOVE, Action::APPEND)
		);
		m_stateOtherTransitionTable.emplace(
			State::LEFT_MOVE, Transition(State::START, TokenType::LEFT_MOVE, Action::RETRACT)
		);
		// !
		m_stateTransitionTable[State::START].emplace(
			Character::NOT, Transition(State::NOT, TokenType::NOT, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::NOT, Transition(State::START, TokenType::NOT, Action::RETRACT)
		);
		m_stateTransitionTable[State::NOT].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::NOT_EQUAL, Action::APPEND)
		);
		// =
		m_stateTransitionTable[State::START].emplace(
			Character::ASSIGN, Transition(State::ASSIGN, TokenType::ASSIGN, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::ASSIGN, Transition(State::START, TokenType::ASSIGN, Action::RETRACT)
		);
		m_stateTransitionTable[State::ASSIGN].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::EQUAL, Action::APPEND)
		);
		// &
		m_stateTransitionTable[State::START].emplace(
			Character::BIT_AND, Transition(State::BIT_AND, TokenType::BIT_AND, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::BIT_AND, Transition(State::START, TokenType::BIT_AND, Action::RETRACT)
		);
		m_stateTransitionTable[State::BIT_AND].emplace(
			Character::BIT_AND, Transition(State::START, TokenType::AND, Action::APPEND)
		);
		m_stateTransitionTable[State::BIT_AND].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_BIT_AND, Action::APPEND)
		);
		// |
		m_stateTransitionTable[State::START].emplace(
			Character::BIT_OR, Transition(State::BIT_OR, TokenType::BIT_OR, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::BIT_OR, Transition(State::START, TokenType::BIT_OR, Action::RETRACT)
		);
		m_stateTransitionTable[State::BIT_OR].emplace(
			Character::BIT_OR, Transition(State::START, TokenType::OR, Action::APPEND)
		);
		m_stateTransitionTable[State::BIT_OR].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_BIT_OR, Action::APPEND)
		);
		// ^
		m_stateTransitionTable[State::START].emplace(
			Character::XOR, Transition(State::XOR, TokenType::XOR, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::XOR, Transition(State::START, TokenType::XOR, Action::RETRACT)
		);
		m_stateTransitionTable[State::XOR].emplace(
			Character::ASSIGN, Transition(State::START, TokenType::SELF_XOR, Action::APPEND)
		);
		//other
		m_stateTransitionTable[State::START].emplace(
			Character::LEFT_PAREN, Transition(State::START, TokenType::LEFT_PAREN, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::RIGHT_PAREN, Transition(State::START, TokenType::RIGHT_PAREN, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::LEFT_BRACE, Transition(State::START, TokenType::LEFT_BRACE, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::RIGHT_BRACE, Transition(State::START, TokenType::RIGHT_BRACE, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::LEFT_SQUARE, Transition(State::START, TokenType::LEFT_SQUARE, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::RIGHT_SQUARE, Transition(State::START, TokenType::RIGHT_SQUARE, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::SEMICOLON, Transition(State::START, TokenType::SEMICOLON, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::COMMA, Transition(State::START, TokenType::COMMA, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::COLON, Transition(State::COLON, TokenType::COLON, Action::FORWARD)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::QUESTION, Transition(State::START, TokenType::QUESTION, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::DOT, Transition(State::START, TokenType::DOT, Action::APPEND)
		);
		m_stateTransitionTable[State::START].emplace(
			Character::BIT_NOT, Transition(State::START, TokenType::BIT_NOT, Action::APPEND)
		);
		m_stateTransitionTable[State::SUBTRACT].emplace(
			Character::GREATER, Transition(State::START, TokenType::POINT_TO, Action::APPEND)
		);
		m_stateTransitionTable[State::COLON].emplace(
			Character::COLON, Transition(State::START, TokenType::BELONG_TO, Action::APPEND)
		);
		//comment
		m_stateTransitionTable[State::DIVIDE].emplace(
			Character::DIVIDE, Transition(State::COMMENT, TokenType::UNKNOWN, Action::FORWARD)
		);
		m_stateOtherTransitionTable.emplace(
			State::COMMENT, Transition(State::COMMENT, TokenType::UNKNOWN, Action::JUMP)
		);
		m_stateTransitionTable[State::COMMENT].emplace(
			Character::LINESPLIT, Transition(State::START, TokenType::UNKNOWN, Action::CLEAR)
		);
		// \ 
		m_stateTransitionTable[State::START].emplace(
			Character::BACKSLASH, Transition(State::START, TokenType::UNKNOWN, Action::JUMP)
		);
	}


	std::vector<Token> Lexer::Tokenize(const std::string& str)
	{
		int row = 1, col = 1;
		std::string buf;
		State::Type state = State::START;
		std::vector<Token> tokens;
		auto it = str.cbegin();
		const auto& characterSet = CharacterSet::Instance();
		const auto& escapeCharacterSet = EscapeCharacterSet::Instance();
		Transition transition;
		while (it != str.cend()) {
			auto chTypeOpt = characterSet.GetCharacterType(*it);
			if (!chTypeOpt) {
				throw LexerException("Unknown character exception'", *it, row, col);
			}
			auto& curStateTransTable = m_stateTransitionTable[state];
			if (auto transIt = curStateTransTable.find(*chTypeOpt); transIt != curStateTransTable.end()) {
				transition = transIt->second;
			}
			else {
				//other state
				transition = m_stateOtherTransitionTable.at(state);
			}
			switch (transition.m_action) {
			case Action::FORWARD: {
				buf.push_back(*it);
				break;
			}
			case Action::RETRACT: {
				tokens.emplace_back(transition.m_tokenType, buf, row, col - buf.size());
				buf.clear();
				break;
			}
			case Action::APPEND: {
				buf.push_back(*it);
				tokens.emplace_back(transition.m_tokenType, buf, row, col - buf.size() + 1);
				buf.clear();
				break;
			}
			case Action::JUMP: {
				break;
			}
			case Action::CLEAR: {
				buf.clear();
				break;
			}
			case Action::NEWLINE: {
				col = 0;
				row += 1;
				break;
			}
			case Action::ESCAPE: {
				auto escapeChOpt = escapeCharacterSet.Transform(*it);
				if (!escapeChOpt) {
					throw LexerException("Unknown escape character exception'", *it, row, col);
				}
				buf.pop_back();
				buf.push_back(*escapeChOpt);
				break;
			}
			}
			state = transition.m_next;
			if (transition.m_action != Action::RETRACT) {
				it++;
				col++;
			}
		}
		if (!buf.empty())
			tokens.emplace_back(transition.m_tokenType, buf, row, col - buf.size() - 1);
		return tokens;
	}
}