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

		struct current_prototype_ctx
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

			std::map<ir::Block*, life_info> lives;

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
				lives.clear();
			}
		} ctx {};

		std::vector<ir::Prototype*> prototypes;

		ir_gen _ir;
		ir::IR& ir;

		void walk_control_flow_preorder_block(ir::Block* block, cf_block_callback fn);
		void walk_control_flow_postorder_block(ir::Block* block, cf_block_callback fn);
		void walk_dom_tree_preorder_block(ir::Block* block, cf_block_callback fn);
		void walk_dom_tree_postorder_block(ir::Block* block, cf_block_callback fn);

	public:

		ssa_gen(ir_gen& _ir) : _ir(_ir), ir(_ir.get_ir_info()) { prototypes = ir.prototypes; }

		bool build_ssa();
		bool build_def_use(ir::Block* entry);
		bool build_in_out(ir::Block* entry);
		bool build_dominance_tree(ir::Prototype* prototype, ir::Block* entry);
		bool build_dominance_frontier(ir::Prototype* prototype, ir::Block* entry);
		bool insert_phis();
		bool rename_values(ir::Block* entry);

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

		const decltype(prototypes)& get_prototypes() { return prototypes; }
	};
}