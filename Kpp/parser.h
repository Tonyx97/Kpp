#pragma once

#include "ast.h"

namespace kpp
{
	class parser
	{
	private:

		lexer& lex;

		ast::AST* tree = nullptr;

	public:

		parser(lexer& lex);
		~parser();

		void print_ast();

		bool parse();

		ast::AST* get_ast() { return tree; }

		ast::Prototype* parse_prototype();
		std::vector<ast::Base*> parse_prototype_params_decl();
		std::vector<ast::Expr*> parse_call_params();
		ast::StmtBody* parse_body(ast::StmtBody* body);
		ast::Base* parse_statement();
		ast::Expr* parse_expression();
		ast::Expr* parse_expression_precedence(ast::Expr* lhs, int min_precedence = LOWEST_PRECEDENCE);
		ast::Expr* parse_primary_expression();
		opt_token_info parse_type();
		opt_token_info parse_keyword();
		opt_token_info parse_id();
	};
}