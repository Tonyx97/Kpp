#pragma once

#include <lexer/lexer.h>

#include "x64/arithmetic.h"
#include "x64/memory.h"
#include "x64/compare.h"
#include "x64/control.h"
#include "x64/stack.h"
#include "x64/alignment.h"
#include "x64/bit.h"
#include "x64/built_in.h"
#include "x64/label.h"

namespace kpp
{
	class asm_gen
	{
	private:

		x64::instruction_list instructions;

		ir_gen& ir;

		ir::Prototype* curr_prototype = nullptr;

	public:

		asm_gen(ir_gen& ir) : ir(ir) {}
		~asm_gen();

		bool init();
		bool fix_jumps();
		bool fix_calls();

		ir::Prototype* get_curr_prototype() const { return curr_prototype; }
	};
}

inline std::unique_ptr<kpp::asm_gen> g_asm;