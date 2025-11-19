#pragma once

#include <vector>
#include <optional>

#include "Token.h"
#include "AST.h"

namespace Osprey
{
	using TokenBuffer = std::vector<Token>;

	std::optional<AST> Parse(const TokenBuffer& tokens);
}