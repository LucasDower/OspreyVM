#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

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

	class ASTIntegerLiteralNode : public ASTNode
	{
	public:
		ASTIntegerLiteralNode(int32_t value);
		virtual ~ASTIntegerLiteralNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		int32_t GetValue() const;

	private:
		int32_t m_value;
	};

	class ASTIntegerVariableNode : public ASTNode
	{
	public:
		ASTIntegerVariableNode(std::string identifier);
		virtual ~ASTIntegerVariableNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const;

	private:
		std::string m_identifier;
	};

	class ASTIntegerAddNode : public ASTNode
	{
	public:
		ASTIntegerAddNode(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right);
		virtual ~ASTIntegerAddNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::unique_ptr<ASTNode>& GetLeftNode() const;
		const std::unique_ptr<ASTNode>& GetRightNode() const;

	private:
		std::unique_ptr<ASTNode> m_left_node;
		std::unique_ptr<ASTNode> m_right_node;
	};

	class ASTIntegerVariableAssignNode : public ASTNode
	{
	public:
		ASTIntegerVariableAssignNode(std::string identifier, std::unique_ptr<ASTNode> expression);
		virtual ~ASTIntegerVariableAssignNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::string& GetIdentifier() const;
		const std::unique_ptr<ASTNode>& GetExpressionNode() const;

	private:
		std::string m_identifier;
		std::unique_ptr<ASTNode> m_expression_node;
	};

	class ASTReturnNode : public ASTNode
	{
	public:
		ASTReturnNode(std::unique_ptr<ASTNode> expression);
		virtual ~ASTReturnNode() = default;

		// ASTNode
		virtual ASTVisitorTraversal Accept(ASTVisitor& visitor) const override;

		const std::unique_ptr<ASTNode>& GetExpressionNode() const;

	private:
		std::unique_ptr<ASTNode> m_expression_node;
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