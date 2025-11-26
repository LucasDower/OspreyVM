#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTUnaryExpr : public ASTExpr
	{
	public:
		ASTUnaryExpr(UnaryOperator op, std::unique_ptr<ASTExpr> node);
		virtual ~ASTUnaryExpr() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		UnaryOperator GetOperator() const;
		const std::unique_ptr<ASTExpr>& GetNode() const;

	private:
		UnaryOperator m_op;
		std::unique_ptr<ASTExpr> m_node;
	};
}