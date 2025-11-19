#include "VMMemory.h"

namespace Osprey
{
	VMMemory::VMMemory(size_t Size)
	{
		Data.resize(Size);
	}

	void VMMemory::Set(size_t Address, int32_t Value)
	{
		Data[Address] = Value;
	}

	int32_t VMMemory::Get(int32_t Address) const
	{
		return Data[Address];
	}
}