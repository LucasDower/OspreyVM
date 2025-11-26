#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTVariableDeclarationStmt : public ASTStmt
	{
	public:
		ASTVariableDeclarationStmt(std::string identifier, Type type, std::unique_ptr<ASTExpr> expression);
		virtual ~ASTVariableDeclarationStmt() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const;
		Type GetType() const { return m_type; }
		const std::unique_ptr<ASTExpr>& GetExpressionNode() const;

	private:
		std::string m_identifier;
		Type m_type;
		std::unique_ptr<ASTExpr> m_expression_node;
	};
}