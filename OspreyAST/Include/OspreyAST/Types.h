#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <optional>
#include <functional>

namespace Osprey
{
	enum class Qualifier
	{
		Mutable,
		Const,
	};

	enum class DataType
	{
		Bool,
		I32,
		F32,
	};

	struct FunctionType;

	class Type
	{
	public:
		Type(DataType type);

		std::string ToString() const;

	private:
		std::variant<DataType, FunctionType*> m_type;
	};

	struct FunctionType
	{
	public:
		FunctionType(std::vector<Type> parameters, Type return_type)
			: m_parameters(std::move(parameters))
			, m_return_type(return_type)
		{
		}

		const std::vector<Type>& GetParameters() const { return m_parameters; }

	private:
		std::vector<Type> m_parameters;
		Type m_return_type;
	};

	struct LabeledFunctionType
	{
	public:
		static std::optional<LabeledFunctionType> Create(std::vector<std::string> parameter_identifiers, FunctionType function_type);

		void ForEach(std::function<void(Type, const std::string&)> func) const;

	private:
		LabeledFunctionType(std::vector<std::string> parameter_identifiers, FunctionType function_type)
			: m_parameter_identifiers(std::move(parameter_identifiers))
			, m_function_type(std::move(function_type))
		{
		}

		std::vector<std::string> m_parameter_identifiers;
		FunctionType m_function_type;
	};

	struct ArgumentList
	{
		std::vector<std::unique_ptr<class ASTExpr>> args;
	};
}