#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTBinaryExpr : public ASTExpr
	{
	public:
		ASTBinaryExpr(BinaryOperator op, std::unique_ptr<ASTExpr> left, std::unique_ptr<ASTExpr> right);
		virtual ~ASTBinaryExpr() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		BinaryOperator GetOperator() const;
		const std::unique_ptr<ASTExpr>& GetLeftNode() const;
		const std::unique_ptr<ASTExpr>& GetRightNode() const;

	private:
		BinaryOperator m_op;
		std::unique_ptr<ASTExpr> m_left_node;
		std::unique_ptr<ASTExpr> m_right_node;
	};
}