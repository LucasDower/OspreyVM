#include "OspreyAST/Statements/Return.h"

#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTReturn::ASTReturn(std::unique_ptr<ASTExpr> expression)
		: m_expression_node(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTReturn::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::unique_ptr<ASTExpr>& ASTReturn::GetExpressionNode() const
	{
		return m_expression_node;
	}
}