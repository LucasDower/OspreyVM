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

	void VMStack::SetFromTop(size_t offset, int32_t value)
	{
		Stack[Stack.size() - offset - 1] = value;
	}

	int32_t VMStack::GetFromTop(size_t offset)
	{
		return Stack[Stack.size() - offset - 1];
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