#include <defs.h>

#include "ir.h"

#include "reg_alloc.h"

using namespace kpp;

bool reg_alloc::allocate_all()
{
	for (auto&& p : ir.prototypes)
	{
		PRINT(C_RED, "PROTOTYPE '%s'", p->name.c_str());

		auto entry_block = p->get_entry_block();
		if (!entry_block)
			continue;

		simulate_control_flow_internal(entry_block, [&](ir::Instruction* i)
		{
			if (auto value = i->get_value())
				PRINT(C_RED, "'%s'", value->name.c_str());
		});
	}

	return true;
}

bool reg_alloc::simulate_control_flow_internal(ir::Block* curr_block, const std::function<void(ir::Instruction*)>& fn)
{
	for (const auto& i : curr_block->items)
	{
		fn(i);

		if (auto branch = rtti::safe_cast<ir::Branch>(i))
			return simulate_control_flow_internal(branch->target, fn);
		else if (auto bcond = rtti::safe_cast<ir::BranchCond>(i))
			return simulate_control_flow_internal(bcond->target_if_true, fn) &&
				   simulate_control_flow_internal(bcond->target_if_false, fn);
	}

	return true;
}

bool reg_alloc::simulate_control_flow_global(ir::Prototype* prototype, ir::Block* curr_block, const std::function<void(ir::Instruction*)>& fn)
{
	if (!curr_block)
		curr_block = prototype->get_entry_block();

	if (!curr_block)
		return false;

	for (const auto& i : curr_block->items)
	{
		fn(i);

		if (auto branch = rtti::safe_cast<ir::Branch>(i))
			return simulate_control_flow_global(prototype, branch->target, fn);
		else if (auto bcond = rtti::safe_cast<ir::BranchCond>(i))
			return simulate_control_flow_global(prototype, bcond->target_if_true, fn) &&
				   simulate_control_flow_global(prototype, bcond->target_if_false, fn);
		else if (auto call = rtti::safe_cast<ir::Call>(i))
			return simulate_control_flow_global(call->prototype, nullptr, fn);
	}

	return true;
}