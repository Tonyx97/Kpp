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
			print_expr(static_cast<ir::Expr*>(item));
			break;
		}
		}
	}

	--print_level;

	PRINT_TABS_NL(C_WHITE, print_level, "}");
}

void ir_parser::print_expr(ir::Expr* expr_base)
{
	switch (expr_base->ir_expr_type)
	{
	case ir::IR_EXPR_DECL_OR_ASSIGN:
	{
		auto expr = static_cast<ir::ExprDeclOrAssign*>(expr_base);
		
		const bool declaration = expr->is_declaration();

		if (declaration)
			PRINT_TABS(C_WHITE, print_level, "%s %s", STRINGIFY_TYPE(expr->type).c_str(), expr->name.c_str());
		else PRINT_TABS(C_WHITE, print_level, "%s", expr->name.c_str());

		if (expr->value)
		{
			PRINT_NNL(C_WHITE, " = %s");



			print_expr(expr->value);
		}

		PRINT_NL;

		break;
	}
	case ir::IR_EXPR_BINARY_OP:
	{
		auto expr = static_cast<ir::ExprBinaryOp*>(expr_base);

		if (expr->left)
			print_expr(expr->left);

		PRINT_NNL(C_WHITE, " %s ", STRINGIFY_OPERATOR(expr->op).c_str());

		if (expr->right)
			print_expr(expr->right);

		break;
	}
	case ir::IR_EXPR_INT_LITERAL:
	{
		print_expr_int_literal(static_cast<ir::ExprIntLiteral*>(expr_base));
		break;
	}
	}
}

void ir_parser::print_expr_int_literal(ir::ExprIntLiteral* expr)
{
	switch (expr->type)
	{
	case TOKEN_U8:  PRINT_NNL(C_WHITE, "%i", expr->value.u8);  break;
	case TOKEN_U16: PRINT_NNL(C_WHITE, "%i", expr->value.u16); break;
	case TOKEN_U32: PRINT_NNL(C_WHITE, "%i", expr->value.u32); break;
	case TOKEN_U64: PRINT_NNL(C_WHITE, "%i", expr->value.u64); break;
	case TOKEN_I8:  PRINT_NNL(C_WHITE, "%i", expr->value.i8);  break;
	case TOKEN_I16: PRINT_NNL(C_WHITE, "%i", expr->value.i16); break;
	case TOKEN_I32: PRINT_NNL(C_WHITE, "%i", expr->value.i32); break;
	case TOKEN_I64: PRINT_NNL(C_WHITE, "%i", expr->value.i64); break;
	}
}

void ir_parser::add_prototype(ir::Prototype* prototype)
{
	gi.prototypes.insert({ prototype->name, prototype });
	iri.prototypes.push_back(prototype);
}

void ir_parser::add_var(const std::string& name, ir::Expr* expr)
{
	pi.vars.insert({ name, expr });
}

ir::Expr* ir_parser::get_declared_var(const std::string& name)
{
	auto it = pi.vars.find(name);
	return (it != pi.vars.end() ? it->second : nullptr);
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

	auto ir_prototype = pi.curr_prototype = new ir::Prototype();

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

	pi.clear();

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
		case ast::STMT_EXPR: ir_body->items.push_back(generate_expr(reinterpret_cast<ast::Expr*>(stmt)));	  break;
		}
	}

	return ir_body;
}

ir::Expr* ir_parser::generate_expr(ast::Expr* expr)
{
	switch (expr->expr_type)
	{
	case ast::EXPR_DECL_OR_ASSIGN: return generate_expr_decl_or_assign(static_cast<ast::ExprDeclOrAssign*>(expr));
	case ast::EXPR_INT_LITERAL:	   return generate_expr_int_literal(static_cast<ast::ExprIntLiteral*>(expr));
	case ast::EXPR_BINARY_OP:	   return generate_expr_binary_op(static_cast<ast::ExprBinaryOp*>(expr));
	}

	return nullptr;
}

ir::ExprDeclOrAssign* ir_parser::generate_expr_decl_or_assign(ast::ExprDeclOrAssign* expr)
{
	auto expr_decl_or_assign = ir::ExprDeclOrAssign::create();

	expr_decl_or_assign->type = expr->type;

	if (expr->value)
		expr_decl_or_assign->value = generate_expr(expr->value);

	auto decl_var = get_declared_var(expr->name);

	expr_decl_or_assign->name = decl_var ? decl_var->get_name() : "v" + std::to_string(pi.vars.size());

	add_var(expr->name, expr_decl_or_assign);

	return expr_decl_or_assign;
}

ir::ExprIntLiteral* ir_parser::generate_expr_int_literal(ast::ExprIntLiteral* expr)
{
	return ir::ExprIntLiteral::create(expr->value, expr->type);
}

ir::ExprBinaryOp* ir_parser::generate_expr_binary_op(ast::ExprBinaryOp* expr)
{
	ir::Expr* left = nullptr,
			* right = nullptr;

	if (expr->left)
		left = generate_expr(expr->left);

	if (expr->right)
		right = generate_expr(expr->right);

	return ir::ExprBinaryOp::create(left, expr->op, right);
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