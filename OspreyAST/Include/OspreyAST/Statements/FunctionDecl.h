#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTFunctionDeclarationStmt : public ASTStmt
	{
	public:
		ASTFunctionDeclarationStmt();
		virtual ~ASTFunctionDeclarationStmt() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

	private:
	};
}