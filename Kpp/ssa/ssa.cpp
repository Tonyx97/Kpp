#include <defs.h>

#include <ir/ir.h>

#include "ssa.h"

using namespace kpp;

bool ssa_gen::build_ssa()
{
	enable_debug = true;

	for (auto prototype : ir.get_ir_info().prototypes)
	{
		bool ok = false;

		auto ctx = std::unique_ptr<ssa_ctx, std::function<void(ssa_ctx*)>>(new ssa_ctx(), [&](ssa_ctx* v) { if (!ok) delete v; });

		auto entry = prototype->get_entry_block();
		if (!entry)
			return false;

		if (!build_def_use(entry))
			return false;

		if (!build_in_out(entry))
			return false;

		if (!build_dominance_tree(prototype, entry))
			return false;

		if (!build_dominance_frontier(prototype, entry))
			return false;

		if (!insert_phis())
			return false;

		if (!rename_values(entry))
			return false;

		if (!clean_load_and_stores(entry))
			return false;

		if (!calculate_lives(entry))
			return false;

		ok = true;

		prototype->ssa = ctx.get();
	}

	return true;
}

bool ssa_gen::build_def_use(ir::Block* entry)
{
	PROFILE("DEF/USE Construction");

	walk_control_flow_by_block(WalkType::PRE_ORDER, entry, [&](ir::Block* b)
	{
		auto& defs_set = ctx.defs[b],
			& uses_set = ctx.uses[b];

		std::unordered_set<ir::Value*> defined_values,
									   used_values;

		for (auto i : b->items)
		{
			i->for_each_lvalue([&](ir::Value* v)
			{
				if (!used_values.contains(v))
				{
					ctx.defs_by_blocks[v].insert(b);
					defs_set.insert(v);
				}

				defined_values.insert(v);

				return nullptr;
			});

			i->for_each_rvalue([&](ir::Value* v)
			{
				if (!defined_values.contains(v))
				{
					ctx.uses_by_blocks[v].insert(b);
					uses_set.insert(v);
				}

				used_values.insert(v);

				return nullptr;
			});
		}

		return true;
	});

	if (enable_debug)
	{
		PRINT_TABS_NL(C_RED, 1, "DEFS:");

		for (auto [b, vset] : ctx.defs)
		{
			PRINT_TABS(C_YELLOW, 2, "DEFS[%s] = { ", b->name.c_str());
			dbg::print_set<ir::Value*>(C_GREEN, vset, ", ", [](auto p) { return p->name; });
			PRINT(C_YELLOW, " }");
		}

		PRINT_TABS_NL(C_RED, 1, "USES:");

		for (auto [b, vset] : ctx.uses)
		{
			PRINT_TABS(C_YELLOW, 2, "USES[%s] = { ", b->name.c_str());
			dbg::print_set<ir::Value*>(C_GREEN, vset, ", ", [](auto p) { return p->name; });
			PRINT(C_YELLOW, " }");
		}

		PRINT_TABS_NL(C_RED, 1, "DEFS BY BLOCKS:");

		for (auto [v, b] : ctx.defs_by_blocks)
		{
			PRINT_TABS(C_YELLOW, 2, "DEFS[%s] = { ", v->name.c_str());
			dbg::print_set<ir::Value*>(C_GREEN, b, ", ", [](auto p) { return p->name; });
			PRINT(C_YELLOW, " }");
		}

		PRINT_TABS_NL(C_RED, 1, "USES BY BLOCKS:");

		for (auto [v, b] : ctx.uses_by_blocks)
		{
			PRINT_TABS(C_YELLOW, 2, "USES[%s] = { ", v->name.c_str());
			dbg::print_set<ir::Value*>(C_GREEN, b, ", ", [](auto p) { return p->name; });
			PRINT(C_YELLOW, " }");
		}

		PRINT_NL;
	}

	return true;
}

bool ssa_gen::build_in_out(ir::Block* entry)
{
	PROFILE("IN/OUT Construction");

	bool in_same = false;

	int iteration = 0;

	for (int i = 0; i < 5; ++i)	// fix this
	{
		std::map<ir::Block*, life_info> curr_blocks_info;

		if (enable_debug)
		{
			PRINT_TABS_NL(C_YELLOW, 1, "Iteration %i", iteration++);
		}

		walk_control_flow_by_block(WalkType::POST_ORDER, entry, [&](ir::Block* b)
		{
			auto& block_info = curr_blocks_info[b];

			const auto& block_uses = ctx.uses[b],
						block_defs = ctx.defs[b];

			auto& block_out = block_info.out,
				& block_in = block_info.in;

			// OUT calculation

			if (auto cf_item = b->get_control_flow_item())
			{
				if (auto branch = rtti::safe_cast<ir::Branch>(cf_item))
				{
					const auto& target_in = curr_blocks_info[branch->target].in;

					std::set<ir::Value*> out;

					std::set_union(target_in.begin(), target_in.end(), block_out.begin(), block_out.end(), std::inserter(out, out.begin()));

					block_out = out;
				}
				else if (auto bcond = rtti::safe_cast<ir::BranchCond>(cf_item))
				{
					const auto& target_if_true_in = curr_blocks_info[bcond->target_if_true].in,
							  & target_if_false_in = curr_blocks_info[bcond->target_if_false].in;

					std::set<ir::Value*> temp_out, out;

					std::set_union(block_out.begin(), block_out.end(), target_if_false_in.begin(), target_if_false_in.end(), std::inserter(temp_out, temp_out.begin()));
					std::set_union(temp_out.begin(), temp_out.end(), target_if_true_in.begin(), target_if_true_in.end(), std::inserter(out, out.begin()));

					block_out = out;
				}
			}

			// IN calculation

			std::set<ir::Value*> out_minus_defs;

			std::set_difference(block_out.begin(), block_out.end(), block_defs.begin(), block_defs.end(), std::inserter(out_minus_defs, out_minus_defs.begin()));
			std::set_union(block_uses.begin(), block_uses.end(), out_minus_defs.begin(), out_minus_defs.end(), std::inserter(block_in, block_in.begin()));

			return true;
		});

		ctx.in_out = curr_blocks_info;
	};

	if (enable_debug)
	{
		for (auto& [b, info] : ctx.in_out)
		{
			PRINT_TABS_NL(C_CYAN, 1, "'%s' has:", b->name.c_str());

			if (!info.in.empty())
			{
				PRINT_TABS(C_YELLOW, 2, "IN  = { ");

				dbg::print_set<ir::Value*>(C_GREEN, info.in, ", ", [](auto p) { return p->name; });

				PRINT(C_YELLOW, " }");
			}

			if (!info.out.empty())
			{
				PRINT_TABS(C_YELLOW, 2, "OUT = { ");

				dbg::print_set<ir::Value*>(C_GREEN, info.out, ", ", [](auto p) { return p->name; });

				PRINT(C_YELLOW, " }");
			}
		}

		PRINT_NL;
	}

	return true;
}

bool ssa_gen::build_dominance_tree(ir::Prototype* prototype, ir::Block* entry)
{
	PROFILE("Dominance Tree Construction");

	auto tree = prototype->dominance_tree = new dom_tree(prototype, entry);
	if (!tree)
		return false;

	tree->build();
	tree->print();

	return true;
}

bool ssa_gen::build_dominance_frontier(ir::Prototype* prototype, ir::Block* entry)
{
	PROFILE("Dominance Frontier Construction");

	auto& local = ctx.df_local,
		& rec = ctx.df_rec;

	// LOCAL calculation

	for (auto b : prototype->blocks)
	{
		b->for_each_successor([&](ir::Block* succ)
		{
			if (succ->idom != b)
				local[b].insert(succ);
		});
	}

	// RECURSIVE calculation

	walk_dom_tree_by_block(WalkType::POST_ORDER, entry, [&](ir::Block* x)
	{
		for (auto z : x->doms)
			for (auto y : local[z])
				if (z->idom == x && y->idom != x)
					rec[x].insert(y);
	});

	for (auto b : prototype->blocks)
	{
		const auto& l = local[b],
				  & r = rec[b];

		auto& bdf = ctx.df[b];

		if (l.empty())
		{
			bdf.insert(r.begin(), r.end());
			continue;
		}

		if (r.empty())
		{
			bdf.insert(l.begin(), l.end());
			continue;
		}

		std::set_union(r.begin(), r.end(), l.begin(), l.end(), std::inserter(bdf, bdf.begin()));
	}

	for (auto& [v, blocks] : ctx.defs_by_blocks)
	{
		auto& idf = ctx.iterative_df[v],
			  last_idf = idf;

		idf.insert(entry);

		for (auto vb : blocks)
			idf.insert(vb);

		std::set<ir::Block*> result;

		for (int i = 0; i < 4; ++i)		// fix this
		{
			for (auto b : idf)
			{
				std::set<ir::Block*> new_set;

				const auto& block_df = ctx.df[b];

				std::set_union(block_df.begin(), block_df.end(), result.begin(), result.end(), std::inserter(new_set, new_set.begin()));

				result = new_set;
			}

			idf.insert(result.begin(), result.end());
		}

		ctx.phi_blocks.insert({ v, result });
	}

	if (enable_debug)
	{
		PRINT_TABS_NL(C_RED, 1, "DF Local:");

		for (auto [b, df] : local)
		{
			PRINT_TABS(C_BLUE, 2, "DF(%s) = { ", b->name.c_str());
			dbg::print_set<ir::Value*>(C_GREEN, df, ", ", [](auto p) { return p->name; });
			PRINT(C_BLUE, " }");
		}

		PRINT_TABS_NL(C_RED, 1, "DF Recursive:");

		for (auto [b, df] : rec)
		{
			PRINT_TABS(C_BLUE, 2, "DF(%s) = { ", b->name.c_str());
			dbg::print_set<ir::Value*>(C_GREEN, df, ", ", [](auto p) { return p->name; });
			PRINT(C_BLUE, " }");
		}

		PRINT_TABS_NL(C_RED, 1, "DF Final:");

		for (auto [b, df] : ctx.df)
		{
			PRINT_TABS(C_BLUE, 2, "DF(%s) = { ", b->name.c_str());
			dbg::print_set<ir::Value*>(C_GREEN, df, ", ", [](auto p) { return p->name; });
			PRINT(C_BLUE, " }");
		}

		PRINT_TABS_NL(C_RED, 1, "Iterated DF:");

		for (auto [v, blocks] : ctx.phi_blocks)
		{
			PRINT_TABS(C_BLUE, 2, "DF(%s) = { ", v->name.c_str());
			dbg::print_set<ir::Value*>(C_GREEN, blocks, ", ", [](auto p) { return p->name; });
			PRINT(C_BLUE, " }");
		}

		PRINT_NL;
	}

	return true;
}

bool ssa_gen::insert_phis()
{
	PROFILE("PHI Insertion");

	for (auto& [v, blocks] : ctx.phi_blocks)
	{
		if (blocks.empty())
			continue;

		for (auto b : blocks)
		{
			if (!ctx.in_out[b].in.contains(v))
				continue;

			auto phi = new ir::Phi(v);

			for (auto predecessor : b->refs)
				phi->add_block(predecessor);

			b->add_phi(phi);
		}
	}

	if (enable_debug)
		PRINT_NL;

	return true;
}

bool ssa_gen::rename_values(ir::Block* entry)
{
	PROFILE("Values Versioning");

	auto& ver_info = ctx.versions_info;

	entry->for_each_dom([&](ir::Block* b)
	{
		for (auto i : b->items)
		{
			if (!rtti::safe_cast<ir::Phi>(i))
			{
				i->for_each_rvalue([&](ir::Value* v) -> ir::Value*
				{
					auto original_val = v->original ? v->original : v;

					const auto& value_to_renamed_val = ver_info[b];

					if (auto first_try_it = value_to_renamed_val.find(original_val); first_try_it != value_to_renamed_val.end())
						return first_try_it->second;
					else
					{
						auto curr_b = b;

						while (curr_b)
						{
							const auto& curr_value_to_renamed_val = ver_info[curr_b];

							if (auto second_try_it = curr_value_to_renamed_val.find(original_val); second_try_it != curr_value_to_renamed_val.end())
								return second_try_it->second;

							if (curr_b == entry && curr_b->idom == entry)
								break;

							curr_b = curr_b->idom;
						}
					}

					return nullptr;
				});
			}

			i->for_each_lvalue([&](ir::Value* v)
			{
				auto new_val = v->create_new();

				if (v->versions++ == 0 && rtti::safe_cast<ir::StackAlloc>(v->definer))
					v->definer->set_value(new_val);

				return (ver_info[b][v] = new_val);
			});
		}

		b->for_each_successor([&](ir::Block* succ)
		{
			for (auto phi : succ->phis)
			{
				auto phi_value = phi->value;
				auto original_val = phi_value->original ? phi_value->original : phi->value;

				for (auto phi_b : phi->blocks)
				{
					const auto& block_value_map = ver_info[phi_b];

					if (auto it = block_value_map.find(original_val); it != block_value_map.end())
						phi->add_value(it->second);
				}
			}
		});

		return true;
	});

	if (enable_debug)
		PRINT_NL;

	return true;
}

bool ssa_gen::clean_load_and_stores(ir::Block* entry)
{
	PROFILE("Load And Stores Cleaning");

	if (enable_debug)
		PRINT_NL;

	return true;
}

bool ssa_gen::calculate_lives(ir::Block* entry)
{
	PROFILE("Values Lives Calculation");

	auto& ssa_values = ctx.ssa_values;

	walk_control_flow_by_block(WalkType::PRE_ORDER, entry, [&](ir::Block* b)
	{
		const auto& out = ctx.in_out[b].out;

		std::unordered_map<ir::Value*, ir::Instruction*> values_last_usage;

		for (auto i : b->items)
		{
			i->for_each_lvalue([&](ir::Value* v)
			{
				if (auto original_val = v->original)
				{
					auto& v_life = v->life;

					v_life.first = i;
					v_life.add_block(b);

					std::unordered_set<ir::Block*> visited_blocks;

					if (out.contains(original_val))
					{
						auto dispatch_block = [&](ir::Block* sb)
						{
							if (visited_blocks.contains(sb))
								return false;

							const auto& in_out = ctx.in_out[sb];

							if (!in_out.in.contains(original_val))
								return false;

							v_life.add_block(sb);
							visited_blocks.insert(sb);

							for (auto phi : sb->phis)
								if (phi->value->original == original_val)
								{
									v_life.last = phi;
									return false;
								}

							if (!in_out.out.contains(original_val))
							{
								v_life.last = i;
								return false;
							}

							return true;
						};

						b->for_each_successor_deep(dispatch_block);
					}
					else v_life.last = i;

					ssa_values.insert(v);
				}

				return nullptr;
			});

			i->for_each_rvalue([&](ir::Value* v)
			{
				values_last_usage[v] = i;
				return nullptr;
			});
		}

		for (auto [v, i] : values_last_usage)
			v->life.last = i;
	});

	if (enable_debug)
	{
		for (auto v : ssa_values)
		{
			PRINT_NNL(C_YELLOW, "'%s' lives in = { ", v->name.c_str());

			if (v->life.blocks.size() == 1)
				dbg::print_set<ir::Block>(C_RED, v->life.blocks, ", ", [](auto p) { return p->name; });
			else dbg::print_set<ir::Block>(C_GREEN, v->life.blocks, ", ", [](auto p) { return p->name; });

			PRINT(C_YELLOW, " }");
			
			PRINT_TABS(C_BLUE, 1, "First instruction: ");
			
			v->life.first->print();

			PRINT_TABS(C_BLUE, 1, "Last instruction: ");
			
			if (v->life.last)
				v->life.last->print();
		}

		PRINT_NL;
	}

	return true;
}

void ssa_gen::print_ssa_ir()
{
	PRINT_NL;

	for (const auto& prototype : ir.get_ir_info().prototypes)
		ir.print_prototype(prototype);
}

void ssa_gen::display_cfg()
{
	PRINT(C_YELLOW, "Displaying Control Flow Graph...\n");

	graph::gv g("cfg.gv", "cfg");

	g.set_bg_color("white");
	g.set_font_name("Inconsolata");
	g.set_font_size("12");

	for (auto prototype : ir.get_ir_info().prototypes)
	{
		auto entry = prototype->get_entry_block();
		if (!entry)
			continue;

		g.set_base_name(prototype->name);

		auto subg = g.create_subgraph(prototype->name);

		walk_control_flow_by_block(WalkType::PRE_ORDER, entry, [&](ir::Block* b)
		{
			b->for_each_successor([&](ir::Block* succ)
			{
				auto node_b = g.get_node_by_name(b->name),
					 node_succ = g.get_node_by_name(succ->name);

				if (!node_b && (node_b = g.create_node(b->name, b->name, "ellipse", b->is_entry ? "cyan" : "gray", "filled")))
					subg->add_node(node_b);

				if (!node_succ && (node_succ = g.create_node(succ->name, succ->name, "ellipse", succ->is_entry ? "cyan" : "gray", "filled")))
					subg->add_node(node_succ);

				if (node_b && node_succ && b != succ)
					node_b->add_link(node_succ);
			});
		});
	}

	g.build();
	g.render("Control Flow Graph");
}

void ssa_gen::display_dominance_tree()
{
	PRINT(C_YELLOW, "Displaying Dominance Tree...\n");

	graph::gv g("dom_tree.gv", "dom_tree");

	g.set_bg_color("white");
	g.set_font_name("Inconsolata");
	g.set_font_size("12");

	for (auto prototype : ir.get_ir_info().prototypes)
	{
		g.set_base_name(prototype->name);

		auto subg = g.create_subgraph(prototype->name);

		for (const auto& [b, dom] : prototype->dominance_tree->get_ordered_doms())
		{
			auto node_b = g.get_node_by_name(b->name),
				 node_dom = g.get_node_by_name(dom->name);

			if (!node_b && (node_b = g.create_node(b->name, b->name, "ellipse", b->is_entry ? "cyan" : "gray", "filled")))
				subg->add_node(node_b);

			if (!node_dom && (node_dom = g.create_node(dom->name, dom->name, "ellipse", dom->is_entry ? "cyan" : "gray", "filled")))
				subg->add_node(node_dom);

			if (node_b && node_dom && b != dom)
				node_dom->add_link(node_b);
		}
	}

	g.build();
	g.render("Dominance Trees");
}

void ssa_gen::walk_control_flow(ir::Block* block, cf_callback fn)
{
	for (auto i : block->items)
		fn(block, i);

	auto cf_item = block->get_control_flow_item();

	if (auto branch = rtti::safe_cast<ir::Branch>(cf_item))
		walk_control_flow(branch->target, fn);
	else if (auto bcond = rtti::safe_cast<ir::BranchCond>(cf_item))
	{
		walk_control_flow(bcond->target_if_true, fn);
		walk_control_flow(bcond->target_if_false, fn);
	}
}

void ssa_gen::walk_control_flow_preorder_block(ir::Block* block, cf_block_callback fn)
{
	if (!wi.traversed_blocks.insert(block).second)
		return;

	fn(block);

	auto cf_item = block->get_control_flow_item();

	if (auto branch = rtti::safe_cast<ir::Branch>(cf_item))
		walk_control_flow_preorder_block(branch->target, fn);
	else if (auto bcond = rtti::safe_cast<ir::BranchCond>(cf_item))
	{
		walk_control_flow_preorder_block(bcond->target_if_true, fn);
		walk_control_flow_preorder_block(bcond->target_if_false, fn);
	}
}

void ssa_gen::walk_control_flow_postorder_block(ir::Block* block, cf_block_callback fn)
{
	if (!wi.traversed_blocks.insert(block).second)
		return;

	auto cf_item = block->get_control_flow_item();

	if (auto branch = rtti::safe_cast<ir::Branch>(cf_item))
		walk_control_flow_postorder_block(branch->target, fn);
	else if (auto bcond = rtti::safe_cast<ir::BranchCond>(cf_item))
	{
		walk_control_flow_postorder_block(bcond->target_if_false, fn);
		walk_control_flow_postorder_block(bcond->target_if_true, fn);
	}

	fn(block);
}

void ssa_gen::walk_dom_tree_preorder_block(ir::Block* block, cf_block_callback fn)
{
	if (!wi.traversed_blocks.insert(block).second)
		return;

	fn(block);

	for (auto b : block->doms)
		walk_dom_tree_preorder_block(b, fn);
}

void ssa_gen::walk_dom_tree_postorder_block(ir::Block* block, cf_block_callback fn)
{
	if (!wi.traversed_blocks.insert(block).second)
		return;

	for (auto b : block->doms)
		walk_dom_tree_postorder_block(b, fn);

	fn(block);
}