#include "OspreyVM/VMStackBindings.h"

#include <cassert>

namespace Osprey
{
	void VMStackBindings::ApplyOffset(int32_t delta)
	{
		m_block_sizes.back() += delta;
		assert(m_block_sizes.back() >= 0);
	}
	
	void VMStackBindings::EnterBlock()
	{
		m_block_sizes.push_back(0);
	}

	void VMStackBindings::ExitBlock()
	{
		assert(!m_block_sizes.empty());

		for (size_t binding_index = m_bindings.size(); binding_index-- > 0;)
		{
			Binding& binding = m_bindings[binding_index];
			if (binding.owning_block == m_block_sizes.size() - 1)
			{
				m_bindings.erase(m_bindings.begin() + binding_index);
			}
		}

		ApplyOffset(-m_block_sizes.back());
		m_block_sizes.pop_back();
	}

	int32_t VMStackBindings::GetStackSize() const
	{
		int32_t size = 0;

		for (const int32_t block_size : m_block_sizes)
		{
			size += block_size;
		}

		return size;
	}

	int32_t VMStackBindings::GetTopStackSize() const
	{
		return m_block_sizes.back();
	}

	bool VMStackBindings::BindToVariable(std::string variable)
	{
		for (const Binding& binding : m_bindings)
		{
			if (binding.identifier == variable)
			{
				return false;
			}
		}

		assert(m_block_sizes.back() > 0); // nothing to bind to

		const int32_t bottom_offset = GetStackSize() - 1;

		m_bindings.push_back(Binding(variable, bottom_offset, m_block_sizes.size() - 1));
	}

	std::optional<int32_t> VMStackBindings::GetBindingOffsetFromTop(std::string_view variable) const
	{
		for (const Binding& binding : m_bindings)
		{
			if (binding.identifier == variable)
			{
				return GetStackSize() - 1 - binding.bottom_offset;
			}
		}

		return std::nullopt;
	}
}