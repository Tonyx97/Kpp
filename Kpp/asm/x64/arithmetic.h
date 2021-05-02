#pragma once

#include "instruction.h"

namespace kpp
{
	namespace x64
	{
		instruction_list generate_binary_op_instruction(ir::BinaryOp* i);
		instruction_list gen_add(ir::Value* op1, ir::Value* op2, ir::Value* op3);
		instruction_list gen_sub(ir::Value* op1, ir::Value* op2, ir::Value* op3);
		instruction_list gen_mul(ir::Value* op1, ir::Value* op2, ir::Value* op3);
		instruction_list gen_div(ir::Value* op1, ir::Value* op2, ir::Value* op3);
		instruction_list gen_xor(ir::Value* op1, ir::Value* op2, ir::Value* op3);

		instruction_list gen_xor(reg* r1, reg* r2);
	}
}