#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTExprStmt : public ASTStmt
	{
	public:
		ASTExprStmt(std::unique_ptr<ASTExpr> expr);
		virtual ~ASTExprStmt() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::unique_ptr<ASTExpr>& GetExpression() const { return m_expr; }

	private:
		std::unique_ptr<ASTExpr> m_expr;
	};
}