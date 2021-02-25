#include <defs.h>

#include "parser.h"

using namespace kpp;

parser::parser(lexer& lex) : lex(lex)
{
	tree = new ast::AST();
}

parser::~parser()
{
	delete tree;
}

void parser::print_ast()
{
	ast::Printer().print(tree);
}

bool parser::parse()
{
	while (!lex.eof())
	{
		if (auto prototype = parse_prototype())
			tree->prototypes.push_back(prototype);
	}

	return true;
}

ast::Prototype* parser::parse_prototype()
{
	if (auto return_type = lex.eat_expect_keyword_declaration())
	{
		if (auto id = lex.eat_expect(TOKEN_ID))
		{
			if (auto paren_open = lex.eat_expect(TOKEN_PAREN_OPEN))
			{
				auto prototype = ast::Prototype::create(id->value);

				prototype->params = parse_prototype_params_decl();
				prototype->return_type = return_type->id;

				if (auto paren_close = lex.eat_expect(TOKEN_PAREN_CLOSE))
				{
					if (!lex.is_current(TOKEN_SEMICOLON))
					{
						if (prototype->body = parse_body(nullptr))
							return prototype;
						else printf_s("Failed parsing main prototype body\n");
					}
					else
					{
						lex.eat();

						prototype->declaration = true;

						return prototype;
					}
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

		stmts.push_back(ast::ExprDeclOrAssign::create(param_id->value, nullptr, param_type->id));

		if (!lex.is_current(TOKEN_COMMA))
			break;

		lex.eat();
	}

	return stmts;
}

std::vector<ast::Expr*> parser::parse_call_params()
{
	std::vector<ast::Expr*> stmts;

	while (!lex.eof() && !lex.is_current(TOKEN_PAREN_CLOSE))
	{
		auto param_value = parse_expression();
		if (!param_value)
			break;

		stmts.push_back(param_value);

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
			if (lex.is_current(TOKEN_ASSIGN))
			{
				lex.eat();
				return ast::ExprDeclOrAssign::create(id->value, parse_expression(), type->id);
			}

			return ast::ExprDeclOrAssign::create(id->value, nullptr, type->id);
		}
		else printf_s("[%s] SYNTAX ERROR: Expected an identifier\n", __FUNCTION__);
	}
	else if (type = parse_keyword())
	{
		if (lex.is(*type, TOKEN_IF))
		{
			lex.eat_expect(TOKEN_PAREN_OPEN);

			auto if_expr = parse_expression();

			lex.eat_expect(TOKEN_PAREN_CLOSE);

			auto if_stmt = ast::StmtIf::create(if_expr, parse_body(nullptr));

			if (lex.is_current(TOKEN_ELSE))
			{
				if (lex.is_next(TOKEN_IF))
				{
					do {
						lex.eat();
						lex.eat();

						lex.eat_expect(TOKEN_PAREN_OPEN);

						auto else_if_expr = parse_expression();

						lex.eat_expect(TOKEN_PAREN_CLOSE);

						if_stmt->ifs.push_back(ast::StmtIf::create(else_if_expr, parse_body(nullptr)));

					} while (!lex.eof() && lex.is_current(TOKEN_ELSE) && lex.is_next(TOKEN_IF));
				}

				if (lex.is_current(TOKEN_ELSE))
				{
					lex.eat();

					if_stmt->else_body = parse_body(nullptr);
				}
			}

			return if_stmt;
		}
		else if (lex.is(*type, TOKEN_FOR))
		{
			lex.eat_expect(TOKEN_PAREN_OPEN);
			
			auto init		= lex.is_current(TOKEN_SEMICOLON)	? nullptr : parse_statement();	lex.eat_expect(TOKEN_SEMICOLON);
			auto condition	= lex.is_current(TOKEN_SEMICOLON)	? nullptr : parse_expression();	lex.eat_expect(TOKEN_SEMICOLON);
			auto step		= lex.is_current(TOKEN_PAREN_CLOSE) ? nullptr : parse_statement();	lex.eat_expect(TOKEN_PAREN_CLOSE);

			return ast::StmtFor::create(condition, init, step, parse_body(nullptr));
		}
	}
	
	return parse_expression();
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

	while (lex.is_token_operator(lookahead) && lookahead.precedence <= min_precedence)
	{
		auto op = lookahead;

		lex.eat();

		auto rhs = parse_primary_expression();

		lookahead = lex.current();

		// no right associative operators yet

		while (lex.is_token_operator(lookahead) && lookahead.precedence < op.precedence)
		{
			rhs = parse_expression_precedence(rhs, lookahead.precedence);
			lookahead = lex.current();
		}

		lhs = ast::ExprBinaryOp::create(lhs, op.id, rhs);
	}

	return lhs;
}

ast::Expr* parser::parse_primary_expression()
{
	if (auto curr = lex.current(); lex.is(curr, TOKEN_INT_LITERAL))
	{
		lex.eat();

		return ast::ExprIntLiteral::create(ast::Int::create(std::stoll(curr.value)), TOKEN_I32);
	}
	else if (lex.is(curr, TOKEN_ID))
	{
		auto id = lex.eat();

		if (lex.is_current(TOKEN_ASSIGN))
		{
			lex.eat();

			return ast::ExprDeclOrAssign::create(id.value, parse_expression());
		}
		else if (lex.is_current(TOKEN_PAREN_OPEN))
		{
			lex.eat();

			auto call = ast::ExprCall::create(curr.value);

			call->stmts = parse_call_params();

			lex.eat_expect(TOKEN_PAREN_CLOSE);

			return call;
		}

		return ast::ExprId::create(curr.value);
	}

	return nullptr;
}

opt_token_info parser::parse_type()
{
	return (lex.is_token_keyword_type() ? lex.eat() : opt_token_info {});
}

opt_token_info parser::parse_keyword()
{
	return (lex.is_token_keyword() ? lex.eat() : opt_token_info {});
}

opt_token_info parser::parse_id()
{
	return (lex.is_current(TOKEN_ID) ? lex.eat() : opt_token_info {});
}