#include "OspreyAST/Expressions/Variable.h"

#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTVariable::ASTVariable(std::string identifier)
		: m_identifier(std::move(identifier))
	{
	}

	ASTVisitorTraversal ASTVariable::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::string& ASTVariable::GetIdentifier() const
	{
		return m_identifier;
	}
}