#include "OspreyAST/Expressions/FunctionCall.h"

#include "OspreyAST/ASTVisitor.h"

namespace Osprey
{
	ASTFunctionCall::ASTFunctionCall(std::string identifier, ArgumentList args)
		: m_identifier(std::move(identifier))
		, m_args(std::move(args))
	{
	}

	ASTVisitorTraversal ASTFunctionCall::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}
}