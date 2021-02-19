#include <defs.h>

#include "lexer.h"

#include "parser.h"

using namespace kpp;

void ast::Printer::print_body(ast::StmtBody* body)
{
	++curr_level;

	PRINT_TABS_NL(C_CYAN, curr_level, "Body");

	for (auto&& stmt_base : body->stmts)
	{
		if (stmt_base->is_body)
		{
			print_body((ast::StmtBody*)stmt_base);
		}
		else
		{
			auto stmt = (ast::Stmt*)stmt_base;
			PRINT_TABS_NL(C_YELLOW, curr_level + 1, "Statement '%s' (%s)", stmt->name.c_str(), STRINGIFY_TOKEN(stmt->type).c_str());
		}
	}

	PRINT_TABS_NL(C_CYAN, curr_level, "End");

	--curr_level;
}

void ast::Printer::print_prototype(ast::Prototype* prototype)
{
	if (!prototype->body)
		return;

	PRINT_TABS(C_WHITE, 0, "Prototype '%s'", prototype->name.c_str());

	if (!prototype->stmts.empty())
	{
		PRINT_TABS(C_WHITE, 0, " | Arguments: ");

		dbg::print_str_vec(C_GREEN, prototype->stmts, ", ", [](ast::StmtBase* stmt_base) { return ((ast::StmtDecl*)stmt_base)->name; });
	}

	PRINT_NL;

	print_body(prototype->body);

	PRINT_TABS_NL(C_WHITE, curr_level, "End\n");
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
		if (auto id = lex.eat_expect(TOKEN_ID))
		{
			if (auto paren_open = lex.eat_expect(TOKEN_PAREN_OPEN))
			{
				auto prototype = new ast::Prototype(id->value);

				while (!lex.eof())
				{
					auto param_type = parse_type();
					if (!param_type)
						break;

					auto param_id = parse_id();
					if (!param_id)
						break;

					prototype->stmts.push_back(ast::StmtDecl::create(param_id->value, param_type->id));

					if (!lex.is_current(TOKEN_COMMA))
						break;

					lex.eat();
				}

				if (auto paren_close = lex.eat_expect(TOKEN_PAREN_CLOSE))
				{
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
	if (!lex.is_current(TOKEN_BRACKET_OPEN))
		return nullptr;

	lex.eat();

	auto curr_body = ast::StmtBody::create();

	while (!lex.eof())
	{
		if (auto new_body = parse_body(curr_body))
			curr_body->stmts.push_back(new_body);

		if (auto stmt = parse_statement())
			curr_body->stmts.push_back(stmt);
		else break;
	}

	if (lex.eat_expect(TOKEN_BRACKET_CLOSE))
		return curr_body;
	
	parser_error("Expected a '}', got '%s'", lex.eof() ? "EOF" : lex.current_value().c_str());
	
	return nullptr;
}

ast::Stmt* parser::parse_statement()
{
	if (auto type = parse_type())
	{
		if (auto id = parse_id())
		{
			auto next = lex.current();

			switch (next.id)
			{
			case TOKEN_ASSIGN:
			{
				printf_s("declaration with assignment\n");
				parse_expression();
				return nullptr;
			}
			case TOKEN_SEMICOLON:
			{
				lex.eat();
				break;
			}
			}

			return ast::Stmt::create(id->value, type->id);
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

opt_token_info parser::parse_type()
{
	return (lex.is_token_keyword_type() ? lex.eat() : opt_token_info {});
}

opt_token_info parser::parse_id()
{
	return (lex.is_current(TOKEN_ID) ? lex.eat() : opt_token_info {});
}