#pragma once

#include "instruction.h"

namespace kpp
{
	namespace x64
	{
		instruction_list generate_compare_instruction(ir::BinaryOp* i);
		instruction_list gen_compare(ir::Value* left, ir::Value* right);
	}
}