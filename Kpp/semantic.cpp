#include <defs.h>

#include "semantic.h"

using namespace kpp;

void semantic::print_errors()
{
	for (auto&& err : errors)
		PRINT(C_RED, err);
}

bool semantic::analyze()
{
	for (auto&& prototype : ast_tree->prototypes)
		analyze_prototype(prototype);

	for (auto&& prototype_decl : gi.decl_prototypes)
		if (gi.prototypes.find(prototype_decl) == gi.prototypes.end())
		{
			if (auto it = gi.prototype_calls.find(prototype_decl); it != gi.prototype_calls.end())
				add_error("Unresolved external '%s' referenced in function '%s'", prototype_decl.c_str(), it->second.c_str());
			else add_error("Unresolved external '%s' declared", prototype_decl.c_str());
		}

	return errors.empty();
}

bool semantic::analyze_prototype(ast::Prototype* prototype)
{
	pi.curr_prototype = prototype;

	if (prototype->declaration)
		add_prototype_decl(prototype->name);
	else add_prototype(prototype->name);

	// we have to check if the params and their types matches with the declaration
	// the problem is when a function tries to use a function that was declared as X
	// and it's used as Y
	
	for (auto&& param : prototype->params)
	{
		auto param_decl = static_cast<ast::ExprDeclOrAssign*>(param);

		if (pi.vars.find(param_decl->name) != pi.vars.end())
			add_error("Parameter '%s' already defined in prototype declaration", param_decl->name.c_str());

		pi.vars.insert(param_decl->name);
	}

	if (auto body = prototype->body)
		analyze_body(body);

	pi.clear();

	return true;
}

bool semantic::analyze_body(ast::StmtBody* body)
{
	for (auto&& stmt : body->stmts)
	{
		switch (stmt->stmt_type)
		{
		case ast::STMT_BODY:
		{
			analyze_body(static_cast<ast::StmtBody*>(stmt));
			break;
		}
		case ast::STMT_EXPR:
		{
			analyze_expr(static_cast<ast::Expr*>(stmt));
			break;
		}
		}
	}

	return true;
}

bool semantic::analyze_expr(ast::Expr* expr)
{
	switch (expr->expr_type)
	{
	case ast::EXPR_ID:
	{
		auto expr_id = static_cast<ast::ExprId*>(expr);

		if (!is_variable_declared(expr_id->name))
			add_error("'%s' identifier is undefined", expr_id->name.c_str());

		break;
	}
	case ast::EXPR_DECL_OR_ASSIGN:
	{
		auto decl_or_assign = static_cast<ast::ExprDeclOrAssign*>(expr);

		const bool declared = is_variable_declared(decl_or_assign->name);

		if (decl_or_assign->is_declaration())
		{
			if (declared)
				add_error("'%s %s' redefinition", STRINGIFY_TYPE(decl_or_assign->type).c_str(), decl_or_assign->name.c_str());

			add_variable(decl_or_assign->name);
		}
		else if (!declared)
			add_error("'%s' identifier is undefined", decl_or_assign->name.c_str());

		if (decl_or_assign->value)
			analyze_expr(decl_or_assign->value);

		break;
	}
	case ast::EXPR_BINARY_OP:
	{
		auto binary_op = static_cast<ast::ExprBinaryOp*>(expr);

		if (binary_op->left)
			analyze_expr(binary_op->left);

		if (binary_op->right)
			analyze_expr(binary_op->right);

		break;
	}
	case ast::EXPR_CALL:
	{
		auto call = static_cast<ast::ExprCall*>(expr);

		const auto& prototype_name = call->name;

		auto prototype = get_prototype(prototype_name);
		if (!prototype)
			return add_error("Identifier '%s' not found", prototype_name.c_str());

		const auto original_params_length = prototype->params.size(),
				   current_params_length = call->stmts.size();

		if (current_params_length < original_params_length)
			add_error("Too few arguments in function call '%s'", prototype_name.c_str());
		else if (current_params_length > original_params_length)
			add_error("Too many arguments in function call '%s'", prototype_name.c_str());

		for (size_t i = 0ull; i < call->stmts.size() && i < prototype->params.size(); ++i)
		{
			const auto& original_param = static_cast<ast::ExprDeclOrAssign*>(prototype->params[i]);
			const auto& current_param = call->stmts[i];

			if (original_param->type != current_param->type)
				add_error("Argument of type '%s' is incompatible with parameter of type '%s'",
						  STRINGIFY_TYPE(current_param->type).c_str(),
						  STRINGIFY_TYPE(original_param->type).c_str());
		}

		gi.prototype_calls.insert({ prototype_name, pi.curr_prototype->name });

		break;
	}
	}

	return true;
}

void semantic::add_prototype_decl(const std::string& name)
{
	gi.decl_prototypes.insert(name);
}

void semantic::add_prototype(const std::string& name)
{
	gi.prototypes.insert(name);
}

void semantic::add_variable(const std::string& name)
{
	pi.vars.insert(name);
}

bool semantic::is_variable_declared(const std::string& name)
{
	return (pi.vars.find(name) != pi.vars.end());
}

ast::Prototype* semantic::get_prototype(const std::string& name)
{
	for (auto&& prototype : ast_tree->prototypes)
		if (!prototype->name.compare(name))
			return prototype;

	return nullptr;
}