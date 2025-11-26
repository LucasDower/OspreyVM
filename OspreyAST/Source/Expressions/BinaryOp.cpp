#include "OspreyAST/Expressions/BinaryOp.h"

#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTBinaryExpr::ASTBinaryExpr(BinaryOperator op, std::unique_ptr<ASTExpr> left, std::unique_ptr<ASTExpr> right)
		: m_op(op)
		, m_left_node(std::move(left))
		, m_right_node(std::move(right))
	{
	}

	ASTVisitorTraversal ASTBinaryExpr::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	BinaryOperator ASTBinaryExpr::GetOperator() const
	{
		return m_op;
	}

	const std::unique_ptr<ASTExpr>& ASTBinaryExpr::GetLeftNode() const
	{
		return m_left_node;
	}

	const std::unique_ptr<ASTExpr>& ASTBinaryExpr::GetRightNode() const
	{
		return m_right_node;
	}
}