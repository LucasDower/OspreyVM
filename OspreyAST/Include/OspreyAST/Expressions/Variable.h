#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTVariable : public ASTExpr
	{
	public:
		ASTVariable(std::string identifier);
		virtual ~ASTVariable() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const;

	private:
		std::string m_identifier;
	};
}