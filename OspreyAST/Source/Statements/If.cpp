#include "OspreyAST/Statements/If.h"

#include "OspreyAST/ASTVisitor.h"
#include "OspreyAST/Statements/Block.h"

namespace Osprey
{
	ASTIfStmt::ASTIfStmt(std::unique_ptr<ASTExpr> predicate, std::unique_ptr<ASTBlock> true_block)
		: m_predicate(std::move(predicate))
		, m_true_block(std::move(true_block))
	{
	}

	ASTVisitorTraversal ASTIfStmt::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::unique_ptr<ASTExpr>& ASTIfStmt::GetPredicate() const
	{
		return m_predicate;
	}

	const std::unique_ptr<ASTBlock>& ASTIfStmt::GetTrueBlock() const
	{
		return m_true_block;
	}
}