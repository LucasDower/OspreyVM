#pragma once

#include "OspreyVM/VMProgram.h"
#include "OspreyVM/VMStack.h"
#include "OspreyVM/VMMemory.h"

#include <optional>
#include <memory>

namespace Osprey
{
	class VM
	{
	public:
		static std::optional<VM> Load(VMProgram program);

		void Execute();

		const VMProgram& GetProgram() const;
		const VMStack& GetStack() const;
		const VMMemory& GetMemory() const;

	private:
		VM(VMProgram program);

		VMProgram m_program;
		VMStack m_stack;
		VMMemory m_memory;
		size_t m_instruction_offset;
	};
}