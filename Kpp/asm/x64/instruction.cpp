#include <defs.h>

#include "instruction.h"

using namespace kpp;

int x64::instruction_list::calc_total_bytes()
{
	total_bytes = 0;

	for (auto ie : list)
		total_bytes += ie->len;

	return total_bytes;
}

void x64::print_instruction(x64::Instruction* ie)
{
	if (!ie->prefixes.empty())
	{
		PRINT_NNL(C_GREEN, "prefixes:     ");

		dbg::print_vec_int<uint8_t>(C_GREEN, ie->prefixes, " ", [&](uint8_t b)
		{
			std::stringstream ss;

			ss << "0x" << std::setw(2) << std::setfill('0') << std::hex << int(b);

			return ss.str();
		});

		PRINT_NL;
	}

	if (!ie->opcodes.empty())
	{
		PRINT_NNL(C_GREEN, "opcodes:      ");

		dbg::print_vec_int<uint8_t>(C_GREEN, ie->opcodes, " ", [&](uint8_t b)
		{
			std::stringstream ss;

			ss << "0x" << std::setw(2) << std::setfill('0') << std::hex << int(b);

			return ss.str();
		});

		PRINT_NL;
	}

	if (ie->modrm.used)
		PRINT(C_GREEN, "ModRM:        0x%x", ie->modrm.value);

	if (ie->sib.used)
		PRINT(C_GREEN, "SIB:          0x%x", ie->sib.value);

	if (ie->disp.used)
		PRINT(C_GREEN, "Displacement: 0x%x (%i bytes)", ie->disp.value, ie->disp.value);

	if (ie->imm.used)
		PRINT(C_GREEN, "Immediate:    0x%llx (%i bytes)", ie->imm.value, ie->imm.size);
}