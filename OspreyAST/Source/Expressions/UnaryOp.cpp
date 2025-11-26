#include "OspreyAST/Expressions/UnaryOp.h"

#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTUnaryExpr::ASTUnaryExpr(UnaryOperator op, std::unique_ptr<ASTExpr> node)
		: m_op(op)
		, m_node(std::move(node))
	{
	}

	ASTVisitorTraversal ASTUnaryExpr::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	UnaryOperator ASTUnaryExpr::GetOperator() const
	{
		return m_op;
	}

	const std::unique_ptr<ASTExpr>& ASTUnaryExpr::GetNode() const
	{
		return m_node;
	}
}