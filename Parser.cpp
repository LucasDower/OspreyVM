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

		//std::unordered_map<std::string, Type> identifier_to_type;

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

		// argument_list := expr (',' expr)*
		auto ParseArgumentList = [&]() -> std::optional<ArgumentList>
			{
				ArgumentList arg_list;

				auto expr = ParseExpression();
				if (!expr)
				{
					std::println("Failed to parse argument list: no expression");
					return std::nullopt;
				}

				arg_list.args.push_back(std::move(expr));

				while (Match(TokenType::Comma))
				{
					Consume(); // eat the ','

					auto expr2 = ParseExpression();
					if (!expr2)
					{
						std::println("Failed to parse argument list: no expression after ','");
						return std::nullopt;
					}

					arg_list.args.push_back(std::move(expr2));
				}

				return arg_list;
			};

		// primary_expr := identifier
		//               | identifier '( argument_list? ')'
		//               | literal
		//               | '(' expr ')'
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
					/*
					auto identifier_type_it = identifier_to_type.find(token->lexeme);

					if (identifier_type_it == identifier_to_type.end())
					{
						std::println("Cannot find type for unknown identifier '{}'", token->lexeme);
						return nullptr;
					}
					*/

					if (Match(TokenType::LeftParen))
					{
						if (Match(TokenType::RightParen))
						{
							return std::make_unique<ASTFunctionCall>(token->lexeme, ArgumentList());
						}
						else if (std::optional<ArgumentList> arg_list = ParseArgumentList())
						{
							return std::make_unique<ASTFunctionCall>(token->lexeme, std::move(*arg_list));
						}
						else
						{
							std::println("Failed to parse function call");
							return nullptr;
						}
					}
					else
					{
						return std::make_unique<ASTVariable>(token->lexeme);
					}
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

						left_expr = std::make_unique<ASTBinaryExpr>(*operator_match, std::move(left_expr), std::move(right_expr));
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

						left_expr = std::make_unique<ASTBinaryExpr>(*operator_match, std::move(left_expr), std::move(right_expr));
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

						left_expr = std::make_unique<ASTBinaryExpr>(*operator_match, std::move(left_expr), std::move(right_expr));
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

					left_expr = std::make_unique<ASTBinaryExpr>(*operator_match, std::move(left_expr), std::move(right_expr));
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

					left_expr = std::make_unique<ASTBinaryExpr>(BinaryOperator::And, std::move(left_expr), std::move(right_expr));
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

					left_expr = std::make_unique<ASTBinaryExpr>(BinaryOperator::Or, std::move(left_expr), std::move(right_expr));
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


		// function_type := "(" ")" "->" type
		//                | "(" identifier ":" type ")" "->" type
		//                | "(" identifier ":" type ("," identifier ":" type)* ")" "->" type
		const auto ParseFunctionType = [&]() -> std::optional<FunctionType>
			{
				const std::optional<Token> open_paren_token = Consume();
				if (!open_paren_token || open_paren_token->type != TokenType::LeftParen)
				{
					std::println("Failed to parse function declaration: no identifier");
					return std::nullopt;
				}

				FunctionType function_type;

				while (true)
				{
					if (Match(TokenType::RightParen))
					{
						Consume(); // eat the '('
						break;
					}
					else if (Match(TokenType::Identifier))
					{
						const Token identifier_token = *Consume();

						const std::optional<Token> colon_token = Consume();
						if (!colon_token || colon_token->type != TokenType::Colon)
						{
							std::println("Failed to parse function declaration: expected ':'");
							return std::nullopt;
						}

						std::optional<Type> type = ParseType();
						if (!type)
						{
							std::println("Failed to parse function declaration: no type");
							return std::nullopt;
						}

						function_type.parameters.push_back({ identifier_token.lexeme, *type });

						if (Match(TokenType::Comma))
						{
							Consume(); // eat the ','
						}
						else if (!Match(TokenType::RightParen))
						{
							std::println("Failed to parse function declaration: unexpected character");
							return std::nullopt;
						}
					}
					else
					{
						const std::optional<Token> next = Peek();
						std::println("Failed to parse function declaration: unexpected character '{}', expected <identifier> or ')'", next ? next->lexeme : "[EOF]");
						return std::nullopt;
					}
				}
				
				const std::optional<Token> right_arrow_token = Consume();
				if (!right_arrow_token || right_arrow_token->type != TokenType::RightArrow)
				{
					std::println("Failed to parse function declaration: expected '->'");
					return std::nullopt;
				}

				const std::optional<Type> result_type = ParseType();
				if (!result_type)
				{
					std::println("Failed to parse function declaration: no result type");
					return std::nullopt;
				}

				function_type.return_type = *result_type;

				return function_type;
			};

		// function_declaration := identifier ":" function_type block
		const auto ParseFunctionDeclaration = [&]() -> std::unique_ptr<ASTFunctionDeclaration>
			{
				const std::optional<Token> identifier_token = Consume();
				if (!identifier_token || identifier_token->type != TokenType::Identifier)
				{
					std::println("Failed to parse function declaration: no identifier");
					return nullptr;
				}

				const std::optional<Token> colon_token = Consume();
				if (!colon_token || colon_token->type != TokenType::Colon)
				{
					std::println("Failed to parse function declaration: expected ':'");
					return nullptr;
				}

				std::optional<FunctionType> function_type = ParseFunctionType();
				if (!function_type)
				{
					std::println("Failed to parse function declaration: no function type");
					return nullptr;
				}

				std::unique_ptr<ASTBlock> block_node = ParseBlock();
				if (!block_node)
				{
					std::println("Failed to parse function declaration: no block");
					return nullptr;
				}

				return std::make_unique<ASTFunctionDeclaration>(identifier_token->lexeme, *function_type, std::move(block_node));
			};

		// program := function_declaration*
		const auto ParseProgram = [&]() -> std::unique_ptr<ASTProgram>
			{
				std::vector<std::unique_ptr<ASTFunctionDeclaration>> functions;

				while (Peek())
				{
					std::unique_ptr<ASTFunctionDeclaration> function = ParseFunctionDeclaration();
					if (!function)
					{
						std::println("Failed to parse program: invalid function");
						return nullptr;
					}

					functions.push_back(std::move(function));
				}

				return std::make_unique<ASTProgram>(std::move(functions));
			};

		std::unique_ptr<ASTProgram> program = ParseProgram();
		if (!program)
		{
			std::println("Failed to parse");
			return std::nullopt;
		}

		return AST(std::move(program));
	}
}