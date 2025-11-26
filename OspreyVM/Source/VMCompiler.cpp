#include "OspreyVM/VMCompiler.h"

#include "OspreyAST/AST.h"
#include "OspreyAST/ASTVisitor.h"
#include "OspreyVM/VMProgram.h"
#include "OspreyVM/VMOpCode.h"
#include "OspreyVM/VMStackBindings.h"

#include "OspreyAST/Expressions/Literal.h"
#include "OspreyAST/Expressions/Variable.h"
#include "OspreyAST/Expressions/UnaryOp.h"
#include "OspreyAST/Expressions/BinaryOp.h"
#include "OspreyAST/Expressions/FunctionCall.h"
#include "OspreyAST/Statements/VariableDecl.h"
#include "OspreyAST/Statements/Return.h"
#include "OspreyAST/Statements/If.h"
#include "OspreyAST/Statements/Block.h"
#include "OspreyAST/Statements/Assignment.h"

#include <vector>
#include <unordered_map>
#include <print>
#include <cassert>
#include <ranges>
#include <set>

namespace Osprey
{
	int32_t TempIndex = 0;

	class VMScopeStack
	{
	public:
		VMScopeStack(int32_t offset_from_bottom)
			: m_offset_from_bottom(offset_from_bottom)
		{
		}

		void ApplyScopeSizeDelta(int32_t delta)
		{
			if (delta > 0)
			{
				for (int32_t i = 0; i < delta; ++i)
				{
					m_bindings.push_back(std::nullopt);
				}
			}
			else if (delta < 0)
			{
				assert(-delta <= m_bindings.size());
				for (int32_t i = 0; i < delta; ++i)
				{
					m_bindings.pop_back();
				}
			}
		}

		int32_t GetOffsetFromBottom() const
		{
			return m_offset_from_bottom;
		}

		int32_t GetCurrentSize() const
		{
			return m_bindings.size();
		}

		bool BindValueToVariable(std::string identifier)
		{
			if (HasVariable(identifier))
			{
				return false;
			}

			if (m_bindings.size() == 0)
			{
				return false;
			}

			if (m_bindings.back()->contains(identifier))
			{
				return false;
			}

			m_bindings.back()->insert(identifier);
			return true;
		}

		bool HasVariable(const std::string& identifier) const
		{
			for (const std::optional<std::set<std::string>>& binding : m_bindings)
			{
				if (binding.has_value())
				{
					if (binding->contains(identifier))
					{
						return true;
					}
				}
			}

			return false;
		}

		//const std::vector<std::string>& GetIdentifiers() const { return m_identifiers; }

	private:
		std::vector<std::optional<std::set<std::string>>> m_bindings;
		int32_t m_offset_from_bottom = 0;
	};

	struct VMInstructionHandle
	{
		int32_t opcode_offset = 0;
		std::optional<int32_t> operand_offset;
	};

	class VMInstruction
	{
	public:
		static VMInstruction PUSH(int32_t value)
		{
			return VMInstruction(VMOpCode::PUSH, value, 1);
		}

		static VMInstruction DUP(int32_t offset)
		{
			return VMInstruction(VMOpCode::DUP, offset, 1);
		}

		static VMInstruction NOT()
		{
			return VMInstruction(VMOpCode::DUP, std::nullopt, 0);
		}

		static VMInstruction NEGATE()
		{
			return VMInstruction(VMOpCode::DUP, std::nullopt, 0);
		}

		static VMInstruction ADD()
		{
			return VMInstruction(VMOpCode::ADD, std::nullopt, -1);
		}

		static VMInstruction MUL()
		{
			return VMInstruction(VMOpCode::MUL, std::nullopt, -1);
		}

		static VMInstruction SWAP(int32_t offset)
		{
			return VMInstruction(VMOpCode::SWAP, offset, 0);
		}

		static VMInstruction POP(int32_t count)
		{
			return VMInstruction(VMOpCode::POP, count, -count);
		}

		static VMInstruction JMP()
		{
			return VMInstruction(VMOpCode::JMP, std::nullopt, -1);
		}

		static VMInstruction HALT()
		{
			return VMInstruction(VMOpCode::HALT, std::nullopt, 0);
		}

		VMOpCode GetOpcode() const { return m_opcode; }
		std::optional<int32_t> GetOperand() const { return m_operand; }
		int32_t GetScopeSizeDelta() const { return m_scope_size_delta; }

	private:
		VMInstruction(VMOpCode opcode, std::optional<int32_t> operand, int32_t scope_size_delta)
			: m_opcode(opcode)
			, m_operand(operand)
			, m_scope_size_delta(scope_size_delta)
		{
		}

		VMOpCode m_opcode;
		std::optional<int32_t> m_operand;
		int32_t m_scope_size_delta;
	};

	class VMCompileContext
	{
	public:
		size_t GetNextInstructionOffset() const
		{
			return instructions.size();
		}

		VMInstructionHandle EmitInstruction(VMInstruction instruction)
		{
			VMInstructionHandle handle;

			handle.opcode_offset = EmitOpCode(instruction.GetOpcode());

			const std::optional<int32_t> operand = instruction.GetOperand();
			if (operand.has_value())
			{
				handle.operand_offset = static_cast<int32_t>(EmitOperand(*operand));
			}

			m_stack_bindings.ApplyOffset(instruction.GetScopeSizeDelta());

			return handle;
		}

		void UpdateOperand(size_t offset, int32_t operand)
		{
			instructions[offset] = operand;
		}

		const std::vector<int32_t>& GetInstructions()
		{
			return instructions;
		}

		VMStackBindings& GetStackBindings() { return m_stack_bindings; }

	private:
		size_t EmitOpCode(VMOpCode op_code)
		{
			instructions.push_back(static_cast<int32_t>(op_code));
			return instructions.size() - 1;
		}

		size_t EmitOperand(int32_t operand)
		{
			instructions.push_back(operand);
			return instructions.size() - 1;
		}


	private:
		VMStackBindings m_stack_bindings;
		std::vector<int32_t> instructions;
		std::vector<VMScopeStack> scopes; // <- this should be a linear array where each identifier has a numeric scope
		int32_t static_data_size = 0;
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
			m_context.EmitInstruction(VMInstruction::PUSH(node.GetValue()));

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTVariable& node)
		{
			const std::optional<int32_t> top_offset = m_context.GetStackBindings().GetBindingOffsetFromTop(node.GetIdentifier());
			if (!top_offset)
			{
				std::println("Variable '{}' does not exist", node.GetIdentifier());
				return ASTVisitorTraversal::Stop;
			}

			m_context.EmitInstruction(VMInstruction::DUP(*top_offset));

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
					m_context.EmitInstruction(VMInstruction::NOT());
					break;
				}
				case UnaryOperator::Minus:
				{
					m_context.EmitInstruction(VMInstruction::NEGATE());
					break;
				}
				default:
				{
					std::println("Unknown unary operator");
					return ASTVisitorTraversal::Stop;
				}
			}

			return ASTVisitorTraversal::Continue;
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
				m_context.EmitInstruction(VMInstruction::ADD());
			}
			else if (node.GetOperator() == BinaryOperator::Asterisk)
			{
				m_context.EmitInstruction(VMInstruction::MUL());
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
			if (node.GetExpressionNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			if (!m_context.GetStackBindings().BindToVariable(node.GetIdentifier()))
			{
				std::println("Failed to push variable declaration");
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
			for (const std::unique_ptr<ASTStmt>& statement : node.GetStatements())
			{
				if (statement->Accept(*this) == ASTVisitorTraversal::Stop)
				{
					return ASTVisitorTraversal::Stop;
				}
			}

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTAssignmentStmt& node)
		{
			std::optional<size_t> top_offset = m_context.GetStackBindings().GetBindingOffsetFromTop(node.GetIdentifier());
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
			m_context.EmitInstruction(VMInstruction::SWAP(*top_offset + 1));
			m_context.EmitInstruction(VMInstruction::POP(1));

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTIfStmt& node)
		{
			std::println("If-statements not supported by the VM compiler");
			return ASTVisitorTraversal::Stop;
		}

		ASTVisitorTraversal Visit(const class ASTFunctionCall& node)
		{
			std::println("Function calls not supported by the VM compiler");
			return ASTVisitorTraversal::Stop;

			/*
			// Before we call a function, we need to push the address of the instruction we want to continue at once the function returns
			const VMInstructionHandle return_address_handle = m_context.EmitInstruction(VMInstruction::PUSH(0));

			// Next we need to push all the function args onto the data stack
			// TODO: type check the args match the parameters

			// Each of these expressions should have pushed one more value on the data stack
			for (const auto& arg : node.GetArgs().args)
			{
				if (arg->Accept(*this) == ASTVisitorTraversal::Stop)
				{
					return ASTVisitorTraversal::Stop;
				}
			}

			// Push the instruction offset of the function we're calling
			std::optional<size_t> function_instruction_offset = m_context.GetFunctionInstructionOffset(node.GetIdentifier());
			if (!function_instruction_offset)
			{
				std::println("Couldn't call function");
				return ASTVisitorTraversal::Stop;
			}

			m_context.EmitInstruction(VMInstruction::PUSH(*function_instruction_offset));

			// Call it!
			m_context.EmitInstruction(VMInstruction::JMP());

			assert(return_address_handle.operand_offset.has_value());
			m_context.UpdateOperand(*return_address_handle.operand_offset, m_context.GetNextInstructionOffset());

			return ASTVisitorTraversal::Continue;
			*/
		}
		
		ASTVisitorTraversal Visit(const class ASTProgram& Node)
		{
			m_context.GetStackBindings().EnterBlock();

			// Push the return address to continue from once the main function has been executed
			// It is always instruction 5 (HALT)
			m_context.EmitInstruction(VMInstruction::PUSH(5));

			// Call the main function
			const VMInstructionHandle main_function_offset_handle = m_context.EmitInstruction(VMInstruction::PUSH(0));

			m_context.EmitInstruction(VMInstruction::JMP());
			m_context.EmitInstruction(VMInstruction::HALT());

			// Generate instructions for all statements
			for (const std::unique_ptr<ASTStmt>& statement : Node.GetStatements())
			{
				if (statement->Accept(*this) == ASTVisitorTraversal::Stop)
				{
					return ASTVisitorTraversal::Stop;
				}
			}

			std::optional<size_t> entry_instruction_offset; // = m_context.GetFunctionInstructionOffset("main");
			if (!entry_instruction_offset)
			{
				std::println("Failed to compile program: no 'main' function");
				return ASTVisitorTraversal::Stop;
			}

			assert(main_function_offset_handle.operand_offset.has_value());
			m_context.UpdateOperand(*main_function_offset_handle.operand_offset, static_cast<int32_t>(*entry_instruction_offset));
				
			m_context.GetStackBindings().ExitBlock();

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

		VMProgram program(context.GetInstructions());

		return program;
	}
}