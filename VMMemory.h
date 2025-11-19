#pragma once

#include <vector>
#include <cstdint>

namespace Osprey
{
	class VMMemory
	{
	public:
		VMMemory(size_t Size);

		int32_t Get(int32_t Address) const;
		void Set(size_t Address, int32_t Value);

	private:
		std::vector<int32_t> Data;
	};
}