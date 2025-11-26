#include "OspreyAST/AST.h"

#include "OspreyAST/ASTVisitor.h"

#include <print>

namespace Osprey
{
	ASTProgram::ASTProgram(std::vector<std::unique_ptr<ASTStmt>> statements)
		: m_statements(std::move(statements))
	{
	}

	ASTVisitorTraversal ASTProgram::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	AST::AST(std::unique_ptr<ASTProgram> root)
		: m_root(std::move(root))
	{
	}

	const std::unique_ptr<ASTProgram>& AST::GetRoot() const
	{
		return m_root;
	}
}