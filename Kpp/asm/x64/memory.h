#pragma once

#include "instruction.h"

namespace kpp
{
	namespace x64
	{
		instruction_list generate_memory_op(ir::Instruction* i);
		instruction_list gen_mov(ir::Value* op1, ir::Value* op2);
		instruction_list gen_mov(reg* r, ir::Value* op, bool inverse = false);
	}
}