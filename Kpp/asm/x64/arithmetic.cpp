#include <defs.h>

#include <ir/ir.h>

#include "arithmetic.h"
#include "registers.h"
#include "compare.h"
#include "memory.h"
#include "conversion.h"
#include "bit.h"

using namespace kpp;

x64::instruction_list x64::generate_binary_op_instruction(ir::BinaryOp* i)
{
	auto op1 = i->op1,
		 op2 = i->op2_i->get_value(),
		 op3 = i->op3_i->get_value();

	switch (i->operation)
	{
	case TOKEN_ADD_ASSIGN:
	case TOKEN_ADD:			return gen_add(op1, op2, op3);
	case TOKEN_SUB_ASSIGN:
	case TOKEN_SUB:			return gen_sub(op1, op2, op3);
	case TOKEN_MUL_ASSIGN:
	case TOKEN_MUL:			return gen_mul(op1, op2, op3);
	case TOKEN_DIV_ASSIGN:
	case TOKEN_DIV:			return gen_div(op1, op2, op3);
	case TOKEN_XOR_ASSIGN:
	case TOKEN_XOR:			return gen_xor(op1, op2, op3);
	case TOKEN_AND:			return gen_bit_and(op1, op2, op3);
	case TOKEN_OR:			return gen_bit_or(op1, op2, op3);
	case TOKEN_SHR:			return gen_bit_shr(op1, op2, op3);
	case TOKEN_SHL:			return gen_bit_shl(op1, op2, op3);
	case TOKEN_EQUAL:
	case TOKEN_NOT_EQUAL:
	case TOKEN_GT:
	case TOKEN_LT:
	case TOKEN_GTE:
	case TOKEN_LTE:			return x64::gen_compare(op2, op3);
	}

	return {};
}

x64::instruction_list x64::gen_add(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	instruction_list ies;

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;

	auto ie = new Instruction();

	ie->add_opcode(0x01);

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

x64::instruction_list x64::gen_sub(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	instruction_list ies;

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;
	
	int digit = 0;

	auto ie = new Instruction();

	ie->add_opcode(0x29);

	if (op1_r == op3_r)
	{
		auto temp = get_reg(RCX);

		ies.add_instructions(gen_mov(temp, op3), gen_mov(op3, op2));

		ie->set_modrm(op1_r->id, digit | temp->id, 0b11);
	}
	else if (op1_r != op2_r)
	{
		ies.add_instructions(gen_mov(op1, op2));

		ie->set_modrm(op1_r->id, digit | op3_r->id, 0b11);
	}
	else ie->set_modrm(op1_r->id, digit | op3_r->id, 0b11);

	ie->generate();

	ies.add_instruction(ie);

	return ies;
}

x64::instruction_list x64::gen_mul(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	instruction_list ies;

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;

	auto ie = new Instruction();

	if (op1_r == op3_r)
	{
		auto temp = get_reg(RCX);

		ies.add_instructions(gen_mov(temp, op3), gen_mov(op3, op2));

		ie->add_opcodes(0x0F, 0xAF);
		ie->set_modrm(temp->id, op1_r->id, 0b11);
	}
	else if (op1_r != op2_r)
	{
		ies.add_instructions(gen_mov(op1, op2));

		ie->add_opcodes(0x0F, 0xAF);
		ie->set_modrm(op3_r->id, op1_r->id, 0b11);
	}
	else
	{
		ies.add_instructions(gen_mov(op1, op2));

		ie->add_opcodes(0x0F, 0xAF);
		ie->set_modrm(op3_r->id, op1_r->id, 0b11);
	}

	ie->generate();

	ies.add_instruction(ie);

	return ies;
}

x64::instruction_list x64::gen_div(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	instruction_list ies;

	auto ie = new Instruction();

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;

	ie->add_opcode(0xF7);
	ie->set_modrm(op3_r->id, 7, 0b11);
	ie->generate();

	auto rax = get_reg(RAX);

	ies.add_instructions(gen_mov(rax, op2));
	ies.add_instructions(gen_cwd_cdq());
	ies.add_instruction(ie);

	if (op1->ret)
		op1->ret->reg_in_place = true;
	else ies.add_instructions(gen_mov(rax, op1, true));

	return ies;
}

x64::instruction_list x64::gen_xor(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	auto ie = new Instruction();

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;
	
	int digit = 0;

	switch (op2->storage_type)
	{
	case ir::STORAGE_INTEGER:
	{
		break;
	}
	case ir::STORAGE_REGISTER:
	{
		ie->add_opcode(0x31);
		ie->set_modrm(op2_r->id, digit | op3_r->id, 0b11);
		break;
	}
	}

	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_xor(reg* r1, reg* r2)
{
	auto ie = new Instruction();

	ie->add_opcode(0x31);
	ie->set_modrm(r1->id, r2->id, 0b11);

	ie->generate();

	return { ie };
}