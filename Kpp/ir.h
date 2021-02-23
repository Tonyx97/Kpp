#pragma once

#include "ast.h"

namespace kpp
{
	namespace ir
	{
		struct PrototypeParameter
		{
			Token type;

			std::string name;
		};

		struct Prototype
		{
			Token return_type = TOKEN_NONE;
			
			std::string name;

			bool declaration = false;

			std::vector<PrototypeParameter*> params;
		};

		struct IR
		{
			std::vector<Prototype*> prototypes;
		};

		struct Info
		{
			std::unordered_map<std::string, Prototype*> prototypes,
														decl_prototypes;

			std::unordered_map<std::string, std::string> vars;

			Prototype* current = nullptr;
		};
	}

	class ir_parser
	{
	private:

		ast::AST* tree = nullptr;

		ir::IR* ir_info = nullptr;

		ir::Info info {};

	public:

		ir_parser(ast::AST* tree);
		~ir_parser();

		void print_ir();
		void print_body();

		bool generate();
		bool is_prototype_declared(const std::string& name);
		bool is_prototype_defined(const std::string& name);
		bool is_var_declared(const std::string& name);

		ir::Prototype* get_prototype(const std::string& name);

		void add_var(const std::string& name);
		void add_prototype(ir::Prototype* prototype);

		void generate_prototype(ast::Prototype* ast_prototype);
		void generate_body(ast::StmtBody* ast_body);
		void generate_expr(ast::Expr* ast_expr);
	};
}