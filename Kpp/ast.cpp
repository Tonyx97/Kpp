#include <defs.h>

#include "lexer.h"

#include "ast.h"

using namespace kpp;

void ast::Printer::print(AST* tree)
{
	for (auto&& prototype : tree->prototypes)
		print_prototype(prototype);
}

void ast::Printer::print_prototype(Prototype* prototype)
{
	if (first_prototype_printed)
		PRINT_NL;

	PRINT_TABS(C_WHITE, 0, "Prototype '%s'", prototype->name.c_str());

	if (!prototype->params.empty())
	{
		PRINT_TABS(C_WHITE, 0, " | Arguments: ");

		dbg::print_vec<ExprDeclOrAssign>(C_GREEN, prototype->params, ", ", [](ExprDeclOrAssign* e)
		{
			return STRINGIFY_TYPE(e->ty) + " " + e->name;
		});
	}

	const bool declaration = prototype->is_declaration();

	if (declaration)
		PRINT_TABS(C_WHITE, 0, " (Declaration)");
	else PRINT_NL;

	if (prototype->body)
		print_body(prototype->body);

	if (!declaration)
		PRINT_TABS_NL(C_WHITE, curr_level, "End");

	first_prototype_printed = true;
}

void ast::Printer::print_body(StmtBody* body)
{
	++curr_level;

	PRINT_TABS_NL(C_CYAN, curr_level, "Body");

	for (auto&& stmt_base : body->stmts)
		print_stmt(stmt_base);

	PRINT_TABS_NL(C_CYAN, curr_level, "End");

	--curr_level;
}

void ast::Printer::print_stmt(Base* stmt)
{
	if (auto body = rtti::safe_cast<StmtBody>(stmt))
		print_body(body);
	else if (auto stmt_if = rtti::safe_cast<StmtIf>(stmt))
		print_if(stmt_if);
	else if (auto stmt_for = rtti::safe_cast<StmtFor>(stmt))
		print_for(stmt_for);
	else if (auto expr = rtti::safe_cast<Expr>(stmt))
		print_expr(expr);
}

void ast::Printer::print_if(StmtIf* stmt_if)
{
	++curr_level;

	PRINT_TABS_NL(C_BLUE, curr_level, "If");

	print_expr(stmt_if->expr);

	print_body(stmt_if->if_body);

	for (auto&& else_if : stmt_if->ifs)
	{
		PRINT_TABS_NL(C_BLUE, curr_level, "Else If");

		print_expr(else_if->expr);

		print_body(else_if->if_body);
	}

	if (stmt_if->else_body)
	{
		PRINT_TABS_NL(C_BLUE, curr_level, "Else");

		print_body(stmt_if->else_body);
	}

	--curr_level;
}

void ast::Printer::print_for(ast::StmtFor* stmt_for)
{
	++curr_level;

	PRINT_TABS_NL(C_BLUE, curr_level, "For");

	print_stmt(stmt_for->init);
	print_expr(stmt_for->condition);
	print_stmt(stmt_for->step);
	print_body(stmt_for->body);

	--curr_level;
}

void ast::Printer::print_expr(Expr* expr)
{
	++curr_level;

	if (auto int_literal = rtti::safe_cast<ExprIntLiteral>(expr))
		print_expr_int(int_literal);
	else if (auto id = rtti::safe_cast<ExprId>(expr))
		print_id(id);
	else if (auto decl_or_assign = rtti::safe_cast<ExprDeclOrAssign>(expr))
		print_decl_or_assign(decl_or_assign);
	else if (auto binary_op = rtti::safe_cast<ExprBinaryOp>(expr))
		print_expr_binary_op(binary_op);
	else if (auto call = rtti::safe_cast<ExprCall>(expr))
		print_expr_call(call);

	--curr_level;
}

void ast::Printer::print_decl_or_assign(ExprDeclOrAssign* assign)
{
	if (assign->ty != TOKEN_NONE)
	{
		PRINT_TABS_NL(C_YELLOW, curr_level, "Declaration assignment '%s' (%s)", assign->name.c_str(), STRINGIFY_TYPE(assign->ty).c_str());
	}
	else
	{
		if (assign->value)
		{
			PRINT_TABS_NL(C_YELLOW, curr_level, "Assignment '%s'", assign->name.c_str());
		}
		else
		{
			PRINT_TABS_NL(C_YELLOW, curr_level, "Declaration '%s'", assign->name.c_str());
		}
	}

	if (assign->value)
		print_expr(assign->value);
}

void ast::Printer::print_expr_int(ExprIntLiteral* expr)
{
	switch (expr->ty)
	{
	case TOKEN_U8:  PRINT_TABS_NL(C_YELLOW, curr_level, "Expr '%i' u8", expr->value.u8);   break;
	case TOKEN_U16: PRINT_TABS_NL(C_YELLOW, curr_level, "Expr '%i' u16", expr->value.u16); break;
	case TOKEN_U32: PRINT_TABS_NL(C_YELLOW, curr_level, "Expr '%i' u32", expr->value.u32); break;
	case TOKEN_U64: PRINT_TABS_NL(C_YELLOW, curr_level, "Expr '%i' u64", expr->value.u64); break;
	case TOKEN_I8:  PRINT_TABS_NL(C_YELLOW, curr_level, "Expr '%i' i8", expr->value.i8);   break;
	case TOKEN_I16: PRINT_TABS_NL(C_YELLOW, curr_level, "Expr '%i' i16", expr->value.i16); break;
	case TOKEN_I32: PRINT_TABS_NL(C_YELLOW, curr_level, "Expr '%i' i32", expr->value.i32); break;
	case TOKEN_I64: PRINT_TABS_NL(C_YELLOW, curr_level, "Expr '%i' i64", expr->value.i64); break;
	}
}

void ast::Printer::print_id(ast::ExprId* expr)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Id '%s'", expr->name.c_str());
}

void ast::Printer::print_expr_binary_op(ExprBinaryOp* expr)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Binary Op (%s)", expr->get_name().c_str());

	++curr_level;

	if (expr->left)
		PRINT_TABS_NL(C_YELLOW, curr_level, "Left operand '%s'", expr->left->get_name().c_str());

	if (auto left = rtti::safe_cast<ExprBinaryOp>(expr->left))
		print_expr_binary_op(left);

	if (expr->right)
		PRINT_TABS_NL(C_YELLOW, curr_level, "Right operand '%s'", expr->right->get_name().c_str());

	if (auto right = rtti::safe_cast<ExprBinaryOp>(expr->right))
		print_expr_binary_op(right);

	--curr_level;
}

void ast::Printer::print_expr_call(ExprCall* expr)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Prototype Call (%s)", expr->name.c_str());

	for (auto&& param : expr->stmts)
	{
		if (auto call = rtti::safe_cast<ExprCall>(param))
		{
			++curr_level;

			print_expr_call(call);

			--curr_level;
		}
		else print_expr(param);
	}
}