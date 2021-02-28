#pragma once

#include "ast.h"

namespace kpp
{
	struct prototype_info
	{
		ast::Prototype* curr_prototype = nullptr;

		std::unordered_set<std::string> values;

		void clear()
		{
			curr_prototype = nullptr;

			values.clear();
		}
	};

	struct global_info
	{
		std::unordered_set<std::string> decl_prototypes,
										prototypes;

		std::unordered_map<std::string, std::string> prototype_calls;
	};

	class semantic
	{
	private:

		std::vector<std::string> errors;

		prototype_info pi {};
		global_info gi {};

		ast::AST* ast_tree = nullptr;
		
	public:

		semantic(ast::AST* ast_tree) : ast_tree(ast_tree) {}

		void print_errors();

		bool analyze();
		bool analyze_prototype(ast::Prototype* prototype);
		bool analyze_body(ast::StmtBody* body);
		bool analyze_expr(ast::Expr* expr);

		void add_prototype_decl(const std::string& name);
		void add_prototype(const std::string& name);
		void add_variable(const std::string& name);

		bool is_variable_declared(const std::string& name);

		ast::Prototype* get_prototype(const std::string& name);

		template <typename... A>
		inline bool add_error(const std::string& format, A... args)
		{
			errors.push_back(fmt(format, args...));
			return false;
		}
	};
}