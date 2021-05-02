#pragma once

#include <ast/ast.h>

#include <dom_tree/dom_tree.h>

#include <asm/x64/instruction.h>

namespace kpp
{
	struct reg;
	struct ssa_ctx;

	namespace ir
	{
		enum InsType
		{
			INS_UNKNOWN,
			INS_BODY,
			INS_ALIAS,
			INS_VALUE_INT,
			INS_VALUE_ID,
			INS_BINARY_OP,
			INS_UNARY_OP,
			INS_CALL,
			INS_STACK_ALLOC,
			INS_STORE,
			INS_LOAD,
			INS_BLOCK,
			INS_BRANCH_COND,
			INS_BRANCH,
			INS_RETURN,
			INS_PHI,
		};

		enum StorageType
		{
			STORAGE_STACK,
			STORAGE_MEMORY,
			STORAGE_REGISTER,
			STORAGE_INTEGER,
			STORAGE_UNKNOWN,
		};

		struct Value;
		struct Block;
		struct Return;

		using ValueFn = const std::function<Value*(Value*)>&;
		using BlockFn = const std::function<void(Block*)>&;
		using BlockRetFn = const std::function<bool(Block*)>&;

		struct dom_set_order
		{
			bool operator() (Block* x, Block* y) const;
		};
		
		/*
		* Instruction
		*/
		struct Instruction
		{
			Block* block_owner = nullptr;
			
			InsType type = INS_UNKNOWN;

			int index = -1;
			
			virtual void print() = 0;
			virtual void for_each_left_op(ValueFn fn) = 0;
			virtual void for_each_right_op(ValueFn fn) = 0;
			virtual void for_each_op(ValueFn fn) = 0;
			virtual Token get_type() = 0;
			virtual Value* get_value() = 0;
			virtual void set_value(Value* v) = 0;
			virtual std::string get_value_str() = 0;
		};

		/*
		* Life
		*/
		struct Life
		{
			std::unordered_set<Block*> blocks;
			
			Instruction* first = nullptr,
					   * last = nullptr;

			void add_block(Block* b)		{ blocks.insert(b); }

			bool has_block(Block* b)		{ return (std::find(blocks.begin(), blocks.end(), b) != blocks.end()); }
		};

		/*
		* Value
		*/
		struct Value
		{
			std::unordered_set<Value*> vers;

			std::string name;

			Life life {};

			Block* block_owner = nullptr;

			Instruction* definer = nullptr;

			Value* original = nullptr;

			Return* ret = nullptr;

			struct
			{
				reg* r = nullptr;

				Int integer {};
			} storage;

			StorageType storage_type = STORAGE_UNKNOWN;

			int versions = 0;

			bool has_versions() const			{ return versions > 0; }

			Value* create_new();

			void copy_to(Value* v);
		};

		/*
		* Body
		*/
		struct Body : public Instruction
		{
			Body()										{ type = INS_BODY; }

			void print() override						{}
			
			void for_each_left_op(ValueFn fn) override	{}
			void for_each_right_op(ValueFn fn) override	{}
			void for_each_op(ValueFn fn) override	{}

			Token get_type() override					{ return TOKEN_NONE; }

			Value* get_value() override					{ return nullptr; }

			void set_value(Value* v) override			{}
			
			std::string get_value_str() override		{ return {}; }

			static bool check_class(Instruction* i)		{ return i->type == INS_BODY; }
		};

		/*
		* Alias
		*/
		struct Alias : public Instruction
		{
			Value* op1 = nullptr,
				 * op2 = nullptr;

			Alias()										{ type = INS_ALIAS; }

			void print() override;

			void for_each_left_op(ValueFn fn) override	{}
			void for_each_right_op(ValueFn fn) override	{}
			void for_each_op(ValueFn fn) override		{}

			Token get_type() override					{ return TOKEN_NONE; }

			Value* get_value() override					{ return nullptr; }

			void set_value(Value* v) override			{}

			std::string get_value_str() override		{ return {}; }

			static bool check_class(Instruction* i)		{ return i->type == INS_ALIAS; }
		};

		/*
		* ValueInt
		*/
		struct ValueInt : public Instruction
		{
			Value* op1 = nullptr,
				 * op2 = nullptr;

			Token ty = TOKEN_NONE;

			ValueInt()									{ type = INS_VALUE_INT; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{ if (auto new_val = fn(op1)) op1 = new_val; }
			void for_each_right_op(ValueFn fn) override	{}
			void for_each_op(ValueFn fn) override		{ for_each_left_op(fn); }
			
			Token get_type() override					{ return ty; }

			Value* get_value() override					{ return op1; }
			
			void set_value(Value* v) override			{ op1 = v; }
			
			std::string get_value_str() override		{ return op1->name; }
			//std::string get_value_str() override		{ return std::to_string(int_val.i64); }

			static bool check_class(Instruction* i)		{ return i->type == INS_VALUE_INT; }
		};

		/*
		* ValueId
		*/
		struct ValueId : public Instruction
		{
			Value* op = nullptr;

			Token ty = TOKEN_NONE;

			ValueId()									{ type = INS_VALUE_ID; }

			void print() override						{};
			
			void for_each_left_op(ValueFn fn) override	{ if (auto new_val = fn(op)) op = new_val; }
			void for_each_right_op(ValueFn fn) override	{}
			void for_each_op(ValueFn fn) override		{ for_each_left_op(fn); }

			Token get_type() override					{ return ty; }

			Value* get_value() override					{ return op; }
			
			void set_value(Value* v) override			{ op = v; }
			
			std::string get_value_str() override		{ return op ? op->name : ""; }

			static bool check_class(Instruction* i)		{ return i->type == INS_VALUE_ID; }
		};

		/*
		* BinaryOp
		*/
		struct BinaryOp : public Instruction
		{
			Value* op1 = nullptr;
			
			Instruction* op2_i = nullptr,
					   * op3_i = nullptr;

			Token operation = TOKEN_NONE,
				  ty = TOKEN_NONE;

			BinaryOp()									{ type = INS_BINARY_OP; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{ if (auto new_val = fn(op1)) op1 = new_val; }
			void for_each_right_op(ValueFn fn) override
			{
				if (auto new_val = fn(op2_i->get_value()))
					op2_i->set_value(new_val);

				if (auto new_val = fn(op3_i->get_value()))
					op3_i->set_value(new_val);
			}
			void for_each_op(ValueFn fn) override		{ for_each_left_op(fn); for_each_right_op(fn); }

			Token get_type() override					{ return ty; }

			Value* get_value() override					{ return op1; }
			
			void set_value(Value* v) override			{ op1 = v; }
			
			std::string get_value_str() override		{ return op1 ? op1->name : ""; }

			static bool check_class(Instruction* i)		{ return i->type == INS_BINARY_OP; }
		};

		/*
		* UnaryOp
		*/
		struct UnaryOp : public Instruction
		{
			Value* op1 = nullptr;

			Instruction* op2_i = nullptr;

			Token operation = TOKEN_NONE,
				  ty = TOKEN_NONE;

			UnaryOp()									{ type = INS_UNARY_OP; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{ if (auto new_val = fn(op1)) op1 = new_val; }
			void for_each_right_op(ValueFn fn) override	{ if (auto new_val = fn(op2_i->get_value())) op2_i->set_value(new_val); }
			void for_each_op(ValueFn fn) override		{ for_each_left_op(fn); for_each_right_op(fn); }

			Token get_type() override					{ return ty; }

			Value* get_value() override					{ return op1; }
			
			void set_value(Value* v) override			{ op1 = v; }
			
			std::string get_value_str() override		{ return op1 ? op1->name : ""; }

			static bool check_class(Instruction* i)		{ return i->type == INS_UNARY_OP; }
		};

		/*
		* Call
		*/
		struct Call : public Instruction
		{
			std::vector<Instruction*> params;

			struct Prototype* prototype = nullptr;

			Call()										{ type = INS_CALL; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{}
			void for_each_right_op(ValueFn fn) override	{}
			void for_each_op(ValueFn fn) override		{}

			Token get_type() override					{ return TOKEN_NONE; }

			Value* get_value() override					{ return nullptr; }
			
			void set_value(Value* v) override			{}
			
			std::string get_value_str() override		{ return {}; }

			static bool check_class(Instruction* i)		{ return i->type == INS_CALL; }
		};

		/*
		* StackAlloc
		*/
		struct StackAlloc : public Instruction
		{
			Value* op = nullptr;

			Token ty;

			StackAlloc()								{ type = INS_STACK_ALLOC; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{}
			void for_each_right_op(ValueFn fn) override	{}
			void for_each_op(ValueFn fn) override		{ for_each_left_op(fn); }
			
			Token get_type() override					{ return ty; }

			Value* get_value() override					{ return op; }
			
			void set_value(Value* v) override			{ op = v; }
			
			std::string get_value_str() override		{ return op ? op->name : ""; }

			static bool check_class(Instruction* i)		{ return i->type == INS_STACK_ALLOC; }
		};

		/*
		* Store
		*/
		struct Store : public Instruction
		{
			Value* op1 = nullptr;

			Instruction* op2_i = nullptr;

			Token ty = TOKEN_NONE;

			Store()										{ type = INS_STORE; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{ if (auto new_val = fn(op1)) op1 = new_val; }
			void for_each_right_op(ValueFn fn) override	{ if (auto new_val = fn(op2_i->get_value())) op2_i->set_value(new_val); }
			void for_each_op(ValueFn fn) override		{ for_each_left_op(fn); for_each_right_op(fn); }
			
			Token get_type() override					{ return ty; }

			Value* get_value() override					{ return op1; }
			
			void set_value(Value* v) override			{ op1 = v; }
			
			std::string get_value_str() override		{ return op1 ? op1->name : ""; }
			
			static bool check_class(Instruction* i)		{ return i->type == INS_STORE; }
		};

		/*
		* Load
		*/
		struct Load : public Instruction
		{
			Value* op1 = nullptr;

			ValueId* op2_i = nullptr;

			Token ty = TOKEN_NONE;

			Load()										{ type = INS_LOAD; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{ if (auto new_val = fn(op1)) op1 = new_val; }
			void for_each_right_op(ValueFn fn) override	{ if (auto new_val = fn(op2_i->get_value())) op2_i->set_value(new_val); }
			void for_each_op(ValueFn fn) override		{ for_each_left_op(fn); for_each_right_op(fn); }

			Token get_type() override					{ return ty; }

			Value* get_value() override					{ return op1; }
			
			void set_value(Value* v) override			{ op1 = v; }
			
			std::string get_value_str() override		{ return op1 ? op1->name : ""; }

			static bool check_class(Instruction* i)		{ return i->type == INS_LOAD; }
		};
		
		/*
		* Phi
		*/
		struct Phi : public Instruction
		{
			std::vector<Block*> blocks;

			std::unordered_set<Value*> values;
			
			Value* op = nullptr;

			Phi(Value* op) : op(op)						{ type = INS_PHI; }

			void add_block(Block* b)					{ blocks.push_back(b); }
			void add_value(Value* v)					{ values.insert(v); }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{ if (auto new_val = fn(op)) op = new_val; }
			void for_each_right_op(ValueFn fn) override	{ for (auto v : values) fn(v); }
			void for_each_op(ValueFn fn) override		{ for_each_left_op(fn); }

			Token get_type() override					{ return TOKEN_NONE; }

			Value* get_value() override					{ return op; }
			
			void set_value(Value* v) override			{ op = v; }
			
			std::string get_value_str() override		{ return op ? op->name : ""; }

			static bool check_class(Instruction* i)		{ return i->type == INS_PHI; }
		};

		/*
		* Block
		*/
		struct Block : public Instruction
		{
			std::vector<Instruction*> items;
			std::vector<Phi*> phis;
			std::vector<Block*> refs;

			std::set<Block*, dom_set_order> doms;

			std::unordered_set<reg*> regs_assigned;

			std::string name;

			Block* prev = nullptr,
				 * next = nullptr,
				 * idom = nullptr;

			int reverse_postorder_index = -1;

			bool is_entry = false,
				 is_last = false;

			struct
			{
				x64::Instruction* label = nullptr;
			} asm_info {};

			using items_it = decltype(items)::iterator;

			Block()														{ type = INS_BLOCK; }

			bool is_empty() const										{ return items.empty(); }

			Block* get_last_ref() const									{ return (refs.empty() ? nullptr : refs.back()); }

			Block* get_asm_target(bool target_if_false = true);

			Instruction* get_jump_item()								{ return (items.empty() ? nullptr : items.back()); }
			
			x64::Instruction* get_label()								{ return asm_info.label; }

			void add_item(Instruction* item)							{ item->block_owner = this; items.push_back(item); }
			void add_phi(Phi* phi)										{ phi->block_owner = this; items.insert(items.begin(), phi); phis.push_back(phi); }
			void add_ref(Block* block)									{ refs.push_back(block); }
			void add_items(const items_it& begin, const items_it& end)	{ items.insert(items.end(), begin, end); }

			void add_phi_alias(Value* op1, Value* op2)
			{
				auto alias = new Alias();

				alias->op1 = op1;
				alias->op2 = op2;
				alias->block_owner = this;

				items.insert(items.end() - 1, alias);
			}

			void set_label(x64::Instruction* label)						{ asm_info.label = label; }

			void fix_ref(Block* old_block, Block* new_block)
			{
				if (auto it = std::find(refs.begin(), refs.end(), old_block); it != refs.end())
					*it = new_block;
			}

			void for_each_successor(BlockFn fn);
			void for_each_successor_deep(BlockRetFn fn);
			void for_each_dom(BlockRetFn fn);

			void print() override;
			
			void for_each_left_op(ValueFn fn) override					{}
			void for_each_right_op(ValueFn fn) override					{}
			void for_each_op(ValueFn fn) override						{}

			Token get_type() override									{ return TOKEN_NONE; }

			Value* get_value() override									{ return nullptr; }
			
			void set_value(Value* v) override							{}
			
			std::string get_value_str() override						{ return name; }

			static bool check_class(Instruction* i)						{ return i->type == INS_BLOCK; }
		};

		/*
		* BranchCond
		*/
		struct BranchCond : public Instruction
		{
			Instruction* comparison = nullptr;

			Block* target_if_true = nullptr,
				 * target_if_false = nullptr;

			BranchCond()								{ type = INS_BRANCH_COND; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{}
			void for_each_right_op(ValueFn fn) override	{ if (auto new_val = fn(comparison->get_value())) comparison->set_value(new_val); }
			void for_each_op(ValueFn fn) override		{ for_each_right_op(fn); }

			Token get_type() override					{ return comparison->get_type(); }

			Value* get_value() override					{ return comparison->get_value(); }
			
			void set_value(Value* v) override			{ comparison->set_value(v); }
			
			std::string get_value_str() override		{ return {}; }

			static bool check_class(Instruction* i)		{ return i->type == INS_BRANCH_COND; }
		};

		/*
		* Branch
		*/
		struct Branch : public Instruction
		{
			Block* target = nullptr;

			bool unused = false;

			Branch()									{ type = INS_BRANCH; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{}
			void for_each_right_op(ValueFn fn) override	{}
			void for_each_op(ValueFn fn) override		{}

			Token get_type() override					{ return target->get_type(); }

			Value* get_value() override					{ return target->get_value(); }
			
			void set_value(Value* v) override			{ target->set_value(v); }
			
			std::string get_value_str() override		{ return target ? target->name : ""; }

			static bool check_class(Instruction* i)		{ return i->type == INS_BRANCH; }
		};

		/*
		* Return
		*/
		struct Return : public Instruction
		{
			Instruction* op_i = nullptr;

			Token ty = TOKEN_NONE;

			bool reg_in_place = false;

			Return()									{ type = INS_RETURN; }

			void print() override;
			
			void for_each_left_op(ValueFn fn) override	{}
			void for_each_right_op(ValueFn fn) override
			{ 
				if (op_i)
					if (auto new_val = fn(op_i->get_value()))
						op_i->set_value(new_val);
			}

			void for_each_op(ValueFn fn) override		{ for_each_right_op(fn); }

			Token get_type() override					{ return ty; }

			Value* get_value() override					{ return op_i->get_value(); }
			
			void set_value(Value* v) override			{ op_i->set_value(v); }
			
			std::string get_value_str() override		{ return op_i ? op_i->get_value()->name : ""; }

			static bool check_class(Instruction* i)		{ return i->type == INS_RETURN; }
		};

		struct PrototypeParam
		{
			std::string name;

			Token type = TOKEN_NONE;

			PrototypeParam(const std::string& name, Token type) : name(name), type(type) {}
		};

		struct Prototype
		{
			std::unordered_map<std::string, Value*> values,
													values_real_name_lookup;

			std::vector<PrototypeParam*> params;

			std::vector<Block*> blocks;

			dom_tree* dominance_tree = nullptr;

			ssa_ctx* ssa = nullptr;

			size_t stack_size = 0,
				   aligned_stack_size = 0;

			std::string name;

			Body* body = nullptr;

			Token ret_ty = TOKEN_NONE;

			Prototype()								{}
			~Prototype()							{ delete dominance_tree; }

			bool is_empty() const					{ return blocks.empty(); }

			void add_param(PrototypeParam* param)	{ params.push_back(param); }
			
			Block* get_entry_block()				{ return (blocks.empty() ? nullptr : blocks[0]); }
			Block* get_exit_block()					{ return (blocks.empty() ? nullptr : blocks.back()); }
			Block* get_second_last_block()			{ return (blocks.empty() ? nullptr : *(blocks.rbegin() + 1)); }
		};

		struct global_info
		{
			std::unordered_map<std::string, Prototype*> prototypes;
		};

		struct if_context
		{
			Block* if_block = nullptr,
				 * else_block = nullptr;

			void clear() { if_block = else_block = nullptr; }
		};

		struct prototype_info
		{
			std::unordered_map<std::string, Value*> values,
													values_real_name_lookup;

			std::unordered_map<std::string, Block*> blocks_map;

			std::vector<Block*> blocks;

			if_context if_context {};

			Prototype* curr_prototype = nullptr;

			Block* curr_block = nullptr;

			size_t stack_size = 0,
				   aligned_stack_size = 0;

			int curr_block_index = 0,
				curr_item_index = 0,
				curr_temp_var_index = 0;

			void clear()
			{
				curr_prototype = nullptr;
				curr_block = nullptr;

				stack_size = aligned_stack_size = 0;
				curr_block_index = curr_item_index = curr_temp_var_index = 0;

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

			void create_branch_in_block_to_next(Block* block)
			{
				if (block && block->next)
					create_branch(block, block->next);
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

			Value* get_value_from_real_name(const std::string& name)
			{
				auto it = values_real_name_lookup.find(name);
				return (it != values_real_name_lookup.end() ? it->second : nullptr);
			}

			Value* add_value(const std::string& name, Instruction* item)
			{
				auto value = new Value();

				value->block_owner = curr_block;
				value->definer = item;

				if (rtti::safe_cast<StackAlloc>(item))
				{
					value->name = name;
					//value->storage_type = STORAGE_STACK;
				}
				else
				{
					value->name = "v" + std::to_string(curr_temp_var_index++);
				}

				value->storage_type = STORAGE_REGISTER;

				values.insert({ value->name, value });
				values_real_name_lookup.insert({ name, value });

				return value;
			}

			Block* create_block(bool add = false)
			{
				auto new_block = new Block();

				if (add)
					add_block(new_block);

				return new_block;
			}

			void clear_out_unused_blocks()
			{
				std::stack<Block*> unused_blocks;

				for (auto&& b : blocks | std::views::reverse)
				{
					if (b->refs.empty())
					{
						unused_blocks.push(b);

						if (auto prev_block = b->prev)
						{
							prev_block->add_items(b->items.begin(), b->items.end());

							for (auto&& item : b->items)
							{
								if (auto branch = rtti::safe_cast<Branch>(item))
									branch->target->fix_ref(b, prev_block);
								else if (auto bcond = rtti::safe_cast<BranchCond>(item))
								{
									bcond->target_if_true->fix_ref(b, prev_block);
									bcond->target_if_false->fix_ref(b, prev_block);
								}
							}
						}
					}
				}

				while (!unused_blocks.empty())
				{
					destroy_block(unused_blocks.top());
					unused_blocks.pop();
				}
			}

			void destroy_block(Block* block)
			{
				if (!block)
					return;

				if (!block->name.empty() && blocks_map.find(block->name) != blocks_map.end())
				{
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
				if (curr_block)
					block->name = "block_" + std::to_string(blocks.size() + 1);
				else
				{
					block->name = "entry";
					block->is_entry = true;
				}

				insert_block(block);

				block->prev = curr_block;

				if (curr_block)
					curr_block->next = block;
				
				curr_block = block;
				curr_block->index = curr_block_index++;

				curr_item_index = 0;
			}

			Branch* create_branch(Block* block, Block* target)
			{
				auto branch = new Branch();
				if (!branch)
					return nullptr;

				branch->target = target;

				target->add_ref(block ? block : curr_block);

				if (block)
					add_item_to_block(block, branch);
				else add_item(branch);

				return branch;
			}
		};

		struct IR
		{
			std::vector<Prototype*> prototypes;
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
		case TOKEN_NOT_EQUAL:	return "cmp ne";
		case TOKEN_LT:			return "cmp lt";
		case TOKEN_LTE:			return "cmp lte";
		case TOKEN_GT:			return "cmp gt";
		case TOKEN_GTE:			return "cmp gte";
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

		ir::global_info gi {};

		ir::prototype_info pi {};

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
		ir::Call* generate_from_expr_call(ast::ExprCall* expr);
		bool generate_from_expr_binary_op_cond(ast::ExprBinaryOp* expr, ir::Block* target_if_true = nullptr, ir::Block* target_if_false = nullptr);
		ir::Instruction* generate_from_if(ast::StmtIf* stmt_if);
		ir::Instruction* generate_from_return(ast::StmtReturn* stmt_return);

		ir::Prototype* get_defined_prototype(ast::Prototype* prototype);

		ast::Prototype* get_prototype_definition(ast::Prototype* prototype_decl);

		ir::IR& get_ir_info() { return iri; }
	};
}