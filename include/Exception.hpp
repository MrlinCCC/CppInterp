#pragma once
#include <stdexcept>
#include <string>

class LangException : public std::runtime_error {
public:
	LangException(const std::string& errorType,
		const std::string& message,
		int row,
		int col)
		: std::runtime_error(BuildMessage(errorType, message, row, col)),
		m_errorType(errorType),
		m_message(message),
		m_row(row),
		m_col(col) {
	}

	inline const std::string& GetErrorType() const noexcept { return m_errorType; }
	inline const std::string& GetMessage() const noexcept { return m_message; }
	inline int GetRow() const noexcept { return m_row; }
	inline int GetCol() const noexcept { return m_col; }

private:
	std::string m_errorType;
	std::string m_message;
	int m_row;
	int m_col;

	inline static std::string BuildMessage(const std::string& errorType,
		const std::string& message,
		int row,
		int col) {
		return errorType + ": " + message +
			" at line " + std::to_string(row) +
			", column " + std::to_string(col);
	}
};


class LexerException : public LangException {
public:
	LexerException(const std::string& message, char ch, int row, int col)
		: LangException("LexerError", std::string(1, ch) + " " + message, row, col),
		m_ch(ch) {
	}

	inline char GetChar() const noexcept { return m_ch; }

private:
	char m_ch;
};

class ParserException : public LangException {
public:
	ParserException(const std::string& message, int row, int col)
		: LangException("ParserError", message, row, col) {
	}
};

class SemanticException : public LangException {
public:
	SemanticException(const std::string& message, int row, int col)
		: LangException("SemanticError", message, row, col) {
	}
};
