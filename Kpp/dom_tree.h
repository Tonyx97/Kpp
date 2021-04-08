#pragma once

namespace kpp
{
	namespace ir
	{
		struct Prototype;
		struct Block;
	}

	using dom_pair = std::pair<ir::Block*, ir::Block*>;

	class dom_tree
	{
	private:

		std::unordered_map<ir::Block*, ir::Block*> doms;

		std::vector<dom_pair> ordered_doms;

		std::unordered_set<ir::Block*> checked_blocks;
		
		std::vector<ir::Block*> reversed_postorder;

		ir::Prototype* prototype = nullptr;

		ir::Block* entry = nullptr;

		void create_reversed_postorder_list_internal(ir::Block* curr);
		void create_reversed_postorder_list();

	public:

		dom_tree(ir::Prototype* prototype, ir::Block* entry) : prototype(prototype), entry(entry) {}

		void build();
		void set_dom(ir::Block* b, ir::Block* dom);
		void print();

		ir::Block* intersect(ir::Block* b1, ir::Block* b2);
		ir::Block* get_dom(ir::Block* b);
		ir::Block* get_entry() const	{ return entry; }

		const auto& get_doms()			{ return doms; }
		const auto& get_ordered_doms()	{ return ordered_doms; }
	};
}