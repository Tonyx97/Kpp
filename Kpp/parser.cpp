#include <defs.h>

#include "lexer.h"

#include "parser.h"

using namespace kpp;

void ast::Printer::print_body(ast::StmtBody* body)
{
	for (auto&& stmt_base : body->stmts)
	{
		if (stmt_base->is_body)
		{
			printf_s("begin body\n");
			print_body((ast::StmtBody*)stmt_base);
		}
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

	printf_s("begin body\n");

	print_body(prototype->body);
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

					if (auto prototype = new ast::Prototype(prototype_name); prototype->body = parse_body())
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

ast::StmtBody* parser::parse_body()
{
	auto bracket_open = lex.eat_expect(TKN_BRACKET_OPEN);
	if (!bracket_open)
		return nullptr;
	
	auto main_body = new ast::StmtBody(),
		 curr_body = main_body;

	while (!lex.eof())
	{
		ast::StmtBody* sub_body = nullptr;

		if (lex.current_token() == TKN_BRACKET_OPEN)
		{
			lex.eat();

			if (lex.current_token() != TKN_BRACKET_CLOSE)
			{
				auto new_body = ast::StmtBody::create();

				if (sub_body)
				{
					sub_body->stmts.push_back(new_body);
				}
				else
				{
					curr_body->stmts.push_back(new_body);
				}

				curr_body = sub_body = new_body;
			}
		}
		
		if (auto stmt = parse_statement())
		{
			curr_body->stmts.push_back(stmt);

			lex.eat();
		}
		else break;

		if (sub_body)
		{
			if (lex.current_token() == TKN_BRACKET_CLOSE)
			{
				lex.eat();

				curr_body = sub_body;
			}
			else printf_s("Failed parsing prototype body\n");
		}
	}

	return (lex.eat_expect(TKN_BRACKET_CLOSE) ? main_body : nullptr);
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