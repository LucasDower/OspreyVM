#pragma once

#include "Types.h"
#include "BinaryOperator.h"

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <optional>

namespace Osprey
{
	class ASTVisitor;
	enum class ASTVisitorTraversal;

	class ASTNode
	{
	public:
		virtual ~ASTNode() = default;

		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const = 0;
	};

	class ASTExpr : public ASTNode
	{
	public:
		virtual Type GetType() const = 0;
	};

	class ASTLiteral : public ASTExpr
	{
	public:
		ASTLiteral(Type type, int32_t value);
		virtual ~ASTLiteral() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		// ASTTypedNode
		virtual Type GetType() const override { return m_type; }

		int32_t GetValue() const;

	private:
		Type m_type;
		int32_t m_value;
	};

	class ASTVariable : public ASTExpr
	{
	public:
		ASTVariable(Type type, std::string identifier);
		virtual ~ASTVariable() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		// ASTTypedNode
		virtual Type GetType() const override;

		const std::string& GetIdentifier() const;

	private:
		Type m_type;
		std::string m_identifier;
	};

	class ASTUnaryExpr : public ASTExpr
	{
	public:
		ASTUnaryExpr(UnaryOperator op, std::unique_ptr<ASTExpr> node);
		virtual ~ASTUnaryExpr() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		// ASTTypedNode
		virtual Type GetType() const override;

		UnaryOperator GetOperator() const;
		const std::unique_ptr<ASTExpr>& GetNode() const;

	private:
		UnaryOperator m_op;
		std::unique_ptr<ASTExpr> m_node;
		Type m_type;
	};
	
	class ASTBinaryExpr : public ASTExpr
	{
	public:
		static std::unique_ptr<ASTBinaryExpr> Create(BinaryOperator op, std::unique_ptr<ASTExpr> left, std::unique_ptr<ASTExpr> right);

		virtual ~ASTBinaryExpr() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		// ASTTypedNode
		virtual Type GetType() const override;

		BinaryOperator GetOperator() const;
		const std::unique_ptr<ASTExpr>& GetLeftNode() const;
		const std::unique_ptr<ASTExpr>& GetRightNode() const;

		ASTBinaryExpr(BinaryOperator op, std::unique_ptr<ASTExpr> left, std::unique_ptr<ASTExpr> right, Type type);
	private:

		BinaryOperator m_op;
		std::unique_ptr<ASTExpr> m_left_node;
		std::unique_ptr<ASTExpr> m_right_node;
		Type m_type;
	};

	class ASTVariableDeclarationStmt : public ASTNode
	{
	public:
		ASTVariableDeclarationStmt(std::string identifier, Type type, std::unique_ptr<ASTExpr> expression);
		virtual ~ASTVariableDeclarationStmt() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const;
		const std::unique_ptr<ASTExpr>& GetExpressionNode() const;

	private:
		std::string m_identifier;
		Type m_type;
		std::unique_ptr<ASTExpr> m_expression_node;
	};

	class ASTAssignmentStmt : public ASTNode
	{
	public:
		ASTAssignmentStmt(std::string identifier, std::unique_ptr<ASTExpr> expression);
		virtual ~ASTAssignmentStmt() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const { return m_identifier; }
		const std::unique_ptr<ASTExpr>& GetExpressionNode() const { return m_expr; }

	private:
		std::string m_identifier;
		Type m_type;
		std::unique_ptr<ASTExpr> m_expr;
	};

	class ASTReturn : public ASTNode
	{
	public:
		ASTReturn(std::unique_ptr<ASTExpr> expression);
		virtual ~ASTReturn() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::unique_ptr<ASTExpr>& GetExpressionNode() const;

	private:
		std::unique_ptr<ASTExpr> m_expression_node;
	};

	class ASTBlock : public ASTNode
	{
	public:
		ASTBlock(std::vector<std::unique_ptr<ASTNode>> expressions);
		virtual ~ASTBlock() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::vector<std::unique_ptr<ASTNode>>& GetExpressionNodes() const;

	private:
		std::vector<std::unique_ptr<ASTNode>> m_expression_nodes;
	};

	class ASTIfStmt : public ASTNode
	{
	public:
		ASTIfStmt(std::unique_ptr<ASTExpr> predicate, std::unique_ptr<ASTBlock> true_block);
		virtual ~ASTIfStmt() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::unique_ptr<ASTExpr>& GetPredicate() const { return m_predicate; }
		const std::unique_ptr<ASTBlock>& GetTrueBlock() const { return m_true_block; }

	private:
		std::unique_ptr<ASTExpr> m_predicate;
		std::unique_ptr<ASTBlock> m_true_block;
	};

	class AST
	{
	public:
		AST(std::unique_ptr<ASTNode> root);

		const std::unique_ptr<ASTNode>& GetRoot() const;

	private:
		std::unique_ptr<ASTNode> m_root;
	};
}