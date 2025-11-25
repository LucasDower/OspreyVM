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
		Divide,
		Percent,
		Return,
		Equality,
		NotEquality,
		Semicolon,
		Colon,
		LeftParen,
		RightParen,
		If,
		LeftCurly,
		RightCurly,
		I32,
		F32,
		Exclamation,
		Minus,
		Lt,
		LtEq,
		Gt,
		GtEq,
		And,
		Or,
		Comma,
		RightArrow,
		Mutable,
	};

	struct Token
	{
		TokenType type;
		std::string lexeme;
		size_t line;
		size_t column;
	};
}