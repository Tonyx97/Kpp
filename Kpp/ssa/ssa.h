#pragma once

namespace kpp
{
	using cf_callback = const std::function<void(ir::Block*, ir::Instruction*)>&;
	using cf_block_callback = const std::function<void(ir::Block*)>&;
	using block_live_range = std::pair<int, int>;

	enum class WalkType : unsigned char
	{
		PRE_ORDER,
		POST_ORDER,
	};

	struct life_info
	{
		std::set<ir::Value*> in, out;
	};

	struct ssa_ctx
	{
		std::unordered_map<ir::Block*, std::unordered_map<ir::Value*, ir::Value*>> versions_info;

		std::unordered_map<ir::Block*, std::set<ir::Value*>> defs,
															 uses;

		std::unordered_map<ir::Value*, std::set<ir::Block*>> defs_by_blocks,
															 uses_by_blocks,
															 iterative_df,
															 phi_blocks;

		std::unordered_map<ir::Block*, std::set<ir::Block*>> df_local,
															 df_rec,
															 df;

		std::map<ir::Block*, life_info> in_out;

		std::unordered_set<ir::Value*> ssa_values;
		
		ir::Prototype* prototype = nullptr;

		void clear()
		{
			versions_info.clear();
			defs.clear();
			uses.clear();
			defs_by_blocks.clear();
			uses_by_blocks.clear();
			iterative_df.clear();
			phi_blocks.clear();
			df_local.clear();
			df_rec.clear();
			df.clear();
			in_out.clear();

			prototype = nullptr;
		}
	};

	class ssa_gen
	{
	private:

		struct walk_info
		{
			std::unordered_set<ir::Block*> traversed_blocks;

			void clear()
			{
				traversed_blocks.clear();
			}
		} wi {};

		ssa_ctx ctx {};

		ir_gen& ir;

		bool enable_debug = false;

		void walk_control_flow_preorder_block(ir::Block* block, cf_block_callback fn);
		void walk_control_flow_postorder_block(ir::Block* block, cf_block_callback fn);
		void walk_dom_tree_preorder_block(ir::Block* block, cf_block_callback fn);
		void walk_dom_tree_postorder_block(ir::Block* block, cf_block_callback fn);

	public:

		ssa_gen(ir_gen& ir) : ir(ir) {}

		bool build_ssa();
		bool build_def_use(ir::Block* entry);
		bool build_in_out(ir::Block* entry);
		bool build_dominance_tree(ir::Prototype* prototype, ir::Block* entry);
		bool build_dominance_frontier(ir::Prototype* prototype, ir::Block* entry);
		bool insert_phis();
		bool rename_values(ir::Block* entry);
		bool clean_load_and_stores(ir::Block* entry);
		bool calculate_lives(ir::Block* entry);

		void print_ssa_ir();
		void display_cfg();
		void display_dominance_tree();

		void walk_control_flow(ir::Block* block, cf_callback fn);

		void walk_control_flow_by_block(WalkType type, ir::Block* block, cf_block_callback fn)
		{
			wi.clear();

			switch (type)
			{
			case WalkType::PRE_ORDER:  return walk_control_flow_preorder_block(block, fn);
			case WalkType::POST_ORDER: return walk_control_flow_postorder_block(block, fn);
			}
		}

		void walk_dom_tree_by_block(WalkType type, ir::Block* block, cf_block_callback fn)
		{
			wi.clear();

			switch (type)
			{
			case WalkType::PRE_ORDER:  return walk_dom_tree_preorder_block(block, fn);
			case WalkType::POST_ORDER: return walk_dom_tree_postorder_block(block, fn);
			}
		}
	};
}