#pragma once

#include "OspreyAST/Types.h"
#include "OspreyAST/BinaryOperator.h"

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <optional>

namespace Osprey
{
	class ASTVisitor;
	enum class ASTVisitorTraversal;

	/*
		All nodes in the AST must derive from ASTNode but should
		derive from either ASTStmt or ASTExpr.
	*/
	class ASTNode
	{
	public:
		virtual ~ASTNode() = default;

		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const = 0;
	};

	/*
		Statements are units of code that perform an action but
		does not produce a value itself.

		For example: if, while, assignment, ...
	*/
	class ASTStmt : public ASTNode
	{
	};

	/*
		Expressions are units of code that produce values.

		For example: 5, x, 1 + 2, ...
	*/
	class ASTExpr : public ASTNode
	{
	};

	class ASTProgram : public ASTNode
	{
	public:
		ASTProgram(std::vector<std::unique_ptr<ASTStmt>> statements);
		virtual ~ASTProgram() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::vector<std::unique_ptr<ASTStmt>>& GetStatements() const { return m_statements; }

	private:
		std::vector<std::unique_ptr<ASTStmt>> m_statements;
	};

	class AST
	{
	public:
		AST(std::unique_ptr<ASTProgram> root);

		const std::unique_ptr<ASTProgram>& GetRoot() const;

	private:
		std::unique_ptr<ASTProgram> m_root;
	};
}