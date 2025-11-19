#pragma once

#include "Types.h"

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

	class ASTTypedNode : public ASTNode
	{
	public:
		virtual std::optional<Type> GetType() const = 0;
	};

	class ASTLiteralNode : public ASTTypedNode
	{
	public:
		ASTLiteralNode(Type type, int32_t value);
		virtual ~ASTLiteralNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		// ASTTypedNode
		virtual std::optional<Type> GetType() const override { return m_type; }

		int32_t GetValue() const;

	private:
		Type m_type;
		int32_t m_value;
	};

	class ASTVariableNode : public ASTTypedNode
	{
	public:
		ASTVariableNode(Type type, std::string identifier);
		virtual ~ASTVariableNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		// ASTTypedNode
		virtual std::optional<Type> GetType() const override;

		const std::string& GetIdentifier() const;

	private:
		Type m_type;
		std::string m_identifier;
	};

	class ASTAddNode : public ASTTypedNode
	{
	public:
		ASTAddNode(std::unique_ptr<ASTTypedNode> left, std::unique_ptr<ASTTypedNode> right);
		virtual ~ASTAddNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		// ASTTypedNode
		virtual std::optional<Type> GetType() const override;

		const std::unique_ptr<ASTTypedNode>& GetLeftNode() const;
		const std::unique_ptr<ASTTypedNode>& GetRightNode() const;

	private:
		std::unique_ptr<ASTTypedNode> m_left_node;
		std::unique_ptr<ASTTypedNode> m_right_node;
	};

	class ASTVariableDeclarationNode : public ASTNode
	{
	public:
		ASTVariableDeclarationNode(std::string identifier, Type type, std::unique_ptr<ASTTypedNode> expression);
		virtual ~ASTVariableDeclarationNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const;
		const std::unique_ptr<ASTTypedNode>& GetExpressionNode() const;

	private:
		std::string m_identifier;
		Type m_type;
		std::unique_ptr<ASTTypedNode> m_expression_node;
	};

	class ASTReturnNode : public ASTNode
	{
	public:
		ASTReturnNode(std::unique_ptr<ASTTypedNode> expression);
		virtual ~ASTReturnNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::unique_ptr<ASTTypedNode>& GetExpressionNode() const;

	private:
		std::unique_ptr<ASTTypedNode> m_expression_node;
	};

	class ASTBlockNode : public ASTNode
	{
	public:
		ASTBlockNode(std::vector<std::unique_ptr<ASTNode>> expressions);
		virtual ~ASTBlockNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::vector<std::unique_ptr<ASTNode>>& GetExpressionNodes() const;

	private:
		std::vector<std::unique_ptr<ASTNode>> m_expression_nodes;
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