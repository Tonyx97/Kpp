#include <defs.h>

#include "lexer.h"

#include "parser.h"

using namespace kpp;

void ast::Printer::print_body(ast::StmtBody* body)
{
	printf_s("begin body\n");

	for (auto&& stmt_base : body->stmts)
	{
		if (stmt_base->is_body)
			print_body((ast::StmtBody*)stmt_base);
		else
		{
			auto stmt = (ast::Stmt*)stmt_base;
			printf_s("statement '%s' (%i)\n", stmt->name.c_str(), stmt->type);
		}
	}

	printf_s("end body\n");
}

void ast::Printer::print_prototype(ast::Prototype* prototype)
{
	printf_s("begin prototype '%s'\n", prototype->name.c_str());

	print_body(prototype->body);

	printf_s("end prototype '%s'\n", prototype->name.c_str());
}

void ast::Printer::print(const std::vector<ast::Prototype*>& prototypes)
{
	for (auto&& prototype : prototypes)
		print_prototype(prototype);
}

parser::~parser()
{
	// we need to free the whole ast tree lol
}

void parser::print_ast()
{
	ast::Printer().print(prototypes);
}

bool parser::parse()
{
	while (!lex.eof())
	{
		if (auto prototype = parse_prototype())
			prototypes.push_back(prototype);
	}

	return true;
}

ast::Prototype* parser::parse_prototype()
{
	if (lex.eat_expect_keyword_declaration())
	{
		if (auto id = lex.eat_expect(TKN_ID))
		{
			const auto prototype_name = id.value().value;

			if (auto paren_open = lex.eat_expect(TKN_PAREN_OPEN))
			{
				// don't expect any arguments yet

				if (auto paren_close = lex.eat_expect(TKN_PAREN_CLOSE))
				{
					printf_s("we got a function '%s'\n", prototype_name.c_str());

					auto prototype = new ast::Prototype(prototype_name);

					if (prototype->body = parse_body(nullptr))
						return prototype;
					else printf_s("Failed parsing main prototype body\n");
				}
			}
		}
		else printf_s("[%s] SYNTAX ERROR: Expected function id\n", __FUNCTION__);
	}
	else printf_s("[%s] SYNTAX ERROR: Expected a return type\n", __FUNCTION__);

	return nullptr;
}

ast::StmtBody* parser::parse_body(ast::StmtBody* body)
{
	if (lex.current_token() != TKN_BRACKET_OPEN)
		return nullptr;

	lex.eat();

	auto curr_body = ast::StmtBody::create();

	while (!lex.eof())
	{
		if (auto new_body = parse_body(curr_body))
			curr_body->stmts.push_back(new_body);

		if (auto stmt = parse_statement())
		{
			curr_body->stmts.push_back(stmt);

			lex.eat();
		}
		else break;
	}

	if (lex.eat_expect(TKN_BRACKET_CLOSE))
		return curr_body;
	
	parser_error("Expected a '}', got '%s'", lex.eof() ? "EOF" : lex.current_value().c_str());
	
	return nullptr;
}

ast::Stmt* parser::parse_statement()
{
	auto current = lex.current();

	// is it a declaration

	if (lex.is_token_keyword_decl(current))
	{
		printf_s("we got a declaration '%s'\n", current.value.c_str());

		lex.eat();

		auto id = lex.eat_expect(TKN_ID);

		// is it an identifier

		if (id)
		{
			printf_s("we got an id '%s'\n", id.value().value.c_str());

			auto next = lex.current();

			switch (next.id)
			{
			case TKN_ASSIGN:
			{
				printf_s("declaration with assignment\n");
				parse_expression();
				return nullptr;
			}
			}

			return ast::Stmt::create(id.value().value, current.id);
		}
		else printf_s("[%s] SYNTAX ERROR: Expected an identifier\n", __FUNCTION__);
	}

	return nullptr;
}

ast::Expr* parser::parse_expression()
{
	printf_s("parsing expression now...\n");
	return nullptr;
}

void parser::parse_literal()
{

}