#include <defs.h>

#include "lexer.h"

#include "ast.h"

using namespace kpp;

void ast::Printer::print_expr_call(ExprCall* expr)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Prototype Call (%s)", expr->name.c_str());

	for (auto&& param : expr->stmts)
	{
		if (param->expr_type == EXPR_CALL)
		{
			++curr_level;

			print_expr_call(static_cast<ExprCall*>(param));

			--curr_level;
		}
		else print_expr(param);
	}
}

void ast::Printer::print_expr_binary_op(ExprBinaryOp* expr)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Binary Op (%s)", STRINGIFY_TOKEN(expr->op).c_str());

	++curr_level;

	PRINT_TABS_NL(C_YELLOW, curr_level, "Left operand '%s'", expr->left->value.c_str());

	if (expr->left->expr_type == EXPR_BINARY_OP)
		print_expr_binary_op(static_cast<ExprBinaryOp*>(expr->left));

	PRINT_TABS_NL(C_YELLOW, curr_level, "Right operand '%s'", expr->right->value.c_str());

	if (expr->right->expr_type == EXPR_BINARY_OP)
		print_expr_binary_op(static_cast<ExprBinaryOp*>(expr->right));

	--curr_level;
}

void ast::Printer::print_expr_int(Expr* expr)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Expr '%s'", expr->value.c_str());
}

void ast::Printer::print_assign(ExprDeclOrAssign* assign)
{
	if (assign->type != TOKEN_NONE)
	{
		PRINT_TABS_NL(C_YELLOW, curr_level, "Declaration assignment '%s' (%s)", assign->name.c_str(), STRINGIFY_TYPE(assign->type).c_str());
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

void ast::Printer::print_expr(Expr* expr)
{
	++curr_level;

	switch (expr->expr_type)
	{
	case EXPR_INT:
		print_expr_int(expr);
		break;
	case EXPR_ASSIGN:
		print_assign(static_cast<ExprDeclOrAssign*>(expr));
		break;
	case EXPR_BINARY_OP:
		print_expr_binary_op(static_cast<ExprBinaryOp*>(expr));
		break;
	case EXPR_CALL:
		print_expr_call(static_cast<ExprCall*>(expr));
		break;
	}

	--curr_level;
}

void ast::Printer::print_for(ast::StmtFor* stmt_for)
{
	PRINT_TABS_NL(C_BLUE, curr_level, "For");

	print_stmt(stmt_for->init);
	print_expr(stmt_for->condition);
	print_stmt(stmt_for->step);
	print_body(stmt_for->body);
}

void ast::Printer::print_if(StmtIf* stmt_if)
{
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
}

void ast::Printer::print_stmt(StmtBase* stmt)
{
	++curr_level;

	switch (stmt->stmt_type)
	{
	case STMT_BODY:
		print_body(static_cast<StmtBody*>(stmt));
		break;
	case STMT_IF:
		print_if(static_cast<StmtIf*>(stmt));
		break;
	case STMT_FOR:
		print_for(static_cast<StmtFor*>(stmt));
		break;
	case STMT_EXPR:
		print_expr(static_cast<Expr*>(stmt));
		break;
	}

	--curr_level;
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

void ast::Printer::print_prototype(Prototype* prototype)
{
	if (!prototype->body)
		return;

	if (first_prototype_printed)
		PRINT_NL;

	PRINT_TABS(C_WHITE, 0, "Prototype '%s'", prototype->name.c_str());

	if (!prototype->stmts.empty())
	{
		PRINT_TABS(C_WHITE, 0, " | Arguments: ");

		dbg::print_vec<ExprDeclOrAssign>(C_GREEN, prototype->stmts, ", ", [](ExprDeclOrAssign* e)
		{
			return STRINGIFY_TYPE(e->type) + " " + e->name;
		});
	}

	PRINT_NL;

	print_body(prototype->body);

	PRINT_TABS_NL(C_WHITE, curr_level, "End");

	first_prototype_printed = true;
}

void ast::Printer::print(AST* tree)
{
	for (auto&& prototype : tree->prototypes)
		print_prototype(prototype);
}