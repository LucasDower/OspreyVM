#include "VMProgram.h"

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
}