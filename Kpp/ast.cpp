#include <defs.h>

#include "lexer.h"

#include "ast.h"

using namespace kpp;
using namespace ast;

void Printer::print_expr_binary_op(BinaryOp* expr)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Binary Op (%s)", STRINGIFY_TOKEN(expr->op).c_str());

	++curr_level;

	PRINT_TABS_NL(C_YELLOW, curr_level, "Left operand '%s'", expr->left->value.c_str());

	if (expr->left->base_type == EXPR_BINARY_OP)
		print_expr_binary_op(static_cast<BinaryOp*>(expr->left));

	PRINT_TABS_NL(C_YELLOW, curr_level, "Right operand '%s'", expr->right->value.c_str());

	if (expr->right->base_type == EXPR_BINARY_OP)
		print_expr_binary_op(static_cast<BinaryOp*>(expr->right));

	--curr_level;
}

void Printer::print_expr_int(Expr* expr)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Expr int '%s'", expr->value.c_str());
}

void Printer::print_expr(Expr* expr)
{
	++curr_level;

	switch (expr->base_type)
	{
	case EXPR_INT:
		print_expr_int(expr);
		break;
	case EXPR_BINARY_OP:
		print_expr_binary_op(static_cast<BinaryOp*>(expr));
		break;
	}

	--curr_level;
}

void Printer::print_assign(StmtAssign* assign)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Decl. with assign '%s'", assign->name.c_str());
	print_expr(assign->value);
}

void Printer::print_decl(StmtDecl* decl)
{
	PRINT_TABS_NL(C_YELLOW, curr_level, "Declaration '%s' (%s)", decl->name.c_str(), STRINGIFY_TOKEN(decl->type).c_str());
}

void Printer::print_body(StmtBody* body)
{
	++curr_level;

	PRINT_TABS_NL(C_CYAN, curr_level, "Body");

	++curr_level;

	for (auto&& stmt_base : body->stmts)
	{
		switch (stmt_base->base_type)
		{
		case STMT_BODY:
			print_body(static_cast<StmtBody*>(stmt_base));
			break;
		case STMT_DECL:
			print_decl(static_cast<StmtDecl*>(stmt_base));
			break;
		case STMT_ASSIGN:
			print_assign(static_cast<StmtAssign*>(stmt_base));
			break;
		}
	}

	--curr_level;

	PRINT_TABS_NL(C_CYAN, curr_level, "End");

	--curr_level;
}

void Printer::print_prototype(Prototype* prototype)
{
	if (!prototype->body)
		return;

	if (first_prototype_printed)
		PRINT_NL;

	PRINT_TABS(C_WHITE, 0, "Prototype '%s'", prototype->name.c_str());

	if (!prototype->stmts.empty())
	{
		PRINT_TABS(C_WHITE, 0, " | Arguments: ");

		dbg::print_vec<StmtDecl>(C_GREEN, prototype->stmts, ", ", [](StmtDecl* stmt)
		{
			return stmt->name + " (" + STRINGIFY_TOKEN(stmt->type) + ")";
		});
	}

	PRINT_NL;

	print_body(prototype->body);

	PRINT_TABS_NL(C_WHITE, curr_level, "End");

	first_prototype_printed = true;
}

void Printer::print(const std::vector<Prototype*>& prototypes)
{
	for (auto&& prototype : prototypes)
		print_prototype(prototype);
}