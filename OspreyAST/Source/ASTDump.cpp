#include "OspreyAST/ASTDump.h"

#include "OspreyAST/Expressions/Literal.h"
#include "OspreyAST/Expressions/Variable.h"
#include "OspreyAST/Expressions/UnaryOp.h"
#include "OspreyAST/Expressions/BinaryOp.h"
#include "OspreyAST/Expressions/FunctionCall.h"
#include "OspreyAST/Statements/VariableDecl.h"
#include "OspreyAST/Statements/Return.h"
#include "OspreyAST/Statements/If.h"
#include "OspreyAST/Statements/Block.h"

#include <print>

namespace Osprey
{
	ASTVisitorTraversal ASTDump::Visit(const ASTLiteral& node)
	{
		PrintIndented(std::format("literal ({}, {})", node.GetType().ToString(), node.GetValue()));

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTVariable& node)
	{
		PrintIndented(std::format("variable ({})", node.GetIdentifier()));

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTUnaryExpr& node)
	{
		PrintIndented("unary_expression");

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTBinaryExpr& node)
	{
		PrintIndented("binary_expression");

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTVariableDeclarationStmt& node)
	{
		PrintIndented(std::format("variable_declaration ({}, {})", node.GetIdentifier(), node.GetType().ToString()));

		++m_indent;
		node.GetExpressionNode()->Accept(*this);
		--m_indent;

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTReturn& node)
	{
		PrintIndented("return_statement");

		++m_indent;
		node.GetExpressionNode()->Accept(*this);
		--m_indent;

		return ASTVisitorTraversal::Continue;
	}


	ASTVisitorTraversal ASTDump::Visit(const ASTAssignmentStmt& node)
	{
		PrintIndented("assignment_statement");

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTIfStmt& node)
	{
		PrintIndented("if_statement");

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTFunctionCall& node)
	{
		PrintIndented(std::format("function_call ({})", node.GetIdentifier()));

		++m_indent;
		for (const auto& arg : node.GetArgs().args)
		{
			arg->Accept(*this);
		}
		--m_indent;

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTBlock& node)
	{
		PrintIndented("block");

		++m_indent;
		for (const auto& expression : node.GetStatements())
		{
			expression->Accept(*this);
		}
		--m_indent;

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTProgram& node)
	{
		PrintIndented("program");
		
		++m_indent;
		for (const std::unique_ptr<ASTStmt>& statement : node.GetStatements())
		{
			statement->Accept(*this);
		}
		--m_indent;

		return ASTVisitorTraversal::Continue;
	}

	void ASTDump::PrintIndented(std::string message)
	{
		std::println("{}{}", std::string(m_indent * 4, ' '), message);
	}
}