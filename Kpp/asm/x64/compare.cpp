#include <defs.h>

#include <ir/ir.h>

#include "compare.h"
#include "registers.h"

using namespace kpp;

x64::instruction_list x64::generate_compare_instruction(ir::BinaryOp* i)
{
	auto left = i->op2_i->get_value(),
		 right = i->op3_i->get_value();

	return gen_compare(left, right);
}

x64::instruction_list x64::gen_compare(ir::Value* l, ir::Value* r)
{
	auto ie = new x64::Instruction();

	auto left_r = l->storage.r,
		 right_r = r->storage.r;

	if (left_r->id == RAX)
		ie->add_opcode(0x3D);
	else ie->add_opcode(0x39);

	ie->set_modrm(left_r->id, right_r->id, 0b11);

	ie->generate();

	return { ie };
}