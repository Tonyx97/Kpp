#pragma once

#include "lexer.h"

namespace kpp
{
	namespace ast
	{
		enum StmtType
		{
			STMT_NONE,
			STMT_EXPR,
			STMT_BODY,
			STMT_IF,
			STMT_FOR,
		};

		enum ExprType
		{
			EXPR_NONE,
			EXPR_INT,
			EXPR_DECL_OR_ASSIGN,
			EXPR_BINARY_OP,
			EXPR_CALL,
		};

		struct StmtBase
		{
			StmtType stmt_type = STMT_NONE;
		};

		struct Expr : public StmtBase
		{
			std::string value;

			ExprType expr_type = EXPR_NONE;

			Token type = TOKEN_NONE;

			Expr()																	{ stmt_type = STMT_EXPR; }
			Expr(const std::string& value, Token type) : value(value), type(type)	{ stmt_type = STMT_EXPR; expr_type = EXPR_INT; }	// this could be any literal pls

			static Expr* create(const std::string& value, Token type)				{ return new Expr(value, type); }
		};

		struct StmtBody : public StmtBase
		{
			std::vector<StmtBase*> stmts;

			StmtBody()					{ stmt_type = STMT_BODY; }

			static StmtBody* create()	{ return new StmtBody(); }
		};

		struct StmtIf : public StmtBase
		{
			Expr* expr = nullptr;

			std::vector<StmtIf*> ifs;

			StmtBody* if_body = nullptr,
					* else_body = nullptr;

			StmtIf(Expr* expr, StmtBody* if_body) : expr(expr), if_body(if_body)	{ stmt_type = STMT_IF; }

			static StmtIf* create(Expr* expr, StmtBody* if_body)					{ return new StmtIf(expr, if_body); }
		};

		struct StmtFor : public StmtBase
		{
			Expr* condition = nullptr;

			StmtBase* init = nullptr,
					* step = nullptr;

			StmtBody* body = nullptr;

			StmtFor(Expr* condition, StmtBase* init, StmtBase* step, StmtBody* body)
					: condition(condition), init(init), step(step), body(body)						{ stmt_type = STMT_FOR; }

			static StmtFor* create(Expr* condition, StmtBase* init, StmtBase* step, StmtBody* body)	{ return new StmtFor(condition, init, step, body); }
		};

		struct ExprDeclOrAssign : public Expr
		{
			std::string name;

			Expr* value = nullptr;

			Token type = TOKEN_NONE;

			ExprDeclOrAssign(const std::string& name, Expr* value, Token type) : name(name), value(value), type(type)	{ expr_type = EXPR_DECL_OR_ASSIGN; }

			bool is_declaration() const																					{ return (type != TOKEN_NONE); }

			static ExprDeclOrAssign* create(const std::string& name, Expr* value = nullptr, Token type = TOKEN_NONE)	{ return new ExprDeclOrAssign(name, value, type); }
		};

		struct ExprBinaryOp : public Expr
		{
			Expr* left = nullptr;
			Token op = TOKEN_NONE;
			Expr* right = nullptr;

			ExprBinaryOp(Expr* left, Token op, Expr* right) : left(left), op(op), right(right)	{ expr_type = EXPR_BINARY_OP; value = STRINGIFY_TOKEN(op); }

			static ExprBinaryOp* create(Expr* left, Token op, Expr* right)						{ return new ExprBinaryOp(left, op, right); }
		};

		struct ExprCall : public Expr
		{
			std::string name;
			
			std::vector<Expr*> stmts;

			ExprCall(const std::string& name) : name(name)		{ expr_type = EXPR_CALL; }

			static ExprCall* create(const std::string& name)	{ return new ExprCall(name); }
		};

		struct Prototype
		{
			std::vector<StmtBase*> params;

			std::string name;

			StmtBody* body = nullptr;

			Token return_type = TOKEN_NONE;

			bool declaration = false;

			Prototype(const std::string& name) : name(name)		{}
			
			static Prototype* create(const std::string& name)	{ return new Prototype(name); }
		};

		struct AST
		{
			std::vector<Prototype*> prototypes;

			~AST()
			{
				// we need to free the whole ast tree lol
			}
		};

		struct Printer
		{
			int curr_level = 0;

			bool first_prototype_printed = false;

			void print(AST* tree);
			void print_prototype(Prototype* prototype);
			void print_body(ast::StmtBody* body);
			void print_stmt(ast::StmtBase* stmt);
			void print_if(ast::StmtIf* stmt_if);
			void print_for(ast::StmtFor* stmt_for);
			void print_expr(ast::Expr* expr);
			void print_decl_or_assign(ast::ExprDeclOrAssign* assign);
			void print_expr_int(ast::Expr* expr);
			void print_expr_binary_op(ast::ExprBinaryOp* expr);
			void print_expr_call(ast::ExprCall* expr);
		};
	}
}