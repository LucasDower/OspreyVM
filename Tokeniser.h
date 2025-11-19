#pragma once

#include <vector>
#include <optional>

#include "Token.h"

namespace Osprey
{
	using TokenBuffer = std::vector<Token>;

	std::optional<TokenBuffer> Tokenise(const std::string in_script);
}