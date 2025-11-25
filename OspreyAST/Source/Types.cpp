#include "OspreyAST/Types.h"

namespace Osprey
{
	Type::Type(DataType type)
		: m_type(type)
	{
	}

	std::string Type::ToString() const
	{
		return "Type"; // TODO
	}

	std::optional<LabeledFunctionType> LabeledFunctionType::Create(std::vector<std::string> parameter_identifiers, FunctionType function_type)
	{
		if (parameter_identifiers.size() != function_type.GetParameters().size())
		{
			return std::nullopt;
		}

		return LabeledFunctionType(std::move(parameter_identifiers), std::move(function_type));
	}

	void LabeledFunctionType::ForEach(std::function<void(Type, const std::string&)> func) const
	{
		const size_t size = m_parameter_identifiers.size();
		const std::vector<Type>& parameter_types = m_function_type.GetParameters();

		for (size_t i = 0; i < size; ++i)
		{
			func(parameter_types[i], m_parameter_identifiers[i]);
		}
	}
}