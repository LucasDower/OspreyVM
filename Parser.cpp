#include "Parser.h"

#include <functional>
#include <memory>
#include <print>

namespace Osprey
{
	std::optional<AST> Parse(const TokenBuffer& tokens)
	{
		size_t cursor = 0;
		const size_t TokenBufferSize = tokens.size();

		const auto Consume = [&]() -> std::optional<Token>
			{
				if (cursor < TokenBufferSize)
				{
					return std::optional<Token>(tokens[cursor++]);
				}
				return std::nullopt;
			};

		const auto Peek = [&]() -> std::optional<Token>
			{
				if (cursor < TokenBufferSize)
				{
					return std::optional<Token>(tokens[cursor]);
				}
				return std::nullopt;
			};

		std::function<std::unique_ptr<ASTNode>()> ParseFactor;
		std::function<std::unique_ptr<ASTNode>()> ParseExpression;

		ParseFactor = [&]() -> std::unique_ptr<ASTNode>
			{
				if (!Peek())
				{
					std::println("Failed to parse factor: nothing to parse");
					return nullptr;
				}

				const Token token = *Consume();

				if (token.type == TokenType::Integer)
				{
					const int32_t Number = std::stoi(token.lexeme); // TODO: throws if fails
					return std::make_unique<ASTIntegerLiteralNode>(Number);
				}
				else if (token.type == TokenType::Identifier)
				{
					return std::make_unique<ASTIntegerVariableNode>(token.lexeme);
				}
				else if (token.type == TokenType::LeftParen)
				{
					std::unique_ptr<ASTNode> expression = ParseExpression();

					std::optional<Token> right_paren_token = Consume();

					if (!right_paren_token || right_paren_token->type != TokenType::RightParen)
					{
						std::println("Failed to parse factor: expected ')'");
						return nullptr;
					}

					return std::move(expression);
				}

				std::println("Failed to parse factor: unexpected character");
				return nullptr;
			};

		ParseExpression = [&]() -> std::unique_ptr<ASTNode>
			{
				std::unique_ptr<ASTNode> factor = ParseFactor();
				if (!factor)
				{
					std::println("Failed to parse expression: no factor");
					return nullptr;
				}

				std::optional<Token> plus_token = Peek();

				if (plus_token && plus_token->type == TokenType::Plus)
				{
					Consume(); // Eat the '+'

					std::unique_ptr<ASTNode> expression = ParseExpression();

					factor = std::make_unique<ASTIntegerAddNode>(std::move(factor), std::move(expression));
				}

				return std::move(factor);
			};

		const auto ParseReturnStatement = [&]() -> std::unique_ptr<ASTNode>
			{
				const std::optional<Token> return_token = Consume();

				if (!return_token || return_token->type != TokenType::Return)
				{
					std::println("Failed to parse return statement: expected 'return'");
					return nullptr;
				}

				std::unique_ptr<ASTNode> expression = ParseExpression();

				const std::optional<Token> semicolon_token = Consume();
				if (!semicolon_token || semicolon_token->type != TokenType::Semicolon)
				{
					std::println("Failed to parse return statement: expected ';'");
					return nullptr;
				}

				return std::make_unique<ASTReturnNode>(std::move(expression));
			};

		const auto ParseAssignmentStatement = [&]() -> std::unique_ptr<ASTNode>
			{
				const std::optional<Token> identifier_token = Consume();

				if (!identifier_token || identifier_token->type != TokenType::Identifier)
				{
					std::println("Failed to parse assignment statement: expected identifier");
					return nullptr;
				}

				const std::optional<Token> assignment_token = Consume();

				if (!assignment_token || assignment_token->type != TokenType::Assign)
				{
					std::println("Failed to parse assignment statement: expected '='");
					return nullptr;
				}

				std::unique_ptr<ASTNode> expression = ParseExpression();

				const std::optional<Token> semicolon_token = Consume();

				if (!semicolon_token || semicolon_token->type != TokenType::Semicolon)
				{
					std::println("Failed to parse assignment statement: expected ';'");
					return nullptr;
				}

				return std::make_unique<ASTIntegerVariableAssignNode>(identifier_token->lexeme, std::move(expression));
			};

		const auto ParseStatement = [&]() -> std::unique_ptr<ASTNode>
			{
				const std::optional<Token> next_token = Peek();

				if (!next_token)
				{
					std::println("Failed to parse statement: nothing to parse");
					return nullptr;
				}

				if (next_token->type == TokenType::Return)
				{
					return ParseReturnStatement();
				}
				else if (next_token->type == TokenType::Identifier)
				{
					return ParseAssignmentStatement();
				}

				std::println("Failed to parse statement: unexpected token");
				return nullptr;
			};

		const auto ParseBlock = [&]() -> std::unique_ptr<ASTNode>
			{
				std::vector<std::unique_ptr<ASTNode>> statement_nodes;

				while (Peek())
				{
					std::unique_ptr<ASTNode> statement_node = ParseStatement();
					
					if (!statement_node)
					{
						std::println("Failed to statement");
						return nullptr;
					}

					statement_nodes.push_back(std::move(statement_node));
				}

				return std::make_unique<ASTBlockNode>(std::move(statement_nodes));
			};

		std::unique_ptr<ASTNode> block_node = ParseBlock();
		
		if (!block_node)
		{
			std::println("Failed to parse");
			return std::nullopt;
		}

		return AST(std::move(block_node));
	}
}