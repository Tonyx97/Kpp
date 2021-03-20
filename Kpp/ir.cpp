#include <defs.h>

#include "ir.h"

#include "gv.h"

using namespace kpp;

namespace kpp::ir
{
	void Call::print()
	{
		PRINT_TABS(C_PURPLE, 1, "call " + prototype->name + " ");

		dbg::print_vec<ir::Instruction>(C_WHITE, params, ", ", [](auto param) { return param->get_value_str(); });

		PRINT_NL;
	}

	void StackAlloc::print()
	{
		PRINT_TABS_NL(C_YELLOW, 1, value->name + " = stackalloc " + STRINGIFY_TYPE(ty));
	}

	void Store::print()
	{
		PRINT_TABS_NL(C_RED, 1, "store " + STRINGIFY_TYPE(ty) + "* " + value->name + ", " + STRINGIFY_TYPE(operand->get_type()) + " " + operand->get_value_str());
	}

	void Load::print()
	{
		PRINT_TABS_NL(C_GREEN, 1, value->name + " = load " + STRINGIFY_TYPE(ty) + ", " + STRINGIFY_TYPE(ty) + "* " + vid->get_value_str());
	}

	void ValueInt::print()
	{
		//PRINT_TABS_NL(C_WHITE, 1, name + " = " + std::to_string(value.u64));
	}

	void BinaryOp::print()
	{
		PRINT_TABS_NL(C_CYAN, 1, value->name + " = " + STRINGIFY_BINARY_OP(op) + " " + left->get_value_str() + ", " + right->get_value_str());
	}

	void UnaryOp::print()
	{
		PRINT_TABS_NL(C_CYAN, 1, value->name + " = " + STRINGIFY_UNARY_OP(op) + " " + operand->get_value_str());
	}

	void Block::print()
	{
		if (!refs.empty())
		{
			PRINT_TABS(C_WHITE, 0, name + ": [refs: ");

			dbg::print_vec<ir::Block>(C_WHITE, refs, ", ", [](auto block) { return block->name; });

			PRINT(C_WHITE, "]");
		}
		else PRINT_TABS(C_WHITE, 0, name + ": [no refs]");
	}

	void BranchCond::print()
	{
		if (!target_if_true || !target_if_false)
		{
			PRINT_TABS_NL(C_BLUE, 1, "bcond " + STRINGIFY_TYPE(get_type()) + " " + comparison->get_value_str());
		}
		else
		{
			PRINT_TABS_NL(C_BLUE, 1, "bcond " + STRINGIFY_TYPE(get_type()) + " " + comparison->get_value_str() + ", " + target_if_true->name + ", " + target_if_false->name);
		}
	}

	void Branch::print()
	{
		PRINT_TABS_NL(C_BLUE, 1, "branch " + target->name);
	}

	void Return::print()
	{
		PRINT_TABS_NL(C_BLUE, 1, "ret " + (value ? value->name : "void"));
	}
}

ir_gen::ir_gen(ast::AST* tree) : tree(tree)
{
}

ir_gen::~ir_gen()
{
}

void ir_gen::print_ir()
{
	for (auto&& prototype : iri.prototypes)
		print_prototype(prototype);
}

void ir_gen::print_prototype(ir::Prototype* prototype)
{
	PRINT_NNL(C_WHITE, "%s %s(", STRINGIFY_TYPE(prototype->ret_ty).c_str(), prototype->name.c_str());

	dbg::print_vec<ir::PrototypeParam>(C_WHITE, prototype->params, ", ", [](auto stmt)
	{
		return STRINGIFY_TYPE(stmt->type) + " " + stmt->name;
	});

	if (!prototype->is_empty())
	{
		PRINT(C_WHITE, ")");

		PRINT_TABS_NL(C_WHITE, print_level, "{");

		print_level = 1;

		for (auto&& block : prototype->blocks)
			print_block(block);

		print_level = 0;

		PRINT_TABS_NL(C_WHITE, print_level, "}");
	}
	else PRINT(C_WHITE, ") {}");

	PRINT_NL;
}

void ir_gen::print_block(ir::Block* block)
{
	block->print();

	for (auto&& item : block->items)
		print_item(item);
}

void ir_gen::print_item(ir::Instruction* item)
{
	item->print();
}

void ir_gen::build_dominance_trees()
{
	std::vector<dom_tree*> trees;

	for (const auto& prototype : iri.prototypes)
	{
		auto entry = prototype->get_entry_block();
		if (!entry)
			return;

		auto tree = prototype->dominance_tree = new dom_tree(prototype, entry);
		if (!tree)
			return;

		tree->build();
	}
}

void ir_gen::display_dominance_tree()
{
	graph::gv g("dom_tree.gv", "dom_tree");

	g.set_bg_color("white");
	g.set_font_name("Inconsolata");
	g.set_font_size("12");

	for (const auto& prototype : iri.prototypes)
	{
		g.set_base_name(prototype->name);

		auto subg = g.create_subgraph(prototype->name);

		for (const auto& [v, dom] : prototype->dominance_tree->get_ordered_doms())
		{
			auto node_v = g.get_node_by_name(v->name),
				 node_dom = g.get_node_by_name(dom->name);

			if (!node_v)
				if (node_v = g.create_node(v->name, v->name, "ellipse", v->is_entry ? "cyan" : "gray", "filled"))
					subg->add_node(node_v);

			if (!node_dom)
				if (node_dom = g.create_node(dom->name, dom->name, "ellipse", dom->is_entry ? "cyan" : "gray", "filled"))
					subg->add_node(node_dom);

			if (node_v && node_dom && v != dom)
				node_dom->add_link(node_v);
		}
	}

	g.build();
	g.render("Dominance Trees");
}

void ir_gen::add_prototype(ir::Prototype* prototype)
{
	gi.prototypes.insert({ prototype->name, prototype });
	iri.prototypes.push_back(prototype);
}

bool ir_gen::generate()
{
	if (!tree)
		return false;

	for (auto&& prototype : tree->prototypes)
		generate_prototype(prototype);

	return true;
}

ir::Prototype* ir_gen::generate_prototype(ast::Prototype* prototype)
{
	if (auto defined_prototype = get_defined_prototype(prototype))
		return defined_prototype;

	if (prototype->is_declaration())
	{
		if (auto prototype_def = get_prototype_definition(prototype))
			return generate_prototype(prototype_def);
		else return nullptr;
	}

	auto ir_prototype = pi.curr_prototype = new ir::Prototype();

	ir_prototype->name = prototype->name;
	ir_prototype->ret_ty = prototype->ret_ty;

	for (auto&& param : prototype->params)
	{
		auto decl_or_assign = static_cast<ast::ExprDeclOrAssign*>(param);

		ir_prototype->add_param(new ir::PrototypeParam(decl_or_assign->name, decl_or_assign->ty));
	}

	if (prototype->body)
	{
		auto entry_block = pi.create_block(true);

		entry_block->add_ref(entry_block);

		ir_prototype->body = generate_from_body(prototype->body);

		pi.create_return(ir_prototype->ret_ty);
	}

	add_prototype(ir_prototype);

	pi.clear_out_unused_blocks();
	pi.copy_to_prototype(ir_prototype);
	pi.clear();

	return ir_prototype;
}

ir::Body* ir_gen::generate_from_body(ast::StmtBody* body)
{
	auto ir_body = new ir::Body();

	for (auto&& stmt : body->stmts)
	{
		if (auto body = rtti::safe_cast<ast::StmtBody>(stmt))		generate_from_body(body);
		else if (auto expr = rtti::safe_cast<ast::Expr>(stmt))		generate_from_expr(expr);
		else if (auto stmt_if = rtti::safe_cast<ast::StmtIf>(stmt)) generate_from_if(stmt_if);
	}

	return ir_body;
}

ir::Instruction* ir_gen::generate_from_expr(ast::Expr* expr)
{
	if (auto int_literal = rtti::safe_cast<ast::ExprIntLiteral>(expr))			 return generate_from_expr_int_literal(int_literal);
	else if (auto id = rtti::safe_cast<ast::ExprId>(expr))						 return generate_from_expr_id(id);
	else if (auto decl_or_assign = rtti::safe_cast<ast::ExprDeclOrAssign>(expr)) return generate_from_expr_decl_or_assign(decl_or_assign);
	else if (auto binary_op = rtti::safe_cast<ast::ExprBinaryOp>(expr))			 return generate_from_expr_binary_op(binary_op);
	else if (auto unary_op = rtti::safe_cast<ast::ExprUnaryOp>(expr))			 return generate_from_expr_unary_op(unary_op);
	else if (auto call = rtti::safe_cast<ast::ExprCall>(expr))					 return generate_from_expr_call(call);

	return nullptr;
}

ir::Instruction* ir_gen::generate_from_expr_decl_or_assign(ast::ExprDeclOrAssign* expr)
{
	if (expr->is_declaration())
	{
		auto stack_alloc = new ir::StackAlloc();

		stack_alloc->value = pi.add_value(expr->name, stack_alloc);
		stack_alloc->ty = expr->ty;

		pi.add_item(stack_alloc);

		if (expr->value)
		{
			auto store = new ir::Store();

			store->value = stack_alloc->value;
			store->ty = stack_alloc->ty;
			store->operand = generate_from_expr(expr->value);

			pi.add_item(store);
		}

		return stack_alloc;
	}
	else
	{
		auto value_to_load = pi.get_value_from_real_name(expr->name);
		if (!value_to_load)
			return nullptr;

		auto store = new ir::Store();

		store->value = value_to_load->get_value();
		store->ty = value_to_load->get_type();
		store->operand = generate_from_expr(expr->value);

		pi.add_item(store);

		return store;
	}
}

ir::ValueInt* ir_gen::generate_from_expr_int_literal(ast::ExprIntLiteral* expr)
{
	auto value_int = new ir::ValueInt();

	value_int->int_val = expr->value;
	value_int->ty = expr->ty;
	value_int->value = pi.add_value(expr->get_name(), value_int);

	pi.add_item(value_int);

	return value_int;
}

ir::BinaryOp* ir_gen::generate_from_expr_binary_op(ast::ExprBinaryOp* expr)
{
	auto binary_op = new ir::BinaryOp();

	binary_op->op = expr->op;
	binary_op->ty = expr->ty;
	binary_op->left = generate_from_expr(expr->left);
	binary_op->right = generate_from_expr(expr->right);
	binary_op->value = pi.add_value(expr->get_name(), binary_op);

	pi.add_item(binary_op);

	return binary_op;
}

ir::UnaryOp* ir_gen::generate_from_expr_unary_op(ast::ExprUnaryOp* expr)
{
	auto unary_op = new ir::UnaryOp();

	unary_op->op = expr->op;
	unary_op->operand = generate_from_expr(expr->value);
	unary_op->value = pi.add_value(expr->get_name(), unary_op);
	unary_op->ty = expr->value->get_ty();

	pi.add_item(unary_op);

	return unary_op;
}

ir::Load* ir_gen::generate_from_expr_id(ast::ExprId* expr)
{
	auto value_to_load = pi.get_value_from_real_name(expr->name);
	if (!value_to_load)
		return nullptr;
	
	auto load = new ir::Load();

	auto value_id = new ir::ValueId();

	value_id->value = value_to_load->get_value();

	load->vid = value_id;
	load->ty = expr->ty;
	load->value = pi.add_value(expr->name, load);

	pi.add_item(load);

	return load;
}

ir::Call* ir_gen::generate_from_expr_call(ast::ExprCall* expr)
{
	auto call = new ir::Call();

	call->prototype = get_defined_prototype(expr->prototype);

	for (auto&& param : expr->stmts)
	{
		call->params.push_back(generate_from_expr(param));
	}

	pi.add_item(call);

	return call;
}

bool ir_gen::generate_from_expr_binary_op_cond(ast::ExprBinaryOp* expr, ir::Block* target_if_true, ir::Block* target_if_false)
{
	const bool is_or = expr->op == TOKEN_LOGICAL_OR,
			   is_and = expr->op == TOKEN_LOGICAL_AND;

	if (!is_or && !is_and)
	{
		auto cond = new ir::BranchCond();

		cond->comparison = generate_from_expr_binary_op(expr);
		cond->target_if_true = target_if_true;
		cond->target_if_false = target_if_false;

		target_if_true->add_ref(pi.curr_block);
		target_if_false->add_ref(pi.curr_block);

		pi.add_item(cond);

		return true;
	}

	auto left_block = pi.create_block(),
		 right_block = pi.create_block();

	auto gen_block = [&](ir::Block* block, ast::ExprBinaryOp* bin_op, bool left)
	{
		pi.add_block(block);

		ir::Block* tit = nullptr,
				 * tif = nullptr;

		if (left)
		{
			tit = is_or ? target_if_true : right_block;
			tif = is_or ? right_block : target_if_false;
		}
		else
		{
			tit = target_if_true;
			tif = target_if_false;
		}

		return generate_from_expr_binary_op_cond(bin_op, tit, tif);
	};

	bool left_ok = gen_block(left_block, static_cast<ast::ExprBinaryOp*>(expr->left), true),
		 right_ok = gen_block(right_block, static_cast<ast::ExprBinaryOp*>(expr->right), false);

	return (left_ok || right_ok);
}

ir::Instruction* ir_gen::generate_from_if(ast::StmtIf* stmt_if)
{
	auto end_block = pi.create_block(),
		 if_block = stmt_if->if_body ? pi.create_block() : end_block,
		 else_block = stmt_if->else_body ? pi.create_block() : end_block;

	std::vector<ir::Block*> else_if_blocks;

	pi.if_context = { if_block, else_block };

	for (auto&& else_if : stmt_if->ifs)
		else_if_blocks.push_back(pi.create_block());

	auto curr_block = pi.curr_block;

	if (auto bin_op = rtti::safe_cast<ast::ExprBinaryOp>(stmt_if->expr))
	{
		generate_from_expr_binary_op_cond(bin_op, pi.if_context.if_block, else_if_blocks.empty() ? pi.if_context.else_block : else_if_blocks[0]);

		pi.create_branch_in_block_to_next(curr_block);
	}
	else if (auto id = rtti::safe_cast<ast::ExprId>(stmt_if->expr))
	{
		auto cmp = new ir::BinaryOp();

		cmp->op = TOKEN_NOT_EQUAL;
		cmp->ty = id->ty;
		cmp->left = generate_from_expr_id(id);
		cmp->right = new ir::ValueInt();
		cmp->value = pi.add_value(id->get_name(), cmp);

		pi.add_item(cmp);

		auto bcond = new ir::BranchCond();

		bcond->comparison = cmp;
		bcond->target_if_true = pi.if_context.if_block;
		bcond->target_if_false = pi.if_context.else_block;
		
		pi.add_item(bcond);
	}

	if (stmt_if->if_body)
	{
		pi.add_block(if_block);
		generate_from_body(stmt_if->if_body);
		pi.create_branch(nullptr, end_block);
	}

	if (auto stmt_ifs_and_blocks = util::stl::zip_next(stmt_if->ifs, else_if_blocks))
		for (const auto& [ast_if, _1, _2, cmp_block, next_block, end2] : stmt_ifs_and_blocks)
		{
			auto else_if_block = pi.create_block();

			pi.add_block(*cmp_block);

			if (auto bin_op = rtti::safe_cast<ast::ExprBinaryOp>((*ast_if)->expr))
				generate_from_expr_binary_op_cond(bin_op, else_if_block, next_block != end2 ? *next_block : else_block);

			pi.add_block(else_if_block);
			generate_from_body((*ast_if)->if_body);
			pi.create_branch(nullptr, end_block);
		}

	if (stmt_if->else_body)
	{
		pi.add_block(else_block);
		generate_from_body(stmt_if->else_body);
		pi.create_branch(nullptr, end_block);
	}

	pi.add_block(end_block);

	return nullptr;
}

ir::Prototype* ir_gen::get_defined_prototype(ast::Prototype* prototype)
{
	auto it = gi.prototypes.find(prototype->name);
	return (it != gi.prototypes.end() ? it->second : nullptr);
}

ast::Prototype* ir_gen::get_prototype_definition(ast::Prototype* prototype_decl)
{
	if (!prototype_decl)
		return nullptr;

	const auto& prototype_decl_name = prototype_decl->name;

	for (auto&& prototype : tree->prototypes)
		if (!prototype->name.compare(prototype_decl_name) && !prototype->is_declaration())
			return prototype;

	return nullptr;
}