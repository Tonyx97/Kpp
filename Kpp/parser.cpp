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
	if (auto ret_ty = lex.eat_expect_keyword_declaration())
	{
		if (auto id = lex.eat_expect(TOKEN_ID))
		{
			if (auto paren_open = lex.eat_expect(TOKEN_PAREN_OPEN))
			{
				auto prototype = new ast::Prototype(id->value);

				prototype->params = parse_prototype_params_decl();
				prototype->ret_ty = ret_ty->id;

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

std::vector<ast::Base*> parser::parse_prototype_params_decl()
{
	std::vector<ast::Base*> stmts;

	while (!lex.eof())
	{
		auto param_type = parse_type();
		if (!param_type)
			break;

		auto param_id = parse_id();
		if (!param_id)
			break;

		stmts.push_back(new ast::ExprDeclOrAssign(param_id->value, nullptr, param_type->id));

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

	auto curr_body = new ast::StmtBody();

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

ast::Base* parser::parse_statement()
{
	if (auto type = parse_type())
	{
		if (auto id = parse_id())
		{
			if (lex.is_current(TOKEN_ASSIGN))
			{
				lex.eat();
				return new ast::ExprDeclOrAssign(id->value, parse_expression(), type->id);
			}

			return new ast::ExprDeclOrAssign(id->value, nullptr, type->id);
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

			auto if_stmt = new ast::StmtIf(if_expr, parse_body(nullptr));

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

						if_stmt->ifs.push_back(new ast::StmtIf(else_if_expr, parse_body(nullptr)));

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

			return new ast::StmtFor(condition, init, step, parse_body(nullptr));
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

		lhs = new ast::ExprBinaryOp(lhs, op.id, TOKEN_I32, rhs);	// we have to check for the real type
	}

	return lhs;
}

ast::Expr* parser::parse_primary_expression()
{
	auto curr = lex.current();

	if (lex.is_current(TOKEN_INT_LITERAL))
	{
		lex.eat();

		return new ast::ExprIntLiteral(Int::create(std::stoull(curr.value)), TOKEN_I32);	// we have to check for the real type
	}
	else if (lex.is_current(TOKEN_SUB) ||
			 lex.is_current(TOKEN_MUL) ||
			 lex.is_current(TOKEN_AND) ||
			 lex.is_current(TOKEN_NOT))
	{
		lex.eat();

		return new ast::ExprUnaryOp(curr.id, parse_primary_expression());
	}
	else if (lex.is_current(TOKEN_ID))
	{
		auto id = lex.eat();

		if (lex.is_current(TOKEN_ASSIGN))
		{
			lex.eat();

			return new ast::ExprDeclOrAssign(id.value, parse_expression());
		}
		else if (lex.is_current(TOKEN_PAREN_OPEN))
		{
			lex.eat();

			auto call = new ast::ExprCall(curr.value);

			call->stmts = parse_call_params();

			lex.eat_expect(TOKEN_PAREN_CLOSE);

			return call;
		}

		return new ast::ExprId(curr.value);
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