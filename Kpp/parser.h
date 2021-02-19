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
		ast::StmtBody* parse_body(ast::StmtBody* body);
		ast::Stmt* parse_statement();
		ast::Expr* parse_expression();
		opt_token_info parse_type();
		opt_token_info parse_id();

	};
}