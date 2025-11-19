#include "ASTVisitor.h"

namespace Osprey
{
	void ASTIntegerLiteralNode::Accept(ASTVisitor& visitor) const
	{
		visitor.Visit(*this);
	}
}