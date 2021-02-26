#pragma once

#include "ast.h"

namespace kpp
{
	namespace ir
	{
		enum IrType
		{
			IR_NONE,
			IR_BODY,
			IR_EXPR,
		};

		enum IrExprType
		{
			IR_EXPR_NONE,
			IR_EXPR_INT_LITERAL,
			IR_EXPR_DECL_OR_ASSIGN,
			IR_EXPR_BINARY_OP,
			IR_EXPR_CALL,
		};

		struct Base
		{
			IrType ir_type = IR_NONE;
		};

		struct Expr : public Base
		{
			IrExprType ir_expr_type = IR_EXPR_NONE;

			Expr() { ir_type = IR_EXPR; }

			virtual std::string get_name() = 0;
		};

		struct ExprIntLiteral : public Expr
		{
			Int value;

			Token type;

			ExprIntLiteral(Int value, Token type) : value(value), type(type)	{ ir_expr_type = IR_EXPR_INT_LITERAL; }

			static ExprIntLiteral* create(Int value, Token type)				{ return new ExprIntLiteral(value, type); }

			std::string get_name() override										{ return {}; }
		};

		struct ExprDeclOrAssign : public Expr
		{
			std::string name;

			Expr* value = nullptr;

			Token type = TOKEN_NONE;

			ExprDeclOrAssign()					{ ir_expr_type = IR_EXPR_DECL_OR_ASSIGN; }
			
			static ExprDeclOrAssign* create()	{ return new ExprDeclOrAssign(); }

			bool is_declaration() const			{ return (type != TOKEN_NONE); }
			
			std::string get_name() override		{ return name; }
		};

		struct ExprBinaryOp : public Expr
		{
			Expr* left = nullptr;
			Token op = TOKEN_NONE;
			Expr* right = nullptr;

			ExprBinaryOp(Expr* left, Token op, Expr* right) : left(left), op(op), right(right)	{ ir_expr_type = IR_EXPR_BINARY_OP; }

			static ExprBinaryOp* create(Expr* left, Token op, Expr* right)						{ return new ExprBinaryOp(left, op, right); }

			std::string get_name() override														{ return STRINGIFY_TOKEN(op); }
		};
		
		struct IrBody : public Base
		{
			std::vector<Base*> items;

			IrBody() { ir_type = IR_BODY; }
		};

		struct PrototypeParam
		{
			std::string name;

			Token type = TOKEN_NONE;

			PrototypeParam(const std::string& name, Token type) : name(name), type(type) {}
		};

		struct Prototype
		{
			std::vector<PrototypeParam*> params;

			std::string name;

			IrBody* body = nullptr;

			Token return_type = TOKEN_NONE;
		};

		struct IR
		{
			std::vector<Prototype*> prototypes;
		};

		struct GlobalInfo
		{
			std::unordered_map<std::string, Prototype*> prototypes;
		};

		struct PrototypeInfo
		{
			Prototype* curr_prototype = nullptr;

			std::unordered_map<std::string, Expr*> vars;

			void clear()
			{
				vars.clear();
			}
		};
	}

	inline std::string STRINGIFY_OP_IR(Token op)
	{
		switch (op)
		{
		case TOKEN_ADD: return "add";
		case TOKEN_SUB: return "sub";
		case TOKEN_MUL: return "mul";
		case TOKEN_DIV: return "div";
		case TOKEN_MOD: return "mod";
		}

		return "unknown";
	}

	class ir_parser
	{
	private:

		ast::AST* tree = nullptr;

		ir::IR iri {};

		ir::GlobalInfo gi {};

		ir::PrototypeInfo pi {};

		int print_level = 0;

	public:

		ir_parser(ast::AST* tree);
		~ir_parser();

		void print_ir();
		void print_body(ir::IrBody* body);
		void print_expr(ir::Expr* expr_base);
		void print_expr_int_literal(ir::ExprIntLiteral* expr);

		void add_prototype(ir::Prototype* prototype);
		void add_var(const std::string& name, ir::Expr* expr);

		ir::Expr* get_declared_var(const std::string& name);

		bool generate();

		ir::Prototype* generate_prototype(ast::Prototype* prototype);
		ir::IrBody* generate_body(ast::StmtBody* body);
		ir::Expr* generate_expr(ast::Expr* expr);
		ir::ExprDeclOrAssign* generate_expr_decl_or_assign(ast::ExprDeclOrAssign* expr);
		ir::ExprIntLiteral* generate_expr_int_literal(ast::ExprIntLiteral* expr);
		ir::ExprBinaryOp* generate_expr_binary_op(ast::ExprBinaryOp* expr);

		ir::Prototype* get_defined_prototype(ast::Prototype* prototype);

		ast::Prototype* get_prototype_definition(ast::Prototype* prototype_decl);
	};
}