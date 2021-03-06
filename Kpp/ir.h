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
			INS_BLOCK,
			INS_BRANCH_COND,
			INS_BRANCH,
			INS_RETURN,
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
		* Block
		*/
		struct Block : public Instruction
		{
			std::vector<Instruction*> items;

			std::string name;

			Block* prev = nullptr,
				 * next = nullptr;
			
			Block()									{ type = INS_BLOCK; }

			void remove_item(Instruction* item)
			{
				if (auto it = std::find(items.begin(), items.end(), item); it != items.end())
					items.erase(it);
			}

			void print() override;

			Token get_type() override				{ return TOKEN_NONE; }

			std::string get_value() override		{ return name; }

			bool is_empty() const					{ return items.empty(); }

			void add_item(Instruction* item)		{ items.push_back(item); }

			static bool check_class(Instruction* i) { return i->type == INS_BLOCK; }
		};

		/*
		* BranchCond
		*/
		struct BranchCond : public Instruction
		{
			Instruction* comparison = nullptr;

			Block* target_if_true = nullptr,
				 * target_if_false = nullptr;

			BranchCond()							{ type = INS_BRANCH_COND; }

			void print() override;

			Token get_type() override				{ return comparison->get_type(); }

			std::string get_value() override		{ return comparison->get_value(); }

			static bool check_class(Instruction* i) { return i->type == INS_BRANCH_COND; }
		};

		/*
		* Branch
		*/
		struct Branch : public Instruction
		{
			Block* target = nullptr;

			Branch()								{ type = INS_BRANCH; }

			void print() override;

			Token get_type() override				{ return target->get_type(); }

			std::string get_value() override		{ return target->get_value(); }

			static bool check_class(Instruction* i) { return i->type == INS_BRANCH; }
		};

		/*
		* Return
		*/
		struct Return : public Instruction
		{
			Token ty = TOKEN_NONE;

			Return()								{ type = INS_RETURN; }

			void print() override;

			Token get_type() override				{ return ty; }

			std::string get_value() override		{ return STRINGIFY_TYPE(ty); }

			static bool check_class(Instruction* i) { return i->type == INS_RETURN; }
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

			Token ret_ty = TOKEN_NONE;

			bool is_empty() const					{ return blocks.empty(); }

			void add_param(PrototypeParam* param)	{ params.push_back(param); }

			Block* get_last_block()					{ return blocks.back(); }
			Block* get_second_last_block()			{ return *(blocks.rbegin() + 1); }
		};

		struct IR
		{
			std::vector<Prototype*> prototypes;
		};

		struct GlobalInfo
		{
			std::unordered_map<std::string, Prototype*> prototypes;
		};

		struct IfContext
		{
			Block* if_block = nullptr,
				 * else_block = nullptr,
				 * end_block = nullptr;

			void clear()
			{
				if_block = else_block = end_block = nullptr;
			}
		};

		struct PrototypeInfo
		{
			
			std::unordered_map<std::string, Instruction*> values,
														  values_real_name_lookup;

			std::unordered_map<std::string, Block*> blocks_map;

			std::vector<Block*> blocks;

			size_t curr_block_num = 0;

			Prototype* curr_prototype = nullptr;

			IfContext if_context {};

			Block* curr_block = nullptr;

			size_t stack_size = 0,
				   aligned_stack_size = 0;

			bool set_branch_next_block_target = false;

			void clear()
			{
				curr_prototype = nullptr;
				curr_block = nullptr;

				stack_size = aligned_stack_size = curr_block_num = 0;

				values.clear();
				values_real_name_lookup.clear();
				blocks_map.clear();
				blocks.clear();

				if_context.clear();
			}

			void copy_to_prototype(Prototype* prototype)
			{
				prototype->values = values;
				prototype->values_real_name_lookup = values_real_name_lookup;
				prototype->stack_size = stack_size;
				prototype->aligned_stack_size = aligned_stack_size;
				prototype->blocks = blocks;
			}

			void create_new_branch_linked_to_next_block()
			{
				set_branch_next_block_target = true;
			}

			void add_item(Instruction* item)
			{
				curr_block->add_item(item);
			}

			void add_item_to_block(Block* block, Instruction* item)
			{
				block->add_item(item);
			}

			void insert_block(Block* block)
			{
				blocks_map.insert({ block->name, block });
				blocks.push_back(block);
			}

			void erase_block(Block* block)
			{
				blocks_map.erase(block->name);

				if (auto it = std::find(blocks.begin(), blocks.end(), block); it != blocks.end())
					blocks.erase(it);
			}

			Instruction* get_value_from_real_name(const std::string& name)
			{
				auto it = values_real_name_lookup.find(name);
				return (it != values_real_name_lookup.end() ? it->second : nullptr);
			}

			std::string add_value(const std::string& name, Instruction* item)
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
					new_block->name = "block_" + std::to_string(++curr_block_num);
				else new_block->name = "entry";

				if (add)
					add_block(new_block);

				return new_block;
			}

			void destroy_block(Block* block)
			{
				if (!block)
					return;

				if (blocks_map.find(block->name) != blocks_map.end())
				{
					auto fix_removed_branches = [&]()
					{
						for (auto&& b : blocks)
						{
							for (auto&& i : b->items)
							{
								if (auto branch = rtti::safe_cast<Branch>(i); branch && branch->target == block)
								{
									if (branch->target->next)
										branch->target = branch->target->next;
									else b->remove_item(branch);

									return true;
								}
							}
						}

						return false;
					};

					fix_removed_branches();

					erase_block(block);

					if (block->prev)
						block->prev->next = block->next;

					if (curr_block)
						curr_block->next = block;
				}

				delete block;
			}

			void add_block(Block* block)
			{
				if (set_branch_next_block_target)
				{
					create_branch(curr_block, block);

					set_branch_next_block_target = false;
				}

				insert_block(block);

				block->prev = curr_block;

				if (curr_block)
					curr_block->next = block;
				
				curr_block = block;
			}

			Branch* create_branch(Block* block = nullptr, Block* target = nullptr)
			{
				auto branch = new Branch();
				if (!branch)
					return nullptr;

				branch->target = target;

				if (block)
					add_item_to_block(block, branch);
				else add_item(branch);

				return branch;
			}

			Return* create_return(Token ty)
			{
				auto ret = new Return();

				ret->ty = ty;

				add_item(ret);

				return ret;
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
		case TOKEN_EQUAL:		return "cmp eq";
		case TOKEN_NOT_EQUAL:	return "cmp neq";
		case TOKEN_LOGICAL_AND:	return "and";
		case TOKEN_LOGICAL_OR:	return "or";
		}

		return "unknown";
	}

	inline std::string STRINGIFY_UNARY_OP(Token op)
	{
		switch (op)
		{
		case TOKEN_ADD:			return "+";
		case TOKEN_SUB:			return "neg";
		case TOKEN_MUL:			return "deref";
		case TOKEN_AND:			return "address";
		case TOKEN_NOT:			return "not";
		case TOKEN_LOGICAL_NOT:	return "logical not";
		case TOKEN_LOGICAL_AND:	return "logical and";
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
		ir::BinaryOp* generate_from_expr_binary_op_cond(ast::ExprBinaryOp* expr);
		ir::UnaryOp* generate_from_expr_unary_op(ast::ExprUnaryOp* expr);
		ir::Load* generate_from_expr_id(ast::ExprId* expr);
		ir::Instruction* generate_from_if(ast::StmtIf* stmt_if);

		ir::Prototype* get_defined_prototype(ast::Prototype* prototype);

		ast::Prototype* get_prototype_definition(ast::Prototype* prototype_decl);
	};
}