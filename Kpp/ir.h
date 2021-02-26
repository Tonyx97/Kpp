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
			IR_EXPR_ID,
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
			std::string var_name;
			
			IrExprType ir_expr_type = IR_EXPR_NONE;

			Expr() { ir_type = IR_EXPR; }

			virtual std::string get_value() = 0;
		};

		struct ExprIntLiteral : public Expr
		{
			Int value;

			Token type;

			ExprIntLiteral(Int value, Token type) : value(value), type(type)	{ ir_expr_type = IR_EXPR_INT_LITERAL; }

			static ExprIntLiteral* create(Int value, Token type)				{ return new ExprIntLiteral(value, type); }

			virtual std::string get_value() override							{ return std::to_string(value.u64); }
		};

		struct ExprId : public Expr
		{
			ExprId()									{ ir_expr_type = IR_EXPR_ID; }

			static ExprId* create()						{ return new ExprId(); }

			virtual std::string get_value() override	{ return var_name; }
		};

		struct ExprDeclOrAssign : public Expr
		{
			Expr* value = nullptr;

			Token type = TOKEN_NONE;

			ExprDeclOrAssign()							{ ir_expr_type = IR_EXPR_DECL_OR_ASSIGN; }
			
			static ExprDeclOrAssign* create()			{ return new ExprDeclOrAssign(); }

			bool is_declaration() const					{ return (type != TOKEN_NONE); }

			virtual std::string get_value() override	{ return var_name; }
		};

		struct ExprBinaryOp : public Expr
		{
			Expr* left = nullptr;

			Token op = TOKEN_NONE;

			Expr* right = nullptr;

			ExprBinaryOp(Expr* left, Token op, Expr* right) : left(left), op(op), right(right)	{ ir_expr_type = IR_EXPR_BINARY_OP; }

			static ExprBinaryOp* create(Expr* left, Token op, Expr* right)						{ return new ExprBinaryOp(left, op, right); }
			
			virtual std::string get_value() override											{ return var_name; }
		};
		
		struct Body : public Base
		{
			Body() { ir_type = IR_BODY; }
		};

		struct PrototypeParam
		{
			std::string name;

			Token type = TOKEN_NONE;

			PrototypeParam(const std::string& name, Token type) : name(name), type(type) {}
		};

		struct Prototype
		{
			std::unordered_map<std::string, Expr*> vars;
			std::unordered_map<std::string, std::string> vars_lookup;

			std::vector<PrototypeParam*> params;

			std::vector<Base*> items;

			size_t stack_size = 0,
				   aligned_stack_size = 0;

			std::string name;

			Body* body = nullptr;

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
			std::unordered_map<std::string, std::string> vars_lookup;

			std::vector<Base*> items;

			size_t stack_size = 0,
				   aligned_stack_size = 0;

			void clear()
			{
				curr_prototype = nullptr;

				stack_size = aligned_stack_size = 0;

				vars.clear();
				vars_lookup.clear();
				items.clear();
			}

			void copy_to_prototype(Prototype* prototype)
			{
				prototype->vars = vars;
				prototype->vars_lookup = vars_lookup;
				prototype->items = items;
				prototype->stack_size = stack_size;
				prototype->aligned_stack_size = aligned_stack_size;
			}

			void create_item(Base* base)
			{
				items.push_back(base);
			}

			std::optional<std::string> get_var_from_name(const std::string& name)
			{
				if (auto it = vars_lookup.find(name); it != vars_lookup.end())
					return it->second;
				return {};
			}

			std::string create_var(const std::string& name, ir::Expr* expr)
			{
				auto var_name = "v" + std::to_string(vars.size());
				vars.insert({ var_name, expr });
				vars_lookup.insert({ name, var_name });
				return var_name;
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
		void print_prototype(ir::Prototype* prototype);
		void print_expr(ir::Expr* expr_base);
		void print_expr_int_literal(ir::ExprIntLiteral* expr);

		void add_prototype(ir::Prototype* prototype);

		bool generate();

		ir::Prototype* generate_prototype(ast::Prototype* prototype);
		ir::Body* generate_body(ast::StmtBody* body);

		ir::Expr* generate_expr(ast::Expr* expr);

		ir::ExprId* generate_expr_id(ast::ExprId* expr);
		ir::ExprDeclOrAssign* generate_expr_decl_or_assign(ast::ExprDeclOrAssign* expr);
		ir::ExprIntLiteral* generate_expr_int_literal(ast::ExprIntLiteral* expr);
		ir::ExprBinaryOp* generate_expr_binary_op(ast::ExprBinaryOp* expr);

		ir::Prototype* get_defined_prototype(ast::Prototype* prototype);

		ast::Prototype* get_prototype_definition(ast::Prototype* prototype_decl);
	};
}