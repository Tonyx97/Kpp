#pragma once

#include <lexer/lexer.h>

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
	};
}