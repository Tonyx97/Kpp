#include <defs.h>

#include "dom_tree.h"

#include "ir.h"

using namespace kpp;

void dom_tree::create_reversed_postorder_list_internal(ir::Block* curr)
{
	if (!curr || checked_blocks.contains(curr))
		return;

	auto cf_item = curr->get_control_flow_item();

	if (auto branch = rtti::safe_cast<ir::Branch>(cf_item))
		create_reversed_postorder_list_internal(branch->target);
	else if (auto bcond = rtti::safe_cast<ir::BranchCond>(cf_item))
	{
		create_reversed_postorder_list_internal(bcond->target_if_true);
		create_reversed_postorder_list_internal(bcond->target_if_false);
	}

	if (curr == entry)
		return;

	curr->reverse_postorder_index = static_cast<int>(reversed_postorder.size());

	reversed_postorder.push_back(curr);
	checked_blocks.insert(curr);
}

void dom_tree::create_reversed_postorder_list()
{
	set_dom(entry, entry);

	entry->reverse_postorder_index = static_cast<int>(prototype->blocks.size()) - 1;

	create_reversed_postorder_list_internal(entry);

	std::reverse(reversed_postorder.begin(), reversed_postorder.end());
}

void dom_tree::build()
{
	create_reversed_postorder_list();

	bool changed = true;

	while (changed)
	{
		changed = false;

		for (const auto& b : reversed_postorder)
		{
			auto new_idom = b->refs[0];

			for (const auto& p : std::span(b->refs).subspan(1))
				if (auto dom = get_dom(p))
					new_idom = intersect(p, new_idom);

			if (auto bdom = get_dom(b); bdom != new_idom)
			{
				set_dom(b, new_idom);
				changed = false;
			}
		}
	}

	for (const auto& [b, dom] : doms)
		dom->doms.insert(b);

	ordered_doms = decltype(ordered_doms)(doms.begin(), doms.end());

	std::sort(ordered_doms.begin(), ordered_doms.end(), [&](const dom_pair& l, const dom_pair& r)
	{
		return l.first->reverse_postorder_index < r.first->reverse_postorder_index;
	});
}

void dom_tree::set_dom(ir::Block* b, ir::Block* dom)
{
	b->idom = dom;

	if (auto it = doms.find(b); it != doms.end())
		it->second = dom;
	else doms.insert({ b, dom });
}

void dom_tree::print()
{
	PRINT(C_BLUE, prototype->name + ":");

	for (const auto& [b, dom] : get_doms())
	{
		PRINT_TABS_NL(C_GREEN, 1, "'%s' sdom '%s'", dom->name.c_str(), b->name.c_str());
	}

	for (const auto& b : reversed_postorder)
	{
		PRINT_TABS_NL(C_GREEN, 1, "idom(%s) = '%s'", b->name.c_str(), b->idom->name.c_str());
	}

	PRINT_NL;
}

ir::Block* dom_tree::intersect(ir::Block* b1, ir::Block* b2)
{
	auto f1 = b1,
		 f2 = b2;

	while (f1 != f2)
	{
		while (f1->reverse_postorder_index < f2->reverse_postorder_index) f1 = get_dom(f1);
		while (f2->reverse_postorder_index < f1->reverse_postorder_index) f2 = get_dom(f2);
	}

	return f1;
}

ir::Block* dom_tree::get_dom(ir::Block* b)
{
	auto it = doms.find(b);
	return (it != doms.end() ? it->second : nullptr);
}