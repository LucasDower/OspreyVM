#include "OspreyVM/VMProgram.h"

#include "OspreyVM/VMOpCode.h"

#include <print>

namespace Osprey
{
	VMProgram::VMProgram(std::vector<int32_t> in_program)
		: m_program(in_program)
	{
	}

	int32_t VMProgram::GetInstruction(size_t offset)
	{
		return m_program[offset];
	}

	void VMProgram::Dump() const
	{
		size_t instruction_offset = 0;
		const size_t program_size = m_program.size();

		bool stop = false;

		while (instruction_offset < program_size && !stop)
		{
			const auto start_instruction_offset = instruction_offset;
			const VMOpCode opcode = static_cast<VMOpCode>(m_program[instruction_offset]);
			++instruction_offset;

			switch (opcode)
			{
				case VMOpCode::PUSH:
				{
					const int32_t operand = m_program[instruction_offset++];
					std::println("{}: PUSH {}", start_instruction_offset, operand);
					break;
				}
				case VMOpCode::POP:
				{
					const int32_t operand = m_program[instruction_offset++];
					std::println("{}: POP {}", start_instruction_offset, operand);
					break;
				}
				case VMOpCode::DUP:
				{
					const int32_t operand = m_program[instruction_offset++];
					std::println("{}: DUP {}", start_instruction_offset, operand);
					break;
				}
				case VMOpCode::SWAP:
				{
					const int32_t operand = m_program[instruction_offset++];
					std::println("{}: SWAP {}", start_instruction_offset, operand);
					break;
				}
				case VMOpCode::JMP:
				{
					std::println("{}: JMP", start_instruction_offset);
					break;
				}
				case VMOpCode::HALT:
				{
					std::println("{}: HALT", start_instruction_offset);
					break;
				}
				default:
				{
					std::println("Unknown opcode");
				}
			}
		}

	}
}