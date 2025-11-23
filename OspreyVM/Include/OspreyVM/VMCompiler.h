#pragma once

#include <optional>

namespace Osprey
{
	class VMProgram;
	class AST;

	std::optional<VMProgram> Compile(const AST& ast);
}