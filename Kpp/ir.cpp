#include <defs.h>

#include "ir.h"

using namespace kpp;

ir_parser::ir_parser(ast::AST* tree) : tree(tree)
{
	ir_info = new ir::IR();
}

ir_parser::~ir_parser()
{
	delete ir_info;
}

void ir_parser::print_ir()
{
	for (auto&& prototype : ir_info->prototypes)
	{
		PRINT_NNL(C_WHITE, "%s %s(", STRINGIFY_TYPE(prototype->return_type).c_str(), prototype->name.c_str());

		dbg::print_vec<ir::PrototypeParameter>(C_WHITE, prototype->params, ", ", [](auto stmt)
		{
			return STRINGIFY_TYPE(stmt->type) + " " + stmt->name;
		});

		if (prototype->declaration)
			PRINT(C_WHITE, ");");
		else
		{
			PRINT(C_WHITE, ")\n{");

			print_body();

			PRINT(C_WHITE, "}");
		}
			
		PRINT_NL;
	}
}

void ir_parser::print_body()
{

}

bool ir_parser::generate()
{
	if (!tree)
		return false;

	for (auto&& prototype : tree->prototypes)
		generate_prototype(prototype);

	return true;
}

bool ir_parser::is_prototype_declared(const std::string& name)
{
	auto it = info.decl_prototypes.find(name);
	return (it != info.decl_prototypes.end());
}

bool ir_parser::is_prototype_defined(const std::string& name)
{
	auto it = info.prototypes.find(name);
	return (it != info.prototypes.end() && !it->second->declaration);
}

bool ir_parser::is_var_declared(const std::string& name)
{
	return (info.vars.find(name) != info.vars.end());
}

ir::Prototype* ir_parser::get_prototype(const std::string& name)
{
	if (auto it = info.decl_prototypes.find(name); it == info.decl_prototypes.end())
	{
		it = info.prototypes.find(name);
		return (it != info.prototypes.end() ? it->second : nullptr);
	}
	else return it->second;
}

void ir_parser::add_var(const std::string& name)
{
	info.vars.insert({ name, name });
}

void ir_parser::add_prototype(ir::Prototype* prototype)
{
	if (prototype->declaration)
		info.decl_prototypes.insert({ prototype->name, prototype });
	else info.prototypes.insert({ prototype->name, prototype });
}

void ir_parser::generate_prototype(ast::Prototype* ast_prototype)
{
	const bool defined = is_prototype_defined(ast_prototype->name),
			   declared = is_prototype_declared(ast_prototype->name);

	if (defined)
		return semantic_error("Prototype '%s' already has a body", ast_prototype->name.c_str());
	else if (ast_prototype->declaration && declared)
		return semantic_error("Prototype '%s' already declared", ast_prototype->name.c_str());

	auto prototype = info.current = new ir::Prototype();

	prototype->name = ast_prototype->name;
	prototype->return_type = ast_prototype->return_type;
	prototype->declaration = ast_prototype->declaration;

	for (auto&& ast_param : ast_prototype->params)
	{
		auto ast_param_decl = static_cast<ast::ExprDeclOrAssign*>(ast_param);

		if (std::find_if(prototype->params.begin(), prototype->params.end(), [&](auto param) { return !param->name.compare(ast_param_decl->name); }) != prototype->params.end())
			return semantic_error("Parameter '%s' already defined in prototype declaration", ast_param_decl->name.c_str());

		auto param = new ir::PrototypeParameter();

		param->name = ast_param_decl->name;
		param->type = ast_param_decl->type;

		prototype->params.push_back(param);
	}

	if (ast_prototype->body)
		generate_body(ast_prototype->body);

	add_prototype(prototype);

	ir_info->prototypes.push_back(prototype);
}

void ir_parser::generate_body(ast::StmtBody* ast_body)
{
	for (auto&& stmt : ast_body->stmts)
	{
		if (stmt->stmt_type == ast::STMT_BODY)
			generate_body(static_cast<ast::StmtBody*>(stmt));
		else
		{
			if (stmt->stmt_type == ast::STMT_EXPR)
				generate_expr(static_cast<ast::Expr*>(stmt));
		}
	}
}

void ir_parser::generate_expr(ast::Expr* ast_expr)
{
	switch (ast_expr->expr_type)
	{
	case ast::EXPR_DECL_OR_ASSIGN:
	{
		auto expr = static_cast<ast::ExprDeclOrAssign*>(ast_expr);

		const bool declared = is_var_declared(expr->name);

		if (expr->is_declaration())
		{
			if (declared)
				return semantic_error("Identifier '%s' already declared", expr->name.c_str());

			add_var(expr->name);
		}
		else if (!declared)
			return semantic_error("Identifier '%s' is undefined", expr->name.c_str());

		break;
	}
	case ast::EXPR_CALL:
	{
		auto expr = static_cast<ast::ExprCall*>(ast_expr);

		const auto& prototype_name = expr->name;

		auto prototype = get_prototype(prototype_name);
		if (!prototype)
			return semantic_error("Identifier '%s' not found", prototype_name.c_str());

		const auto original_params_length = prototype->params.size(),
				   current_params_length = expr->stmts.size();

		if (current_params_length < original_params_length)
			return semantic_error("Too few arguments in function call '%s'", prototype_name.c_str());
		else if (current_params_length > original_params_length)
			return semantic_error("Too many arguments in function call '%s'", prototype_name.c_str());
		
		for (size_t i = 0ull; i < expr->stmts.size(); ++i)
		{
			const auto& original_param = prototype->params[i];
			const auto& current_param = expr->stmts[i];

			if (original_param->type != current_param->type)
				return semantic_error("Argument of type '%s' is incompatible with parameter of type '%s'",
									  STRINGIFY_TYPE(current_param->type).c_str(),
									  STRINGIFY_TYPE(original_param->type).c_str());
		}

		break;
	}
	}
}