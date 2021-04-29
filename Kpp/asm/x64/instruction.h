#pragma once

namespace kpp
{
	namespace x64
	{
		using instruction_list = std::vector<struct Instruction*>;
		using bytes_list = std::vector<uint8_t>;

		struct Instruction
		{
			bytes_list all_bytes {},
					   prefixes {},
					   opcodes {};

			struct
			{
				uint64_t value = 0;
				uint8_t size = 0;
				bool used = false;
			} imm {};

			struct
			{
				uint64_t value = 0;
				uint8_t size = 0;
				bool used = false;
			} disp {};

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
			} sib;
			
			int len = 0;

			void add_prefix(uint8_t v)											{ prefixes.push_back(v); }
			void add_opcode(uint8_t v)											{ opcodes.push_back(v); }
			
			template <typename... A>
			void add_prefixes(A... args)										{ (prefixes.push_back(args), ...); }

			template <typename... A>
			void add_opcodes(A... args)											{ (opcodes.push_back(args), ...); }
			
			void set_modrm(uint8_t rm, uint8_t reg = 0, uint8_t mod = 0)		{ modrm.rm = rm;	modrm.reg = reg;	modrm.mod = mod;	modrm.used = true; }
			void set_sib(uint8_t base, uint8_t index = 0, uint8_t scale = 0)	{ sib.base = base;	sib.index = index;	sib.scale = scale;	sib.used = true; }
			void set_imm(uint64_t v, uint8_t size)								{ imm.value = v;	imm.size = size;						imm.used = true; }
			void set_disp(uint64_t v, uint8_t size)								{ disp.value = v;	disp.size = size;						disp.used = true; }

			void generate()
			{
				for (auto b : prefixes) { ++len; all_bytes.push_back(b); }
				for (auto b : opcodes)	{ ++len; all_bytes.push_back(b); }

				if (modrm.used) { ++len;			all_bytes.push_back(modrm.value); }
				if (sib.used)	{ ++len;			all_bytes.push_back(sib.value); }
				if (disp.used)  { len += disp.size; all_bytes.insert(all_bytes.end(), (uint8_t*)&disp.value, (uint8_t*)&disp.value + disp.size); }
				if (imm.used)   { len += imm.size;	all_bytes.insert(all_bytes.end(), (uint8_t*)&imm.value, (uint8_t*)&imm.value + imm.size); }
			}

			void regenerate()
			{
				all_bytes.clear();
				len = 0;
				generate();
			}
		};

		void print_instruction(Instruction* i);
	}
}