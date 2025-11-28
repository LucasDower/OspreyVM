#include "OspreyAST/Statements/FunctionDecl.h"

#include "OspreyAST/Expressions/FunctionExpression.h"
#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTFunctionDeclarationStmt::ASTFunctionDeclarationStmt(std::string identifier, std::unique_ptr<ASTFunctionExpr> function_expr)
		: m_identifier(std::move(identifier))
		, m_function_expr(std::move(function_expr))
	{
	}

	ASTVisitorTraversal ASTFunctionDeclarationStmt::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}
}