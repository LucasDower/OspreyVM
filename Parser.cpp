#include "Parser.h"

#include "Types.h"
#include "BinaryOperator.h"

#include <functional>
#include <memory>
#include <print>

namespace Osprey
{
	std::optional<AST> Parse(const TokenBuffer& tokens)
	{
		size_t cursor = 0;
		const size_t TokenBufferSize = tokens.size();

		std::unordered_map<std::string, Type> identifier_to_type;

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

		std::function<std::unique_ptr<ASTTypedNode>()> ParseFactor;
		std::function<std::unique_ptr<ASTTypedNode>()> ParseTerm;
		std::function<std::unique_ptr<ASTTypedNode>()> ParseExpression;

		ParseFactor = [&]() -> std::unique_ptr<ASTTypedNode>
			{
				if (!Peek())
				{
					std::println("Failed to parse factor: nothing to parse");
					return nullptr;
				}

				const Token token = *Consume();

				if (token.type == TokenType::I32)
				{
					const int32_t Number = std::stoi(token.lexeme); // TODO: throws if fails
					return std::make_unique<ASTLiteralNode>(Type::I32, Number);
				}
				else if (token.type == TokenType::F32)
				{
					const float Number = std::stof(token.lexeme); // TODO: throws if fails
					return std::make_unique<ASTLiteralNode>(Type::F32, *reinterpret_cast<const int32_t*>(&Number));
				}
				else if (token.type == TokenType::Identifier)
				{
					auto identifier_type_it = identifier_to_type.find(token.lexeme);

					if (identifier_type_it == identifier_to_type.end())
					{
						std::println("Cannot find type for unknown identifier '{}'", token.lexeme);
						return nullptr;
					}

					return std::make_unique<ASTVariableNode>(identifier_type_it->second, token.lexeme);
				}
				else if (token.type == TokenType::LeftParen)
				{
					std::unique_ptr<ASTTypedNode> expression = ParseExpression();

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

		ParseTerm = [&]() -> std::unique_ptr<ASTTypedNode>
			{
				std::unique_ptr<ASTTypedNode> factor_node = ParseFactor();
				if (!factor_node)
				{
					std::println("Failed to parse term: no factor");
					return nullptr;
				}

				const std::optional factor_type = factor_node->GetType();
				if (!factor_type)
				{
					std::println("Failed to parse term: cannot get type of factor");
					return nullptr;
				}

				const std::optional<Token> operator_token = Peek();
				if (operator_token && operator_token->type == TokenType::Asterisk)
				{
					Consume(); // eat the '*'

					std::unique_ptr<ASTTypedNode> next_factor_node = ParseFactor();
					if (!next_factor_node)
					{
						std::println("Failed to parse term: no following factor");
						return nullptr;
					}

					const std::optional next_factor_type = next_factor_node->GetType();
					if (!next_factor_type)
					{
						std::println("Failed to parse term: cannot get type of followed factor");
						return nullptr;
					}

					const bool can_multiply = (*factor_type == Type::I32) && (*next_factor_type == Type::I32);
					if (!can_multiply)
					{
						std::println("Type-check: cannot multiply expressions of types '{}' and '{}'", TypeToString(*factor_type), TypeToString(*next_factor_type));
						return nullptr;
					}

					return std::make_unique<ASTBinaryOperatorNode>(BinaryOperator::Asterisk, std::move(factor_node), std::move(next_factor_node));
				}

				return factor_node;
			};

		ParseExpression = [&]() -> std::unique_ptr<ASTTypedNode>
			{
				std::unique_ptr<ASTTypedNode> term = ParseTerm();
				if (!term)
				{
					std::println("Failed to parse expression: no term");
					return nullptr;
				}

				const std::optional term_type = term->GetType();
				if (!term_type)
				{
					std::println("Failed to parse expression: cannot get type of term");
					return nullptr;
				}

				std::optional<Token> plus_token = Peek();

				if (plus_token && plus_token->type == TokenType::Plus)
				{
					Consume(); // Eat the '+'

					std::unique_ptr<ASTTypedNode> expression = ParseExpression();
					if (!expression)
					{
						std::println("Failed to parse expression: no expression");
						return nullptr;
					}

					const std::optional expression_type = expression->GetType();
					if (!expression_type)
					{
						std::println("Failed to parse expression: cannot get type of expression");
						return nullptr;
					}

					const bool can_add = (term_type == Type::I32) && (expression_type == Type::I32);

					if (!can_add)
					{
						std::println("Type-check: cannot add expressions of types '{}' and '{}'", TypeToString(*term_type), TypeToString(*expression_type));
						return nullptr;
					}

					return std::make_unique<ASTBinaryOperatorNode>(BinaryOperator::Plus, std::move(term), std::move(expression));
				}

				return std::move(term);
			};

		const auto ParseReturnStatement = [&]() -> std::unique_ptr<ASTReturnNode>
			{
				const std::optional<Token> return_token = Consume();

				if (!return_token || return_token->type != TokenType::Return)
				{
					std::println("Failed to parse return statement: expected 'return'");
					return nullptr;
				}

				std::unique_ptr<ASTTypedNode> expression = ParseExpression();
				if (!expression)
				{
					std::println("Failed to parse return statement: no expression");
					return nullptr;
				}

				const std::optional<Token> semicolon_token = Consume();
				if (!semicolon_token || semicolon_token->type != TokenType::Semicolon)
				{
					std::println("Failed to parse return statement: expected ';'");
					return nullptr;
				}

				return std::make_unique<ASTReturnNode>(std::move(expression));
			};

		const auto ParseType = [&]() -> std::optional<Type>
			{
				const std::optional<Token> type_token = Consume();
				if (!type_token)
				{
					std::println("Failed to parse type: expected type");
					return std::nullopt;
				}

				if (type_token->type == TokenType::I32)
				{
					return Type::I32;
				}
				if (type_token->type == TokenType::F32)
				{
					return Type::F32;
				}

				std::println("Failed to parse type: unexpected '{}'", type_token->lexeme);
				return std::nullopt;
			};

		const auto ParseVariableDeclarationStatement = [&]() -> std::unique_ptr<ASTVariableDeclarationNode>
			{
				const std::optional<Token> identifier_token = Consume();

				if (!identifier_token || identifier_token->type != TokenType::Identifier)
				{
					std::println("Failed to parse assignment statement: expected identifier");
					return nullptr;
				}

				if (identifier_to_type.contains(identifier_token->lexeme))
				{
					std::println("Failed to parse assignment statement: identifier '{}' already exists", identifier_token->lexeme);
					return nullptr;
				}

				const std::optional<Token> colon_token = Consume();

				if (!colon_token || colon_token->type != TokenType::Colon)
				{
					std::println("Failed to parse assignment statement: expected ':'");
					return nullptr;
				}

				std::optional<Type> type = ParseType();
				if (!type)
				{
					std::println("Failed to parse assignment statement: expected type");
					return nullptr;
				}

				identifier_to_type.insert({ identifier_token->lexeme, *type });

				const std::optional<Token> assignment_token = Consume();

				if (!assignment_token || assignment_token->type != TokenType::Assign)
				{
					std::println("Failed to parse assignment statement: expected '='");
					return nullptr;
				}

				std::unique_ptr<ASTTypedNode> expression = ParseExpression();
				if (!expression)
				{
					std::println("Failed to parse assignment statement: no expression");
					return nullptr;
				}

				const std::optional<Type> expression_type = expression->GetType();

				if (!expression_type)
				{
					std::println("Type-check failed: Failed to get type of expression");
					return nullptr;
				}

				if (expression_type != type)
				{
					std::println("Type-check failed: Trying to assign a {} expression to a {} variable", TypeToString(*expression_type), TypeToString(*type));
					return nullptr;
				}

				const std::optional<Token> semicolon_token = Consume();

				if (!semicolon_token || semicolon_token->type != TokenType::Semicolon)
				{
					std::println("Failed to parse assignment statement: expected ';'");
					return nullptr;
				}

				return std::make_unique<ASTVariableDeclarationNode>(identifier_token->lexeme, *type, std::move(expression));
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
					return ParseVariableDeclarationStatement();
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