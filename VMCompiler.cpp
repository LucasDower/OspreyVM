#include "VMCompiler.h"

#include "AST.h"
#include "VMProgram.h"
#include "ASTVisitor.h"
#include "VMOpCode.h"

#include <vector>
#include <unordered_map>
#include <print>

namespace Osprey
{
	struct VMCompileContext
	{
		void EmitOpCode(VMOpCode op_code)
		{
			instructions.push_back(static_cast<int32_t>(op_code));
		}

		void EmitOperand(int32_t operand)
		{
			instructions.push_back(operand);
		}

		int32_t GetOrAddIdentifierAddress(const std::string& identifier)
		{
			const std::optional<int32_t> address = GetIdentifierAddress(identifier);
			if (address.has_value())
			{
				return *address;
			}

			int32_t new_address = static_data_size++;
			identifier_to_address.insert({ identifier, new_address });

			return new_address;
		}

		std::optional<int32_t> GetIdentifierAddress(const std::string& identifier)
		{
			auto address_it = identifier_to_address.find(identifier);
			if (address_it == identifier_to_address.end())
			{
				return std::nullopt;
			}
			return address_it->second;
		}

		std::vector<int32_t> instructions;
		std::unordered_map<std::string, int32_t> identifier_to_address;
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
		ASTVisitorTraversal Visit(const ASTIntegerLiteralNode& node)
		{
			m_context.EmitOpCode(VMOpCode::PUSH);
			m_context.EmitOperand(node.GetValue());

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTIntegerVariableNode& node)
		{
			const std::optional<int32_t> address = m_context.GetIdentifierAddress(node.GetIdentifier());
			if (!address)
			{
				std::println("Variable '{}' does not exist", node.GetIdentifier());
				return ASTVisitorTraversal::Stop;
			}

			m_context.EmitOpCode(VMOpCode::LOAD);
			m_context.EmitOperand(*address);

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTIntegerAddNode& node)
		{
			if (node.GetLeftNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			if (node.GetRightNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			m_context.EmitOpCode(VMOpCode::ADD);

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTIntegerVariableAssignNode& node)
		{
			int32_t address = m_context.GetOrAddIdentifierAddress(node.GetIdentifier());

			if (node.GetExpressionNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			m_context.EmitOpCode(VMOpCode::STORE);
			m_context.EmitOperand(address);

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTReturnNode& node)
		{
			if (node.GetExpressionNode()->Accept(*this) == ASTVisitorTraversal::Stop)
			{
				return ASTVisitorTraversal::Stop;
			}

			m_context.EmitOpCode(VMOpCode::HALT);

			return ASTVisitorTraversal::Continue;
		}

		ASTVisitorTraversal Visit(const ASTBlockNode& node)
		{
			for (const std::unique_ptr<ASTNode>& expression_node : node.GetExpressionNodes())
			{
				if (expression_node->Accept(*this) == ASTVisitorTraversal::Stop)
				{
					return ASTVisitorTraversal::Stop;
				}
			}

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

		VMProgram program(context.instructions);

		return program;
	}
}