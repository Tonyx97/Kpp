#include <defs.h>

#include <graph_viz/gv.h>
#include <asm/x64/registers.h>

#include "ir.h"

using namespace kpp;

namespace kpp::ir
{
	bool dom_set_order::operator() (Block* x, Block* y) const
	{
		return (x->reverse_postorder_index > y->reverse_postorder_index);
	}

	Value* Value::create_new()
	{
		auto new_val = new Value();

		copy_to(new_val);

		new_val->name = name + "_" + std::to_string(versions);
		new_val->original = this;

		vers.insert(new_val);

		return new_val;
	}

	void Value::copy_to(Value* v)
	{
		v->block_owner = block_owner;
		v->definer = definer;
		v->storage_type = storage_type;
		v->storage = storage;
		v->ret = ret;
	}

	void Alias::print()
	{
		PRINT_TABS(C_DARK_RED, 1, "(unused) ");
		PRINT_NNL(C_YELLOW, op1->name);
		PRINT_NNL(C_WHITE, " = ");
		PRINT(C_YELLOW, op2->name);
	}

	void Call::print()
	{
		PRINT_TABS(C_YELLOW, 1, op->name);
		PRINT_NNL(C_WHITE, " = ");
		PRINT_NNL(C_PURPLE, "call " + (prototype ? prototype->name : name) + " ");

		dbg::print_vec<ir::Instruction>(C_WHITE, params, ", ", [](auto param) { return param->get_value_str(); });

		PRINT_NL;
	}

	void StackAlloc::print()
	{
		PRINT_TABS(C_YELLOW, 1, op->name);
		PRINT_NNL(C_WHITE, " = ");
		PRINT_NNL(C_GREEN, "stackalloc ");
		PRINT(C_BLUE, STRINGIFY_TYPE(ty));
	}

	void Store::print()
	{
		PRINT_TABS(C_DARK_RED, 1, "(unused) ");
		PRINT_NNL(C_GREEN, "store ");
		PRINT_NNL(C_BLUE, STRINGIFY_TYPE(ty) + "* ");
		PRINT_NNL(C_YELLOW, op1->name);
		PRINT_NNL(C_WHITE, ", ");
		PRINT_NNL(C_BLUE, STRINGIFY_TYPE(op2_i->get_type()) + " ");
		PRINT(C_YELLOW, op2_i->get_value_str());
	}

	void Load::print()
	{
		PRINT_TABS(C_DARK_RED, 1, "(unused) ");
		PRINT_NNL(C_YELLOW, op1->name);
		PRINT_NNL(C_WHITE, " = ");
		PRINT_NNL(C_GREEN, "load ");
		PRINT_NNL(C_BLUE, STRINGIFY_TYPE(ty));
		PRINT_NNL(C_WHITE, ", ");
		PRINT_NNL(C_BLUE, STRINGIFY_TYPE(ty) + "* ");
		PRINT(C_YELLOW, op2_i->get_value_str());
	}

	void ValueInt::print()
	{
		PRINT_TABS(C_YELLOW, 1, op1->name);
		PRINT_NNL(C_WHITE, " = ");
		PRINT(C_CYAN, std::to_string(op2->storage.integer.i64));
	}

	void BinaryOp::print()
	{
		PRINT_TABS(C_YELLOW, 1, op1->name);
		PRINT_NNL(C_WHITE, " = ");
		PRINT_NNL(C_GREEN, STRINGIFY_BINARY_OP(operation) + " ");
		PRINT_NNL(C_YELLOW, op2_i->get_value_str());
		PRINT_NNL(C_WHITE, ", ");
		PRINT(C_YELLOW, op3_i->get_value_str());
	}

	void UnaryOp::print()
	{
		PRINT_TABS(C_YELLOW, 1, op1->name);
		PRINT_NNL(C_WHITE, " = ");
		PRINT_NNL(C_GREEN, STRINGIFY_UNARY_OP(operation) + " ");
		PRINT(C_YELLOW, op2_i->get_value_str());
	}

	void Block::print()
	{
		if (!is_entry)
			PRINT_NL;

		if (!refs.empty())
		{
			PRINT_TABS(C_WHITE, 0, name + ": [refs: ");

			dbg::print_vec<ir::Block>(C_WHITE, refs, ", ", [](auto block) { return block->name; });

			PRINT(C_WHITE, "]");
		}
		else PRINT_TABS(C_WHITE, 0, name + ": [no refs]");
	}

	void Block::for_each_successor(BlockFn fn)
	{
		if (auto cfg_item = get_jump_item())
		{
			if (auto branch = rtti::safe_cast<Branch>(cfg_item))
				fn(branch->target);
			else if (auto bcond = rtti::safe_cast<BranchCond>(cfg_item))
			{
				fn(bcond->target_if_true);
				fn(bcond->target_if_false);
			}
		}
	}

	void Block::for_each_successor_deep(BlockRetFn fn)
	{
		if (auto cfg_item = get_jump_item())
		{
			if (auto branch = rtti::safe_cast<Branch>(cfg_item))
			{
				if (fn(branch->target))
					branch->target->for_each_successor_deep(fn);
			}
			else if (auto bcond = rtti::safe_cast<BranchCond>(cfg_item))
			{
				if (fn(bcond->target_if_true))
				{
					bcond->target_if_true->for_each_successor_deep(fn);

					if (fn(bcond->target_if_false))
						bcond->target_if_false->for_each_successor_deep(fn);
				}
			}
		}
	}

	void Block::for_each_dom(BlockRetFn fn)
	{
		if (!fn(this))
			return;

		for (auto dom : doms)
			if (dom != this)
				dom->for_each_dom(fn);
	}

	Block* Block::get_asm_target(bool& reversed)
	{
		auto jmp_item = get_jump_item();

		if (auto branch = rtti::safe_cast<Branch>(jmp_item))
			return (branch->unused ? nullptr : branch->target);
		else if (auto bcond = rtti::safe_cast<BranchCond>(jmp_item))
		{
			if (next == bcond->target_if_false)
			{
				reversed = true;
				return bcond->target_if_true;
			}

			return bcond->target_if_false;
		}

		return nullptr;
	}

	void BranchCond::print()
	{
		PRINT_TABS(C_GREEN, 1, "bcond ");

		if (!target_if_true || !target_if_false)
		{
			PRINT_NNL(C_BLUE, STRINGIFY_TYPE(get_type()) + " ");
			PRINT(C_YELLOW, comparison->get_value_str());
		}
		else
		{
			PRINT_NNL(C_BLUE, STRINGIFY_TYPE(get_type()) + " ");
			PRINT_NNL(C_YELLOW, comparison->get_value_str());
			PRINT_NNL(C_WHITE, ", ");
			PRINT_NNL(C_WHITE, target_if_true->name);
			PRINT_NNL(C_WHITE, ", ");
			PRINT(C_WHITE, target_if_false->name);
		}
	}

	void Branch::print()
	{
		PRINT_TABS(C_GREEN, 1, "branch ");
		PRINT(C_WHITE, target->name);
	}

	void Return::print()
	{
		PRINT_TABS(C_GREEN, 1, "ret ");
		if (op_i)
			PRINT(C_YELLOW, op_i->get_value_str());
		else PRINT(C_BLUE, "void");
	}

	void Phi::print()
	{
		PRINT_TABS(C_YELLOW, 1, op->name);
		PRINT_NNL(C_WHITE, " = ");
		PRINT_NNL(C_GREEN, "phi");
		PRINT_NNL(C_WHITE, "(");

		if (values.empty())
		{
			auto original_val = op->original ? op->original : op;

			dbg::print_set<ir::Block>(C_YELLOW, blocks, ", ", [&](auto p)
			{
				return original_val->name + "." + p->name;
			});
		}
		else
		{
			dbg::print_set<ir::Value>(C_YELLOW, values, ", ", [&](auto p)
			{
				return p->name;
			});
		}

		PRINT(C_WHITE, ")");
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
	for (auto prototype : iri.prototypes)
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

void ir_gen::add_prototype(ir::Prototype* prototype)
{
	gi.prototypes.insert({ prototype->name, prototype });
	iri.prototypes.push_back(prototype);
}

bool ir_gen::generate()
{
	if (!tree)
		return false;

	for (auto prototype : tree->prototypes)
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

	for (auto param : prototype->params)
	{
		auto decl_or_assign = static_cast<ast::ExprDeclOrAssign*>(param);

		ir_prototype->add_param(new ir::PrototypeParam(decl_or_assign->name, decl_or_assign->ty));
	}

	if (prototype->body)
	{
		auto entry_block = pi.create_block(true);

		entry_block->add_ref(entry_block);

		ir_prototype->body = generate_from_body(prototype->body);

		if (!ir_prototype->has_return)
		{
			pi.add_item(new ir::Return());

			ir_prototype->has_return = true;
		}
	}

	add_prototype(ir_prototype);

	pi.clear_out_unused_blocks();
	pi.copy_to_prototype(ir_prototype);
	pi.clear();

	if (auto last_block = ir_prototype->get_exit_block())
		last_block->is_last = true;

	return ir_prototype;
}

ir::Body* ir_gen::generate_from_body(ast::StmtBody* body)
{
	auto ir_body = new ir::Body();

	for (auto stmt : body->stmts)
	{
		if (auto body = rtti::safe_cast<ast::StmtBody>(stmt))					generate_from_body(body);
		else if (auto expr = rtti::safe_cast<ast::Expr>(stmt))					generate_from_expr(expr);
		else if (auto stmt_if = rtti::safe_cast<ast::StmtIf>(stmt))				generate_from_if(stmt_if);
		else if (auto stmt_return = rtti::safe_cast<ast::StmtReturn>(stmt))		generate_from_return(stmt_return);
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

		stack_alloc->op = pi.add_value(expr->name, stack_alloc);
		stack_alloc->ty = expr->ty;

		pi.add_item(stack_alloc);

		if (expr->value)
		{
			auto store = new ir::Store();

			store->op1 = stack_alloc->op;
			store->ty = stack_alloc->ty;
			store->op2_i = generate_from_expr(expr->value);

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

		store->op1 = value_to_load;
		store->ty = TOKEN_I32;		// fix me
		store->op2_i = generate_from_expr(expr->value);

		pi.add_item(store);

		return store;
	}
}

ir::ValueInt* ir_gen::generate_from_expr_int_literal(ast::ExprIntLiteral* expr)
{
	auto value_int = new ir::ValueInt();

	value_int->ty = expr->ty;
	value_int->op1 = pi.add_value(expr->get_name(), value_int);
	value_int->op1->storage.integer = expr->value;
	value_int->op1->storage_type = ir::STORAGE_INTEGER;
	value_int->op2 = new ir::Value();
	value_int->op2->storage.integer = expr->value;
	value_int->op2->storage_type = ir::STORAGE_INTEGER;

	pi.add_item(value_int);

	return value_int;
}

ir::BinaryOp* ir_gen::generate_from_expr_binary_op(ast::ExprBinaryOp* expr)
{
	auto binary_op = new ir::BinaryOp();

	binary_op->operation = expr->op;
	binary_op->ty = expr->ty;
	binary_op->op2_i = generate_from_expr(expr->left);
	binary_op->op3_i = generate_from_expr(expr->right);
	binary_op->op1 = pi.add_value(expr->get_name(), binary_op);

	pi.add_item(binary_op);

	if (auto value_id = rtti::safe_cast<ast::ExprId>(expr->left); expr->assign)
	{
		auto value_to_store = pi.get_value_from_real_name(value_id->name);
		if (!value_to_store)
		{
			delete binary_op;
			return nullptr;
		}

		auto store = new ir::Store();

		store->op1 = value_to_store;
		store->ty = expr->ty;
		store->op2_i = binary_op;

		pi.add_item(store);
	}

	return binary_op;
}

ir::UnaryOp* ir_gen::generate_from_expr_unary_op(ast::ExprUnaryOp* expr)
{
	auto unary_op = new ir::UnaryOp();

	unary_op->operation = expr->op;
	unary_op->op2_i = generate_from_expr(expr->value);
	unary_op->op1 = pi.add_value(expr->get_name(), unary_op);
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

	value_id->op = value_to_load;

	load->op2_i = value_id;
	load->ty = expr->ty;
	load->op1 = pi.add_value(expr->name, load);

	pi.add_item(load);

	return load;
}

ir::Call* ir_gen::generate_from_expr_call(ast::ExprCall* expr)
{
	auto call = new ir::Call();

	const bool built_in = expr->built_in;
	
	if (call->prototype = built_in ? nullptr : get_defined_prototype(expr->prototype))
		call->prototype->callee_count++;

	call->name = expr->name;
	call->ty = expr->get_ty();
	call->op = pi.add_value({}, call);
	call->op->storage.default_r = x64::get_reg(RAX);
	call->op->storage.r = call->op->storage.default_r;
	call->built_in = built_in;

	pi.curr_prototype->caller_count++;

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

		cmp->operation = TOKEN_NOT_EQUAL;
		cmp->ty = id->ty;
		cmp->op2_i = generate_from_expr_id(id);
		cmp->op3_i = new ir::ValueInt();
		cmp->op1 = pi.add_value(id->get_name(), cmp);

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
		for (auto& [ast_if, _1, _2, cmp_block, next_block, end2] : stmt_ifs_and_blocks)
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

ir::Instruction* ir_gen::generate_from_return(ast::StmtReturn* stmt_return)
{
	auto ret = new ir::Return();

	if (auto expr = stmt_return->expr)
	{
		ret->ty = expr->get_ty();

		if (auto op_i = ret->op_i = generate_from_expr(expr))
			op_i->get_value()->ret = ret;
	}

	pi.curr_prototype->has_return = true;

	pi.add_item(ret);

	return ret;
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

	for (auto prototype : tree->prototypes)
		if (!prototype->name.compare(prototype_decl_name) && !prototype->is_declaration())
			return prototype;

	return nullptr;
}