#pragma once

#include <vector>

namespace Osprey
{
	class VMProgram
	{
	public:
		VMProgram(std::vector<int32_t> in_program);

		int32_t GetInstruction(size_t offset);

		void Dump() const;

	private:
		std::vector<int32_t> m_program;
	};
}