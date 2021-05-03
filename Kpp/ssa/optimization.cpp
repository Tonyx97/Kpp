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
				else if (auto store = rtti::safe_cast<ir::Store>(i))
					optimize_store(store);
			}
		}

		/*for (auto i : unused_instructions)
			i->block_owner->remove_item(i);

		for (auto [from, to] : replaceable_instructions)
			from->block_owner->replace_item(from, to);

		unused_instructions.clear();
		replaceable_instructions.clear();*/
	}

	ir.print_ir();

	return true;
}

bool optimization::optimize_alias(ir::Alias* i)
{
	if (i->op1->storage.r == i->op2->storage.r)
		i->unused = true;

	return true;
}

bool optimization::optimize_load(ir::Load* i)
{
	if (i->op1->storage.r == i->op2_i->get_value()->storage.r)
	{
		i->unused = true;

		/*auto alias = new ir::Alias();

		alias->unused = true;
		alias->op1 = i->op1;
		alias->op2 = i->op2_i->get_value();

		replace_instruction(i, alias);*/
	}

	return true;
}

bool optimization::optimize_store(ir::Store* i)
{
	if (i->op1->storage.r == i->op2_i->get_value()->storage.r)
	{
		i->unused = true;
	}

	return true;
}
