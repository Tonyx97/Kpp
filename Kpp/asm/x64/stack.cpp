#include <defs.h>

#include <ir/ir.h>

#include "stack.h"
#include "registers.h"

using namespace kpp;

x64::instruction_list x64::gen_push(reg* r)
{
	auto ie = new Instruction();

	if (r->id >= R8)
		ie->set_rex(0, 0, 0, 1);

	ie->add_opcode(0x50 + r->id % R8);
	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_pop(reg* r)
{
	auto ie = new Instruction();

	if (r->id >= R8)
		ie->set_rex(0, 0, 0, 1);

	ie->add_opcode(0x58 + r->id % R8);
	ie->generate();

	return { ie };
}