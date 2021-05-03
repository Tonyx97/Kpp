#pragma once

namespace kpp
{
	namespace ir { struct Instruction; }

	namespace x64
	{
		enum InsType
		{
			INS_INSTRUCTION,
			INS_LABEL,
			INS_UNKNOWN,
		};

		struct Instruction;

		using instruction_list_vec = std::vector<Instruction*>;
		using bytes_list = std::vector<uint8_t>;

		struct instruction_list
		{
			instruction_list_vec list;

			int total_bytes = 0;

			instruction_list()										{}
			instruction_list(Instruction* ie)						{ add_instruction(ie); }

			Instruction* first() const								{ return list.front(); }
			Instruction* last() const								{ return list.back(); }

			auto begin() const										{ return list.begin(); }
			auto end() const										{ return list.end(); }

			int size() const										{ return static_cast<int>(list.size()); }
			int calc_total_bytes();

			bool empty() const										{ return list.empty(); }

			void add_instruction(Instruction* ie)					{ list.push_back(ie); }
			void add_instructions(const instruction_list_vec& v)	{ list.insert(list.end(), v.begin(), v.end()); }
			void add_instructions(const instruction_list& v)		{ list.insert(list.end(), v.list.begin(), v.list.end()); }

			template <typename... A>
			void add_instructions(const A&... args)					{ (list.insert(list.end(), args.list.begin(), args.list.end()), ...); }
		};

		struct Instruction
		{
			bytes_list bytes {},
					   prefixes {},
					   opcodes {};

			struct
			{
				union
				{
					uint8_t value = 0;

					struct
					{
						uint8_t b : 1;
						uint8_t x : 1;
						uint8_t r : 1;
						uint8_t w : 1;
						uint8_t reserved : 4;
					};
				};

				bool used = false;

				void clear() { *this = {}; }
			} rex;

			struct
			{
				union
				{
					uint8_t value = 0;

					struct
					{
						uint8_t rm : 3;
						uint8_t reg : 3;
						uint8_t mod : 2;
					};
				};

				bool used = false;

				void clear() { *this = {}; }
			} modrm;

			struct
			{
				union
				{
					uint8_t value = 0;

					struct
					{
						uint8_t base : 3;
						uint8_t index : 3;
						uint8_t scale : 2;
					};
				};

				bool used = false;

				void clear() { *this = {}; }
			} sib;

			struct
			{
				uint64_t value = 0;
				uint8_t size = 0;
				bool used = false;

				void clear() { *this = {}; }
			} imm {};

			struct
			{
				uint64_t value = 0;
				uint8_t size = 0;
				bool used = false;

				void clear() { *this = {}; }
			} disp {};

			ir::Instruction* owner = nullptr;

			Instruction* prev = nullptr,
					   * next = nullptr,
					   * target_label = nullptr;

			InsType type = INS_INSTRUCTION;
			
			int len = 0;

			bool target_label_look_up = false,
				 jump_reversed = false;

			Instruction()																{}
			Instruction(uint8_t opcode)													{ add_opcode(opcode); generate(); }
			Instruction(ir::Instruction* owner) : owner(owner)							{}

			void add_prefix(uint8_t v)													{ prefixes.push_back(v); }
			void add_opcode(uint8_t v)													{ opcodes.push_back(v); }
			
			template <typename... A>
			void add_prefixes(A... args)												{ (prefixes.push_back(args), ...); }

			template <typename... A>
			void add_opcodes(A... args)													{ (opcodes.push_back(args), ...); }
			
			void set_rex(uint8_t w = 0, uint8_t r = 0, uint8_t x = 0, uint8_t b = 0)	{ rex.w = w;		 rex.r = r;			rex.x = x;			rex.b = b;	rex.used = true; }
			void set_modrm(uint8_t rm, uint8_t reg = 0, uint8_t mod = 0)				{ modrm.rm = rm;	 modrm.reg = reg;	modrm.mod = mod;				modrm.used = true; }
			void set_sib(uint8_t base, uint8_t index = 0, uint8_t scale = 0)			{ sib.base = base;	 sib.index = index;	sib.scale = scale;				sib.used = true; }
			void set_imm(uint64_t v, uint8_t size)										{ imm.value = v;	 imm.size = size;									imm.used = true; }
			void set_disp(uint64_t v, uint8_t size)										{ disp.value = v;	 disp.size = size;									disp.used = true; }
			void set_digit(uint8_t v)													{ modrm.reg |= v; }
			void set_target_label(Instruction* ie, bool look_up = false)				{ target_label = ie; target_label_look_up = look_up; }

			void generate()
			{
				for (auto b : prefixes) { ++len; bytes.push_back(b); }

				if (rex.used)			{ ++len; bytes.push_back((1 << 6) | rex.value); }

				for (auto b : opcodes)	{ ++len; bytes.push_back(b); }
				
				if (modrm.used)			{ ++len;			bytes.push_back(modrm.value); }
				if (sib.used)			{ ++len;			bytes.push_back(sib.value); }
				if (disp.used)			{ len += disp.size; bytes.insert(bytes.end(), (uint8_t*)&disp.value, (uint8_t*)&disp.value + disp.size); }
				if (imm.used)			{ len += imm.size;	bytes.insert(bytes.end(), (uint8_t*)&imm.value, (uint8_t*)&imm.value + imm.size); }
			}

			void regenerate()
			{
				bytes.clear();

				len = 0;

				generate();
			}

			void clear()
			{
				opcodes.clear();
				prefixes.clear();
				rex.clear();
				modrm.clear();
				sib.clear();
				imm.clear();
				disp.clear();
			}

			static bool check_class(Instruction* i) { return i->type == INS_INSTRUCTION; }
		};

		struct Label : public Instruction
		{
			int id;
			
			Label(int id) : id(id)					{ type = INS_LABEL; }

			static bool check_class(Instruction* i) { return i->type == INS_LABEL; }
		};

		void print_instruction(Instruction* ie);
	}
}