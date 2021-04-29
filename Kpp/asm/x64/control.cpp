#include <defs.h>

#include <ir/ir.h>

#include "control.h"
#include "registers.h"
#include "memory.h"

using namespace kpp;

x64::instruction_list x64::generate_control_instruction(ir::Instruction* i)
{
	if (auto branch_cond = rtti::safe_cast<ir::BranchCond>(i))
		return gen_cond_jump(branch_cond);
	else if (auto branch = rtti::safe_cast<ir::Branch>(i))
		return gen_jump(branch);
	else if (auto ret = rtti::safe_cast<ir::Return>(i))
		return gen_ret(ret);

	return {};
}

x64::instruction_list x64::gen_cond_jump(ir::BranchCond* i)
{
	auto ie = new x64::Instruction();

	auto op = i->get_value();

	if (auto compare = rtti::safe_cast<ir::BinaryOp>(op->definer))
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

	ie->set_imm(0x0, 1);
	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_jump(ir::Branch* i)
{
	auto ie = new x64::Instruction();

	ie->add_opcode(0xEB);
	ie->set_imm(0x0, 1);
	ie->generate();

	return { ie };
}

x64::instruction_list x64::gen_ret(ir::Return* i)
{
	std::vector<x64::Instruction*> ies { new x64::Instruction() };

	/*if (i->op)
		ies.push_back(gen_mov(RAX, i->op));*/

	ies[0]->add_opcode(0xC3);
	ies[0]->generate();

	return ies;
}