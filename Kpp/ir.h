#pragma once

#include "ast.h"

namespace kpp
{
	namespace ir
	{
		struct Base
		{
			virtual void print() = 0;
			virtual Token get_type() = 0;
			virtual std::string get_value() = 0;
		};
		
		struct Body : public Base
		{
			Body()								{}

			void print() override				{}

			Token get_type() override			{ return TOKEN_NONE; };

			std::string get_value() override	{ return {}; }
		};

		struct ValueInt : public Base
		{
			std::string name;

			Int value;

			Token ty = TOKEN_NONE;

			void print() override;
			
			Token get_type() override			{ return ty; };

			std::string get_value() override	{ return std::to_string(value.u64); }
		};

		struct BinaryOp : public Base
		{
			std::string value;
			
			Base* left = nullptr;

			Token op = TOKEN_NONE,
				  ty = TOKEN_NONE;

			Base* right = nullptr;

			void print() override;

			Token get_type() override			{ return ty; };

			std::string get_value() override	{ return value; }
		};

		struct StackAlloc : public Base
		{
			std::string value;

			Token ty;

			void print() override;
			
			Token get_type() override			{ return ty; };

			std::string get_value() override	{ return value; }
		};

		struct Store : public Base
		{
			std::string value;

			Token ty = TOKEN_NONE;

			Base* operand = nullptr;

			void print() override;
			
			Token get_type() override			{ return ty; };

			std::string get_value() override	{ return value; }
		};

		struct PrototypeParam
		{
			std::string name;

			Token type = TOKEN_NONE;

			PrototypeParam(const std::string& name, Token type) : name(name), type(type) {}
		};

		struct Prototype
		{
			std::unordered_map<std::string, Base*> values;
			std::unordered_map<std::string, std::string> values_lookup;

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

			std::unordered_map<std::string, Base*> values;
			std::unordered_map<std::string, std::string> values_lookup;

			std::vector<Base*> items;

			size_t stack_size = 0,
				   aligned_stack_size = 0;

			void clear()
			{
				curr_prototype = nullptr;

				stack_size = aligned_stack_size = 0;

				values.clear();
				values_lookup.clear();
				items.clear();
			}

			void copy_to_prototype(Prototype* prototype)
			{
				prototype->values = values;
				prototype->values_lookup = values_lookup;
				prototype->items = items;
				prototype->stack_size = stack_size;
				prototype->aligned_stack_size = aligned_stack_size;
			}

			void create_item(Base* base)
			{
				items.push_back(base);
			}

			std::optional<std::string> get_value_from_name(const std::string& name)
			{
				if (auto it = values_lookup.find(name); it != values_lookup.end())
					return it->second;
				return {};
			}

			std::string create_value(const std::string& name, ir::Base* item)
			{
				auto var_name = "v" + std::to_string(values.size());
				values.insert({ var_name, item });
				values_lookup.insert({ name, var_name });
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

	class ir_gen
	{
	private:

		ast::AST* tree = nullptr;

		ir::IR iri {};

		ir::GlobalInfo gi {};

		ir::PrototypeInfo pi {};

		int print_level = 0;

	public:

		ir_gen(ast::AST* tree);
		~ir_gen();

		void print_ir();
		void print_prototype(ir::Prototype* prototype);
		void print_item(ir::Base* base);

		void add_prototype(ir::Prototype* prototype);

		bool generate();

		ir::Prototype* generate_prototype(ast::Prototype* prototype);
		ir::Body* generate_from_body(ast::StmtBody* body);

		ir::Base* generate_from_expr(ast::Expr* expr);
		ir::Base* generate_from_expr_decl_or_assign(ast::ExprDeclOrAssign* expr);
		ir::ValueInt* generate_from_expr_int_literal(ast::ExprIntLiteral* expr);
		ir::BinaryOp* generate_from_expr_binary_op(ast::ExprBinaryOp* expr);

		/*
		ir::ExprId* generate_expr_id(ast::ExprId* expr);*/

		ir::Prototype* get_defined_prototype(ast::Prototype* prototype);

		ast::Prototype* get_prototype_definition(ast::Prototype* prototype_decl);
	};
}