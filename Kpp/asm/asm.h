#pragma once

#include <lexer/lexer.h>

#include "x64/arithmetic.h"
#include "x64/memory.h"
#include "x64/compare.h"
#include "x64/control.h"
#include "x64/label.h"

namespace kpp
{
	class asm_gen
	{
	private:

		x64::instruction_list instructions;

		ir_gen& ir;

	public:

		asm_gen(ir_gen& ir) : ir(ir) {}
		~asm_gen();

		bool init();
		bool fix_jumps();
		bool fix_calls();

		x64::instruction_list generate_from_binary_op(ir::BinaryOp* i);
		x64::instruction_list generate_from_store(ir::Store* i);
		x64::instruction_list generate_from_value_int(ir::ValueInt* i);
		x64::instruction_list generate_from_alias(ir::Alias* i);
		x64::instruction_list generate_from_load(ir::Load* i);
		x64::instruction_list generate_from_call(ir::Call* i);
		x64::instruction_list generate_from_any_branch(ir::Instruction* i);
	};
}