#include "OspreyAST/Expressions/Literal.h"

#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTLiteral::ASTLiteral(Type type, int32_t value)
		: m_type(type)
		, m_value(value)
	{
	}

	ASTVisitorTraversal ASTLiteral::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	int32_t ASTLiteral::GetValue() const
	{
		return m_value;
	}
}