#pragma once

namespace kpp
{
	namespace ast
	{
		enum ExprType
		{
			EXPR_INT,
			EXPR_BINARY_OP,
		};

		enum StmtType
		{
			STMT_BODY,
			STMT_DECL,
			STMT_ASSIGN,
		};
		
		struct Expr
		{
			std::string value;

			ExprType base_type;

			Expr() {}
			Expr(const std::string& value) : value(value) { base_type = EXPR_INT; }

			static Expr* create(const std::string& value) { return new Expr(value); }
		};

		struct BinaryOp : public Expr
		{
			Expr* left;
			Token op;
			Expr* right;

			BinaryOp(Expr* left, Token op, Expr* right) : left(left), op(op), right(right)	{ base_type = EXPR_BINARY_OP; value = STRINGIFY_TOKEN(op); }

			static BinaryOp* create(Expr* left, Token op, Expr* right)						{ return new BinaryOp(left, op, right); }
		};

		struct StmtBase
		{
			StmtType base_type;
		};

		struct StmtDecl : public StmtBase
		{
			std::string name;
			Token type;

			StmtDecl(const std::string& name, Token type) : name(name), type(type)	{ base_type = STMT_DECL; }

			static StmtDecl* create(const std::string& name, Token type)			{ return new StmtDecl(name, type); }
		};

		struct StmtAssign : public StmtBase
		{
			std::string name;
			Expr* value;

			StmtAssign(const std::string& name, Expr* value) : name(name), value(value)	{ base_type = STMT_ASSIGN; }

			static StmtAssign* create(const std::string& name, Expr* value)				{ return new StmtAssign(name, value); }
		};

		struct StmtBody : public StmtBase
		{
			std::vector<StmtBase*> stmts;

			StmtBody()					{ base_type = STMT_BODY; }

			static StmtBody* create()	{ return new StmtBody(); }
		};

		struct Prototype
		{
			std::vector<StmtBase*> stmts;

			std::string name;

			StmtBody* body;

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
			void print_assign(ast::StmtAssign* assign);
			void print_decl(ast::StmtDecl* decl);
			void print_body(ast::StmtBody* body);
			void print_prototype(Prototype* prototype);
			void print(const std::vector<Prototype*>& prototypes);
		};
	}
}