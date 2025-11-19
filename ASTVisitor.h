#pragma once

namespace Osprey
{
	enum class ASTVisitorTraversal
	{
		Continue,
		Stop,
	};

	class ASTVisitor
	{
	public:
		virtual ASTVisitorTraversal Visit(const class ASTIntegerLiteralNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTIntegerVariableNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTIntegerAddNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTIntegerVariableAssignNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTReturnNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTBlockNode& Node) = 0;
	};
}