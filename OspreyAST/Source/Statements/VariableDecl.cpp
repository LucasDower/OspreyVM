#include "OspreyAST/Statements/VariableDecl.h"

#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTVariableDeclarationStmt::ASTVariableDeclarationStmt(std::string identifier, Type type, std::unique_ptr<ASTExpr> expression)
		: m_identifier(std::move(identifier))
		, m_type(type)
		, m_expression_node(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTVariableDeclarationStmt::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::string& ASTVariableDeclarationStmt::GetIdentifier() const
	{
		return m_identifier;
	}

	const std::unique_ptr<ASTExpr>& ASTVariableDeclarationStmt::GetExpressionNode() const
	{
		return m_expression_node;
	}
}