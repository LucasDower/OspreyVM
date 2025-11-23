#pragma once

#include "OspreyAST/Token.h"

#include <vector>
#include <optional>

namespace Osprey
{
	using TokenBuffer = std::vector<Token>;

	std::optional<TokenBuffer> Tokenise(const std::string in_script);
}