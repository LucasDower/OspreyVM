#include "OspreyAST/AST.h"

#include "OspreyAST/ASTVisitor.h"

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

	ASTVariable::ASTVariable(std::string identifier)
		: m_identifier(std::move(identifier))
	{
	}

	ASTVisitorTraversal ASTVariable::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::string& ASTVariable::GetIdentifier() const
	{
		return m_identifier;
	}

	// ASTFunctionCall

	ASTFunctionCall::ASTFunctionCall(std::string identifier, ArgumentList args)
		: m_identifier(std::move(identifier))
		, m_args(std::move(args))
	{
	}

	ASTVisitorTraversal ASTFunctionCall::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	// ASTUnaryExpr

	ASTUnaryExpr::ASTUnaryExpr(UnaryOperator op, std::unique_ptr<ASTExpr> node)
		: m_op(op)
		, m_node(std::move(node))
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

	// ASTAddNode

	ASTBinaryExpr::ASTBinaryExpr(BinaryOperator op, std::unique_ptr<ASTExpr> left, std::unique_ptr<ASTExpr> right)
		: m_op(op)
		, m_left_node(std::move(left))
		, m_right_node(std::move(right))
	{
	}

	ASTVisitorTraversal ASTBinaryExpr::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
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

	ASTBlock::ASTBlock(std::vector<std::unique_ptr<ASTNode>> statements)
		: m_statements(std::move(statements))
	{
	}

	ASTVisitorTraversal ASTBlock::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	const std::vector<std::unique_ptr<ASTNode>>& ASTBlock::GetStatements() const
	{
		return m_statements;
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

	// ASTAssignmentStmt

	ASTFunctionDeclaration::ASTFunctionDeclaration(std::string identifier, FunctionType type, std::unique_ptr<ASTBlock> body)
		: m_identifier(std::move(identifier))
		, m_type(std::move(type))
		, m_body(std::move(body))
	{
	}

	ASTVisitorTraversal ASTFunctionDeclaration::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	// ASTProgram

	ASTProgram::ASTProgram(std::vector<std::unique_ptr<ASTFunctionDeclaration>> functions)
		: m_functions(std::move(functions))
	{
	}

	ASTVisitorTraversal ASTProgram::Accept(ASTVisitor& visitor) const
	{
		return visitor.Visit(*this);
	}

	// AST

	AST::AST(std::unique_ptr<ASTProgram> root)
		: m_root(std::move(root))
	{
	}

	const std::unique_ptr<ASTProgram>& AST::GetRoot() const
	{
		return m_root;
	}
}