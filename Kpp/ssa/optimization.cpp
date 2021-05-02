#include <defs.h>

#include <ir/ir.h>

#include "optimization.h"

using namespace kpp;

bool optimization::optimize()
{
	const auto& prototypes = ir.get_ir_info().prototypes;

	for (auto prototype : prototypes)
	{
		for (auto b : prototype->blocks)
		{
			for (auto i : b->items)
			{
				if (auto alias = rtti::safe_cast<ir::Alias>(i))
					optimize_alias(alias);
				else if (auto load = rtti::safe_cast<ir::Load>(i))
					optimize_load(load);
			}
		}

		for (auto i : unused_instructions)
			i->block_owner->remove_item(i);

		unused_instructions.clear();
	}

	ir.print_ir();

	return true;
}

bool optimization::optimize_alias(ir::Alias* i)
{
	if (i->op1->storage.r == i->op2->storage.r)
		remove_instruction(i);

	return true;
}

bool optimization::optimize_load(ir::Load* i)
{
	if (i->op1->storage.r == i->op2_i->get_value()->storage.r)
		remove_instruction(i);

	return true;
}