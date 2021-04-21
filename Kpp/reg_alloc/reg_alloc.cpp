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

		/*for (auto b : prototype->blocks)
		{
			PRINT(C_CYAN, "'%s':", b->name.c_str());

			PRINT_TABS_NL(C_CYAN, 1, "Block info:");

			for (auto info : b->regs_assigned)
				PRINT_TABS(C_GREEN, 2, "'%s'\n", STRINGIFY_REGISTER(info->name).c_str());

			PRINT_TABS_NL(C_CYAN, 1, "Values info:");

			for (auto i : b->items)
			{
				i->for_each_lvalue([&](ir::Value* v)
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

	return true;
}

bool reg_alloc::calculate_register_usage(ssa_ctx* ctx, ir::Block* block)
{
	for (auto v_in : ctx->in_out[block].in)
		for (auto v_ver : v_in->vers)
			if (v_ver->r && v_ver->life.has_block(block))
				block->regs_assigned.insert(v_ver->r);

	PRINT(C_CYAN, "'%s':", block->name.c_str());

	for (auto i : block->items)
	{
		i->for_each_rvalue([&](ir::Value* v)
		{
			if (v->life.last == i)
			{
				free_reg(block, v->r);
				PRINT_TABS_NL(C_RED, 1, "'%s' -> '%s' freed", v->name.c_str(), STRINGIFY_REGISTER(v->r->id).c_str());
			}

			return nullptr;
		});

		i->for_each_lvalue([&](ir::Value* v)
		{
			if (v->life.first != v->life.last)
			{
				v->r = alloc_reg(block);
				PRINT_TABS_NL(C_GREEN, 1, "'%s' -> '%s' allocated", v->name.c_str(), STRINGIFY_REGISTER(v->r->id).c_str());
			}

			return nullptr;
		});
	}

	for (auto dom : block->doms)
		if (dom != block)
			calculate_register_usage(ctx, dom);

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

				return r;
			}
	}

	PRINT(C_RED, "There are no registers left to assign");

	return nullptr;
}