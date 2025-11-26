#pragma once

#include "OspreyAST/Token.h"
#include "OspreyAST/AST.h"

#include <vector>
#include <expected>

namespace Osprey
{
	using TokenBuffer = std::vector<Token>;
	using ErrorMessage = std::string;

	std::expected<AST, ErrorMessage> Parse(const TokenBuffer& tokens);
}