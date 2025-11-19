#pragma once

#include <vector>
#include <cstdint>

namespace Osprey
{
	class VMStack
	{
	public:
		int32_t Pop();
		void Push(int32_t Data);

		void Dump() const;

	private:
		std::vector<int32_t> Stack;
	};
}