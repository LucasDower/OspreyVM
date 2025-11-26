#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTAssignmentStmt : public ASTStmt
	{
	public:
		ASTAssignmentStmt(std::string identifier, std::unique_ptr<ASTExpr> expression);
		virtual ~ASTAssignmentStmt() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const { return m_identifier; }
		const std::unique_ptr<ASTExpr>& GetExpressionNode() const { return m_expr; }

	private:
		std::string m_identifier;
		std::unique_ptr<ASTExpr> m_expr;
	};
}