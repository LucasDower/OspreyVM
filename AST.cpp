#include "AST.h"

#include "ASTVisitor.h"

#include <print>

namespace Osprey
{
	// ASTIntegerLiteralNode

	ASTLiteralNode::ASTLiteralNode(Type type, int32_t value)
		: m_type(type)
		, m_value(value)
	{
	}

	ASTVisitorTraversal ASTLiteralNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	int32_t ASTLiteralNode::GetValue() const
	{
		return m_value;
	}

	// ASTIntegerVariableNode

	ASTVariableNode::ASTVariableNode(Type type, std::string identifier)
		: m_type(type)
		, m_identifier(std::move(identifier))
	{
	}

	ASTVisitorTraversal ASTVariableNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	std::optional<Type> ASTVariableNode::GetType() const
	{
		return m_type;
	}

	const std::string& ASTVariableNode::GetIdentifier() const
	{
		return m_identifier;
	}

	// ASTAddNode

	ASTAddNode::ASTAddNode(std::unique_ptr<ASTTypedNode> left, std::unique_ptr<ASTTypedNode> right)
		: m_left_node(std::move(left))
		, m_right_node(std::move(right))
	{
	}

	ASTVisitorTraversal ASTAddNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	std::optional<Type> ASTAddNode::GetType() const
	{
		const std::optional<Type> left_type = m_left_node->GetType();
		if (!left_type)
		{
			std::println("Failed to get type of left node in add expression");
			return std::nullopt;
		}

		const std::optional<Type> right_type = m_right_node->GetType();
		if (!right_type)
		{
			std::println("Failed to get type of right node in add expression");
			return std::nullopt;
		}

		if (*left_type == Type::I32 && right_type == Type::I32)
		{
			return Type::I32;
		}

		std::println("Types '{}' and '{}' cannot be added", TypeToString(*left_type), TypeToString(*right_type));
		return std::nullopt;
	}

	const std::unique_ptr<ASTTypedNode>& ASTAddNode::GetLeftNode() const
	{
		return m_left_node;
	}

	const std::unique_ptr<ASTTypedNode>& ASTAddNode::GetRightNode() const
	{
		return m_right_node;
	}

	// ASTAssignNode

	ASTVariableDeclarationNode::ASTVariableDeclarationNode(std::string identifier, Type type, std::unique_ptr<ASTTypedNode> expression)
		: m_identifier(std::move(identifier))
		, m_type(type)
		, m_expression_node(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTVariableDeclarationNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::string& ASTVariableDeclarationNode::GetIdentifier() const
	{
		return m_identifier;
	}

	const std::unique_ptr<ASTTypedNode>& ASTVariableDeclarationNode::GetExpressionNode() const
	{
		return m_expression_node;
	}

	// ASTReturnNode

	ASTReturnNode::ASTReturnNode(std::unique_ptr<ASTTypedNode> expression)
		: m_expression_node(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTReturnNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::unique_ptr<ASTTypedNode>& ASTReturnNode::GetExpressionNode() const
	{
		return m_expression_node;
	}

	// ASTBlockNode

	ASTBlockNode::ASTBlockNode(std::vector<std::unique_ptr<ASTNode>> expressions)
		: m_expression_nodes(std::move(expressions))
	{
	}

	ASTVisitorTraversal ASTBlockNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::vector<std::unique_ptr<ASTNode>>& ASTBlockNode::GetExpressionNodes() const
	{
		return m_expression_nodes;
	}

	// AST

	AST::AST(std::unique_ptr<ASTNode> root)
		: m_root(std::move(root))
	{
	}

	const std::unique_ptr<ASTNode>& AST::GetRoot() const
	{
		return m_root;
	}
}