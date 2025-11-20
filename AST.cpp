#include "AST.h"

#include "ASTVisitor.h"

#include <print>

namespace Osprey
{
	// ASTIntegerLiteralNode

	ASTLiteral::ASTLiteral(Type type, int32_t value)
		: m_type(type)
		, m_value(value)
	{
	}

	ASTVisitorTraversal ASTLiteral::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	int32_t ASTLiteral::GetValue() const
	{
		return m_value;
	}

	// ASTIntegerVariableNode

	ASTVariable::ASTVariable(Type type, std::string identifier)
		: m_type(type)
		, m_identifier(std::move(identifier))
	{
	}

	ASTVisitorTraversal ASTVariable::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	Type ASTVariable::GetType() const
	{
		return m_type;
	}

	const std::string& ASTVariable::GetIdentifier() const
	{
		return m_identifier;
	}

	// ASTIntegerVariableNode

	ASTUnaryExpr::ASTUnaryExpr(UnaryOperator op, std::unique_ptr<ASTExpr> node)
		: m_op(op)
		, m_node(std::move(node))
		, m_type(m_node->GetType())
	{
	}

	ASTVisitorTraversal ASTUnaryExpr::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	UnaryOperator ASTUnaryExpr::GetOperator() const
	{
		return m_op;
	}

	const std::unique_ptr<ASTExpr>& ASTUnaryExpr::GetNode() const
	{
		return m_node;
	}

	Type ASTUnaryExpr::GetType() const
	{
		return m_type;
	}

	// ASTAddNode

	ASTBinaryExpr::ASTBinaryExpr(BinaryOperator op, std::unique_ptr<ASTExpr> left, std::unique_ptr<ASTExpr> right, Type type)
		: m_op(op)
		, m_left_node(std::move(left))
		, m_right_node(std::move(right))
		, m_type(type)
	{
	}

	std::unique_ptr<ASTBinaryExpr> ASTBinaryExpr::Create(BinaryOperator op, std::unique_ptr<ASTExpr> left, std::unique_ptr<ASTExpr> right)
	{
		if (left && right)
		{
			const Type left_type = left->GetType();
			const Type right_type = right->GetType();

			if (left_type == right_type)
			{
				std::optional<Type> result_type;
				switch (op)
				{
					case BinaryOperator::Equality:
					{
						result_type = Type::Bool;
						break;
					}
					case BinaryOperator::Plus:
					case BinaryOperator::Minus:
					case BinaryOperator::Asterisk:
					{
						result_type = left_type;
						break;
					}
				}

				if (!result_type)
				{
					std::println("Could not create result type for binary expression");
					return nullptr;
				}

				return std::make_unique<ASTBinaryExpr>(op, std::move(left), std::move(right), *result_type);
			}
		}

		return nullptr;
	}

	ASTVisitorTraversal ASTBinaryExpr::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	Type ASTBinaryExpr::GetType() const
	{
		return m_type;
	}

	BinaryOperator ASTBinaryExpr::GetOperator() const
	{
		return m_op;
	}

	const std::unique_ptr<ASTExpr>& ASTBinaryExpr::GetLeftNode() const
	{
		return m_left_node;
	}

	const std::unique_ptr<ASTExpr>& ASTBinaryExpr::GetRightNode() const
	{
		return m_right_node;
	}

	// ASTVariableDeclarationNode

	ASTVariableDeclarationStmt::ASTVariableDeclarationStmt(std::string identifier, Type type, std::unique_ptr<ASTExpr> expression)
		: m_identifier(std::move(identifier))
		, m_type(type)
		, m_expression_node(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTVariableDeclarationStmt::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::string& ASTVariableDeclarationStmt::GetIdentifier() const
	{
		return m_identifier;
	}

	const std::unique_ptr<ASTExpr>& ASTVariableDeclarationStmt::GetExpressionNode() const
	{
		return m_expression_node;
	}

	// ASTReturnNode

	ASTReturn::ASTReturn(std::unique_ptr<ASTExpr> expression)
		: m_expression_node(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTReturn::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::unique_ptr<ASTExpr>& ASTReturn::GetExpressionNode() const
	{
		return m_expression_node;
	}

	// ASTBlockNode

	ASTBlock::ASTBlock(std::vector<std::unique_ptr<ASTNode>> expressions)
		: m_expression_nodes(std::move(expressions))
	{
	}

	ASTVisitorTraversal ASTBlock::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::vector<std::unique_ptr<ASTNode>>& ASTBlock::GetExpressionNodes() const
	{
		return m_expression_nodes;
	}

	// ASTIfStmt

	ASTIfStmt::ASTIfStmt(std::unique_ptr<ASTExpr> predicate, std::unique_ptr<ASTBlock> true_block)
		: m_predicate(std::move(predicate))
		, m_true_block(std::move(true_block))
	{
	}

	ASTVisitorTraversal ASTIfStmt::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	// ASTAssignmentStmt

	ASTAssignmentStmt::ASTAssignmentStmt(std::string identifier, std::unique_ptr<ASTExpr> expression)
		: m_identifier(identifier)
		, m_expr(std::move(expression))
	{
	}

	ASTVisitorTraversal ASTAssignmentStmt::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
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