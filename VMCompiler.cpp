#include "VMCompiler.h"

#include "AST.h"
#include "VMProgram.h"
#include "ASTVisitor.h"
#include "VMOpCode.h"

#include <vector>
#include <unordered_map>
#include <print>
#include <cassert>
#include <ranges>

namespace Osprey
{
	class VMIdentifier
	{
	public:
		VMIdentifier(std::string identifier, Type type)
			: m_identifier(std::move(identifier))
			, m_type(type)
		{
		}

		const std::string& GetIdentifier() const { return m_identifier; }

	private:
		std::string m_identifier;
		Type m_type;
	};

	class VMScopeStack
	{
	public:
		int32_t Temporaries = 0;

		VMScopeStack(std::string debug_label)
			: m_debug_label(debug_label)
		{
		}

		const std::string& GetDebugLabel() const { return m_debug_label; }

		bool PushVariable(VMIdentifier identifier)
		{
			if (HasVariable(identifier.GetIdentifier()))
			{
				return false;
			}
			else
			{
				m_identifiers.push_back(identifier);
				return true;
			}
		}

		bool HasVariable(const std::string& identifier) const
		{
			for (const VMIdentifier& existing_identifier : m_identifiers)
			{
				if (existing_identifier.GetIdentifier() == identifier)
				{
					return true;
				}
			}

			return false;
		}

		bool PushFunction(std::string function, size_t instruction_offset)
		{
			if (HasFunction(function))
			{
				return false;
			}
			else
			{
				m_functions.push_back({ std::move(function), instruction_offset });
				return true;
			}
		}

		bool HasFunction(const std::string& function)
		{
			for (const std::pair<std::string, size_t>& existing_function : m_functions)
			{
				if (existing_function.first == function)
				{
					return true;
				}
			}

			return false;
		}

		const std::vector<VMIdentifier>& GetIdentifiers() const { return m_identifiers; }
		const std::vector<std::pair<std::string, size_t>>& GetFunctions() const { return m_functions; }

	private:
		std::string m_debug_label;
		std::vector<VMIdentifier> m_identifiers;
		std::vector<std::pair<std::string, size_t>> m_functions;
	};

	class VMCompileContext
	{
	public:
		void PushScope(std::string debug_label)
		{
			scopes.push_back(VMScopeStack(debug_label));
		}

		const VMScopeStack& TopScope() const { return scopes.back(); }
		VMScopeStack& TopScope() { return scopes.back(); }

		uint32_t Temporaries = 0;

		void PopScope()
		{
			scopes.pop_back();
		}

		bool PushFunction(std::string identifier)
		{
			return scopes.back().PushFunction(identifier, instructions.size());
		}

		bool HasFunction(std::string identifier)
		{
			for (const VMScopeStack& scope : scopes)
			{
				if (scope.HasVariable(identifier))
				{
					return true;
				}
			}

			return false;
		}

		std::optional<size_t> GetFunctionInstructionOffset(std::string identifier)
		{
			for (const VMScopeStack& scope : scopes)
			{
				for (const std::pair<std::string, size_t>& some_function : scope.GetFunctions())
				{
					if (some_function.first == identifier)
					{
						return some_function.second;
					}
				}
			}

			return std::nullopt;
		}

		std::optional<size_t> GetVariableStackOffsetFromBottom(std::string identifier)
		{
			size_t offset = 0;

			for (const VMScopeStack& scope : scopes)
			{
				for (const VMIdentifier& some_identifier : scope.GetIdentifiers())
				{
					if (some_identifier.GetIdentifier() == identifier)
					{
						return offset;
					}
					++offset;
				}
				offset += scope.Temporaries;
			}

			return std::nullopt;
		}

		size_t GetVariableStackSize()
		{
			size_t size = 0;

			for (const VMScopeStack& scope : scopes)
			{
				size += scope.GetIdentifiers().size() + scope.Temporaries;
			}

			return size;
		}

		std::optional<size_t> GetVariableStackOffsetFromTop(std::string identifier)
		{
			std::optional<size_t> offset_from_bottom = GetVariableStackOffsetFromBottom(identifier);
			if (!offset_from_bottom)
			{
				return std::nullopt;
			}
			return GetVariableStackSize() - 1 - *offset_from_bottom;
		}

		bool PushVariable(VMIdentifier identifier)
		{
			if (HasVariable(identifier.GetIdentifier()))
			{
				return false;
			}
			else
			{
				assert(scopes.back().PushVariable(identifier));
				return true;
			}
		}

		bool HasVariable(std::string identifier)
		{
			for (const VMScopeStack& scope : scopes)
			{
				if (scope.HasVariable(identifier))
				{
					return true;
				}
			}

			return false;
		}

		void EmitOpCode(VMOpCode op_code, std::string debug_message = "")
		{
			instructions.push_back(static_cast<int32_t>(op_code));
			debug_message = debug_message == "" ? (OpCodeToString(op_code)) : (OpCodeToString(op_code) + " (" + debug_message + ")");
			debug_message += " [" + scopes.back().GetDebugLabel() + "]";
			debug_instructions.push_back(debug_message);
		}

		size_t EmitOperand(int32_t operand, std::string debug_message = "")
		{
			instructions.push_back(operand);
			debug_message = debug_message == "" ? (std::to_string(operand)) : (std::to_string(operand) + " (" + debug_message + ")");
			debug_message += " [" + scopes.back().GetDebugLabel() + "]";
			debug_instructions.push_back(debug_message);
			return instructions.size() - 1;
		}

		void UpdateOperand(size_t offset, int32_t operand, std::string debug_message = "")
		{
			instructions[offset] = operand;
			debug_message = debug_message == "" ? (std::to_string(operand)) : (std::to_string(operand) + " (" + debug_message + ")");
			debug_message += " [" + scopes.back().GetDebugLabel() + "]";
			debug_instructions[offset] = debug_message;
		}

		const std::vector<int32_t>& GetInstructions()
		{
			return instructions;
		}

		const std::vector<std::string>& GetDebugInstructions()
		{
			return debug_instructions;
		}

	private:
		std::vector<int32_t> instructions;
		std::vector<std::string> debug_instructions;
		std::vector<VMScopeStack> scopes; // <- this should be a linear array where each identifier has a numeric scope
		int32_t static_data_size = 0;
	};

	class VMScopedScope
	{
	public:
		VMScopedScope(VMCompileContext& context, std::string debug_label)
			: m_context(context)
		{
			m_context.PushScope(debug_label);
		}

		~VMScopedScope()
		{
			m_context.PopScope();
		}

		VMScopedScope(const VMScopedScope&) = delete;
		VMScopedScope& operator=(const VMScopedScope&) = delete;

		VMScopedScope(VMScopedScope&&) = delete;
		VMScopedScope& operator=(VMScopedScope&&) = delete;

	private:
		VMCompileContext& m_context;
	};

	class VMCompiler : public ASTVisitor
	{
	public:
		const VMCompileContext& GetContext() const
		{
			return m_context;
		}

	private:
		ASTVisitorTraversal Visit(const ASTLiteral& node)
		{
			const std::string dbg_message = std::format("{} literal = {}", TypeToString(node.GetType()), node.GetValue());
			m_context.EmitOpCode(VMOpCode::PUSH, dbg_message);
			m_context.EmitOperand(node.GetValue(), dbg_message);

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTVariable& node)
		{
			const std::optional<size_t> top_offset = m_context.GetVariableStackOffsetFromTop(node.GetIdentifier());
			if (!top_offset)
			{
				std::println("Variable '{}' does not exist", node.GetIdentifier());
				return ASTVisitorTraversal::Stop;
			}

			m_context.EmitOpCode(VMOpCode::DUP);
			m_context.EmitOperand(*top_offset);

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTUnaryExpr& node)
		{
			if (node.GetNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			switch (node.GetOperator())
			{
				case UnaryOperator::Exclamation:
				{
					m_context.EmitOpCode(VMOpCode::NOT);
					break;
				}
				case UnaryOperator::Minus:
				{
					m_context.EmitOpCode(VMOpCode::NEGATE);
					break;
				}
			}

			std::println("Unknown unary operator");
			return ASTVisitorTraversal::Stop;
		}
		
		ASTVisitorTraversal Visit(const ASTBinaryExpr& node)
		{
			if (node.GetLeftNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			if (node.GetRightNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			if (node.GetOperator() == BinaryOperator::Plus)
			{
				m_context.EmitOpCode(VMOpCode::ADD);
			}
			else if (node.GetOperator() == BinaryOperator::Asterisk)
			{
				m_context.EmitOpCode(VMOpCode::MUL);
			}
			else
			{
				std::println("Unknown operator");
				return ASTVisitorTraversal::Stop;
			}

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTVariableDeclarationStmt& node)
		{
			if (!m_context.PushVariable(VMIdentifier(node.GetIdentifier(), node.GetType())))
			{
				std::println("Failed to push variable declaration");
				return ASTVisitorTraversal::Stop;
			}

			// It is guaranteed that this variable will be placed on the top of the data stack
			// so we can just directly push its value onto the stack

			// This expression will have ended in a value being pushed onto the stack
			if (node.GetExpressionNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTReturn& node)
		{
			if (node.GetExpressionNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTBlock& node)
		{
			for (const std::unique_ptr<ASTNode>& expression_node : node.GetStatements())
			{
				if (expression_node->Accept(*this) == ASTVisitorTraversal::Stop)
				{
					return ASTVisitorTraversal::Stop;
				}
			}

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTAssignmentStmt& node)
		{
			std::optional<size_t> top_offset = m_context.GetVariableStackOffsetFromTop(node.GetIdentifier());
			if (!top_offset)
			{
				std::println("Trying to assign to a variable that doesn't exist", node.GetIdentifier());
				return ASTVisitorTraversal::Stop;
			}

			if (node.GetExpressionNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			// Because of the expression above, we will now have 1 extra value on the data stack.
			// The data stack should look like this
			// (BOTTOM) [ _, _, _, k, _, a0, a1, a2, e ] (TOP)
			// where k is the old value we want to assign and e is its new value
			m_context.EmitOpCode(VMOpCode::SWAP);
			m_context.EmitOperand(*top_offset + 1);
			m_context.EmitOpCode(VMOpCode::POP);
			m_context.EmitOperand(1);

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTIfStmt& node)
		{
			std::println("If-statements not supported by the VM compiler");
			return ASTVisitorTraversal::Stop;
		}

		ASTVisitorTraversal Visit(const class ASTFunctionCall& node)
		{
			// Before we call a function, we need to push the address of the instruction we want to continue at once the function returns
			m_context.EmitOpCode(VMOpCode::PUSH);
			size_t return_address_instruction_offset = m_context.EmitOperand(0); // todo

			// Next we need to push all the function args onto the data stack
			// TODO: type check the args match the parameters

			// Each of these expressions should have pushed one more value on the data stack
			for (const auto& arg : node.GetArgs().args)
			{
				if (arg->Accept(*this) == ASTVisitorTraversal::Stop)
				{
					return ASTVisitorTraversal::Stop;
				}
				++m_context.TopScope().Temporaries;
			}
			m_context.TopScope().Temporaries = 0;

			// Push the instruction offset of the function we're calling
			std::optional<size_t> function_instruction_offset = m_context.GetFunctionInstructionOffset(node.GetIdentifier());
			if (!function_instruction_offset)
			{
				std::println("Couldn't call function");
				return ASTVisitorTraversal::Stop;
			}

			m_context.EmitOpCode(VMOpCode::PUSH, std::format("push function instruction offset of '{}'", node.GetIdentifier()));
			m_context.EmitOperand(*function_instruction_offset, "^^");

			// Call it!
			m_context.EmitOpCode(VMOpCode::JMP, std::format("call '{}'", node.GetIdentifier()));

			m_context.UpdateOperand(return_address_instruction_offset, m_context.GetInstructions().size());

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const class ASTFunctionDeclaration& node)
		{
			if (!m_context.PushFunction(node.GetIdentifier()))
			{
				std::println("Failed to compile function '{}': could not register function, does it already exist?", node.GetIdentifier());
				return ASTVisitorTraversal::Stop;
			}

			const VMScopedScope scope(m_context, node.GetIdentifier());

			for (const auto& parameter : node.GetFunctionType().parameters)
			{
				m_context.PushVariable(VMIdentifier(parameter.first, parameter.second));
			}

			// TODO: need type-checking and check function ends in a return statement

			if (node.GetBody()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			// we should now have the return type of the function on the top of the stack
			// TODO: the type-checker actually needs to verify this

			// we should also still have all this function's arguments on the stack just below this
			// (as long as the function body has behaved itself)
			// let's get rid of those and keep the blocks return value

			// e.g:   [ X, Y, Z, a0, a1, a2, r ] -> [ X, Y, Z, r ]
			// where X, Y, Z is some data on the stack we have no idea about
			// a0, a1, a2 are all this function's arguments
			// and r is the return value

			//const int32_t parameter_count = node.GetFunctionType().parameters.size();
			const int32_t identifier_count = m_context.TopScope().GetIdentifiers().size();

			const int32_t cleanup_count = identifier_count;// +parameter_count;

			m_context.EmitOpCode(VMOpCode::SWAP, "cleanup function working data");
			m_context.EmitOperand(cleanup_count, "^^");
			m_context.EmitOpCode(VMOpCode::POP, "^^");
			m_context.EmitOperand(cleanup_count, "^^");

			// we now need to return to the function that called us
			// if the callee behaved themselves then they would have kindly pushed a return address onto the stack
			// (which should be 'Z' in the diagram above)
			// let's go there!
			m_context.EmitOpCode(VMOpCode::SWAP, "get the return address to the top of the stack");
			m_context.EmitOperand(1, "^^");
			m_context.EmitOpCode(VMOpCode::JMP, "return to the callee");

			return ASTVisitorTraversal::Continue;
		}
		
		ASTVisitorTraversal Visit(const class ASTProgram& Node)
		{
			const VMScopedScope global_scope(m_context, "entry");

			// Push the return address to continue from once the main function has been executed
			// It is always instruction 5 (HALT)
			m_context.EmitOpCode(VMOpCode::PUSH, "return address after calling 'main'");
			m_context.EmitOperand(5, "^^");

			// Call the main function
			m_context.EmitOpCode(VMOpCode::PUSH, "function instruction offset of 'main'");
			m_context.EmitOperand(0, "^^"); // This instruction offset gets filled in later
			m_context.EmitOpCode(VMOpCode::JMP, "call 'main'");
			m_context.EmitOpCode(VMOpCode::HALT, "exit the program");

			// Generate instructions for all functions
			for (const std::unique_ptr<ASTFunctionDeclaration>& function : Node.GetFunctions())
			{
				if (function->Accept(*this) == ASTVisitorTraversal::Stop)
				{
					return ASTVisitorTraversal::Stop;
				}
			}

			std::optional<size_t> entry_instruction_offset = m_context.GetFunctionInstructionOffset("main");
			if (!entry_instruction_offset)
			{
				std::println("Failed to compile program: no 'main' function");
				return ASTVisitorTraversal::Stop;
			}

			m_context.UpdateOperand(3, static_cast<int32_t>(*entry_instruction_offset), "^^");
				
			return ASTVisitorTraversal::Continue;
		}

	private:
		VMCompileContext m_context;
	};

	std::optional<VMProgram> Compile(const AST& ast)
	{
		VMCompiler compiler;

		ASTVisitorTraversal result = ast.GetRoot()->Accept(compiler);

		if (result == ASTVisitorTraversal::Stop)
		{
			std::println("Failed to compile");
			return std::nullopt;
		}

		VMCompileContext context = compiler.GetContext();

		const auto& instructions = context.GetInstructions();
		const auto& debug_instructions = context.GetDebugInstructions();
		for (size_t instruction_idx = 0; instruction_idx < instructions.size(); ++instruction_idx)
		{
			std::string msg = debug_instructions[instruction_idx];
			std::println("{}: {}", instruction_idx, msg);
		}

		VMProgram program(context.GetInstructions());

		return program;
	}
}