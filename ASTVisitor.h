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
		virtual ASTVisitorTraversal Visit(const class ASTLiteralNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTVariableNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTAddNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTVariableDeclarationNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTReturnNode& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTBlockNode& Node) = 0;
	};
}