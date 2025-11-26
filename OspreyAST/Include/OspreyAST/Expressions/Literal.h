#pragma once

#include "OspreyAST/AST.h"

namespace Osprey
{
	class ASTLiteral : public ASTExpr
	{
	public:
		ASTLiteral(Type type, int32_t value);
		virtual ~ASTLiteral() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		Type GetType() const { return m_type; }
		int32_t GetValue() const;

	private:
		Type m_type;
		int32_t m_value;
	};
}