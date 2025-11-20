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

		const auto Peek = [&](size_t lookahead = 0) -> std::optional<Token>
			{
				if (cursor < TokenBufferSize)
				{
					return std::optional<Token>(tokens[cursor + lookahead]);
				}
				return std::nullopt;
			};

		const auto Match = [&](TokenType token_type) -> bool
		{
			std::optional<Token> next = Peek();
			return next && next->type == token_type;
		};

		std::function<std::unique_ptr<ASTExpr>()> ParseExpression;
		std::function<std::unique_ptr<ASTBlock>()> ParseBlock;

		// primary_expr := identifier | literal | '('
		std::function<std::unique_ptr<ASTExpr>()> ParsePrimaryExpr = [&]() -> std::unique_ptr<ASTExpr>
			{
				const std::optional<Token> token = Consume();
				if (!token)
				{
					std::println("Failed to parse primary expr: nothing to parse");
					return nullptr;
				}

				if (token->type == TokenType::I32)
				{
					const int32_t Number = std::stoi(token->lexeme); // TODO: throws if fails
					return std::make_unique<ASTLiteral>(Type::I32, Number);
				}
				else if (token->type == TokenType::F32)
				{
					const float Number = std::stof(token->lexeme); // TODO: throws if fails
					return std::make_unique<ASTLiteral>(Type::F32, *reinterpret_cast<const int32_t*>(&Number));
				}
				else if (token->type == TokenType::Identifier)
				{
					auto identifier_type_it = identifier_to_type.find(token->lexeme);

					if (identifier_type_it == identifier_to_type.end())
					{
						std::println("Cannot find type for unknown identifier '{}'", token->lexeme);
						return nullptr;
					}

					return std::make_unique<ASTVariable>(identifier_type_it->second, token->lexeme);
				}
				else if (token->type == TokenType::LeftParen)
				{
					std::unique_ptr<ASTExpr> expression = ParseExpression();

					std::optional<Token> right_paren_token = Consume();

					if (!right_paren_token || right_paren_token->type != TokenType::RightParen)
					{
						std::println("Failed to parse primary expr: expected ')'");
						return nullptr;
					}

					return std::move(expression);
				}

				std::println("Failed to parse primary expr: unexpected character '{}'", token->lexeme);
				return nullptr;
			};

		// unary_expr := "!" unary_expr
		//			   | "-" unary_expr
		//             | primary_expr
		std::function<std::unique_ptr<ASTExpr>()> ParseUnaryExpr = [&]() -> std::unique_ptr<ASTExpr>
			{
				if (Match(TokenType::Exclamation))
				{
					Consume(); // eat '!'
					
					std::unique_ptr<ASTExpr> expr = ParseUnaryExpr();
					if (!expr)
					{
						std::println("Failed to parse unary expr after '!'");
						return nullptr;
					}

					return std::make_unique<ASTUnaryExpr>(UnaryOperator::Exclamation, std::move(expr));
				}
				else if (Match(TokenType::Minus))
				{
					Consume(); // eat '-'

					std::unique_ptr<ASTExpr> expr = ParseUnaryExpr();
					if (!expr)
					{
						std::println("Failed to parse unary expr after '-'");
						return nullptr;
					}

					return std::make_unique<ASTUnaryExpr>(UnaryOperator::Minus, std::move(expr));
				}
				else
				{
					return ParsePrimaryExpr();
				}
			};

		// multiplicative_expr := unary_expr ('*' unary_expr)*
		//                      | unary_expr ('/' unary_expr)*
		//                      | unary_expr ('%' unary_expr)*
		std::function<std::unique_ptr<ASTExpr>()> ParseMultiplicativeExpr = [&]() -> std::unique_ptr<ASTExpr>
			{
				std::unique_ptr<ASTExpr> left_expr = ParseUnaryExpr();
				if (!left_expr)
				{
					std::println("Failed to parse left multiplicative expression");
					return nullptr;
				}

				while (true)
				{
					std::optional<BinaryOperator> operator_match;

					if (Match(TokenType::Asterisk))
					{
						operator_match = BinaryOperator::Asterisk;
					}
					else if (Match(TokenType::Divide))
					{
						operator_match = BinaryOperator::Divide;
					}
					else if (Match(TokenType::Percent))
					{
						operator_match = BinaryOperator::Percent;
					}

					if (operator_match)
					{
						Consume();

						std::unique_ptr<ASTExpr> right_expr = ParseUnaryExpr();
						if (!right_expr)
						{
							std::println("Failed to parse right multiplicative expression");
							return nullptr;
						}

						left_expr = ASTBinaryExpr::Create(*operator_match, std::move(left_expr), std::move(right_expr));
						if (!left_expr)
						{
							std::println("Failed to construct binary expr");
							return nullptr;
						}
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
		std::function<std::unique_ptr<ASTExpr>()> ParseAdditiveExpr = [&]() -> std::unique_ptr<ASTExpr>
			{
				std::unique_ptr<ASTExpr> left_expr = ParseMultiplicativeExpr();
				if (!left_expr)
				{
					std::println("Failed to parse left additive expression");
					return nullptr;
				}

				while (true)
				{
					std::optional<BinaryOperator> operator_match;

					if (Match(TokenType::Plus))
					{
						operator_match = BinaryOperator::Plus;
					}
					else if (Match(TokenType::Minus))
					{
						operator_match = BinaryOperator::Minus;
					}

					if (operator_match)
					{
						Consume();

						std::unique_ptr<ASTExpr> right_expr = ParseMultiplicativeExpr();
						if (!right_expr)
						{
							std::println("Failed to parse right additive expression");
							return nullptr;
						}

						left_expr = ASTBinaryExpr::Create(*operator_match, std::move(left_expr), std::move(right_expr));
						if (!left_expr)
						{
							std::println("Failed to construct binary expr");
							return nullptr;
						}
					}
					else
					{
						break;
					}
				}

				return left_expr;
			};

		// relational_expr := additive_expr ('<' additive_expr)?
		//                  | additive_expr ('<=' additive_expr)?
		//                  | additive_expr ('>' additive_expr)?
		//                  | additive_expr ('>=' additive_expr)?
		std::function<std::unique_ptr<ASTExpr>()> ParseRelationalExpr = [&]() -> std::unique_ptr<ASTExpr>
			{
				std::unique_ptr<ASTExpr> left_expr = ParseAdditiveExpr();
				if (!left_expr)
				{
					std::println("Failed to parse left relational expression");
					return nullptr;
				}

				while (true)
				{
					std::optional<BinaryOperator> operator_match;

					if (Match(TokenType::Lt))
					{
						operator_match = BinaryOperator::Lt;
					}
					else if (Match(TokenType::LtEq))
					{
						operator_match = BinaryOperator::LtEq;
					}
					else if (Match(TokenType::Gt))
					{
						operator_match = BinaryOperator::Gt;
					}
					else if (Match(TokenType::GtEq))
					{
						operator_match = BinaryOperator::GtEq;
					}

					if (operator_match)
					{
						Consume();

						std::unique_ptr<ASTExpr> right_expr = ParseMultiplicativeExpr();
						if (!right_expr)
						{
							std::println("Failed to parse right relational expression");
							return nullptr;
						}

						left_expr = ASTBinaryExpr::Create(*operator_match, std::move(left_expr), std::move(right_expr));
						if (!left_expr)
						{
							std::println("Failed to construct binary expr");
							return nullptr;
						}
					}
					else
					{
						break;
					}
				}

				return left_expr;
			};

		// equality_expr := relational_expr '==' relational_expr
		//                | relational_expr '!=' relational_expr
		std::function<std::unique_ptr<ASTExpr>()> ParseEqualityExpr = [&]() -> std::unique_ptr<ASTExpr>
			{
				std::unique_ptr<ASTExpr> left_expr = ParseRelationalExpr();
				if (!left_expr)
				{
					std::println("Failed to parse left equality expression");
					return nullptr;
				}

				std::optional<BinaryOperator> operator_match;

				if (Match(TokenType::Equality))
				{
					operator_match = BinaryOperator::Equality;
				}
				else if (Match(TokenType::NotEquality))
				{
					operator_match = BinaryOperator::NotEquality;
				}

				if (operator_match)
				{
					Consume();

					std::unique_ptr<ASTExpr> right_expr = ParseMultiplicativeExpr();
					if (!right_expr)
					{
						std::println("Failed to parse right equality expression");
						return nullptr;
					}

					left_expr = ASTBinaryExpr::Create(*operator_match, std::move(left_expr), std::move(right_expr));
					if (!left_expr)
					{
						std::println("Failed to construct binary expr");
						return nullptr;
					}
				}

				return left_expr;
			};

		// logical_and_expr := equality_expr ('&&' equality_expr)?
		std::function<std::unique_ptr<ASTExpr>()> ParseLogicalAndExpr = [&]() -> std::unique_ptr<ASTExpr>
			{
				std::unique_ptr<ASTExpr> left_expr = ParseEqualityExpr();
				if (!left_expr)
				{
					std::println("Failed to parse left logical and expression");
					return nullptr;
				}

				if (Match(TokenType::And))
				{
					Consume();

					std::unique_ptr<ASTExpr> right_expr = ParseEqualityExpr();
					if (!right_expr)
					{
						std::println("Failed to parse left logical and expression");
						return nullptr;
					}

					left_expr = ASTBinaryExpr::Create(BinaryOperator::And, std::move(left_expr), std::move(right_expr));
					if (!left_expr)
					{
						std::println("Failed to construct binary expr");
						return nullptr;
					}
				}

				return left_expr;
			};

		// logical_or_expr := equality_expr ('||' equality_expr)?
		std::function<std::unique_ptr<ASTExpr>()> ParseLogicalOrExpr = [&]() -> std::unique_ptr<ASTExpr>
			{
				std::unique_ptr<ASTExpr> left_expr = ParseLogicalAndExpr();
				if (!left_expr)
				{
					std::println("Failed to parse left logical or expression");
					return nullptr;
				}

				if (Match(TokenType::Or))
				{
					Consume();

					std::unique_ptr<ASTExpr> right_expr = ParseLogicalAndExpr();
					if (!right_expr)
					{
						std::println("Failed to parse left logical or expression");
						return nullptr;
					}

					left_expr = ASTBinaryExpr::Create(BinaryOperator::Or, std::move(left_expr), std::move(right_expr));
					if (!left_expr)
					{
						std::println("Failed to construct binary expr");
						return nullptr;
					}
				}

				return left_expr;
			};
		 
		// expr := logical_or_expr
		ParseExpression = ParseLogicalOrExpr;

		const auto ParseReturnStatement = [&]() -> std::unique_ptr<ASTReturn>
			{
				const std::optional<Token> return_token = Consume();

				if (!return_token || return_token->type != TokenType::Return)
				{
					std::println("Failed to parse return statement: expected 'return'");
					return nullptr;
				}

				std::unique_ptr<ASTExpr> expression = ParseExpression();
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

				return std::make_unique<ASTReturn>(std::move(expression));
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

		const auto ParseAssignmentStatement = [&]() -> std::unique_ptr<ASTAssignmentStmt>
			{
				const std::optional<Token> identifier_token = Consume();
				if (!identifier_token || identifier_token->type != TokenType::Identifier)
				{
					std::println("Failed to parse variable declaration statement: expected identifier");
					return nullptr;
				}

				if (!identifier_to_type.contains(identifier_token->lexeme))
				{
					std::println("Failed to parse assignment statement: identifier '{}' does not exists", identifier_token->lexeme);
					return nullptr;
				}

				const Type identifier_type = identifier_to_type.at(identifier_token->lexeme);

				const std::optional<Token> assignment_token = Consume();
				if (!assignment_token || assignment_token->type != TokenType::Assign)
				{
					std::println("Failed to parse variable declaration statement: expected '='");
					return nullptr;
				}

				std::unique_ptr<ASTExpr> expr = ParseExpression();
				if (!expr)
				{
					std::println("Failed to parse assignment statement: no expr");
					return nullptr;
				}

				const Type expr_type = expr->GetType();

				if (identifier_type != expr_type)
				{
					std::println("Failed to type-check assignment statement: variable '{}' is of type '{}' but is being assigned a value of type '{}'", identifier_token->lexeme, TypeToString(identifier_type), TypeToString(expr_type));
					return nullptr;
				}

				const std::optional<Token> semicolon_token = Consume();
				if (!semicolon_token || semicolon_token->type != TokenType::Semicolon)
				{
					std::println("Failed to type-check assignment statement: expected ';'");
					return nullptr;
				}

				return std::make_unique<ASTAssignmentStmt>(identifier_token->lexeme, std::move(expr));
			};

		// variable_declaration_stmt := identifier ":" type "=" expr ";"
		const auto ParseVariableDeclarationStatement = [&]() -> std::unique_ptr<ASTVariableDeclarationStmt>
			{
				const std::optional<Token> identifier_token = Consume();

				if (!identifier_token || identifier_token->type != TokenType::Identifier)
				{
					std::println("Failed to parse variable declaration statement: expected identifier");
					return nullptr;
				}

				if (identifier_to_type.contains(identifier_token->lexeme))
				{
					std::println("Failed to parse variable declaration statement: identifier '{}' already exists", identifier_token->lexeme);
					return nullptr;
				}

				const std::optional<Token> colon_token = Consume();

				if (!colon_token || colon_token->type != TokenType::Colon)
				{
					std::println("Failed to parse variable declaration statement: expected ':'");
					return nullptr;
				}

				std::optional<Type> type = ParseType();
				if (!type)
				{
					std::println("Failed to parse variable declaration statement: expected type");
					return nullptr;
				}

				identifier_to_type.insert({ identifier_token->lexeme, *type });

				const std::optional<Token> assignment_token = Consume();

				if (!assignment_token || assignment_token->type != TokenType::Assign)
				{
					std::println("Failed to parse variable declaration statement: expected '='");
					return nullptr;
				}

				std::unique_ptr<ASTExpr> expression = ParseExpression();
				if (!expression)
				{
					std::println("Failed to parse variable declaration statement: no expression");
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
					std::println("Failed to parse variable declaration statement: expected ';'");
					return nullptr;
				}

				return std::make_unique<ASTVariableDeclarationStmt>(identifier_token->lexeme, *type, std::move(expression));
			};

		const auto ParseIfStatement = [&]()->std::unique_ptr<ASTNode>
			{
				const std::optional<Token> if_token = Consume();
				if (!if_token || if_token->type != TokenType::If)
				{
					std::println("Failed to parse if statement: expected 'if'");
					return nullptr;
				}

				const std::optional<Token> left_paren_token = Consume();
				if (!left_paren_token || left_paren_token->type != TokenType::LeftParen)
				{
					std::println("Failed to parse if statement: expected '('");
					return nullptr;
				}

				std::unique_ptr<ASTExpr> predicate_node = ParseExpression();
				if (!predicate_node)
				{
					std::println("Failed to parse if statement: expected predicate");
					return nullptr;
				}

				const std::optional<Type> predicate_type = predicate_node->GetType();
				if (!predicate_type || (*predicate_type) != Type::Bool)
				{
					std::println("Failed to parse if statement: expected predicate to be of type 'bool'");
					return nullptr;
				}

				const std::optional<Token> right_paren_token = Consume();
				if (!right_paren_token || right_paren_token->type != TokenType::RightParen)
				{
					std::println("Failed to parse if statement: expected ')'");
					return nullptr;
				}

				std::unique_ptr<ASTBlock> block = ParseBlock();
				if (!block)
				{
					std::println("Failed to parse if statement: no true block");
					return nullptr;
				}

				return std::make_unique<ASTIfStmt>(std::move(predicate_node), std::move(block));
			};

		// stmt := return_stmt
		//       | variable_declaration_stmt
		//       | assignment_stmt
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
					if (Peek(1) && Peek(1)->type == TokenType::Colon)
					{
						return ParseVariableDeclarationStatement();
					}
					else
					{
						return ParseAssignmentStatement();
					}
				}
				else if (next_token->type == TokenType::If)
				{
					return ParseIfStatement();
				}

				std::println("Failed to parse statement: unexpected token '{}'", next_token->lexeme);
				return nullptr;
			};

		ParseBlock = [&]() -> std::unique_ptr<ASTBlock>
			{
				const std::optional<Token> left_curly_token = Consume();
				if (!left_curly_token || left_curly_token->type != TokenType::LeftCurly)
				{
					std::println("Failed to parse if statement: expected left curly");
					return nullptr;
				}

				std::vector<std::unique_ptr<ASTNode>> statement_nodes;

				while (Peek() && Peek()->type != TokenType::RightCurly)
				{
					std::unique_ptr<ASTNode> statement_node = ParseStatement();
					
					if (!statement_node)
					{
						std::println("Failed to statement");
						return nullptr;
					}

					statement_nodes.push_back(std::move(statement_node));
				}

				const std::optional<Token> right_curly_token = Consume();
				if (!right_curly_token || right_curly_token->type != TokenType::RightCurly)
				{
					std::println("Failed to parse if statement: expected right curly");
					return nullptr;
				}

				return std::make_unique<ASTBlock>(std::move(statement_nodes));
			};

		std::unique_ptr<ASTBlock> block_node = ParseBlock();
		
		if (!block_node)
		{
			std::println("Failed to parse");
			return std::nullopt;
		}

		return AST(std::move(block_node));
	}
}