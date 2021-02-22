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

		dbg::print_vec<ir::PrototypeParameter>(C_WHITE, prototype->stmts, ", ", [](auto stmt)
		{
			return STRINGIFY_TYPE(stmt->type) + " " + stmt->name;
		});

		PRINT(C_WHITE, ")\n{");

		PRINT(C_WHITE, "}");
	}
}

bool ir_parser::generate()
{
	if (!tree)
		return false;

	for (auto&& prototype : tree->prototypes)
		generate_prototype(prototype);

	return true;
}

void ir_parser::generate_prototype(ast::Prototype* ast_prototype)
{
	auto prototype = new ir::Prototype();

	prototype->name = ast_prototype->name;
	prototype->return_type = ast_prototype->return_type;

	for (auto&& ast_param : ast_prototype->stmts)
	{
		auto ast_param_decl = static_cast<ast::ExprDeclOrAssign*>(ast_param);

		if (std::find_if(prototype->stmts.begin(), prototype->stmts.end(), [&](auto param) { return !param->name.compare(ast_param_decl->name); }) != prototype->stmts.end())
			return semantic_error("Parameter '%s' already defined in prototype declaration", ast_param_decl->name.c_str());

		auto param = new ir::PrototypeParameter();

		param->name = ast_param_decl->name;
		param->type = ast_param_decl->type;

		prototype->stmts.push_back(param);
	}

	ir_info->prototypes.push_back(prototype);
}