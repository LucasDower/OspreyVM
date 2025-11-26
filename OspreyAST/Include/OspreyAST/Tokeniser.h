#pragma once

#include "OspreyAST/Token.h"

#include <vector>
#include <expected>

namespace Osprey
{
	using TokenBuffer = std::vector<Token>;
	using ErrorMessage = std::string;

	std::expected<TokenBuffer, ErrorMessage> Tokenise(const std::string in_script);
}