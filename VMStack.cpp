#include "VMStack.h"

#include <iostream>

namespace Osprey
{
	int32_t VMStack::Pop()
	{
		int32_t Top = Stack.back();
		Stack.pop_back();
		return Top;
	}

	void VMStack::Push(int32_t Data)
	{
		Stack.push_back(Data);
	}

	void VMStack::Dump() const
	{
		for (int32_t Value : Stack)
		{
			std::cout << Value << std::endl;
		}
	}
}