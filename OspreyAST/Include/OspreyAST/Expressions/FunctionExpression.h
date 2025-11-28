#pragma once

#include "OspreyAST/AST.h"

#include <vector>

namespace Osprey
{
	class ASTBlock;

	class FunctionParameter
	{
	public:
		FunctionParameter(std::string identifier, Type type);

	private:
		std::string m_identifier;
		Type m_type;
	};

	using ParameterList = std::vector<FunctionParameter>;

	class ASTFunctionExpr : public ASTExpr
	{
	public:
		ASTFunctionExpr(std::vector<FunctionParameter> parameters, Type return_type, std::unique_ptr<ASTBlock> body);
		virtual ~ASTFunctionExpr() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const ParameterList& GetParameters() const;
		Type GetReturnType() const;
		const std::unique_ptr<ASTBlock>& GetBody() const;

	private:
		ParameterList m_parameters;
		Type m_return_type;
		std::unique_ptr<ASTBlock> m_body;
	};
}