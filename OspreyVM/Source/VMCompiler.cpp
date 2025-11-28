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
#include "OspreyAST/Expressions/FunctionExpression.h"
#include "OspreyAST/Statements/VariableDecl.h"
#include "OspreyAST/Statements/Return.h"
#include "OspreyAST/Statements/If.h"
#include "OspreyAST/Statements/Block.h"
#include "OspreyAST/Statements/Assignment.h"
#include "OspreyAST/Statements/FunctionDecl.h"

#include <vector>
#include <unordered_map>
#include <print>
#include <cassert>
#include <ranges>
#include <set>

namespace Osprey
{
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

	enum class VMCompilePhase
	{
		None,
		FirstPass,
		DeferredFunctions,
	};

	class VMCompileContext
	{
	public:
		void RegisterFunctionToCompile(ASTFunctionExpr* function, VMInstructionHandle handle_to_fix)
		{
			m_deferred_functions.push_back({ function, handle_to_fix });
		}

		const std::vector<std::pair<ASTFunctionExpr*, VMInstructionHandle>>& GetDeferredFunctions() const
		{
			return m_deferred_functions;
		}

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

		VMCompilePhase GetPhase() const { return m_phase; }
		void SetPhase(VMCompilePhase phase) { assert(phase > m_phase); m_phase = phase; }

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
		std::vector<std::pair<ASTFunctionExpr*, VMInstructionHandle>> m_deferred_functions;

		VMStackBindings m_stack_bindings;
		std::vector<int32_t> instructions;
		VMCompilePhase m_phase = VMCompilePhase::None;
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
			m_context.GetStackBindings().EnterBlock();

			for (const std::unique_ptr<ASTStmt>& statement : node.GetStatements())
			{
				if (statement->Accept(*this) == ASTVisitorTraversal::Stop)
				{
					return ASTVisitorTraversal::Stop;
				}
			}

			m_context.GetStackBindings().ExitBlock();

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

		ASTVisitorTraversal Visit(const ASTFunctionDeclarationStmt& node)
		{
			VMInstructionHandle handle = m_context.EmitInstruction(VMInstruction::PUSH(0));
			m_context.GetStackBindings().BindToVariable(node.GetIdentifier());

			// We need to push the instruction offset of the function onto the
			// data stack and bind the identifier to that value.

			// We haven't compiled the function's instructions into the
			// bytecode buffer yet so we need to use a placeholder 0 value
			// until it has been compiled.

			// We'll add this node to a 'To Compile' list and add make a note
			// of this operand that we need to fix.

			m_context.RegisterFunctionToCompile(node.GetFunction().get(), handle);

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const class ASTFunctionExpr& node)
		{
			assert(m_context.GetPhase() == VMCompilePhase::DeferredFunctions);

			//m_context.GetStackBindings().EnterBlock();

			if (node.GetBody()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			// The semantic analyser should check that the body ends in a return statement.
			// The data stack looks something like this where x, y, z are the functions arguments:
			// 
			// offset:       4                   3  2  1  0
			// stack:  [..., caller_return_addr, x, y, z, return_value ]
			//
			// we want to transform it into:
			//
			// offset: 
			// stack:  [..., return_value, caller_return_addr ]
			//
			// and then call JMP

			const int32_t return_address_offset = m_context.GetStackBindings().GetTopStackSize();

			if (node.GetParameters().size() > 0)
			{
				m_context.EmitInstruction(VMInstruction::SWAP(return_address_offset - 1));
				// now looks like [..., caller_return_addr, return_value, y, z, x ]

				m_context.EmitInstruction(VMInstruction::POP(node.GetParameters().size()));
				// now looks like [..., caller_return_addr, return_value ]

			}

			m_context.EmitInstruction(VMInstruction::SWAP(1));
			// finally [..., return_value, caller_return_addr ]

			m_context.EmitInstruction(VMInstruction::JMP());

			//m_context.GetStackBindings().ExitBlock();

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const class ASTFunctionCall& node)
		{
			const VMInstructionHandle return_instruction_offset = m_context.EmitInstruction(VMInstruction::PUSH(0));

			std::optional<int32_t> function_instruction_offset =  m_context.GetStackBindings().GetBindingOffsetFromTop(node.GetIdentifier());
			if (!function_instruction_offset)
			{
				std::println("Failed to call undefined function '{}'", node.GetIdentifier());
				return ASTVisitorTraversal::Stop;
			}

			// TODO: handle args (may have to go before function_instruction_offset is calculated

			m_context.EmitInstruction(VMInstruction::DUP(*function_instruction_offset));
			m_context.EmitInstruction(VMInstruction::JMP());

			const auto next_instruction_offset = m_context.GetNextInstructionOffset();
			m_context.UpdateOperand(*return_instruction_offset.operand_offset, next_instruction_offset);

			return ASTVisitorTraversal::Continue;
		}
		
		ASTVisitorTraversal Visit(const class ASTProgram& Node)
		{
			m_context.GetStackBindings().EnterBlock();

			// Generate instructions for all statements (except function expressions that we compile last)
			{
				m_context.SetPhase(VMCompilePhase::FirstPass);

				for (const std::unique_ptr<ASTStmt>& statement : Node.GetStatements())
				{
					if (statement->Accept(*this) == ASTVisitorTraversal::Stop)
					{
						return ASTVisitorTraversal::Stop;
					}
				}
			}

			// Create a fake call function node
			ASTFunctionCall main_call_node("main", {});
			if (main_call_node.Accept(*this) == ASTVisitorTraversal::Stop)
			{
				std::println("Failed to call 'main' function");
				return ASTVisitorTraversal::Stop;
			}

			// We now have the return value on the stack unaccounted for, let's adjust the binding offsets to accomodate
			//m_context.GetStackBindings().ApplyOffset(1); // TODO: maybe needed

			// Once main returns we need to halt the program
			m_context.EmitInstruction(VMInstruction::HALT());

			std::optional<int32_t> main_stack_entry = m_context.GetStackBindings().GetBindingOffsetFromTop("main");
			if (!main_stack_entry)
			{
				std::println("Failed to compile program: no 'main' function");
				return ASTVisitorTraversal::Stop;
			}

			// Generate instructions for all function expressions
			{
				m_context.SetPhase(VMCompilePhase::DeferredFunctions);

				for (const auto& deferred_function : m_context.GetDeferredFunctions())
				{
					const auto function_entry_offset = m_context.GetNextInstructionOffset();
					m_context.UpdateOperand(*deferred_function.second.operand_offset, function_entry_offset);

					if (deferred_function.first->Accept(*this) == ASTVisitorTraversal::Stop)
					{
						std::println("Failed to compile function");
						return ASTVisitorTraversal::Stop;
					}
				}
			}

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