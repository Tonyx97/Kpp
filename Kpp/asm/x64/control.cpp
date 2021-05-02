#include <defs.h>

#include <ir/ir.h>

#include "control.h"
#include "registers.h"
#include "memory.h"

using namespace kpp;

x64::instruction_list x64::generate_control_instruction(ir::Instruction* i)
{
	if (auto branch = rtti::safe_cast<ir::Branch>(i))
		return gen_jump(branch);
	else if (auto branch_cond = rtti::safe_cast<ir::BranchCond>(i))
		return gen_cond_jump(branch_cond);
	else if (auto ret = rtti::safe_cast<ir::Return>(i))
		return gen_ret(ret);

	return {};
}

x64::instruction_list x64::gen_jump(ir::Branch* i)
{
	auto ie = new Instruction();

	ie->add_opcode(0xEB);
	ie->set_imm(0, 1);
	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_cond_jump(ir::BranchCond* i)
{
	auto ie = new Instruction(i);

	if (auto compare = rtti::safe_cast<ir::BinaryOp>(i->get_value()->definer))
	{
		switch (compare->operation)
		{
		case TOKEN_GTE:			ie->add_opcode(0x72); break;	// LT opcode
		case TOKEN_LT:			ie->add_opcode(0x73); break;	// GTE opcode
		case TOKEN_NOT_EQUAL:	ie->add_opcode(0x74); break;	// EQUAL opcode
		case TOKEN_EQUAL:		ie->add_opcode(0x75); break;	// NOT_EQUAL opcode
		case TOKEN_GT:			ie->add_opcode(0x76); break;	// LTE opcode
		case TOKEN_LTE:			ie->add_opcode(0x77); break;	// GT opcode
		}
	}

	ie->set_imm(0, 1);
	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_ret(ir::Return* i)
{
	instruction_list ies;

	if (i->op_i && !i->reg_in_place)
		ies.add_instructions(gen_mov(get_reg(RAX), i->op_i->get_value()));

	auto ie = new Instruction();

	ie->add_opcode(0xC3);
	ie->generate();

	ies.add_instruction(ie);

	return ies;
}

bool x64::fix_jump(Instruction* ie, int imm)
{
	const int size = util::math::in_range<int>(imm, 0, UINT8_MAX) ? 1 : 4;

	const bool expanded = (size != ie->imm.size);
	
	if (expanded)
	{
		ie->clear();

		if (auto owner = ie->owner)
		{
			auto op = owner->get_value();

			if (auto compare = rtti::safe_cast<ir::BinaryOp>(op->definer))
			{
				switch (compare->operation)
				{
				case TOKEN_GTE:			ie->add_opcodes(0x0F, 0x82); break;	// LT opcode
				case TOKEN_LT:			ie->add_opcodes(0x0F, 0x83); break;	// GTE opcode
				case TOKEN_NOT_EQUAL:	ie->add_opcodes(0x0F, 0x84); break;	// EQUAL opcode
				case TOKEN_EQUAL:		ie->add_opcodes(0x0F, 0x85); break;	// NOT_EQUAL opcode
				case TOKEN_GT:			ie->add_opcodes(0x0F, 0x86); break;	// LTE opcode
				case TOKEN_LTE:			ie->add_opcodes(0x0F, 0x87); break;	// GT opcode
				}
			}
		}
		else ie->add_opcode(0xE9);
	}

	ie->set_imm(imm, size);
	ie->regenerate();

	return expanded;
}