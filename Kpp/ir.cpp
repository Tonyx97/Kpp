#include <defs.h>

#include "ir.h"

using namespace kpp;

ir_parser::ir_parser(ast::AST* tree) : tree(tree)
{
}

ir_parser::~ir_parser()
{
}

void ir_parser::print_ir()
{
	for (auto&& prototype : iri.prototypes)
	{
		PRINT_NNL(C_WHITE, "%s %s(", STRINGIFY_TYPE(prototype->return_type).c_str(), prototype->name.c_str());

		dbg::print_vec<ir::PrototypeParam>(C_WHITE, prototype->params, ", ", [](auto stmt)
		{
			return STRINGIFY_TYPE(stmt->type) + " " + stmt->name;
		});

		if (!prototype->body->items.empty())
		{
			PRINT(C_WHITE, ")");

			print_body(prototype->body);
		}
		else PRINT(C_WHITE, ") {}");
			
		PRINT_NL;
	}
}

void ir_parser::print_body(ir::IrBody* body)
{
	PRINT_TABS_NL(C_WHITE, print_level, "{");

	++print_level;

	for (auto&& item : body->items)
	{
		switch (item->ir_type)
		{
		case ir::IR_BODY:
		{
			print_body(static_cast<ir::IrBody*>(item));
			break;
		}
		case ir::IR_EXPR:
		{
			print_expr(static_cast<ir::IrExpr*>(item));
			break;
		}
		}
	}

	--print_level;

	PRINT_TABS_NL(C_WHITE, print_level, "}");
}

void ir_parser::print_expr(ir::IrExpr* expr_base)
{
	switch (expr_base->ir_expr_type)
	{
	case ir::IR_EXPR_DECL_OR_ASSIGN:
	{
		auto expr = static_cast<ir::IrExprDeclOrAssign*>(expr_base);

		PRINT_TABS_NL(C_WHITE, print_level, "%s %s", STRINGIFY_TYPE(expr->type).c_str(), expr->name.c_str());

		break;
	}
	}
}

void ir_parser::add_prototype(ir::Prototype* prototype)
{
	gi.prototypes.insert({ prototype->name, prototype });
	iri.prototypes.push_back(prototype);
}

bool ir_parser::generate()
{
	if (!tree)
		return false;

	for (auto&& prototype : tree->prototypes)
		generate_prototype(prototype);

	return true;
}

ir::Prototype* ir_parser::generate_prototype(ast::Prototype* prototype)
{
	if (auto defined_prototype = get_defined_prototype(prototype))
		return defined_prototype;

	if (prototype->declaration)
	{
		if (auto prototype_def = get_prototype_definition(prototype))
			return generate_prototype(prototype_def);
		else return nullptr;
	}

	auto ir_prototype = new ir::Prototype();

	ir_prototype->name = prototype->name;
	ir_prototype->return_type = prototype->return_type;

	for (auto&& param : prototype->params)
	{
		auto decl_or_assign = static_cast<ast::ExprDeclOrAssign*>(param);

		ir_prototype->params.push_back(new ir::PrototypeParam(decl_or_assign->name, decl_or_assign->type));
	}

	if (prototype->body)
		ir_prototype->body = generate_body(prototype->body);

	add_prototype(ir_prototype);

	return ir_prototype;
}

ir::IrBody* ir_parser::generate_body(ast::StmtBody* body)
{
	auto ir_body = new ir::IrBody();

	for (auto&& stmt : body->stmts)
	{
		switch (stmt->stmt_type)
		{
		case ast::STMT_BODY: ir_body->items.push_back(generate_body(reinterpret_cast<ast::StmtBody*>(stmt))); break;
		case ast::STMT_EXPR: ir_body->items.push_back(generate_expr(reinterpret_cast<ast::Expr*>(stmt)));	 break;
		}
	}

	return ir_body;
}

ir::IrExpr* ir_parser::generate_expr(ast::Expr* expr)
{
	switch (expr->expr_type)
	{
	case ast::EXPR_DECL_OR_ASSIGN: return generate_expr_decl_or_assign(static_cast<ast::ExprDeclOrAssign*>(expr));
	//case ir::IR_EXPR_BINARY_OP:
	//case ir::IR_EXPR_CALL:
	}

	return nullptr;
}

ir::IrExprDeclOrAssign* ir_parser::generate_expr_decl_or_assign(ast::ExprDeclOrAssign* expr)
{
	auto expr_decl_or_assign = new ir::IrExprDeclOrAssign();

	expr_decl_or_assign->type = expr->type;
	expr_decl_or_assign->name = expr->name;

	if (expr->value)
	{
		auto new_expr = new ir::IrExpr();

		switch (expr->value->expr_type)
		{
		case ast::EXPR_INT_LITERAL:
		{
			auto expr_int = static_cast<ast::Expr*>(expr);
			break;
		}
		}

		expr_decl_or_assign->value = new_expr;
	}

	return expr_decl_or_assign;
}

ir::Prototype* ir_parser::get_defined_prototype(ast::Prototype* prototype)
{
	auto it = gi.prototypes.find(prototype->name);
	return (it != gi.prototypes.end() ? it->second : nullptr);
}

ast::Prototype* ir_parser::get_prototype_definition(ast::Prototype* prototype_decl)
{
	if (!prototype_decl)
		return nullptr;

	const auto& prototype_decl_name = prototype_decl->name;

	for (auto&& prototype : tree->prototypes)
		if (!prototype->name.compare(prototype_decl_name) && !prototype->declaration)
			return prototype;

	return nullptr;
}