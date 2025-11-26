#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTBlock : public ASTStmt
	{
	public:
		ASTBlock(std::vector<std::unique_ptr<ASTStmt>> statements);
		virtual ~ASTBlock() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::vector<std::unique_ptr<ASTStmt>>& GetStatements() const;

	private:
		std::vector<std::unique_ptr<ASTStmt>> m_statements;
	};
}