#pragma once

#include "OspreyAST/Token.h"
#include "OspreyAST/AST.h"

#include <vector>
#include <optional>

namespace Osprey
{
	using TokenBuffer = std::vector<Token>;

	std::optional<AST> Parse(const TokenBuffer& tokens);
}