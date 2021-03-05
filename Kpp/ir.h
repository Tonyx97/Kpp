#pragma once

#include "ast.h"

namespace kpp
{
	namespace ir
	{
		enum InsType
		{
			INS_UNKNOWN,
			INS_BODY,
			INS_VALUE_INT,
			INS_VALUE_ID,
			INS_BINARY_OP,
			INS_UNARY_OP,
			INS_STACK_ALLOC,
			INS_STORE,
			INS_LOAD,
			INS_COMPARE,
			INS_BLOCK,
		};
		
		/*
		* Instruction
		*/
		struct Instruction
		{
			InsType type = INS_UNKNOWN;
			
			virtual void print() = 0;
			virtual Token get_type() = 0;
			virtual std::string get_value() = 0;
		};

		/*
		* Body
		*/
		struct Body : public Instruction
		{
			Body()									{ type = INS_BODY; }

			void print() override					{}

			Token get_type() override				{ return TOKEN_NONE; }

			std::string get_value() override		{ return {}; }

			static bool check_class(Instruction* i) { return i->type == INS_BODY; }
		};

		/*
		* ValueInt
		*/
		struct ValueInt : public Instruction
		{
			std::string name;

			Int value;

			Token ty = TOKEN_NONE;

			ValueInt()								{ type = INS_VALUE_INT; }

			void print() override;
			
			Token get_type() override				{ return ty; }

			//std::string get_value() override		{ return name; }
			std::string get_value() override		{ return std::to_string(value.i64); }

			static bool check_class(Instruction* i) { return i->type == INS_VALUE_INT; }
		};

		/*
		* ValueId
		*/
		struct ValueId : public Instruction
		{
			std::string name;

			Token ty = TOKEN_NONE;

			ValueId()								{ type = INS_VALUE_ID; }

			void print() override					{}

			Token get_type() override				{ return ty; }

			std::string get_value() override		{ return name; }

			static bool check_class(Instruction* i) { return i->type == INS_VALUE_ID; }
		};

		/*
		* BinaryOp
		*/
		struct BinaryOp : public Instruction
		{
			std::string value;
			
			Instruction* left = nullptr;

			Token op = TOKEN_NONE,
				  ty = TOKEN_NONE;

			Instruction* right = nullptr;

			BinaryOp()								{ type = INS_BINARY_OP; }

			void print() override;

			Token get_type() override				{ return ty; }

			std::string get_value() override		{ return value; }

			static bool check_class(Instruction* i) { return i->type == INS_BINARY_OP; }
		};

		/*
		* UnaryOp
		*/
		struct UnaryOp : public Instruction
		{
			std::string value;

			Instruction* operand = nullptr;

			Token op = TOKEN_NONE;

			UnaryOp()								{ type = INS_UNARY_OP; }

			void print() override;

			Token get_type() override				{ return op; }

			std::string get_value() override		{ return value; }

			static bool check_class(Instruction* i) { return i->type == INS_UNARY_OP; }
		};

		/*
		* StackAlloc
		*/
		struct StackAlloc : public Instruction
		{
			std::string value;

			Token ty;

			StackAlloc()							{ type = INS_STACK_ALLOC; }

			void print() override;
			
			Token get_type() override				{ return ty; }

			std::string get_value() override		{ return value; }

			static bool check_class(Instruction* i) { return i->type == INS_STACK_ALLOC; }
		};

		/*
		* Store
		*/
		struct Store : public Instruction
		{
			std::string value;

			Token ty = TOKEN_NONE;

			Instruction* operand = nullptr;

			Store()									{ type = INS_STORE; }

			void print() override;
			
			Token get_type() override				{ return ty; }

			std::string get_value() override		{ return value; }
			
			static bool check_class(Instruction* i) { return i->type == INS_STORE; }
		};

		/*
		* Load
		*/
		struct Load : public Instruction
		{
			std::string dest_value;

			ValueId* value = nullptr;

			Token ty = TOKEN_NONE;

			Load()									{ type = INS_LOAD; }

			void print() override;

			Token get_type() override				{ return ty; }

			std::string get_value() override		{ return dest_value; }

			static bool check_class(Instruction* i) { return i->type == INS_LOAD; }
		};

		/*
		* Compare
		*/
		struct Compare : public Instruction
		{
			std::vector<Instruction*> items;

			Token ty = TOKEN_NONE;

			Compare()								{ type = INS_COMPARE; }

			void print() override;

			Token get_type() override				{ return ty; }

			std::string get_value() override		{ return {}; }

			static bool check_class(Instruction* i) { return i->type == INS_COMPARE; }
		};

		/*
		* Block
		*/
		struct Block : public Instruction
		{
			std::vector<Instruction*> items;

			std::string name;
			
			Block()									{ type = INS_BLOCK; }

			void print() override;

			Token get_type() override				{ return TOKEN_NONE; }

			std::string get_value() override		{ return name; }

			bool is_empty() const					{ return items.empty(); }

			void add_item(Instruction* item)		{ items.push_back(item); }

			static bool check_class(Instruction* i) { return i->type == INS_BLOCK; }
		};

		struct PrototypeParam
		{
			std::string name;

			Token type = TOKEN_NONE;

			PrototypeParam(const std::string& name, Token type) : name(name), type(type) {}
		};

		struct Prototype
		{
			std::unordered_map<std::string, Instruction*> values;
			std::unordered_map<std::string, Instruction*> values_real_name_lookup;

			std::vector<PrototypeParam*> params;

			std::vector<Block*> blocks;

			size_t stack_size = 0,
				   aligned_stack_size = 0;

			std::string name;

			Body* body = nullptr;

			Token return_type = TOKEN_NONE;

			bool is_empty() const					{ return blocks.empty(); }

			void add_param(PrototypeParam* param)	{ params.push_back(param); }
			void add_block(Block* block)			{ blocks.push_back(block); }
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
			std::unordered_map<std::string, Instruction*> values,
												   values_real_name_lookup;

			std::unordered_map<std::string, Block*> blocks;

			Prototype* curr_prototype = nullptr;

			Block* curr_block = nullptr;

			size_t stack_size = 0,
				   aligned_stack_size = 0;

			void clear()
			{
				curr_prototype = nullptr;
				curr_block = nullptr;

				stack_size = aligned_stack_size = 0;

				values.clear();
				values_real_name_lookup.clear();
				blocks.clear();
			}

			void copy_to_prototype(Prototype* prototype)
			{
				prototype->values = values;
				prototype->values_real_name_lookup = values_real_name_lookup;
				prototype->stack_size = stack_size;
				prototype->aligned_stack_size = aligned_stack_size;
			}

			void create_item(Instruction* item)
			{
				curr_block->add_item(item);
			}

			Instruction* get_value_from_real_name(const std::string& name)
			{
				auto it = values_real_name_lookup.find(name);
				return (it != values_real_name_lookup.end() ? it->second : nullptr);
			}

			std::string create_value(const std::string& name, Instruction* item)
			{
				auto var_name = "v" + std::to_string(values.size());
				values.insert({ var_name, item });
				values_real_name_lookup.insert({ name, item });
				return var_name;
			}

			Block* create_block(bool add = false)
			{
				auto new_block = new Block();

				if (curr_block)
					new_block->name = "block_" + std::to_string(blocks.size());
				else new_block->name = "entry";

				blocks.insert({ new_block->name, new_block });

				if (add)
					add_block(new_block);

				return new_block;
			}

			void add_block(Block* block)
			{
				curr_prototype->add_block(curr_block = block);
			}
		};
	}

	inline std::string STRINGIFY_BINARY_OP(Token op)
	{
		switch (op)
		{
		case TOKEN_ADD:			return "add";
		case TOKEN_SUB:			return "sub";
		case TOKEN_MUL:			return "mul";
		case TOKEN_DIV:			return "div";
		case TOKEN_MOD:			return "mod";
		case TOKEN_XOR:			return "xor";
		case TOKEN_EQUAL:		return "equal";
		case TOKEN_NOT_EQUAL:	return "not equal";
		}

		return "unknown";
	}

	inline std::string STRINGIFY_UNARY_OP(Token op)
	{
		switch (op)
		{
		case TOKEN_ADD:		 return "+";
		case TOKEN_SUB:		 return "neg";
		case TOKEN_MUL:		 return "deref";
		case TOKEN_AND:		 return "address";
		case TOKEN_BOOL_NOT: return "bool not";
		case TOKEN_NOT:		 return "logical not";
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
		void print_block(ir::Block* block);
		void print_item(ir::Instruction* item);

		void add_prototype(ir::Prototype* prototype);

		bool generate();

		ir::Prototype* generate_prototype(ast::Prototype* prototype);
		ir::Body* generate_from_body(ast::StmtBody* body);

		ir::Instruction* generate_from_expr(ast::Expr* expr);
		ir::Instruction* generate_from_expr_decl_or_assign(ast::ExprDeclOrAssign* expr);
		ir::ValueInt* generate_from_expr_int_literal(ast::ExprIntLiteral* expr);
		ir::BinaryOp* generate_from_expr_binary_op(ast::ExprBinaryOp* expr);
		ir::UnaryOp* generate_from_expr_unary_op(ast::ExprUnaryOp* expr);
		ir::Load* generate_from_expr_id(ast::ExprId* expr);
		ir::Instruction* generate_from_if(ast::StmtIf* stmt_if);

		ir::Prototype* get_defined_prototype(ast::Prototype* prototype);

		ast::Prototype* get_prototype_definition(ast::Prototype* prototype_decl);
	};
}