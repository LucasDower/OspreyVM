#include "OspreyAST/Statements/Block.h"

#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTBlock::ASTBlock(std::vector<std::unique_ptr<ASTStmt>> statements)
		: m_statements(std::move(statements))
	{
	}

	ASTVisitorTraversal ASTBlock::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::vector<std::unique_ptr<ASTStmt>>& ASTBlock::GetStatements() const
	{
		return m_statements;
	}
}