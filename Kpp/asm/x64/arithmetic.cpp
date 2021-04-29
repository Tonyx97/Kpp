#include <defs.h>

#include <ir/ir.h>

#include "arithmetic.h"
#include "registers.h"
#include "compare.h"

using namespace kpp;

x64::instruction_list x64::generate_binary_op_instruction(ir::BinaryOp* i)
{
	auto op1 = i->op1,
		 op2 = i->op2_i->get_value(),
		 op3 = i->op3_i->get_value();

	switch (i->operation)
	{
	case TOKEN_ADD:			return gen_add(op2, op3);
	case TOKEN_SUB:			return gen_sub(op2, op3);
	case TOKEN_MUL:			return gen_mul(op1, op2, op3);
	case TOKEN_DIV:			return gen_div(op1);
	case TOKEN_XOR:			return gen_xor(op2, op3);
	case TOKEN_EQUAL:
	case TOKEN_NOT_EQUAL:
	case TOKEN_GT:
	case TOKEN_LT:
	case TOKEN_GTE:
	case TOKEN_LTE:			return x64::gen_compare(op2, op3);
	}

	return {};
}

x64::instruction_list x64::gen_add(ir::Value* op1, ir::Value* op2)
{
	auto ie = new x64::Instruction();

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r;

	if (op1_r->id != RAX)
	{
		if (op1_r && !op2_r)
			ie->add_opcode(0x81);
		else if (op1_r && op2_r)
			ie->add_opcode(0x01);
	}
	else ie->add_opcode(0x05);

	ie->set_modrm(op1_r->id, op2_r->id, 0b11);
	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_sub(ir::Value* op1, ir::Value* op2)
{
	auto ie = new x64::Instruction();

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r;
	
	int digit = 0;

	if (op1_r->id == RAX)
		ie->add_opcode(0x2D);
	else
	{
		if (op1_r && !op2_r)
		{
			ie->add_opcode(0x81);
			digit = 5;
		}
		else if (op1_r && op2_r)
			ie->add_opcode(0x29);
	}

	ie->set_modrm(op1_r->id, digit | op2_r->id, 0b11);
	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_mul(ir::Value* op1, ir::Value* op2, ir::Value* op3)
{
	auto ie = new x64::Instruction();

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r,
		 op3_r = op3->storage.r;

	if (op1_r == op2_r)
	{
		ie->add_opcodes(0x0F, 0xAF);
		ie->set_modrm(op2_r->id, op3_r->id, 0b11);
	}
	else
	{
		ie->add_opcode(0x6B);
		ie->set_modrm(op1_r->id, op2_r->id, 0b11);
		ie->set_imm(op3->storage.integer.u64, 1);
	}

	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_div(ir::Value* op1)
{
	auto ie = new x64::Instruction();

	auto op1_r = op1->storage.r;

	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_xor(ir::Value* op1, ir::Value* op2)
{
	auto ie = new x64::Instruction();

	auto op1_r = op1->storage.r,
		 op2_r = op2->storage.r;
	
	int digit = 0;

	if (op1_r->id == RAX)
		ie->add_opcode(0x35);

	switch (op2->storage_type)
	{
	case ir::STORAGE_INTEGER:
	{
		break;
	}
	case ir::STORAGE_REGISTER:
	{
		ie->add_opcode(0x31);
		ie->set_modrm(op1_r->id, digit | op2_r->id, 0b11);
		break;
	}
	}

	ie->generate();

	return { ie };
}