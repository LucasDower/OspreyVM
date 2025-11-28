#include "OspreyAST/Expressions/FunctionExpression.h"

#include "OspreyAST/ASTVisitor.h"
#include "OspreyAST/Statements/Block.h"

namespace Osprey
{
	ASTFunctionExpr::ASTFunctionExpr(std::vector<FunctionParameter> parameters, Type return_type, std::unique_ptr<ASTBlock> body)
		: m_parameters(std::move(parameters))
		, m_return_type(std::move(return_type))
		, m_body(std::move(body))
	{
	}

	ASTVisitorTraversal ASTFunctionExpr::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const ParameterList& ASTFunctionExpr::GetParameters() const
	{
		return m_parameters;
	}

	Type ASTFunctionExpr::GetReturnType() const
	{
		return m_return_type;
	}

	const std::unique_ptr<ASTBlock>& ASTFunctionExpr::GetBody() const
	{
		return m_body;
	}
}