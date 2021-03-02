#include <defs.h>

#include "ir.h"

using namespace kpp;

namespace kpp::ir
{
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
		PRINT_TABS_NL(C_CYAN, 1, value + " = " + STRINGIFY_OP_IR(op) + " " + left->get_value() + ", " + right->get_value());
	}

	void Block::print()
	{
		PRINT_TABS_NL(C_WHITE, 0, name + ":");
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
	PRINT_NNL(C_WHITE, "%s %s(", STRINGIFY_TYPE(prototype->return_type).c_str(), prototype->name.c_str());

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
	ir_prototype->return_type = prototype->return_type;

	for (auto&& param : prototype->params)
	{
		auto decl_or_assign = static_cast<ast::ExprDeclOrAssign*>(param);

		ir_prototype->add_param(new ir::PrototypeParam(decl_or_assign->name, decl_or_assign->ty));
	}

	if (prototype->body)
	{
		pi.create_block();

		ir_prototype->body = generate_from_body(prototype->body);
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
	/*else if (auto call = rtti::safe_cast<ast::ExprCall>(expr))					 return generate_from_call(call);*/

	return nullptr;
}

ir::Instruction* ir_gen::generate_from_expr_decl_or_assign(ast::ExprDeclOrAssign* expr)
{
	if (expr->is_declaration())
	{
		auto stack_alloc = new ir::StackAlloc();

		stack_alloc->value = pi.create_value(expr->name, stack_alloc);
		stack_alloc->ty = expr->ty;

		pi.create_item(stack_alloc);

		if (expr->value)
		{
			auto store = new ir::Store();

			store->value = stack_alloc->value;
			store->ty = stack_alloc->ty;
			store->operand = generate_from_expr(expr->value);

			pi.create_item(store);
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

		pi.create_item(store);

		return store;
	}
}

ir::ValueInt* ir_gen::generate_from_expr_int_literal(ast::ExprIntLiteral* expr)
{
	auto value_int = new ir::ValueInt();

	value_int->value = expr->value;
	value_int->ty = expr->ty;
	value_int->name = pi.create_value(expr->get_name(), value_int);

	pi.create_item(value_int);

	return value_int;
}

ir::BinaryOp* ir_gen::generate_from_expr_binary_op(ast::ExprBinaryOp* expr)
{
	auto binary_op = new ir::BinaryOp();

	binary_op->left = generate_from_expr(expr->left);
	binary_op->op = expr->op;
	binary_op->ty = expr->ty;
	binary_op->right = generate_from_expr(expr->right);
	binary_op->value = pi.create_value(expr->get_name(), binary_op);

	pi.create_item(binary_op);

	return binary_op;
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
	load->dest_value = pi.create_value(expr->name, load);

	pi.create_item(load);

	return load;
}

ir::Instruction* ir_gen::generate_from_if(ast::StmtIf* stmt_if)
{
	//pi.create_block();

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