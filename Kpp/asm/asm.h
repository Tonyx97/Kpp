#pragma once

#include <lexer/lexer.h>

#include "x64/arithmetic.h"
#include "x64/memory.h"
#include "x64/compare.h"
#include "x64/control.h"

namespace kpp
{
	class asm_gen
	{
	private:

		ir_gen& ir;

	public:

		asm_gen(ir_gen& ir) : ir(ir) {}
		~asm_gen();

		bool init();

		std::vector<x64::Instruction*> generate_from_binary_op(ir::BinaryOp* i);
		std::vector<x64::Instruction*> generate_from_store(ir::Store* i);
		std::vector<x64::Instruction*> generate_from_value_int(ir::ValueInt* i);
		std::vector<x64::Instruction*> generate_from_alias(ir::Alias* i);
		std::vector<x64::Instruction*> generate_from_any_branch(ir::Instruction* i);
	};
}