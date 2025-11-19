#pragma once

#include <string>

#include "Types.h"

namespace Osprey
{
	enum class Type
	{
		I32,
		F32
	};

	static inline std::string TypeToString(Type type)
	{
		if (type == Type::I32)
		{
			return "i32";
		}
		if (type == Type::F32)
		{
			return "f32";
		}
		return "";
	}
}