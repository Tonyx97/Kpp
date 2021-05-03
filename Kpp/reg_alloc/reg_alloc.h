#pragma once

#include <asm/x64/registers.h>

namespace kpp
{
	class reg_alloc
	{
	private:

		ir_gen& ir;

		std::set<reg*, reg_set_order> used_regs,
									  free_regs;

		int current_usage = 0,
			max_usage = 0;

	public:

		reg_alloc(ir_gen& ir) : ir(ir) {}
		~reg_alloc();

		bool init();
		bool calculate();
		bool calculate_register_usage(ssa_ctx* ctx, ir::Block* block);
		bool undo_phis(ir::Prototype* prototype);

		void free_reg(ir::Block* b, reg* r);

		reg* alloc_reg(ir::Block* b);
	};
}