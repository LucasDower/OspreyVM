#pragma once

#include <cstdint>

namespace Osprey
{
	enum class VMOpCode : uint8_t
	{
		PUSH,
		ADD,
		LOAD,
		STORE,
		LT,
		JZ,
		JMP,
		HALT,
	};
}