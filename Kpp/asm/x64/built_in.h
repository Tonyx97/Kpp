#pragma once

#include "instruction.h"

namespace kpp
{
	namespace x64
	{
		using built_in_fn_t = std::function<instruction_list(ir::Call* i)>;
		
		instruction_list generate_built_in_fn(ir::Call* i);
		instruction_list gen_rdtsc(ir::Call* i);
		instruction_list gen_int3(ir::Call* i);

		bool is_built_in_fn(const std::string& name);
	}
}