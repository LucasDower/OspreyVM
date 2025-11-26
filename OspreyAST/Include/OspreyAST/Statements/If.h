#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTBlock;
	class ASTExpr;

	class ASTIfStmt : public ASTStmt
	{
	public:
		ASTIfStmt(std::unique_ptr<ASTExpr> predicate, std::unique_ptr<ASTBlock> true_block);
		virtual ~ASTIfStmt() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::unique_ptr<ASTExpr>& GetPredicate() const;
		const std::unique_ptr<ASTBlock>& GetTrueBlock() const;

	private:
		std::unique_ptr<ASTExpr> m_predicate;
		std::unique_ptr<ASTBlock> m_true_block;
	};
}