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
		virtual ASTVisitorTraversal Visit(const class ASTLiteral& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTVariable& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTUnaryExpr& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTBinaryExpr& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTVariableDeclarationStmt& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTReturn& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTBlock& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTAssignmentStmt& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTIfStmt& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTProgram& Node) = 0;
		virtual ASTVisitorTraversal Visit(const class ASTFunctionCall& Node) = 0;
	};
}