#include "OspreyAST/Parser.h"

#include "OspreyAST/Types.h"
#include "OspreyAST/BinaryOperator.h"
#include "OspreyAST/Expressions/Literal.h"
#include "OspreyAST/Expressions/Variable.h"
#include "OspreyAST/Expressions/UnaryOp.h"
#include "OspreyAST/Expressions/BinaryOp.h"
#include "OspreyAST/Expressions/FunctionCall.h"
#include "OspreyAST/Expressions/FunctionExpression.h"
#include "OspreyAST/Statements/VariableDecl.h"
#include "OspreyAST/Statements/Return.h"
#include "OspreyAST/Statements/If.h"
#include "OspreyAST/Statements/Block.h"
#include "OspreyAST/Statements/Assignment.h"
#include "OspreyAST/Statements/FunctionDecl.h"

#include <functional>
#include <memory>
#include <print>
#include <cassert>
#include <span>

namespace Osprey
{
	class TokenReader
	{
	public:
		TokenReader(const TokenBuffer& tokens)
			: m_tokens(tokens)
			, m_cursor(0)
		{
		}

		// Return the current token
		[[nodiscard]] std::optional<Token> Peek(const size_t lookahead = 0) const
		{
			if (m_cursor + lookahead < m_tokens.size())
			{
				return m_tokens[m_cursor + lookahead];
			}
			return std::nullopt;
		}

		// Return the current token and advance the cursor
		std::optional<Token> Consume()
		{
			std::optional<Token> token = Peek();
			++m_cursor;
			return token;
		}

		// Returns whether the current token is of the given type
		[[nodiscard]] bool MatchPeek(TokenType token_type, size_t lookahead = 0)
		{
			std::optional<Token> next = Peek(lookahead);
			return next && next->type == token_type;
		}

		// Returns the current token if it is of the given type and consumes it
		[[nodiscard]] std::optional<Token> MatchConsume(TokenType token_type)
		{
			if (MatchPeek(token_type))
			{
				return *Consume();
			}
			return std::nullopt;
		}

		[[nodiscard]] bool HasMore() const
		{
			return Peek().has_value();
		}

	private:
		const TokenBuffer& m_tokens;
		size_t m_cursor;
	};

	template<typename T>
	using ParseResult = std::expected<T, ErrorMessage>;
	template<typename T>
	using ParseResultPtr = std::expected<std::unique_ptr<T>, ErrorMessage>;

	// forward declarations
	ParseResultPtr<ASTExpr> ParseExpression(TokenReader& reader);
	ParseResultPtr<ASTBlock> ParseBlock(TokenReader& reader);
	ParseResult<ArgumentList> ParseArgumentList(TokenReader& reader);
	ParseResultPtr<ASTFunctionExpr> ParseFunctionExpression(TokenReader& reader);
	ParseResult<Type> ParseFunctionType(TokenReader& reader);

	// primary_expr := identifier
	//               | identifier '( argument_list? ')'
	//               | literal
	//               | '(' expr ')'
	//				 | function_expr
	ParseResultPtr<ASTExpr> ParsePrimaryExpr(TokenReader& reader)
	{
		if (!reader.HasMore())
		{
			return std::unexpected("Ran out of tokens when parsing primary expression");
		}

		if (std::optional<Token> i32_literal = reader.MatchConsume(TokenType::I32))
		{
			const int32_t value = std::stoi(i32_literal->lexeme); // TODO: throws if fails
			return std::make_unique<ASTLiteral>(Type(DataType::I32), value);
		}
		else if (std::optional<Token> identifier = reader.MatchConsume(TokenType::Identifier))
		{
			if (reader.MatchConsume(TokenType::LeftParen))
			{
				if (reader.MatchConsume(TokenType::RightParen))
				{
					return std::make_unique<ASTFunctionCall>(identifier->lexeme, ArgumentList());
				}
				
				std::expected<ArgumentList, ErrorMessage> arg_list = ParseArgumentList(reader);
				if (!arg_list)
				{
					return std::unexpected(arg_list.error());
				}
				
				return std::make_unique<ASTFunctionCall>(identifier->lexeme, std::move(*arg_list));
			}
			else
			{
				return std::make_unique<ASTVariable>(identifier->lexeme);
			}
		}
		else if (reader.MatchPeek(TokenType::LeftParen))
		{
			if (reader.MatchPeek(TokenType::RightParen) || (reader.MatchPeek(TokenType::Identifier, 1) && reader.MatchPeek(TokenType::Colon, 2)))
			{
				return ParseFunctionExpression(reader);
			}
			else
			{
				reader.Consume(); // Eat the '('

				std::expected<std::unique_ptr<ASTExpr>, ErrorMessage> expression = ParseExpression(reader);
				if (!expression)
				{
					return std::unexpected(expression.error());
				}

				if (!reader.MatchConsume(TokenType::RightParen))
				{
					return std::unexpected("Expected ')' at {} when parsing primary expression");
				}

				return std::move(expression);
			}
		}

		return std::unexpected("Unexpected '{}' when parsing primary expression");
	}

	// argument_list := expr (',' expr)*
	ParseResult<ArgumentList> ParseArgumentList(TokenReader& reader)
	{
		ArgumentList arg_list;

		ParseResultPtr<ASTExpr> expr = ParseExpression(reader);
		if (!expr)
		{
			return std::unexpected(expr.error());
		}

		arg_list.args.push_back(std::move(*expr));

		while (reader.MatchConsume(TokenType::Comma))
		{
			ParseResultPtr<ASTExpr> expr2 = ParseExpression(reader);
			if (!expr2)
			{
				return std::unexpected(expr.error());
			}

			arg_list.args.push_back(std::move(*expr2));
		}

		if (!reader.MatchConsume(TokenType::RightParen))
		{
			return std::unexpected("Expected ')' when parsing argument list");
		}

		return arg_list;
	}

	// unary_expr := "!" unary_expr
	//			   | "-" unary_expr
	//             | primary_expr
	ParseResultPtr<ASTExpr> ParseUnaryExpr(TokenReader& reader)
	{
		if (reader.MatchConsume(TokenType::Exclamation))
		{
			ParseResultPtr<ASTExpr> expr = ParseUnaryExpr(reader);
			if (!expr)
			{
				return std::unexpected(expr.error());
			}
			return std::make_unique<ASTUnaryExpr>(UnaryOperator::Exclamation, std::move(*expr));
		}
		else if (reader.MatchConsume(TokenType::Minus))
		{
			ParseResultPtr<ASTExpr> expr = ParseUnaryExpr(reader);
			if (!expr)
			{
				return std::unexpected(expr.error());
			}
			return std::make_unique<ASTUnaryExpr>(UnaryOperator::Minus, std::move(*expr));
		}
		else
		{
			return ParsePrimaryExpr(reader);
		}
	}

	// multiplicative_expr := unary_expr ('*' unary_expr)*
	//                      | unary_expr ('/' unary_expr)*
	//                      | unary_expr ('%' unary_expr)*
	ParseResultPtr<ASTExpr> ParseMultiplicativeExpr(TokenReader& reader)
	{
		ParseResultPtr<ASTExpr> left_expr = ParseUnaryExpr(reader);
		if (!left_expr)
		{
			return std::unexpected(left_expr.error());
		}

		while (true)
		{
			std::optional<BinaryOperator> operator_match;

			if (reader.MatchPeek(TokenType::Asterisk))
			{
				operator_match = BinaryOperator::Asterisk;
			}
			else if (reader.MatchPeek(TokenType::Divide))
			{
				operator_match = BinaryOperator::Divide;
			}
			else if (reader.MatchPeek(TokenType::Percent))
			{
				operator_match = BinaryOperator::Percent;
			}

			if (operator_match)
			{
				reader.Consume();

				ParseResultPtr<ASTExpr> right_expr = ParseUnaryExpr(reader);
				if (!right_expr)
				{
					return std::unexpected(right_expr.error());
				}

				left_expr = std::make_unique<ASTBinaryExpr>(*operator_match, std::move(*left_expr), std::move(*right_expr));
			}
			else
			{
				break;
			}
		}

		return left_expr;
	};

	// additive_expr := multiplicative_expr ('+' multiplicative_expr)*
		//                | multiplicative_expr ('-' multiplicative_expr)*
	ParseResultPtr<ASTExpr> ParseAdditiveExpr(TokenReader& reader)
	{
		ParseResultPtr<ASTExpr> left_expr = ParseMultiplicativeExpr(reader);
		if (!left_expr)
		{
			return std::unexpected(left_expr.error());
		}

		while (true)
		{
			std::optional<BinaryOperator> operator_match;

			if (reader.MatchPeek(TokenType::Plus))
			{
				operator_match = BinaryOperator::Plus;
			}
			else if (reader.MatchPeek(TokenType::Minus))
			{
				operator_match = BinaryOperator::Minus;
			}

			if (operator_match)
			{
				reader.Consume();

				ParseResultPtr<ASTExpr> right_expr = ParseMultiplicativeExpr(reader);
				if (!right_expr)
				{
					return std::unexpected(right_expr.error());
				}

				left_expr = std::make_unique<ASTBinaryExpr>(*operator_match, std::move(*left_expr), std::move(*right_expr));
			}
			else
			{
				break;
			}
		}

		return left_expr;
	}

	// relational_expr : = additive_expr('<' additive_expr) ?
	//                  | additive_expr ('<=' additive_expr)?
	//                  | additive_expr ('>' additive_expr)?
	//                  | additive_expr ('>=' additive_expr)?
	ParseResultPtr<ASTExpr> ParseRelationalExpr(TokenReader& reader)
	{
		ParseResultPtr<ASTExpr> left_expr = ParseAdditiveExpr(reader);
		if (!left_expr)
		{
			return std::unexpected(left_expr.error());
		}

		while (true)
		{
			std::optional<BinaryOperator> operator_match;

			if (reader.MatchPeek(TokenType::Lt))
			{
				operator_match = BinaryOperator::Lt;
			}
			else if (reader.MatchPeek(TokenType::LtEq))
			{
				operator_match = BinaryOperator::LtEq;
			}
			else if (reader.MatchPeek(TokenType::Gt))
			{
				operator_match = BinaryOperator::Gt;
			}
			else if (reader.MatchPeek(TokenType::GtEq))
			{
				operator_match = BinaryOperator::GtEq;
			}

			if (operator_match)
			{
				reader.Consume();

				ParseResultPtr<ASTExpr> right_expr = ParseMultiplicativeExpr(reader);
				if (!right_expr)
				{
					return std::unexpected(right_expr.error());
				}

				left_expr = std::make_unique<ASTBinaryExpr>(*operator_match, std::move(*left_expr), std::move(*right_expr));
			}
			else
			{
				break;
			}
		}

		return left_expr;
	}

	// equality_expr := relational_expr ('==' relational_expr)*
	//                | relational_expr ('!=' relational_expr)*
	ParseResultPtr<ASTExpr> ParseEqualityExpr(TokenReader& reader)
	{
		ParseResultPtr<ASTExpr> left_expr = ParseRelationalExpr(reader);
		if (!left_expr)
		{
			return std::unexpected(left_expr.error());
		}

		std::optional<BinaryOperator> operator_match;

		if (reader.MatchConsume(TokenType::Equality))
		{
			operator_match = BinaryOperator::Equality;
		}
		else if (reader.MatchConsume(TokenType::NotEquality))
		{
			operator_match = BinaryOperator::NotEquality;
		}
		else
		{
			return std::move(*left_expr);
		}

		ParseResultPtr<ASTExpr> right_expr = ParseRelationalExpr(reader);
		if (!right_expr)
		{
			return std::unexpected(right_expr.error());
		}

		return std::make_unique<ASTBinaryExpr>(*operator_match, std::move(*left_expr), std::move(*right_expr));
	}

	// logical_and_expr := equality_expr ('&&' equality_expr)?
	ParseResultPtr<ASTExpr> ParseLogicalAndExpr(TokenReader& reader)
	{
		ParseResultPtr<ASTExpr> left_expr = ParseEqualityExpr(reader);
		if (!left_expr)
		{
			return std::unexpected(left_expr.error());
		}

		if (reader.MatchConsume(TokenType::And))
		{
			ParseResultPtr<ASTExpr> right_expr = ParseEqualityExpr(reader);
			if (!right_expr)
			{
				return std::unexpected(right_expr.error());
			}

			return std::make_unique<ASTBinaryExpr>(BinaryOperator::And, std::move(*left_expr), std::move(*right_expr));
		}

		return left_expr;
	}

	// logical_or_expr := equality_expr ('||' equality_expr)?
	ParseResultPtr<ASTExpr> ParseLogicalOrExpr(TokenReader& reader)
	{
		ParseResultPtr<ASTExpr> left_expr = ParseLogicalAndExpr(reader);
		if (!left_expr)
		{
			return std::unexpected(left_expr.error());
		}

		if (reader.MatchConsume(TokenType::Or))
		{
			ParseResultPtr<ASTExpr> right_expr = ParseLogicalAndExpr(reader);
			if (!right_expr)
			{
				return std::unexpected(right_expr.error());
			}

			return std::make_unique<ASTBinaryExpr>(BinaryOperator::And, std::move(*left_expr), std::move(*right_expr));
		}

		return left_expr;
	};

	// expr := logical_or_expr
	ParseResultPtr<ASTExpr> ParseExpression(TokenReader& reader)
	{
		return ParseLogicalOrExpr(reader);
	}

	// return := "return" expr ";"
	ParseResultPtr<ASTReturn> ParseReturnStatement(TokenReader& reader)
	{
		if (!reader.MatchConsume(TokenType::Return))
		{
			return std::unexpected("Expected 'return' when parsing return statement");
		}

		ParseResultPtr<ASTExpr> expression = ParseExpression(reader);
		if (!expression)
		{
			return std::unexpected(expression.error());
		}

		if (!reader.MatchConsume(TokenType::Semicolon))
		{
			return std::unexpected("Expected ';' when parsing return statement");
		}

		return std::make_unique<ASTReturn>(std::move(*expression));
	}

	// type := i32
	//		 | function_type
	ParseResult<Type> ParseType(TokenReader& reader)
	{
		if (reader.MatchConsume(TokenType::I32))
		{
			return Type(DataType::I32);
		}
		else if (reader.MatchPeek(TokenType::LeftParen))
		{
			return ParseFunctionType(reader);
		}
		else if (std::optional<Token> current = reader.Peek())
		{
			return std::unexpected(std::format("Unexpected '{}' when parsing type", current->lexeme));
		}
		else
		{
			return std::unexpected("Ran out of tokens when parsing type");
		}
	}

	// type_list := type ("," type)*
	ParseResult<std::vector<Type>> ParseTypeList(TokenReader& reader)
	{
		return std::unexpected("Parsing type list is not implemented");
	}

	// function_type := "(" ")" "->" type
	//                | "(" type_list ")" "->" type
	ParseResult<Type> ParseFunctionType(TokenReader& reader)
	{
		return std::unexpected("Parsing function type is not implemented");
	}

	// parameter_list := identifier ":" type ("," identifier ":" type)*
	ParseResult<ParameterList> ParseParameterList(TokenReader& reader)
	{
		return std::unexpected("Parsing parameter list is not implemented");
	}

	// function_expr := "(" ")" "->" type block
	//                | "(" parameter_list? ")" "->" type block
	ParseResultPtr<ASTFunctionExpr> ParseFunctionExpression(TokenReader& reader)
	{
		if (!reader.MatchConsume(TokenType::LeftParen))
		{
			return std::unexpected("Expected '(' when parsing function declaration statement");
		}

		ParameterList parameter_list;

		if (!reader.MatchConsume(TokenType::RightParen))
		{
			ParseResult<ParameterList> parsed_parameter_list = ParseParameterList(reader);
			if (!parsed_parameter_list)
			{
				return std::unexpected(parsed_parameter_list.error());
			}

			parameter_list = *parsed_parameter_list;

			if (!reader.MatchConsume(TokenType::RightParen))
			{
				return std::unexpected("Expected ')' when parsing function declaration statement");
			}
		}

		if (!reader.MatchConsume(TokenType::RightArrow))
		{
			return std::unexpected("Expected '->' when parsing function declaration statement");
		}

		ParseResult<Type> return_type = ParseType(reader);
		if (!return_type)
		{
			return std::unexpected(return_type.error());
		}

		ParseResultPtr<ASTBlock> body = ParseBlock(reader);
		if (!body)
		{
			return std::unexpected(body.error());
		}

		return std::make_unique<ASTFunctionExpr>(std::move(parameter_list), *return_type, std::move(*body));
	}

	// function_declaration_statement := identifier ":" "=" function_expr
	ParseResultPtr<ASTFunctionDeclarationStmt> ParseFunctionDeclarationStatement(TokenReader& reader)
	{
		std::optional<Token> identifier = reader.MatchConsume(TokenType::Identifier);
		if (!identifier)
		{
			return std::unexpected("Expected identifier when parsing function declaration statement");
		}

		if (!reader.MatchConsume(TokenType::Colon))
		{
			return std::unexpected("Expected ':' when parsing function declaration statement");
		}

		if (!reader.MatchConsume(TokenType::Assign))
		{
			return std::unexpected("Expected '=' when parsing function declaration statement");
		}

		ParseResultPtr<ASTFunctionExpr> function_expression = ParseFunctionExpression(reader);
		if (!function_expression)
		{
			return std::unexpected(function_expression.error());
		}

		return std::make_unique<ASTFunctionDeclarationStmt>(identifier->lexeme, std::move(*function_expression));
	}

	// assignment_statement := identifier "=" expr ";"
	ParseResultPtr<ASTAssignmentStmt> ParseAssignmentStatement(TokenReader& reader)
	{
		std::optional<Token> identifier = reader.MatchConsume(TokenType::Identifier);
		if (!identifier)
		{
			return std::unexpected("Expected identifier when parsing assignment statement");
		}

		if (!reader.MatchConsume(TokenType::Assign))
		{
			return std::unexpected("Expected '=' when parsing assignment statement");
		}

		ParseResultPtr<ASTExpr> expr = ParseExpression(reader);
		if (!expr)
		{
			return std::unexpected(expr.error());
		}

		if (!reader.MatchConsume(TokenType::Semicolon))
		{
			return std::unexpected("Expected ';' when parsing assignment statement");
		}

		return std::make_unique<ASTAssignmentStmt>(identifier->lexeme, std::move(*expr));
	};

	// variable_declaration_stmt := identifier ":" ("mut")? type "=" expr ";"
	ParseResultPtr<ASTVariableDeclarationStmt> ParseVariableDeclarationStatement(TokenReader& reader)
	{
		std::optional<Token> identifier = reader.MatchConsume(TokenType::Identifier);
		if (!identifier)
		{
			return std::unexpected("Expected identifier when parsing assignment statement");
		}

		if (!reader.MatchConsume(TokenType::Colon))
		{
			return std::unexpected("Expected ':' when parsing assignment statement");
		}

		Qualifier qualifier = reader.MatchConsume(TokenType::Mutable) ? Qualifier::Const : Qualifier::Mutable;

		ParseResult<Type> type = ParseType(reader);
		if (!type)
		{
			return std::unexpected(type.error());
		}

		if (!reader.MatchConsume(TokenType::Assign))
		{
			return std::unexpected("Expected '=' when parsing assignment statement");
		}

		ParseResultPtr<ASTExpr> expr = ParseExpression(reader);
		if (!expr)
		{
			return std::unexpected(expr.error());
		}

		if (!reader.MatchConsume(TokenType::Semicolon))
		{
			return std::unexpected("Expected ';' when parsing assignment statement");
		}

		return std::make_unique<ASTVariableDeclarationStmt>(identifier->lexeme, *type, std::move(*expr));
	}

	// if_statement := if "(" expr ")" block
	ParseResultPtr<ASTIfStmt> ParseIfStatement(TokenReader& reader)
	{
		if (!reader.MatchConsume(TokenType::If))
		{
			return std::unexpected("Expected 'if' when parsing if statement");
		}

		if (!reader.MatchConsume(TokenType::LeftParen))
		{
			return std::unexpected("Expected '(' when parsing if statement");
		}

		ParseResultPtr<ASTExpr> expr = ParseExpression(reader);
		if (!expr)
		{
			return std::unexpected(expr.error());
		}

		if (!reader.MatchConsume(TokenType::RightParen))
		{
			return std::unexpected("Expected ')' when parsing if statement");
		}

		ParseResultPtr<ASTBlock> block = ParseBlock(reader);
		if (!block)
		{
			return std::unexpected(block.error());
		}

		return std::make_unique<ASTIfStmt>(std::move(*expr), std::move(*block));
	}

	// stmt := return_stmt
	//		 | if_stmt
	//       | variable_declaration_stmt
	//       | assignment_stmt
	//       | function_declaration_stmt
	ParseResultPtr<ASTStmt> ParseStatement(TokenReader& reader)
	{
		if (reader.MatchPeek(TokenType::Return))
		{
			return ParseReturnStatement(reader);
		}

		if (reader.MatchPeek(TokenType::If))
		{
			return ParseIfStatement(reader);
		}

		if (reader.MatchPeek(TokenType::Identifier))
		{
			if (reader.MatchPeek(TokenType::Colon, 1))
			{
				if (reader.MatchPeek(TokenType::Assign, 2))
				{
					return ParseFunctionDeclarationStatement(reader);
				}
				else
				{
					return ParseVariableDeclarationStatement(reader);
				}
			}
			else
			{
				return ParseAssignmentStatement(reader);
			}
		}

		if (reader.Peek())
		{
			return std::unexpected(std::format("Unexpected '{}' when parsing statement", reader.Peek()->lexeme));
		}
		else
		{
			return std::unexpected("Ran out of tokens when parsing statement");
		}
	}

	ParseResultPtr<ASTBlock> ParseBlock(TokenReader& reader)
	{
		if (!reader.MatchConsume(TokenType::LeftCurly))
		{
			return std::unexpected("Expected '{' when parsing block");
		}

		std::vector<std::unique_ptr<ASTStmt>> statements;

		while (reader.Peek() && !reader.MatchPeek(TokenType::RightCurly))
		{
			ParseResultPtr<ASTStmt> statement = ParseStatement(reader);
			if (!statement)
			{
				return std::unexpected(statement.error());
			}

			statements.push_back(std::move(*statement));
		}

		if (!reader.MatchConsume(TokenType::RightCurly))
		{
			return std::unexpected("Expected '}' when parsing block");
		}

		return std::make_unique<ASTBlock>(std::move(statements));
	}

	// program := statement*
	ParseResultPtr<ASTProgram> ParseProgram(TokenReader& reader)
	{
		std::vector<std::unique_ptr<ASTStmt>> statements;

		while (reader.HasMore())
		{
			ParseResultPtr<ASTStmt> statement = ParseStatement(reader);
			if (!statement)
			{
				return std::unexpected(statement.error());
			}

			statements.push_back(std::move(*statement));
		}

		return std::make_unique<ASTProgram>(std::move(statements));
	}

	std::expected<AST, ErrorMessage> Parse(const TokenBuffer& tokens)
	{
		TokenReader reader(tokens);

		ParseResultPtr<ASTProgram> program = ParseProgram(reader);
		if (!program)
		{
			std::optional<Token> current_token = reader.Peek();

			if (current_token)
			{
				return std::unexpected(std::format("{} (at line {}, column {})", program.error(), current_token->line, current_token->column));
			}
			else
			{
				return std::unexpected(program.error());
			}
		}

		return AST(std::move(*program));
	}
}