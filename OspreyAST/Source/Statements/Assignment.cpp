#include "OspreyAST/Statements/Assignment.h"

#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTAssignmentStmt::ASTAssignmentStmt(std::string identifier, std::unique_ptr<ASTExpr> expression)
		: m_identifier(identifier)
		, m_expr(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTAssignmentStmt::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}
}