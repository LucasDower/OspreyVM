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

	class ASTFunctionExpr : public ASTExpr
	{
	public:
		ASTFunctionExpr(std::string identifier, ArgumentList args);
		virtual ~ASTFunctionExpr() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::vector<FunctionParameter>& GetParameters() const;
		Type GetReturnType() const;
		const std::unique_ptr<ASTBlock>& GetBody() const;

	private:
		std::vector<FunctionParameter> m_parameters;
		Type m_return_type;
		std::unique_ptr<ASTBlock> m_body;
	};
}