#pragma once

namespace kpp
{
	namespace ir
	{
		struct Prototype;
		struct Block;
	}

	class dom_tree
	{
	private:

		std::map<ir::Block*, ir::Block*> doms;

		std::vector<ir::Block*> reversed_postorder;

		ir::Prototype* prototype = nullptr;

		ir::Block* entry = nullptr;

		void create_reversed_postorder_list_internal(ir::Block* curr);
		void create_reversed_postorder_list();

	public:

		dom_tree(ir::Prototype* prototype, ir::Block* entry) : prototype(prototype), entry(entry) {}

		void build();
		void set_dom(ir::Block* v, ir::Block* dom);
		void print();

		ir::Block* intersect(ir::Block* b1, ir::Block* b2);
		ir::Block* get_dom(ir::Block* v);
		ir::Block* get_entry() const	{ return entry; }

		const auto& get_doms()			{ return doms; }
	};
}