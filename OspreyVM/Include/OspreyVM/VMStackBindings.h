#pragma once

#include <string>
#include <vector>
#include <optional>

namespace Osprey
{
	class VMStackBindings
	{
	public:
		void EnterBlock();
		void ExitBlock();

		void ApplyOffset(int32_t delta);

		bool BindToVariable(std::string variable);
		int32_t GetStackSize() const;

		std::optional<int32_t> GetBindingOffsetFromTop(std::string_view variable) const;

	private:
		struct Binding
		{
			Binding(std::string in_identifier, int32_t in_bottom_offset, int32_t in_owning_block)
				: identifier(in_identifier)
				, bottom_offset(in_bottom_offset)
				, owning_block(in_owning_block)
			{
			}

			std::string identifier;
			int32_t bottom_offset = 0;
			int32_t owning_block = 0;
		};

		std::vector<int32_t> m_block_sizes;
		std::vector<Binding> m_bindings;
	};
}