#pragma once

#include <cstdint>
#include <string>

namespace Osprey
{
	enum class VMOpCode : uint8_t
	{
		PUSH,
		POP,
		ADD,
		NOT,
		NEGATE,
		MUL,
		LOAD,
		STORE,
		LT,
		JZ,
		JMP,
		HALT,
		SWAP,
		DUP,
	};

	inline static std::string OpCodeToString(VMOpCode opcode)
	{
		switch (opcode)
		{
		case VMOpCode::PUSH:
			return "PUSH";
		case VMOpCode::POP:
			return "POP";
		case VMOpCode::ADD:
			return "ADD";
		case VMOpCode::NOT:
			return "NOT";
		case VMOpCode::NEGATE:
			return "NEGATE";
		case VMOpCode::MUL:
			return "MUL";
		case VMOpCode::LOAD:
			return "LOAD";
		case VMOpCode::STORE:
			return "STORE";
		case VMOpCode::LT:
			return "LT";
		case VMOpCode::JZ:
			return "JZ";
		case VMOpCode::JMP:
			return "JMP";
		case VMOpCode::HALT:
			return "HALT";
		case VMOpCode::SWAP:
			return "SWAP";
		case VMOpCode::DUP:
			return "DUP";
		}
		return "<Unknown OpCode>";
	}
}