#include <defs.h>

#include <ir/ir.h>

#include "bit.h"
#include "memory.h"
#include "registers.h"

using namespace kpp;

x64::instruction_list x64::gen_bit_and(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	instruction_list ies;

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;

	auto ie = new Instruction();

	ie->add_opcode(0x21);

	if (op1_r == op3_r)
	{
		auto temp = get_reg(RCX);

		ies.add_instructions(gen_mov(temp, op3), gen_mov(op3, op2));

		ie->set_modrm(op1_r->id, temp->id, 0b11);
	}
	else if (op1_r != op2_r)
	{
		ies.add_instructions(gen_mov(op1, op2));

		ie->set_modrm(op1_r->id, op3_r->id, 0b11);
	}
	else
	{
		if (op3_r->id >= R8)
			ie->set_rex(0, 1);

		ie->set_modrm(op1_r->id, op3_r->id % R8, 0b11);
	}

	ie->generate();

	ies.add_instruction(ie);

	return ies;
}

x64::instruction_list x64::gen_bit_or(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	instruction_list ies;

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;

	auto ie = new Instruction();

	ie->add_opcode(0x09);

	if (op1_r == op3_r)
	{
		auto temp = get_reg(RCX);

		ies.add_instructions(gen_mov(temp, op3), gen_mov(op3, op2));

		ie->set_modrm(op1_r->id, temp->id, 0b11);
	}
	else if (op1_r != op2_r)
	{
		ies.add_instructions(gen_mov(op1, op2));

		ie->set_modrm(op1_r->id, op3_r->id, 0b11);
	}
	else
	{
		if (op3_r->id >= R8)
			ie->set_rex(0, 1);

		ie->set_modrm(op1_r->id, op3_r->id % R8, 0b11);
	}

	ie->generate();

	ies.add_instruction(ie);

	return ies;
}

x64::instruction_list x64::gen_bit_shr(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	instruction_list ies;

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;

	auto ie = new Instruction();

	const bool shift_once = (op3->storage.integer.u64 > 1);

	if (op1_r == op2_r)
	{
		ie->set_modrm(op1_r->id, 5, 0b11);
	}

	if (shift_once)
	{
		ie->add_opcode(0xC1);
		ie->set_imm(op3->storage.integer.u8, 1);
	}
	else ie->add_opcode(0xD1);

	ie->generate();

	ies.add_instruction(ie);

	return ies;
}

x64::instruction_list x64::gen_bit_shl(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	instruction_list ies;

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;

	auto ie = new Instruction();

	const bool shift_once = (op3->storage.integer.u64 > 1);

	if (op1_r == op2_r)
	{
		ie->set_modrm(op1_r->id, 4, 0b11);
	}

	if (shift_once)
	{
		ie->add_opcode(0xC1);
		ie->set_imm(op3->storage.integer.u8, 1);
	}
	else ie->add_opcode(0xD1);

	ie->generate();

	ies.add_instruction(ie);

	return ies;
}