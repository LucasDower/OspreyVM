#pragma once

#include "OspreyAST/AST.h"
#include "OspreyAST/ASTVisitor.h"

#include <string>

namespace Osprey
{
	class ASTDump : public ASTVisitor
	{
	public:
		ASTVisitorTraversal Visit(const class ASTLiteral& node);
		ASTVisitorTraversal Visit(const class ASTVariable& node);
		ASTVisitorTraversal Visit(const class ASTUnaryExpr& node);
		ASTVisitorTraversal Visit(const class ASTBinaryExpr& node);
		ASTVisitorTraversal Visit(const class ASTVariableDeclarationStmt& node);
		ASTVisitorTraversal Visit(const class ASTReturn& node);
		ASTVisitorTraversal Visit(const class ASTBlock& node);
		ASTVisitorTraversal Visit(const class ASTAssignmentStmt& node);
		ASTVisitorTraversal Visit(const class ASTIfStmt& node);
		ASTVisitorTraversal Visit(const class ASTFunctionCall& node);
		ASTVisitorTraversal Visit(const class ASTFunctionDeclaration& node);
		ASTVisitorTraversal Visit(const class ASTProgram& Node);

	private:
		void PrintIndented(std::string message);

		size_t m_indent = 0;
	};
}