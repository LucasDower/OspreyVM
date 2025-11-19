#pragma once

#include <string>

namespace Osprey
{
	enum class TokenType
	{
		Identifier,
		Assign,
		Plus,
		Return,
		Integer,
		Equality,
		Semicolon,
		LeftParen,
		RightParen,
	};

	struct Token
	{
		TokenType type;
		std::string lexeme;
		size_t line;
		size_t column;
	};
}