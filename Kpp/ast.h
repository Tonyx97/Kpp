#pragma once

namespace kpp
{
	namespace ast
	{
		enum StmtType
		{
			STMT_NONE,
			STMT_BODY,
			STMT_DECL,
			STMT_ASSIGN,
			STMT_IF,
			STMT_FOR,
		};

		enum ExprType
		{
			EXPR_NONE,
			EXPR_INT,
			EXPR_BINARY_OP,
		};

		struct StmtBase
		{
			StmtType base_type = STMT_NONE;
		};

		struct Expr
		{
			std::string value;

			ExprType base_type = EXPR_NONE;

			Expr() {}
			Expr(const std::string& value) : value(value) { base_type = EXPR_INT; }

			static Expr* create(const std::string& value) { return new Expr(value); }
		};

		struct StmtDecl : public StmtBase
		{
			std::string name;
			Token type = TOKEN_NONE;

			StmtDecl(const std::string& name, Token type) : name(name), type(type)	{ base_type = STMT_DECL; }

			static StmtDecl* create(const std::string& name, Token type)			{ return new StmtDecl(name, type); }
		};

		struct StmtAssign : public StmtBase
		{
			std::string name;

			Expr* value = nullptr;

			Token type = TOKEN_NONE;

			StmtAssign(const std::string& name, Expr* value, Token type) : name(name), value(value), type(type)	{ base_type = STMT_ASSIGN; }

			static StmtAssign* create(const std::string& name, Expr* value, Token type)							{ return new StmtAssign(name, value, type); }
		};

		struct StmtBody : public StmtBase
		{
			std::vector<StmtBase*> stmts;

			StmtBody()					{ base_type = STMT_BODY; }

			static StmtBody* create()	{ return new StmtBody(); }
		};

		struct StmtIf : public StmtBase
		{
			Expr* expr = nullptr;

			std::vector<StmtIf*> ifs;

			StmtBody* if_body = nullptr,
					* else_body = nullptr;

			StmtIf(Expr* expr, StmtBody* if_body) : expr(expr), if_body(if_body)	{ base_type = STMT_IF; }

			static StmtIf* create(Expr* expr, StmtBody* if_body)					{ return new StmtIf(expr, if_body); }
		};

		struct StmtFor : public StmtBase
		{
			Expr* condition = nullptr;

			StmtBase* init = nullptr,
					* step = nullptr;

			StmtBody* body = nullptr;

			StmtFor(Expr* condition, StmtBase* init, StmtBase* step, StmtBody* body)
					: condition(condition), init(init), step(step), body(body)						{ base_type = STMT_FOR; }

			static StmtFor* create(Expr* condition, StmtBase* init, StmtBase* step, StmtBody* body)	{ return new StmtFor(condition, init, step, body); }
		};

		struct BinaryOp : public Expr
		{
			Expr* left = nullptr;
			Token op = TOKEN_NONE;
			Expr* right = nullptr;

			BinaryOp(Expr* left, Token op, Expr* right) : left(left), op(op), right(right)	{ base_type = EXPR_BINARY_OP; value = STRINGIFY_TOKEN(op); }

			static BinaryOp* create(Expr* left, Token op, Expr* right)						{ return new BinaryOp(left, op, right); }
		};

		struct Prototype
		{
			std::vector<StmtBase*> stmts;

			std::string name;

			StmtBody* body = nullptr;

			Prototype(const std::string& name) : name(name)		{}
			
			static Prototype* create(const std::string& name)	{ return new Prototype(name); }
		};

		struct Printer
		{
			int curr_level = 0;

			bool first_prototype_printed = false;

			void print_expr_binary_op(ast::BinaryOp* expr);
			void print_expr_int(ast::Expr* expr);
			void print_expr(ast::Expr* expr);
			void print_for(ast::StmtFor* stmt_for);
			void print_if(ast::StmtIf* stmt_if);
			void print_assign(ast::StmtAssign* assign);
			void print_decl(ast::StmtDecl* decl);
			void print_stmt(ast::StmtBase* stmt);
			void print_body(ast::StmtBody* body);
			void print_prototype(Prototype* prototype);
			void print(const std::vector<Prototype*>& prototypes);
		};
	}
}