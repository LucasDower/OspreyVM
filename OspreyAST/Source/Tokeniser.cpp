#include "OspreyAST/Tokeniser.h"

#include <print>
#include <optional>

namespace Osprey
{
	std::expected<TokenBuffer, ErrorMessage> Tokenise(const std::string script)
	{
		TokenBuffer tokens;
		size_t cursor = 0;
		const size_t script_size = script.size();
		size_t column = 0;
		size_t line = 0;

		const auto Consume = [&]() -> uint8_t
			{
				uint8_t curr_char = script[cursor++];
				column++;
				return curr_char;
			};

		const auto MakeToken = [&](TokenType token_type, std::string lexeme) -> Token
			{
				return { token_type, std::move(lexeme), line, column };
			};

		const auto Peek = [&]() -> std::optional<uint8_t>
			{
				return (cursor < script_size) ? std::optional<uint8_t>(script[cursor]) : std::nullopt;
			};

		while (cursor < script_size)
		{
			uint8_t curr_char = Consume();

			switch (curr_char)
			{
				case ' ':
				{
					break;
				}
				case '\r':
				{
					break;
				}
				case '\n':
				{
					++line;
					column = 0;
					break;
				}
				case '\t':
				{
					column += 4;
					break;
				}
				case ':':
				{
					tokens.push_back(MakeToken(TokenType::Colon, ":"));
					break;
				}
				case ';':
				{
					tokens.push_back(MakeToken(TokenType::Semicolon, ";"));
					break;
				}
				case '(':
				{
					tokens.push_back(MakeToken(TokenType::LeftParen, "("));
					break;
				}
				case ')':
				{
					tokens.push_back(MakeToken(TokenType::RightParen, ")"));
					break;
				}
				case '{':
				{
					tokens.push_back(MakeToken(TokenType::LeftCurly, "{"));
					break;
				}
				case '}':
				{
					tokens.push_back(MakeToken(TokenType::RightCurly, "}"));
					break;
				}
				case ',':
				{
					tokens.push_back(MakeToken(TokenType::Comma, ","));
					break;
				}
				case '+':
				{
					tokens.push_back(MakeToken(TokenType::Plus, "+"));
					break;
				}
				case '-':
				{
					if (Peek() && *Peek() == '>')
					{
						tokens.push_back(MakeToken(TokenType::RightArrow, "->"));
						Consume();
					}
					else
					{
						tokens.push_back(MakeToken(TokenType::Plus, "+"));
					}
					break;
				}
				case '*':
				{
					tokens.push_back(MakeToken(TokenType::Asterisk, "*"));
					break;
				}
				case '=':
				{
					if (Peek() && *Peek() == '=')
					{
						tokens.push_back(MakeToken(TokenType::Equality, "=="));
						Consume();
					}
					else
					{
						tokens.push_back(MakeToken(TokenType::Assign, "="));
					}
					break;
				}
				default:
				{
					if (std::isdigit(curr_char))
					{
						// Only support basic positive integers!

						std::string number_string(1, static_cast<char>(curr_char));

						// TODO: use regex instead, as '0.0.0f' is passes as a valid float
						while (Peek() && (std::isdigit(*Peek()) || *Peek() == '.'))
						{
							number_string += static_cast<unsigned char>(Consume());
						}

						/*
						if (Peek() && *Peek() == 'f')
						{
							Consume(); // eat the 'f'
							tokens.push_back(MakeToken(TokenType::F32, number_string));
						}
						else
						{
						*/
						tokens.push_back(MakeToken(TokenType::I32, number_string));
						//}
					}
					else if (std::isalpha(curr_char))
					{
						std::string identifier_or_keyword(1, static_cast<char>(curr_char));

						while (Peek() && (std::isalpha(*Peek()) || std::isdigit(*Peek())))
						{
							identifier_or_keyword += static_cast<unsigned char>(Consume());
						}

						if (identifier_or_keyword == "return")
						{
							tokens.push_back(MakeToken(TokenType::Return, "return"));
						}
						else if (identifier_or_keyword == "if")
						{
							tokens.push_back(MakeToken(TokenType::If, "if"));
						}
						else if (identifier_or_keyword == "i32")
						{
							tokens.push_back(MakeToken(TokenType::I32, "i32"));
						}
						/*
						else if (identifier_or_keyword == "f32")
						{
							tokens.push_back(MakeToken(TokenType::F32, "f32"));
						}
						*/
						else if (identifier_or_keyword == "mut")
						{
							tokens.push_back(MakeToken(TokenType::Mutable, "mut"));
						}
						else
						{
							tokens.push_back(MakeToken(TokenType::Identifier, std::move(identifier_or_keyword)));
						}
					}
					else
					{
						const auto EscapeChar = [](char c) -> std::string
							{
								switch (c)
								{
									case '\n': return "\\n";
									case '\r': return "\\r";
									case '\t': return "\\t";
									case '\0': return "\\0";
									case '\'': return "\\'";
									case '\"': return "\\\"";
									case '\\': return "\\\\";
									default:
									{
										if (std::isprint(static_cast<unsigned char>(c)))
										{
											return std::string(1, c);
										}
										else
										{
											return std::format("\\x{:02x}", static_cast<unsigned char>(c)); // hex for other control chars
										}
									}
								}
							};

						return std::unexpected(std::format("Unexpected character '{}'", EscapeChar(static_cast<char>(curr_char)), line, column));
					}
				}
			}
		}

		return tokens;
	}
}