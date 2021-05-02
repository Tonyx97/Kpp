#include <defs.h>

#include <ir/ir.h>
#include <ssa/ssa.h>

#include "reg_alloc.h"

using namespace kpp;

reg_alloc::~reg_alloc()
{
}

bool reg_alloc::init()
{
	x64::for_each_register([&](reg* r) { free_regs.insert(r); });

	return true;
}

bool reg_alloc::calculate()
{
	for (auto prototype : ir.get_ir_info().prototypes)
	{
		auto entry = prototype->get_entry_block();
		if (!entry)
			return false;

		if (!calculate_register_usage(prototype->ssa, entry))
			return false;

		if (!undo_phis(prototype))
			return false;

		PRINT(C_DARK_RED, "Max usage: %i", max_usage);

		/*for (auto b : prototype->blocks)
		{
			PRINT(C_CYAN, "'%s':", b->name.c_str());

			PRINT_TABS_NL(C_CYAN, 1, "Block info:");

			for (auto info : b->regs_assigned)
				PRINT_TABS(C_GREEN, 2, "'%s'\n", STRINGIFY_REGISTER(info->name).c_str());

			PRINT_TABS_NL(C_CYAN, 1, "Values info:");

			for (auto i : b->items)
			{
				i->for_each_left_op([&](ir::Value* v)
				{
					if (v->r)
						PRINT_TABS(C_GREEN, 2, "'%s' -> '%s'\n", v->name.c_str(), STRINGIFY_REGISTER(v->r->name).c_str());
					else PRINT_TABS(C_GREEN, 2, "'%s' has no register\n", v->name.c_str());

					return nullptr;
				});
			}
		}

		for (auto v : ctx->ssa_values)
		{
			if (v->r)
				PRINT(C_CYAN, "'%s' -> '%s'", v->name.c_str(), STRINGIFY_REGISTER(v->r->name).c_str());
			else PRINT(C_CYAN, "'%s' has no register (xd)", v->name.c_str());
		}*/
	}

	ir.print_ir();

	return true;
}

bool reg_alloc::calculate_register_usage(ssa_ctx* ctx, ir::Block* block)
{
	for (auto v_in : ctx->in_out[block].in)
		for (auto v_ver : v_in->vers)
			if (v_ver->storage.r && v_ver->life.has_block(block))
				block->regs_assigned.insert(v_ver->storage.r);

	PRINT(C_CYAN, "'%s':", block->name.c_str());

	for (auto i : block->items)
	{
		i->for_each_right_op([&](ir::Value* v)
		{
			if (v->life.last == i)
			{
				free_reg(block, v->storage.r);
				PRINT_TABS_NL(C_RED, 1, "'%s' -> '%s' freed", v->name.c_str(), STRINGIFY_REGISTER(v->storage.r->id).c_str());
			}

			return nullptr;
		});

		i->for_each_left_op([&](ir::Value* v)
		{
			if (v->life.first != v->life.last)
			{
				v->storage.r = alloc_reg(block);
				PRINT_TABS_NL(C_GREEN, 1, "'%s' -> '%s' allocated", v->name.c_str(), STRINGIFY_REGISTER(v->storage.r->id).c_str());
			}

			return nullptr;
		});
	}

	for (auto dom : block->doms)
		if (dom != block)
			calculate_register_usage(ctx, dom);

	return true;
}

bool reg_alloc::undo_phis(ir::Prototype* prototype)
{
	for (auto b : prototype->blocks)
		for (auto phi : b->phis)
			for (auto param : phi->values)
				for (auto predecessor : b->refs)
					if (param->life.has_block(predecessor))
						predecessor->add_phi_alias(phi->op, param);

	return true;
}

void reg_alloc::free_reg(ir::Block* b, reg* r)
{
	if (!r || !r->general)
		return;

	if (used_regs.contains(r))
	{
		b->regs_assigned.erase(r);
		used_regs.erase(r);
		free_regs.insert(r);

		--current_usage;
	}
}

reg* reg_alloc::alloc_reg(ir::Block* b)
{
	if (!free_regs.empty())
	{
		for (auto r : free_regs)
			if (r->general)
			{
				b->regs_assigned.insert(r);
				free_regs.erase(r);
				used_regs.insert(r);

				max_usage = std::max(++current_usage, max_usage);

				return r;
			}
	}

	PRINT(C_RED, "There are no registers left to assign");

	return nullptr;
}