#pragma once

#include "instruction.h"

namespace kpp
{
	namespace x64
	{
		instruction_list generate_control_instruction(ir::Instruction* i);
		instruction_list gen_cond_jump(ir::BranchCond* i);
		instruction_list gen_jump(ir::Branch* i);
		instruction_list gen_ret(ir::Return* i);
	}
}