#pragma once

#include "OspreyAST/Token.h"

namespace Osprey
{
	enum class UnaryOperator
	{
		Exclamation,
		Minus,
	};

	enum class BinaryOperator
	{
		Asterisk,
		Plus,
		Minus,
		Divide,
		Percent,
		Lt,
		LtEq,
		Gt,
		GtEq,
		Equality,
		NotEquality,
		And,
		Or
	};
}