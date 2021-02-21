#pragma once

#include "ast.h"

namespace kpp
{
	class parser
	{
	private:

		lexer& lex;

		std::vector<ast::Prototype*> prototypes;

	public:

		parser(lexer& lex) : lex(lex) {}
		~parser();

		void print_ast();

		bool parse();

		ast::Prototype* parse_prototype();
		std::vector<ast::StmtBase*> parse_prototype_params_decl();
		ast::StmtBody* parse_body(ast::StmtBody* body);
		ast::StmtBase* parse_statement();
		ast::Expr* parse_expression();
		ast::Expr* parse_expression_precedence(ast::Expr* lhs, int min_precedence = LOWEST_PRECEDENCE);
		ast::Expr* parse_primary_expression();
		opt_token_info parse_type();
		opt_token_info parse_keyword();
		opt_token_info parse_id();

	};
}