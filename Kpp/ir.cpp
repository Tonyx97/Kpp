#include <defs.h>

#include "ir.h"

using namespace kpp;

namespace kpp::ir
{
	void Call::print()
	{
		PRINT_TABS(C_PURPLE, 1, "call " + prototype->name + " ");

		dbg::print_vec<ir::Instruction>(C_WHITE, params, ", ", [](auto ins)
		{
			return ins->get_value();
		});

		PRINT_NL;
	}

	void StackAlloc::print()
	{
		PRINT_TABS_NL(C_YELLOW, 1, value + " = stackalloc " + STRINGIFY_TYPE(ty));
	}

	void Store::print()
	{
		PRINT_TABS_NL(C_RED, 1, "store " + STRINGIFY_TYPE(ty) + "* " + value + ", " + STRINGIFY_TYPE(operand->get_type()) + " " + operand->get_value());
	}

	void Load::print()
	{
		PRINT_TABS_NL(C_GREEN, 1, dest_value + " = load " + STRINGIFY_TYPE(ty) + ", " + STRINGIFY_TYPE(ty) + "* " + value->name);
	}

	void ValueInt::print()
	{
		//PRINT_TABS_NL(C_WHITE, 1, name + " = " + std::to_string(value.u64));
	}

	void BinaryOp::print()
	{
		PRINT_TABS_NL(C_CYAN, 1, value + " = " + STRINGIFY_BINARY_OP(op) + " " + left->get_value() + ", " + right->get_value());
	}

	void UnaryOp::print()
	{
		PRINT_TABS_NL(C_CYAN, 1, value + " = " + STRINGIFY_UNARY_OP(op) + " " + operand->get_value());
	}

	void Block::print()
	{
		PRINT_TABS_NL(C_WHITE, 0, name + ":");
	}

	void BranchCond::print()
	{
		if (!target_if_true || !target_if_false)
		{
			PRINT_TABS_NL(C_BLUE, 1, "bcond " + STRINGIFY_TYPE(get_type()));
		}
		else
		{
			PRINT_TABS_NL(C_BLUE, 1, "bcond " + STRINGIFY_TYPE(get_type()) + " " + comparison->get_value() + ", " + target_if_true->get_value() + ", " + target_if_false->get_value());
		}
	}

	void Branch::print()
	{
		PRINT_TABS_NL(C_BLUE, 1, "branch " + target->get_value());
	}

	void Return::print()
	{
		PRINT_TABS_NL(C_BLUE, 1, "ret " + get_value());
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
		pi.create_block(true);

		ir_prototype->body = generate_from_body(prototype->body);

		pi.create_return(ir_prototype->ret_ty);
	}

	add_prototype(ir_prototype);

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

	value_int->value = expr->value;
	value_int->ty = expr->ty;
	value_int->name = pi.add_value(expr->get_name(), value_int);

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

	value_id->name = value_to_load->get_value();

	load->value = value_id;
	load->ty = expr->ty;
	load->dest_value = pi.add_value(expr->name, load);

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

ir::BinaryOp* ir_gen::generate_from_expr_binary_op_cond(ast::ExprBinaryOp* expr)
{
	if (expr->op != TOKEN_LOGICAL_OR && expr->op != TOKEN_LOGICAL_AND)
		return generate_from_expr_binary_op(expr);

	auto left_block = pi.create_block(),
		 right_block = pi.create_block();

	auto bcond_left = new ir::BranchCond(),
		 bcond_right = new ir::BranchCond();

	auto gen_block = [&](ir::BranchCond* cond, ir::Block* block, ast::ExprBinaryOp* bin_op) -> ir::Instruction*
	{
		pi.add_block(block);

		if (auto new_expr = generate_from_expr_binary_op_cond(bin_op))
		{
			cond->comparison = new_expr;
			//cond->target_if_true = (expr->op == TOKEN_LOGICAL_AND && expr->left == bin_op ? right_block : pi.if_context.if_block);
			//cond->target_if_false = (expr->op == TOKEN_LOGICAL_AND && expr->left == bin_op ? right_block : pi.if_context.end_block);

			pi.add_item(cond);

			return new_expr;
		}
		else pi.destroy_block(block);

		return nullptr;
	};

	gen_block(bcond_left, left_block, static_cast<ast::ExprBinaryOp*>(expr->left));
	gen_block(bcond_right, right_block, static_cast<ast::ExprBinaryOp*>(expr->right));

	return nullptr;
}

ir::Instruction* ir_gen::generate_from_if(ast::StmtIf* stmt_if)
{
	auto if_block = pi.create_block(),
		 else_block = pi.create_block(),
		 end_block = pi.create_block();

	pi.if_context = { if_block, else_block, end_block };

	pi.create_new_branch_linked_to_next_block();

	if (auto bin_op = rtti::safe_cast<ast::ExprBinaryOp>(stmt_if->expr))
		generate_from_expr_binary_op_cond(bin_op);

	if (stmt_if->if_body)
	{
		pi.add_block(if_block);
		generate_from_body(stmt_if->if_body);
		pi.create_branch(nullptr, end_block);
	}
	else pi.destroy_block(if_block);

	/*for (auto&& else_if : stmt_if->ifs)
	{
		if (auto bin_op = rtti::safe_cast<ast::ExprBinaryOp>(else_if->expr))
			generate_from_expr_binary_op_cond(bin_op);

		pi.create_block(true);
		generate_from_body(else_if->if_body);
		pi.create_branch(nullptr, end_block);
	}*/

	if (stmt_if->else_body)
	{
		pi.add_block(else_block);
		generate_from_body(stmt_if->else_body);
		pi.create_branch(nullptr, end_block);
	}
	else pi.destroy_block(else_block);

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