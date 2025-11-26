#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTFunctionCall : public ASTExpr
	{
	public:
		ASTFunctionCall(std::string identifier, ArgumentList args);
		virtual ~ASTFunctionCall() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const { return m_identifier; }
		const ArgumentList& GetArgs() const { return m_args; }

	private:
		std::string m_identifier;
		ArgumentList m_args;
	};
}