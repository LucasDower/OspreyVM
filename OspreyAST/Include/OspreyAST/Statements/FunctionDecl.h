#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTFunctionExpr;

	class ASTFunctionDeclarationStmt : public ASTStmt
	{
	public:
		ASTFunctionDeclarationStmt(std::string identifier, std::unique_ptr<ASTFunctionExpr> function_expr);
		virtual ~ASTFunctionDeclarationStmt() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const { return m_identifier; }
		const std::unique_ptr<ASTFunctionExpr>& GetFunction() const { return m_function_expr; }

	private:
		std::string m_identifier;
		std::unique_ptr<ASTFunctionExpr> m_function_expr;
	};
}