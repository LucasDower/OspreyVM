#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTReturn : public ASTStmt
	{
	public:
		ASTReturn(std::unique_ptr<ASTExpr> expression);
		virtual ~ASTReturn() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::unique_ptr<ASTExpr>& GetExpressionNode() const;

	private:
		std::unique_ptr<ASTExpr> m_expression_node;
	};
}