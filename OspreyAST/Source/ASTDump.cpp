#include "OspreyAST/ASTDump.h"

#include <print>

namespace Osprey
{
	ASTVisitorTraversal ASTDump::Visit(const ASTLiteral& node)
	{
		PrintIndented(std::format("literal ({}, {})", TypeToString(node.GetType()), node.GetValue()));

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
		PrintIndented(std::format("variable_declaration ({}, {})", node.GetIdentifier(), TypeToString(node.GetType())));

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

	ASTVisitorTraversal ASTDump::Visit(const ASTFunctionDeclaration& node)
	{
		PrintIndented(std::format("function_declaration (\"{}\")", node.GetIdentifier()));

		++m_indent;
		node.GetBody()->Accept(*this);
		--m_indent;

		return ASTVisitorTraversal::Continue;
	}

	ASTVisitorTraversal ASTDump::Visit(const ASTProgram& node)
	{
		PrintIndented("program_declaration");
		
		++m_indent;
		for (const auto& function : node.GetFunctions())
		{
			function->Accept(*this);
		}
		--m_indent;

		return ASTVisitorTraversal::Continue;
	}

	void ASTDump::PrintIndented(std::string message)
	{
		std::println("{}{}", std::string(m_indent * 4, ' '), message);
	}
}