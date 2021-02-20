#include <defs.h>

#include "lexer.h"

#include "parser.h"

using namespace kpp;

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
				auto prototype = ast::Prototype::create(id->value);

				prototype->stmts = parse_prototype_params_decl();

				if (auto paren_close = lex.eat_expect(TOKEN_PAREN_CLOSE))
				{
					if (prototype->body = parse_body(nullptr))
						return prototype;
					else printf_s("Failed parsing main prototype body\n");
				}

				delete prototype;
			}
		}
		else printf_s("[%s] SYNTAX ERROR: Expected function id\n", __FUNCTION__);
	}
	else printf_s("[%s] SYNTAX ERROR: Expected a return type\n", __FUNCTION__);

	return nullptr;
}

std::vector<ast::StmtBase*> parser::parse_prototype_params_decl()
{
	std::vector<ast::StmtBase*> stmts;

	while (!lex.eof())
	{
		auto param_type = parse_type();
		if (!param_type)
			break;

		auto param_id = parse_id();
		if (!param_id)
			break;

		stmts.push_back(ast::StmtDecl::create(param_id->value, param_type->id));

		if (!lex.is_current(TOKEN_COMMA))
			break;

		lex.eat();
	}

	return stmts;
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
		{
			curr_body->stmts.push_back(stmt);

			if (lex.is_current(TOKEN_SEMICOLON))
				lex.eat();
		}
		else break;
	}

	if (lex.eat_expect(TOKEN_BRACKET_CLOSE))
		return curr_body;
	
	parser_error("Expected a '}', got '%s'", lex.eof() ? "EOF" : lex.current_value().c_str());
	
	return nullptr;
}

ast::StmtBase* parser::parse_statement()
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
				lex.eat();
				return ast::StmtAssign::create(id->value, parse_expression());
			}
			}

			return ast::StmtDecl::create(id->value, type->id);
		}
		else printf_s("[%s] SYNTAX ERROR: Expected an identifier\n", __FUNCTION__);
	}
	
	return nullptr;
}

ast::Expr* parser::parse_expression()
{
	return parse_expression_precedence(parse_primary_expression());
}

ast::Expr* parser::parse_expression_precedence(ast::Expr* lhs, int min_precedence)
{
	if (!lhs)
		return nullptr;

	auto lookahead = lex.current();

	while (lookahead.precedence <= min_precedence)
	{
		auto op = lookahead;

		lex.eat();

		auto rhs = parse_primary_expression();
		if (!rhs)
			break;

		lookahead = lex.current();

		// no right associative operators yet

		while (lookahead.precedence < op.precedence)
		{
			rhs = parse_expression_precedence(rhs, lookahead.precedence);
			lookahead = lex.current();
		}

		lhs = ast::BinaryOp::create(lhs, op.id, rhs);
	}

	return lhs;
}

ast::Expr* parser::parse_primary_expression()
{
	if (auto curr = lex.current(); lex.is(curr, TOKEN_INT_LITERAL))
	{
		lex.eat();

		return ast::Expr::create(curr.value);
	}
	else if (lex.is(curr, TOKEN_ID))
	{
		lex.eat();

		if (lex.is_current(TOKEN_PAREN_OPEN))
		{
			lex.eat();

			lex.eat_expect(TOKEN_PAREN_CLOSE);
		}

		return ast::Expr::create(curr.value);
	}

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