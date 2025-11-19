#include "AST.h"

#include "ASTVisitor.h"

namespace Osprey
{
	// ASTIntegerLiteralNode

	ASTIntegerLiteralNode::ASTIntegerLiteralNode(int32_t value)
		: m_value(value)
	{
	}

	ASTVisitorTraversal ASTIntegerLiteralNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	int32_t ASTIntegerLiteralNode::GetValue() const
	{
		return m_value;
	}

	// ASTIntegerVariableNode

	ASTIntegerVariableNode::ASTIntegerVariableNode(std::string identifier)
		: m_identifier(std::move(identifier))
	{
	}

	ASTVisitorTraversal ASTIntegerVariableNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::string& ASTIntegerVariableNode::GetIdentifier() const
	{
		return m_identifier;
	}

	// ASTIntegerAddNode

	ASTIntegerAddNode::ASTIntegerAddNode(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
		: m_left_node(std::move(left))
		, m_right_node(std::move(right))
	{
	}

	ASTVisitorTraversal ASTIntegerAddNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::unique_ptr<ASTNode>& ASTIntegerAddNode::GetLeftNode() const
	{
		return m_left_node;
	}

	const std::unique_ptr<ASTNode>& ASTIntegerAddNode::GetRightNode() const
	{
		return m_right_node;
	}

	// ASTIntegerVariableAssignNode

	ASTIntegerVariableAssignNode::ASTIntegerVariableAssignNode(std::string identifier, std::unique_ptr<ASTNode> expression)
		: m_identifier(std::move(identifier))
		, m_expression_node(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTIntegerVariableAssignNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::string& ASTIntegerVariableAssignNode::GetIdentifier() const
	{
		return m_identifier;
	}

	const std::unique_ptr<ASTNode>& ASTIntegerVariableAssignNode::GetExpressionNode() const
	{
		return m_expression_node;
	}

	// ASTReturnNode

	ASTReturnNode::ASTReturnNode(std::unique_ptr<ASTNode> expression)
		: m_expression_node(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTReturnNode::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::unique_ptr<ASTNode>& ASTReturnNode::GetExpressionNode() const
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