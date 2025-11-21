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

		void SetFromTop(size_t offset, int32_t value);
		int32_t GetFromTop(size_t offset);

		void Dump() const;

	private:
		std::vector<int32_t> Stack;
	};
}