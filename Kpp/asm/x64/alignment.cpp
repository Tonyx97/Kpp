#include <defs.h>

#include <ir/ir.h>

#include "alignment.h"
#include "registers.h"

using namespace kpp;

x64::instruction_list x64::gen_fn_padding(int count)
{
	instruction_list ies;

	for (int i = 0; i < count; ++i)
		ies.add_instruction(new Instruction(0xCC));

	return ies;
}