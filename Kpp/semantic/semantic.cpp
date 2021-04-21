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

	if (prototype->is_declaration())
		add_prototype_decl(prototype->name);
	else add_prototype(prototype->name);

	// we have to check if the params and their types matches with the declaration
	// the problem is when a function tries to use a function that was declared as X
	// and it's used as Y
	
	for (auto&& param : prototype->params)
	{
		auto param_decl = static_cast<ast::ExprDeclOrAssign*>(param);

		if (pi.values.find(param_decl->name) != pi.values.end())
			add_error("Parameter '%s' already defined in prototype declaration", param_decl->name.c_str());

		add_variable(param_decl);
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
		if (auto body = rtti::safe_cast<ast::StmtBody>(stmt))		analyze_body(body);
		else if (auto expr = rtti::safe_cast<ast::Expr>(stmt))		analyze_expr(expr);
		else if (auto stmt_if = rtti::safe_cast<ast::StmtIf>(stmt)) analyze_if(stmt_if);
	}

	return true;
}

bool semantic::analyze_expr(ast::Expr* expr)
{
	if (auto id = rtti::safe_cast<ast::ExprId>(expr))
	{
		auto variable = get_declared_variable(id->name);

		if (!variable)
			add_error("'%s' identifier is undefined", id->name.c_str());
		else id->set_ty(variable->ty);
	}
	else if (auto decl_or_assign = rtti::safe_cast<ast::ExprDeclOrAssign>(expr))
	{
		auto declared_var = get_declared_variable(decl_or_assign->name);

		if (decl_or_assign->is_declaration())
		{
			if (declared_var)
				add_error("'%s %s' redefinition", STRINGIFY_TYPE(decl_or_assign->ty).c_str(), decl_or_assign->name.c_str());

			add_variable(decl_or_assign);
		}
		else if (!declared_var)
			add_error("'%s' identifier is undefined", decl_or_assign->name.c_str());

		if (decl_or_assign->value)
			return analyze_expr(decl_or_assign->value);
	}
	else if (auto binary_op = rtti::safe_cast<ast::ExprBinaryOp>(expr))
	{
		if (!binary_op->left || !analyze_expr(binary_op->left))
			add_error("Expected an expression");

		if (!binary_op->right || !analyze_expr(binary_op->right))
			add_error("Expected an expression");
	}
	else if (auto unary_op = rtti::safe_cast<ast::ExprUnaryOp>(expr))
	{
		if (auto value_unary_op = rtti::safe_cast<ast::ExprUnaryOp>(unary_op->value))
			return analyze_expr(value_unary_op);
		else
		{
			if (!rtti::safe_cast<ast::ExprId>(unary_op->value) &&
				!rtti::safe_cast<ast::ExprIntLiteral>(unary_op->value))
				add_error("Expression must be an lvalue");

			return analyze_expr(unary_op->value);
		}
	}
	else if (auto call = rtti::safe_cast<ast::ExprCall>(expr))
	{
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

			if (!analyze_expr(current_param))
				return false;

			if (original_param->ty != current_param->get_ty())
				add_error("Argument of type '%s' is incompatible with parameter of type '%s'",
						  STRINGIFY_TYPE(current_param->get_ty()).c_str(),
						  STRINGIFY_TYPE(original_param->ty).c_str());
		}

		call->prototype = prototype;

		gi.prototype_calls.insert({ prototype_name, pi.curr_prototype->name });
	}

	return true;
}

bool semantic::analyze_if(ast::StmtIf* stmt_if)
{
	auto internal_analyze_if = [&](ast::StmtIf* curr_if)
	{
		if (!analyze_expr(curr_if->expr))
			return false;

		if (curr_if->if_body)
			if (!analyze_body(curr_if->if_body))
				return false;

		if (curr_if->else_body)
			if (!analyze_body(curr_if->else_body))
				return false;

		return true;
	};

	if (!internal_analyze_if(stmt_if))
		return false;

	for (auto&& else_if : stmt_if->ifs)
		if (!internal_analyze_if(else_if))
			return false;

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

void semantic::add_variable(ast::ExprDeclOrAssign* expr)
{
	pi.values.insert({ expr->name, expr });
}

ast::ExprDeclOrAssign* semantic::get_declared_variable(const std::string& name)
{
	auto it = pi.values.find(name);
	return (it != pi.values.end() ? it->second : nullptr);
}

ast::Prototype* semantic::get_prototype(const std::string& name)
{
	for (auto&& prototype : ast_tree->prototypes)
		if (!prototype->name.compare(name))
			return prototype;

	return nullptr;
}