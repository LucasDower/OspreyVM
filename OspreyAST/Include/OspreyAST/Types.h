#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Osprey
{
	enum class Type
	{
		Bool,
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

	struct FunctionType
	{
		std::vector<std::pair<std::string, Type>> parameters;
		Type return_type;
	};

	struct ArgumentList
	{
		std::vector<std::unique_ptr<class ASTExpr>> args;
	};
}