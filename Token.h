#pragma once

#include <string>

namespace Osprey
{
	enum class TokenType
	{
		Identifier,
		Assign,
		Plus,
		Asterisk,
		Return,
		Equality,
		Semicolon,
		Colon,
		LeftParen,
		RightParen,
		I32,
		F32,
	};

	struct Token
	{
		TokenType type;
		std::string lexeme;
		size_t line;
		size_t column;
	};
}