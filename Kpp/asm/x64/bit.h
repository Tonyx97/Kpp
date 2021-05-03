#pragma once

#include "instruction.h"

namespace kpp
{
	namespace x64
	{
		instruction_list gen_bit_and(ir::Value* op1, ir::Value* op2, ir::Value* op);
		instruction_list gen_bit_or(ir::Value* op1, ir::Value* op2, ir::Value* op);
		instruction_list gen_bit_shr(ir::Value* op1, ir::Value* op2, ir::Value* op);
		instruction_list gen_bit_shl(ir::Value* op1, ir::Value* op2, ir::Value* op);
	}
}