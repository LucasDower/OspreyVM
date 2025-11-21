#include "VM.h"
#include "VMOpCode.h"

#include <print>

namespace Osprey
{
	VM::VM(VMProgram program)
		: m_program(std::move(program))
		, m_stack()
		, m_memory(1'024)
		, m_instruction_offset(0)
	{
	}

	const VMProgram& VM::GetProgram() const
	{
		return m_program;
	}

	const VMStack& VM::GetStack() const
	{
		return m_stack;
	}

	const VMMemory& VM::GetMemory() const
	{
		return m_memory;
	}

	std::optional<VM> VM::Load(VMProgram program)
	{
		return VM(std::move(program));
	}

	void VM::Execute()
	{
		bool is_running = true;

		while (is_running)
		{
			const VMOpCode instruction = static_cast<VMOpCode>(m_program.GetInstruction(m_instruction_offset++));

			switch (instruction)
			{
				case VMOpCode::PUSH:
				{
					const int32_t value = m_program.GetInstruction(m_instruction_offset++);
					m_stack.Push(value);
					break;
				}
				case VMOpCode::POP:
				{
					const int32_t count_to_pop = m_program.GetInstruction(m_instruction_offset++);
					for (int32_t i = 0; i < count_to_pop; ++i)
					{
						m_stack.Pop();
					}
					break;
				}
				case VMOpCode::DUP:
				{
					const int32_t offset_to_dup = m_program.GetInstruction(m_instruction_offset++);
					int32_t value = m_stack.GetFromTop(offset_to_dup);
					m_stack.Push(value);
					break;
				}
				case VMOpCode::ADD:
				{
					const int32_t left = m_stack.Pop();
					const int32_t right = m_stack.Pop();
					m_stack.Push(left + right);
					break;
				}
				case VMOpCode::MUL:
				{
					const int32_t left = m_stack.Pop();
					const int32_t right = m_stack.Pop();
					m_stack.Push(left * right);
					break;
				}
				case VMOpCode::STORE:
				{
					int32_t address = m_program.GetInstruction(m_instruction_offset++);
					int32_t value = m_stack.Pop();
					m_memory.Set(address, value);
					break;
				}
				case VMOpCode::LOAD:
				{
					int32_t address = m_program.GetInstruction(m_instruction_offset++);
					int32_t value = m_memory.Get(address);
					m_stack.Push(value);
					break;
				}
				case VMOpCode::LT:
				{
					// Stack: (Bottom)   (Top)
					//          |          |
					//        [ X, ..., A, B ]
					const int32_t left = m_stack.Pop();
					const int32_t right = m_stack.Pop();
					m_stack.Push(left < right ? 1 : 0);
					break;
				}
				case VMOpCode::JZ:
				{
					int32_t new_instruction_offset = m_program.GetInstruction(m_instruction_offset++);
					int32_t value = m_stack.Pop();
					if (value == 0)
					{
						m_instruction_offset = new_instruction_offset;
					}
					break;
				}
				case VMOpCode::JMP:
				{
					int32_t address = m_stack.Pop();
					m_instruction_offset = address;
					break;
				}
				case VMOpCode::HALT:
				{
					is_running = false;
					break;
				}
				case VMOpCode::SWAP:
				{
					int32_t top_offset = m_program.GetInstruction(m_instruction_offset++);
					if (top_offset > 0)
					{
						int32_t top_value = m_stack.GetFromTop(0);
						int32_t other_value = m_stack.GetFromTop(top_offset);
						m_stack.SetFromTop(top_offset, top_value);
						m_stack.SetFromTop(0, other_value);
					}
					break;
				}
				default:
				{
					std::println("Unknown opcode: {}", OpCodeToString(instruction));
					is_running = false;
					break;
				}
			}
		}
	}
}