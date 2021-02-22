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

			size_t stack_size = 0;

			std::vector<PrototypeParameter*> stmts;
		};

		struct IR
		{
			std::vector<Prototype*> prototypes;
		};
	}

	class ir_parser
	{
	private:

		ast::AST* tree = nullptr;

		ir::IR* ir_info = nullptr;

	public:

		ir_parser(ast::AST* tree);
		~ir_parser();

		void print_ir();

		bool generate();

		void generate_prototype(ast::Prototype* ast_prototype);
	};
}