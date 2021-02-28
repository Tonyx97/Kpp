#include <defs.h>

#include "ir.h"

using namespace kpp;

namespace kpp::ir
{
	void StackAlloc::print()
	{
		PRINT_TABS_NL(C_WHITE, 1, value + " = stackalloc " + STRINGIFY_TYPE(ty));
	}

	void Store::print()
	{
		PRINT_TABS_NL(C_WHITE, 1, "store " + STRINGIFY_TYPE(ty) + "* " + value + ", " + STRINGIFY_TYPE(operand->get_type()) + " " + operand->get_value());
	}

	void ValueInt::print()
	{
		//PRINT_TABS_NL(C_WHITE, 1, name + " = " + std::to_string(value.u64));
	}

	void BinaryOp::print()
	{
		PRINT_TABS_NL(C_WHITE, 1, value + " = " + STRINGIFY_OP_IR(op) + " " + left->get_value() + ", " + right->get_value());
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
	{
		PRINT_NNL(C_WHITE, "%s %s(", STRINGIFY_TYPE(prototype->return_type).c_str(), prototype->name.c_str());

		dbg::print_vec<ir::PrototypeParam>(C_WHITE, prototype->params, ", ", [](auto stmt)
		{
			return STRINGIFY_TYPE(stmt->type) + " " + stmt->name;
		});

		if (!prototype->items.empty())
		{
			PRINT(C_WHITE, ")");

			print_prototype(prototype);
		}
		else PRINT(C_WHITE, ") {}");
			
		PRINT_NL;
	}
}

void ir_gen::print_prototype(ir::Prototype* prototype)
{
	PRINT_TABS_NL(C_WHITE, print_level, "{");

	++print_level;

	for (auto&& item : prototype->items)
		print_item(item);

	--print_level;

	PRINT_TABS_NL(C_WHITE, print_level, "}");
}

void ir_gen::print_item(ir::Base* base)
{
	base->print();
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

		ir_prototype->params.push_back(new ir::PrototypeParam(decl_or_assign->name, decl_or_assign->type));
	}

	if (prototype->body)
		ir_prototype->body = generate_from_body(prototype->body);

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
		switch (stmt->stmt_type)
		{
		case ast::STMT_BODY: generate_from_body(reinterpret_cast<ast::StmtBody*>(stmt)); break;
		case ast::STMT_EXPR: generate_from_expr(reinterpret_cast<ast::Expr*>(stmt));	 break;
		}
	}

	return ir_body;
}

ir::Base* ir_gen::generate_from_expr(ast::Expr* expr)
{
	switch (expr->expr_type)
	{
	case ast::EXPR_DECL_OR_ASSIGN: return generate_from_expr_decl_or_assign(static_cast<ast::ExprDeclOrAssign*>(expr));
	case ast::EXPR_INT_LITERAL:	   return generate_from_expr_int_literal(static_cast<ast::ExprIntLiteral*>(expr));
	case ast::EXPR_BINARY_OP:	   return generate_from_expr_binary_op(static_cast<ast::ExprBinaryOp*>(expr));
	//case ast::EXPR_ID:		   return generate_expr_id(static_cast<ast::ExprId*>(expr));
	}

	return nullptr;
}

ir::Base* ir_gen::generate_from_expr_decl_or_assign(ast::ExprDeclOrAssign* expr)
{
	if (expr->is_declaration())
	{
		auto stack_alloc = new ir::StackAlloc();

		stack_alloc->value = pi.create_value(expr->name, stack_alloc);
		stack_alloc->ty = expr->type;

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

		return nullptr;
	}
}

ir::ValueInt* ir_gen::generate_from_expr_int_literal(ast::ExprIntLiteral* expr)
{
	auto value_int = new ir::ValueInt();

	value_int->value = expr->value;
	value_int->ty = expr->type;
	value_int->name = pi.create_value(expr->base_name, value_int);

	pi.create_item(value_int);

	return value_int;
}

ir::BinaryOp* ir_gen::generate_from_expr_binary_op(ast::ExprBinaryOp* expr)
{
	auto binary_op = new ir::BinaryOp();

	binary_op->value = pi.create_value(expr->base_name, binary_op);
	binary_op->left = generate_from_expr(expr->left);
	binary_op->op = expr->op;
	binary_op->ty = expr->ty;
	binary_op->right = generate_from_expr(expr->right);

	pi.create_item(binary_op);

	return binary_op;
}

/*
ir::ExprId* ir_gen::generate_expr_id(ast::ExprId* expr)
{
	auto expr_id = ir::ExprId::create();

	if (auto var_name = pi.get_value_from_name(expr->name))
		expr_id->var_name = *var_name;
	else expr_id->var_name = pi.create_value(expr->name, expr_id);

	return expr_id;
}

ir::ExprBinaryOp* ir_gen::generate_expr_binary_op(ast::ExprBinaryOp* expr)
{
	ir::Expr* left = nullptr,
			* right = nullptr;

	if (expr->left)
		left = generate_expr(expr->left);

	if (expr->right)
		right = generate_expr(expr->right);

	auto expr_bin_op = ir::ExprBinaryOp::create(left, expr->op, right);

	expr_bin_op->var_name = pi.create_value("binary_op", expr_bin_op);

	pi.create_item(expr_bin_op);

	return expr_bin_op;
}*/

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