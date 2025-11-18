#include <iostream>
#include <vector>
#include <cstdint>
#include <optional>
#include <string>
#include <functional>
#include <unordered_map>

enum Instructions
{
	PUSH,
	ADD,
	LOAD,
	STORE,
	LT,
	JZ,
	JMP,
	SYSCALL,
	HALT,
};

class FProgram
{
public:
	FProgram(std::vector<int32_t> InProgram)
		: Program(InProgram)
	{
	}

	int32_t GetInstruction(int32_t Offset)
	{
		return Program[Offset];
	}

private:
	std::vector<int32_t> Program;
};

class FStack
{
public:
	int32_t Pop()
	{
		int32_t Top = Stack.back();
		Stack.pop_back();
		return Top;
	}

	void Push(int32_t Data)
	{
		Stack.push_back(Data);
	}

	void Dump() const
	{
		for (int32_t Value : Stack)
		{
			std::cout << Value << std::endl;
		}
	}

private:
	std::vector<int32_t> Stack;
};

class FMemory
{
public:
	FMemory(size_t Size)
	{
		Data.resize(Size);
	}

	void Set(size_t Address, int32_t Value)
	{
		Data[Address] = Value;
	}

	int32_t Get(int32_t Address)
	{
		return Data[Address];
	}

private:
	std::vector<int32_t> Data;
};

class FProcessor
{
public:
	FProcessor(FProgram InProgram)
		: Program(InProgram)
		, DataStack()
		, Memory(1'024)
		, InstructionOffset(0)
	{

	}

	void Execute()
	{
		bool IsRunning = true;

		while (IsRunning)
		{
			int32_t Instruction = Program.GetInstruction(InstructionOffset++);

			switch (Instruction)
			{
			case PUSH:
				{
					int32_t Value = Program.GetInstruction(InstructionOffset++);
					DataStack.Push(Value);
					break;
				}
			case ADD:
				{
					int32_t A = DataStack.Pop();
					int32_t B = DataStack.Pop();
					DataStack.Push(A + B);
					break;
				}
			case STORE:
				{
					int32_t Address = Program.GetInstruction(InstructionOffset++);
					int32_t Value = DataStack.Pop();
					Memory.Set(Address, Value);
					std::cout << "Store " << Value << " in " << Address << std::endl;
					break;
				}
			case LOAD:
				{
					int32_t Address = Program.GetInstruction(InstructionOffset++);
					int32_t Value = Memory.Get(Address);
					DataStack.Push(Value);
					std::cout << "Load " << Value << " from " << Address << std::endl;
					break;
				}
			case LT:
			{
				// Stack: (Bottom)   (Top)
				//          |          |
				//        [ X, ..., A, B ]
				int32_t A = DataStack.Pop();
				int32_t B = DataStack.Pop();
				DataStack.Push(A < B ? 1 : 0);
				break;
			}
			case JZ:
			{
				int32_t NewInstructionOffset = Program.GetInstruction(InstructionOffset++);
				int32_t Value = DataStack.Pop();
				if (Value == 0)
				{
					InstructionOffset = NewInstructionOffset;
				}
				break;
			}
			case JMP:
				{
					int32_t Address = Program.GetInstruction(InstructionOffset++);
					int32_t NewInstructionOffset = Memory.Get(Address);
					InstructionOffset = NewInstructionOffset;
					break;
				}
			case HALT:
				{
					IsRunning = false;
					break;
				}
			default:
				{
					IsRunning = false;
					break;
				}
			}
		}
	}

	const FStack& GetDataStack()
	{
		return DataStack;
	}

private:
	FProgram Program;
	FStack DataStack;
	FMemory Memory;
	size_t InstructionOffset;
};

struct FCompileContext
{
	std::vector<int32_t> Instructions;
	std::unordered_map<std::string, int32_t> IdentifierToAddress;
	size_t StaticDataSize = 0;
};

class FNode
{
public:
	virtual bool Compile(FCompileContext& Context) = 0;
};

class FIntegerExpressionNode : public FNode
{
};

class FIntegerLiteralNode : public FIntegerExpressionNode
{
public:
	FIntegerLiteralNode(int32_t InValue)
		: Value(InValue)
	{
	}

private:
	virtual bool Compile(FCompileContext& Context) override
	{
		Context.Instructions.push_back(PUSH);
		Context.Instructions.push_back(Value);

		return true;
	}

	int32_t Value;
};

class FIntegerVariableNode : public FIntegerExpressionNode
{
public:
	FIntegerVariableNode(std::string InIdentifier)
		: Identifier(InIdentifier)
	{
	}

private:
	virtual bool Compile(FCompileContext& Context) override
	{
		if (!Context.IdentifierToAddress.contains(Identifier))
		{
			std::cout << "Variable " << Identifier << " does not exist\n";
			return false;
		}

		int32_t Address = Context.IdentifierToAddress.at(Identifier);

		Context.Instructions.push_back(LOAD);
		Context.Instructions.push_back(Address);

		return true;
	}

	std::string Identifier;
};

class FIntegerAddNode : public FIntegerExpressionNode
{
public:
	FIntegerAddNode(std::unique_ptr<FNode> InA, std::unique_ptr<FNode> InB)
		: A(std::move(InA))
		, B(std::move(InB))
	{
	}

private:
	virtual bool Compile(FCompileContext& Context) override
	{
		if (!A->Compile(Context))
		{
			return false;
		}

		if (!B->Compile(Context))
		{
			return false;
		}

		Context.Instructions.push_back(ADD);

		return true;
	}

	std::unique_ptr<FNode> A;
	std::unique_ptr<FNode> B;
};

class FVariableSetNode : public FNode
{
public:
	FVariableSetNode(std::string InIdentifier, std::unique_ptr<FNode> InNode)
		: Identifier(InIdentifier)
		, IntegerNode(std::move(InNode))
	{
	}

private:
	virtual bool Compile(FCompileContext& Context) override
	{
		if (!Context.IdentifierToAddress.contains(Identifier))
		{
			int32_t Address = Context.StaticDataSize++;
			Context.IdentifierToAddress.insert({ Identifier, Address });
		}
		int32_t Address = Context.IdentifierToAddress.at(Identifier);

		if (!IntegerNode->Compile(Context))
		{
			return false;
		}

		Context.Instructions.push_back(STORE);
		Context.Instructions.push_back(Address);

		return true;
	}

	std::string Identifier;
	std::unique_ptr<FNode> IntegerNode;
};

class FReturnNode : public FNode
{
public:
	FReturnNode(std::unique_ptr<FNode> InExpression)
		: Expression(std::move(InExpression))
	{
	}

private:
	virtual bool Compile(FCompileContext& Context) override
	{
		if (!Expression->Compile(Context))
		{
			return false;
		}

		Context.Instructions.push_back(HALT);

		return true;
	}

	std::unique_ptr<FNode> Expression;
};

class FBlockNode : public FNode
{
public:
	FBlockNode()
	{
	}

	void Add(std::unique_ptr<FNode> Node)
	{
		Nodes.push_back(std::move(Node));
	}

private:
	virtual bool Compile(FCompileContext& Context) override
	{
		for (std::unique_ptr<FNode>& Node : Nodes)
		{
			if (!Node->Compile(Context))
			{
				return false;
			}
		}

		return true;
	}

	std::vector<std::unique_ptr<FNode>> Nodes;
};

class FAST
{
public:
	std::optional<FProgram> Compile() const
	{
		FCompileContext Context;

		if (Root->Compile(Context))
		{
			return FProgram(Context.Instructions);
		}

		return std::nullopt;
	}

	FAST(std::unique_ptr<FNode> InRoot)
		: Root(std::move(InRoot))
	{
	}

private:
	std::unique_ptr<FNode> Root;
};

enum class ETokenType
{
	Identifier,
	Assign,
	Plus,
	Return,
	Integer,
	Equality,
	Semicolon,
	LeftParen,
	RightParen,
};

class FToken
{
public:
	ETokenType Type;
	std::string Lexeme;
	size_t Line;
	size_t Column;
};

using TTokenBuffer = std::vector<FToken>;

class FLexer
{
public:
	static std::optional<TTokenBuffer> Tokenise(const std::string& InScript)
	{
		TTokenBuffer Tokens;
		size_t Cursor = 0;
		const size_t ScriptSize = InScript.size();
		size_t Column = 0;
		size_t Line = 0;
		//size_t Start = 0;

		const auto Consume = [&]() -> uint8_t
			{
				uint8_t Char = InScript[Cursor++];
				Column++;
				return Char;
			};

		const auto MakeToken = [&](ETokenType InTokenType, std::string Lexeme) -> FToken
			{
				return { InTokenType, Lexeme, Line, Column };
			};

		const auto Peek = [&]() -> std::optional<uint8_t>
			{
				return (Cursor < ScriptSize) ? std::optional<uint8_t>(InScript[Cursor]) : std::nullopt;
			};

		while (Cursor < ScriptSize)
		{
			uint8_t Char = Consume();

			switch (Char)
			{
			case ' ':
				{
					break;
				}
			case '\n':
				{
					++Line;
					Column = 0;
					break;
				}
			case ';':
				{
					Tokens.push_back(MakeToken(ETokenType::Semicolon, ";"));
					break;
				}
				case '(':
				{
					Tokens.push_back(MakeToken(ETokenType::LeftParen, "("));
					break;
				}
				case ')':
				{
					Tokens.push_back(MakeToken(ETokenType::RightParen, ")"));
					break;
				}
			case '+':
				{
					Tokens.push_back(MakeToken(ETokenType::Plus, "+"));
					break;
				}
			case '=':
				{
					if (Peek() && *Peek() == '=')
					{
						Tokens.push_back(MakeToken(ETokenType::Equality, "=="));
						Consume();
					}
					else
					{
						Tokens.push_back(MakeToken(ETokenType::Assign, "="));
					}
					break;
				}
			default:
				{
					if (std::isdigit(Char))
					{
						// Only support basic positive integers!

						std::string NumberString(1, static_cast<char>(Char));

						while (Peek() && std::isdigit(*Peek()))
						{
							NumberString += static_cast<unsigned char>(Consume());
						}

						Tokens.push_back(MakeToken(ETokenType::Integer, NumberString));
					}
					else if (std::isalpha(Char))
					{
						// Identifiers cannot have numbers in them!

						std::string IdentifierOrKeyword(1, static_cast<char>(Char));

						while (Peek() && std::isalpha(*Peek()))
						{
							IdentifierOrKeyword += static_cast<unsigned char>(Consume());
						}

						if (IdentifierOrKeyword == "return")
						{
							Tokens.push_back(MakeToken(ETokenType::Return, "return"));
						}
						else
						{
							Tokens.push_back(MakeToken(ETokenType::Identifier, std::move(IdentifierOrKeyword)));
						}
					}
					else
					{
						std::cout << "Unexpected character\n";
					}
				}
			}
		}

		return Tokens;
	}

private:
};

class FParser
{
public:
	static std::optional<FAST> Parse(const TTokenBuffer& InTokenBuffer)
	{
		size_t Cursor = 0;
		const size_t TokenBufferSize = InTokenBuffer.size();

		const auto Consume = [&]() -> std::optional<FToken>
			{
				if (Cursor < TokenBufferSize)
				{
					return std::optional<FToken>(InTokenBuffer[Cursor++]);
				}
				return std::nullopt;
			};

		const auto Peek = [&]() -> std::optional<FToken>
			{
				if (Cursor < TokenBufferSize)
				{
					return std::optional<FToken>(InTokenBuffer[Cursor]);
				}
				return std::nullopt;
			};

		std::function<std::unique_ptr<FNode>()> ParseFactor;
		std::function<std::unique_ptr<FNode>()> ParseExpression;

		ParseFactor = [&]() -> std::unique_ptr<FNode>
			{
				if (!Peek())
				{
					std::cout << "Failed to parse factor: nothing to parse\n";
					return nullptr;
				}

				const FToken Token = *Consume();

				if (Token.Type == ETokenType::Integer)
				{
					const int32_t Number = std::stoi(Token.Lexeme); // TODO: throws is fails
					return std::make_unique<FIntegerLiteralNode>(Number);
				}
				else if (Token.Type == ETokenType::Identifier)
				{
					return std::make_unique<FIntegerVariableNode>(Token.Lexeme);
				}
				else if (Token.Type == ETokenType::LeftParen)
				{
					std::unique_ptr<FNode> Expression = ParseExpression();

					std::optional<FToken> RightParenOpt = Consume();

					if (!RightParenOpt)
					{
						std::cout << "Failed to parse factor: expected ')', got nothing\n";
						return nullptr;
					}

					if (RightParenOpt->Type != ETokenType::RightParen)
					{
						std::cout << "Failed to parse factor: expected ')'\n";
						return nullptr;
					}
				}
			};
		
		ParseExpression = [&]() -> std::unique_ptr<FNode>
			{
				std::unique_ptr<FNode> Factor = ParseFactor();
				if (!Factor)
				{
					std::cout << "Failed to parse expression: no factor\n";
					return nullptr;
				}

				const std::optional<FToken> TokenOpt = Peek();
				if (TokenOpt && TokenOpt->Type == ETokenType::Plus)
				{
					Consume(); // Eat the '+'
					std::unique_ptr<FNode> Expression = ParseExpression();

					Factor = std::make_unique<FIntegerAddNode>(std::move(Factor), std::move(Expression));
				}

				return Factor;
			};

		const auto ParseReturnStatement = [&]() -> std::unique_ptr<FNode>
			{
				// Consume 'return'
				{
					const std::optional<FToken> TokenOpt = Consume();
					if (!TokenOpt)
					{
						std::cout << "Failed to parse return statement: nothing to parse\n";
						return nullptr;
					}

					if (TokenOpt->Type != ETokenType::Return)
					{
						std::cout << "Failed to parse return statement: expected 'return'\n";
						return nullptr;
					}
				}

				// Consume expression
				std::unique_ptr<FNode> Expression = ParseExpression();

				// Consume ';'
				{
					const std::optional<FToken> TokenOpt = Consume();
					if (!TokenOpt)
					{
						std::cout << "Failed to parse return statement: nothing to parse\n";
						return nullptr;
					}

					if (TokenOpt->Type != ETokenType::Semicolon)
					{
						std::cout << "Failed to parse return statement: expected ';'\n";
						return nullptr;
					}
				}

				return std::make_unique<FReturnNode>(std::move(Expression));
			};

		const auto ParseAssignmentStatement = [&]() -> std::unique_ptr<FNode>
			{
				const std::optional<FToken> IdentifierToken = Consume();

				if (!IdentifierToken || IdentifierToken->Type != ETokenType::Identifier)
				{
					std::cout << "Failed to parse assignment statement: expected identifier\n";
					return nullptr;
				}

				const std::optional<FToken> AssignmentToken = Consume();

				if (!AssignmentToken || AssignmentToken->Type != ETokenType::Assign)
				{
					std::cout << "Failed to parse assignment statement: expected '='\n";
					return nullptr;
				}

				std::unique_ptr<FNode> Expression = ParseExpression();

				const std::optional<FToken> SemicolonToken = Consume();

				if (!SemicolonToken || SemicolonToken->Type != ETokenType::Semicolon)
				{
					std::cout << "Failed to parse assignment statement: expected ';'\n";
					return nullptr;
				}

				return std::make_unique<FVariableSetNode>(IdentifierToken->Lexeme, std::move(Expression));
			};

		const auto ParseStatement = [&]() -> std::unique_ptr<FNode>
			{
				const std::optional<FToken> NextTokenOpt = Peek();
				if (!NextTokenOpt)
				{
					std::cout << "Failed to parse statement: nothing to parse\n";
					return nullptr;
				}

				const FToken& NextToken = *NextTokenOpt;

				if (NextToken.Type == ETokenType::Return)
				{
					return ParseReturnStatement();
				}
				else if (NextToken.Type == ETokenType::Identifier)
				{
					return ParseAssignmentStatement();
				}
				
				std::cout << "Failed to parse statement: unexpected token\n";
				return nullptr;
			};

		const auto ParseBlock = [&]() -> std::unique_ptr<FNode>
			{
				std::unique_ptr<FBlockNode> Block = std::make_unique<FBlockNode>();

				while (Peek())
				{
					std::unique_ptr<FNode> Statement = ParseStatement();
					if (!Statement)
					{
						std::cout << "Failed to statement\n";
						return nullptr;
					}
					Block->Add(std::move(Statement));
				}

				return Block;
			};

		std::unique_ptr<FNode> Block = ParseBlock();
		if (!Block)
		{
			std::cout << "Failed to parse\n";
			return std::nullopt;
		}

		return FAST(std::move(Block));
	}

private:
};



int main()
{
	std::string Script =
		"xxx = 502;\n"
		"yyy = 35\n"
		"z = 2;\n"
		"return xxx + yyy + z;";

	std::optional<TTokenBuffer> Tokens = FLexer::Tokenise(Script);
	if (!Tokens)
	{
		std::cout << "Could not tokenise script\n";
		return 1;
	}

	std::optional<FAST> AST = FParser::Parse(*Tokens);
	if (!AST)
	{
		std::cout << "Could not parse tokens\n";
		return 1;
	}

	/*
	std::unique_ptr<FBlockNode> Block = std::make_unique<FBlockNode>();

	Block->Add(
		std::make_unique<FVariableSetNode>(
			"x",
			std::make_unique<FIntegerLiteralNode>(5)));

	Block->Add(
		std::make_unique<FVariableSetNode>(
			"y",
			std::make_unique<FIntegerLiteralNode>(6)));

	Block->Add(
		std::make_unique<FReturnNode>(
			std::make_unique<FIntegerAddNode>(
				std::make_unique<FIntegerVariableNode>("x"),
				std::make_unique<FIntegerVariableNode>("y"))));

	FAST AST(std::move(Block));
	*/

	std::optional<FProgram> CompiledProgram = AST->Compile();

	if (CompiledProgram.has_value())
	{
		FProcessor Processor(*CompiledProgram);
		Processor.Execute();

		Processor.GetDataStack().Dump();
	}

	return 0;
}