#include <defs.h>

#include "dom_tree.h"

#include "ir.h"

using namespace kpp;

void dom_tree::create_reversed_postorder_list_internal(ir::Block* curr)
{
	if (!curr)
		return;

	for (const auto& item : curr->items)
	{
		if (auto branch = rtti::safe_cast<ir::Branch>(item))
			create_reversed_postorder_list_internal(branch->target);

		if (auto bcond = rtti::safe_cast<ir::BranchCond>(item))
		{
			create_reversed_postorder_list_internal(bcond->target_if_true);
			create_reversed_postorder_list_internal(bcond->target_if_false);
		}
	}

	if (std::find(reversed_postorder.begin(), reversed_postorder.end(), curr) == reversed_postorder.end())
	{
		curr->reverse_postorder_index = static_cast<int>(reversed_postorder.size());
		reversed_postorder.push_back(curr);
	}
}

void dom_tree::create_reversed_postorder_list()
{
	set_dom(entry, entry);

	entry->reverse_postorder_index = static_cast<int>(prototype->blocks.size()) - 1;

	create_reversed_postorder_list_internal(entry->next);

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

	ordered_doms = decltype(ordered_doms)(doms.begin(), doms.end());

	std::sort(ordered_doms.begin(), ordered_doms.end(), [&](const dom_pair& l, const dom_pair& r)
	{
		return l.first->reverse_postorder_index < r.first->reverse_postorder_index;
	});
}

void dom_tree::set_dom(ir::Block* v, ir::Block* dom)
{
	if (auto it = doms.find(v); it != doms.end())
		it->second = dom;
	else doms.insert({ v, dom });
}

void dom_tree::print()
{
	PRINT(C_BLUE, prototype->name + ":");

	for (const auto& [v, dom] : get_doms())
	{
		PRINT_TABS_NL(C_GREEN, 1, "'%s' dominates '%s'", dom->name.c_str(), v->name.c_str());
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

ir::Block* dom_tree::get_dom(ir::Block* v)
{
	auto it = doms.find(v);
	return (it != doms.end() ? it->second : nullptr);
}